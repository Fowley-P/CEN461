/* Patrick Fowley
 * ICEN 461
 * imshrink.c
 */

#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ImageStuff.h"

#define NUM_THREADS 4

int ThParam[NUM_THREADS];
pthread_t ThHandle[NUM_THREADS];
pthread_attr_t ThAttr;
pthread_mutex_t lock;

unsigned char** image;
unsigned char** imgCpy;
struct ImgProp ip;
int skipR, skipC, newR;
int currR = 0;

void *shrink(void* tid) {
	int col, row, newC;
	newC = 0;
	col = 0;
	int tr = *(int *)tid;
	row = currR + (tr*skipR);

	while (col < ip.Hbytes) {
		imgCpy[newR+tr][newC] = image[row][col];
		imgCpy[newR+tr][newC+1] = image[row][col+1];
		imgCpy[newR+tr][newC+2] = image[row][col+2];
		newC += 3;
		col += skipC*3;
	}
	
	pthread_exit(NULL);
}

int main (int argc, char** argv) {
	int ThErr, i;
		
	if (argc != 5) {
		printf("Incorrect call. Proper call:\n");
		printf("./filename in.bmp out.bmp xShrink yShrink\n");
		return 0;
	}
	
	image = ReadBMP(argv[1]);
	imgCpy = CreateBlankBMP(255);
	
	skipC = atoi(argv[3]);
	skipR = atoi(argv[4]);
	
	
	pthread_attr_init(&ThAttr);
	pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
	
	while ((currR + (skipR*NUM_THREADS))<ip.Vpixels)
	{
		for(i=0; i<NUM_THREADS; i++){
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, shrink, (void *)&ThParam[i]);
			if(ThErr != 0){
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		
		for(i=0; i<NUM_THREADS; i++) {
			pthread_join(ThHandle[i], NULL);
		}

		pthread_mutex_lock(&lock);
		newR += NUM_THREADS;
		currR += (skipR * NUM_THREADS);
		pthread_mutex_unlock(&lock);
	}
	WriteBMP(imgCpy, argv[2]);
	/*for (i=0; i<ip.Vpixels; i++) {
		free(image[i]);
	}*/
	for (i=0; i<newR; i++) {
		free(image[i]);
		free(imgCpy[i]);
	}
	free(image);	
	free(imgCpy);
	printf("\n");
	return(EXIT_SUCCESS);
}
