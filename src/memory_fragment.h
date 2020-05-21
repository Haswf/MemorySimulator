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
    int type;
    int page_start;
    int byte_start;
    int page_length;
    int byte_length;
    int pid;
    int last_access;
    int load_time;
} memory_fragment_t;

memory_fragment_t* create_hole_fragment(int byte_start, int page_start, int byte_length, int page_length);
memory_fragment_t* create_process_fragment(int byte_start, int page_start, int byte_length, int page_length, int pid);
void print_fragment(memory_fragment_t* fragment);
void free_fragment(memory_fragment_t* fragment);
void dlist_free_fragment(void* fragment);
void log_fragment(memory_fragment_t* fragment);

#endif //COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H
