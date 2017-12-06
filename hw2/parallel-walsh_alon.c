#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/*
 * bit table counter has 64*4 = 256 entries, each 8b -> size 2kb = 0.25 KB = 256 B
 * masks and table must be outside of the function or else they will be put in the stack each time, meaning defined and
 * calculated each time the function is called.
 */
uint8_t btc[] = {B6(0), B6(1), B6(1), B6(2) };
uint32_t mask1 = 0xff000000;
uint32_t mask2 = 0x00ff0000;
uint32_t mask3 = 0x0000ff00;
uint32_t mask4 = 0x000000ff;
#define IS_EVEN_MASK  0x1
// calculate the amount of beats in n.
#define BIT_COUNT(n) (btc[((n) & mask1) >> 24] + btc[((n) & mask2) >> 16] + btc[((n) & mask3) >> 8] + btc[((n) & mask4)])
/* calculate if the matrix entry is -1 or +1.
 * 1. count the amount of beats in n. if it is evan the lsb is 0, odd lsb is 1.
 * 2. turn the lsb --> !lsb
 * 3. get only the lsb --> now we have odd = 0, even = 1.
 * 4. arithmetic shift left 1, multiply by 2 --> odd = 0, even = 2.
 * 5. -1 --> odd = -1, even = 1.
 */
#define GET_SIGN(n) ((((!BIT_COUNT((n))) & IS_EVEN_MASK) << 1) - 1)

void simple_parallel_walsh(int* vec, int vecSize)
{
    int* initVec = (int*) malloc(vecSize * sizeof(int));
#pragma omp parallel
    {
        register unsigned i;
        /* copy the initial vector so each thread can calculate using it.
         * reset the vec, so we can start writing in to it calculations. */
#pragma omp for schedule(static)
        for (i=0; i<vecSize; ++i) {
            initVec[i] = vec[i];
            vec[i] = 0;
        }
#pragma omp for schedule(static)
        for (i=0; i<vecSize; ++i) {
            register unsigned j, n;
            for ( j=0; j<vecSize; ++j) {
                n = j&i;
                vec[i] += GET_SIGN(n) * initVec[j];
            }
        }
    }
    free (initVec);
}

void simple_parallel_walsh(int* vec, int vecSize)
{
    int* initVec = (int*) malloc(vecSize * sizeof(int));
#pragma omp parallel
    {
        register unsigned i;
        /* copy the initial vector so each thread can calculate using it.
         * reset the vec, so we can start writing in to it calculations. */
#pragma omp for schedule(static)
        for ( i=0; i<vecSize; ++i) {
            initVec[i] = vec[i];
        }
#pragma omp for schedule(static)
        for ( i=0; i<vecSize; ++i) {
            register unsigned* column = (unsigned*) malloc(sizeof(unsigned) * vecSize);
            register unsigned n;
            register unsigned j;
            for (j=0; j<vecSize; ++j) {
                n = i&j;
                column[j] = GET_SIGN(n);
            }

            for (j=0; j<vecSize; ++j)
                vec[i] += initVec[j]*colunn[j];

            free (column);
        }

    }
}
