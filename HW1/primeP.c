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
unsigned long long int hold[8192];
unsigned long long int primeI[8192];
unsigned long long int primes[8192];
long numT;

void *threadFunc(void *tid)
{
	long tl = *((int*)tid);
	tl *= 8192/numT;
	long chunk = tl+(8192/numT)-1;
	int i,j,flag;
	for(i = tl; i <= chunk; i++) {
		flag = 0;
		for (j = 2; j <= sqrt(hold[i]); j++) {
			if ((hold[i] % j) == 0) {
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			pthread_mutex_lock(&lock);
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
   
	struct timespec start, end;
   
	FILE *fr;FILE *fw;
   
	if(argc != 3 && argc != 4)
	{
		printf("Error! Incorrect command line arguments\n");
		printf("Usage is: ./file input output #threads\n");
		printf("If no thread # specified, default is 4\n");
		exit(1);
	}
   
	if ((fr = fopen(argv[1], "r")) == NULL)
	{
		printf("Error opening read file\n");
		exit(2);
	}
   
	if ((fw = fopen(argv[2], "w")) == NULL)
	{
		printf("Error opening write file\n");
		exit(3);
	}
	
	if (argc == 4)
		numT = atoi(argv[3]);
	else
		numT = 4;
		
	if(numT <= 0) {
		printf("Using default number of threads\n");
		numT = 4;
	}
   
	for(i = 0; i < 8192; i++)
	{
		fscanf(fr,"%llu", &hold[i]);
	}
   
	clock_gettime(CLOCK_MONOTONIC, &start);
   
	if(numT > 0) { 
		pthread_attr_init(&ThAttr);
		pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
		for(i=0; i<numT; i++) {
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, threadFunc, (void *)&ThParam[i]);
			if(ThErr != 0){
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		for(i=0; i<numT; i++) {
			pthread_join(ThHandle[i], NULL);
		}
	}
   
	clock_gettime(CLOCK_MONOTONIC, &end);
	unsigned long long int diff = (end.tv_sec - start.tv_sec);
	printf("%llu ns\n", diff);
   
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
