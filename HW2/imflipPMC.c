#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "ImageStuff.h"

#define REPS 	     129
#define MAXTHREADS   128

long  			NumThreads;         		// Total number of threads working in parallel
int 	     	ThParam[MAXTHREADS];		// Thread parameters ...
pthread_t      	ThHandle[MAXTHREADS];		// Thread handles
pthread_attr_t 	ThAttr;						// Pthread attrributes
void (*FlipFunc)(unsigned char** img);		// Function pointer to flip the image
void* (*MTFlipFunc)(void *arg);				// Function pointer to flip the image, multi-threaded version

unsigned char**	TheImage;					// This is the main image
struct ImgProp 	ip;

void *MTFlipHMC (void* tid) {
	struct Pixel pix;
	int row, col, done;
	unsigned long swap[5461];
	unsigned long val, val2;
	
	long ts = *((int *) tid);
	ts *= ip.Vpixels/NumThreads;
	long te = ts+ip.Vpixels/NumThreads-1;
	done = ip.Hpixels*3/8;
	for (row = ts; row < te; row++) {
		memcpy((void *)swap, (void *)TheImage[row], (size_t)ip.Hbytes);
		col = 0;
		while(col<(done>>1)){
			//printf("%d\n", row);
			val = (swap[col] & 0xFFFFFF0000000000) >> 40;
			swap[col] = swap[col] & 0x000000FFFFFFFFFF;
			swap[col] = ((swap[(done)-(col+1)] & 0xFFFFFF) << 40) | swap[col];
			swap[done-(col+1)] = (swap[done-(col+1)] & 0xFFFFFFFFFF000000) | val;
			
			val = (swap[col] & 0xFFFFFF0000) << 8;
			swap[col] = swap[col] & 0xFFFFFF000000FFFF;
			swap[col] = ((swap[done-(col+1)] & 0xFFFFFF000000) >> 8) | swap[col];
			swap[done-(col+1)] = (swap[(done)-(col+1)] & 0xFFFF000000FFFFFF) | val;
			
			val = (swap[col] & 0xFFFF) << 8;
			val = val | ((swap[col+1] & 0xFF00000000000000) >> 56);
			swap[col] = swap[col] & 0xFFFFFFFFFFFF0000;
			val2 = swap[(done)-(col+2)] & 0xFF << 16;
			val2 = val2 | (swap[done-(col+1)] & 0xFFFF000000000000 >> 48);
			swap[col] = swap[col] | (val2 >> 8);
			swap[col+1] = (swap[col+1] & 0xFFFFFFFFFFFFFF) | (val2 & 0xFF);
			swap[done-(col+1)] = (swap[(done)-(col+1)] & 0xFFFFFFFFFFFF) | ((val&0xFFFF) << 48);
			swap[done-(col+2)] = (swap[(done)-(col+2)] & 0xFFFFFFFFFFFFFF00) | (val >> 16);
			
			val = (swap[col+1] & 0xFFFFFF00000000) >> 24;
			swap[col+1] = swap[col+1] & 0xFF000000FFFFFFFF;
			swap[col+1] = ((swap[(done)-(col+2)] & 0xFFFFFF00) << 24) | swap[col+1];
			swap[done-(col+2)] = (swap[done-(col+2)] & 0xFFFFFFFF000000FF) | val;
			
			val = (swap[col+1] & 0xFFFFFF00) << 24;
			swap[col+1] = swap[col+1] & 0xFFFFFFFF000000FF;
			swap[col+1] = ((swap[done-(col+2)] & 0xFFFFFF00000000) >> 24) | swap[col+1];
			swap[done-(col+2)] = (swap[done-(col+2)] & 0xFF000000FFFFFFFF) | val;
			
			val = (swap[col+1] & 0xFF) << 16;
			val = val | (swap[col+2] & 0xFFFF000000000000) >> 48;
			val2 = (swap[done-(col+2)] & 0xFF00000000000000) >> 40;
			val2 = (swap[done-(col+3)] & 0xFFFF) | val2;
			swap[col+1] = swap[col+1] & 0xFFFFFFFFFFFFFF00;
			swap[col+1] = swap[col+1] | (val2 >> 16);
			swap[col+2] = swap[col+2] & 0xFFFFFFFFFFFF;
			swap[col+2] = swap[col+2] | ((val2 & 0xFFFF) << 48);
			swap[done-(col+2)] = (swap[done-(col+2)] & 0xFFFFFFFFFFFFFF) | ((val & 0xFF) >> 56);
			swap[done-(col+3)] = (swap[done-(col+3)] & 0xFFFFFFFFFFFF0000) | (val & 0xFFFF);
			
			val = (swap[col+2] & 0xFFFFFF000000);
			swap[col+2] = swap[col+2] & 0xFFFF000000FFFFFF;
			swap[col+2] = (swap[done-(col+3)] & 0xFFFFFF000000) | swap[col+2];
			swap[done-(col+3)] = (swap[done-(col+3)] & 0xFFFF000000FFFFFF) | val;
			
			val = (swap[col+2] & 0xFFFFFF);
			swap[col+2] = (swap[col+2] & 0xFFFFFFFFFF000000) | (swap[done-(col+3)] & 0xFFFFFF);
			swap[done-(col+3)] = (swap[done-(col+3)] & 0xFFFFFFFFFF) | (val << 40);
			memcpy((void *) TheImage[row], (void *)swap, (size_t) ip.Hbytes);
			//printf("%d\n", col);
			col += 3;
		}
	}
}

