#ifndef VIRTUALMEMFRAMEWORKC_THREAD_H
#define VIRTUALMEMFRAMEWORKC_THREAD_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "PageTableEntry.h"

extern const int NUM_PAGES;

/**
 * This struct defines a thread, you will have to add to it for all the functionality required. What is provided
 * is the minimal amount need for the tests to compile and run.
 */
typedef struct Thread
{
    pthread_t thread;
    int threadId;

    // start and end of stack / heap
    int stackTop;  // 8M -> 6M
    int heapBottom; // 1M -> 6M

    // page table
    PageTableEntry VPNToPFN[2048];
} Thread;

/**
 * This function should create and return a thread that can be started anytime after the return of the call.
 * A caller may execute the pthread contained within the thread struct anytime after returning from this call.
 * @return A fully functional thread object that has been initialized and is ready to run.
 */
Thread* createThread();

/**
 * Destroys a thread object and cleans up any allocated memory. This function will also do a pthread_join so that
 * the called thread can finish completing. Removing the pthread_join will cause bad behavior.
 * @param thread
 */
void destroyThread(Thread* thread);

#endif //VIRTUALMEMFRAMEWORKC_THREAD_H