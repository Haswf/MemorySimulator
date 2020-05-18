#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
/*
 Array Implementation of MinHeap data Structure
*/

int heap_size(heap_t* heap) {
    return heap->count;
}

heap_t *create_heap(int capacity, int (*cmp)(void *, void *)){
    heap_t *h = (heap_t * ) malloc(sizeof(*h)); //one is number of heap

    //check if memory allocation is fails
    if(h == NULL){
        printf("Memory Error!");
        return NULL;
    }
    h->count=0;
    h->capacity = capacity;
    h->arr = (data *) malloc(capacity*sizeof(data)); //size in bytes
    h->cmp = cmp;
    //check if allocation succeed
    if ( h->arr == NULL){
        printf("Memory Error!");
        return NULL;
    }
    return h;
}

void heap_insert(heap_t *h, data key){
    if( h->count < h->capacity){
        h->arr[h->count] = key;
        heapify_bottom_top(h, h->count);
        h->count++;
    }
}



void heapify_bottom_top(heap_t *h,int index){
    data temp;
    int parent_node = (index-1)/2;

    if (h->cmp(&(h->arr[parent_node]), &(h->arr[index]))){
        //swap and recursive call
        temp = h->arr[parent_node];
        h->arr[parent_node] = h->arr[index];
        h->arr[index] = temp;
        heapify_bottom_top(h,parent_node);
    }
}

void heapify_top_bottom(heap_t *h, int parent_node){
    int left = parent_node*2+1;
    int right = parent_node*2+2;
    int min;
    data temp;

    if(left >= h->count || left <0)
        left = -1;
    if(right >= h->count || right <0)
        right = -1;

    if(left != -1 && h->cmp(&h->arr[left], &h->arr[parent_node]) < 0) {
        min=left;
    }
    else {
        min = parent_node;
    }
    if(right != -1 && h->cmp(&h->arr[right], &h->arr[min]) < 0)
        min = right;

    if(min != parent_node){
        temp = h->arr[min];
        h->arr[min] = h->arr[parent_node];
        h->arr[parent_node] = temp;

        // recursive  call
        heapify_top_bottom(h, min);
    }
}

data heap_pop_min(heap_t *h){
    data pop;
    if(h->count==0){
        printf("\n__Heap is Empty__\n");
    }
    // replace first node by last and delete last
    pop = h->arr[0];
    h->arr[0] = h->arr[h->count-1];
    h->count--;
    heapify_top_bottom(h, 0);
    return pop;
}


void heap_print(heap_t *h, void (*print)(void *)){
    int i;
    for(i=0;i< h->count;i++){
        print(&h->arr[i]);
    }
}