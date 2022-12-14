#include "thread.h"
#include "./utils.h"
#include <string.h>

// 4k is the size of a page
const int PAGE_SIZE= 4*1024;
// A total of 8M exists in memory
const int ALL_MEM_SIZE = 8*1024*1024;
// USER Space starts at 1M
const int USER_BASE_ADDR = 1024*1024;
// Stack starts at 8M and goes down to 6M
const int STACK_END_ADDR = 6*1024*1024;
// There are total of 2048 pages
const int NUM_PAGES = 2*1024;

// Each page has 512 Bytes to store its meta data.
// 1M / 2*1024 pages = 1024 * 1024 Bytes / 2 * 1024 pages = 512 Bytes / page
const int SPACE_PER_PAGE = 512;

// 8MB memory
char RAW_SYSTEM_MEMORY_ACCESS[8 * 1024 * 1024];
void *SYSTEM_MEMORY = RAW_SYSTEM_MEMORY_ACCESS;

typedef struct FrameEntry {
  int threadId;
  int vpn;
} FrameEntry;

// frame table
FrameEntry PFNTable[1792];

// assign frames to thread to
void allocateMemory(Thread *thread, int begin, int end) {
  int beginPageNum = begin / PAGE_SIZE;
  int endPageNum = end / PAGE_SIZE;

  int curPageNum = beginPageNum;

  // assign frames from beginPageNum to endPageNum
  for (int i = 0; i < 1792; i++) {
    if (PFNTable[i].threadId == 0) {
      PFNTable[i].threadId = thread->threadId;
      PFNTable[i].vpn = curPageNum++;
    }
    if (curPageNum > endPageNum) {
      break;
    }
  }
}

int allocateHeapMem(Thread *thread, int size) {
  // unable to allocate size memory
  int availableMemory = STACK_END_ADDR - thread->heapBottom;
  if (availableMemory < size) {
    return -1;
  }

  // find previous page's remained memory
  int prePageRemainedMemory = PAGE_SIZE - thread->heapBottom % PAGE_SIZE;

  if (prePageRemainedMemory >= size) {
    // if remaimned memory is enough, do not need another page
  } else {
    // else, allocate more pages
    allocateMemory(thread, thread->heapBottom, thread->heapBottom + size);
  }

  // store the first bit of heap
  int res = thread->heapBottom;

  // able to allocate size memory
  thread->heapBottom += size;

  return res;
}

int allocateStackMem(Thread *thread, int size) {
  // unable to allocate size memory
  int availableMemory = thread->stackTop - STACK_END_ADDR;
  if (availableMemory < size) {
    return -1;
  }

  // find previous page's remained memory
  int prePageRemainedMemory = PAGE_SIZE - thread->stackTop % PAGE_SIZE;

  if (prePageRemainedMemory >= size) {
    // if remaimned memory is enough, do not need another page
  } else {
    // else, allocate more pages
    allocateMemory(thread, thread->stackTop - size, thread->stackTop);
  }

  // able to allocate size memory
  thread->stackTop -= size;

  return thread->stackTop;
}

/**
 * @brief Helper function to log and print out something to the console.
 * 
 * @param buffer Given character buffer.
 * @param message Given message.
 */
void logHelper(char* buffer, char* message);

void writeToAddr(const Thread* thread, int addr, int size, const void* data) {
  char buffer[1024];

  // write to kernel memory, call kernelPanic()
  if (addr < USER_BASE_ADDR) {
    kernelPanic(thread, addr);
    return;
  }

  // address out of range
  if (addr >= ALL_MEM_SIZE) {
    logHelper(buffer, "Address is out of range.");
    return;
  }

  memcpy(SYSTEM_MEMORY + addr - USER_BASE_ADDR, data, size);
}

void readFromAddr(Thread* thread, int addr, int size, void* outData) {
  char buffer[1024];

  // read to kernel memory, call kernelPanic()
  if (addr < USER_BASE_ADDR) {
    kernelPanic(thread, addr);
    return;
  }

  // address out of range
  if (addr >= ALL_MEM_SIZE) {
    logHelper(buffer, "Address is out of range.");
    return;
  }

  memcpy(outData, SYSTEM_MEMORY + addr - USER_BASE_ADDR, size);
}

char* getCacheFileName(Thread* thread, int addr) {
  return NULL;
}

void logHelper(char* buffer, char* message) {
  sprintf(buffer, message);
  logData(buffer);
  flushLog();
}