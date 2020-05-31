/**
 * Unlimited memory module
 * Created by Haswell on 20/05/2020.
 */

#include "unlimited.h"

/**
 * Allocate some to a process.
 * Do nothing for unlimited memory.
 * @param structure
 * @param process
 * @param clock
 * @return
 */
void* unlimited_allocate_memory(void* structure, process_t* process,long long int clock) {
    return (void*)1;
};
/**
 * Simulates the reference to memory.
 * Do nothing for unlimited memory b/c we don't care.
 * @param structure
 * @param process
 * @param clock
 */
void unlimited_use_memory(void* structure, process_t* process, long long int clock) {

};
/**
 * Free memory allocated to a process.
 * Do nothing for unlimited memory.
 * @param structure
 * @param process
 * @param clock
 */
void unlimited_free_memory(void* structure, process_t* process, long long int clock) {

};


long long int unlimited_load_time_left(void* structure, process_t* process) {
    return 0;
};
/**
 * Simulate the process of loading a page from disk to memory.
 * Do nothing since no loading is required.
 * @param structure
 * @param process
 */
void unlimited_load_memory(void* structure, process_t* process) {

};
/**
 * Returns if a process has been allocated all memory it requires.
 * Since memory is unlimited, always return 0
 * @param memoryList
 * @param process
 * @return
 */
long long int unlimited_require_allocation(void* structure, process_t* process) {
    return 0;
}


/**
 * Returns how many page fault will occur during execution.
 * Since swapping won't cause page fault, always returns 0
 * @param structure
 * @param process
 * @return
 */
long long int unlimited_page_fault(void* structure, process_t* process) {
    return 0;
}
/**
 * Print status of a process and its memory usage.
 * @param structure
 * @param process
 * @param clock
 */
void unlimited_process_info(void* structure, process_t* process, long long int clock) {
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", clock, process->pid, process->remaining_time);
}
/**
 * Create an implementation of memory allocator for unlimited memory
 * @param memory_size
 * @param page_size
 * @return
 */
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
