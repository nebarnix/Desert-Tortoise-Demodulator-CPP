#include <iostream>
#include <ctype.h>
#include <unistd.h>
#include <cstdio>
#include <complex>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <conio.h>
//#include "wave.h"
#include "AGC.h"
#include "CarrierTrackingPLL.h"
#include "LowPassFilter.h"
#include "GardenerClockRecovery.h"
#include "ManchesterDecode.h"
#include "ByteSync.h"
#include "portaudio.h"

//#define SAMPLE_RATE         (48000)
#define SAMPLE_RATE         (50000)
#define PA_SAMPLE_TYPE      paFloat32
#define NUM_CHANNELS        (2)
#define DEFAULT_CHUNKSIZE  (1000)

//these are obsolete for POES
#define DSP_MAX_CARRIER_DEVIATION   (4500.0) //was (4500)
#define DSP_PLL_LOCK_THRESH         (0.025) //(0.10)
#define DSP_PLL_ACQ_GAIN            (0.025) //(0.005)  
#define DSP_PLL_TRCK_GAIN           (0.0013) //(0.0015)
#define DSP_SQLCH_THRESH            (0.01) //was (0.25)
#define DSP_MM_MAX_DEVIATION        (10.0) //was (3.0)
#define DSP_MM_GAIN                 (0.15)
#define DSP_GDNR_ERR_LIM            (0.1)
#define DSP_GDNR_GAIN               (2.5)
#define DSP_MCHSTR_RESYNC_LVL       (0.75) //was (0.5)
#define DSP_AGC_ATCK_RATE           (1e-2)//      //attack is when gain is INCREASING (weak signal)
#define DSP_AGC_DCY_RATE            (2e-2) //     //decay is when the gain is REDUCING (strong signal)
//#define DSP_AGC_GAIN               (.00025)
#define DSP_AGCC_GAIN               (0.00025)
#define DSP_LPF_FC                  (11000)//(was (11000)
#define DSP_LPF_INTERP              (3) //interpolation order
#define DSP_LPF_ORDER               (26*DSP_LPF_INTERP) //was (26)

#define TRUE 1
#define FALSE 0

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//#define RAW_OUTPUT_FILES

//int spaceCraftID, dayNum, 

unsigned int CheckSum(unsigned char *dataStreamReal, unsigned long nSamples)
   {
   unsigned int sum=0;
   unsigned long idx;
   for(idx = 0; idx < nSamples; idx++)
      {
      sum += (dataStreamReal[idx]);
      //printf("%.2X %ld,",dataStreamReal[idx]);
      }      
   //printf("\n");
   return sum;
   }

const char *get_filename_ext(const char *filename) 
   {
   const char *dot = strrchr(filename, '.');
   if(!dot || dot == filename)
      return "";
   return dot + 1;
   }
   
