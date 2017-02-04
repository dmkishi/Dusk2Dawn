/*  Dusk2Dawn.h
 *  Get estimate time of sunrise and sunset given a set of coordinates.
 *  Created by DM Kishi <dm.kishi@gmail.com> on 2017-02-01.
 *  <https://github.com/dmkishi/Dusk2Dawn>
 */

#ifndef Dusk2Dawn_h
#define Dusk2Dawn_h

  #include "Arduino.h"
  #include <Math.h>

  class Dusk2Dawn {
    public:
      Dusk2Dawn(float, float, float);
      int sunrise(int, int, int, bool);
      int sunset(int, int, int, bool);
    private:
      float _latitude, _longitude;
      int _timezone;
      int sunriseSet(bool isRise, int y, int m, int d, bool isDST);
      float sunriseSetUTC(bool isRise, float jday, float latitude, float longitude);
      float equationOfTime(float t);
      float meanObliquityOfEcliptic(float t);
      float eccentricityEarthOrbit(float t);
      float sunDeclination(float t);
      float sunApparentLong(float t);
      float sunTrueLong(float t);
      float sunEqOfCenter(float t);
      float hourAngleSunrise(float lat, float solarDec);
      float obliquityCorrection(float t);
      float geomMeanLongSun(float t);
      float geomMeanAnomalySun(float t);
      float jDay(int year, int month, int day);
      float fractionOfCentury(float jd);
      float radToDeg(float rad);
      float degToRad(float deg);
  };

#endif
