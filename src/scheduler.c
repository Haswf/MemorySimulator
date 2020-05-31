//
// Created by Haswell on 18/05/2020.
//

#include "scheduler.h"



void inspectArguments(char* fileName, long long int schedulingAlgorithm, long long int memoryAllocation, long long int memorySize, long long int quantum) {
    printf("Filename: %s\n", fileName);
    printf("Scheduling Algorithm: %lld\n", schedulingAlgorithm);
    printf("Memory Allocation: %lld\n", memoryAllocation);
    printf("Memory Size: %lld\n", memorySize);
    printf("quantum: %lld\n", quantum);
}

long long int readProcessesFromFile(char* fileName, Deque* deque) {
    FILE *fp;
    long long int count = 0;

    fp = fopen(fileName, "r"); // read mode

    if (fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    long long int time = -1;
    long long int pid =-1;
    long long int memory = -1;
    long long int jobTime = -1;

    while((fscanf(fp, "%lld %lld %lld %lld\n", &time, &pid, &memory, &jobTime)) != EOF){
        process_t* process = create_process(time, pid, memory, jobTime);
        deque_push(deque, process);
        count++;
    }

    fclose(fp);
    return count;
}

void execute(process_t* process, long long int clock) {
    static process_t* last = NULL;
    process->remaining_time--;
    if (process!=last) {
        log_info("<Scheduler> process %d start executing, ETA: %d ticks", process->pid, process->remaining_time);
        last = process;
    }
    log_trace("<Scheduler> process %d is running, ETA: %d ticks", process->pid, process->remaining_time);
}

long long int load_process(Deque* pending, Deque* suspended, long long int clock) {
    long long int count = 0;
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
        process_t* next = create_process(next_process.timeArrived, next_process.pid, next_process.memory, next_process.job_time);
        deque_insert(suspended, next);
        count++;
    }
//    printf("---------loading---------------\n");
//    print_deque(suspended);
    return count;
}

void tick(long long int* clock) {
    *clock = *clock+1;
//    log_info("-------- t=%d --------", *clock);
}

long long int init(Deque* processes, Deque* pending, Deque* suspended) {
    while (deque_size(processes) > 0) {
        process_t *process = deque_pop(processes);
        deque_push(pending, process);
    }
    return 0;
};


void firstComeFirstServe(memory_allocator_t* allocator, Deque* processes, Deque* finish, long long int* clock) {
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

        if (allocator->require_allocation(allocator->structure, process)) {
            allocator->malloc(allocator->structure, process, *clock);
        }
        allocator->info(allocator->structure, process, *clock);

        while (process->remaining_time > 0) {
            if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                allocator->load(allocator->structure, process);
            } else {
                execute(process, *clock);
                allocator->use(allocator->structure, process, *clock);

            }
            load_process(pending, suspended, *clock);
            tick(clock);
        }
        /* A process that has 0 seconds left to run, should be "evicted" from memory before marking the process as
         * finished
         */
        allocator->free(allocator->structure, process, *clock);
        finish_process(process, finish, *clock, deque_size(suspended));
    }
}

void roundRobin(memory_allocator_t* allocator, Deque* processes, Deque* finish, long long int* clock, long long int quantum) {
    Deque *suspended = new_deque((void (*)(void *)) printProcess);
    Deque *pending = new_deque((void (*)(void *)) printProcess);
    init(processes, pending, suspended);

    while (deque_size(pending) > 0 || deque_size(suspended) > 0) {
        load_process(pending, suspended, *clock);
        // continue if there is no process ready to run
        if (deque_size(suspended) > 0) {
            long long int quantumLeft = quantum;
            process_t* process = deque_pop(suspended);

            if (allocator->require_allocation(allocator->structure, process)) {
                allocator->malloc(allocator->structure, process, *clock);
            }
            long long int page_fault_time = allocator->page_fault(allocator->structure, process) > 0;
            process->remaining_time += page_fault_time;
            allocator->info(allocator->structure, process, *clock);

            while (quantumLeft > 0 && process->remaining_time > 0) {
                // Allocate space for the process if it's not in the memory
                if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                    allocator->load(allocator->structure, process);
                }
                else {
                    execute(process, *clock);
                    allocator->use(allocator->structure, process, *clock);
                    quantumLeft--;
                }
                tick(clock);
                load_process(pending, suspended, *clock);
            }
            if (process->remaining_time > 0) {
                deque_insert(suspended, process);
            } else {
                allocator->free(allocator->structure, process, *clock);
                finish_process(process, finish, *clock, deque_size(suspended));
            }
        }
        else {
            tick(clock);
        }
    }
}

