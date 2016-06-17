#ifndef GARDENERCLOCKRECOVERY_H
#define GARDENERCLOCKRECOVERY_H

#include <iostream>
#include <cmath>
using namespace std;

class GardenerClockRecovery
   {
   private:
      double baud;
      double stepSize;
      double nextSample;
      double prevBit;
      double halfSample;
      double kp;
      double errorLimit;
   public:
      GardenerClockRecovery(double Fs, double errorLimitNew, double kpNew);
      unsigned long Work(double *dataStreamIn, double *dataStreamInTime, double *dataStreamOut, double *dataStreamOutTime, unsigned long numSamples);
   };
#endif