#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <ctype.h>
#include <cuda.h>

#define MEGA 1024*1024
#define TWOBILLS 2000000000

typedef unsigned char uch;
typedef unsigned long ul;
typedef unsigned int  ui;

FILE *fw;
long num;
long count = 0;
int chunk, NumBlocks, NumThreads;
int *arri, *arriG;
int *arris;
float *arrf, *arrfG;
float *arrfs;
double *arrd, *arrdG;
double *arrds;
long *arrl, *arrlG;
long *arrls;

__global__
void *iBubG(int *arriG) {
	long myBid = blockIdx.x;
	long myTid = threadIdx.x;
	long ThPerBlk = blockDim.x;
	long myIndex = ((myBid*ThPerBlk)+myTid)*chunk;
	long i = myIndex, j = myIndex;
	int hold;
	for (; i<myIndex+chunk;i++) {
		for(; j<myIndex+chunk-(i-myIndex);j++) {
			if (arriG[j] < arriG[j+1]) {
				hold = arriG[j];
				arriG[j] = arriG[j+1];
				arriG[j+1] = hold;
			}
		}
	}
}
__global__
void *fBubG(float *arrfG) {
	long myBid = blockIdx.x;
	long myTid = threadIdx.x;
	long ThPerBlk = blockDim.x;
	long myIndex = ((myBid*ThPerBlk)+myTid)*chunk;
	long i = myIndex, j = myIndex;
	float hold;
	for (; i<myIndex+chunk;i++) {
		for(; j<myIndex+chunk-(i-myIndex);j++) {
			if (arrfG[j] < arrfG[j+1]) {
				hold = arrfG[j];
				arrfG[j] = arrfG[j+1];
				arrfG[j+1] = hold;
			}
		}
	}
}
__global__
void *dBubG(double *arrdG) {
	long myBid = blockIdx.x;
	long myTid = threadIdx.x;
	long ThPerBlk = blockDim.x;
	long myIndex = ((myBid*ThPerBlk)+myTid)*chunk;
	long i = myIndex, j = myIndex;
	double hold;
	for (; i<myIndex+chunk;i++) {
		for(; j<myIndex+chunk-(i-myIndex);j++) {
			if (arrdG[j] < arrdG[j+1]) {
				hold = arrdG[j];
				arrdG[j] = arrdG[j+1];
				arrdG[j+1] = hold;
			}
		}
	}
}
__global__
void *lBubG(long *arrlG) {
	long myBid = blockIdx.x;
	long myTid = threadIdx.x;
	long ThPerBlk = blockDim.x;
	long myIndex = ((myBid*ThPerBlk)+myTid)*chunk;
	long i = myIndex, j = myIndex;
	long hold;
	for (; i<myIndex+chunk;i++) {
		for(; j<myIndex+chunk-(i-myIndex);j++) {
			if (arrlG[j] < arrlG[j+1]) {
				hold = arri[j];
				arrlG[j] = arrlG[j+1];
				arrlG[j+1] = hold;
			}
		}
	}
}

void iMer() {
	int temp[chunk];
	int hold;
	long i;
	int j;
	while(count<num){
		i = count;
		j = 0;
		for (;i<count+chunk;i++)
			temp[i-count] = arri[i];
	
		for (i=0; i<count; i++) {
			while(temp[j] >= arri[i] && j<chunk-2) {
				j++;
			}
		
			if (temp[j] < arri[i]) {
				hold = temp[j];
				temp[j] = arri[i];
				arri[i] = hold;
			}
		}
		for (i = 0; i<chunk; i++)
			arri[i+count] = temp[i];

		count+=chunk;
	}
	
	return;
}

void fMer() {
	int temp[chunk];
	int hold;
	long i;
	int j;
	while(count<num){
		i = count;
		j = 0;
		for (;i<count+chunk;i++)
			temp[i-count] = arrf[i];
	
		for (i=0; i<count; i++) {
			while(temp[j] >= arrf[i] && j<chunk-2) {
				j++;
			}
		
			if (temp[j] < arrf[i]) {
				hold = temp[j];
				temp[j] = arrf[i];
				arrf[i] = hold;
			}
		}
		for (i = 0; i<chunk; i++)
			arrf[i+count] = temp[i];

		count+=chunk;
	}
	
	return;
}

