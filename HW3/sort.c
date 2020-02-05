#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MEGA 1024*1024
#define NUM_THREADS 4
#define TWOBILLS 2000000000

int ThParam[NUM_THREADS];
pthread_t ThHandle[NUM_THREADS];
pthread_attr_t ThAttr;
pthread_mutex_t lock;

FILE *fw;
long num;
long count = 0;
int chunk;
int *arri;
int *arris;
float *arrf;
float *arrfs;
double *arrd;
double *arrds;
long *arrl;
long *arrls;


void *iBub(void *tid) {
	long tp = *((int *) tid);
	tp = (chunk*tp) + count;
	long i = tp, j = tp;
	int hold;
	for (; i<tp+chunk;i++) {
		for(; j<tp+chunk-(i-tp);j++) {
			if (arri[j] < arri[j+1]) {
				pthread_mutex_lock(&lock);
				hold = arri[j];
				arri[j] = arri[j+1];
				arri[j+1] = hold;
				pthread_mutex_unlock(&lock);
			}
		}
	}
	pthread_exit(NULL);
}

void *fBub(void *tid) {
	long tp = *((int *) tid);
	tp = (chunk*tp) + count;
	long i = tp, j = tp;
	float hold;
	for (; i<tp+chunk;i++) {
		for(; j<tp+chunk-(i-tp);j++) {
			if (arrf[j] < arrf[j+1]) {
				pthread_mutex_lock(&lock);
				hold = arrf[j];
				arrf[j] = arrf[j+1];
				arrf[j+1] = hold;
				pthread_mutex_unlock(&lock);
			}
		}
	}
	pthread_exit(NULL);
}

void *dBub(void *tid) {
	long tp = *((int *) tid);
	tp = (chunk*tp) + count;
	long i = tp, j = tp;
	double hold;
	for (; i<tp+chunk;i++) {
		for(; j<tp+chunk-(i-tp);j++) {
			if (arrd[j] < arrd[j+1]) {
				pthread_mutex_lock(&lock);
				hold = arrd[j];
				arrd[j] = arrd[j+1];
				arrd[j+1] = hold;
				pthread_mutex_unlock(&lock);
			}
		}
	}
	pthread_exit(NULL);
}

void *lBub(void *tid) {
	long tp = *((int *) tid);
	tp = (chunk*tp) + count;
	long i = tp, j = tp;
	long hold;
	for (; i<tp+chunk;i++) {
		for(; j<tp+chunk-(i-tp);j++) {
			if (arrl[j] < arrl[j+1]) {
				pthread_mutex_lock(&lock);
				hold = arrl[j];
				arrl[j] = arrl[j+1];
				arrl[j+1] = hold;
				pthread_mutex_unlock(&lock);
			}
		}
	}
	pthread_exit(NULL);
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
	int ThErr;
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
	pthread_attr_init(&ThAttr);
	pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
	while (count < num) {
		for(i=0; i<NUM_THREADS; i++){
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, iBub, (void *)&ThParam[i]);
			if(ThErr != 0){
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		for(i=0; i<NUM_THREADS; i++){
			pthread_join(ThHandle[i], NULL);
		}
		pthread_mutex_lock(&lock);
		count += chunk*NUM_THREADS;
		pthread_mutex_unlock(&lock);
	}
	printf("%ld\n", count);
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
	int ThErr;
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
	pthread_attr_init(&ThAttr);
	pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
	while (count < num) {
		for(i=0; i<NUM_THREADS; i++){
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, fBub, (void *)&ThParam[i]);
			if(ThErr != 0){
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		for(i=0; i<NUM_THREADS; i++){
			pthread_join(ThHandle[i], NULL);
		}
		
		pthread_mutex_lock(&lock);
		count += chunk*NUM_THREADS;
		pthread_mutex_unlock(&lock);
	}
	printf("%ld\n", count);
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
	int ThErr;
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
	pthread_attr_init(&ThAttr);
	pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
	while (count < num) {
		for(i=0; i<NUM_THREADS; i++){
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, dBub, (void *)&ThParam[i]);
			if(ThErr != 0){
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		for(i=0; i<NUM_THREADS; i++){
			pthread_join(ThHandle[i], NULL);
		}
		
		pthread_mutex_lock(&lock);
		count += chunk*NUM_THREADS;
		pthread_mutex_unlock(&lock);
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
	int ThErr;
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
	pthread_attr_init(&ThAttr);
	pthread_attr_setdetachstate(&ThAttr, PTHREAD_CREATE_JOINABLE);
	while (count < num) {
		for(i=0; i<NUM_THREADS; i++){
			ThParam[i] = i;
			ThErr = pthread_create(&ThHandle[i], &ThAttr, lBub, (void *)&ThParam[i]);
			if(ThErr != 0){
				printf("\nThread Creation Error %d. Exiting abruptly... \n",ThErr);
				exit(EXIT_FAILURE);
			}
		}
		for(i=0; i<NUM_THREADS; i++){
			pthread_join(ThHandle[i], NULL);
		}
		
		pthread_mutex_lock(&lock);
		count += chunk*NUM_THREADS;
		pthread_mutex_unlock(&lock);
	}
	count = 0;
	lMer();
	
	memcpy(arrls, arrl, num*sizeof(long));
	fw = fopen("outl.txt", "w");
	for (i = 0; i<num; i++) {
		fprintf(fw, "%ld\n", arrls[i]);
	}
	fclose(fw);
	free (arrl);
	free(arrls);
	return;
}
		
int main (int argc, char** argv) {
	char choice;
		
	if (argc != 4) {
		printf("Incorrect call. Correct call:\n");
		printf("./filename num type chunk\n");
		return 1;
	}
	
	choice = toupper(*argv[2]);
	chunk = atoi(argv[3]);
	num = (atoi(argv[1])*MEGA);
	if (num%chunk!=0) {
		printf("Please choose a number that is evenly divisible by your #chunks.\n");
		return 1;
	}
	
	switch (choice) {
		case 'I': genI(num); break;
		case 'F': genF(num); break;
		case 'D': genD(num); break;
		case 'L': genL(num); break;
		default: printf("Please revise your function call. Accepted inputs are:\n");
				 printf("d, i, l, f, D, I, L, F.\n");
				 return 1;
	}
	
	return(EXIT_SUCCESS);
}

