//
// Created by Haswell on 18/05/2020.
//

#ifndef SCHEDULER_SWAPPING_H
#define SCHEDULER_SWAPPING_H

#include "dlist.h"
#include "dlist.h"
#include <stdlib.h>
#include <assert.h>
#include "memory_fragment.h"
#include <stdio.h>
#include <stdbool.h>
#include "memory_allocator.h"

typedef struct memory_list {
    Dlist* list;
    int page_size;
} memory_list_t;

int swapping_load_time_left(memory_list_t* memoryList, process_t* process);
void log_memory_list(memory_list_t* memoryList);
memory_list_t* create_memory_list(int mem_size, int page_size);
void free_memory_list(memory_list_t* memoryList);
Node* first_fit(memory_list_t* memoryList, process_t* process);
Node* allocate(memory_list_t* memoryList, Node* hole, process_t* process);
void print_memory_list(memory_list_t* memoryList);
Node* evict(memory_list_t* memoryList, Node* nodeToEvict);
Node* find_least_recently_used(memory_list_t* memoryList);
void swapping_use_memory(memory_list_t* memoryList, process_t* process, int clock);
Node* swapping_allocate_memory(memory_list_t* memoryList, process_t* process);
void swapping_free_memory(memory_list_t* memoryList, process_t* process);
int byteToRequiredPage(int bytes, int page_size);
int byteToAvailablePage(int bytes, int page_size);
void swapping_load_memory(memory_list_t* memoryList, process_t* process);
memory_allocator_t* create_swapping_allocator(int memory_size, int page_size);

#endif //SCHEDULER_SWAPPING_H
