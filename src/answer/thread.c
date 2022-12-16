#include "thread.h"
#include "utils.h"
#include <strings.h>

int currentThreadId = 1;

extern const int NUM_PAGES;
extern const int ALL_MEM_SIZE;
extern const int USER_BASE_ADDR;
extern const int STACK_END_ADDR;
extern const int NUM_USER_SPACE_PAGES;

Thread* createThread() {
  Thread* ret = (Thread*) malloc(sizeof(Thread));
  bzero(ret, sizeof(Thread));

  ret->threadId = currentThreadId;
  currentThreadId++;

  ret->stackTop = ALL_MEM_SIZE;
  ret->heapBottom = USER_BASE_ADDR;

  for (int i = 0; i < NUM_USER_SPACE_PAGES; i++) {
    ret->VPNToPFN[i].physicalFrameNumber = -1;
  }

  return ret;
}

void destroyThread(Thread* thread) {
  // This is line is ABSOLUTELY REQUIRED for the tests to run properly. This allows the thread to finish its work
  // DO NOT REMOVE.
  if (thread->thread) pthread_join(thread->thread, NULL);
  free(thread);
}
