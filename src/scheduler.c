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

void execute(process_t* process, int clock) {
    static process_t* last = NULL;
    process->jobTime--;
    if (process!=last) {
        log_info("<Scheduler> process %d start executing, ETA: %d ticks", process->pid, process->jobTime);
        last = process;
    }
    log_trace("<Scheduler> process %d is running, ETA: %d ticks", process->pid, process->jobTime);
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
//    printf("---------loading---------------\n");
//    print_deque(suspended);
    return count;
}

void tick(int* clock) {
    *clock = *clock+1;
//    log_info("-------- t=%d --------", *clock);
}

int init(Deque* processes, Deque* pending, Deque* suspended) {
    while (deque_size(processes) > 0) {
        process_t *process = deque_pop(processes);
        deque_push(pending, process);
    }
    return 0;
};


int firstComeFirstServe(memory_allocator_t* allocator, Deque* processes, int* clock, int* finish) {
    Deque *suspended = new_deque((void (*)(void *)) printProcess);
    Deque *pending = new_deque((void (*)(void *)) printProcess);

    init(processes, pending, suspended);

    while (deque_size(suspended) > 0 || deque_size(pending) > 0) {
        load_process(pending, suspended, *clock);
        // continue if there is no process ready to run
        if (deque_size(suspended) == 0) {
            tick(clock);
            continue;
        }
        process_t* process = deque_pop(suspended);
        printf("%d, RUNNING, id=%d, remaining-time=%d\n", *clock, process->pid, process->jobTime);

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
                execute(process, *clock);
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
    Deque *suspended = new_deque((void (*)(void *)) printProcess);
    Deque *pending = new_deque((void (*)(void *)) printProcess);
    init(processes, pending, suspended);

    while (deque_size(pending) > 0 || deque_size(suspended) > 0) {
        load_process(pending, suspended, *clock);
        // continue if there is no process ready to run
        if (deque_size(suspended) > 0) {
            int quantumLeft = quantum;
            process_t* process = deque_pop(suspended);

            if (allocator->require_allocation(allocator->structure, process)) {
                allocator->allocate_memory(allocator->structure, process);
            }
            int page_fault_time = allocator->page_fault(allocator->structure, process) > 0;
            process->jobTime += page_fault_time;

            printf("%d, RUNNING, id=%d, remaining-time=%d\n", *clock, process->pid, process->jobTime);

            while (quantumLeft > 0 && process->jobTime > 0) {
                // Allocate space for the process if it's not in the memory
                if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                    allocator->load_memory(allocator->structure, process);
                }
                else {
                    execute(process, *clock);
                    allocator->use_memory(allocator->structure, process, *clock);
                    quantumLeft--;
                }
                tick(clock);
                load_process(pending, suspended, *clock);
            }
            if (process->jobTime > 0) {
                deque_insert(suspended, process);
            } else {
                allocator->free_memory(allocator->structure, process);
                finish_process(process, finish, *clock, deque_size(suspended));
            }
        }
        else {
            tick(clock);
        }
    }
}


int compare_job_time(void * a, void * b) {
    return ((process_t*)a)->jobTime - ((process_t*)b)->jobTime;
}

int compare_remaining_time(void * a, void * b) {
    return ((process_t*)a)->jobTime - ((process_t*)b)->jobTime;
}


int compare_PID(void * a, void * b) {
    return (((process_t *)a)->pid - ((process_t*)b)->pid);
}


int shortestRemainingTimeFirst(memory_allocator_t* allocator, Deque* processes, int* total, int* clock, int* finished) {
    heap_t *suspended = create_heap(*total, compare_job_time);
    Deque *pending = new_deque((void (*)(void *)) printProcess);
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
            process_t* process = NULL;
            process_t running = heap_pop_min(suspended);
            process = &running;
            log_debug("<Scheduler> Next process to execute is %d (%d ticks remaining)", process->pid, process->jobTime);
            if (allocator->require_allocation(allocator->structure, process)){
                void* allocation = allocator->allocate_memory(allocator->structure, process);
            }
//            int page_fault_time = allocator->page_fault(allocator->structure, process) > 0;
//            process->jobTime += page_fault_time;

            printf("%d, RUNNING, id=%d, remaining-time=%d\n", *clock, process->pid, process->jobTime);

            if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                allocator->load_memory(allocator->structure, process);
            } else {
                execute(process, *clock);
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
    char file_name[100];
    int scheduling_algorithm = -1;
    int memory_allocation = -1;
    int memory_size = -1;
    int quantum = 10;
    int total = 0;


    char opt ;
    while ((opt  = getopt (argc, argv, ":f:a:m:s:q:")) != -1) {
        switch (opt) {
            case 'f':
                strncpy(file_name, optarg, strlen(optarg));
                file_name[strlen(optarg)] = '\0';
                break;
            case 'a':
                if (strcasecmp(optarg, "ff") == 0) {
                    scheduling_algorithm = FIRST_COME_FIRST_SERVED;
                } else if (strcasecmp(optarg, "rr") == 0) {
                    scheduling_algorithm = ROUND_ROBIN;
                } else if (strcasecmp(optarg, "cs") == 0) {
                    scheduling_algorithm = CUSTOMISED_SCHEDULING;
                }
                break;
            case 'm':
                if (strcasecmp(optarg, "u") == 0) {
                    memory_allocation = UNLIMITED;
                } else if (strcasecmp(optarg, "p") == 0) {
                    memory_allocation = SWAPPING;
                } else if (strcasecmp(optarg, "v") == 0) {
                    memory_allocation = VIRTUAL_MEMORY;
                } else if (strcasecmp(optarg, "v") == 0) {
                    memory_allocation = CUSTOMISED_MEMORY;
                }
                break;
            case 's':
                memory_size = atoi(optarg);
                break;
            case 'q':
                quantum = atoi(optarg);
                break;
            default:
                abort();
        }
    }

    log_set_level(LOG_LEVEL);

    inspectArguments(file_name, scheduling_algorithm, memory_allocation, memory_size, quantum);

    /*
     * Create a memory allocator based on input argument
     */
    memory_allocator_t* allocator = NULL;
    if (memory_allocation == UNLIMITED) {
        allocator = create_unlimited_allocator();
    } else if (memory_allocation == SWAPPING) {
        allocator = create_swapping_allocator(memory_size, PAGE_SIZE);
    } else if (memory_allocation == VIRTUAL_MEMORY) {
        allocator = create_virtual_memory_allocator(memory_size, PAGE_SIZE);
    }

    Deque *processes = new_deque((void (*)(void *)) printProcess);
    int finished = 0;
    total = readProcessesFromFile(file_name, processes);
    int clock = 0;

    if (scheduling_algorithm == FIRST_COME_FIRST_SERVED) {
        firstComeFirstServe(allocator, processes, &clock, &finished);
    } else if (scheduling_algorithm == ROUND_ROBIN) {
        roundRobin(allocator, processes, &clock, &finished, quantum);
    } else if (scheduling_algorithm == CUSTOMISED_SCHEDULING) {
        shortestRemainingTimeFirst(allocator, processes, &total, &clock, &finished);
    }
    return 0;
};

int cmp_int (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

void print_memory(int* addresses, int count) {
    printf("[");
    qsort(addresses, count, sizeof(*addresses), cmp_int);
    for (int i=0; i<count; i++) {
        if (i == 0) {
            printf("%d",addresses[i]);
        }
        else {
            printf(",%d",addresses[i]);
        }
    }
    printf("]");
}

