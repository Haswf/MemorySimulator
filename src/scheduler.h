//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_SCHEDULER_H
#define COMP30023_2020_PROJECT_2_SCHEDULER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>

#include "process.h"
#include "heap.h"
#include "swapping.h"
#include "memory_fragment.h"
#include "log.h"
#include "memory_allocator.h"
#include "unlimited.h"
#include "deque.h"
#include "virtual_memory.h"

int compare_PID(void * a, void * b);

void firstComeFirstServe(memory_allocator_t* allocator, Deque* processes, Deque* finish, long long int* clock);
void execute(process_t* process, long long int clock);
void finish_process(process_t* process, Deque* finish, long long int clock, long long int proc_remaining);
void analysis(Deque* finished, long long int clock);
void load_new_process(heap_t* suspended, Deque* pending, long long int clock);
void print_memory(long long int* addresses, long long int count);

#define MAX_PROCESS_ARRIVAL_PER_TICK 100
#endif //COMP30023_2020_PROJECT_2_SCHEDULER_H
