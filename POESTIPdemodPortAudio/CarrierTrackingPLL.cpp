#include "CarrierTrackingPLL.h"

CarrierTrackPLL::CarrierTrackPLL(double FsNew, double freqRangeNew, double d_lock_thresholdNew, double loopbw_acqNew, double loopbw_trackNew)
   {
   d_lock_threshold = d_lock_thresholdNew;
   Fs = FsNew;
   firstLock = -1;
   loopbw_acq = loopbw_acqNew;
   loopbw_track = loopbw_trackNew;
   freqRange = freqRangeNew;
   bw = loopbw_acq;
   damp=0.999;
  
   d_locksig = 0;
   lockSigAlpha = 0.00005;
   
   averagePhaseAlpha=0.00005;
   
   d_alpha = (4 * damp * bw) / (1 + 2 * damp * bw + bw * bw);
   d_beta = (4 * bw * bw) / (1 + 2 * damp * bw + bw * bw);
   
   d_phase = 0.1; //something not zero for benchmarking the function (sin(0) is probably a shortcut)
   d_freq  = 2.0*M_PI*0 / Fs;
   d_max_freq   = 2.0*M_PI*freqRange/Fs; //+/-4500 for 2m polar sats
   d_min_freq   = -2.0*M_PI*freqRange/Fs;
   firstLock = 0;
   averagePhase=1.5708;
   }

double CarrierTrackPLL::Work(std::complex<double> *complexDataIn, double *realDataOut, unsigned int nSamples)
   {
   //using namespace std::complex_literals;
   double sample_phase;  
   
   double t_imag;
   double t_real;
   double PLLOutSamplePhase,error;
   
   unsigned int idx;
   
   std::complex<double> PLLOutSample;
           
   for (idx=0; idx<nSamples; idx++)
      {
       
       t_imag = sin(d_phase);
       t_real = cos(d_phase);    
              
       //shift the frequency by the carrier 
       //PLLOutSample = complexDataIn[idx] * (t_real+(-t_imag*1i));
       PLLOutSample = complexDataIn[idx] * std::complex<double>(t_real,-t_imag);
       
       //data bits are in the imaginary part
       realDataOut[idx] = imag(PLLOutSample);
       
       //calculate phase angle for quality estimation 
       PLLOutSamplePhase = atan2(imag(PLLOutSample),real(PLLOutSample));
       
       //running average of the absolute value of the phase
       averagePhase = averagePhase * (1.0 - averagePhaseAlpha) + averagePhaseAlpha * std::abs(PLLOutSamplePhase);
       
       //Calculate Error
       sample_phase = atan2(imag(complexDataIn[idx]),real(complexDataIn[idx]));
       
       //mod2pi the error
       if((sample_phase-d_phase) > M_PI)
           error = (sample_phase-d_phase)-2*M_PI;
       else if((sample_phase-d_phase) < -M_PI)
           error = (sample_phase-d_phase)+2*M_PI;
       else
           error= sample_phase-d_phase;
       
       //advance the loop
       d_freq = d_freq + d_beta * error;
       d_phase = d_phase + d_freq + d_alpha * error;
       
       //wrap the phase
       while(d_phase > 2*M_PI)
           d_phase = d_phase-2*M_PI;
       
       while(d_phase < -2*M_PI)
           d_phase = d_phase+2*M_PI;
       
       //Limit the frequency
       if(d_freq > d_max_freq)
           d_freq = d_max_freq;
       else if(d_freq < d_min_freq)
           d_freq = d_min_freq;
       
      d_locksig = d_locksig * (1.0 - lockSigAlpha) + lockSigAlpha*(real(complexDataIn[idx]) * t_real + imag(complexDataIn[idx]) * t_imag);
       
      
      if(d_locksig > d_lock_threshold && firstLock == 0) //This needs to be a moving average or at least somewhat smoothed
         {
         printf(" : PLL locked at %0.2fHz\n", d_freq*Fs/(2.0*M_PI));
         //fprintf(['PLL locked at ' num2str(d_freq*Fs/(2*pi)) '\n']);
         firstLock = idx;
         //d_alpha = 0.001; //decrease to tracking gain
         bw = loopbw_track;        
         d_alpha = (4 * damp * bw) / (1 + 2 * damp * bw + bw * bw);
         d_beta = (4 * bw * bw) / (1 + 2 * damp * bw + bw * bw);
         //d_alpha = alpha_track;
         //d_beta = d_alpha/25;
         }
      }
   
   return averagePhase;
   }
