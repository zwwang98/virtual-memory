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

char RAW_SYSTEM_MEMORY_ACCESS[8 * 1024 * 1024];
void *SYSTEM_MEMORY = RAW_SYSTEM_MEMORY_ACCESS;


int allocateHeapMem(Thread *thread, int size) {
  int memoryBeginsAt = thread->heapTop;
  // heap gros downwards, update the new heap top
  thread->heapTop += size;
  return memoryBeginsAt;
}

int allocateStackMem(Thread *thread, int size) {
  int memoryBeginsAt = thread->stackBottom - size;
  // stack gros upwards, update the new stack bottom
  thread->stackBottom -= size;
  return memoryBeginsAt;
}

void writeToAddr(const Thread* thread, int addr, int size, const void* data) {
  if (addr < USER_BASE_ADDR) {
    kernelPanic(thread, addr);
    return;
  }
  memcpy(SYSTEM_MEMORY + addr - USER_BASE_ADDR, data, size);
}

void readFromAddr(Thread* thread, int addr, int size, void* outData) {
    if (addr < USER_BASE_ADDR) {
      kernelPanic(thread, addr);
      return;
    }
    memcpy(outData, SYSTEM_MEMORY + addr - USER_BASE_ADDR, size);
}

char* getCacheFileName(Thread* thread, int addr) {
  return NULL;
}