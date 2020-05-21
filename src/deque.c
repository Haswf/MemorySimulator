/* * * * * * *
 * Deque module (i.e., double ended queue) for Assignment 1
 *
 * created for COMP20007 Design of Algorithms 2019
 * template by Tobias Edwards <tobias.edwards@unimelb.edu.au>
 * implementation by Shuyang Fan
 */

//                   WRITE YOUR IMPLEMENTATION HERE
//
// You should fill in the function definitions for
//  - new_deque()
//  - free_deque()
//  - deque_push()
//  - deque_insert()
//  - deque_pop()
//  - deque_remove()
//  - deque_size()
//
// Don't be shy to add any extra functions or types you may need.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "deque.h"

// Create a new empty Deque and return a pointer to it
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
Deque *new_deque(void (*print)(void *)) {
    Deque *newdq = (Deque*)malloc(sizeof(Deque));
    /* We don't want the linked list to free data it contains in this case,
     * so a empty cleaner is provided */
    newdq->list = new_dlist(empty_cleaner, print);
    return newdq;
}

// Free the memory associated with a Deque
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
void free_deque(Deque *deque) {
    // call free_dlist to free each node in the dlist
    free_dlist(deque->list);
    // free the structure itself
    free(deque);
}

// Add a Point to the top of a Deque
//
// TODO: Fill in the runtime of this function
// Runtime: O(1)
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
void deque_push(Deque *deque, tNode* data) {
    dlist_add_end(deque->list, data);
}

// Add a Point to the bottom of a Deque
//
// TODO: Fill in the runtime of this function
// Runtime: O(1)
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
void deque_insert(Deque *deque, tNode* data) {
    // TODO: Implement deque_insert()
    dlist_add_start(deque->list, data);
}

// Remove and return the top Point from a Deque
//
// TODO: Fill in the runtime of this function
// Runtime: O(1)
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
tNode* deque_pop(Deque *deque) {
    // TODO: Implement deque_pop()
    return dlist_remove_end(deque->list);
}

void print_deque(Deque *deque){
    print_dlist(deque->list);
}

tNode* last_to_pop(Deque *deque) {
    if (deque->list->head) {
        return deque->list->head->data;
    }
    return NULL;
}

tNode* next_to_pop(Deque *deque) {
    if (deque->list->tail) {
        return deque->list->tail->data;
    }
    return NULL;
}

// Remove and return the bottom Point from a Deque
//
// TODO: Fill in the runtime of this function
// Runtime: O(1)
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
tNode* deque_remove(Deque *deque) {
    // TODO: Implement deque_remove()
    return dlist_remove_start(deque->list);
}

// Return the number of Points in a Deque
//
// TODO: Fill in the runtime of this function
// Runtime: O(1)
//
// DO NOT CHANGE THIS FUNCTION SIGNATURE
int deque_size(Deque *deque) {
    return dlist_size(deque->list);
}