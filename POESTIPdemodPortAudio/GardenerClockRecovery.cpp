#include "GardenerClockRecovery.h"
GardenerClockRecovery::GardenerClockRecovery(double Fs, double errorLimitNew, double kpNew)
   {
   baud = 8320*2+0.3;
   stepSize = Fs / baud;
   nextSample = 0;
   prevBit = 0;
   halfSample =0;
   kp = kpNew;
   errorLimit = errorLimitNew;
   }

//Gardener Clock Recovery Loop
unsigned long GardenerClockRecovery::Work(double *dataStreamIn, double *dataStreamInTime, double *dataStreamOut, double *dataStreamOutTime, unsigned long numSamples)
   {      
   double currentBit;
   double Error;
   unsigned long count = 0;   
      
   while(rint(nextSample) < numSamples)
      {    
      //Stores Bit 
      currentBit  = dataStreamIn[(unsigned int)(rint(nextSample))];
      halfSample  = dataStreamIn[(unsigned int)(rint(halfSample))];
      //dataStreamOutTime(count) = dataStreamInTime(rint(nextSample));
      dataStreamOut[count] = currentBit;
      dataStreamOutTime[count] = dataStreamInTime[(unsigned int)(rint(nextSample))];
      //Ind(count)  = nextSample;
      
      
      //Calculates Error
      //Error = sign(prevBit)*currentBit - sign(currentBit)*prevBit;
      Error = kp * (currentBit - prevBit) * (halfSample);
      
      //Updates Step Size
      //stepSize = stepSize + kp*Error;
      
      //Limits Step size3
      if( Error > errorLimit)
         Error = errorLimit;
      else if( Error < -errorLimit )
         Error = -errorLimit;
           
      
      //Updates nextSample
      //nextSample = nextSample - Error;
      nextSample = (nextSample - Error);
      
      halfSample = nextSample + stepSize/2.0;
      nextSample = nextSample + stepSize;    
      prevBit = currentBit;
      count = count + 1;
      }
   nextSample =  nextSample - numSamples; //roll over to next chunk
   //return count-1; //does this make things better?
   return count;
   }