//
// Created by Haswell on 18/05/2020.
//

#include "scheduler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"
#include "deque.h"
#include <assert.h>

void parseFileName(char** file, int index, int argc, char *argv[]) {
    if (strstr(argv[index], "-f") && index+1 < argc) {
        *file = argv[index+1];
    }
}

int parseSchedulingAlgorithm(int* scheduler, int index, int argc, char *argv[]) {
    if (strstr(argv[index], "-a") && index+1 < argc) {
        if (strncasecmp(argv[index+1], "ff", 2) == 0) {
            *scheduler = FIRST_COME_FIRST_SERVED;
        } else if (strncasecmp(argv[index+1], "rr", 2) == 0) {
            *scheduler = ROUND_ROBIN;
        } else if (strncasecmp(argv[index+1], "cs", 2) == 0) {
            *scheduler = CUSTOMISED_SCHEDULING;
        } else {
            printf("invalid argument value %s\n", argv[index+1]);
            exit(EXIT_FAILURE);
        }
    }
};

void parseMemoryAllocation(int* memoryAllocator, int index, int argc, char *argv[]) {
    if (strstr(argv[index], "-m") && index+1 < argc) {
        if (strncasecmp(argv[index+1], "u", 1) == 0) {
            *memoryAllocator = UNLIMITED;
        } else if (strncasecmp(argv[index+1], "p", 1) == 0) {
            *memoryAllocator = SWAPPING;
        } else if (strncasecmp(argv[index+1], "v", 1) == 0) {
            *memoryAllocator = VIRTUAL_MEMORY;
        } else if (strncasecmp(argv[index+1], "cm", 2) == 0) {
            *memoryAllocator = CUSTOMISED_MEMORY;
        } else {
            printf("invalid argument value %s\n", argv[index+1]);
            exit(EXIT_FAILURE);
        }
    }
}

void parseMemorySize(int* memory, int index, int argc, char* argv[]) {
    if (strstr(argv[index], "-s") && index+1 < argc) {
        *memory = atoi(argv[index+1]);
    }
}

void parseQuantum(int* quantum, int index, int argc, char* argv[]) {
    if (strstr(argv[index], "-q") && index+1 < argc) {
        *quantum = atoi(argv[index+1]);
    }
}

void inspectArguments(char* fileName, int schedulingAlgorithm, int memoryAllocation, int memorySize, int quantum) {
    printf("Filename: %s\n", fileName);
    printf("Scheduling Algorithm: %d\n", schedulingAlgorithm);
    printf("Memory Allocation: %d\n", memoryAllocation);
    printf("Memory Size: %d\n", memorySize);
    printf("quantum: %d\n", quantum);
}

int readProcessesFromFile(char* fileName, Deque* deque) {
    FILE *fp;

    fp = fopen(fileName, "r"); // read mode

    if (fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    int time = -1;
    int pid =-1;
    int memory = -1;
    int jobTime = -1;

    while((fscanf(fp, "%d %d %d %d\n", &time, &pid, &memory, &jobTime)) != EOF){
        process_t* process = Process(time, pid, memory, jobTime);
        deque_push(deque, process);
    }

    fclose(fp);
    return 0;
}

void execute(process_t* process, int clock) {
    process->jobTime--;
    fprintf(stderr, "%d\t: pid: %d is running\n", clock, process->pid);
}

int cmpProcess (const void* a, const void* b) {
    return ((process_t*)a)->pid - ((process_t*)b)->pid;
}

int loadNewProcess(Deque* pending, Deque* suspended, int clock) {
    int count = 0;
    Deque* buffer = new_deque();

    while (next_to_pop(pending) && next_to_pop(pending)->timeArrived == clock) {
        process_t* process = deque_pop(pending);
        deque_push(buffer, process);
        assert(process->timeArrived == clock);
        fprintf(stderr, "%d\t: Process %d added to suspended\n", clock, process->pid);
        count++;
    }

    // Sort processes arrived at the same time by its pid
    process_t toAdd[count];
    for (int i=0; i<count; i++) {
        process_t* temp = deque_pop(buffer);
        toAdd[i] = *temp;
        freeProcess(temp);
    }
    qsort(toAdd, count, sizeof(process_t), cmpProcess);
    for (int i=0; i<count; i++) {
        printProcess(&toAdd[i]);
        deque_insert(suspended, memcpy(malloc(sizeof(*toAdd)), &toAdd[i], sizeof(*toAdd)));
    }
    return count;
}

void tick(int* clock) {
    *clock = *clock+1;
}

int firstComeFirstServe(Deque* pending, Deque* suspended, int* clock) {
    process_t* process = deque_pop(suspended);
    while (process->jobTime > 0) {
        loadNewProcess(pending, suspended, *clock);
        execute(process, *clock);
        tick(clock);
    }
    freeProcess(process);
}

int roundRobin(Deque* pending, Deque* suspended, int* clock, int quantum) {
    process_t* process = deque_pop(suspended);
    int quantumLeft = quantum;
    while (quantumLeft > 0) {
        loadNewProcess(pending, suspended, *clock);
        execute(process, *clock);
        tick(clock);
        quantumLeft--;
    }
    if (process->jobTime > 0) {
        deque_insert(suspended, process);
    } else {
        freeProcess(process);
    }
}


int main(int argc, char *argv[]) {
    char* fileName;
    int schedulingAlgorithm;
    int memoryAllocation;
    int memorySize;
    int quantum = 10;

    printf("%d\n", argc);
    for (int i=1; i<argc; i++) {
        parseFileName(&fileName, i, argc, argv);
        parseSchedulingAlgorithm(&schedulingAlgorithm, i, argc, argv);
        parseMemoryAllocation(&memoryAllocation, i, argc, argv);
        parseMemorySize(&memorySize, i, argc, argv);
        parseQuantum(&quantum, i, argc, argv);
    }
    inspectArguments(fileName, schedulingAlgorithm, memoryAllocation, memorySize, quantum);
    Deque* processes = new_deque();
    Deque* suspended = new_deque();
    Deque* pending = new_deque();

    readProcessesFromFile(fileName, processes);

    while (deque_size(processes) > 0) {
        process_t* process = deque_pop(processes);
        if (process->timeArrived > 0) {
            deque_push(pending, process);
        } else {
            deque_push(suspended, process);
        }
    }

    int clock = 0;


    while (deque_size(suspended) > 0 || deque_size(pending) > 0) {
        if (schedulingAlgorithm == FIRST_COME_FIRST_SERVED) {
            firstComeFirstServe(pending, suspended, &clock);
        } else if (schedulingAlgorithm == ROUND_ROBIN){
            roundRobin(pending, suspended, &clock, quantum);
        }
    }

    return 0;
}

