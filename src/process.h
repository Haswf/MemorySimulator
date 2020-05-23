//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_PROCESS_H
#define COMP30023_2020_PROJECT_2_PROCESS_H
#include "log.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct process {
    int timeArrived;
    int pid;
    int memory;
    int remaining_time;
    int job_time;
    int finish_time;
} process_t;


void output_finish(int clock, process_t* process, int proc_remaining);

process_t* create_process(int timeArrived, int pid, int memory, int jobTime);

void printProcess(process_t* process);

void free_process(process_t* process);

void dlist_free_process(void* process);

void log_process(process_t* process);

#endif //COMP30023_2020_PROJECT_2_PROCESS_H
