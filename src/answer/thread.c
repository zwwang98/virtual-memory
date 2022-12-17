#include "thread.h"
#include "utils.h"
#include <strings.h>
#include "FrameTable.h"

int currentThreadId = 1;

extern const int NUM_PAGES;
extern const int ALL_MEM_SIZE;
extern const int USER_BASE_ADDR;
extern const int STACK_END_ADDR;
extern const int NUM_USER_SPACE_PAGES;
extern const char* RAW_SYSTEM_MEMORY_ACCESS;
extern FrameEntry PFNTable[2048];
extern const pthread_mutex_t lock;

Thread* createThread() {
  Thread* ret = (Thread*) malloc(sizeof(Thread));
  bzero(ret, sizeof(Thread));

  ret->threadId = currentThreadId;
  currentThreadId++;

  ret->stackTop = ALL_MEM_SIZE;
  ret->heapBottom = USER_BASE_ADDR;

  for (int i = 0; i < NUM_PAGES; i++) {
    ret->VPNToPFN[i].physicalFrameNumber = -1;
  }

  return ret;
}

void destroyThread(Thread* thread) {
  // pthread_mutex_lock(&lock);
  // This is line is ABSOLUTELY REQUIRED for the tests to run properly. This allows the thread to finish its work
  // DO NOT REMOVE.
  if (thread->thread) pthread_join(thread->thread, NULL);

  char buffer[1024];
  sprintf(buffer, "[destroyThread] Gonna to destroy {thread: %d}.\n", thread->threadId);
  logData(buffer);
  flushLog();

  // release all used frames
  for (int i = 256; i < NUM_PAGES; i++) {
    int pfn = thread->VPNToPFN[i].physicalFrameNumber;
    if (pfn == -1) {
      continue;
    }
    // sprintf(buffer, "[destroyThread] {pfn: %d}.\n", pfn);
    // logData(buffer);
    // flushLog();
    PFNTable[pfn].isUsed = false;
  }

  free(thread);
  // pthread_mutex_unlock(&lock);

  sprintf(buffer, "[destroyThread] Finish destroying {thread: %d}.\n", thread->threadId);
  logData(buffer);
  flushLog();
}