void dMer() {
	int temp[chunk];
	int hold;
	long i;
	int j;
	while(count<num){
		i = count;
		j = 0;
		for (;i<count+chunk;i++)
			temp[i-count] = arri[i];
	
		for (i=0; i<count; i++) {
			while(temp[j] >= arrd[i] && j<chunk-2) {
				j++;
			}
		
			if (temp[j] < arrd[i]) {
				hold = temp[j];
				temp[j] = arrd[i];
				arrd[i] = hold;
			}
		}
		for (i = 0; i<chunk; i++)
			arrd[i+count] = temp[i];

		count+=chunk;
	}
	
	return;
}

void lMer() {
	int temp[chunk];
	int hold;
	long i;
	int j;
	while(count<num){
		i = count;
		j = 0;
		for (;i<count+chunk;i++)
			temp[i-count] = arrl[i];
	
		for (i=0; i<count; i++) {
			while(temp[j] >= arrl[i] && j<chunk-2) {
				j++;
			}
		
			if (temp[j] < arrl[i]) {
				hold = temp[j];
				temp[j] = arrl[i];
				arrl[i] = hold;
			}
		}
		for (i = 0; i<chunk; i++)
			arrl[i+count] = temp[i];

		count+=chunk;
	}
	
	return;
}

void genI () {
	float			totalTime, tfrCPUtoGPU, tfrGPUtoCPU, kernelExecutionTime; // GPU code run times
	cudaError_t		cudaStatus, cudaStatus2;
	cudaEvent_t		time1, time2, time3, time4;
	cudaDeviceProp	GPUprop;
	arri = (int *)malloc(num*sizeof(int));
	arris = (int *)malloc(num*sizeof(int));
	long i;
	int pn;
	for(i = 0; i< num; i++) {
		pn = rand()%2;
		if (pn == 0)
			arri[i] = rand();
			
		else
			arri[i] = -1*rand();
	}
	
	int NumGPUs = 0;
	cudaGetDeviceCount(&NumGPUs);
	if (NumGPUs == 0){
		printf("No CUDA Device is available\n");
		exit(EXIT_FAILURE);
	}
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?\n");
		exit(EXIT_FAILURE);
	}
	
	cudaGetDeviceProperties(&GPUprop, 0);
	
	cudaEventCreate(&time1);
	cudaEventCreate(&time2);
	cudaEventCreate(&time3);
	cudaEventCreate(&time4);
	
	cudaEventRecord(time1, 0);		// Time stamp at the start of the GPU transfer
	// Allocate GPU buffer for the input and output images
	cudaStatus = cudaMalloc((int *)&arriG, num*sizeof(int));
	if (cudaStatus!=cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed! Can't allocate GPU memory\n");
		exit(EXIT_FAILURE);
	}
	
	cudaStatus = cudaMemcpy(arriG, arri, num*(sizeof(int)), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy  CPU to GPU  failed!\n");
		exit(EXIT_FAILURE);
	}

	cudaEventRecord(time2, 0);
	iBubG <<<NumBlocks, NumThreads>>> (arriG);
	
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching the kernel!\n", cudaStatus);
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time3, 0);

	// Copy output (results) from GPU buffer to host (CPU) memory.
	cudaStatus = cudaMemcpy(arri, arriG, num*sizeof(int)), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy GPU to CPU  failed!\n");
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time4, 0);

	cudaEventSynchronize(time1);
	cudaEventSynchronize(time2);
	cudaEventSynchronize(time3);
	cudaEventSynchronize(time4);

	cudaEventElapsedTime(&totalTime, time1, time4);
	cudaEventElapsedTime(&tfrCPUtoGPU, time1, time2);
	cudaEventElapsedTime(&kernelExecutionTime, time2, time3);
	cudaEventElapsedTime(&tfrGPUtoCPU, time3, time4);

	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Program failed after cudaDeviceSynchronize()!\n");
		free(arri);
		free(arris);
		exit(EXIT_FAILURE);
	}
	
	cudaFree(arriG);
	cudaEventDestroy(time1);
	cudaEventDestroy(time2);
	cudaEventDestroy(time3);
	cudaEventDestroy(time4);
	
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		free(arri);
		free(arris);
		exit(EXIT_FAILURE);
	}
	count = 0;
	iMer();
	
	memcpy(arris, arri, num*sizeof(int));
	fw = fopen("outi.txt", "w");
	for (i = 0; i<num; i++) {
		fprintf(fw, "%d\n", arris[i]);
	}
	fclose(fw);
	free(arri);
	free(arris);
	return;
}

