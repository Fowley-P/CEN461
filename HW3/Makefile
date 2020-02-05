imshrink: ImageStuff.h ImageStuff.c imshrink.c
		  gcc -Wall imshrink.c ImageStuff.c -o imshrink -lm -lpthread
		  
imshrinkG: ImageStuff.h ImageStuff.c imshrinkG.cu
		   nvcc imshrinkG -o ImageStuff.c imshrinkG.cu -lm
		  
sort: sort.c
	  gcc -Wall sort.c -o sort -lpthread
	 
sortG: sortG.cu
	   nvcc sort -o sort.cu
