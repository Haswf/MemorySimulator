//
// Created by Haswell on 18/05/2020.
//

#include "memory_fragment.h"




memory_fragment_t* create_hole_fragment(long long int byte_start, long long int page_start, long long int byte_length, long long int page_length) {
    memory_fragment_t* fragment = (memory_fragment_t*)malloc(sizeof(*fragment));
    fragment->type = HOLE_FRAGMENT;
    fragment->byte_start = byte_start;
    fragment->page_start = page_start;
    fragment->byte_length = byte_length;
    fragment->page_length = page_length;
    fragment -> pid = -1;
    fragment->last_access = -1;
    fragment->load_time = -1;
    return fragment;
}

memory_fragment_t* create_process_fragment(long long int byte_start, long long int page_start, long long int byte_length, long long int page_length, long long int pid) {
    memory_fragment_t* fragment = (memory_fragment_t*)malloc(sizeof(*fragment));
    fragment->type = PROCESS_FRAGMENT;
    fragment->byte_start = byte_start;
    fragment->page_start = page_start;
    fragment->byte_length = byte_length;
    fragment->page_length = page_length;
    fragment -> pid = pid;
    fragment->last_access= -1;
    fragment->load_time = page_length*LOADING_TIME_PER_PAGE;
    return fragment;
}

void print_fragment(memory_fragment_t* fragment) {
    printf("%s b_start: %4lld | p_start: %4lld | b_length: %4lld | p_length:%lld | pid:%4lld | last_access:%4lld|\n",
              fragment->type == HOLE_FRAGMENT?"H\t": "P\t",
              fragment->byte_start,
              fragment->page_start,
              fragment->byte_length,
              fragment->page_length,
              fragment->pid,
              fragment->last_access
    );
}

void log_fragment(memory_fragment_t* fragment) {
    log_trace("%s b_start: %4d | p_start: %4d | b_length: %4d | p_length:%d | pid:%4d | last_access:%4d|",
            fragment->type == HOLE_FRAGMENT?"H\t": "P\t",
            fragment->byte_start,
            fragment->page_start,
            fragment->byte_length,
            fragment->page_length,
            fragment->pid,
            fragment->last_access
            );
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