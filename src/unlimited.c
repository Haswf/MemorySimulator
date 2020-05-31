//
// Created by Haswell on 20/05/2020.
//

#include "unlimited.h"

void* unlimited_allocate_memory(void* structure, process_t* process,long long int clock) {
    return (void*)1;
};

void unlimited_use_memory(void* structure, process_t* process, long long int clock) {

};
void unlimited_free_memory(void* structure, process_t* process, long long int clock) {

};

long long int unlimited_load_time_left(void* structure, process_t* process) {
    return 0;
};
void unlimited_load_memory(void* structure, process_t* process) {

};
long long int unlimited_require_allocation(void* structure, process_t* process) {
    return 0;
}
long long int unlimited_page_fault(void* structure, process_t* process) {
    return 0;
}
void unlimited_process_info(void* structure, process_t* process, long long int clock) {
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", clock, process->pid, process->remaining_time);
}
memory_allocator_t* create_unlimited_allocator() {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    allocator->malloc = unlimited_allocate_memory,
    allocator->info = unlimited_process_info;
    allocator->use = unlimited_use_memory;
    allocator->free = unlimited_free_memory;
    allocator->load = unlimited_load_memory;
    allocator->load_time_left = unlimited_load_time_left;
    allocator->require_allocation = unlimited_require_allocation;
    allocator->page_fault = unlimited_page_fault;
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = NULL;
    return allocator;
}
