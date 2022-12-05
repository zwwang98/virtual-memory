#include "thread.h"
#include "util.h"

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

int allocateHeapMem(Thread *thread, int size) {
}

int allocateStackMem(Thread *thread, int size) {
}

void writeToAddr(const Thread* thread, int addr, int size, const void* data) {
  if (addr < USER_BASE_ADDR) {
    kernelPanic(thread, addr);
    return;
  }
}

void readFromAddr(Thread* thread, int addr, int size, void* outData) {
    if (addr < USER_BASE_ADDR) {
      kernelPanic(thread, addr);
      return;
    }
}

char* getCacheFileName(Thread* thread, int addr) {
}