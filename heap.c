#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
/*
 Array Implementation of MinHeap data Structure
*/

int heapSize(heap_t* heap) {
    return heap->count;
}

heap_t *createHeap(int capacity,int heap_type){
    heap_t *h = (heap_t * ) malloc(sizeof(*h)); //one is number of heap

    //check if memory allocation is fails
    if(h == NULL){
        printf("Memory Error!");
        return NULL;
    }
    h->heap_type = heap_type;
    h->count=0;
    h->capacity = capacity;
    h->arr = (data *) malloc(capacity*sizeof(data)); //size in bytes

    //check if allocation succeed
    if ( h->arr == NULL){
        printf("Memory Error!");
        return NULL;
    }
    return h;
}

void insert(heap_t *h, data key){
    if( h->count < h->capacity){
        h->arr[h->count] = key;
        heapifyBottomTop(h, h->count);
        h->count++;
    }
}

int cmpData(data a, data b) {
    return a.jobTime - b.jobTime;
}

void heapifyBottomTop(heap_t *h,int index){
    data temp;
    int parent_node = (index-1)/2;

    if (cmpData(h->arr[parent_node], h->arr[index])){
        //swap and recursive call
        temp = h->arr[parent_node];
        h->arr[parent_node] = h->arr[index];
        h->arr[index] = temp;
        heapifyBottomTop(h,parent_node);
    }
}

void heapifyTopBottom(heap_t *h, int parent_node){
    int left = parent_node*2+1;
    int right = parent_node*2+2;
    int min;
    data temp;

    if(left >= h->count || left <0)
        left = -1;
    if(right >= h->count || right <0)
        right = -1;

//    if(left != -1 && h->arr[left] < h->arr[parent_node]) {
    if(left != -1 && cmpData(h->arr[left], h->arr[parent_node]) < 0) {
        min=left;
    }
    else {
        min = parent_node;
    }
//    if(right != -1 && h->arr[right] < h->arr[min])
    if(right != -1 && cmpData(h->arr[right], h->arr[min]) < 0)
        min = right;

    if(min != parent_node){
        temp = h->arr[min];
        h->arr[min] = h->arr[parent_node];
        h->arr[parent_node] = temp;

        // recursive  call
        heapifyTopBottom(h, min);
    }
}

data popMin(heap_t *h){
    data pop;
    if(h->count==0){
        printf("\n__Heap is Empty__\n");
    }
    // replace first node by last and delete last
    pop = h->arr[0];
    h->arr[0] = h->arr[h->count-1];
    h->count--;
    heapifyTopBottom(h, 0);
    return pop;
}

void print(heap_t *h){
    int i;
    printf("____________Print Heap_____________\n");
    for(i=0;i< h->count;i++){
        printProcess(&h->arr[i]);
    }
    printf("->__/\\__\n");
}