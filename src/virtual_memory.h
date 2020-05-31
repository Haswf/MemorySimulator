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
#include <stdint.h>
#include "ctype.h"
#include "constants.h"
#include "scheduler.h"
#define NOT_OCCUPIED -1
#define MIN_PAGE_REQUIRED_TO_RUN 4

typedef struct virtual_memory {
    long long int page_size;
    long long int free_frame;
    long long int total_frame;
    /* This array records which process each page has been mapped into */
    long long int* page_frames;
    /* This array records how many time each page has been referenced */
    unsigned int* counter;
    Dlist* page_tables;
} virtual_memory_t;

typedef struct page_table_entry {
    long long int validity;
    long long int frame_number;
    long long int reference;
} page_table_entry_t;


typedef struct page_table_node {
    long long int pid;
    long long int page_count;
    page_table_entry_t* page_table_pointer;
    long long int last_access;
    long long int valid_page_count;
    long long int loading_time_left;
} page_table_node_t;

long long int evict_one_page(virtual_memory_t* memory_manager, long long int frame_number);
page_table_node_t* create_page_table_node(long long int pid, long long int page_count);
virtual_memory_t* create_virtual_memory(long long int memory_size, long long int page_size);
long long int find_the_oldest_process(virtual_memory_t* memory_manager, long long int skip);
void virtual_memory_allocate_memory_LRU(virtual_memory_t* memory_manager, process_t* process, long long int clock);
void virtual_memory_allocate_memory_LFU(virtual_memory_t* memory_manager, process_t* process, long long int clock);

long long int least_recent_used(virtual_memory_t* memory_manager, long long int skip);
memory_allocator_t* create_virtual_memory_allocator_LFU(long long int memory_size, long long int page_size);
memory_allocator_t* create_virtual_memory_allocator_LRU(long long int memory_size, long long int page_size);
long long int map(page_table_node_t * page_table, long long int frame_number);
long long int virtual_memory_usage(virtual_memory_t* memory_manager);
void virtual_print_addresses(virtual_memory_t* memory_manager, process_t* process);
void aging(virtual_memory_t* memory_manager);


/**
 * Return the frame number of the first frame of a process in memory
 * @param memory_manager
 * @param pid
 * @return
 */
long long int first_page(virtual_memory_t* memory_manager, long long int pid);
#endif //SCHEDULER_VIRTUAL_MEMORY_H