int compare_remaining_time(void * a, void * b) {
    long long int r1 = ((process_t*)a)->remaining_time;
    long long int r2 = ((process_t*)b)->remaining_time;
    if (r1 > r2) {
        return 1;
    } else if (r1 < r2) {
        return -1;
    } else {
        return 0;
    }
}


int compare_PID(void * a, void * b) {
    long long int pid1 = ((process_t *)a)->pid;
    long long int pid2 = ((process_t *)b)->pid;
    if (pid1 > pid2) {
        return 1;
    }
    else if (pid2 > pid1) {
        return -1;
    }
    else {
        return 0;
    }
}



void shortestRemainingTimeFirst(memory_allocator_t* allocator, Deque* processes, Deque* finish, long long int* clock) {
    heap_t *suspended = create_heap(deque_size(processes), compare_remaining_time);
    Deque *pending = new_deque((void (*)(void *)) printProcess);
    while (deque_size(processes) > 0) {
        process_t *process = deque_pop(processes);
        deque_push(pending, process);
    }

    while (heap_size(suspended) > 0 || deque_size(pending) > 0){
        load_new_process(suspended, pending, *clock);

        if (heap_size(suspended) > 0) {
            process_t* process = NULL;
            process_t running = heap_pop_min(suspended);
            process = &running;

            if (allocator->require_allocation(allocator->structure, process)){
                allocator->malloc(allocator->structure, process, *clock);
            }
            long long int page_fault_time = allocator->page_fault(allocator->structure, process) > 0;
            process->remaining_time += page_fault_time;

            allocator->info(allocator->structure, process, *clock);

            while (process->remaining_time > 0) {
                if ((allocator->load_time_left(allocator->structure, process)) > 0) {
                    allocator->load(allocator->structure, process);
                } else {
                    execute(process, *clock);
                    allocator->use(allocator->structure, process, *clock);
                }
                tick(clock);
                load_new_process(suspended, pending, *clock);
            }

            allocator->free(allocator->structure, process, *clock);
            process_t * copy = create_process(process->timeArrived, process->pid, process->memory, process->job_time);
            copy->finish_time = *clock;
            deque_insert(finish, copy);
        } else {
            tick(clock);
        }
    }
}

void load_new_process(heap_t* suspended, Deque* pending, long long int clock) {
    // Add newly arrived processes
    heap_t* toAdd = create_heap(MAX_PROCESS_ARRIVAL_PER_TICK, compare_PID);
    while (deque_size(pending) && next_to_pop(pending)->timeArrived == clock) {
        process_t* process = deque_pop(pending);
        assert(process->timeArrived == clock);
        heap_insert(toAdd, *process);
        free_process(process);
        log_info("Process %d added to suspended", process->pid);
        log_process(process);
    }
    while (heap_size(toAdd) > 0) {
        process_t next = heap_pop_min(toAdd);
        heap_insert(suspended, next);
    }
}