void genF () {
float			totalTime, tfrCPUtoGPU, tfrGPUtoCPU, kernelExecutionTime; // GPU code run times
	cudaError_t		cudaStatus, cudaStatus2;
	cudaEvent_t		time1, time2, time3, time4;
	cudaDeviceProp	GPUprop;
	arrf = (float *)malloc(num*sizeof(float));
	arrfs = (float *)malloc(num*sizeof(float));
	long i;
	int pn;
	for(i = 0; i< num; i++) {
		pn = rand()%2;
		if (pn == 0)
			arrf[i] = rand();
			
		else
			arrf[i] = -1*rand();
	}
	
	int NumGPUs = 0;
	cudaGetDeviceCount(&NumGPUs);
	if (NumGPUs == 0){
		printf("No CUDA Device is available\n");
		exit(EXIT_FAILURE);
	}
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?\n");
		exit(EXIT_FAILURE);
	}
	
	cudaGetDeviceProperties(&GPUprop, 0);
	
	cudaEventCreate(&time1);
	cudaEventCreate(&time2);
	cudaEventCreate(&time3);
	cudaEventCreate(&time4);
	
	cudaEventRecord(time1, 0);		// Time stamp at the start of the GPU transfer
	// Allocate GPU buffer for the input and output images
	cudaStatus = cudaMalloc((float0 *)&arrfG, num*sizeof(float));
	if (cudaStatus!=cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed! Can't allocate GPU memory\n");
		exit(EXIT_FAILURE);
	}
	
	cudaStatus = cudaMemcpy(arriG, arri, num*(sizeof(int)), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy  CPU to GPU  failed!\n");
		exit(EXIT_FAILURE);
	}

	cudaEventRecord(time2, 0);
	iBubG <<<NumBlocks, NumThreads>>> (arriG);
	
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching the kernel!\n", cudaStatus);
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time3, 0);

	// Copy output (results) from GPU buffer to host (CPU) memory.
	cudaStatus = cudaMemcpy(arrf, arrfG, num*sizeof(float)), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy GPU to CPU  failed!\n");
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time4, 0);

	cudaEventSynchronize(time1);
	cudaEventSynchronize(time2);
	cudaEventSynchronize(time3);
	cudaEventSynchronize(time4);

	cudaEventElapsedTime(&totalTime, time1, time4);
	cudaEventElapsedTime(&tfrCPUtoGPU, time1, time2);
	cudaEventElapsedTime(&kernelExecutionTime, time2, time3);
	cudaEventElapsedTime(&tfrGPUtoCPU, time3, time4);

	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Program failed after cudaDeviceSynchronize()!\n");
		free(arrf);
		free(arrfs);
		exit(EXIT_FAILURE);
	}
	
	cudaFree(arrfG);
	cudaEventDestroy(time1);
	cudaEventDestroy(time2);
	cudaEventDestroy(time3);
	cudaEventDestroy(time4);
	
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		free(arrf);
		free(arrfs);
		exit(EXIT_FAILURE);
	}
	count = 0;
	fMer();
	
	memcpy(arrfs, arrf, num*sizeof(float));
	fw = fopen("outf.txt", "w");
	for (i = 0; i<num; i++) {
		fprintf(fw, "%f\n", arrfs[i]);
	}
	fclose(fw);
	free(arrf);
	free(arrfs);
	return;
}

