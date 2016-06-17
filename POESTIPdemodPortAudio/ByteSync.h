#ifndef BYTESYNC_H
#define BYTESYNC_H

#include <string.h>
#include <stdio.h>
#include <iostream>

class ByteSyncOnSyncword
   {
   private:
      char *historyBufferCirc; //circular buffer
      int oldest, syncIndicator, frameByteIdx, minorFrameShiftFlag, bitIdx;
      unsigned char zero, one;
      unsigned char byte;
      char const *syncWord;
      unsigned int syncWordLength;
      //ofstream *minorFrameFile;
      FILE *minorFrameFile;
   public:
      void SetFileHandle(FILE *minorFrameFileIn);
      ByteSyncOnSyncword(char const *syncWordIn, unsigned int syncWordLengthIn);
      ~ByteSyncOnSyncword();
      int Work(unsigned char *bitStreamIn, double *bitStreamInTime, unsigned long nSamples);
   };
#endif