void *MTFlipHM(void* tid)
{
    struct Pixel pix; //temp swap pixel
    int row, col;
	unsigned char Buffer[16384];	 // This is the buffer to use to get the entire row

    long ts = *((int *) tid);       	// My thread ID is stored here
    ts *= ip.Vpixels/NumThreads;			// start index
	long te = ts+ip.Vpixels/NumThreads-1; 	// end index

    for(row=ts; row<=te; row++){
        memcpy((void *) Buffer, (void *) TheImage[row], (size_t) ip.Hbytes);
		col=0;
        while(col<ip.Hpixels*3/2){
            pix.B = Buffer[col];
            pix.G = Buffer[col+1];
            pix.R = Buffer[col+2];
            
            Buffer[col]   = Buffer[ip.Hpixels*3-(col+3)];
            Buffer[col+1] = Buffer[ip.Hpixels*3-(col+2)];
            Buffer[col+2] = Buffer[ip.Hpixels*3-(col+1)];
            
            Buffer[ip.Hpixels*3-(col+3)] = pix.B;
            Buffer[ip.Hpixels*3-(col+2)] = pix.G;
            Buffer[ip.Hpixels*3-(col+1)] = pix.R;
            
            col+=3;
        }
        memcpy((void *) TheImage[row], (void *) Buffer, (size_t) ip.Hbytes);
    }
    pthread_exit(NULL);
}

void *MTFlipVM2(void* tid) {
	struct Pixel pix; //temp swap pixel
    int row, row2, col;
	unsigned char Buffer[16384];	 // This is the buffer to get the first row

    long ts = *((int *) tid);       	// My thread ID is stored here
    ts *= ip.Vpixels/NumThreads/2;				// start index
	long te = ts+(ip.Vpixels/NumThreads/2)-1; 	// end index

    for(row=ts; row<=te; row++){
        memcpy((void *) Buffer, (void *) TheImage[row], (size_t) ip.Hbytes);
        row2=ip.Vpixels-(row+1);   
		// swap row with row2
		memcpy((void *) TheImage[row], (void *) TheImage[row2], (size_t) ip.Hbytes);
		memcpy((void *) TheImage[row2], (void *) Buffer, (size_t) ip.Hbytes);
    }
    pthread_exit(NULL);
}


int main(int argc, char** argv)
{
	char 				Flip;
    int 				a,i,ThErr;
    struct timeval 		t;
    double         		StartTime, EndTime;
    double         		TimeElapsed;
	char				FlipType[50];
	
    switch (atoi(argv[4])){
		case 0 : NumThreads=atoi(argv[3]); 				MTFlipFunc = MTFlipHM;		break;
		case 1 : NumThreads=atoi(argv[3]);  			MTFlipFunc = MTFlipHMC;		break;
		default: printf("\n\nUsage: imflipPm input output [0-128] [0,1]");
				 printf("\nUse 0 for the M version, 1 for the MC version.");
				 printf("\n\nNumThreads=0 for the serial version, and 1-128 for the Pthreads version\n\n");
				 printf("\n\nExample: imflipPm infilename.bmp outname.bmp w 8\n\n");
				 printf("\n\nExample: imflipPm infilename.bmp outname.bmp V 0\n\n");
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
		printf("\nExecuting the serial (non-Pthreaded) version ...\n");
	}else{
		printf("\nExecuting the multi-threaded version with %ld threads ...\n",NumThreads);
	}

	TheImage = ReadBMP(argv[1]);

	gettimeofday(&t, NULL);
    StartTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
	
    if(NumThreads >0){
		pthread_attr_init(&ThAttr);
		pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
		for(a=0; a<REPS; a++){
			for(i=0; i<NumThreads; i++){
				ThParam[i] = i;
				ThErr = pthread_create(&ThHandle[i], &ThAttr, MTFlipFunc, (void *)&ThParam[i]);
				if(ThErr != 0){
					printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
					exit(EXIT_FAILURE);
				}
			}
			for(i=0; i<NumThreads; i++){
				pthread_join(ThHandle[i], NULL);
			}
		}
	}else{
		for(a=0; a<REPS; a++){
			(*FlipFunc)(TheImage);
		}
	}
	
    gettimeofday(&t, NULL);
    EndTime = (double)t.tv_sec*1000000.0 + ((double)t.tv_usec);
	TimeElapsed=(EndTime-StartTime)/1000.00;
	TimeElapsed/=(double)REPS;
	
    //merge with header and write to file
    WriteBMP(TheImage, argv[2]);
	
 	// free() the allocated memory for the image
	for(i = 0; i < ip.Vpixels; i++) { free(TheImage[i]); }
	free(TheImage);
   
    printf("\n\nTotal execution time: %9.4f ms.  ",TimeElapsed);
	if(NumThreads>1) printf("(%9.4f ms per thread).  ",TimeElapsed/(double)NumThreads);
	printf("\n\nFlip Type =  '%s'",FlipType);
    printf("\n (%6.3f ns/pixel)\n", 1000000*TimeElapsed/(double)(ip.Hpixels*ip.Vpixels));
    
    return (EXIT_SUCCESS);
}
