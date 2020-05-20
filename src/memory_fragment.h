//
// Created by Haswell on 18/05/2020.
//

#ifndef COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H
#define COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H
#define HOLE_FRAGMENT 5
#define PROCESS_FRAGMENT 6
#include "process.h"
#include "log.h"

typedef struct memory_fragment {
    int type;
    int start;
    int length;
    int pid;
    int last_access;
} memory_fragment_t;

memory_fragment_t* create_hole_fragment(int start, int length);
memory_fragment_t* create_process_fragment(int start, int length, int pid);
void print_fragment(memory_fragment_t* fragment);
void free_fragment(memory_fragment_t* fragment);
void dlist_free_fragment(void* fragment);
void log_fragment(memory_fragment_t* fragment);

#endif //COMP30023_2020_PROJECT_2_MEMORY_FRAGMENT_H
