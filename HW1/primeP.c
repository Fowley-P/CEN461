//Patrick Fowley

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<math.h>
#include<stdlib.h>
#include<ctype.h>
#include<time.h>
#include<unistd.h>

#define BILLION 1000000000L
#define REPS 1
#define MAXTHREADS 128

pthread_mutex_t lock;

pthread_t ThHandle[MAXTHREADS];
pthread_attr_t ThAttr;
int ThParam[MAXTHREADS];
unsigned long long int hold[8192];				//holds all the numbers to be compared
unsigned long long int primeI[8192];				//holds the index at which each prime is located
unsigned long long int primes[8192];				//holds the numbers determined to be primes
long numT;

/*
 *checks to see if a number is prime
 *if it is, puts the number into primes[] and its index into primeI[]
 *@param: thread ID (*tid)
 */
void *threadFunc(void *tid)
{
	long tl = *((int*)tid);					//need an int
	tl *= 8192/numT;					//uses the int to determine the starting point for each thread
	long chunk = tl+(8192/numT)-1;				//determines the highest number to check
	int i,j,flag;
	for(i = tl; i <= chunk; i++) {				//loops through the available numbers to check for primes
		flag = 0;
		for (j = 2; j <= sqrt(hold[i]); j++) {		//if a divisor hasn't been found by sqrt(num), there is no divisor
			if ((hold[i] % j) == 0) {		//if a divisor is found, throw a flag and then break the loop
				flag = 1;
				break;
			}
		}
		if (flag == 0) {				//need to lock down the arrays, otherwise race conditions
			pthread_mutex_lock(&lock);		//those are bad, mmkay
			primeI[i] = i;
			primes[i] = hold[i];
			pthread_mutex_unlock(&lock);
		}
	}
	pthread_exit(NULL);
}

int main(int argc,char* argv[])
{
	int i, ThErr;
   
	struct timespec start, end;				//Need to time things
   
	FILE *fr;FILE *fw;					//fr == file read, fw == file write
   
	if(argc != 3 && argc != 4)				//Input sanitization saves lives
	{
		printf("Error! Incorrect command line arguments\n");
		printf("Usage is: ./file input output #threads\n");
		printf("If no thread # specified, default is 4\n");
		exit(1);
	}
   
	if ((fr = fopen(argv[1], "r")) == NULL)			//if the 2 files can't be opened, something went wrong
	{
		printf("Error opening read file\n");
		exit(2);
	}
   
	if ((fw = fopen(argv[2], "w")) == NULL)
	{
		printf("Error opening write file\n");
		exit(3);
	}
	
	if (argc == 4)						//the optional 4th argument provides the number of threads to run
		numT = atoi(argv[3]);
	else
		numT = 4;					//default number of threads is 4
		
	if(numT <= 0) {
		printf("Using default number of threads\n");
		numT = 4;
	}
   
	for(i = 0; i < 8192; i++)				//reads the numbers to check for primes
	{
		fscanf(fr,"%llu", &hold[i]);
	}
   
	clock_gettime(CLOCK_MONOTONIC, &start);			//Ladies and gents, start your engines
   
	if(numT > 0) { 						//can't have 0 or fewer threads
		pthread_attr_init(&ThAttr);
		pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
		for(i=0; i<numT; i++) {				//loops to create threads
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, threadFunc, (void *)&ThParam[i]);
			if(ThErr != 0){				//ThErr == 0 on successful thread creation
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		for(i=0; i<numT; i++) {
			pthread_join(ThHandle[i], NULL);
		}
	}
   
	clock_gettime(CLOCK_MONOTONIC, &end);			//Game over, man
	unsigned long long int diff = (end.tv_sec - start.tv_sec);
	printf("%llu ns\n", diff);
   
	//Ok so technically it should have been a while loop with a break statement
	//but I was young and naive.
	for(i = 0; i < 8192; i++) {
		if(primeI[i] != 0)
		{
			fprintf(fw, "%llu : %llu\n", primeI[i] + 1, primes[i]);
		}
	}
   
	fclose(fr);
	fclose(fw);
  
	return 0;
}
