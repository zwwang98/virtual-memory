#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "thread.h"
#include "utils.h"

int currentThreadId = 1;

Thread* createThread() {
    Thread* ret = malloc(sizeof(Thread));
    bzero(ret, sizeof(Thread));

    ret->threadId = currentThreadId;
    currentThreadId++;

    ret->heapTop = 1024 * 1024;
    ret->stackBottom = 8 * 1024 * 1024;

    return ret;
}

void destroyThread(Thread* thread) {
    // This is line is ABSOLUTELY REQUIRED for the tests to run properly. This allows the thread to finish its work
    // DO NOT REMOVE.
    if (thread->thread) pthread_join(thread->thread, NULL);
    free(thread);
}
