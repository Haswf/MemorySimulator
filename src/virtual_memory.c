//
// Created by Haswell on 22/05/2020.
//

#include "virtual_memory.h"
#include "output.h"

page_table_node_t* create_page_table_node(int pid, int page_count) {
    page_table_node_t* page = (page_table_node_t*)malloc(sizeof(*page));
    assert(page);
    page->page_count = page_count;
    page->pid = pid;
    page->valid_page_count = 0;
    page->loading_time_left = 0;
    page->page_table_pointer = malloc(sizeof(*page->page_table_pointer) * page_count);
    assert(page->page_table_pointer);
    page->last_access = -1;
    for (int i=0; i<page_count; i++) {
        page->page_table_pointer[i].frame_number = -1;
        page->page_table_pointer[i].referenced = 0;
        page->page_table_pointer[i].validity = 0;
    }
    return page;
}

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

void virtual_memory_load_process(virtual_memory_t* memory_manager, process_t* process) {
    Node* current = memory_manager->page_tables->head;
    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (page_table->pid == process->pid) {
            page_table->loading_time_left -= 1;
            log_trace("Process %d is loading. ETA: %d ticks", process->pid, page_table->loading_time_left);
            return;
        }
        current = current->next;
    }
}

int map_frame_to_virtual_page(page_table_node_t * page_table, int frame_number) {
    for (int i=0; i<page_table->page_count; i++) {
        if (page_table->page_table_pointer[i].validity == 0) {
            page_table->page_table_pointer[i].validity = 1;
            page_table->page_table_pointer[i].frame_number = frame_number;
            page_table->valid_page_count += 1;
            return 1;
        }
    }
    return 0;
}

void print_page_table(page_table_node_t* page_table, bool verbose) {
    printf("pid: %d %d/%d last access: %d\n", page_table->pid, page_table->valid_page_count, page_table->page_count, page_table->last_access);
    for (int i=0; i<page_table->page_count; i++) {
        printf("%d: [%d] %d\t", i, page_table->page_table_pointer[i].validity, page_table->page_table_pointer[i].frame_number);
    }
    printf("\n");
}

void init_memory(int* page_frames, int total_frame) {
    for (int i=0; i<total_frame; i++) {
        page_frames[i] = NOT_OCCUPIED;
    }
}

virtual_memory_t* create_virtual_memory(int memory_size, int page_size) {
    virtual_memory_t* memory = (virtual_memory_t*)malloc(sizeof(*memory));
    memory->page_size = page_size;
    memory->total_frame = memory_size/page_size;
    memory->free_frame = memory_size/page_size;
    memory->page_tables = new_dlist(dlist_free_page_table_node, (void (*)(void *)) print_page_table);
    memory->page_frames = (int*)malloc(sizeof(memory->page_frames) * memory->total_frame);
    init_memory(memory->page_frames, memory->total_frame);
    return memory;
}

/**
 * returns page table of a process.
 * NULL if not found in memory.
 * @param memory_manager
 * @param process
 * @return
 */
