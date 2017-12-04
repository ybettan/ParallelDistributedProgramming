//
// Created by kwarta on 04/12/2017.
//
#include <stdint.h>
#include <time.h>

/*
 *  A slow way to sum bits is to iterate over all the bits.
 *  The input most be uint32_t or unsigned, because arithmetic shift right has a sign extension.
 */
int slow_bit_count(unsigned n) {
    int res = 0;
    for (unsigned mask = 0x1;n ; n >>= 1) {
        res += n & mask;
    }
    return res;
}

/*
 *  A fast way to calculate sum of bits is to sum them ahead of time, and hard code it in to the program.
 *  B2 is the amount of beats in a 2 word bit, size is 4
 *  B4 is the amount of beats in a 4 word bit, size is 4*4 = 16.
 *  B6 is the amount of beats in a 6 word bit, size is 4*16 = 64.
 *  we will separate 32 bit words in to 4 words of 8 bits and sum them after looking them up in a predefined table.
 */
#define B2(n) n, n+1, n+1, n+2
#define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
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
int fast_bit_count(unsigned n) {
    return btc[(n & mask1) >> 24] + btc[(n & mask2) >> 16] + btc[(n & mask3) >> 8] + btc[(n & mask4)];
}

bool test_bit_counter(){
    unsigned last = 0xffffffff;

    // check the algorithm is right.
    for (unsigned i=0; i < last; ++i)
        assert(fast_bit_count(i)==slow_bit_count(n));

    assert(fast_bit_count(limit)==slow_bit_count(limit));

    clock_t start_t[2], end_t[2];
    // check the algorithm is actually faster. we should compare with a few more functions to actually get the best one
    // since these functions will be called a lot.
    start_t[0] = clock();
    for (unsigned i=0; i < last; ++i)
        fast_bit_count(i);
    end_t[0] = clock();

    start_t[1] = clock();
    for (unsigned i=0; i < last; ++i)
        slow_bit_count(i);
    end_t[1] = clock();

    assert( (end_t[0] - start_t[0]) < (end_t[1] - start_t[1]) );

    return true;
}

/*
 * we should use this line inside our functions.
 * The idea is to use an arithmetic calculation rather than a branch.
 */
int plus_or_minus_1_1(unsigned n) {
    return ((fast_bit_count(n) % 2) * 2) - 1;
}
int plus_or_minus_1_2(unsigned n) {
    return ((fast_bit_count(n) & 0x1) * 2) - 1;
}
// with a branch
int plus_or_minus_1_3(unsigned n) {
    return (fast_bit_count(n) & 0x1) ? 1 : - 1;
}