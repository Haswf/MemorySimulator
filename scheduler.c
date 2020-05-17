//
// Created by Haswell on 18/05/2020.
//

#include "scheduler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"

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

int readProcessesFromFile(char* fileName) {
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
        printProcess(process);
    }

    fclose(fp);
    return 0;
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
    printf("%s\n", fileName);
    readProcessesFromFile(fileName);
    return 0;
}


