#ifndef AGC_H
#define AGC_H

#include <iostream>
#include <complex>
#include <cmath>
using namespace std;

class FindSignalAmplitude
   {
   private:
      double average;
      double alpha;
   public:
      FindSignalAmplitude(double alphaNew);
      //~FindSignalAmplitude();
      double Work(double *dataStreamIn, unsigned long nSamples);
   };

class Squelch
   {
   private:
      int lastSquelch;
      double squelchThreshold;
   public:
      Squelch(double squelchThresholdNew);
      //~Squelch();
      void Work(double *dataStream, double *squelchStreamIn, unsigned long nSamples);
   };

class StaticGain
   {
   private:
      double desiredLevel;
   public:
      StaticGain(double desiredLevelNew);
      double Work(std::complex<double> *complexData, unsigned int nSamples);
   };

class NormalizingAGC
   {
   private:
      double gain; //Initial Gain Value      
      double desired;
      double AGC_loop_gain;
   public:
      NormalizingAGC(double AGC_loop_gain);
      void Work(double *dataStreamIn, unsigned long nSamples);            
   };
   
class NormalizingAGC2
   {
   private:
      double gain; //Initial Gain Value      
      double attack_rate;
      double decay_rate;
   public:
      NormalizingAGC2(double attack_rateNew, double decay_rateNew);
      double Work(double *dataStreamIn, unsigned long nSamples);            
   };
   
class NormalizingAGCC
   {
   private:
      double gain; //Initial Gain Value      
      double desired;
      double AGC_loop_gain;
   public:
      NormalizingAGCC(double AGC_loop_gainNew);
      void SetGain(double gainNew);      
      void Work(std::complex<double> *dataStreamIn, unsigned long nSamples);            
   };

#endif