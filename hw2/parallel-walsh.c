#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


#define GET_VAL(i, col) -(((__builtin_popcount((i) & (col)) & 1) << 1)-1)

#define L1_SIZE 32 << 12
#define L2_SIZE 256 << 12
#define CHUNK_SIZE (L1_SIZE>>1) >> 2



//FIXME: can we remove some includes?
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <smmintrin.h>

//FIXME: remove old one
#define OLD_GET_VAL(i, col) -(((set_bits_num((i) & (col)) & 1) << 1)-1)


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
//                           part A - Alon
//-----------------------------------------------------------------------------

//#define IS_EVEN_MASK  0x1
//#define GET_SIGN(n) ((((!__builtin_popcount((n))) & IS_EVEN_MASK) << 1) - 1)
//
//void simple_parallel_walsh(int* vec, int vecSize)
//{
//    int* initVec = (int*) malloc(vecSize * sizeof(int));
//#pragma omp parallel
//    {
//        register unsigned i;
//        /* copy the initial vector so each thread can calculate using it.
//         * reset the vec, so we can start writing in to it calculations. */
//#pragma omp for schedule(static)
//        for (i=0; i<(unsigned)vecSize; ++i) {
//            initVec[i] = vec[i];
//            vec[i] = 0;
//        }
//#pragma omp for schedule(static)
//        for (i=0; i<(unsigned)vecSize; ++i) {
//            register unsigned j, n;
//            for ( j=0; j<(unsigned)vecSize; ++j) {
//                n = j&i;
//                vec[i] += GET_SIGN(n) * initVec[j];
//            }
//        }
//    }
//    free (initVec);
//}

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

    // First half of the vector
    register int halfVecSize = vecSize >> 1;
    for(register int i=0;i<halfVecSize; i++){
        vec[i] =vec[i] + vec[i+halfVecSize];
    }

    // Second half of the vector
    for(register int i=halfVecSize;i<vecSize; i++){
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

void fast_parallel_walsh_vec_generator(register int* vec,
		register int vecSize, register int numOfThreads) {

	register int switchSize = vecSize / numOfThreads;
    #pragma omp parallel shared(vec, vecSize, switchSize)
	{
		register int currentVecSize;
		for (currentVecSize = vecSize; currentVecSize > switchSize; (currentVecSize >>= 1)){
			register int currentHalfVec = (currentVecSize >> 1);
			/* go over all parts */
			for (register int offset = 0; offset < vecSize; offset += currentVecSize){
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

/* FIXME : i think this need to be done with nested parallelizem */
void fast_parallel_walsh(register int *vec, register int vecSize) {

	register int numOfThreads = omp_get_max_threads();

    // Prepare the base vector for all the lower order WHT problems
    fast_parallel_walsh_vec_generator(vec, vecSize,numOfThreads);

    register int vecSizeOfThread=vecSize/numOfThreads;
    if (vecSizeOfThread==1) {
        return;
    }

    #pragma omp parallel
	{
        register int threadIdx = omp_get_thread_num();
        // Each thread calls to serial_fast_walsh with his private vector
        //	located inside "vec"
        serial_fast_walsh(vec + threadIdx *vecSizeOfThread,	vecSizeOfThread);
	}
}

