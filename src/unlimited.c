//
// Created by Haswell on 20/05/2020.
//

#include "unlimited.h"

void unlimited_use_memory(void* structure, process_t* process, int clock) {
};
void* unlimited_allocate_memory(void* structure, process_t* process) {
    return (void*)1;
};
void unlimited_free_memory(void* structure, process_t* process) {

};

int unlimited_load_time_left(void* structure, process_t* process) {
    return 0;
};
void unlimited_load_memory(void* structure, process_t* process) {

};