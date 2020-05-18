//
// Created by Haswell on 19/05/2020.
//

#include "swapping_test.h"
#include "../src/swapping.h"
#include "../src/memory_fragment.h"
#include <stdio.h>

/*
 *  Testcase for [Process | ToEvict| Process]
 */
int evict_test_in_PXP() {
    memory_list_t* mem_list = create_memory_list();
    init_memory_list(mem_list, 1000);
    process_t* process1 = create_process(1, 1, 20, 5);
    process_t* process2 = create_process(1, 2, 800, 5);
    process_t* process3 = create_process(1, 3, 100, 5);
    Node* after1 = first_fit(mem_list, process1);
    Node* allocated1 = allocate(mem_list, after1, process1);

    Node* after2 = first_fit(mem_list, process2);
    Node* allocated2 = allocate(mem_list, after2, process2);

    Node* after3 = first_fit(mem_list, process3);
    Node* allocate3 = allocate(mem_list, after3, process3);

    int oldLength = ((memory_fragment_t*)mem_list->list->head->next->data)->length;
    int oldStart = ((memory_fragment_t*)mem_list->list->head->next->data)->start;

    evict(mem_list, allocated2);
    assert(((memory_fragment_t*)mem_list->list->head->next->data)->type == HOLE_FRAGMENT);
    assert(((memory_fragment_t*)mem_list->list->head->next->data)->process == NULL);
    assert(((memory_fragment_t*)mem_list->list->head->next->data)->length == oldLength);
    assert(((memory_fragment_t*)mem_list->list->head->next->data)->start == oldStart);
}

/*
 * Testcase for [...| Hole | X| Process]
 * expected output: [...| Hole | Process]
 */
int evict_test_in_HXP() {
    memory_list_t* mem_list = create_memory_list();
    init_memory_list(mem_list, 1000);
    process_t* process1 = create_process(1, 1, 20, 5);
    process_t* process2 = create_process(1, 2, 800, 5);
    process_t* process3 = create_process(1, 3, 100, 5);
    Node* after1 = first_fit(mem_list, process1);
    Node* allocated1 = allocate(mem_list, after1, process1);

    Node* after2 = first_fit(mem_list, process2);
    Node* allocated2 = allocate(mem_list, after2, process2);
    evict(mem_list, allocated1);

    Node* after3 = first_fit(mem_list, process3);
    Node* allocate3 = allocate(mem_list, after3, process3);
//    print_memory_list(mem_list);
//    printf("--------------\n");
    Node* merged = evict(mem_list, allocated2);
//    print_memory_list(mem_list);
    memory_fragment_t* freeSpace = (memory_fragment_t*) merged->data;
    assert(freeSpace->type == HOLE_FRAGMENT);
    assert(freeSpace->process == NULL);
    assert(freeSpace->length == 820);
    assert(freeSpace->start == 0);
}

/*
 * Testcase for [...| Process | X| Hole]
 * expected output: [...| Process | Hole]
 */
int evict_test_in_PXH() {
    memory_list_t* mem_list = create_memory_list();
    init_memory_list(mem_list, 1000);
    process_t* process1 = create_process(1, 1, 20, 5);
    process_t* process2 = create_process(1, 2, 800, 5);
    process_t* process3 = create_process(1, 3, 100, 5);
    Node* after1 = first_fit(mem_list, process1);
    Node* allocated1 = allocate(mem_list, after1, process1);

    Node* after2 = first_fit(mem_list, process2);
    Node* allocated2 = allocate(mem_list, after2, process2);

    Node* after3 = first_fit(mem_list, process3);
    Node* allocate3 = allocate(mem_list, after3, process3);

    Node* merged = evict(mem_list, allocate3);
    memory_fragment_t* freeSpace = (memory_fragment_t*) merged->data;
    assert(freeSpace->type == HOLE_FRAGMENT);
    assert(freeSpace->process == NULL);
    assert(freeSpace->length == 180);
    assert(freeSpace->start == 820);
}

