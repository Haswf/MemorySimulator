//
// Created by Haswell on 18/05/2020.
//
#include "process.h"
#include <stdlib.h>
#include <stdio.h>

void output_execute(int clock, process_t* process, int loading_time) {
    printf("%d, RUNNING, id=%d, remaining-time=%d\n", clock, process->pid, process->jobTime);
}

void output_finish(int clock, process_t* process, int proc_remaining) {
    printf("%d, FINISHED, id=%d, proc-remaining=%d\n", clock, process->pid, proc_remaining);
}

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

void log_process(process_t* process) {
    log_trace("arrived: %d\tpid: %d\tmemory: %d\tjobTime: %d\n", process->timeArrived, process->pid, process->memory, process->jobTime);
}

void free_process(process_t* process) {
    if (!process) {
        free(process);
    }
}

void dlist_free_process(void* process) {
    free(process);
}

void finish_process(process_t* process, int* finish, int clock, int proc_remaining) {
    log_info("<Scheduler> Process %d finished",process->pid);
    output_finish(clock, process, proc_remaining);
    *finish += 1;
    free_process(process);
}

#include "process.h"