void genD () {
float			totalTime, tfrCPUtoGPU, tfrGPUtoCPU, kernelExecutionTime; // GPU code run times
	cudaError_t		cudaStatus, cudaStatus2;
	cudaEvent_t		time1, time2, time3, time4;
	cudaDeviceProp	GPUprop;
	arrd = (double *)malloc(num*sizeof(double));
	arrds = (double *)malloc(num*sizeof(double));
	long i;
	int pn;
	for(i = 0; i< num; i++) {
		pn = rand()%2;
		if (pn == 0)
			arrd[i] = rand();
			
		else
			arrd[i] = -1*rand();
	}
	
	int NumGPUs = 0;
	cudaGetDeviceCount(&NumGPUs);
	if (NumGPUs == 0){
		printf("No CUDA Device is available\n");
		exit(EXIT_FAILURE);
	}
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?\n");
		exit(EXIT_FAILURE);
	}
	
	cudaGetDeviceProperties(&GPUprop, 0);
	
	cudaEventCreate(&time1);
	cudaEventCreate(&time2);
	cudaEventCreate(&time3);
	cudaEventCreate(&time4);
	
	cudaEventRecord(time1, 0);		// Time stamp at the start of the GPU transfer
	// Allocate GPU buffer for the input and output images
	cudaStatus = cudaMalloc((double *)&arrdG, num*sizeof(double));
	if (cudaStatus!=cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed! Can't allocate GPU memory\n");
		exit(EXIT_FAILURE);
	}
	
	cudaStatus = cudaMemcpy(arriG, arri, num*(sizeof(int)), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy  CPU to GPU  failed!\n");
		exit(EXIT_FAILURE);
	}

	cudaEventRecord(time2, 0);
	iBubG <<<NumBlocks, NumThreads>>> (arrdG);
	
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching the kernel!\n", cudaStatus);
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time3, 0);

	// Copy output (results) from GPU buffer to host (CPU) memory.
	cudaStatus = cudaMemcpy(arrd, arrdG, num*sizeof(double)), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy GPU to CPU  failed!\n");
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time4, 0);

	cudaEventSynchronize(time1);
	cudaEventSynchronize(time2);
	cudaEventSynchronize(time3);
	cudaEventSynchronize(time4);

	cudaEventElapsedTime(&totalTime, time1, time4);
	cudaEventElapsedTime(&tfrCPUtoGPU, time1, time2);
	cudaEventElapsedTime(&kernelExecutionTime, time2, time3);
	cudaEventElapsedTime(&tfrGPUtoCPU, time3, time4);

	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Program failed after cudaDeviceSynchronize()!\n");
		free(arrd);
		free(arrds);
		exit(EXIT_FAILURE);
	}
	
	cudaFree(arrdG);
	cudaEventDestroy(time1);
	cudaEventDestroy(time2);
	cudaEventDestroy(time3);
	cudaEventDestroy(time4);
	
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		free(arrd);
		free(arrds);
		exit(EXIT_FAILURE);
	}
	count = 0;
	dMer();
	
	memcpy(arrds, arrd, num*sizeof(double));
	fw = fopen("outd.txt", "w");
	for (i = 0; i<num; i++) {
		fprintf(fw, "%lf\n", arrds[i]);
	}
	fclose(fw);
	free(arrd);
	free(arrds);
	return;
}

