#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "ImageStuff.h"

#define REPS         11
#define MAXTHREADS   128
#define BUFFER_SIZE  10*1024

long  			NumThreads;         		// Total number of threads working in parallel
int 	     	ThParam[MAXTHREADS];		// Thread parameters ...
pthread_t      	ThHandle[MAXTHREADS];		// Thread handles
pthread_attr_t 	ThAttr;						// Pthread attrributes

unsigned char**	Image1;
unsigned char**	Image2;
unsigned char**	OutImage;
struct ImgProp 	ip;  // the important parts (resolution) must be common to Image1 and Image2

// Add two images (0.7*img1 + 0.3*img2).
void AddImages() {
  int row, col;
  for (col=0; col<ip.Hbytes; col++) {
    for(row=0; row<ip.Vpixels; row++) {
      OutImage[row][col] = 0.7*Image1[row][col] + 0.3*Image2[row][col];
    }
  }
}

void *AddImagesMT(void* tid)
{
  /*long ts = *((int *) tid);              // My thread ID is stored here
  ts *= ip.Vpixels/NumThreads;           // start index
  long te = ts+ip.Vpixels/NumThreads-1;  // end index

  int row, col;
  for (col=0; col<ip.Hbytes; col++) {
    for(row=ts; row<=te; row++) {
      OutImage[row][col] = 0.7*Image1[row][col] + 0.3*Image2[row][col];
    }
  }*/
  unsigned char buff[BUFFER_SIZE];
  unsigned char buff2[BUFFER_SIZE];
  const int sever = 734007;
  const int three = 314572;
  const int ten = 1024*1024;
  unsigned int chunk, thrID, firstRow, lastRow;
  
  thrID = *((unsigned int *) tid);
  chunk = ip.Vpixels/NumThreads;
  firstRow = thrID * chunk;
  
  //lastRow = firstRow + chunk;
  
  if(thrID == NumThreads - 1)
	lastRow = ip.Vpixels;
  
  for (unsigned int currR = firstRow; currR < lastRow; currR++)
  {
	memcpy((void *)buff, (void *)Image1[currR], (size_t) ip.Hbytes);
	memcpy((void *)buff2, (void *)Image2[currR], (size_t) ip.Hbytes);
	
	for (unsigned int currC = 0; currC < ip.Hbytes; currC++)
	{
		buff1[currC] = (seven*buff1[currC] + three*buff2[currC])/ten;
	}
	
	memcpy((void *)OutImage[currR], (void *)buff1, (size_t) ip.Hbytes);
  }
}

int main(int argc, char** argv)
{
  int 				a,i,ThErr;
  struct timeval 		t;
  double         		StartTime, EndTime;
  double         		TimeElapsed;
	
  switch (argc) {
    case 4 : NumThreads=0; break;
    case 5 : NumThreads=atoi(argv[4]); break;
    default: printf("\n\nUsage: imadd input1 input2 output [0,1-128]");
				 printf("\n\nNumThreads=0 for the serial version, and 1-128 for the Pthreads version\n\n");
				 printf("\n\nExample: imflipPm infilename1.bmp infilename2.bmp outname.bmp 0\n\n");
				 printf("\n\nNothing executed ... Exiting ...\n\n");
				 exit(EXIT_FAILURE);
  }
	if((NumThreads<0) || (NumThreads>MAXTHREADS)){
            printf("\nNumber of threads must be between 0 and %u... \n",MAXTHREADS);
            printf("\n'1' means Pthreads version with a single thread\n");
            printf("\nYou can also specify '0' which means the 'serial' (non-Pthreads) version... \n\n");
			 printf("\n\nNothing executed ... Exiting ...\n\n");
            exit(EXIT_FAILURE);
	}
	if(NumThreads == 0){
		printf("\nExecuting the serial (non-pthreaded) version ...\n");
	}else{
		printf("\nExecuting the multi-threaded version with %li threads ...\n",NumThreads);
	}

	Image1 = ReadBMP(argv[1]);
	Image2 = ReadBMP(argv[2]);
	// We should verify that they have the same dimensions here, but you'll
	// find out when you get garbage / segfault.

	// Allocate OutImage:
	OutImage = (unsigned char **)malloc(ip.Vpixels * sizeof(unsigned char*));
	for(i=0; i<ip.Vpixels; i++) {
	  OutImage[i] = (unsigned char *)malloc(ip.Hbytes * sizeof(unsigned char));
	}

	gettimeofday(&t, NULL);
    StartTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
	
    if(NumThreads > 0) {
      pthread_attr_init(&ThAttr);
      pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
      for(a=0; a<REPS; a++){
	for(i=0; i<NumThreads; i++){
	  ThParam[i] = i;
	  ThErr = pthread_create(&ThHandle[i], &ThAttr, AddImagesMT, (void *)&ThParam[i]);
	  if(ThErr != 0){
	    printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
	    exit(EXIT_FAILURE);
	  }
	}
	for(i=0; i<NumThreads; i++){
	  pthread_join(ThHandle[i], NULL);
	}
      }
    }
    else {
      for(a=0; a<REPS; a++){
	AddImages();
      }
    }

    gettimeofday(&t, NULL);
    EndTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
	TimeElapsed=(EndTime-StartTime)/1000.00;
	TimeElapsed/=(double)REPS;
	
    //merge with header and write to file
    WriteBMP(OutImage, argv[3]);
	
    // free() the allocated memory for the images
    for(i = 0; i < ip.Vpixels; i++) { free(Image1[i]); }
    free(Image1);
    for(i = 0; i < ip.Vpixels; i++) { free(Image2[i]); }
    free(Image2);
    for(i = 0; i < ip.Vpixels; i++) { free(OutImage[i]); }
    free(OutImage);
   
    printf("\n\nTotal execution time: %9.4f ms.  ",TimeElapsed);
	if(NumThreads>1) printf("(%9.4f ms per thread).  ",TimeElapsed/(double)NumThreads);
    printf("\n (%6.3f ns/pixel)\n", 1000000*TimeElapsed/(double)(ip.Hpixels*ip.Vpixels));
    
    return (EXIT_SUCCESS);
}
