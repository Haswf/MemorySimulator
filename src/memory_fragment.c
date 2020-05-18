//
// Created by Haswell on 18/05/2020.
//

#include "memory_fragment.h"
#include <stdlib.h>
#include <stdio.h>
#define PAGE_SIZE 4
memory_fragment_t* create_hole_fragment(int start, int length) {
    memory_fragment_t* fragment = (memory_fragment_t*)malloc(sizeof(*fragment));
    fragment->type = HOLE_FRAGMENT;
    fragment->start = start;
    fragment->length = length;
    fragment -> process = NULL;
    fragment->last_access = -1;
    return fragment;
}

memory_fragment_t* create_process_fragment(int start, int length, process_t* process) {
    memory_fragment_t* fragment = (memory_fragment_t*)malloc(sizeof(*fragment));
    fragment->type = PROCESS_FRAGMENT;
    fragment->start = start;
    fragment->length = length;
    fragment -> process = process;
    fragment->last_access= -1;
    return fragment;
}

void print_fragment(memory_fragment_t* fragment) {
    if (fragment->type == HOLE_FRAGMENT) {
        printf("H\t");
    } else {
        printf("P\t");
    }
    printf("start: %d\tlength: %d\t last access: %d\t", fragment->start, fragment->length, fragment->last_access);
    if (fragment->process) {
        printf("pid: %d", fragment->process->pid);
    }
    printf("\n");
}

void free_fragment(memory_fragment_t* fragment) {
    if (fragment) {
        free(fragment);
    }
}


void dlist_free_fragment(void* fragment) {
    if (fragment) {
        free(fragment);
    }
}