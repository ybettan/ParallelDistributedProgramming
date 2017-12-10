#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>


#define GET_VAL(i, col) -(((__builtin_popcount((i) & (col)) & 1) << 1)-1)



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

/* The base logic for each thread to handle.
 * This is the normal WHT fast transform on a size that can be handle by 
 * a sigle thread */
void fast_serial_walsh(register int* vec, register int vecSize){

    register int devidedVecSize = vecSize >> 1;

    for(register int i=0 ; i<devidedVecSize ; i++) {
        register int tmp = vec[i];
        vec[i] += vec[i+devidedVecSize];
    	vec[i+devidedVecSize] = tmp - vec[i+devidedVecSize];
        //if (NEED_FETCHING(i)) {
        //    __builtin_prefetch(vec + (i+CACHE_SIZE), 0);
        //}
        //__builtin_prefetch(vec + (i+TEST), 1);
    }
    /* by putting the stop condition here we gain 1 uneeded recursive call */
    if(devidedVecSize == 1) {
        return;
    }

    fast_serial_walsh(vec, devidedVecSize);
    fast_serial_walsh(vec + devidedVecSize, devidedVecSize);
}

/* do exactly what the serial version is doing, but stop when the current
 * vector size has reach a size that can be handle by a single thread when
 * using maximum utilization of our threads resource*/ 
void reorgenize_input_vector(register int* vec, register int vecSize, 
        register int numOfThreads) {

    register int minVecSize = vecSize / numOfThreads;
    for (register int currVecSize=vecSize ; currVecSize>minVecSize ; 
            currVecSize >>= 1) {

        register int currDevidedVecSize = currVecSize >> 1;
        for (register int diff=0 ; diff<vecSize ; diff+=currVecSize){

            register int currDevidedVec = diff + currDevidedVecSize;
            #pragma omp parallel for schedule(guided)
            for (register int i = diff; i < currDevidedVec; i++){

                register int index = currDevidedVecSize + i;
                register int tmp = vec[i];
                vec[i] += vec[index];
                vec[index] = tmp - vec[index];
            }
	    }
	}
}


void fast_parallel_walsh(register int *vec, register int vecSize) {

	register int numOfThreads = omp_get_max_threads();
    register int minSize = vecSize/numOfThreads;
    if (minSize==1) {
        return;
    }

    /* reorgenize the vector until we reach a size that fit a single thread */
    reorgenize_input_vector(vec, vecSize, numOfThreads);

    #pragma omp parallel
	{
        /* give each thread a part of the vector to compute */
        register int id = omp_get_thread_num();
        fast_serial_walsh(vec + id*minSize,	minSize);
	}
}


