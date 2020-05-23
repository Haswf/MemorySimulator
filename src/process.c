//
// Created by Haswell on 18/05/2020.
//
#include "process.h"

void output_finish(int clock, process_t* process, int proc_remaining) {
    printf("%d, FINISHED, id=%d, proc-remaining=%d\n", clock, process->pid, proc_remaining);
}

process_t* create_process(int timeArrived, int pid, int memory, int job_time) {
    process_t* newProcess = (process_t*)malloc(sizeof(*newProcess));
    if (newProcess == NULL) {
        return NULL;
    }
    newProcess->timeArrived = timeArrived;
    newProcess->pid = pid;
    newProcess->memory = memory;
    newProcess->job_time = job_time;
    newProcess->remaining_time = job_time;
    return newProcess;
};

void printProcess(process_t* process) {
    fprintf(stderr, "arrived: %d\tpid: %d\tmemory: %d\tjobTime: %d\n", process->timeArrived, process->pid, process->memory, process->remaining_time);
}

void log_process(process_t* process) {
    log_trace("arrived: %d\tpid: %d\tmemory: %d\tjobTime: %d\n", process->timeArrived, process->pid, process->memory, process->remaining_time);
}

void free_process(process_t* process) {
    if (!process) {
        free(process);
    }
}

void dlist_free_process(void* process) {
    free(process);
}

#include "process.h"
