//
// Created by Haswell on 18/05/2020.
//

#ifndef SCHEDULER_SWAPPING_H
#define SCHEDULER_SWAPPING_H
#include "dlist.h"
#include "dlist.h"
#include "stdlib.h"
#include "assert.h"
#include "memory_fragment.h"
#include "stdio.h"

typedef struct memory_list {
    Dlist* list;
} memory_list_t;

void log_memory_list(memory_list_t* memoryList);
memory_list_t* create_memory_list();
void init_memory_list(memory_list_t* memoryList, int mem_size);
void free_memory_list(memory_list_t* memoryList);
Node* first_fit(memory_list_t* memoryList, process_t* process, int clock);
Node* allocate(memory_list_t* memoryList, Node* hole, process_t* process);
void print_memory_list(memory_list_t* memoryList);
Node* evict(memory_list_t* memoryList, Node* nodeToEvict);
void use_memory(memory_list_t* memoryList, process_t* process, int clock);
Node* find_least_recently_used(memory_list_t* memoryList);
void allocate_memory(memory_list_t* memoryList, process_t* process, int clock);
void free_memory(memory_list_t* memoryList, process_t* process, int clock);
#endif //SCHEDULER_SWAPPING_H
