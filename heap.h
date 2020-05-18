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
    int heap_type; // 0 for min heap , 1 for max heap
} heap_t;
//typedef process_t data;
void heapifyBottomTop(heap_t *h,int index);
heap_t *createHeap(int capacity,int heap_type);
void insert(heap_t *h, data key);
void print(heap_t *h);
data popMin(heap_t *h);
int heapSize(heap_t* heap);

#endif //HEAP_H
