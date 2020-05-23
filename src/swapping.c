//
// Created by Haswell on 18/05/2020.
//
#include "swapping.h"
#include "virtual_memory.h"


/**
 * Create a memory list
 * @param mem_size total memory size
 * @param page_size size of each page in memory
 * @return
 */
memory_list_t* create_memory_list(int mem_size, int page_size) {
    memory_list_t* m_list = (memory_list_t*)malloc(sizeof(*m_list));
    assert(m_list);
    m_list->page_size = page_size;
    m_list->list = new_dlist(dlist_free_fragment, (void (*)(void *)) print_fragment);
    assert(m_list->list);
    /* The first process is always given a memory page 0*/
    memory_fragment_t* empty_memory = create_hole_fragment(0, 0, mem_size, byteToAvailablePage(mem_size, page_size));
    dlist_add_start(m_list->list, empty_memory);
    return m_list;
}

/**
 * Free memory allocated to the memory list
 * @param memoryList
 */
void free_memory_list(memory_list_t* memoryList) {
    assert(memoryList);
    free_dlist(memoryList->list);
    free(memoryList);
}

/**
 * Find the first memory fragment that has enough space for the given process
 * @param memoryList
 * @param process
 * @param clock current time for logging purpose
 * @return
 */
Node* first_fit(memory_list_t* memoryList, process_t* process) {
    assert(memoryList);
    assert(process);
    /*
     * Find how many pages are required. If a process need 98 bytes, 25 pages are required.
     */
    int pages_required = byteToRequiredPage(process->memory, memoryList->page_size);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*) current->data;
        if (fragment->type == HOLE_FRAGMENT && fragment->page_length >= pages_required) {
            log_debug("<MEMORY> First fit for pid %d (%d pages) is at %d", process->pid, pages_required, fragment->page_start);
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Convert number of bytes to pages
 * @param bytes
 * @param page_size
 * @return
 */
int byteToRequiredPage(int bytes, int page_size) {
    if (bytes % page_size == 0) {
        return bytes/page_size;
    }
    else {
        return (bytes+bytes % page_size)/page_size;
    }
}

/**
 * Convert a consecutive memory length in bytes to available page sizes.
 * For instance, a memory of 99 bytes can hold 24 pages.
 */
int byteToAvailablePage(int bytes, int page_size) {
    return bytes/page_size;
}

/**
 * Allocate
 * @param memoryList
 * @param hole
 * @param process
 * @return
 */
Node* allocate(memory_list_t* memoryList, Node* hole, process_t* process) {
    memory_fragment_t* fragment = (memory_fragment_t*)hole->data;
    // Calculate how many pages are required for the process.
    int required_page = byteToRequiredPage(process->memory, memoryList->page_size);
    // Calculate how much space will be required to save these pages.
    int required_memory = required_page*4;
    // Break the hole into two parts
    dlist_insert_after(memoryList->list, hole,
            create_hole_fragment(
                    fragment->byte_start + required_memory,
                    fragment->page_start + required_page,
                    fragment->byte_length - required_memory,
                    byteToAvailablePage(fragment->byte_length - required_memory, memoryList->page_size)
                    ));
    // Convert the hole fragment to a process fragment
    fragment->byte_length = required_memory;
    // update page start
    fragment->page_length = required_page;
    fragment->load_time = LOADING_TIME_PER_PAGE * required_page;
    fragment->type = PROCESS_FRAGMENT;
    fragment->pid = process->pid;
    log_info("<Scheduler> Memory allocated for process %d (%d bytes)", process->pid, process->memory);
    return hole;
}
int swapping_load_time_left(memory_list_t* memoryList, process_t* process) {
    Node* current = memoryList->list->head;
    memory_fragment_t* fragment = NULL;
    while (current) {
        fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT) {
            if (fragment->pid == process->pid) {
                return fragment->load_time ;
            }
        }
        current = current->next;
    }
    return -1;
}

void swapping_load_memory(memory_list_t* memoryList, process_t* process) {
    Node* current = memoryList->list->head;
    memory_fragment_t* fragment = NULL;
    while (current) {
        fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT) {
            if (fragment->pid == process->pid && fragment->load_time > 0) {
                fragment->load_time -= 1;
                log_trace("<Scheduler> Loading pages for process %d ETA: %d ticks", process->pid, fragment->load_time);
            }
        }
        current = current->next;
    }
}

void log_memory_list(memory_list_t* memoryList) {
    Node* current = memoryList->list->head;
    while (current) {
        log_fragment(current->data);
        current = current -> next;
    }
}

void deallocate_memory_fragment(Node* nodeToFree) {
    memory_fragment_t* fragmentToFree = (memory_fragment_t*) nodeToFree->data;
    fragmentToFree->type = HOLE_FRAGMENT;
    fragmentToFree->pid = -1;
    fragmentToFree->load_time = -1;
    fragmentToFree->last_access = -1;
}

