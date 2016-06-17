#ifndef LOWPASSFILTER_H
#define LOWPASSFILTER_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <cmath>

class LowPassFilter
   {
   private:
      int N;
      int interpFactor;
      int oldest;
      double *x; //circular buffer
      double *filterCoeffs;
   public:
      //void LowPassFilter(double *dataStream, unsigned long nSamples, double *filterCoeffs, int N);
      LowPassFilter(int NIn, double FcIn, double FsIn, int interpFactorIn=1);
      ~LowPassFilter();
      void Work(double *dataStreamInTime, double *dataStreamIn, double *dataStreamOut, double *dataStreamOutTime, unsigned long nSamples);
      int MakeLPFIR(double *h, int N, double Fc, double Fs);
   };

#endif