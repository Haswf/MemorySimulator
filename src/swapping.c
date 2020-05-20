//
// Created by Haswell on 18/05/2020.
//
#include "swapping.h"
#include "../test/swapping_test.h"

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
    m_list->list = new_dlist(dlist_free_fragment);
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
Node* first_fit(memory_list_t* memoryList, process_t* process, int clock) {
    assert(memoryList);
    assert(process);
    /*
     * Find how many pages are required. If a process need 98 bytes, 25 pages are required.
     */
    int pages_required = byteToRequiredPage(process->memory, memoryList->page_size);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*) current->data;
        if (fragment->type == HOLE_FRAGMENT && fragment->page_length > pages_required) {
            log_debug("<MEMORY> First fit for pid %d (%d pages) is at %d", clock, process->pid, pages_required, fragment->page_start);
            log_fragment(fragment);
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
    fragment->page_length = fragment->page_start + required_page;
    fragment->type = PROCESS_FRAGMENT;
    fragment->pid = process->pid;
    return hole;
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
void allocate_memory(memory_list_t* memoryList, process_t* process, int clock) {
    /*
     * Use first fit algorithm to find a fragment large enough for the process
     */
    Node* freeSpace = first_fit(memoryList, process, clock);

    /*
     * If not found, evict the pages belongs to the least recently executed process
     * until a fragment is found.
     */
    while (!freeSpace){
        log_debug("<MEMORY> Insufficient memory for process t=%d\t requiring %d bytes\n", process->pid, process->memory);
        Node* toEvict = find_least_recently_used(memoryList);
        memory_fragment_t* fragment = (memory_fragment_t*)toEvict->data;
        log_debug("<MEMORY> Evicting pages for process %d (last access: %d) with %d pages (%d bytes)\n", fragment->pid, fragment->last_access, fragment->page_length, fragment->byte_length);
        evict(memoryList, toEvict);
        freeSpace = first_fit(memoryList, process, clock);
    }

    allocate(memoryList, freeSpace, process);
}

/**
 * Use memory
 * This internally updated last access time of the fragment.
 * @param memoryList
 * @param process
 * @param clock
 */
void use_memory(memory_list_t* memoryList, process_t* process, int clock) {
    assert(memoryList && process);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT && fragment->pid == process->pid) {
            fragment->last_access = clock;
        }
        current = current->next;
    }
}

void free_memory(memory_list_t* memoryList, process_t* process, int clock) {
    assert(memoryList && process);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT && fragment->pid == process->pid) {
            log_debug("<MEMORY> Free memory allocated for process %d (%d pages %d bytes)", process->pid, fragment->page_length, fragment->byte_length);
            log_trace("-------------before------------");
            log_memory_list(memoryList);
            evict(memoryList, current);
            log_trace("-------------after------------");
            log_memory_list(memoryList);
            log_trace("------------------------------");
            return;
        }
        current = current->next;
    }
}