# Dusk2Dawn

Minimal Arduino library for sunrise and sunset time.

Given a set of a coordinates, a preferred time zone, and whether Daylight Saving Time is in effect, an estimate time of [*apparent* sunrise or sunset](https://www.esrl.noaa.gov/gmd/grad/solcalc/glossary.html#apparentsunrise) is returned in **minutes elapsed since midnight**.

## Installation
1. Download the [ZIP file](https://github.com/dmkishi/Dusk2Dawn/archive/master.zip).
2. Launch the Arduino IDE, navigate to *Sketch → Include Library → Add .ZIP Library...*. From the prompt, select the ZIP just downloaded.

More info here: https://www.arduino.cc/en/Guide/Libraries#toc4

## Usage
```C++
  // Multiple instances can be created. Arguments are longitude, latitude, and
  // time zone offset in hours from UTC.
  Dusk2Dawn losAngeles(34.0522, -118.2437, -8);
  Dusk2Dawn tokyo(35.6895, 139.6917, 9);
  Dusk2Dawn antarctica(-77.85, 166.6667, 12);

  // Available methods are sunrise() and sunset(). Arguments are year, month,
  // day, and if Daylight Saving Time is in effect.
  int la1 = losAngeles.sunrise(2017, 12, 31, false);
  int la2 = losAngeles.sunset(2017, 12, 1, false);
  int ant = antarctica.sunrise(2017, 11, 30, false);

  // Time is returned in minutes elapsed since midnight. If no sunrises or
  // sunsets are expected, a "-1" is returned.
  Serial.println(la1); // 418
  Serial.println(la2); // 1004
  Serial.println(ant); // -1
```

## History
- **2017-2-3**: Released.

## Credits
This is a paired down port of the [NOAA Solar Calculator](https://www.esrl.noaa.gov/gmd/grad/solcalc/) (as of February 1st, 2017.) The algorithm and Javascript source was found at <https://www.esrl.noaa.gov/gmd/grad/solcalc/main.js>.
