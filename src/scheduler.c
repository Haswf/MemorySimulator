//
// Created by Haswell on 18/05/2020.
//

#include "scheduler.h"
#include "unlimited.h"
#include "output.h"
#include "log.h"
//#include "log.c"
#include "memory_fragment.h"
#include "virtual_memory.h"
/**
 * memory page size
 */
#define PAGE_SIZE 4
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
    int count = 0;

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
        process_t* process = create_process(time, pid, memory, jobTime);
        deque_push(deque, process);
        count++;
    }

    fclose(fp);
    return count;
}
void run(process_t* process) {
    static process_t* last = NULL;
    process->jobTime--;
    if (process!=last) {
        log_info("<Scheduler> process %d start executing", process->pid);
        last = process;
    }
    log_trace("<Scheduler> process %d is running", process->pid);
}

void execute(process_t* process) {
    static process_t* last = NULL;
    process->jobTime--;
    if (process!=last) {
        log_info("<Scheduler> process %d start executing", process->pid);
        last = process;
    }
    log_trace("<Scheduler> process %d is running", process->pid);
}

int load_process(Deque* pending, Deque* suspended, int clock) {
    int count = 0;
    heap_t* toAdd = create_heap(MAX_PROCESS_ARRIVAL_PER_TICK, compare_PID);
    while (deque_size(pending) && next_to_pop(pending)->timeArrived == clock) {
        process_t* process = deque_pop(pending);
        assert(process->timeArrived == clock);
        heap_insert(toAdd, *process);
        free_process(process);
    }

    while (heap_size(toAdd) > 0) {
        process_t next_process = heap_pop_min(toAdd);
        log_info("<Scheduler> Process %d inserted to suspended queue", next_process.pid);
        process_t* next = create_process(next_process.timeArrived, next_process.pid, next_process.memory, next_process.jobTime);
        deque_insert(suspended, next);
        count++;
    }
    return count;
}

void tick(int* clock) {
    *clock = *clock+1;
    log_info("-------- t=%d --------", *clock);
}

int init(Deque* processes, Deque* pending, Deque* suspended) {
    while (deque_size(processes) > 0) {
        process_t *process = deque_pop(processes);
        if (process->timeArrived > 0) {
            deque_push(pending, process);
        } else {
            deque_push(suspended, process);
        }
    }
    return 0;
};


int firstComeFirstServe(memory_allocator_t* allocator, Deque* processes, int* clock, int* finish) {
    Deque *suspended = new_deque();
    Deque *pending = new_deque();

    init(processes, pending, suspended);

    while (deque_size(suspended) > 0 || deque_size(pending) > 0) {
        load_process(pending, suspended, *clock);
        // continue if there is no process ready to run
        if (deque_size(suspended) == 0) {
            tick(clock);
            continue;
        }
        process_t* process = deque_pop(suspended);

        if (allocator->require_allocation(allocator->structure, process)) {
            void* allocation = allocator->allocate_memory(allocator->structure, process);
            if (!allocation){
                log_warn("OOM for process %d", process->pid);
                finish_process(process, finish, *clock, deque_size(suspended));
                break;
            }
        }
        output_execute(*clock, process, (allocator->load_time_left(allocator->structure, process)));

        while (process->jobTime > 0) {
            if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                allocator->load_memory(allocator->structure, process);
            } else {
                execute(process);
                allocator->use_memory(allocator->structure, process, *clock);

            }
            load_process(pending, suspended, *clock);
            tick(clock);
        }
        /* A process that has 0 seconds left to run, should be "evicted" from memory before marking the process as
         * finished
         */
        allocator->free_memory(allocator->structure, process);
        finish_process(process, finish, *clock, deque_size(suspended));
    }
}

int roundRobin(memory_allocator_t* allocator, Deque* processes, int* clock, int* finish, int quantum) {
    Deque *suspended = new_deque();
    Deque *pending = new_deque();
    init(processes, pending, suspended);

    while (deque_size(suspended) > 0 || deque_size(pending) > 0) {
        load_process(pending, suspended, *clock);
        // continue if there is no process ready to run
        if (deque_size(suspended) == 0) {
            tick(clock);
            continue;
        }
        int quantumLeft = quantum;
        process_t* process = deque_pop(suspended);
        // Allocate space for the process if it's not in the memory
        if (allocator->require_allocation(allocator->structure, process)) {
            void* allocation = allocator->allocate_memory(allocator->structure, process);
            if (!allocation){
                log_warn("OOM for process %d", process->pid);
                finish_process(process, finish, *clock, deque_size(suspended));
                continue;
            }
        }

        while (quantumLeft > 0) {

            if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                allocator->load_memory(allocator->structure, process);
            }
            else {
                execute(process);
                allocator->use_memory(allocator->structure, process, *clock);
                quantumLeft--;
            }
            load_process(pending, suspended, *clock);
            tick(clock);
        }
        if (process->jobTime > 0) {
            deque_insert(suspended, process);
        } else {
            allocator->free_memory(allocator->structure, process);
            finish_process(process, finish, *clock, deque_size(suspended));
        }
    }
}


