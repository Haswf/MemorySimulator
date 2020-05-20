//
// Created by Haswell on 20/05/2020.
//

#ifndef SCHEDULER_UNLIMITED_H
#define SCHEDULER_UNLIMITED_H

#include "swapping.h"

void unlimited_use_memory(void* structure, process_t* process, int clock);
void* unlimited_allocate_memory(void* structure, process_t* process);
void unlimited_free_memory(void* structure, process_t* process);
int unlimited_load_time_left(void* structure, process_t* process);
void unlimited_load_memory(void* structure, process_t* process);
#endif //SCHEDULER_UNLIMITED_H
