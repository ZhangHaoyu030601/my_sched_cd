#include "tasks.h"
#include "busy_wait.h"
#include "executive.h"

#include <stdio.h>
#include <stdlib.h>

#define HYPER_PERIOD 36
#define NUM_OF_TASKS 5
#define NUM_OF_FRAMES 3

const unsigned int H_PERIOD = HYPER_PERIOD;
const unsigned int NUM_FRAMES = NUM_OF_FRAMES;
const unsigned int NUM_P_TASKS = NUM_OF_TASKS;

void task1()
{
    printf("Doing task 1\n");
    busy_wait(EXECUTIVE_QUANT * 1);
    printf("task 1 done\n");
}
void task2()
{
    printf("Doing task 2\n");
    busy_wait(EXECUTIVE_QUANT * 1.5);
    printf("task 2 done\n");
}
void task3()
{
    printf("Doing task 3\n");
    busy_wait(EXECUTIVE_QUANT * 1.5);
    printf("task 3 done\n");
}
void task4()
{
    printf("Doing task 4\n");
    busy_wait(EXECUTIVE_QUANT * 4);
    printf("task 4 done\n");
}

void ap_task_code()
{
    printf("Doing ap task\n");
    // busy_wait(5);
    printf("ap task done\n");
}

// ! 只是样例实现，实际不应调用busy_wait()！
void idle_task()
{
    printf("IDLE!\n");
    busy_wait(EXECUTIVE_QUANT * 0.5); // ! 实际不应调用该函数
    printf("NOT IDLE!\n");
}

int *schedule[NUM_OF_FRAMES];
int SLACK[NUM_OF_FRAMES];

task_routine P_TASKS[NUM_OF_TASKS];
task_routine AP_TASK;

int AP_WCET;
int AP_DEADLINE;

void task_init()
{
    P_TASKS[0] = task1;
    P_TASKS[1] = task2;
    P_TASKS[2] = task3;
    P_TASKS[3] = task4;
    P_TASKS[4] = idle_task;

    AP_TASK = ap_task_code;

    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        schedule[i] = (int *)malloc(sizeof(int) * (FRAME_SIZE + 1));
    }
    // for (int slot = 0; slot < H_PERIOD; ++slot)
    // {
    //     // 分配schedule[frame][slot]
    // }
    // ! -1 表示结束
    schedule[0] = (int *)malloc(sizeof(int) * 6);
    schedule[0][0] = 2;
    schedule[0][1] = 0;
    schedule[0][2] = 4;
    schedule[0][3] = 1;
    schedule[0][4] = 3;
    schedule[0][5] = -1;

    SLACK[0] = 0;

    schedule[1] = (int *)malloc(sizeof(int) * 7);
    schedule[1][0] = 0;
    schedule[1][1] = 4;
    schedule[1][2] = 1;
    schedule[1][3] = 2;
    schedule[1][4] = 1;
    schedule[1][5] = 0;
    schedule[1][6] = -1;

    SLACK[1] = 1;

    schedule[2] = (int *)malloc(sizeof(int) * 7);

    schedule[2][0] = 2;
    schedule[2][1] = 3;
    schedule[2][2] = 0;
    schedule[2][3] = 4;
    schedule[2][4] = 1;
    schedule[2][5] = -1;

    SLACK[2] = 0;

    // 样例
    AP_WCET = 1;
    AP_DEADLINE = 10;

    get_frequency();
}

void task_deconstructor()
{
    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        free(schedule[i]);
    }
}
