/**
 * Virtual Memory Module
 * Created by Haswell on 18/05/2020.
 */

#include "virtual_memory.h"

/**
 * Creates a page table node, which is basically a page table contained in a linklist node.
 * @param pid
 * @param page_count
 * @return
 */
page_table_node_t* create_page_table_node(long long int pid, long long int page_count) {
    page_table_node_t* page = (page_table_node_t*)malloc(sizeof(*page));
    assert(page);
    page->page_count = page_count;
    page->pid = pid;
    page->valid_page_count = 0;
    page->loading_time_left = 0;
    page->page_table_pointer = malloc(sizeof(*page->page_table_pointer) * page_count);
    assert(page->page_table_pointer);
    page->last_access = -1;
    for (long long int i=0; i<page_count; i++) {
        page->page_table_pointer[i].frame_number = -1;
        page->page_table_pointer[i].reference = 0;
        page->page_table_pointer[i].validity = 0;
    }
    return page;
}

/**
 * Free a page table
 * @param page_table
 */
void free_page_table_node(page_table_node_t* page_table) {
    assert(page_table);
    free(page_table->page_table_pointer);
    free(page_table);
}

/**
 * Dlist wrapper for free_page_table_node
 * @param page_table
 */
void dlist_free_page_table_node(void* page_table) {
    assert(page_table);
    free(((page_table_node_t*)page_table)->page_table_pointer);
    free(page_table);
}

/**
 * Simulates the process of loading a page from disk to memory.
 * It reduces remaining loading time by 1.
 * @param memory_manager
 * @param process
 */
void virtual_memory_load_process(virtual_memory_t* memory_manager, process_t* process) {
    Node* current = memory_manager->page_tables->head;
    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (page_table->pid == process->pid) {
            page_table->loading_time_left -= 1;
            fprintf(stderr, "Process %lld is loading. ETA: %lld ticks", process->pid, page_table->loading_time_left);
            return;
        }
        current = current->next;
    }
}

/**
 * Create a mapping from a frame number to a virtual address of a process.
 * @param page_table
 * @param frame_number
 * @return
 */
long long int map(page_table_node_t * page_table, long long int frame_number) {
    for (long long int i=0; i<page_table->page_count; i++) {
        if (page_table->page_table_pointer[i].validity == 0) {
            page_table->page_table_pointer[i].validity = 1;
            page_table->page_table_pointer[i].frame_number = frame_number;
            page_table->valid_page_count += 1;
            return 1;
        }
    }
    return 0;
}

/**
 * Print page table for debugging purpose
 * @param page_table
 * @param verbose
 */
void print_page_table(page_table_node_t* page_table, bool verbose) {
    printf("pid: %lld %lld/%lld last access: %lld\n", page_table->pid, page_table->valid_page_count, page_table->page_count, page_table->last_access);
    for (long long int i=0; i<page_table->page_count; i++) {
        printf("%llu: [%lld] %lld %c\t", i, page_table->page_table_pointer[i].validity, page_table->page_table_pointer[i].frame_number, page_table->page_table_pointer[i].reference==1?'R':' ');
    }
    printf("\n");
}

/**
 * Create a data structure to manage virtual memory.
 * @param memory_size
 * @param page_size
 * @return
 */
virtual_memory_t* create_virtual_memory(long long int memory_size, long long int page_size) {
    virtual_memory_t* memory = (virtual_memory_t*)malloc(sizeof(*memory));
    memory->page_size = page_size;
    memory->total_frame = memory_size/page_size;
    memory->free_frame = memory_size/page_size;
    memory->page_tables = new_dlist(dlist_free_page_table_node, (void (*)(void *)) print_page_table);
    memory->page_frames = malloc(sizeof(memory->page_frames) * memory->total_frame);
    memory->counter = malloc(sizeof(memory->counter) * memory->total_frame);
    for (long long int i=0; i<memory->total_frame; i++) {
        memory->page_frames[i] = NOT_OCCUPIED;
        memory->counter[i] = 0;
    }
    return memory;
}

