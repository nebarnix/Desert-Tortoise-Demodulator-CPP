#include "ByteSync.h"

void ByteSyncOnSyncword::SetFileHandle(FILE *minorFrameFileIn)
{
minorFrameFile = minorFrameFileIn;
}

ByteSyncOnSyncword::ByteSyncOnSyncword(char const *syncWordIn, unsigned int syncWordLengthIn)
//ByteSyncOnSyncword::ByteSyncOnSyncword(char *syncWordIn, unsigned int syncWordLengthIn, ofstream& minorFrameFileIn)
   {
   oldest = 0;
   zero = 0;
   one = 1;
   byte = 0;
   syncWordLength = syncWordLengthIn;
   syncWord = syncWordIn;
   
   //printf("Asking for %d Bytes",sizeof(char) * syncWordLength);
   historyBufferCirc = new char[syncWordLength];//(char *) malloc(sizeof(char) * syncWordLength);
   memset(historyBufferCirc, 48, sizeof(char) * syncWordLength);  
   }
   
ByteSyncOnSyncword::~ByteSyncOnSyncword()
   {
   delete[] historyBufferCirc;
   }
   
int ByteSyncOnSyncword::Work(unsigned char *bitStreamIn, double *bitStreamInTime, unsigned long nSamples)
   {
   int idx2;
   int framesFound=0;
   long idx;        
         
   //loop through samples
   for(idx = 0; idx < nSamples; idx++)
      {      
      //Enter this loop if we found a syncword last time around            
      if(minorFrameShiftFlag == 1)
         {
         if(bitStreamIn[idx]=='0')     
            {
            byte = byte << 1; //This is a zero, just shift
            byte = byte | zero;
            }
         else
            {
            byte = byte << 1; //This is a one, set the bit then shift               
            byte = byte | one;
            }
         
         bitIdx++;   
         if(bitIdx > 7)
            {
            fprintf(minorFrameFile,"%.2X ",byte);
            //ofstream << std::hex << std::uppercase << byte << std::nouppercase << std::dec << std::endl;
            byte = 0;
            bitIdx = 0;
            frameByteIdx++;
            if(frameByteIdx > 103)
               {
               minorFrameShiftFlag = 0;
               fprintf(minorFrameFile,"\n");
               }
            }
         }         
         
      //overwrite oldest bit in cir buffer with newest bit   
      historyBufferCirc[oldest] = bitStreamIn[idx];       
      syncIndicator = 1;
            
      //Look for syncword
      for (idx2 = 0; idx2 < syncWordLength; idx2++) 
         {
         //compare syncword bytes to appropriate circular buffer bytes         
         if (syncWord[idx2] != historyBufferCirc[(oldest + idx2 + 1) % syncWordLength])
            {
            syncIndicator = 0;
            break;
            }
         }
         
      if(syncIndicator == 1 && minorFrameShiftFlag == 0)
         {
         fprintf(minorFrameFile,"%.5f ",bitStreamInTime[idx]);
         fprintf(minorFrameFile,"%.2X ",0b11101101);
         fprintf(minorFrameFile,"%.2X ",0b11100010);
         frameByteIdx = 2;
         minorFrameShiftFlag = 1;
         framesFound++;
         bitIdx=3;
         byte=0;
         zero = 0;
         one = 1;
         }       
      
         
      //Look for Inverse Syncword
      syncIndicator = 1;
      for (idx2 = 0; idx2 < syncWordLength; idx2++) 
         {
         if (syncWord[idx2] == historyBufferCirc[(oldest + idx2 + 1) % syncWordLength])
            {
            syncIndicator = 0;
            //printf("NO\n\n");
            break;
            }         
         }
         
      if(syncIndicator == 1 && minorFrameShiftFlag == 0)
         {
         fprintf(minorFrameFile,"%.5fi ",bitStreamInTime[idx]);
         fprintf(minorFrameFile,"%.2X ",0b11101101);
         fprintf(minorFrameFile,"%.2X ",0b11100010);
         frameByteIdx = 2;
         minorFrameShiftFlag = 1;
         framesFound++;
         bitIdx=3;
         byte=0;
         zero = 1;
         one = 0;
         }     
    
      oldest = (oldest + 1) % syncWordLength;
      }
   return framesFound;   
   }