//
// Created by Haswell on 22/05/2020.
//

#ifndef SCHEDULER_VIRTUAL_MEMORY_H
#define SCHEDULER_VIRTUAL_MEMORY_H

#include "dlist.h"
#include "memory_allocator.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "constants.h"
#define NOT_OCCUPIED -1
#define MIN_PAGE_REQUIRED_TO_RUN 4

typedef struct virtual_memory {
    int page_size;
    int free_frame;
    int total_frame;
    int* page_frames;
    Dlist* page_tables;
} virtual_memory_t;

typedef struct page_table_entry {
    int validity;
    int frame_number;
    int referenced;
} page_table_entry_t;


typedef struct page_table_node {
    int pid;
    int page_count;
    page_table_entry_t* page_table_pointer;
    int last_access;
    int valid_page_count;
    int loading_time_left;
} page_table_node_t;

int deallocate_one_page(virtual_memory_t* memory_manager, page_table_node_t* page_table);
page_table_node_t* create_page_table_node(int pid, int page_count);
virtual_memory_t* create_virtual_memory(int memory_size, int page_size);
page_table_node_t* find_the_oldest_process(virtual_memory_t* memory_manager, process_t* process);
memory_allocator_t* create_virtual_memory_allocator(int memory_size, int page_size);
void print_memory(int* addresses, int count);
int map(page_table_node_t * page_table, int frame_number);
int virtual_memory_usage(virtual_memory_t* memory_manager);
void virtual_print_addresses(virtual_memory_t* memory_manager, process_t* process);
#endif //SCHEDULER_VIRTUAL_MEMORY_H
