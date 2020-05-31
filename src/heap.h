//
// Created by Has on 14/05/2019.
//

#ifndef HEAP_H
#define HEAP_H


#include <stdlib.h>
#include <stdio.h>
#include "process.h"
typedef process_t data;

typedef struct Heap{
    data *arr;
    int count;
    int capacity;
    int (*cmp)(void *, void *);
} heap_t;
//typedef process_t data;
void heapify_bottom_top(heap_t *h,int index);
heap_t *create_heap(int capacity, int (*cmp)(void *, void *));
void heap_insert(heap_t *h, data key);
void heap_print(heap_t *h, void (*print)(void *));
data heap_pop_min(heap_t *h);
int heap_size(heap_t* heap);
void free_heap(heap_t *h);
#endif //HEAP_H