/**
 * Free the data structure of virtual memory management
 * @param memory_manager
 */

void free_memory(virtual_memory_t *memory_manager) {
    free_dlist(memory_manager->page_tables);
    free(memory_manager->page_frames);
    free(memory_manager->counter);
    free(memory_manager);
}

/**
 * returns page table of a process.
 * NULL if not found in memory.
 * @param memory_manager
 * @param process
 * @return
 */
page_table_node_t* get_page_table(virtual_memory_t* memory_manager, long long int pid) {
    Node* current = memory_manager->page_tables->head;
    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (pid == page_table->pid) {
            return page_table;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Allocate all free memory to the given process
 * @param memory_manager
 * @param allocated
 * @return
 */
long long int allocate_all_free_memory(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    assert(page_table);
    long long int newly_allocated = 0;
    while (memory_manager->free_frame > 0 && page_table->valid_page_count < page_table->page_count) {
        for (long long int i = 0; i < memory_manager->total_frame; i++) {
            if (memory_manager->page_frames[i] == NOT_OCCUPIED) {
                memory_manager->page_frames[i] = page_table->pid;
                memory_manager->counter[i] = 0;
                map(page_table, i);
                memory_manager->free_frame -= 1;
                newly_allocated++;
                /* Increase loading time */
                page_table->loading_time_left += LOADING_TIME_PER_PAGE;
                /* Break the inner for loop to check if enough memory has been allocated */
                break;
            }
        }
    }
    return newly_allocated;
}

/**
 * returns a frame number to evict using least recently used algorithm.
 */
long long int LRU(virtual_memory_t* memory_manager, long long int ignore) {
    long long int victim_pid = find_the_oldest_process(memory_manager, ignore);
    long long int frame_number = first_page(memory_manager, victim_pid);
    assert(frame_number>=0);
    return frame_number;
}

/**
 * returns a frame number to evict using least frequently used with aging.
 * @param memory_manager
 * @param ignore
 * @return
 */
long long int LFU(virtual_memory_t* memory_manager, long long int ignore) {
    long long int victim_pid = -1;
    long long int min_freq = INT_MAX;
    for (long long int i=0; i<memory_manager->total_frame; i++) {
        if (memory_manager->counter[i] < min_freq && memory_manager->page_frames[i] != ignore) {
            min_freq = memory_manager->counter[i] < min_freq;
            victim_pid = memory_manager->page_frames[i];
        }
    }

    long long int frame_number = first_page(memory_manager, victim_pid);
    assert(frame_number>=0);
    return frame_number;
}

/**
 * Allocate memory to a process. Evicting pages using LRU if memory is not sufficient.
 * @param memory_manager
 * @param process
 * @param clock
 */
void virtual_memory_allocate_memory_LRU(virtual_memory_t* memory_manager, process_t* process, long long int clock) {
    /* convert bytes to page counts */
    long long int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
    long long int allocation_target = page_required>MIN_PAGE_REQUIRED_TO_RUN?MIN_PAGE_REQUIRED_TO_RUN: page_required;

    page_table_node_t* allocated = get_page_table(memory_manager, process->pid);

    /* Create a page table for the process if not exist */
    if (!allocated) {
        allocated = create_page_table_node(process->pid, page_required);
        dlist_add_end(memory_manager->page_tables, allocated);
    }
    allocate_all_free_memory(memory_manager, process);

    /* The number of pages must be evicted to let the process run */
    long long int evict_page_count = allocation_target - allocated->valid_page_count;
    long long int* to_print = malloc(sizeof(*to_print) * evict_page_count);
    long long int index = 0;

    /* Evict pages if memory allocated isn't enough for execution */
    while (allocated->valid_page_count < allocation_target){
        long long int victim = LRU(memory_manager, allocated->pid);

        to_print[index++] = evict_one_page(memory_manager, victim);
        allocate_all_free_memory(memory_manager, process);
    }
    if (evict_page_count > 0) {
        printf("%lld, EVICTED, mem-addresses=", clock);
        print_memory(to_print, evict_page_count);
        printf("\n");
    }
    free(to_print);
}

/**
 * Allocate memory to a process. Evicting pages using LFU if memory is not sufficient.
 * @param memory_manager
 * @param process
 * @param clock
 */
void virtual_memory_allocate_memory_LFU(virtual_memory_t* memory_manager, process_t* process, long long int clock) {
    /* convert bytes to page counts */
    long long int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
    long long int allocation_target = page_required>MIN_PAGE_REQUIRED_TO_RUN?MIN_PAGE_REQUIRED_TO_RUN: page_required;

    page_table_node_t* allocated = get_page_table(memory_manager, process->pid);

    /* Create a page table for the process if not exist */
    if (!allocated) {
        allocated = create_page_table_node(process->pid, page_required);
        dlist_add_end(memory_manager->page_tables, allocated);
    }
    allocate_all_free_memory(memory_manager, process);

    /* The number of pages must be evicted to let the process run */
    long long int evict_page_count = allocation_target - allocated->valid_page_count;
    if (evict_page_count > 0) {
        long long* to_print = malloc(sizeof(*to_print) * evict_page_count);
        long long int index = 0;

        /* Evict pages if memory allocated isn't enough for execution */
        while (allocated->valid_page_count < allocation_target){
            long long int victim = LFU(memory_manager, allocated->pid);

            to_print[index++] = evict_one_page(memory_manager, victim);
            allocate_all_free_memory(memory_manager, process);
        }
        if (evict_page_count > 0) {
            printf("%lld, EVICTED, mem-addresses=", clock);
            print_memory(to_print, evict_page_count);
            printf("\n");
        }
        free(to_print);
    }

}
/*
 * Returns the pid of the least recently executed process in memory
 * @param memory_manager
 * @return
 */
long long int find_the_oldest_process(virtual_memory_t* memory_manager, long long int skip) {
    assert(memory_manager);
    long long int max_time = INT_MAX;
    page_table_node_t* page_table_to_return = NULL;
    Node* current = memory_manager->page_tables->head;

    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (page_table->pid != skip && page_table->valid_page_count > 0 && page_table->last_access < max_time) {
            max_time = page_table->last_access;
            page_table_to_return = page_table;
        }
        current = current->next;
    }
    assert(page_table_to_return);
    return page_table_to_return->pid;
}

/**
 * Return the frame number of the first frame of a process in memory
 * @param memory_manager
 * @param pid
 * @return
 */
long long int first_page(virtual_memory_t* memory_manager, long long int pid) {
    for (long long int i=0; i<memory_manager->total_frame; i++) {
        // Find a page that's mapped into a page frame
        if (memory_manager->page_frames[i] == pid) {
            return i;
        }
    }
    return -1;
}

/**
 * Destroy the mapping from a frame number into a virtual address of a process
 * @param memory_manager
 * @param page_table
 * @param frame_number
 */
void unmap(virtual_memory_t* memory_manager, page_table_node_t* page_table, long long int frame_number) {
    for (long long int i=0; i<page_table->page_count; i++) {
        if (page_table->page_table_pointer[i].validity == 1 && page_table->page_table_pointer[i].frame_number == frame_number) {
            // Set page frame to -1, indicating not occupied
            memory_manager->page_frames[frame_number] = NOT_OCCUPIED;
            memory_manager->counter[frame_number] = 0;
            page_table->page_table_pointer[i].validity = 0;
            page_table->page_table_pointer[i].frame_number = -1;
            page_table->valid_page_count -= 1;
            memory_manager->free_frame += 1;
        }
    }
}
/**
 * Evicts the given frame from memory
 * @param memory_manager
 * @param frame_number
 * @return
 */
long long int evict_one_page(virtual_memory_t* memory_manager, long long int frame_number) {
    page_table_node_t* page_table = get_page_table(memory_manager, memory_manager->page_frames[frame_number]);
    unmap(memory_manager, page_table, frame_number);
    return frame_number;
}

/**
 * Print page frames for debugging purpose
 * @param memory_manager
 */
void print_page_frames(virtual_memory_t* memory_manager) {
    for (long long int i=0; i<memory_manager->total_frame; i++) {
        printf("%llu %lld %d\n", i, memory_manager->page_frames[i], memory_manager->counter[i]);
    }
}

/**
 * Simulate the use of  memory
 * This internally updated last access time of the fragment and the frequency counter of its pages.
 * @param memoryList
 * @param process
 * @param clock
 */
void virtual_use_memory(virtual_memory_t* memory_manager, process_t* process, long long int clock) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    page_table->last_access = clock;
    /*
     * Set the reference bits to 1
     */
    for (long long int i=0; i<page_table->page_count; i++) {
        page_table->page_table_pointer[i].reference = 1;
    }
    aging(memory_manager);

}

