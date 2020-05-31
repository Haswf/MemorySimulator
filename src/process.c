/**
 * Data structure to save process information
 * Created by Haswell on 18/05/2020.
 */
#include "process.h"

void output_finish(long long int clock, process_t* process, long long int proc_remaining) {
    printf("%lld, FINISHED, id=%lld, proc-remaining=%lld\n", clock, process->pid, proc_remaining);
}

/**
 * Create a process
 * @param timeArrived
 * @param pid
 * @param memory
 * @param job_time
 * @return
 */
process_t* create_process(long long int timeArrived, long long int pid, long long int memory, long long int job_time) {
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

/**
 * Log process inforamtion to stderr
 * @param process
 */
void log_process(process_t* process) {
    fprintf(stderr, "arrived: %lld\tpid: %lld\tmemory: %lld\tjobTime: %lld\n", process->timeArrived, process->pid, process->memory, process->remaining_time);
}

/**
 * Free a process
 * @param process
 */
void free_process(process_t* process) {
    if (!process) {
        free(process);
    }
}

/**
 * a wrapper of free process for dlist
 * @param process
 */
void dlist_free_process(void* process) {
    free(process);
}
