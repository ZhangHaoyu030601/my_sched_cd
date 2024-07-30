#pragma once

#include "excstate.h"

#define EXECUTIVE_QUANT 100

typedef struct
{
    // tasks
    pthread_t pchild;
    excstate excstate;

    int index;

    excstate task_in_execution;
} frame_descriptor;

void executive_init();