/**
 * Print status of a process and its memory usage
 * @param memory_manager
 * @param process
 * @param clock
 */
void virtual_process_info(virtual_memory_t* memory_manager, process_t* process, long long int clock) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%lld, mem-usage=%lld%%, ",
           clock,
           process->pid,
           process->remaining_time,
           page_table->loading_time_left,
           virtual_memory_usage(memory_manager));
    virtual_print_addresses(memory_manager, process);
}

/**
 * Returns memory usage as a percentage
 * @param memory_manager
 * @return
 */
long long int virtual_memory_usage(virtual_memory_t* memory_manager) {
    return ceil(100 * (double)(memory_manager->total_frame - memory_manager->free_frame) / (double)memory_manager->total_frame);
}

/**
 * Frees memory allocated to a process
 * @param memory_manager
 * @param process
 * @return
 */
long long int virtual_memory_free_memory(virtual_memory_t* memory_manager, process_t* process, long long int clock) {
    page_table_node_t* page_table= get_page_table(memory_manager, process->pid);
    assert(page_table);
    long long int page_to_free = page_table->valid_page_count;
    long long int* to_print = malloc(sizeof(*to_print) * page_to_free);
    long long int index = 0;

    long long int free_counter = 0;
    for (long long int i=0; i<page_table->page_count; i++) {
        // Find a page that's mapped into a page frame
        if (page_table->page_table_pointer[i].validity == 1 && page_table->page_table_pointer[i].frame_number != NOT_OCCUPIED) {
            // Set page frame to -1, indicating not occupied
            to_print[index++] = page_table->page_table_pointer[i].frame_number;
            memory_manager->page_frames[page_table->page_table_pointer[i].frame_number] = NOT_OCCUPIED;
            memory_manager->counter[page_table->page_table_pointer[i].frame_number] = 0;
            page_table->page_table_pointer[i].frame_number = -1;
            page_table->page_table_pointer[i].validity = 0;
            page_table->valid_page_count -= 1;
            memory_manager->free_frame += 1;
            free_counter++;
        }
    }
    printf("%lld, EVICTED, mem-addresses=", clock);
    print_memory(to_print, page_to_free);
    printf("\n");
    free(to_print);
    fprintf(stderr, "<Memory> Deallocate %lld virtual pages of process %lld\n",
              free_counter,
              page_table->pid);
    return free_counter;
}

