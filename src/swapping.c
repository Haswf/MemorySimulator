//
// Created by Haswell on 18/05/2020.
//

#include "swapping.h"
#include "../test/swapping_test.h"

memory_list_t* create_memory_list() {
    memory_list_t* m_list = (memory_list_t*)malloc(sizeof(*m_list));
    m_list->list = new_dlist(dlist_free_fragment);
    return m_list;
}

void init_memory_list(memory_list_t* memoryList, int mem_size) {
    assert(memoryList);
    memory_fragment_t* empty_memory = create_hole_fragment(0, mem_size);
    dlist_add_start(memoryList->list, empty_memory);
}

void free_memory_list(memory_list_t* memoryList) {
    assert(memoryList);
    free_dlist(memoryList->list);
    free(memoryList);
}

Node* first_fit(memory_list_t* memoryList, process_t* process) {
    assert(memoryList);
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*) current->data;
        if (fragment->type == HOLE_FRAGMENT && fragment->length > process->memory) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Node* allocate(memory_list_t* memoryList, Node* hole, process_t* process) {
    memory_fragment_t* fragment = (memory_fragment_t*)hole->data;

    dlist_insert_after(memoryList->list, hole, create_hole_fragment(
            fragment->start + process->memory,
            fragment->length - process->memory));

    fragment->length = process->memory;
    fragment->type = PROCESS_FRAGMENT;
    fragment->process = process;
    return hole;
}

void print_memory_list(memory_list_t* memoryList) {
    Node* current = memoryList->list->head;
    while (current) {
        print_fragment(current->data);
        current = current -> next;
    }
}
void free_memory_fragment(Node* nodeToFree) {
    memory_fragment_t* fragmentToFree = (memory_fragment_t*) nodeToFree->data;
    fragmentToFree->type = HOLE_FRAGMENT;
    fragmentToFree->process = NULL;
}

Node* join_prev(memory_list_t* memoryList, Node* nodeToEvict) {
    memory_fragment_t* fragmentToEvict = (memory_fragment_t*) nodeToEvict->data;
    memory_fragment_t* prevFragment = (memory_fragment_t*) nodeToEvict->prev->data;
    assert(fragmentToEvict->type==HOLE_FRAGMENT && prevFragment->type==HOLE_FRAGMENT);
    int total_size = prevFragment->length + fragmentToEvict->length;
    prevFragment->length = total_size;
    Node* merged = nodeToEvict->prev;
    dlist_remove(memoryList->list, nodeToEvict);
    return merged;
}

Node* join_next(memory_list_t* memoryList, Node* nodeToEvict) {
    memory_fragment_t* fragmentToEvict = (memory_fragment_t*) nodeToEvict->data;
    memory_fragment_t* nextFragment = (memory_fragment_t*) nodeToEvict->next->data;
    assert(fragmentToEvict->type == HOLE_FRAGMENT && nextFragment->type == HOLE_FRAGMENT);
    int total_size = fragmentToEvict->length + nextFragment->length;
    fragmentToEvict->length = total_size;
    Node* merged = nodeToEvict;
    dlist_remove(memoryList->list, nodeToEvict->next);
    return merged;
}

Node* find_least_recently_used(memory_list_t* memoryList) {
    Node* current = memoryList->list->head;
    memory_fragment_t* fragment = (memory_fragment_t*)current->data;
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

Node* evict(memory_list_t* memoryList, Node* nodeToEvict) {
    Node* merged = nodeToEvict;
    memory_fragment_t* fragmentToEvict = (memory_fragment_t*) nodeToEvict->data;
    assert(fragmentToEvict->type == PROCESS_FRAGMENT);
    free_memory_fragment(merged);
    if (merged->prev) {
        memory_fragment_t* prevFragment = (memory_fragment_t*) merged->prev->data;
        if (prevFragment->type == HOLE_FRAGMENT) {
            merged = join_prev(memoryList, merged);
        }
    }
    if (merged->next) {
        memory_fragment_t *nextFragment = (memory_fragment_t *) merged->next->data;
        if (nextFragment->type == HOLE_FRAGMENT) {
            merged = join_next(memoryList, merged);
        }
    }
    return merged;
}

void use_memory(memory_list_t* memoryList, int pid, int clock) {
    Node* current = memoryList->list->head;
    while (current) {
        memory_fragment_t* fragment = (memory_fragment_t*)current->data;
        if (fragment->type == PROCESS_FRAGMENT && fragment->process->pid == pid) {
            fragment->last_access = clock;
        }
        current = current->next;
    }
}

int main(){
   evict_test();

}