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

int allocate_page(page_table_node_t * page_table, int frame_number) {
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
        printf("%d: [%d] %d\n", i, page_table->page_table_pointer[i].validity, page_table->page_table_pointer[i].frame_number);
    }
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

void virtual_memory_allocate_memory(virtual_memory_t* memory_manager, process_t* process) {
    /* Find the minimum number of pages required to execute this process */
    int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
    int allocation_target = page_required>MIN_PAGE_REQUIRED_TO_RUN?MIN_PAGE_REQUIRED_TO_RUN: page_required;
    page_table_node_t* allocated = get_page_table(memory_manager, process);
    /* If enough page has been allocated in memory, just return*/
    if (allocated && allocated->valid_page_count >= page_required) {
        log_trace("<Memory> process %d has %d/%d pages in memory. No new allocation is needed.", process->pid, allocated->valid_page_count, allocated->page_count);
        return;
    }
    /* Create a page table for the process if not exist */
    if (!allocated) {
        allocated = create_page_table_node(process->pid, page_required);
    }

    log_trace("<Memory> Minimum allocation for process %d is %d pages", process->pid, allocation_target);
    int newly_allocated = 0;

    while (allocated->valid_page_count < allocation_target){
        while (memory_manager->free_frame > 0 && allocated->valid_page_count < allocated->page_count) {
            for (int i=0; i<memory_manager->total_frame; i++) {
                if (memory_manager->page_frames[i] < 0){
                    memory_manager->page_frames[i] = process->pid;
                    allocate_page(allocated, i);
                    memory_manager->free_frame-=1;
                    newly_allocated++;
                    // Break the inner for loop to check if enough memory has been allocated
                    break;
                }
            }
        }
        if (memory_manager->free_frame == 0 && allocated->valid_page_count < allocation_target) {
            page_table_node_t* page = find_least_recently_executed_process(memory_manager, process);
            assert(page->pid != process->pid);
            deallocate_one_page(memory_manager, page);
        }
    }
    /* Calculate loading time based on how many page has been allocated */
    allocated->loading_time_left += LOADING_TIME_PER_PAGE * newly_allocated;
    log_trace("<Memory> %d/%d pages allocated for process %d. Loading requires %d ticks", newly_allocated, allocated->page_count, process->pid, allocated->loading_time_left);
    // Add the page table to the linked list. This avoids the case where the newly created table happens to be the
    // least recently executed b/c the default last access time is -1
    dlist_add_end(memory_manager->page_tables, allocated);
}

/**
 * Returns the pid of the least recently execuated process
 * @param memory_manager
 * @return
 */
page_table_node_t* find_least_recently_executed_process(virtual_memory_t* memory_manager, process_t* process) {
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
    assert(page_table_to_return);
    return page_table_to_return;
}

int deallocate_one_page(virtual_memory_t* memory_manager, page_table_node_t* page_table) {
    for (int i=0; i<page_table->page_count; i++) {
        // Find a page that's mapped into a page frame
        if (page_table->page_table_pointer[i].validity == 1 && page_table->page_table_pointer[i].frame_number > -1) {
            log_trace("<Memory> Deallocate virtual page %d (actual frame: %d) of process %d last access: %d",
                    i,
                    page_table->page_table_pointer[i].frame_number,
                    page_table->pid,
                    page_table->last_access);
            // Set page frame to -1, indicating not occupied
            memory_manager->page_frames[page_table->page_table_pointer[i].frame_number] = NOT_OCCUPIED;
            page_table->page_table_pointer[i].frame_number = -1;
            page_table->page_table_pointer[i].validity = 0;
            page_table->valid_page_count -= 1;
            memory_manager->free_frame += 1;
            return 1;
        }
    }
    return 0;
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

int virtual_memory_usage(virtual_memory_t* memory_manager) {
    return ceil((double)(memory_manager->total_frame - memory_manager->free_frame) / (double)memory_manager->total_frame);
}

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

int virtual_require_allocation(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process);
    int page_required = byteToRequiredPage(process->memory, memory_manager->page_size);
    int allocation_target = page_required>MIN_PAGE_REQUIRED_TO_RUN?MIN_PAGE_REQUIRED_TO_RUN:page_required ;
    if (page_table) {
        if (page_table->valid_page_count >= allocation_target) {
            return 0;
        }
        else {
            return allocation_target - page_table->valid_page_count;
        }
    }
    else {
        return -1;
    }
}
int virtual_load_time_left(virtual_memory_t* memory_manager, process_t* process) {
    page_table_node_t* page_table = get_page_table(memory_manager, process);
    assert(page_table);
    return page_table->loading_time_left;
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
    // Unlimited allocator doesn't have a structure to manage memory;
    allocator->structure = create_virtual_memory(memory_size, page_size);
    return allocator;
}
