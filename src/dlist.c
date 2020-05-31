/* * * * * * *
 * Module for creating and manipulating doubly-linked lists of Data
 *
 * created for Project 1 COMP20007 Design of Algorithms 2019
 * by Shuyang Fan <shuyangf@student.unimelb.edu.au>
 * derived from linked list module written by Matt Farrugia <matt.farrugia@unimelb.edu.au> */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "dlist.h"

void empty_cleaner(void* data) {

}
// helper function to create a new dlist and return its address
Dlist* new_dlist(void (*clean)(void *), void (*print)(void *)){
    Dlist *new = malloc(sizeof(Dlist));
    assert(new);
    new->print = print;
    new->head = NULL;
    new->tail = NULL;
    new->size = 0;
    new->clean = clean;
    return new;
}

void print_dlist(Dlist* list) {
    Node *curr = list->head;
    // free list node by node
    while (curr) {
        // record next node
        list->print(curr->data);
        curr = curr->next;
    }
}
// free a dlist node by node
void free_dlist(Dlist *ddl) {
    assert(ddl != NULL);
    Node *curr = ddl->head;
    Node *next;
    // free list node by node
    while (curr) {
        // record next node
        next = curr->next;
        free_node(curr, ddl->clean);
        curr = next;
    }
    // free the dlist itself
    free(ddl);
}

// // helper function to print from head to tail
// void forward_print(Dlist *ddl){
//     assert(ddl != NULL);
//     Node *curr = ddl->head;
//     while (curr) {
//         print_point(curr->data);
//         curr = curr->next;
//     }
//     printf("\n");
// }
//
// // helper function to print from tail to head
// void backward_print(Dlist *ddl){
//     assert(ddl != NULL);
//     Node *curr = ddl->tail;
//     while (curr) {
//         print_point(curr->data);
//         curr = curr->prev;
//     }
//     printf("\n");
// }


// helper function to clear memory of a node
void free_node(Node *node, void (*clean)(void *)) {
    clean(node->data);
    free(node);
}

// add an element to the front of a list
// This operation is O(1)
Node* dlist_add_start(Dlist *ddl, Data data){
    assert(ddl != NULL);

    // create a new node
    Node *new = new_node();
    new->data = data;
    new->next = ddl->head; // next will be the old first node (may be null)
    new->prev = NULL; // head has no prev node

    // if dlist was empty, this new node is also the last node now
    if (ddl->size == 0) {
        ddl->tail = new;
    }

    // if list was not empty, change prev of old head to new head.
    else {
        ddl->head->prev = new;
        }

    // Change head to new node
    ddl->head = new;

    // keep size updated!
    ddl->size++;
    return new;
}

// add an element to the back of a list
// This operation is O(1)
Node* dlist_add_end(Dlist *ddl, Data data) {
    assert(ddl != NULL);

    // we'll need a new list node to store this data
    Node *new = new_node();
    new->data = data;
    new->next = NULL; // as the last node, there's no next node
    new->prev = ddl->tail; // its prev was the old tail

    if(ddl->size == 0) {
        // if the list was empty, new node is now the first node
        ddl->head = new;
    } else {
        // otherwise, it goes after the current last node
        ddl->tail->next = new;
    }

    // place this new node at the end of the list
    ddl->tail = new;

    // and keep size updated too
    ddl->size++;
    return new;
}

// remove and return the first element from a doubly linked list
// this operation is O(1)
// error if the list is empty (so first ensure list_size() > 0)
Data dlist_remove_start(Dlist *ddl) {
    assert(ddl != NULL);
    assert(ddl->size > 0);

    // we'll need to save the data to return it
    Node *curr = ddl->head;
    Data data = curr->data;

    // if this was the last node in the list, the tail also needs to be cleared
    if(ddl->size == 1) {
        ddl->tail = NULL;
    }
    else{
        // if this was not the last node, change prev of next node
        ddl->head->next->prev = NULL;
    }

    // then replace the head with its next node (may be null)
    // if the head has next node
    ddl->head = ddl->head->next;

    // decrement size by one
    ddl->size--;

    // free node
    free_node(curr, ddl->clean);

    // return data retrieved
    return data;
}

// Malloc a new node, return its address
Node *new_node() {
    Node *node = malloc(sizeof *node);
    assert(node);
    return node;
}


Node* dlist_insert_after(Dlist *ddl, Node* after, Data newData) {
    assert(ddl != NULL);
    assert(ddl->size > 0);
    Node* current = ddl->head;
    while (current) {
        if (current == after) {
            Node* new = (Node*)malloc(sizeof(*new));
            new->data = newData;
            new->prev = current;
            new->next = current->next;
            if (current->next) {
                current->next->prev = new;
            }
            current->next = new;
            if (new->prev == NULL) {
                ddl->head = new;
            }
            else if (new->next == NULL) {
                ddl->tail = new;
            }
            ddl->size++;
            return new;
        }
        current = current -> next;
    }
    return NULL;
}

Node* dlist_remove(Dlist *ddl, Node* toRemove) {
    assert(ddl != NULL);
    assert(ddl->size > 0);

    if (toRemove->prev) {
        toRemove->prev->next = toRemove->next;
    } else {
        ddl->head = toRemove->next;
    }
    if (toRemove->next) {
        toRemove->next->prev = toRemove->prev;
    } else {
        ddl->tail = toRemove->prev;
    }
    free_node(toRemove, ddl->clean);

    ddl->size--;
    return NULL;
}
// Remove the last element in a doubly linked list.
// This operation is (1)
// Make sure list has at least 1 element.
Data dlist_remove_end(Dlist *ddl) {
    assert(ddl != NULL);
    assert(ddl->size > 0);

    // we'll need to save the data to return it
    Node *curr = ddl->tail;
    Data data = curr->data;

    if(ddl->size == 1) {
        // if we're removing the last node, the head also needs clearing
        ddl->head = NULL;
    } else {
        // otherwise, the second-last node needs to drop the removed last node
        ddl->tail->prev->next = NULL;
    }

    // then replace the tail with the second-last node.
    ddl->tail = ddl->tail->prev;

    // decrement size by 1
    ddl->size--;

    // we're finished with the list node holding this data
    free_node(curr, ddl->clean);

    // done!
    return data;
}

// Return the size of given dlist
int dlist_size(Dlist*ddl){
    return ddl->size;
}



