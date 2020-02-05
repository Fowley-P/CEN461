#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
 

#define NUM_AMT 8192 
#define MAX_NUM_LEN 15	
#define DEF_NUM_THR 4	
#define REPS 1	
#define MAX_THREADS 128	


long  buffer[NUM_AMT], primes[NUM_AMT];			
	  
size_t primeIndices[NUM_AMT], CreateNum, ChunkSize, NumChunks;


void *MTfindPrimes(void*);		
void findPrimes();				
int isPrime(long);				




int main(int argc, char *argv[])
{
	
	FILE *fr,	
		 *fw;	
	
	char *line = NULL;
		 
	size_t i, a, cnt, len, read, readI, writeI, numT, numC, incI, ThParam[MAX_THREADS];
	cnt = 0;
	len = 0;
	readI = 0;
		   
	double start, end;
	
	struct timeval t;
	
	pthread_t ThHandle[MAX_THREADS];
	
	pthread_attr_t ThAttr;

	if(argc < 4) {
		perror("Need 4 arguments. Example: ./prime numbers.txt output.txt numThreads chunkSize\n");
		return 1;
	}
	
	numT = atoi(argv[3]);
	ChunkSize = atoi(argv[4]);
	
	
	if((ChunkSize * numT) > NUM_AMT) {
		perror("Bad numbers given.\n");
		return 1;
	}
		
	if((fr = fopen(argv[1], "r")) == NULL) {
		perror("Unable to open input file.");
		return 1;
	}
	
	if((fw = fopen(argv[2], "w")) == NULL) {
		perror("Unable to open output file.");
		return 1;
	}
	
	while((read = getline(&line, &len, fr)) != -1) {
		buffer[readI]=atol(line);
		
		readI++;
	}
	
	numC = NumChunks/numT;
	incI = numT*ChunkSize;
	CreateNum=0;
	
	gettimeofday(&t, NULL);
	start = (double)t.tv_sec*1e6 + ((double)t.tv_usec);
	
	if (numT==1) {
		for(a=0; a < numC; a++) {
			findPrimes();
			
			CreateNum+=incI;
		}
	}
	
	else {

		pthread_attr_init(&ThAttr);
		pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
		
		for(a=0; a < numC; a++) {
			for(i=0; i < numT; i++) {
				ThParam[i] = i;
				
				if(pthread_create(&ThHandle[i], &ThAttr, MTfindPrimes, (void *)&ThParam[i]) != 0) {
					perror("Error Creating Threads!");
					return 1;
				}
				
			}

			for(i=0; i<numT; i++)
				pthread_join(ThHandle[i], NULL);
			
			CreateNum+=incI;
		}	
	}
	
	gettimeofday(&t, NULL);
	end = (double)t.tv_sec*1e6 + ((double)t.tv_usec);

	for(writeI=0; writeI<NUM_AMT; writeI++) {
		if(primeIndices[writeI] != 0) {
			if(cnt!=0)
				fputs("\r\n",fw);

			fprintf(fw,"%zu : %ld",primeIndices[writeI],primes[writeI]);
			cnt++;
		}
	}

	printf("%zu,%zu,%0.03f\n", numT, ChunkSize, (end-start)/1e6/REPS);
	
	fclose(fr);
	fclose(fw);
	
	return 0;
}

//multi-threaded version of primality tester
void *MTfindPrimes(void* tid)
{
	/* start = id*chunkSize; */
	/* stop = start+chunkSize; loop while < stop */
	
	long currNumber;	//current integer to test for primality
	
	//determine boundaries
	size_t start = (*((size_t *)tid))*ChunkSize+CreateNum,
		   stop = start+ChunkSize,
		   execIndex;	//execution index
	
	//loop through all numbers read from file
	for(execIndex=start; execIndex<stop; execIndex++)
	{
		//obtain current number from buffer
		currNumber = buffer[execIndex];
		
		//test for primality
		if(isPrime(currNumber))
		{
			//if prime, record the number and file line (starting from 1)
			primes[execIndex]=currNumber;
			primeIndices[execIndex]=execIndex+1;
		}

	}
	
	//exit the thread
	pthread_exit(NULL);
}

//serial version of primality tester
void findPrimes()
{
	size_t execIndex;	//execution index
	
	long currNumber;	//current integer to test for primality
	
	//loop through all numbers read from file
	for(execIndex=CreateNum; execIndex<CreateNum+ChunkSize; execIndex++)
	{
		//obtain current number from buffer
		currNumber = buffer[execIndex];
		
		//test for primality
		if(isPrime(currNumber))
		{
			//if prime, record the number and file line (starting from 1)
			primes[execIndex]=currNumber;
			primeIndices[execIndex]=execIndex+1;
		}

	}
}

//checks if an integer is prime
int isPrime(long number)
{
	long i;	//index for primality test
	
	//1 is prime
	if(number == 1)
		return 1;
	
	
	//check if any number between 2 and sqrt(number) evenly divides number
	for(i = 2; i*i<=number; i++){
		 
		 //if so, number is not prime
		 if(number % i == 0){
			 return 0;
		 }
		 
		 
	}
		
	//if we are out here, number is prime
	return 1;
}
