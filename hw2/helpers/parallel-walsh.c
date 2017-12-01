
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

/*
 * description:
 * 		Implements WHT Similar to FFT in a serial recursive way.
 *		Every call to the function is converting a WHT of size N
 *		into two WHT of size N/2
 */

void serial_fast_walsh(int register* vec, int register vecSize){

    // First half of the vector
    int register halfVecSize=vecSize/2;
    for(int register i=0;i<halfVecSize; i++){
        vec[i] =vec[i] + vec[i+halfVecSize];
    }

    // Second half of the vector
    for(int register i=halfVecSize;i<vecSize; i++){
        // vector[i-vecSize/2]old=vector[i-vecSize/2]new-vector[i]
    	// thats why we subtract vector[i] twice
    	vec[i] = ( vec[i-halfVecSize]-vec[i] ) - vec[i];
    }

    // Recursion stop - Nothing to do in vec size 1
    if(halfVecSize == 1) {
        return;
    }

    serial_fast_walsh(vec, halfVecSize);
    serial_fast_walsh(vec + (halfVecSize), halfVecSize);

}

/*
 * description:
 *      Prepare the base vector which will be the initial vector
 *      of every thread.
 *      After enough loops of the main for loop
 *      we will have a vector divided to numOfThreads smaller
 *      and equal size vectors, these smaller vectors will be the initial
 *      vector for a WHT/log(numOfThreads) size problem for each thread.
 *
 * return:
 *      the result vector is inserted in the same input vector - vec
 */

void fast_parallel_walsh_vec_generator(int register* vec,
		int register vecSize, int register numOfThreads) {

	int register switchSize = vecSize / numOfThreads;
#pragma omp parallel shared(vec, vecSize, switchSize)
	{
		int register currentVecSize;
		for (currentVecSize = vecSize; currentVecSize > switchSize; (currentVecSize >>= 1)){
			register int currentHalfVec = (currentVecSize >> 1);
			/* go over all parts */
			for (int register offset = 0; offset < vecSize; offset += currentVecSize){
				register int currentMiddleVec = offset + currentHalfVec;
				#pragma omp for schedule(guided)
				for (register int i = offset; i < currentMiddleVec; i++){
					/* a = vec[i]
					 * b = vec[halfPlusI]
					 * vec[i] = a + b
					 * vec[halfPlusI] = a - b */
					register int halfPlusI;

					halfPlusI = currentHalfVec + i;
					vec[i] += vec[halfPlusI];
					vec[halfPlusI] = vec[i] - (vec[halfPlusI] << 1);
				}
			}
		}
	}
}



/*
 * description:
 * 		The main function of fast_parallel_walsh
 * 		tasks:
 * 			1. call fast_parallel_walsh_vec_generator to prepare
 * 				the base vector that hold the initial vector of every
 * 				thread (implemented in parallel)
 * 			2. call serial_fast_walsh in parallel so every thread
 * 				will calculate his own vectors and WHT of a reduced
 * 				size problem.
 *
 * return:
 * 		the WHT transform of vector is inserted into the same input vector
 */

void fast_parallel_walsh(int register *vector, int register vecSize) {

	int register numOfThreads = omp_get_max_threads();

    // Prepare the base vector for all the lower order WHT problems
    fast_parallel_walsh_vec_generator(vector, vecSize,numOfThreads);

    int register vecSizeOfThread=vecSize/numOfThreads;
    if (vecSizeOfThread==1) {
        return;
    }

#pragma omp parallel
	{
        int register threadIdx = omp_get_thread_num();
        // Each thread calls to serial_fast_walsh with his private vector
        //	located inside "vector"
        serial_fast_walsh(vector + threadIdx *vecSizeOfThread,
        		vecSizeOfThread);
	}
}

// We use uint32_t because of shifting problems with signed
int set_bits_num(uint32_t register i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

int* generate_hadamard_matrix_column(int register size, int register col) {
	int register* whtColumn = (int*) malloc(size*sizeof(int));
    for(int register i=0; i<size; i++) {
    	whtColumn[i] = (set_bits_num(i & col) % 2 == 0) ? 1 : -1;
    }
    return whtColumn;
}

void simple_parallel_walsh(int register* vec, int register vecSize)
{
	// copy vector into main thread mem
    register int* lclCpyVec = (int*) malloc(vecSize*sizeof(int));
    memcpy(lclCpyVec, vec, vecSize*sizeof(int));

	#pragma omp parallel for schedule(dynamic)
    for( int register i=0;i<vecSize;i++) {
    	int register* whtColumn = generate_hadamard_matrix_column(vecSize, i);
    	int register mulSum = 0;
        for(int register j=0; j < vecSize; j++) {
            mulSum += whtColumn[j] * lclCpyVec[j];
        }
        vec[i] = mulSum;
        free(whtColumn);
    }
    free(lclCpyVec);
}
