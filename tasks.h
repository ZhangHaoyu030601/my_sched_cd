#pragma once
#include <stdbool.h>

#define FRAME_SIZE (HYPER_PERIOD / NUM_OF_FRAMES)

typedef void (*task_routine)();

extern task_routine P_TASKS[];

extern task_routine AP_TASK;

extern const unsigned int NUM_P_TASKS;

extern const unsigned int H_PERIOD;

extern const unsigned int NUM_FRAMES;

extern int *schedule[];

extern int SLACK[];

extern int AP_WCET;
extern int AP_DEADLINE;

void task_init();

void task_deconstructor();

/* 请求ap_task，如果任务被接受，则返回 true*/
bool ap_task_request();
