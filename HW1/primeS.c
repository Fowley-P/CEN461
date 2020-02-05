#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

int main (int argc, char* argv[])
{
	int line = 0;
	//unsigned long long int num = 0;
	unsigned long long int count;
	//size_t pass = 0;
	double timer;
	int mark = 0;
	bool isPrime;
	unsigned long long int hold[8192];
	unsigned long long int primes[8192];
	int primeI[8192];
	//char *buf = NULL;
	clock_t start, stop;
	
	FILE *fr;
	FILE *fw;
	
	if (argc != 3) {
		printf("Incorrect call.\n");
		printf("Usage is: ./name input.txt output.txt.\n");
		return -1;
	}
	
	if ((fr = fopen(argv[1], "r")) == NULL) {
		printf("Read error\n");
		return -1;
	}
	if ((fw = fopen(argv[2], "w")) == NULL) {
		printf("Write error\n");
		return -1;
	}
	
	start = clock();
	while(fscanf(fr, "%llu", &hold[line]) != -1) {
		line += 1;
	}

	for(line = 0; line < 8192; line++) {
		isPrime = true;
		for (count = 2; count < sqrt(hold[line]); count++) {
			if (hold[line]%count == 0) {
				isPrime = false;
				count = hold[line];
				}
		}
		if (isPrime) {
			primes[mark] = hold[line];
			primeI[mark] = line;
			mark++;
		}
	}
	
	stop = clock();
	timer = (double)(stop-start)/(double)CLOCKS_PER_SEC;
	
	for(line = 0; line < mark; line++)
	{
		fprintf(fw, "%d : %llu\n", (primeI[line]+1), primes[line]);
	}
	
	printf("Execution time: %.2fns\n", timer);
	
	fclose(fr);
	fclose(fw);

	return 0;
}
	