int main(int argc, char **argv) 
   {
   //PortAduio Variables
   PaStreamParameters inputParameters;
   PaStream *stream;
   PaError err;
   
   //Files we will use
   #ifdef RAW_OUTPUT_FILES
      FILE *rawOutFilePtr;
      FILE *rawOutFilePtr2;
   #endif
   
   FILE *minorFrameFile;
   
   unsigned long chunkSize = DEFAULT_CHUNKSIZE, i=0, idx, idx2, nSymbols, nBits, totalSymbols=0, totalBits=0, totalSamples=0;
   
   float *waveFrame; //data from input stream
   float realPart, imagPart;
   
   double *waveDataTime,*dataStreamLPF,*dataStreamReal, *dataStreamSymbols, *dataStreamLPFTime, *dataStreamSymbolsTime, *dataStreamBitsTime;
   double Fs = SAMPLE_RATE;   
   double averagePhase;
   double normFactor=0;
   double Time=0;
   struct tm tm; //time object
   time_t t;
   
   complex<double> *waveData;
   char qualityString[20];
   
   unsigned int CheckSum1=0, CheckSum2=0, CheckSum3=0;
   int nFrames=0, totalFrames=0,c;
   
   unsigned char *dataStreamBits;
   char outFileName[100];   
   
   char useRawFilesFlag=0;
   
   const char *build_date = __DATE__;
   printf("Project Desert Tortoise: Realtime NOAA POES Demodulator by Nebarnix.\nBuild date: %s\n",build_date); 
   
   //Define Objects used for DSP stuff
   StaticGain staticGainObj(1.0);
   NormalizingAGCC agcObj(DSP_AGCC_GAIN);
   NormalizingAGC2 agcObj2(DSP_AGC_ATCK_RATE, DSP_AGC_DCY_RATE);
   CarrierTrackPLL PLLObj(Fs, DSP_MAX_CARRIER_DEVIATION, DSP_PLL_LOCK_THRESH, DSP_PLL_ACQ_GAIN, DSP_PLL_TRCK_GAIN);
   LowPassFilter LPF(DSP_LPF_ORDER, DSP_LPF_FC, Fs, DSP_LPF_INTERP);
   GardenerClockRecovery clockSyncObj(Fs*DSP_LPF_INTERP, DSP_GDNR_ERR_LIM, DSP_GDNR_GAIN);
   ManchesterDecode manchesterDecodeObj(DSP_MCHSTR_RESYNC_LVL);
   ByteSyncOnSyncword byteSyncObj("1110110111100010000", 19);
   
   while ((c = getopt (argc, argv, "rn:c:")) != -1)
      {
      switch (c)
         {
         case 'r':
            useRawFilesFlag = 1;
            printf("Outputting debug data in RAW format\n");
            break;
         case 'n':
            if(optarg == NULL)
               {
               printf("Static gain unspecified");
               return 1;
               }
            normFactor = atof(optarg);
            printf("Static Gain Override %f\n",normFactor);
            break;
         case 'c':
            if(optarg == NULL)
               {
               printf("Chucksize unspecified");
               return 1;
               }
            chunkSize = atoi(optarg);
            printf("Using %ld chunkSize\n",chunkSize);
            break;
         case '?':
            if (optopt == 'c' || optopt == 'n')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
               fprintf (stderr,
               "Unknown option character `\\x%x'.\n",
               optopt);
               return 1;
         default:
            abort ();
         }
      }
      
   printf("Using %ld chunkSize\n",chunkSize);   
   
   //Allocate the memory we will need         
   waveData = new std::complex<double>[chunkSize];
   waveFrame = new float[chunkSize*2];
   waveDataTime   = new double[chunkSize];
   dataStreamReal = new double[chunkSize];
   dataStreamLPF = new double[chunkSize*DSP_LPF_INTERP];
   dataStreamLPFTime = new double[chunkSize*DSP_LPF_INTERP];   
   dataStreamSymbols = new double[chunkSize];
   dataStreamSymbolsTime = new double[chunkSize]; 
   dataStreamBits = new unsigned char[chunkSize];
   dataStreamBitsTime = new double[chunkSize];
   
   if (dataStreamBits == NULL ||         
      waveDataTime == NULL ||
      waveData  == NULL || 
      dataStreamReal == NULL || 
      dataStreamLPF == NULL ||
      dataStreamLPFTime == NULL ||
      dataStreamSymbols  == NULL)
      {
      printf("Error in memory allocation\n");
      exit(1);
      }     
      
   //Initiate audio stream
   err = Pa_Initialize();
   
   if( err != paNoError ) goto error;
   
   inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
   
   if (inputParameters.device == paNoDevice) 
      {
      fprintf(stderr,"Error: No default input device.\n");
      goto error;
      }
      
   inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
   inputParameters.channelCount = NUM_CHANNELS;
   inputParameters.sampleFormat = PA_SAMPLE_TYPE;
   inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
   inputParameters.hostApiSpecificStreamInfo = NULL;
   
   err = Pa_OpenStream(
            &stream,
            &inputParameters,
            NULL,
            SAMPLE_RATE,
            chunkSize,
            0, /* paClipOff, */  /* we won't output out of range samples so don't bother clipping them */
            NULL,
            NULL );
   
   if( err != paNoError ) goto error;
   
   err = Pa_StartStream( stream );
   if( err != paNoError ) goto error;  

   // open files
   printf("Opening Output files..\n");
   
   t = time(NULL);
   tm = *localtime(&t);
   //printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
   snprintf(outFileName, 100,"minorframes_%4d%02d%02d_%02d%02d%02d.txt",tm.tm_year + 1900,tm.tm_mon + 1,tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec);
   
   
   minorFrameFile = fopen(outFileName, "w");
   if (minorFrameFile == NULL)
      {
      printf("Error opening output files\n");
      exit(1);
      }
   byteSyncObj.SetFileHandle(minorFrameFile);
   
   #ifdef RAW_OUTPUT_FILES
   rawOutFilePtr = fopen("output.raw", "wb");
   if (rawOutFilePtr == NULL)
      {
      printf("Error opening output file\n");
      exit(1);
      }
      
   rawOutFilePtr2 = fopen("outputbits.raw", "wb");
   if (rawOutFilePtr2 == NULL)
      {
      printf("Error opening output file\n");
      exit(1);
      }
   #endif
   
   //LPF_Fc = DSP_LPF_FC;   
   //MakeLPFIR(filterCoeffs, LPF_Order, LPF_Fc, Fs);

   
   while(!kbhit())
      { 
      err = Pa_ReadStream(stream, waveFrame, chunkSize);
      if(err != paNoError)
         {
         printf("overflow :(\n");
         //goto error;
         }
         
      idx2 = 0;
      //convert from stereo IQ to complex stream
      for(idx = 0; idx < chunkSize*2; idx+=2)
         {
         realPart = waveFrame[idx];
         imagPart = waveFrame[idx+1];
         waveData[idx2] = std::complex<double>(realPart, imagPart);
         Time += (1/Fs);
         waveDataTime[idx2] = Time;
         idx2++;
         //if(idx % 100 == 0) printf("\r%6.3f\t\t\t%6.3f",realPart, imagPart);
         }
      
      if(i == 0 && normFactor == 0)
         {
         normFactor = staticGainObj.Work(waveData, chunkSize);
         //normFactor = 0.000041;
         printf("Normalization Factor: %f\n",normFactor);
         agcObj.SetGain(normFactor);
         }
         
      i+=chunkSize;
      
      //Normalizing AGC
      agcObj.Work(waveData, chunkSize);
      
      //CheckSum0 += CheckSum((unsigned char*)waveData, sizeof(*waveData) * nSamples);
      
      averagePhase = PLLObj.Work(waveData, dataStreamReal, chunkSize); //crashes when commented out?!      
      //CheckSum1 += CheckSum((unsigned char *)dataStreamReal, sizeof(*dataStreamReal) * nSamples);
      
      //Low pass filter
      LPF.Work(waveDataTime, dataStreamReal, dataStreamLPF, dataStreamLPFTime, chunkSize); //Commenting THIS out makes the program work OK
      
      //CheckSum2 += CheckSum((unsigned char *)dataStreamReal, sizeof(*dataStreamReal) * nSamples);
      
      //Normalizing AGC 2
      agcObj2.Work(dataStreamLPF, chunkSize*DSP_LPF_INTERP);
      
      //CheckSum3 += CheckSum((unsigned char *)dataStreamReal, sizeof(*dataStreamReal) * nSamples);
      
      //#ifdef RAW_OUTPUT_FILES
      //   fwrite(dataStreamLPF, sizeof(double), chunkSize*DSP_LPF_INTERP,rawOutFilePtr);
      //#endif
      
      nSymbols = clockSyncObj.Work(dataStreamLPF, dataStreamLPFTime, dataStreamSymbols, dataStreamSymbolsTime, chunkSize*DSP_LPF_INTERP);
      
      
      #ifdef RAW_OUTPUT_FILES
         fwrite(dataStreamSymbols, sizeof(double), nSymbols,rawOutFilePtr);
      #endif
      
            //ManchesterDecode(double *dataStreamIn, double *dataStreamInTime, unsigned long nSymbols, unsigned char *bitStream, double resyncThreshold)
      nBits = manchesterDecodeObj.Work(dataStreamSymbols, dataStreamSymbolsTime, dataStreamBits, dataStreamBitsTime, nSymbols);
      
      
      #ifdef RAW_OUTPUT_FILES
         fwrite(dataStreamBits, sizeof(char), nBits,rawOutFilePtr2);
      #endif
      
      nFrames = byteSyncObj.Work(dataStreamBits, dataStreamBitsTime, nBits);
      
      totalBits += nBits;
      totalFrames += nFrames;
      totalSymbols += nSymbols;
      totalSamples += chunkSize;
      
      //Print updates     
      printf("\r");
      
      averagePhase = 10.0 * log10( pow(1.5708 - averagePhase,2));
      
      if(averagePhase > -4.3)
         snprintf(qualityString, 20,"%s%02.1fQ%s", ANSI_COLOR_GREEN,averagePhase,ANSI_COLOR_RESET);
      else if(averagePhase > -5)
         snprintf(qualityString, 20,"%s%02.1fQ%s", ANSI_COLOR_YELLOW,averagePhase,ANSI_COLOR_RESET);
      else if(averagePhase > -6)
         snprintf(qualityString, 20,"%s%02.1fQ%s", ANSI_COLOR_YELLOW,averagePhase,ANSI_COLOR_RESET);
      else
         snprintf(qualityString, 20,"%s%02.1fQ%s", ANSI_COLOR_RED,averagePhase,ANSI_COLOR_RESET);
      
      //SUPRESS OUTPUT HERE
      printf("%0.3f Ks : %0.1f Sec: %ld Sym : %ld Bits : %d Frames : %s   ",(totalSamples)/1000.0, waveDataTime[0], totalSymbols, totalBits, totalFrames, qualityString);
      
      
      //fwrite("2", sizeof(unsigned char), 1, rawOutFilePtr);
      /*for(idx=0; idx < nSamples; idx++)
         {
         fVal = (crealf(waveData[i]));
         fwrite(&fVal,sizeof(fVal),1,rawOutFilePtr);
         fVal = (cimagf(waveData[i]));
         fwrite(&fVal,sizeof(fVal),1,rawOutFilePtr);
         }*/
      }
   
   getch(); //clear the buffer
   //printf("\nChecksum1=%X Checksum2=%X Checksum3=%X", CheckSum1,CheckSum2,CheckSum3);
   printf("\nAll done! Closing files and exiting.\nENJOY YOUR BITS AND HAVE A NICE DAY\n");
   
   err = Pa_CloseStream( stream );
   if( err != paNoError ) goto error;
   
   Pa_Terminate();
   delete[] waveFrame;
   delete[] waveData;
   //return 0;
 
    
   #ifdef RAW_OUTPUT_FILES
      fclose(rawOutFilePtr);
      fclose(rawOutFilePtr2);
   #endif
   
   fclose(minorFrameFile);
   
   
   // cleanup before quitting
   delete[] dataStreamReal;
   delete[] dataStreamSymbols;
   delete[] dataStreamSymbolsTime;
   delete[] dataStreamReal;
   delete[] dataStreamLPF;
   delete[] waveData;
   delete[] dataStreamBits;
   delete[] dataStreamBitsTime;
   delete[] waveDataTime;
   
   //quit
   //fflush(stdout);
   return 0;
   
   error:
      #ifdef RAW_OUTPUT_FILES
         fclose(rawOutFilePtr);
         fclose(rawOutFilePtr2);
      #endif
      
      fclose(minorFrameFile);
      
      // cleanup before quitting
      
      delete[] dataStreamReal;
      delete[] dataStreamSymbols;
      delete[] dataStreamReal;
      delete[] dataStreamLPF;
      delete[] waveData;
      delete[] dataStreamBits;
      delete[] dataStreamBitsTime;
      delete[] waveDataTime;
      
      Pa_Terminate();
      fprintf( stderr, "An error occured while using the portaudio stream\n" );
      fprintf( stderr, "Error number: %d\n", err );
      fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
      delete[] waveFrame;
      delete[] waveData;
      return -1;
   } 
   