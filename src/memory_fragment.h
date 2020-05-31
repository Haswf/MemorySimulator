//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H
#define COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H

#include "process.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"


typedef struct memory_fragment {
    long long int type;
    long long int page_start;
    long long int byte_start;
    long long int page_length;
    long long int byte_length;
    long long int pid;
    long long int last_access;
    long long int load_time;
} memory_fragment_t;

memory_fragment_t* create_hole_fragment(long long int byte_start, long long int page_start, long long int byte_length, long long int page_length);
memory_fragment_t* create_process_fragment(long long int byte_start, long long int page_start, long long int byte_length, long long int page_length, long long int pid);
void print_fragment(memory_fragment_t* fragment);
void free_fragment(memory_fragment_t* fragment);
void dlist_free_fragment(void* fragment);
void log_fragment(memory_fragment_t* fragment);

#endif //COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H