/**
 * Merge a hole fragment with the previous fragment.
 * @param memoryList
 * @param nodeToEvict
 * @return
 */
Node* join_prev(memory_list_t* memoryList, Node* nodeToEvict) {
    assert(memoryList && nodeToEvict);
    memory_fragment_t* fragmentToEvict = (memory_fragment_t*) nodeToEvict->data;
    /* Find previous memory fragment*/
    memory_fragment_t* prevFragment = (memory_fragment_t*) nodeToEvict->prev->data;
    /* Assert both current fragment and the previous fragment are hole i.e. not occupied */
    assert(fragmentToEvict->type==HOLE_FRAGMENT && prevFragment->type==HOLE_FRAGMENT);

    /* Merge this fragment with previous fragment.
     *     prev        +        current     =    merged
     * [0, 0, 100, 25] + [100, 25, 100, 25] = [0, 0, 200, 50]
     * Byte start and page start of the previous fragment should remain unchanged */
    /* Update total byte length*/
    int total_size = prevFragment->byte_length + fragmentToEvict->byte_length;
    prevFragment->byte_length = total_size;
    /* Update total page length*/
    prevFragment->page_length = byteToAvailablePage(total_size, memoryList->page_size);
    Node* merged = nodeToEvict->prev;
    /* Free this memory fragment*/
    dlist_remove(memoryList->list, nodeToEvict);
    return merged;
}

/**
 * Merge a hole fragment with the next fragment.
 * @param memoryList
 * @param nodeToEvict
 * @return
 */
Node* join_next(memory_list_t* memoryList, Node* nodeToEvict) {
    assert(memoryList && nodeToEvict);
    memory_fragment_t* fragmentToEvict = (memory_fragment_t*) nodeToEvict->data;
    /* Find next memory fragment*/
    memory_fragment_t* nextFragment = (memory_fragment_t*) nodeToEvict->next->data;

    /* Assert both current fragment and the next fragment are hole i.e. not occupied */
    assert(fragmentToEvict->type == HOLE_FRAGMENT && nextFragment->type == HOLE_FRAGMENT);

    /* Merge this fragment with previous fragment.
     *      current    +       next         = merged
     * [0, 0, 100, 25] + [100, 25, 100, 25] = [0, 0, 200, 50]
     * Byte start and page start of the previous fragment should remain unchanged */

    /* Update total byte length*/
    int total_size = fragmentToEvict->byte_length + nextFragment->byte_length;
    fragmentToEvict->byte_length = total_size;
    /* Update total page length*/
    fragmentToEvict->page_length = byteToAvailablePage(total_size, memoryList->page_size);
    Node* merged = nodeToEvict;
    /* Free the next fragment*/
    dlist_remove(memoryList->list, nodeToEvict->next);
    return merged;
}

/**
 * Find the memory fragment that is the least recently executed
 * @param memoryList
 * @return A code pointer containing the fragment
 */
Node* find_least_recently_used(memory_list_t* memoryList) {
    Node* current = memoryList->list->head;
    memory_fragment_t* fragment = NULL;
    Node* nodeToSwap = NULL;
    int minLastAccess;

    while (current) {
        fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT) {
            if (!nodeToSwap) {
                minLastAccess = fragment->last_access;
                nodeToSwap = current;
            }
            if (fragment->last_access < minLastAccess){
                minLastAccess = fragment->last_access;
                nodeToSwap = current;
            }
        }
        current = current->next;
    }
    return nodeToSwap;
}

/**
 * Evict a given fragment form memory
 * @param memoryList
 * @param nodeToEvict
 * @return
 */
Node* evict(memory_list_t* memoryList, Node* nodeToEvict) {
    Node* merged = nodeToEvict;
    memory_fragment_t* fragmentToEvict = (memory_fragment_t*) nodeToEvict->data;
    assert(fragmentToEvict->type == PROCESS_FRAGMENT);
    /* Deallocate the memory fragment */
    deallocate_memory_fragment(merged);
    /* Merge with the previous fragment if it exists and it's empty too */
    if (merged->prev) {
        memory_fragment_t* prevFragment = (memory_fragment_t*) merged->prev->data;
        if (prevFragment->type == HOLE_FRAGMENT) {
            merged = join_prev(memoryList, merged);
        }
    }
    /* Merge with the next fragment if it exists and it's empty too */
    if (merged->next) {
        memory_fragment_t *nextFragment = (memory_fragment_t *) merged->next->data;
        if (nextFragment->type == HOLE_FRAGMENT) {
            merged = join_next(memoryList, merged);
        }
    }
    /* Returns the free space */
    return merged;
}

/**
 * Allocate memory for a process
 * @param memoryList
 * @param process
 * @param clock
 */
