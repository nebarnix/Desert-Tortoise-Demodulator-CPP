#ifndef MANCHESTERDECODE_H
#define MANCHESTERDECODE_H

#include <cmath>
#include <iostream>

class ManchesterDecode
   {
   private:
      double resyncThreshold;
      unsigned int clockmod;
      double currentSample;
      double prevSample;
      double prevPrevSample;
      unsigned char evenOddCounter;   
      int sign(double x);    
   public:
      ManchesterDecode(double resyncThresholdIn=0.5);
      void SetResyncTheshold(double resyncThresholdIn);
      unsigned long Work(double *dataStreamIn, double *dataStreamInTime, unsigned char *bitStream, double *dataStreamOutTime, unsigned long nSymbols);
   };

#endif