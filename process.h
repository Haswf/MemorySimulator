//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_PROCESS_H
#define COMP30023_2020_PROJECT_2_PROCESS_H

typedef struct process {
    int timeArrived;
    int pid;
    int memory;
    int jobTime;
} process_t;

process_t* Process(int timeArrived, int pid, int memory, int jobTime);

void printProcess(process_t* process);

#endif //COMP30023_2020_PROJECT_2_PROCESS_H