/**
 * Print pages allocated to a process
 * @param memory_manager
 * @param process
 */
void virtual_print_addresses(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    assert(page_table);
    long long int* addr_to_print = malloc(sizeof(*addr_to_print) * page_table->valid_page_count);
    long long int index = 0;
    for (long long int i=0; i<memory_manager->total_frame; i++) {
        if (memory_manager->page_frames[i] == process->pid) {
            addr_to_print[index] = i;
            index++;
        }
    }
    printf("mem-addresses=");
    print_memory(addr_to_print, page_table->valid_page_count);
    printf("\n");
    free(addr_to_print);
}

/**
 * Returns if a process has all memory it needed
 * @param memory_manager
 * @param process
 * @return
 */
long long int virtual_require_allocation(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    long long int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
    if (page_table) {
        /*
         * if all pages a process needs has been allocated, return 0;
         */
        if (page_table->valid_page_count == page_required) {
            return 0;
        }
        /*
         * Returns the number of pages needs to be allocated
         */
        else {
            return page_required - page_table->valid_page_count;
        }
    }
    /**
     * If the process hasn't been allocated any memory, return -1;
     */
    else {
        return -1;
    }
}
/**
 * Returns how many ticks remaining to load page from disk
 * @param memory_manager
 * @param process
 * @return
 */
