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
int unlimited_require_allocation(void* structure, process_t* process) {
    return 0;
}
memory_allocator_t* create_unlimited_allocator() {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    allocator->allocate_memory = unlimited_allocate_memory,
    allocator->use_memory = unlimited_use_memory;
    allocator->free_memory = unlimited_free_memory;
    allocator->load_memory = unlimited_load_memory;
    allocator->load_time_left = unlimited_load_time_left;
    allocator->require_allocation = unlimited_require_allocation;
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = NULL;
    return allocator;
}
