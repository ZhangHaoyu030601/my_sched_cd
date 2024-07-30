#pragma once
#include <stddef.h>

#define TT_PERIOD 1000 /*us*/

struct tt_task_period
{
	const char *task_comm;
	const unsigned int period;
};

struct tt_task_period task_period_cpu2[4] = {
	{
		.task_comm = "Task1",
		.period = 9,
	},
	{
		.task_comm = "Task2",
		.period = 9,
	},
	{
		.task_comm = "Task3",
		.period = 12,
	},
	{
		.task_comm = "Task4",
		.period = 18,
	},
};

struct sched_tt_slot
{
	const char *task_comm;
	const unsigned int slot_seq;
	const double start_time;
	const double stop_time;
};

struct sched_tt_slot tt_table_cpu2[13] = {
	{
		.task_comm = "Task3",
		.slot_seq = 1,
		.start_time = 0,
		.stop_time = 2,
	},
	{
		.task_comm = "Task1",
		.slot_seq = 2,
		.start_time = 2,
		.stop_time = 3.5,
	},
	{
		.task_comm = "Task2",
		.slot_seq = 3,
		.start_time = 5,
		.stop_time = 7,
	},
	{
		.task_comm = "Task4",
		.slot_seq = 4,
		.start_time = 7,
		.stop_time = 12,
	},
	{
		.task_comm = "Task1",
		.slot_seq = 5,
		.start_time = 12,
		.stop_time = 13.5,
	},
	{
		.task_comm = "Task2",
		.slot_seq = 6,
		.start_time = 15,
		.stop_time = 17,
	},
	{
		.task_comm = "Task3",
		.slot_seq = 7,
		.start_time = 17,
		.stop_time = 19,
	},
	{
		.task_comm = "Task2",
		.slot_seq = 8,
		.start_time = 19,
		.stop_time = 21,
	},
	{
		.task_comm = "Task1",
		.slot_seq = 9,
		.start_time = 21,
		.stop_time = 22.5,
	},
	{
		.task_comm = "Task3",
		.slot_seq = 10,
		.start_time = 24,
		.stop_time = 26,
	},
	{
		.task_comm = "Task4",
		.slot_seq = 11,
		.start_time = 26,
		.stop_time = 31,
	},
	{
		.task_comm = "Task1",
		.slot_seq = 12,
		.start_time = 31,
		.stop_time = 32.5,
	},
	{
		.task_comm = "Task2",
		.slot_seq = 13,
		.start_time = 34,
		.stop_time = 36,
	},
};

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct sched_tt_table
{
	const char *task_comm;
	const unsigned int pinned_cpu;
	const unsigned int hyper_period;
	const tt_task_period *task_period;
	const unsigned int task_num;
	const sched_tt_slot *slot_table;
	const unsigned int slot_num;
};

struct sched_tt_table global_tt_table[4] = {
	{
		.pinned_cpu = 0,
		.hyper_period = 0,
		.task_period = NULL,
		.task_num = 0,
		.slot_table = NULL,
		.slot_num = 0,
	},
	{
		.pinned_cpu = 1,
		.hyper_period = 0,
		.task_period = NULL,
		.task_num = 0,
		.slot_table = NULL,
		.slot_num = 0,
	},
	{
		.pinned_cpu = 2,
		.hyper_period = 36,
		.task_period = task_period_cpu2,
		.task_num = ARRAY_SIZE(task_period_cpu2),
		.slot_table = tt_table_cpu2,
		.slot_num = ARRAY_SIZE(tt_table_cpu2),
	},
	{
		.pinned_cpu = 3,
		.hyper_period = 0,
		.task_period = NULL,
		.task_num = 0,
		.slot_table = NULL,
		.slot_num = 0,
	},
};
