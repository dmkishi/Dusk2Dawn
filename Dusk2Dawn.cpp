/*  Dusk2Dawn.h
 *  Get estimate time of sunrise and sunset given a set of coordinates.
 *  Created by DM Kishi <dm.kishi@gmail.com> on 2017-02-01
 *  <https://github.com/dmkishi/Dusk2Dawn>
 */

#include "Arduino.h"
#include <Math.h>
#include "TimeLord.h"




/******************************************************************************/
/*                                   PUBLIC                                   */
/******************************************************************************/
Dusk2Dawn::Dusk2Dawn(float latitude, float longitude, int timezone) {
  _latitude  = latitude;
  _longitude = longitude;
  _timezone  = timezone;
}


int Dusk2Dawn::sunriseMinute(int y, int m, int d, bool isDST) {
  return sunriseSet(true, y, m, d, isDST);
}


int Dusk2Dawn::sunsetMinute(int y, int m, int d, bool isDST) {
  return sunriseSet(false, y, m, d, isDST);
}




/******************************************************************************/
/*                                  PRIVATE                                   */
/******************************************************************************/
int Dusk2Dawn::sunriseSet(bool isRise, int y, int m, int d, bool isDST) {
  float jday, newJday, timeUTC, newTimeUTC, timeLocal;
  bool  hasSunriseSet;

  jday    = jDay(y, m, d);
  timeUTC = sunriseSetUTC(isRise, jday, _latitude, _longitude);

  // Advance the calculated time by a fraction of itself. I've no idea what the
  // purpose of this is.
  newJday    = jday + timeUTC / (60 * 24);
  newTimeUTC = sunriseSetUTC(isRise, newJday, _latitude, _longitude);

  hasSunriseSet = !isnan(newTimeUTC);
  if (hasSunriseSet) {
    timeLocal  = newTimeUTC + (_timezone * 60);
    timeLocal += (isDST) ? 60 : 0;
    return timeString(timeLocal, 2);
  } else {
    // We're in the (ant)arctic and there is no sunrise (or sunset) today!
    // DO WHAT
  }
}


float Dusk2Dawn::sunriseSetUTC(bool isRise, float jday, float latitude, float longitude) {
  float t         = fractionOfCentury(jday);
  float eqTime    = equationOfTime(t);
  float solarDec  = sunDeclination(t);
  float hourAngle = hourAngleSunrise(latitude, solarDec);

  hourAngle = isRise ? hourAngle : -hourAngle;
  float delta   = longitude + radToDeg(hourAngle);
  float timeUTC = 720 - (4 * delta) - eqTime; // in minutes
  return timeUTC;
}


/* ---------------------------- EQUATION OF TIME ---------------------------- */
/* The difference between mean solar time (as shown by clocks) and apparent
 * solar time (indicated by sundials), which varies with the time of year.
 */
float Dusk2Dawn::equationOfTime(float t) {
  float epsilon = obliquityCorrection(t);
  float l0      = geomMeanLongSun(t);
  float e       = eccentricityEarthOrbit(t);
  float m       = geomMeanAnomalySun(t);

  float y = tan(degToRad(epsilon) / 2);
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
float Dusk2Dawn::meanObliquityOfEcliptic(float t) {
  float seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * 0.001813));
  float e0      = 23 + (26 + (seconds / 60)) / 60;
  return e0; // in degrees
}


float Dusk2Dawn::eccentricityEarthOrbit(float t) {
  float e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
  return e; // unitless
}


/* --------------------------- SOLAR DECLINATION ---------------------------- */
float Dusk2Dawn::sunDeclination(float t) {
  float e      = obliquityCorrection(t);
  float lambda = sunApparentLong(t);

  float sint  = sin(degToRad(e)) * sin(degToRad(lambda));
  float theta = radToDeg(asin(sint));
  return theta; // in degrees
}


float Dusk2Dawn::sunApparentLong(float t) {
  float o      = sunTrueLong(t);
  float omega  = 125.04 - 1934.136 * t;
  float lambda = o - 0.00569 - 0.00478 * sin(degToRad(omega));
  return lambda; // in degrees
}


float Dusk2Dawn::sunTrueLong(float t) {
  float l0 = geomMeanLongSun(t);
  float c  = sunEqOfCenter(t);
  float O  = l0 + c;
  return O; // in degrees
}


float Dusk2Dawn::sunEqOfCenter(float t) {
  float m     = geomMeanAnomalySun(t);
  float mrad  = degToRad(m);
  float sinm  = sin(mrad);
  float sin2m = sin(mrad * 2);
  float sin3m = sin(mrad * 3);
  float C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
  return C; // in degrees
}


/* ------------------------------- HOUR ANGLE ------------------------------- */
float Dusk2Dawn::hourAngleSunrise(float lat, float solarDec) {
  float latRad = degToRad(lat);
  float sdRad  = degToRad(solarDec);
  float HAarg  = (cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));
  float HA     = acos(HAarg);
  return HA; // in radians (for sunset, use -HA)
}


/* ---------------------------- SHARED FUNCTIONS ---------------------------- */
float Dusk2Dawn::obliquityCorrection(float t) {
  float e0    = meanObliquityOfEcliptic(t);
  float omega = 125.04 - 1934.136 * t;
  float e     = e0 + 0.00256 * cos(degToRad(omega));
  return e; // in degrees
}


float Dusk2Dawn::geomMeanLongSun(float t) {
  float L0 = 280.46646 + t * (36000.76983 + t * 0.0003032);
  while (L0 > 360) {
    L0 -= 360;
  }
  while (L0 < 0) {
    L0 += 360;
  }
  return L0; // in degrees
}


float Dusk2Dawn::geomMeanAnomalySun(float t) {
  float M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
  return M; // in degrees
}


/* --------------------------- UTILITY FUNCTIONS ---------------------------- */
/* Convert Gregorian calendar date to Julian Day.
 */
float Dusk2Dawn::jDay(int year, int month, int day) {
  if (month <= 2) {
    year  -= 1;
    month += 12;
  }

  int A = floor(year/100);
  int B = 2 - A + floor(A/4);
  return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) +
         day + B - 1524.5;
}


/* Return fraction of time elapsed this century, AD 2000–2100.
 *
 * NOTE: 2,451,545 was the Julian day starting at noon UTC on 1 January AD 2000.
 *       36,525 is a Julian century.
 */
float Dusk2Dawn::fractionOfCentury(float jd) {
  return (jd - 2451545) / 36525;
}


float Dusk2Dawn::radToDeg(float rad) {
  return 180 * rad / PI;
}


float Dusk2Dawn::degToRad(float deg) {
  return PI * deg / 180;
}