int compare_job_time(void * a, void * b) {
    return ((process_t*)a)->jobTime - ((process_t*)b)->jobTime;
}

int compare_PID(void * a, void * b) {
    return (((process_t *)a)->pid - ((process_t*)b)->pid);
}


int shortestRemainingTimeFirst(memory_allocator_t* allocator, Deque* processes, int* total, int* clock, int* finished) {
    heap_t *suspended = create_heap(*total, compare_job_time);
    Deque *pending = new_deque();
    while (deque_size(processes) > 0) {
        process_t *process = deque_pop(processes);
        if (process->timeArrived > 0) {
            deque_push(pending, process);
        } else {
            heap_insert(suspended, *process);
        }
    }

    while (heap_size(suspended) > 0 || deque_size(pending) > 0){
        // Add newly arrived processes
        heap_t* toAdd = create_heap(MAX_PROCESS_ARRIVAL_PER_TICK, compare_PID);
        while (deque_size(pending) && next_to_pop(pending)->timeArrived == *clock) {
            process_t* process = deque_pop(pending);
            assert(process->timeArrived == *clock);
            heap_insert(toAdd, *process);
            free_process(process);
            log_info("Process %d added to suspended", process->pid);
            log_process(process);
        }
        while (heap_size(toAdd) > 0) {
            process_t next = heap_pop_min(toAdd);
            heap_insert(suspended, next);
        }
        if (heap_size(suspended) > 0) {
//            heap_print(suspended, printProcess);
            process_t running = heap_pop_min(suspended);
            process_t* process = &running;
            log_debug("<Scheduler> Next process to execute is %d (%d ticks remaining)", process->pid, process->jobTime);
            if (allocator->require_allocation(allocator->structure, process)){
                void* allocation = allocator->allocate_memory(allocator->structure, process);
                if (!allocation){
                    log_warn("OOM for process %d", process->pid);
                    *finished += 1;
                    log_info("<Scheduler> Process %d skipped",process->pid);
                    continue;
                }
            }
            if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                allocator->load_memory(allocator->structure, process);
            } else {
                execute(process);
                allocator->use_memory(allocator->structure, process, *clock);
            }

            if (running.jobTime > 0) {
                heap_insert(suspended, running);
            } else {
                allocator->free_memory(allocator->structure, process);
                log_info("<Scheduler> Process %d finished",process->pid);
                *finished += 1;
            }
        }
        tick(clock);
    }
}

int main(int argc, char *argv[]) {
    char *fileName = NULL;
    int schedulingAlgorithm = -1;
    int memoryAllocation = -1;
    int memorySize = -1;
    int quantum = 10;
    int total = 0;
    log_set_level(LOG_LEVEL);
    int throughput[1000];
    for (int i=0; i<1000; i++) {
        throughput[i] = 0;
    }

    for (int i = 1; i < argc; i++) {
        parseFileName(&fileName, i, argc, argv);
        parseSchedulingAlgorithm(&schedulingAlgorithm, i, argc, argv);
        parseMemoryAllocation(&memoryAllocation, i, argc, argv);
        parseMemorySize(&memorySize, i, argc, argv);
        parseQuantum(&quantum, i, argc, argv);
    }
    inspectArguments(fileName, schedulingAlgorithm, memoryAllocation, memorySize, quantum);

    memory_allocator_t* allocator = NULL;
    if (memoryAllocation == UNLIMITED) {
        allocator = create_unlimited_allocator();
    } else if (memoryAllocation == SWAPPING) {
        allocator = create_swapping_allocator(memorySize, PAGE_SIZE);
    } else if (memoryAllocation == VIRTUAL_MEMORY) {
        allocator = create_virtual_memory_allocator(memorySize, PAGE_SIZE);
    }

    Deque *processes = new_deque();
    int finished = 0;
    total = readProcessesFromFile(fileName, processes);

    int clock = 0;

    if (schedulingAlgorithm == FIRST_COME_FIRST_SERVED) {
        firstComeFirstServe(allocator, processes, &clock, &finished);
    } else if (schedulingAlgorithm == ROUND_ROBIN) {
        roundRobin(allocator, processes, &clock, &finished, quantum);
    } else if (schedulingAlgorithm == CUSTOMISED_SCHEDULING) {
        shortestRemainingTimeFirst(allocator, processes, &total, &clock, &finished);
    }
    return 0;
};

int simulate() {

}


