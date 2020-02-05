#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <ctype.h>
#include <cuda.h>
#include <math.h>

#define	CEIL(a,b)		((a+b-1)/b)
#define SWAP(a,b,t)		t=b; b=a; a=t;
#define DATAMB(bytes)			(bytes/1024/1024)
#define DATABW(bytes,timems)	((float)bytes/(timems * 1.024*1024.0*1024.0))

typedef unsigned char uch;
typedef unsigned long ul;
typedef unsigned int  ui;

uch *image, *imgCpy;
uch *imageG, *imgCpyG, *outG;

struct ImgProp{
	int Hpixels;
	int Vpixels;
	uch HeaderInfo[54];
	ul Hbytes;
} ip;

#define	IPHB		ip.Hbytes
#define	IPH			ip.Hpixels
#define	IPV			ip.Vpixels
#define	IMAGESIZE	(IPHB*IPV)
#define	IMAGEPIX	(IPH*IPV)

__global__
void *shrinkG(uch* dest, uch* source, ui skipC, ui skipR) {
	ui ThPerBlk = blockDim.x;
	ui myBid = blockIdx.x;
	ui myTid = threadIdx.x;
	ui myRow = blockIdx.y;
	ui myIndex = (myBid*ThPerBlk + myTid) * skipR * IPHB;
	if (myIndex > IMAGESIZE)
		return;
	ui destIndex = (myBid*ThPerBlk + myTid) * IPHB;
	while (myIndex < (myIndex+IPHB-2)) {
		dest[destIndex] = source[myIndex];
		dest[destIndex+1] = source[myIndex+1];
		dest[destIndeex+2] = source[myIndex+2];
		destIndex += 3;
		myIndex += (skipC*3);
	}
}

uch *ReadBMPlin(char* fn)
{
	static uch *Img;
	FILE* f = fopen(fn, "rb");
	if (f == NULL){	printf("%s NOT FOUND\n", fn);	exit(EXIT_FAILURE); }

	uch HeaderInfo[54];
	fread(HeaderInfo, sizeof(uch), 54, f); // read the 54-byte header
	// extract image height and width from header
	int width = *(int*)&HeaderInfo[18];			ip.Hpixels = width;
	int height = *(int*)&HeaderInfo[22];		ip.Vpixels = height;
	int RowBytes = (width * 3 + 3) & (~3);		ip.Hbytes = RowBytes;
	//save header for re-use
	memcpy(ip.HeaderInfo, HeaderInfo,54);
	printf("Input File name: %17s  (%u x %u)   File Size=%u\n", fn, 
			ip.Hpixels, ip.Vpixels, IMAGESIZE);
	// allocate memory to store the main image (1 Dimensional array)
	Img  = (uch *)malloc(IMAGESIZE);
	if (Img == NULL) return Img;      // Cannot allocate memory
	// read the image from disk
	fread(Img, sizeof(uch), IMAGESIZE, f);
	fclose(f);
	return Img;
}


// Write the 1D linear-memory stored image into file.
void WriteBMPlin(uch *Img, char* fn)
{
	FILE* f = fopen(fn, "wb");
	if (f == NULL){ printf("FILE CREATION ERROR: %s\n", fn); exit(1); }
	//write header
	fwrite(ip.HeaderInfo, sizeof(uch), 54, f);
	//write data
	fwrite(Img, sizeof(uch), IMAGESIZE, f);
	printf("Output File name: %17s  (%u x %u)   File Size=%u\n", fn, ip.Hpixels, ip.Vpixels, IMAGESIZE);
	fclose(f);
}