int evict_test_in_HXH() {
    memory_list_t* mem_list = create_memory_list();
    init_memory_list(mem_list, 1000);
    process_t* process1 = create_process(1, 1, 20, 5);
    process_t* process2 = create_process(1, 2, 800, 5);
    process_t* process3 = create_process(1, 3, 100, 5);
    Node* after1 = first_fit(mem_list, process1);
    Node* allocated1 = allocate(mem_list, after1, process1);

    Node* after2 = first_fit(mem_list, process2);
    Node* allocated2 = allocate(mem_list, after2, process2);

    Node* after3 = first_fit(mem_list, process3);
    Node* allocated3 = allocate(mem_list, after3, process3);

    evict(mem_list, allocated1);
    evict(mem_list, allocated3);
//    print_memory_list(mem_list);
//    printf("--------------\n");
    Node* merged = evict(mem_list, allocated2);
//    print_memory_list(mem_list);

    memory_fragment_t* freeSpace = (memory_fragment_t*) merged->data;
    assert(freeSpace->type == HOLE_FRAGMENT);
    assert(freeSpace->process == NULL);
    assert(freeSpace->length == 1000);
    assert(freeSpace->start == 0);
}

int test_use_memory() {
    memory_list_t* mem_list = create_memory_list();
    init_memory_list(mem_list, 1000);
    process_t* process1 = create_process(1, 1, 20, 5);
    process_t* process2 = create_process(1, 2, 800, 5);
    process_t* process3 = create_process(1, 3, 100, 5);
    Node* after1 = first_fit(mem_list, process1);
    Node* allocated1 = allocate(mem_list, after1, process1);

    Node* after2 = first_fit(mem_list, process2);
    Node* allocated2 = allocate(mem_list, after2, process2);

    Node* after3 = first_fit(mem_list, process3);
    Node* allocate3 = allocate(mem_list, after3, process3);
//    print_memory_list(mem_list);
    //    printf("--------------\n");
    use_memory(mem_list, 1, 999);
//    print_memory_list(mem_list);
    assert(((memory_fragment_t*)mem_list->list->head->data)->last_access == 999);
}

int test_find_least_recently_used() {
    memory_list_t* mem_list = create_memory_list();
    init_memory_list(mem_list, 1000);
    process_t* process1 = create_process(1, 1, 20, 5);
    process_t* process2 = create_process(1, 2, 800, 5);
    process_t* process3 = create_process(1, 3, 100, 5);
    Node* after1 = first_fit(mem_list, process1);
    Node* allocated1 = allocate(mem_list, after1, process1);

    Node* after2 = first_fit(mem_list, process2);
    Node* allocated2 = allocate(mem_list, after2, process2);

    Node* after3 = first_fit(mem_list, process3);
    Node* allocate3 = allocate(mem_list, after3, process3);
//    print_memory_list(mem_list);
    //    printf("--------------\n");
    use_memory(mem_list, 1, 999);
    use_memory(mem_list, 2, 888);
    use_memory(mem_list, 3, 4);
//    print_memory_list(mem_list);

    Node* found = find_least_recently_used(mem_list);
    assert(((memory_fragment_t*)found->data)->last_access == 4);
}

int evict_test(){
    /*
     * Testcase for [...| Process | X| Process]
     * expected output: [...| Process | Hole| Process]
     */
    evict_test_in_PXP();
    /*
     * Testcase for [...| Process | X| Hole]
     * expected output: [...| Process | Hole]
     */
    evict_test_in_PXH();
    /**
     * Testcase for [...| Hole | X| Process]
     * expected output: [...| Hole | Process]
     */
    evict_test_in_HXP();
    /**
    * Testcase for [...| Hole | X| Hole]
    * expected output: [...| Hole]
    */
    evict_test_in_HXH();
    test_use_memory();
    test_find_least_recently_used();
}