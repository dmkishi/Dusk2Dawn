/*  Dusk2Dawn.h
 *  Get estimate time of sunrise and sunset given a set of coordinates.
 *  Created by DM Kishi <dm.kishi@gmail.com> on 2017-02-01
 *  <https://github.com/dmkishi/Dusk2Dawn>
 */

#ifndef Dusk2Dawn_h
#define Dusk2Dawn_h

  #include "Arduino.h"
  #include <Math.h>

  class Dusk2Dawn {
    public:
      Dusk2Dawn(float latitude, float longitude, int timezone);
      int sunriseMinute(int y, int m, int d, bool isDST);
      int sunsetMinute(int y, int m, int d, bool isDST);
    private:
      float _latitude, _longitude;
      int _timezone;
  };

#endif
