//
// Created by Haswell on 18/05/2020.
//
#include "process.h"
#include <stdlib.h>
#include <stdio.h>

process_t* create_process(int timeArrived, int pid, int memory, int jobTime) {
    process_t* newProcess = (process_t*)malloc(sizeof(*newProcess));
    if (newProcess == NULL) {
        return NULL;
    }
    newProcess->timeArrived = timeArrived;
    newProcess->pid = pid;
    newProcess->memory = memory;
    newProcess->jobTime = jobTime;
    return newProcess;
};

void printProcess(process_t* process) {
    fprintf(stderr, "arrived: %d\tpid: %d\tmemory: %d\tjobTime: %d\n", process->timeArrived, process->pid, process->memory, process->jobTime);
}

void freeProcess(process_t* process) {
    if (process == NULL) {
        return;
    }
    free(process);
}

#include "process.h"