int main(int argc, char **argv)
{
	float			totalTime, tfrCPUtoGPU, tfrGPUtoCPU, kernelExecutionTime; // GPU code run times
	cudaError_t		cudaStatus, cudaStatus2;
	cudaEvent_t		time1, time2, time3, time4;
	ui				BlkPerRow, BlkPerRowInt, BlkPerRowInt2;
	ui				ThrPerBlk = 256, NumBlocks, NB2, NB4, NB8;
	cudaDeviceProp	GPUprop;
	ul				SupportedKBlocks, SupportedMBlocks, MaxThrPerBlk;
	ui				*imgCpyI, *imageI;
	char			SupportedBlocks[100];
	int				KernelNum=1, skipR, skipC;
	char			KernelName[255];

	if (argc != 5)
		printf("Incorrect call. Correct call:\n");
		printf("./filename in.bmp out.bmp xShrink yShrink\n");
		exit(EXIT_FAILURE);
	}
	
	image = ReadBMPlin(argv[1]);
	if (image == NULL){
		printf("Cannot allocate memory for the input image...\n");
		exit(EXIT_FAILURE);
	}
	skipC = atoi(argv[3]);
	skipR = atoi(argv[4]);
	
	NumBlocks = floor(ip.Vpixels/(NumThreads*skipR));
	imgCpy = (uch *)malloc(IMAGESIZE);
	if (imgCpy == NULL){
		free(image);
		printf("Cannot allocate memory for the input image...\n");
		exit(EXIT_FAILURE);
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
	SupportedKBlocks = (ui)GPUprop.maxGridSize[0] * (ui)GPUprop.maxGridSize[1] * (ui)GPUprop.maxGridSize[2] / 1024;
	SupportedMBlocks = SupportedKBlocks / 1024;
	sprintf(SupportedBlocks, "%u %c", (SupportedMBlocks >= 5) ? SupportedMBlocks : SupportedKBlocks, (SupportedMBlocks >= 5) ? 'M' : 'K');
	MaxThrPerBlk = (ui)GPUprop.maxThreadsPerBlock;

	cudaEventCreate(&time1);
	cudaEventCreate(&time2);
	cudaEventCreate(&time3);
	cudaEventCreate(&time4);

	cudaEventRecord(time1, 0);		// Time stamp at the start of the GPU transfer
	// Allocate GPU buffer for the input and output images
	cudaStatus = cudaMalloc((void**)&imageG, IMAGESIZE);
	cudaStatus2 = cudaMalloc((void**)&imgCpyG, IMAGESIZE/(skipC*skipR));
	if ((cudaStatus != cudaSuccess) || (cudaStatus2 != cudaSuccess)) {
		fprintf(stderr, "cudaMalloc failed! Can't allocate GPU memory\n");
		exit(EXIT_FAILURE);
	}
	// These are the same pointers as GPUCopyImg and GPUImg, however, casted to an integer pointer
	imgCpyI = (ui *)imgCpyG;
	imageI = (ui *)imageG;

	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(imageG, image, IMAGESIZE, cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy  CPU to GPU  failed!\n");
		exit(EXIT_FAILURE);
	}

	cudaEventRecord(time2, 0);
	
	shrinkG <<<NumBlocks, NumThreads>>> (imgCpyG, imageG, skipC, skipR);
	
	outG = imgCpyG;
	//GPUDataTransfer = 2 * IMAGESIZE;

	// cudaDeviceSynchronize waits for the kernel to finish, and returns
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching the kernel!\n", cudaStatus);
		exit(EXIT_FAILURE);
	}
	cudaEventRecord(time3, 0);

	// Copy output (results) from GPU buffer to host (CPU) memory.
	cudaStatus = cudaMemcpy(imgCpy, outG, IMAGESIZE, cudaMemcpyDeviceToHost);
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
	//checkError(cudaGetLastError());	// screen for errors in kernel launches
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Program failed after cudaDeviceSynchronize()!\n");
		free(image);
		free(imgCpy);
		exit(EXIT_FAILURE);
	}
	WriteBMPlin(imgCpy, OutputFileName);
	printf("--------------------------------------------------------------------------\n");
	printf("%s    ComputeCapab=%d.%d  [max %s blocks; %d thr/blk] \n",
		GPUprop.name, GPUprop.major, GPUprop.minor, SupportedBlocks, MaxThrPerBlk);
	printf("--------------------------------------------------------------------------\n");
	printf("%s %s %s %c %u %u  [%u BLOCKS, %u BLOCKS/ROW]\n", ProgName, InputFileName, OutputFileName, Flip, ThrPerBlk, KernelNum, NumBlocks, BlkPerRow);
	printf("--------------------------------------------------------------------------\n");
	printf("%s\n",KernelName);
	printf("--------------------------------------------------------------------------\n");
	printf("CPU->GPU Transfer   =%7.2f ms  ...  %4d MB  ...  %6.2f GB/s\n", tfrCPUtoGPU, DATAMB(IMAGESIZE), DATABW(IMAGESIZE, tfrCPUtoGPU));
	printf("Kernel Execution    =%7.2f ms  ...  %4d MB  ...  %6.2f GB/s\n", kernelExecutionTime, DATAMB(GPUDataTransfer), DATABW(GPUDataTransfer, kernelExecutionTime));
	printf("GPU->CPU Transfer   =%7.2f ms  ...  %4d MB  ...  %6.2f GB/s\n", tfrGPUtoCPU, DATAMB(IMAGESIZE), DATABW(IMAGESIZE, tfrGPUtoCPU)); 
	printf("--------------------------------------------------------------------------\n");
	printf("Total time elapsed  =%7.2f ms       %4d MB  ...  %6.2f GB/s\n", totalTime, DATAMB((2*IMAGESIZE+GPUDataTransfer)), DATABW((2 * IMAGESIZE + GPUDataTransfer), totalTime));
	printf("--------------------------------------------------------------------------\n\n");

	// Deallocate CPU, GPU memory and destroy events.
	cudaFree(imageG);
	cudaFree(imgCpyG);
	cudaEventDestroy(time1);
	cudaEventDestroy(time2);
	cudaEventDestroy(time3);
	cudaEventDestroy(time4);
	// cudaDeviceReset must be called before exiting in order for profiling and
	// tracing tools such as Parallel Nsight and Visual Profiler to show complete traces.
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		free(image);
		free(imgCpy);
		exit(EXIT_FAILURE);
	}
	free(image);
	free(imgCpy);
	return(EXIT_SUCCESS);
}



