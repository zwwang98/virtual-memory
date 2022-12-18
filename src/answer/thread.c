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

/***/
void removeLocalCacheFile(Thread* thread) {
  char buffer[1024];
  sprintf(buffer, "\n\n[removeLocalCacheFile] Start to remove cache files of {thread: %d}.\n", thread->threadId);
  logData(buffer);
  flushLog();
  char filename[1024];
  for (int i = 0; i < NUM_PAGES; i++) {
    snprintf(filename, sizeof(filename), ".page_thread_%d_vpn_%d", thread->threadId, i);
    if (remove(filename) == 0) {
      sprintf(buffer, "[removeLocalCacheFile] Remvoed {filename: %s} successfully.\n", filename);
      logData(buffer);
      flushLog();
    }
  }
  sprintf(buffer, "\n[removeLocalCacheFile] Finish removing cache files.\n\n");
  logData(buffer);
  flushLog();
}

void destroyThread(Thread* thread) {

  // This is line is ABSOLUTELY REQUIRED for the tests to run properly. This allows the thread to finish its work
  // DO NOT REMOVE.
  if (thread->thread) pthread_join(thread->thread, NULL);

  char buffer[1024];

  pthread_mutex_lock(&lock);
  sprintf(buffer, "[destroyThread] Lock the frame table.\n");
  logData(buffer);
  flushLog();

  sprintf(buffer, "[destroyThread] Gonna to destroy {thread: %d}.\n", thread->threadId);
  logData(buffer);
  flushLog();

  // release all used frames
  for (int i = 0; i < NUM_PAGES; i++) {
    int pfn = thread->VPNToPFN[i].physicalFrameNumber;
    if (pfn == -1) {
      continue;
    }
    // sprintf(buffer, "[destroyThread] {pfn: %d}.\n", pfn);
    // logData(buffer);
    // flushLog();
    PFNTable[pfn].isUsed = false;
    PFNTable[pfn].thread = NULL;
  }

  removeLocalCacheFile(thread);

  sprintf(buffer, "[destroyThread] Finish destroying {thread: %d}.\n", thread->threadId);
  logData(buffer);
  flushLog();

  free(thread);
  pthread_mutex_unlock(&lock);
  sprintf(buffer, "[destroyThread] Release the lock over the frame table.\n");
  logData(buffer);
  flushLog();

}
