//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_SCHEDULER_H
#define COMP30023_2020_PROJECT_2_SCHEDULER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"
#include "deque.h"
#include <assert.h>
#include "heap.h"
#include "swapping.h"
#include "memory_fragment.h"
#include "log.h"

#define FIRST_COME_FIRST_SERVED 1
#define ROUND_ROBIN 2
#define CUSTOMISED_SCHEDULING 3

#define UNLIMITED -1
#define SWAPPING -2
#define VIRTUAL_MEMORY -3
#define CUSTOMISED_MEMORY -4
#define LOG_LEVEL LOG_DEBUG

#include "log.h"

int compare_PID(void * a, void * b);

int firstComeFirstServe(Deque* processes, int* clock, int* finish);

#define MAX_PROCESS_ARRIVAL_PER_TICK 100
#endif //COMP30023_2020_PROJECT_2_SCHEDULER_H
