//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_PROCESS_H
#define COMP30023_2020_PROJECT_2_PROCESS_H
#include "log.h"
typedef struct process {
    int timeArrived;
    int pid;
    int memory;
    int jobTime;
} process_t;

process_t* create_process(int timeArrived, int pid, int memory, int jobTime);

void printProcess(process_t* process);

void freeProcess(process_t* process);

void dlist_free_process(void* process);

void finish_process(process_t* process, int* finish, int clock);
#endif //COMP30023_2020_PROJECT_2_PROCESS_H