long long int virtual_load_time_left(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    assert(page_table);
    return page_table->loading_time_left;
}

/**
 * Estimates the number of page faults during executing.
 * The process must has a page table
 * @param memory_manager
 * @param process
 * @return the number of pages not loaded in memory, returns -1 if process is not found in memory
 *
 */
long long int virtual_page_fault(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process->pid);
    assert(page_table);
    return page_table->page_count - page_table->valid_page_count;
}


/**
 * Create an implementation of memory allocator for virtual memory using LRU
 * @param memory_size
 * @param page_size
 * @return
 */
memory_allocator_t* create_virtual_memory_allocator_LRU(long long int memory_size, long long int page_size) {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    allocator->malloc = (void *(*)(void *, process_t *, long long int)) virtual_memory_allocate_memory_LRU;
    allocator->use = (void (*)(void *, process_t *, long long int)) virtual_use_memory;
    allocator->info = (void (*)(void *, process_t *, long long int)) virtual_process_info;
    allocator->free = (void (*)(void *, process_t *, long long int)) virtual_memory_free_memory;
    allocator->load = (void (*)(void *, process_t *)) virtual_memory_load_process;
    allocator->load_time_left = (long long int (*)(void *, process_t *)) virtual_load_time_left;
    allocator->require_allocation = (long long int (*)(void *, process_t *)) (long long int (*)(void *,
                                                                             process_t *)) virtual_require_allocation;
    allocator->page_fault = (long long int (*)(void *, process_t *)) virtual_page_fault;
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = create_virtual_memory(memory_size, page_size);
    return allocator;
}

/**
 * Create an implementation of memory allocator for virtual memory using LFU
 * @param memory_size
 * @param page_size
 * @return
 */
memory_allocator_t* create_virtual_memory_allocator_LFU(long long int memory_size, long long int page_size) {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    allocator->malloc = (void *(*)(void *, process_t *, long long int)) virtual_memory_allocate_memory_LFU;
    allocator->use = (void (*)(void *, process_t *, long long int)) virtual_use_memory;
    allocator->info = (void (*)(void *, process_t *, long long int)) virtual_process_info;
    allocator->free = (void (*)(void *, process_t *, long long int)) virtual_memory_free_memory;
    allocator->load = (void (*)(void *, process_t *)) virtual_memory_load_process;
    allocator->load_time_left = (long long int (*)(void *, process_t *)) virtual_load_time_left;
    allocator->require_allocation = (long long int (*)(void *, process_t *)) (long long int (*)(void *,
                                                                            process_t *)) virtual_require_allocation;
    allocator->page_fault = (long long int (*)(void *, process_t *)) virtual_page_fault;
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = create_virtual_memory(memory_size, page_size);
    return allocator;
}

/**
 * Reduces the frequency of pages according to its usage.
 * @param memory_manager
 */
void aging(virtual_memory_t* memory_manager) {

    Node* curr = memory_manager->page_tables->head;
    while (curr) {
        page_table_node_t* page_table = (page_table_node_t*)curr->data;
        assert(page_table);
        for (long long int i=0; i<page_table->page_count; i++) {
            if (page_table->page_table_pointer[i].validity) {
                long long int frame_number = page_table->page_table_pointer[i].frame_number;
                /* shift the counter 1 bit right */
                memory_manager->counter[frame_number] >>= 0x1;
                /* and put 1 if the R bit is 1 */
                if (page_table->page_table_pointer[i].reference == 1) {
                    memory_manager->counter[frame_number] |= 0x1 << 7;
                    page_table->page_table_pointer[i].reference = 0;
                }
            }
        }
        curr = curr->next;
    }
}