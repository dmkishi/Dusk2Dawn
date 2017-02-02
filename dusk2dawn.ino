#include <Math.h>




/* Set defaults
 */
float const LATITUDE  =   37.776445;
float const LONGITUDE = -122.441448;




void setup() {
  Serial.begin(115200);

  int y = 2017;
  int m = 2;
  int d = 1;
  int timezone = -8;
  bool isDst = true;

  float sunrise = sunriseSet(true, y, m, d, LATITUDE, LONGITUDE, timezone, isDst);
  float sunset  = sunriseSet(false, y, m, d, LATITUDE, LONGITUDE, timezone, isDst);

  Serial.println(sunrise);
  Serial.println(sunset);
  timeString(sunrise, false);
  timeString(sunset, false);
}


void loop() {}



/* ================================== MAIN ================================== */
float sunriseSet(bool isRise, int y, int m, int d, float latitude, float longitude, int timezone, bool dst) {
  float t = fractionOfCentury(jDay(y, m, d));

  return sunriseSetUTC(isRise, t, latitude, longitude);
}


float sunriseSetUTC(bool isRise, float t, float latitude, float longitude) {
  float eqTime    = equationOfTime(t);
  float solarDec  = sunDeclination(t);
  float hourAngle = hourAngleSunrise(latitude, solarDec);

  hourAngle = isRise ?: -hourAngle;
  float delta   = longitude + radToDeg(hourAngle);
  float timeUTC = 720 - (4.0 * delta) - eqTime; // in minutes
  return timeUTC;
}




/* ============================ EQUATION OF TIME ============================ */
/* The difference between mean solar time (as shown by clocks) and apparent
 * solar time (indicated by sundials), which varies with the time of year.
 */
float equationOfTime(float t) {
  float epsilon = obliquityCorrection(t);
  float l0      = geomMeanLongSun(t);
  float e       = eccentricityEarthOrbit(t);
  float m       = geomMeanAnomalySun(t);

  float y = tan(degToRad(epsilon) / 2.0);
  y *= y;

  float sin2l0 = sin(2.0 * degToRad(l0));
  float sinm   = sin(degToRad(m));
  float cos2l0 = cos(2.0 * degToRad(l0));
  float sin4l0 = sin(4.0 * degToRad(l0));
  float sin2m  = sin(2.0 * degToRad(m));

  float Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0 - 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;
  return radToDeg(Etime) * 4.0; // in minutes of time
}


/* Obliquity of the ecliptic is the term used by astronomers for the inclination
 * of Earth's equator with respect to the ecliptic, or of Earth's rotation axis 
 * to a perpendicular to the ecliptic.
 */
float  meanObliquityOfEcliptic(float t) {
  float seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * 0.001813));
  float e0      = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
  return e0; // in degrees
}


float eccentricityEarthOrbit(float t) {
  float e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
  return e; // unitless
}


/* =========================== SOLAR DECLINATION ============================ */
float sunDeclination(float t) {
  float e      = obliquityCorrection(t);
  float lambda = sunApparentLong(t);

  float sint  = sin(degToRad(e)) * sin(degToRad(lambda));
  float theta = radToDeg(asin(sint));
  return theta; // in degrees
}


float sunApparentLong(float t) {
  float o      = sunTrueLong(t);
  float omega  = 125.04 - 1934.136 * t;
  float lambda = o - 0.00569 - 0.00478 * sin(degToRad(omega));
  return lambda; // in degrees
}


float sunTrueLong(float t) {
  float l0 = geomMeanLongSun(t);
  float c  = sunEqOfCenter(t);
  float O  = l0 + c;
  return O; // in degrees
}


float sunEqOfCenter(float t) {
  float m     = geomMeanAnomalySun(t);
  float mrad  = degToRad(m);
  float sinm  = sin(mrad);
  float sin2m = sin(mrad * 2);
  float sin3m = sin(mrad * 3);
  float C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
  return C; // in degrees
}


/* =============================== HOUR ANGLE =============================== */
float hourAngleSunrise(float lat, float solarDec) {
  float latRad = degToRad(lat);
  float sdRad  = degToRad(solarDec);
  float HAarg  = (cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));
  float HA     = acos(HAarg);
  return HA; // in radians (for sunset, use -HA)
}



/* ============================ SHARED FUNCTIONS ============================ */
float obliquityCorrection(float t) {
  float e0    = meanObliquityOfEcliptic(t);
  float omega = 125.04 - 1934.136 * t;
  float e     = e0 + 0.00256 * cos(degToRad(omega));
  return e; // in degrees
}


float geomMeanLongSun(float t) {
  float L0 = 280.46646 + t * (36000.76983 + t * 0.0003032);
  while (L0 > 360.0) {
    L0 -= 360.0;
  }
  while (L0 < 0.0) {
    L0 += 360.0;
  }
  return L0; // in degrees
}


float geomMeanAnomalySun(float t) {
  float M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
  return M; // in degrees
}



/* =========================== UTILITY FUNCTIONS ============================ */
/* Convert Gregorian calendar date to Julian Day.
 */
float jDay(int year, int month, int day) {
  if (month <= 2) {
    year  -= 1;
    month += 12;
  }

  int A = floor(year/100);
  int B = 2 - A + floor(A/4);
  return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) +
         day + B - 1524.5;
}


/* Return fraction of time elapsed this century, AD 2000–2100 Gregorian calendar.
 *
 * NOTE: 2,451,545 was the Julian day starting at noon UTC on 1 January AD 2000.
 *       36,525 is a Julian century.
 */
float fractionOfCentury(float jd) {
  return (jd - 2451545.0) / 36525.0;
}


/* Convert time in minutes to zero-padded string (HH:MM:SS).
 */
String timeString(float minutes, bool showSeconds) {
  float floatHour, floatMinute, floatSec;
  int   hour, minute, second;
  
  if ((minutes >= 0) && (minutes < 1440)) {
    floatHour   = minutes / 60.0;
    floatMinute = 60.0 * (floatHour - floor(floatHour));
    floatSec    = 60.0 * (floatMinute - floor(floatMinute));
    hour   = (int) floatHour;
    minute = (int) floatMinute;
    second = (int) (floatSec + 0.5);
    
    if (second > 59) {
      second = 0;
      minute += 1;
    }
    if (showSeconds && (second >= 30)) {
      minute++;
    }
    if (minute > 59) {
      minute = 0;
      hour  += 1;
    }
  }
Serial.println(hour);
Serial.println(minute);
//    String output = zeroPad(hour,2) + ":" + zeroPad(minute,2);
//    if (!showSeconds) output = output + ":" + zeroPad(second,2);
//  } else { 
//    var output = "error"
//  }
  return "a";
}


float radToDeg(float rad) {
  return 180.0 * rad / PI;
}


float degToRad(float deg) {
  return PI * deg / 180.0;
}

