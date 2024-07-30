#include "tasks.h"
#include "executive.h"
#include "excstate.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>

// absolute time useful to jump in the next frame
struct timespec abstime;
// how long to wait for the next frame
long long wait_time;

#ifdef SLACK_STEALING
struct timespec sp_abstime;
long long wait_s_time;
#endif

static pthread_cond_t efb_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t efb_mutex = PTHREAD_MUTEX_INITIALIZER;

frame_descriptor *frame_des;

excstate executive_frame_index;

frame_descriptor executive_ap_frame_des;

// how many frames to wait before testing the deadline
excstate executive_ap_count_frame;

frame_descriptor executive_exec_des;

/**
 * Check the deadline for a frame
 * @param index to understand which frame
 */
void executive_check_deadline_frame(int index)
{
    printf("[FRAME %d] [DEADLINE CHECK]\n", index);
    int state = excstate_get_state(&frame_des[index].excstate);
    if (state == WORKING || state == PENDING)
    {
        int task = excstate_get_state(&frame_des[index].task_in_execution);
        printf("[FRAME %d] [DEADLINE MISS] [TASK %d]!\n", index, task);
        exit(1);
    }
}

/**
 * Test if the aporadic thread can respect the deadline.
 * Compute how many frames the executive has to wait before testing the sp task deadline (executive_sp_count_frame)
 * @param frame_index to understand where the aporadic thread starts(which frame)
 */
bool acceptance_test(int frame_index)
{
    int index = frame_index;
    index++;
    int quant = H_PERIOD / NUM_FRAMES;
    int qi = index * quant;
    int y = 1;
    int i = 0;
    int count_slack = 0;
    while ((i = qi + (y * quant)) <= (qi + AP_DEADLINE))
    {
        int slack = SLACK[(index + y - 1) % NUM_FRAMES];
        count_slack += slack;
        y++;
    }
    printf("[AP] [WAIT] %d frame\n", --y);
    // set how many frames to wait for check deadline
    excstate_set_state(&executive_ap_count_frame, y);
    // return static test of the deadline
    return (count_slack >= AP_WCET ? true : false);
}

/**
 * Check any deadline miss of ap_task if deadline arrives.
 */
void executive_ap_check_deadline()
{
    // update counter of frame to wait
    int state = excstate_get_state(&executive_ap_frame_des.excstate);

    if (state == WORKING || state == COMPLETED)
    {
        // get which frame we are executing
        int frames = excstate_get_state(&executive_ap_count_frame);
        // set how many frames to wait before test deadline of ap task
        excstate_set_state(&executive_ap_count_frame, --frames);
        printf("[AP] [WAIT] [%d FRAMES]\n", excstate_get_state(&executive_ap_count_frame));

        // test if execute should check deadline of ap task
        if (frames < 0)
        {
            printf("[AP] [DEADLINE CHECK]\n");
            if (state == WORKING || state == PENDING)
            {
                printf("[AP] [ERROR] AP not respect deadline!\n");
                exit(1);
            }

            // set state IDLE
            excstate_set_state(&executive_ap_frame_des.excstate, IDLE);
        }
    }
}

/**
 * Try to schedule an aporadic task
 */
bool ap_task_request()
{
    // Check if another aporadic thread is already in execution
    if (excstate_get_state(&executive_ap_frame_des.excstate) != IDLE)
    {
        printf("[AP] [ALREADY SCHEDULED] can't schedule new sporadic job\n");
        return false;
    }

    int index = excstate_get_state(&executive_frame_index);

    // acceptance test
    printf("[AP] [CHECK AP REQ] frame %d\n", index);
    if (!acceptance_test(index))
    {
        printf("[AP] [ERROR] Abort this AP task\n");
        exit(1);
    }

    printf("[AP] [TASK already SCHEDULED]\n");

    excstate_set_state(&executive_ap_frame_des.excstate, READY);
    return true;
}

/**
 * AP task handler
 */
