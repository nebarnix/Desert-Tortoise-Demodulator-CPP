#include "AGC.h"


//////////////////////////////////////////////////
FindSignalAmplitude::FindSignalAmplitude(double alphaNew)
   {
   average = 0;
   alpha = alphaNew;
   }

double FindSignalAmplitude::Work(double *dataStreamIn, unsigned long nSamples)
   {
   unsigned long i;
   
   for(i=0; i < nSamples; i++)
      {    
       average = average * (1.0 - alpha) + alpha*fabs(dataStreamIn[i]);    
      }
   return average;
   }

//////////////////////////////////////////////////
Squelch::Squelch(double squelchThresholdNew)
   {
   lastSquelch=0;
   squelchThreshold = squelchThresholdNew;
   }

void Squelch::Work(double *dataStream, double *squelchStreamIn, unsigned long nSamples)
   {
   unsigned long i;
   int squelch;
   for(i=0; i < nSamples; i++)
      {   
      if(squelchStreamIn[i] < squelchThreshold)
         {
         squelch = 1;
         dataStream[i] = 0;
         }
      else
         squelch = 0;
      lastSquelch = squelch;
      }
   }

//////////////////////////////////////////////////
StaticGain::StaticGain(double desiredLevelNew)
   {
   desiredLevel=desiredLevelNew;
   }
   
double StaticGain::Work(std::complex<double> *complexData,unsigned int nSamples)
   {
   unsigned long i;
   double avgLevel=0;
   avgLevel = std::abs(complexData[0]);
   for(i=0; i < nSamples; i++)
      {
      avgLevel += std::abs(complexData[i]);
      avgLevel /= 2.0;
      }      
   return desiredLevel / avgLevel  ;
   }

//////////////////////////////////////////////////
//Automatic Gain Control Block (GNUradio based )
NormalizingAGC2::NormalizingAGC2(double attack_rateNew, double decay_rateNew)
   {
   attack_rate = attack_rateNew;
   decay_rate = decay_rateNew;
   gain = 1;
   }
   
double NormalizingAGC2::Work(double *dataStreamIn, unsigned long nSamples)
   {
   
   unsigned long idx;
   double rate;
   //double attack_rate = 1e-1; 
   //double decay_rate = 1e-1;
   double reference = 1.0;  
   double max_gain = 50;   
   double error;
   
   for(idx=0; idx< nSamples; idx++)
      {     
   
      //output = input * _gain;
      dataStreamIn[idx] *= gain;
      
      error = (fabs(dataStreamIn[idx])) - reference;
      rate = decay_rate;
      
      if(fabs(error) > gain) 
         {
         rate = attack_rate;
         }
         
      gain -= error*rate;
      
      // Not sure about this
      if(gain < 0.0)
         gain = 10e-5;
      
      if(max_gain > 0.0 && gain > max_gain)
         {
         gain = max_gain;
         }     
      }
   }
   
//////////////////////////////////////////////////
//Automatic Gain Control Block (nebarnix original)
NormalizingAGC::NormalizingAGC(double AGC_loop_gainNew)
   {
   AGC_loop_gain = AGC_loop_gainNew;
   gain = 1; //Initial Gain Value
   desired = 10;//6.6366;//0.6366; //because a sin wave of amplitude 1 has this average absolute value
   }

void NormalizingAGC::Work(double *dataStreamIn, unsigned long nSamples)
   {
   //Todo: implement a 'relock' mode for LARGE error values (either adjust gain
   //outright or adjust loop gain
   
   unsigned long idx;
   
   double error;
   
   for(idx=0; idx< nSamples; idx++)
      {
      dataStreamIn[idx] *= gain;
      error = desired - (gain * fabs(dataStreamIn[idx]));
      
      //if(error > 1.0) //do something for really large errors
      //    error = -1/error;
      //end
      
      gain = gain + AGC_loop_gain * error ;
      //gaini(idx) = gain;
      }
   }
   
//////////////////////////////////////////////////
//Automatic Gain Control Block
NormalizingAGCC::NormalizingAGCC(double AGC_loop_gainNew)
   {
   AGC_loop_gain = AGC_loop_gainNew;
   gain = 0;
   }

void NormalizingAGCC::SetGain(double gainNew)
   {
   gain = gainNew;
   }
   
void NormalizingAGCC::Work(std::complex<double> *dataStreamIn, unsigned long nSamples)
   {
   //Todo: implement a 'relock' mode for LARGE error values (either adjust gain
   //outright or adjust loop gain
   
   unsigned long idx;

   double desired = 5; //because a sin wave of amplitude 1 has this average absolute value
   double error;
   
   
   for(idx=0; idx< nSamples; idx++)
      {
      dataStreamIn[idx] *= gain;
      error = desired - (gain * std::abs(dataStreamIn[idx]));
      
      //if(error > 1.0) //do something for really large errors
      //    error = -1/error;
      //end
      
      gain = gain + AGC_loop_gain * error ;
      //gaini(idx) = gain;
      }
   }