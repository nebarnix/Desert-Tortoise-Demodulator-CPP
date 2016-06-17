//Main loop taken from http://www.barrgroup.com/Embedded-Systems/How-To/Digital-Filters-FIR-IIR
//coeffs computed in matlab. Only valid for 50ksps data!

#include "LowPassFilter.h"


//LPF Test Code
/*
main()
{
int interpolation_factor = 3;
double DataIn[50], DataOut[50*interpolation_factor];
LowPassFilter LPF(26, 11e3, 50e3, interpolation_factor);
while(1)
   LPF.Work(DataIn, DataIn, DataOut, DataOut, 50);

}*/


LowPassFilter::~LowPassFilter()
   {
   delete[] filterCoeffs;
   delete[] x;
   }

LowPassFilter::LowPassFilter(int NIn, double FcIn, double FsIn, int interpFactorIn)
   {
   interpFactor = interpFactorIn;
   N = NIn;
   oldest = 0;
   x = new double [N];          //Allocate memory for circular buffer
   memset(x,0,sizeof(double)*N); //zero out the circular buffer    
   filterCoeffs = new double[N];   
   MakeLPFIR(filterCoeffs, N, FcIn, FsIn);
   }

void LowPassFilter::Work(double *dataStreamInTime, double *dataStreamIn, double *dataStreamOut, double *dataStreamOutTime, unsigned long nSamples)
   {
   /*
   * Insert the newest sample into an N-sample circular buffer.
   * The oldest sample in the circular buffer is overwritten.
   */

   
   
   //static double h[] = {0,-0.000109655250327,0.000316665647855,0.001676613642539,-0.000476921998763,-0.007259094943430,-0.003189409960007, 0.019323908569853,0.020108432578903,-0.036803451537779,-0.072012095309267,0.053204443013450,0.305238645914910,0.439963839264128,0.305238645914910,0.053204443013450,-0.072012095309267,-0.036803451537779,0.020108432578903,0.019323908569853,-0.003189409960007, -0.007259094943430,-0.000476921998763,0.001676613642539,0.000316665647855,-0.000109655250327, 0};
   double y;
   unsigned long idxI;    //idxI is used to track the output buffers
   unsigned int idxF;    //idxF is used to track the filtercoeffs
   unsigned long idxO=0; //idxO is used to track the input buffers
   
   for(idxI = 0; idxI < (nSamples*interpFactor); idxI++)
      {
      //zero stuffing interpolation   
      if((idxI % interpFactor) == 0)
         {
         x[oldest] = dataStreamIn[idxO]; //stuff the current sample into the buffer and push the oldest one out
         idxO++;
         if(idxO > nSamples)
            break;
         }
      else
         x[oldest] = 0;
      
      
      // Multiply the last N inputs by the appropriate coefficients.
      // Their sum is the current output.
      
      y = 0;
      for (idxF = 0; idxF < N; idxF++) 
         { 
         y += filterCoeffs[idxF] * x[(oldest + idxF) % N];         
         }  
      
      dataStreamOut[idxI] = y; 
      dataStreamOutTime[idxI] = dataStreamInTime[idxO];  
      oldest = (oldest + 1) % N;   
      }
   }

/*
void LowPassFilter(double *dataStream, unsigned long nSamples, double *filterCoeffs, int N)
   {
   
   // Insert the newest sample into an N-sample circular buffer.
   // The oldest sample in the circular buffer is overwritten.
   
   unsigned int idxF;
   static char firsttime = 1;
   //static double h[] = {0,-0.000109655250327,0.000316665647855,0.001676613642539,-0.000476921998763,-0.007259094943430,-0.003189409960007, 0.019323908569853,0.020108432578903,-0.036803451537779,-0.072012095309267,0.053204443013450,0.305238645914910,0.439963839264128,0.305238645914910,0.053204443013450,-0.072012095309267,-0.036803451537779,0.020108432578903,0.019323908569853,-0.003189409960007, -0.007259094943430,-0.000476921998763,0.001676613642539,0.000316665647855,-0.000109655250327, 0};
   static double *x; //circular buffer
   static int oldest = 0;
   double y;
   long idxI;
   //long idxF = -N; //BUG! -N delay should only be for FIRST GO AROUND!
   if(firsttime == 1)
      {   
      firsttime = 0;
      x = (double *) malloc(sizeof(double) * N);
      if (x == NULL)
            {
            printf("Error in malloc\n");
            exit(1);
            }
      memset(x,0,sizeof(double)*N); //zero out the circular buffer
      }
   // printf("LPF\n");
   
   for(idxI = 0; idxI < nSamples; idxI++)
      {
      x[oldest] = dataStream[idxI]; 
      
      
       // Multiply the last N inputs by the appropriate coefficients.
       // Their sum is the current output.
       
      y = 0;
      for (idxF = 0; idxF < N; idxF++) 
         { 
         y += filterCoeffs[idxF] * x[(oldest + idxF) % N];         
         }      
      
      //visualize buffers
      //printf("circ: ");
      //for(idxF = 0; idxF < N; idxF++)
      //   {         
      //   printf("%0.2f,", x[(oldest + 1 + idxF) % N]);
      //   }
      //printf("\n");
      
       //spit out what is in the buffer
      //printf("raw:  ");
      //for(idxF = 0; idxF < N; idxF++)
      //   {         
      //   printf("%0.2f,", x[idxF]);
      //   }
      //printf("\n");
      
      
      //Output the result.
       
      //if(idxF >= 0) //delay output so that the buffer can fill 
      //   dataStream[idxF] = y;
      //idxF++;
      dataStream[idxI] = y; //you have to live with the delay, otherwise, how to handle multiple calls?
      //if(idxI > 30)
      //   exit(1);
      oldest = (oldest + 1) % N;
      }
   }*/
   
int LowPassFilter::MakeLPFIR(double *h, int N, double Fc, double Fs)
   {
   double *hd;
   int n;
   double T = 1.0 / Fs;
   double wc = 2.0*M_PI*Fc*T;
   double tou = (N-1.0)/2.0;
   double wn;
   
   hd = new double[N];
         
   // compute IDFT
   for(n=0; n < N; n++)
      {
      hd[n]=(sin(wc*(n-tou))) / (M_PI*(n-tou));
       
      //check for center 
      if ((n == tou) && ((int)((N / 2)*2) != N))
         {
         hd[n] = wc / M_PI;
         //printf("%d, %d\n",(int)((N / 2)*2), N);
         }
      //printf("hd[%d]=%f\n",n,hd[n]);
      }
   
   // apply window
   for(n=0; n < N; n++)
      {
      //wn = 1;
      wn = 0.42 - 0.5 * cos((2 * M_PI * n)/(N-1)) + 0.08 * cos((4 * M_PI * n)/(N-1));
      //wn = 0.54-0.46*cos((2*M_PI*n)/(N-1));
      //wn = (1 - cos((2*M_PI*n) / (N-1)));
      h[n] = hd[n] * wn;
      }
   
   /*for(n=0;n < N;n++)
      {
      printf("h[%d]=%f\n",n,h[n]);
      }*/
   delete[] hd;
   return n;
   }