void *ap_task_handler()
{
    while (1)
    {
        if (excstate_get_state(&executive_ap_frame_des.excstate) == WORKING)
        {
            excstate_set_state(&executive_ap_frame_des.excstate, COMPLETED);
        }

        excstate_wait_running(&executive_ap_frame_des.excstate);
        excstate_set_state(&executive_ap_frame_des.excstate, WORKING);

        printf("[AP] [EXEC AP TASK]\n");
        AP_TASK();
        printf("[AP] [END]\n");
    }
    return NULL;
}

void executive_init_frame()
{
    float quant = H_PERIOD / NUM_FRAMES;
    wait_time = quant * EXECUTIVE_QUANT * 1000000;
    printf("[INIT TIME] [QUANT %f] [EXECUTIVE QUANT %d] [NANOSEC FRAME %lld]\n", quant, EXECUTIVE_QUANT, wait_time);
}

/**
 * Frame handler
 * @param arg frame index
 */
void *frame_handler(void *arg)
{
    int index = *(int *)arg;

    while (1)
    {
        excstate_set_state(&frame_des[index].excstate, IDLE);

        excstate_wait_running(&frame_des[index].excstate);
        excstate_set_state(&frame_des[index].excstate, WORKING);
        printf("[FRAME %d] [WAKE UP]\n", index);

        // execute task list
        int task_index = 0, i = 0;
        while ((task_index = schedule[index][i]) != -1)
        {
            printf("[FRAME %d] [EXECUTE %d] [START]\n", index, task_index);

            excstate_set_state(&frame_des[index].task_in_execution, task_index);
            // execute the task
            P_TASKS[task_index]();

            printf("[FRAME %d] [EXECUTE %d] [END]\n", index, task_index);
            // next task
            i++;
        }
    }

    return NULL;
}

/**
 * Executive handler
 */
void *executive()
{
    struct timeval utime;
    gettimeofday(&utime, NULL);

    abstime.tv_sec = utime.tv_sec;
    abstime.tv_nsec = utime.tv_usec * 1000;

    int index = 0;
    while (1)
    {
        printf("\n------[FRAME %d]-------\n", index);

        // compute next timeout
        abstime.tv_sec += (abstime.tv_nsec + wait_time) / 1000000000;
        abstime.tv_nsec = (abstime.tv_nsec + wait_time) % 1000000000;

#ifdef SLACK_STEALING
        // if ap thread is not IDLE
        if (excstate_get_state(&executive_ap_frame_des.excstate) != IDLE)
        {
            // compute next timeout: end of slack time available (nsec)
            wait_s_time = SLACK[index] * EXECUTIVE_QUANT * 1000000;
            // set HIGH priority for sp thread
            struct sched_param param;
            param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 2;
            pthread_setschedparam(executive_sp_frame_desc.pchild, SCHED_FIFO, &param);
            // printf("[AP] [SET PRIORITY %d] [HIGH]\n", param.sched_priority);
            // endif
        }
#endif

        // wakeup next frame
        printf("[FRAME %d] [PENDING]!\n", index);
        excstate_set_state(&frame_des[index].excstate, PENDING);

        // wakeup sporadic thread
        if (excstate_get_state(&executive_ap_frame_des.excstate) == READY)
        {
            printf("[AP] [PENDING] ap thread!\n");
            excstate_set_state(&executive_ap_frame_des.excstate, PENDING);
        }

#ifdef SLACK_STEALING
        // if ap thread is WORKING
        int state = excstate_get_state(&executive_ap_frame_des.excstate);
        if (state != IDLE)
        {
            // wait slack time
            ap_abstime.tv_sec += (abstime.tv_nsec + slacktime2wait) / 1000000000;
            ap_abstime.tv_nsec = (abstime.tv_nsec + slacktime2wait) % 1000000000;
            pthread_cond_timedwait(&efb_cond, &efb_mutex, &sp_abstime);
            // set LOW priority for ap thread
            struct sched_param param;
            param.sched_priority = sched_get_priority_max(SCHED_FIFO) - NUM_FRAMES - 3;
            pthread_setschedparam(executive_ap_frame_desc.pchild, SCHED_FIFO, &param);
            // printf("[AP] [SET PRIORITY %d] [LOW]\n", param.sched_priority);
            // endif
        }
#endif

        // wait frame finish to compute
        pthread_cond_timedwait(&efb_cond, &efb_mutex, &abstime);
        printf("------[END OF FRAME]------ wait %lld nsec - next %lld sec %li nsec\n\n", wait_time, (long long)abstime.tv_sec, abstime.tv_nsec);

        // check deadline of the frame
        executive_check_deadline_frame(index);
        // check deadline of the SP task!
        executive_ap_check_deadline();

        // next frame
        index++;
        index = index % NUM_FRAMES;

        // set frame_index
        excstate_set_state(&executive_frame_index, index);
    }
}

