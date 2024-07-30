#pragma once

#include <sys/time.h>

#ifndef BUSY_WAIT
#define BUSY_WAIT

// calculate how many cycles can be executed in 1ms
void get_frequency();

// actual busy_wait func that manipulates the occupation of cpu;
// ms is the actual time of waiting
void busy_wait(unsigned int ms);

#endif
