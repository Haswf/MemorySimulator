//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_PROCESS_H
#define COMP30023_2020_PROJECT_2_PROCESS_H
#include <stdlib.h>
#include <stdio.h>
#include "stdbool.h"

typedef struct process {
    long long int timeArrived;
    long long int pid;
    long long int memory;
    long long int remaining_time;
    long long int job_time;
    long long int finish_time;
} process_t;


void output_finish(long long int clock, process_t* process, long long int proc_remaining);

process_t* create_process(long long int timeArrived, long long int pid, long long int memory, long long int jobTime);

void printProcess(process_t* process);

void free_process(process_t* process);

void dlist_free_process(void* process);

void log_process(process_t* process);

#endif //COMP30023_2020_PROJECT_2_PROCESS_H