page_table_node_t* get_page_table(virtual_memory_t* memory_manager, process_t* process) {
    Node* current = memory_manager->page_tables->head;
    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (process->pid == page_table->pid) {
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
int allocate_all_free_memory(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* allocated = get_page_table(memory_manager, process);
    assert(allocated);
    int newly_allocated = 0;
    while (memory_manager->free_frame > 0 && allocated->valid_page_count < allocated->page_count) {
        for (int i = 0; i < memory_manager->total_frame; i++) {
            if (memory_manager->page_frames[i] == NOT_OCCUPIED) {
                memory_manager->page_frames[i] = allocated->pid;
                map_frame_to_virtual_page(allocated, i);
                memory_manager->free_frame -= 1;
                newly_allocated++;
                /* Increase loading time */
                allocated->loading_time_left += LOADING_TIME_PER_PAGE;
                /* Break the inner for loop to check if enough memory has been allocated */
                break;
            }
        }
    }
    return newly_allocated;
}

void virtual_memory_allocate_memory(virtual_memory_t* memory_manager, process_t* process) {
    /* convert bytes to page counts */
    int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
    int allocation_target = page_required>MIN_PAGE_REQUIRED_TO_RUN?MIN_PAGE_REQUIRED_TO_RUN: page_required;

    page_table_node_t* allocated = get_page_table(memory_manager, process);

    /* Create a page table for the process if not exist */
    if (!allocated) {
        allocated = create_page_table_node(process->pid, page_required);
        dlist_add_end(memory_manager->page_tables, allocated);
    }
    allocate_all_free_memory(memory_manager, process);;

     /* The number of pages must be evicted to let the process run */
    int evict_page_count = allocation_target - allocated->valid_page_count;
    int* to_print = malloc(sizeof(*to_print) * evict_page_count);
    int index = 0;

    /* Evict pages if memory allocated isn't enough for execution */
    while (allocated->valid_page_count < allocation_target){
        page_table_node_t* page = find_the_oldest_process(memory_manager, process);
        assert(page->pid != process->pid);
        to_print[index++] = deallocate_one_page(memory_manager, page);
        allocate_all_free_memory(memory_manager, process);
    }
    if (evict_page_count > 0) {
        printf("%d, EVICTED, mem-addresses=", 1);
        print_memory(to_print, evict_page_count);
        printf("\n");
    }
    free(to_print);
    log_trace("<Memory> %d pages allocated for process %d. Loading requires %d ticks", allocated->valid_page_count, allocated->page_count, process->pid, allocated->loading_time_left);
}

/*
 * Returns the pid of the least recently execuated process
 * @param memory_manager
 * @return
 */
page_table_node_t* find_the_oldest_process(virtual_memory_t* memory_manager, process_t* process) {
    assert(memory_manager);
    int max_time = INT_MAX;
    page_table_node_t* page_table_to_return = NULL;
    Node* current = memory_manager->page_tables->head;

    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (page_table->pid != process->pid && page_table->valid_page_count > 0 && page_table->last_access < max_time) {
            max_time = page_table->last_access;
            page_table_to_return = page_table;
        }
        current = current->next;
    }
//    print_dlist(memory_manager->page_tables);
//    printf("next process %d\n", page_table_to_return->pid);
    assert(page_table_to_return);
    return page_table_to_return;
}

int next_frame_to_deallocate(virtual_memory_t* memory_manager, int pid) {
    for (int i=0; i<memory_manager->total_frame; i++) {
        // Find a page that's mapped into a page frame
        if (memory_manager->page_frames[i] == pid) {
            return i;
        }
    }
}



int deallocate_one_page(virtual_memory_t* memory_manager, page_table_node_t* page_table) {
    int frame_number = next_frame_to_deallocate(memory_manager, page_table->pid);
    for (int i=0; i<page_table->page_count; i++) {
        if (page_table->page_table_pointer[i].validity == 1 && page_table->page_table_pointer[i].frame_number == frame_number) {
            // Set page frame to -1, indicating not occupied
            memory_manager->page_frames[frame_number] = NOT_OCCUPIED;
            page_table->page_table_pointer[i].validity = 0;
            page_table->page_table_pointer[i].frame_number = -1;
            page_table->valid_page_count -= 1;
            memory_manager->free_frame += 1;
        }
    }
    return frame_number;
}

void print_page_frames(virtual_memory_t* memory_manager, bool verbose) {
    int last_pid = -1;
    for (int i=0; i<memory_manager->total_frame; i++) {
        if (memory_manager->page_frames[i] != -1) {
            printf("%d %d\n", i, memory_manager->page_frames[i]);
            last_pid = memory_manager->page_frames[i];
        }
    }
}

void update_last_access(page_table_node_t* page_table, int clock) {
    page_table->last_access = clock;
}

/**
 *
 * @param memory_manager
 * @param process
 * @param clock
 */
void virtual_memory_use_memory(virtual_memory_t* memory_manager, process_t* process, int clock) {
    Node* current = memory_manager->page_tables->head;
    while (current) {
        page_table_node_t* page_table = (page_table_node_t*)current->data;
        if (page_table->pid == process->pid) {
            update_last_access(page_table, clock);
            return;
        }
        current = current->next;
    }
}

/**
 * Returns memory usage as a percentage
 * @param memory_manager
 * @return
 */
int virtual_memory_usage(virtual_memory_t* memory_manager) {
    return ceil((double)(memory_manager->total_frame - memory_manager->free_frame) / (double)memory_manager->total_frame);
}

/**
 * Frees memory allocated to a process
 * @param memory_manager
 * @param process
 * @return
 */
int virtual_memory_free_memory(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table= get_page_table(memory_manager, process);
    assert(page_table);
    int free_counter = 0;
    for (int i=0; i<page_table->page_count; i++) {
        // Find a page that's mapped into a page frame
        if (page_table->page_table_pointer[i].validity == 1 && page_table->page_table_pointer[i].frame_number > -1) {
            // Set page frame to -1, indicating not occupied
            memory_manager->page_frames[page_table->page_table_pointer[i].frame_number] = NOT_OCCUPIED;
            page_table->page_table_pointer[i].frame_number = -1;
            page_table->page_table_pointer[i].validity = 0;
            page_table->valid_page_count -= 1;
            memory_manager->free_frame += 1;
            free_counter++;
        }
    }
    log_trace("<Memory> Deallocate %d virtual pages of process %d",
              free_counter,
              page_table->pid);
    return free_counter;
}

void virtual_print_addresses(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process);
    assert(page_table);
    int* addr_to_print = malloc(sizeof(*addr_to_print) * page_table->valid_page_count);
    int index = 0;
    for (int i=0; i<memory_manager->total_frame; i++) {
        if (memory_manager->page_frames[i] == process->pid) {
            addr_to_print[index] = i;
            index++;
        }
    }
    print_memory(addr_to_print, page_table->valid_page_count);
    free(addr_to_print);
}

/**
 * Returns if a process has all memory it needed
 * @param memory_manager
 * @param process
 * @return
 */
int virtual_require_allocation(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process);
    int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
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
int virtual_load_time_left(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process);
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
int virtual_page_fault(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process);
    assert(page_table);
    return page_table->page_count - page_table->valid_page_count;
}



memory_allocator_t* create_virtual_memory_allocator(int memory_size, int page_size) {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    allocator->allocate_memory = (void *(*)(void *, process_t *)) virtual_memory_allocate_memory;
    allocator->use_memory = (void (*)(void *, process_t *, int)) virtual_memory_use_memory;
    allocator->free_memory = (void (*)(void *, process_t *)) virtual_memory_free_memory;
    allocator->load_memory = (void (*)(void *, process_t *)) virtual_memory_load_process;
    allocator->load_time_left = (int (*)(void *, process_t *)) virtual_load_time_left;
    allocator->require_allocation = (int (*)(void *, process_t *)) (int (*)(void *,
                                                                             process_t *)) virtual_require_allocation;
    allocator->page_fault = (int (*)(void *, process_t *)) virtual_page_fault;
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = create_virtual_memory(memory_size, page_size);
    return allocator;
}
