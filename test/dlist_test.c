//
// Created by Haswell on 19/05/2020.
//
#include "../src/dlist.h"
#include <assert.h>

int dlist_test_insert_after_between() {
    Dlist* list = new_dlist(empty_cleaner);
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;
    dlist_add_start(list, &data3);
    Node* head = dlist_add_start(list, &data2);

    Node* inserted = dlist_insert_after(list, head, &data1);
    assert((*(int*)(list->head->next->data)) == 1);
    assert(list->size==3);
    assert(list->head->next == inserted);
    assert(list->head == head);
}

int dlist_test_insert_after_head() {
    Dlist* list = new_dlist(empty_cleaner);
    int data1 = 1;
    int data3 = 3;
    Node* head = dlist_add_start(list, &data3);

    Node* inserted = dlist_insert_after(list, head, &data1);
    assert((*(int*)(list->head->next->data)) == 1);
    assert(list->size==2);
    assert(list->head->next == inserted);
    assert(list->head == head);
}

int dlist_test_insert_after_tail() {
    Dlist* list = new_dlist(empty_cleaner);
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    dlist_add_start(list, &data3);
    dlist_add_end(list, &data1);

    Node* oldTail = list->tail;

    Node* inserted = dlist_insert_after(list, list->tail, &data2);
    assert((*(int*)(list->tail->data)) == 2);
    assert(list->size==3);
    assert(list->tail->prev == oldTail);
    assert(list->tail==inserted);
}


int dlist_test() {
    dlist_test_remove_head();
    dlist_test_remove_tail();
    dlist_test_remove_middle();
    dlist_test_insert_after_head();
    dlist_test_insert_after_between();
    dlist_test_insert_after_tail();

    return 0;
}

void dlist_test_print(Dlist* list) {
    Node* curr = list->head;
    while (curr) {
        printf("%d\n", *(int*)curr->data);
        curr = curr -> next;
    }
}

int dlist_test_remove_head() {
    Dlist* list = new_dlist(empty_cleaner);
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    dlist_add_start(list, &data3);
    dlist_add_start(list, &data2);
    dlist_add_start(list, &data1);
    Node* secondHead = list->head->next;
    Node* tail = list->tail;
    dlist_remove(list, list->head);
    assert(list->head == secondHead);
    assert(list->tail == tail);
    assert(list->size==2);
    return 0;
}

int dlist_test_remove_tail() {
    Dlist* list = new_dlist(empty_cleaner);
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    dlist_add_start(list, &data3);
    dlist_add_start(list, &data2);
    dlist_add_start(list, &data1);
    Node* head = list->head;
    Node* secondTail = list->tail->prev;
    dlist_remove(list, list->tail);
    assert(list->size==2);
    assert(list->tail == secondTail);
    assert(list->head == head);
}

int dlist_test_remove_middle() {
    Dlist* list = new_dlist(empty_cleaner);
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    dlist_add_start(list, &data3);
    Node* mid = dlist_add_start(list, &data2);
    dlist_add_start(list, &data1);

    Node* head = list->head;
    Node* tail = list->tail;

    dlist_remove(list, mid);
    assert((*(int*)(list->head->next->data)) != 2);
    assert(list->size==2);
    assert(list->tail == tail);
    assert(list->head == head);
}