Node* swapping_allocate_memory(memory_list_t* memoryList, process_t* process, int clock) {
    /*
     * Use first fit algorithm to find a fragment large enough for the process
     */
    Node* freeSpace = first_fit(memoryList, process);

    /*
     * If not found, evict the pages belongs to the least recently executed process
     * until a fragment is found.
     */
    while (!freeSpace){
        log_debug("<MEMORY> Insufficient memory for process %d\t requiring %d bytes", process->pid, process->memory);
        Node* toEvict = find_least_recently_used(memoryList);
        if (toEvict) {
            memory_fragment_t* fragment = (memory_fragment_t*)toEvict->data;
            int page_to_free = fragment->page_length;
            int* addr_to_print = malloc(sizeof(*addr_to_print) * page_to_free);
            int index = 0;
            for (int i=0; i<fragment->page_length; i++) {
                addr_to_print[index++] = fragment->page_start + i;
            }
            printf("%d, EVICTED, mem-addresses=", clock);
            print_memory(addr_to_print, fragment->page_length);
            printf("\n");
            free(addr_to_print);
            freeSpace = first_fit(memoryList, process);
            evict(memoryList, toEvict);
        } else {
            return NULL;
        }

    }
    return allocate(memoryList, freeSpace, process);
}

memory_fragment_t* get_fragment(memory_list_t* memoryList, process_t* process) {
    Node* current = memoryList->list->head;

    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT && fragment->pid == process->pid) {
            return fragment;
        }
        current = current->next;
    }
}

/**
 * Use memory
 * This internally updated last access time of the fragment.
 * @param memoryList
 * @param process
 * @param clock
 */
void swapping_use_memory(memory_list_t* memoryList, process_t* process, int clock) {
    assert(memoryList && process);
    memory_fragment_t* fragment = get_fragment(memoryList, process);
    fragment->last_access = clock;
}

void swapping_process_info(memory_list_t* memoryList, process_t* process, int clock) {
    memory_fragment_t* fragment = get_fragment(memoryList, process);
    printf("%d, RUNNING, id=%d, remaining-time=%d, load-time=%d, mem-usage=%d%%, mem-addresses=",
           clock,
           process->pid,
           process->remaining_time,
           fragment->load_time,
           swapping_memory_usage(memoryList, process));
    swapping_print_addresses(memoryList, process);
    printf("\n");
}

void swapping_print_addresses(memory_list_t* memoryList, process_t* process) {
    assert(memoryList && process);
    memory_fragment_t* fragment = get_fragment(memoryList, process);
    int* addr_to_print = malloc(sizeof(*addr_to_print) * fragment->page_length);
    int index = 0;
    for (int i=0; i<fragment->page_length; i++) {
        addr_to_print[index++] = fragment->page_start + i;
    }
    print_memory(addr_to_print, fragment->page_length);
    free(addr_to_print);
}

int swapping_memory_usage(memory_list_t* memoryList, process_t* process) {
    assert(memoryList && process);
    int total = 0;
    int in_use = 0;
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT) {
            in_use+=fragment->page_length;
        }
        total+=fragment->page_length;
        current = current->next;
    }
    return ceil((double)in_use * 100 /(double)total);
}

void swapping_free_memory(memory_list_t* memoryList, process_t* process, int clock) {
    assert(memoryList && process);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT && fragment->pid == process->pid) {
            int page_to_free = fragment->page_length;
            int* addr_to_print = malloc(sizeof(*addr_to_print) * page_to_free);
            int index = 0;
            for (int i=0; i<fragment->page_length; i++) {
                addr_to_print[index++] = fragment->page_start + i;
            }
            printf("%d, EVICTED, mem-addresses=", clock);
            print_memory(addr_to_print, fragment->page_length);
            printf("\n");
            free(addr_to_print);
            current = evict(memoryList, current);
        } else {
            current = current->next;
        };
    }
}

int swapping_require_allocation(memory_list_t* memoryList, process_t* process) {
    assert(memoryList && process);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT && fragment->pid == process->pid) {
            return 0;
        }
        current = current->next;
    }
    return -1;
}

int swapping_page_fault(memory_list_t* memoryList, process_t* process) {
    assert(memoryList && process);
    return 0;
}

memory_allocator_t* create_swapping_allocator(int memory_size, int page_size) {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    allocator->malloc = (void *(*)(void *, process_t *)) swapping_allocate_memory;
    allocator->info = (void (*)(void *, process_t *, int)) swapping_process_info;
    allocator->use = (void (*)(void *, process_t *, int)) swapping_use_memory;
    allocator->free = (void (*)(void *, process_t *)) swapping_free_memory;
    allocator->load = (void (*)(void *, process_t *)) swapping_load_memory;
    allocator->load_time_left = (int (*)(void *, process_t *)) swapping_load_time_left;
    allocator->require_allocation = (int (*)(void *, process_t *)) swapping_require_allocation;
    allocator->page_fault = (int (*)(void *, process_t *)) swapping_page_fault;
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = create_memory_list(memory_size, page_size);
    return allocator;
}