int main(int argc, char *argv[]) {
    char* file_name = NULL;
    long long int scheduling_algorithm = -1;
    long long int memory_allocation = -1;
    long long int memory_size = -1;
    long long int quantum = 10;

    char opt;
    while ((opt = getopt (argc, argv, ":f:a:m:s:q:")) != -1) {
        switch (opt) {
            case 'f':
                file_name = malloc(sizeof(file_name) * (strlen(optarg)+1));
                strcpy(file_name, optarg);
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
                } else if (strcasecmp(optarg, "cm") == 0) {
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

//    inspectArguments(file_name, scheduling_algorithm, memory_allocation, memory_size, quantum);

    /*
     * Create a memory allocator based on input argument
     */
    memory_allocator_t* allocator = NULL;
    if (memory_allocation == UNLIMITED) {
        allocator = create_unlimited_allocator();
    } else if (memory_allocation == SWAPPING) {
        allocator = create_swapping_allocator(memory_size, PAGE_SIZE);
    } else if (memory_allocation == VIRTUAL_MEMORY) {
        allocator = create_virtual_memory_allocator_LRU(memory_size, PAGE_SIZE);
    } else {
        allocator = create_virtual_memory_allocator_LFU(memory_size, PAGE_SIZE);
    }

    Deque *processes = new_deque((void (*)(void *)) printProcess);
    Deque *finish = new_deque((void (*)(void *)) printProcess);
    readProcessesFromFile(file_name, processes);
    long long int clock = 0;

    if (scheduling_algorithm == FIRST_COME_FIRST_SERVED) {
        firstComeFirstServe(allocator, processes, finish, &clock) ;
    } else if (scheduling_algorithm == ROUND_ROBIN) {
        roundRobin(allocator, processes, finish, &clock, quantum);
    } else if (scheduling_algorithm == CUSTOMISED_SCHEDULING) {
        shortestRemainingTimeFirst(allocator, processes, finish, &clock);
    }
    analysis(finish, clock);
    return 0;
}

int cmp_long_long_int (const void * a, const void * b) {
    long long int val1 = *(long long int*)a;
    long long int val2= *(long long int*)b;
    if (val1 > val2) {
        return 1;
    }
    else if (val1 < val2) {
        return -1;
    }
    else {
        return 0;
    }
}

void print_memory(long long int* addresses, long long int count) {
    printf("[");
    qsort(addresses, count, sizeof(*addresses), cmp_long_long_int);
    for (long long int i=0; i<count; i++) {
        if (i == 0) {
            printf("%lld", addresses[i]);
        }
        else {
            printf(",%lld",addresses[i]);
        }
    }
    printf("]");
}

void finish_process(process_t* process, Deque* finish, long long int clock, long long int proc_remaining) {
    log_info("<Scheduler> Process %d finished",process->pid);
    process->finish_time = clock;
    output_finish(clock, process, proc_remaining);
    deque_insert(finish, process);
//    free_process(process);
}
void analysis(Deque* finished, long long int clock) {
    long long int interval = (int)ceil((double)(clock)/61);
    long long int throughput[interval];
    for (long long int i=0; i<interval; i++) {
        throughput[i] = 0;
    }

    long long int total_job = deque_size(finished);
    long long int total_turn_around = 0;
    long long int total_job_time = 0;
    double max_overhead = 0;
    double total_overhead = 0;
    while (deque_size(finished) > 0) {
        process_t* process = deque_pop(finished);
        long long int turn_around = process->finish_time - process->timeArrived;
        double overhead = (double)turn_around/(double)process->job_time;
        total_overhead += overhead;
        if (overhead > max_overhead) {
            max_overhead = overhead;
        }
        for (long long int t=0; t<interval;t++) {
            if (process->finish_time > 60*t && process->finish_time<= 60*(t+1)) {
                throughput[t] += 1;
                break;
            }
        }
        total_turn_around += turn_around;
        total_job_time += process->job_time;
        free_process(process);
    }

    long long int throughput_total = 0;
    long long int throughput_min = INT_MAX;
    long long int throughput_max = INT_MIN;

    for (long long int j=0; j<interval; j++) {
        throughput_total += throughput[j];
        if (throughput[j] < throughput_min) {
            throughput_min = throughput[j];
        }
        if (throughput[j] > throughput_max) {
            throughput_max = throughput[j];
        }

    }

    printf("Throughput %d, %lld, %lld\n", (int)ceil((double)throughput_total/(double)interval), throughput_min, throughput_max);
    printf("Turnaround time %d\n", (int)ceil((double)total_turn_around/(double)total_job));
    printf("Time overhead %.2f %.2f\n", max_overhead, (double)total_overhead/(double)total_job);
    printf("Makespan %lld\n", clock);
}