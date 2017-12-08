#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


#define GET_VAL(i, col) -(((__builtin_popcount((i) & (col)) & 1) << 1)-1)

//FIXME : remove
#define VAR_IN_CACHE_LINE 16
#define CACHE_LINE_SIZE 64
#define CACHE_SIZE 32768
#define NEED_FETCHING(num) !((num) & 15)


//FIXME: can we remove some includes?
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <smmintrin.h>
#include <time.h>

//FIXME: remove old one
#define OLD_GET_VAL(i, col) -(((set_bits_num((i) & (col)) & 1) << 1)-1)
#define TEST 256


//-----------------------------------------------------------------------------
//                                part A
//-----------------------------------------------------------------------------


void simple_parallel_walsh(register int* vec, register int vecSize)
{
	// copy vector into main thread mem
    register int* vecCopy = (int*)malloc(vecSize*sizeof(int));
    memcpy(vecCopy, vec, vecSize*sizeof(int));

	#pragma omp parallel for schedule(dynamic)
    for (register int i=0 ; i<vecSize ; i++) {
    	register int sum = 0;
        for(register int j=0 ; j < vecSize ; j++) {
            sum += GET_VAL(i, j) * vecCopy[j];
        }
        vec[i] = sum;
    }
    free(vecCopy);
}


//-----------------------------------------------------------------------------
//                                part B
//-----------------------------------------------------------------------------

/*
 * description:
 * 		Implements WHT Similar to FFT in a serial recursive way.
 *		Every call to the function is converting a WHT of size N
 *		into two WHT of size N/2
 */
void serial_fast_walsh(register int* vec, register int vecSize){

    register int halfVecSize = vecSize >> 1;

    // First half of the vector
    for(register int i=0 ; i<halfVecSize ; i++){
        vec[i] =vec[i] + vec[i+halfVecSize];
        //if (NEED_FETCHING(i)) {
        //    __builtin_prefetch(vec + (i+CACHE_SIZE), 0);
        //}
        //__builtin_prefetch(vec + (i+TEST), 1);
    }
    // Second half of the vector
    for(register int i=halfVecSize ; i<vecSize ; i++){
    	vec[i] = vec[i-halfVecSize] - (vec[i] << 1) ;
        //if (NEED_FETCHING(i)) {
        //    __builtin_prefetch(vec + (i+CACHE_SIZE), 0);
        //}
        //__builtin_prefetch(vec + (i+TEST), 1);
    }
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
void fast_parallel_walsh_vec_generator(register int* vec, register int vecSize, 
        register int numOfThreads) {

    register int switchSize = vecSize / numOfThreads;
    register int currentVecSize;

    for (currentVecSize = vecSize; currentVecSize > switchSize; (currentVecSize >>= 1)){
        register int currentHalfVec = (currentVecSize >> 1);
        /* go over all parts */
        for (register int offset = 0; offset < vecSize; offset += currentVecSize){
            register int currentMiddleVec = offset + currentHalfVec;
                
            #pragma omp parallel for schedule(guided)
            for (register int i = offset; i < currentMiddleVec; i++){
                register int halfPlusI;
                halfPlusI = currentHalfVec + i;
                register int tmp = vec[i];
                vec[i] += vec[halfPlusI];
                vec[halfPlusI] = tmp - vec[halfPlusI];
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
void fast_parallel_walsh(register int *vec, register int vecSize) {

    // FIXME: to be removed
    //clock_t start, end;
    //start = clock();
    //end = clock();
    //printf("parallel serial part takes : %Lf\n", (long double)(end-start));    
                   
	register int numOfThreads = omp_get_max_threads();

    // Prepare the base vector for all the lower order WHT problems
    fast_parallel_walsh_vec_generator(vec, vecSize, numOfThreads);

    register int vecSizeOfThread = vecSize/numOfThreads;
    if (vecSizeOfThread==1) {
        return;
    }

    #pragma omp parallel
	{
        register int threadIdx = omp_get_thread_num();
        /* each thread nead  ~4300 cycles to compute all its L1 data 
         *                   ~2200 cycles to compute half of its L1 date */ 
        serial_fast_walsh(vec + threadIdx *vecSizeOfThread,	vecSizeOfThread);
	}
}


