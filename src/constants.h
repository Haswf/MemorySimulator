/**
 * Constants used in this project are defined here.
 * Created by Haswell on 22/05/2020.
 */

#ifndef SCHEDULER_CONSTANTS_H
#define SCHEDULER_CONSTANTS_H
/*
 * Scheduling Algorithm
 */
#define FIRST_COME_FIRST_SERVED 1
#define ROUND_ROBIN 2
#define CUSTOMISED_SCHEDULING 3
/**
 * Page Replacement Algorithm
 */
#define UNLIMITED -1
#define SWAPPING -2
#define VIRTUAL_MEMORY -3
#define CUSTOMISED_MEMORY -4
/**
 * Page Size in Bytes
 */
#define PAGE_SIZE 4
/**
 * Memory Fragment Type
 */
#define HOLE_FRAGMENT 5
#define PROCESS_FRAGMENT 6
/**
 * Loading time per page
 */
#define LOADING_TIME_PER_PAGE 2

#endif //SCHEDULER_CONSTANTS_H
