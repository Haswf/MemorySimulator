/* * * * * * *
 * Module for creating and manipulating doubly-linked lists of Data
 *
 * created for Project 1 COMP20007 Design of Algorithms 2019
 * by Shuyang Fan <shuyangf@student.unimelb.edu.au>
 * derived from linked list module written by Matt Farrugia <matt.farrugia@unimelb.edu.au>
 */

#ifndef DLIST_H
#define DLIST_H
#include "process.h"
typedef struct dlist Dlist;
typedef struct node Node;
// Change data type stored in dlist
typedef void* Data;

struct node {
    Node* next;
    Node* prev;
    Data data;
};

struct dlist {
    Node *head;
    Node *tail;
    int size;
    void (*clean)(void *);
};

// Dlist Creation and deletion

// helper function to create a new dlist and return its address
Dlist* new_dlist(void (*clean)(void *));

//free a dlist node by node
void free_dlist(Dlist *ddl);

// Malloc a new node, return its address
Node *new_node();

// helper function to clear memory of a node
void free_node(Node *node, void (*clean)(void *));

// Manipulation

// add an element to the front of a list
Node* dlist_add_start(Dlist *ddl, Data data);

// add an element to the back of a list
Node* dlist_add_end(Dlist *ddl, Data data);

// remove and return the first element from a doubly linked list
Data dlist_remove_start(Dlist *ddl);

// Remove the last element in a doubly linked list.
Data dlist_remove_end(Dlist *ddl);

// Return the size of given dlist
int dlist_size(Dlist*ddl);

// helper function to print from head to tail
void forward_print(Dlist *ddl);

// helper function to print from tail to head
void backward_print(Dlist *ddl);

Node* dlist_insert_after(Dlist *ddl, Node* after, Data newData);

Node* dlist_remove(Dlist *ddl, Node* toRemove);

int dlist_test();

void empty_cleaner(void* data);

#endif //DLIST_H
