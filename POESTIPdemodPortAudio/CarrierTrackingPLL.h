#ifndef CARRIERTRACKPLL_H
#define CARRIERTRACKPLL_H

#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <stdio.h>

class CarrierTrackPLL
   {
   private:
      double Fs;
      double firstLock;
      double bw;  
      double loopbw_acq;
      double loopbw_track;
      double freqRange;
      double damp;
      
      double d_alpha;
      double d_beta;
      
      double d_phase;
      double d_freq;
      double d_max_freq;
      double d_min_freq;
      
      double averagePhase; 
      double d_locksig;
      double d_lock_threshold;
      double lockSigAlpha;
      double averagePhaseAlpha;
      
   public:
      CarrierTrackPLL(double FsNew, double freqRangeNew, double d_lock_thresholdNew, double loopbw_acqNew, double loopbw_trackNew);
      double Work(std::complex<double> *complexDataIn, double *realDataOut, unsigned int nSamples);
   };


#endif