void genL () {
	float			totalTime, tfrCPUtoGPU, tfrGPUtoCPU, kernelExecutionTime; // GPU code run times
	cudaError_t		cudaStatus, cudaStatus2;
	cudaEvent_t		time1, time2, time3, time4;
	cudaDeviceProp	GPUprop;
	arrl = (long *)malloc(num*sizeof(long));
	arrls = (long *)malloc(num*sizeof(long));
	long i;
	int pn;
	for(i = 0; i< num; i++) {
		pn = rand()%2;
		if (pn == 0) {
			pn = (rand() << 16);
			arrl[i] = pn%TWOBILLS;
		}	
		else {
			pn = (rand() << 16);
			arrl[i] = -1*(pn%TWOBILLS);
		}
	}
	int NumGPUs = 0;
	cudaGetDeviceCount(&NumGPUs);
	if (NumGPUs == 0){
		printf("No CUDA Device is available\n");
		exit(EXIT_FAILURE);
	}
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?\n");
		exit(EXIT_FAILURE);
	}
	
	cudaGetDeviceProperties(&GPUprop, 0);
	
	cudaEventCreate(&time1);
	cudaEventCreate(&time2);
	cudaEventCreate(&time3);
	cudaEventCreate(&time4);
	
	cudaEventRecord(time1, 0);		// Time stamp at the start of the GPU transfer
	// Allocate GPU buffer for the input and output images
	cudaStatus = cudaMalloc((long *)&arrlG, num*sizeof(long));
	if (cudaStatus!=cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed! Can't allocate GPU memory\n");
		exit(EXIT_FAILURE);
	}
	
	cudaStatus = cudaMemcpy(arrlG, arrl, num*(sizeof(long)), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy  CPU to GPU  failed!\n");
		exit(EXIT_FAILURE);
	}

	cudaEventRecord(time2, 0);
	lBubG <<<NumBlocks, NumThreads>>> (arrlG);
	
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching the kernel!\n", cudaStatus);
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time3, 0);

	// Copy output (results) from GPU buffer to host (CPU) memory.
	cudaStatus = cudaMemcpy(arrl, arrlG, num*sizeof(long)), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy GPU to CPU  failed!\n");
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time4, 0);

	cudaEventSynchronize(time1);
	cudaEventSynchronize(time2);
	cudaEventSynchronize(time3);
	cudaEventSynchronize(time4);

	cudaEventElapsedTime(&totalTime, time1, time4);
	cudaEventElapsedTime(&tfrCPUtoGPU, time1, time2);
	cudaEventElapsedTime(&kernelExecutionTime, time2, time3);
	cudaEventElapsedTime(&tfrGPUtoCPU, time3, time4);

	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Program failed after cudaDeviceSynchronize()!\n");
		free(arrl);
		free(arrls);
		exit(EXIT_FAILURE);
	}
	
	cudaFree(arrlG);
	cudaEventDestroy(time1);
	cudaEventDestroy(time2);
	cudaEventDestroy(time3);
	cudaEventDestroy(time4);
	
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		free(arrl);
		free(arrls);
		exit(EXIT_FAILURE);
	}
	count = 0;
	lMer();
	
	memcpy(arrls, arrl, num*sizeof(long));
	fw = fopen("outl.txt", "w");
	for (i = 0; i<num; i++) {
		fprintf(fw, "%ld\n", arrls[i]);
	}
	fclose(fw);
	free(arrl);
	free(arrls);
	return;
}
		
int main (int argc, char** argv) {
	char choice;
		
	if (argc != 5) {
		printf("Incorrect call. Correct call:\n");
		printf("./filename num type chunk threads\n");
		return 1;
	}
	
	choice = toupper(*argv[2]);
	chunk = atoi(argv[3]);
	NumThreads = atoi(argv[4]);
	num = (atoi(argv[1])*MEGA);
	NumBlocks = num/(NumThreads*chunk);
	if (num%chunk!=0) {
		printf("Please choose a number that is evenly divisible by your #chunks.\n");
		return 1;
	}
	
	switch (choice) {
		case 'I': genI(); break;
		case 'F': genF(); break;
		case 'D': genD(); break;
		case 'L': genL(); break;
		default: printf("Please revise your function call. Accepted inputs are:\n");
				 printf("d, i, l, f, D, I, L, F.\n");
				 return 1;
	}
	
	return(EXIT_SUCCESS);
}

