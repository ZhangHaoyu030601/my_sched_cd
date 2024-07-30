#include "busy_wait.h"
#include <stddef.h>
#include <limits.h>
#include <stdio.h>

// the 'infinite' loop
void loop(unsigned int num_of_cycles)
{
    volatile unsigned int dummy;
    for (unsigned int i = 0; i < num_of_cycles; ++i)
    {
        dummy = i; // useless loops
    }
}

unsigned int frequency_ms = 0;

void get_frequency()
{
    struct timeval start;
    struct timeval end;
    const unsigned int num_of_loops = INT_MAX;

    gettimeofday(&start, NULL);
    loop(num_of_loops);
    gettimeofday(&end, NULL);

    frequency_ms = (double)num_of_loops * 1000 / ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
}

void busy_wait(unsigned int time_ms)
{
    loop(time_ms * frequency_ms);
}

#ifdef TEST_BW
int main()
{
    get_frequency();
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    busy_wait(5);
    gettimeofday(&end, NULL);
    printf("%ld", (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
    return 0;
}
#endif