/**
 * Set priority attribute
 * @param attr attribute
 * @param priority which priority have to set
 */
void executive_new_pthread_attr(pthread_attr_t *attr, int priority)
{
    // attr init
    if (pthread_attr_init(attr))
    {
        perror("error init attr\n");
        exit(1);
    }

    // set scheduler FIFO
    if (pthread_attr_setschedpolicy(attr, SCHED_FIFO))
    {
        perror("error init attr - set policy\n");
        exit(1);
    }

    // set inherit scheduler
    pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);

    // set priority
    struct sched_param p;
    p.sched_priority = priority;
    // set scheduler Sched Param
    if (pthread_attr_setschedparam(attr, &p))
    {
        perror("error init attr - set param\n");
        exit(1);
    }
}

/**
 * Executive initalization
 */
void executive_init()
{
    // task init
    task_init();

    // init frame
    executive_init_frame();

    // init state
    excstate_init(&executive_frame_index, IDLE);
    excstate_init(&executive_ap_count_frame, IDLE);

    // init tasks descriptor
    frame_des = (frame_descriptor *)malloc(sizeof(frame_descriptor) * NUM_FRAMES);

    int max_priority = sched_get_priority_max(SCHED_FIFO);

    // [INIT PTHREADS]
    int i = 1;
    for (i = 0; i < NUM_FRAMES; i++)
    {
        // [new task descriptor]
        frame_descriptor *fd = &frame_des[i];

        // [init excstate]
        excstate_init(&fd->excstate, IDLE);

        // [set attr]
        pthread_attr_t attr;

        int frame_priority = max_priority - 3;

#ifndef FRAME_HANDLER_SAME_PRIORITY
        frame_priority -= i;
#endif

        executive_new_pthread_attr(&attr, frame_priority);

        printf("[FRAME %d] [TASK INIT] priority %d\n", i, frame_priority);

        // set index
        fd->index = i;

        // [create task]
        pthread_create(&fd->pchild, &attr, frame_handler, &fd->index);
    }

    // [INIT APORADIC THREAD]

    // init aporadic thread state
    excstate_init(&executive_ap_frame_des.excstate, IDLE);

    // set attr thread ap task handler
    printf("[AP] [TASK INIT] priority %d\n", max_priority - i - 3);
    pthread_attr_t attr_ap;
    executive_new_pthread_attr(&attr_ap, max_priority - i - 3);

    // create task
    pthread_create(&executive_ap_frame_des.pchild, &attr_ap, &ap_task_handler, NULL);

    // [INIT EXECUTIVE THREAD]

    // set attr thread executive task handler
    printf("[EXECUTIVE] [TASK INIT] priority %d\n", max_priority - 1);
    pthread_attr_t attr_exec;
    executive_new_pthread_attr(&attr_exec, max_priority - 1);

    // create task
    pthread_create(&executive_exec_des.pchild, &attr_exec, &executive, NULL);

    // wait executive thread
    pthread_join(executive_exec_des.pchild, NULL);

    // wait thread
    /*for(i=0; i<NUM_P_TASKS; i++){
      frame_descriptor fd = frame_descs[i];
      pthread_join( fd.pchild, NULL );
    }*/
}
