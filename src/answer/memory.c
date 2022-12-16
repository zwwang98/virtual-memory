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

const int NUM_USER_SPACE_PAGES = NUM_PAGES * 7 / 8;

// Each page has 512 Bytes to store its meta data.
// 1M / 2*1024 pages = 1024 * 1024 Bytes / 2 * 1024 pages = 512 Bytes / page
const int SPACE_PER_PAGE = 512;

// 8MB memory
char RAW_SYSTEM_MEMORY_ACCESS[8 * 1024 * 1024];
void *SYSTEM_MEMORY = RAW_SYSTEM_MEMORY_ACCESS;

typedef struct FrameEntry {
  int threadId;
  int vpn;
  bool isUsed;
} FrameEntry;

// frame table
FrameEntry PFNTable[1792];

// assign frames to thread to
void allocateMemory(Thread *thread, int begin, int end) {
  char buffer[1024];

  int beginPageNum = begin / PAGE_SIZE;
  int endPageNum = end / PAGE_SIZE;
  int curPageNum = beginPageNum;

  sprintf(buffer, "[allocateMemory] {begin: %d}, {beginPageNum: %d}, {end: %d}, {endPageNum: %d}, {curPageNum: %d}.\n", begin, beginPageNum, end, endPageNum, curPageNum);
  logData(buffer);
  flushLog();


  int x = 5;

  // assign frames from beginPageNum to endPageNum
  for (int i = 0; i < 1792; i++) {
    while (curPageNum <= endPageNum && thread->VPNToPFN[curPageNum].physicalFrameNumber != -1) {
      curPageNum++;
      sprintf(buffer, "[allocateMemory] Skip assigning frame to thread %d page %d, because it already has a frame  %d.\n", thread->threadId, curPageNum, thread->VPNToPFN[curPageNum]);
      logData(buffer);
      flushLog();
    }
    if (curPageNum > endPageNum) {
      break;
    }
    if (!PFNTable[i].isUsed) {
      // update frame table
      PFNTable[i].threadId = thread->threadId;
      PFNTable[i].vpn = curPageNum;
      PFNTable[i].isUsed = true;
      // update thread's page table
      thread->VPNToPFN[curPageNum].physicalFrameNumber = i;
      thread->VPNToPFN[curPageNum].present = 1;
      thread->VPNToPFN[curPageNum].accessed = 1;

      // // log
      if (curPageNum >= 1531) {
        if (x-- > 0) {
          sprintf(buffer, "[allocateMemory] Assign frame %d to thread %d page %d.\n", i, thread->threadId, curPageNum);
          logData(buffer);
          flushLog();
        }
      }

      curPageNum++;
    }
  }

  // for some unknown reason, there are some pages not given frames
  sprintf(buffer, "[allocateMemory] {begin: %d}, {beginPageNum: %d}, {end: %d}, {endPageNum: %d}, {curPageNum: %d}.\n", begin, beginPageNum, end, endPageNum, curPageNum);
  logData(buffer);
  flushLog();

  // for (int i = 0; i < 1792; i++) {
  //   if (PFNTable[i].threadId == 0) {
  //     sprintf(buffer, "[allocateMemory] {pfn: %d}, {threadId: %d}, {vpn: %d}.\n", i, PFNTable[i].threadId, PFNTable[i].vpn);
  //     logData(buffer);
  //     flushLog();
  //   }
  // }
}

void allocateFrameToPage(Thread* thread, int vpn) {

}

int allocateHeapMem(Thread *thread, int size) {
  char buffer[1024];
  sprintf(buffer, "[allocateHeapMem] Remaimned memory is enough, do not need another page.\n");
  logData(buffer);
  flushLog();
  // unable to allocate size memory
  int availableMemory = STACK_END_ADDR - thread->heapBottom;
  if (availableMemory < size) {
    return -1;
  }

  // find previous page's remained memory
  int prePageRemainedMemory = PAGE_SIZE - thread->heapBottom % PAGE_SIZE;

  if (prePageRemainedMemory >= size) {
    // if remaimned memory is enough, do not need another page
    sprintf(buffer, "[allocateHeapMem] Remaimned memory is enough, do not need another page.\n");
    logData(buffer);
    flushLog();
  } else {
    // else, allocate more pages
    sprintf(buffer, "[allocateHeapMem] To allocate memory.\n");
    logData(buffer);
    flushLog();

    sprintf(buffer, "[allocateHeapMem] {heap bottom: %d}.\n", thread->heapBottom);
    logData(buffer);
    flushLog();

    allocateMemory(thread, thread->heapBottom, thread->heapBottom + size);
    sprintf(buffer, "[allocateHeapMem] Allocated memory.\n");
    logData(buffer);
    flushLog();
  }

  // store the first bit of heap
  int res = thread->heapBottom;

  // able to allocate size memory
  thread->heapBottom += size;

  return res;
}

int allocateStackMem(Thread *thread, int size) {
  char buffer[1024];
  sprintf(buffer, "[allocateStackMem] Start allocating memory to stack.\n");
  logData(buffer);
  flushLog();

  // unable to allocate size memory
  int availableMemory = thread->stackTop - STACK_END_ADDR;
  if (availableMemory < size) {
    return -1;
  }

  // find previous page's remained memory
  int existingStackMemory = ALL_MEM_SIZE - thread->stackTop + 1;
  int prePageRemainedMemory = (PAGE_SIZE - ALL_MEM_SIZE - thread->stackTop) % PAGE_SIZE;
  sprintf(buffer, "[allocateStackMem] {thread->stackTop: %d}, {prePageRemainedMemory: %d}.\n", thread->stackTop, prePageRemainedMemory);
  logData(buffer);
  flushLog();

  if (prePageRemainedMemory >= size) {
    // if remaimned memory is enough, do not need another page
    sprintf(buffer, "[allocateStackMem] Remaimned memory is enough, do not need another page.\n");
    logData(buffer);
    flushLog();
  } else {
    // else, allocate more pages
    allocateMemory(thread, thread->stackTop - size, thread->stackTop);
  }

  // able to allocate size memory
  thread->stackTop -= size;

  sprintf(buffer, "[allocateStackMem] End allocating memory to stack.\n");
  logData(buffer);
  flushLog();

  return thread->stackTop;
}

void printOutDataGivenLen(char data[], int len, int bitToPrint) {
  if (bitToPrint > len) {
    bitToPrint = len;
  }

  char buffer[1024];
  for (int i = 0; i < bitToPrint; i++) {
    printf(buffer, "[printOutData] data[%d] = %d\n", i, data[i]);
    logData(buffer);
    flushLog();
  }
}

char* getDataArr(const void* data, int size) {
  char dataArr[size];
  char* dataPtr = data;
  for (int i = 0; i < size; i++) {
    dataArr[i] = *dataPtr++;
  }
  return dataArr;
}

int bitToPrint = 5;

void writeToAddr(const Thread* thread, int addr, int size, const void* data) {
  char buffer[1024];
  int originalSize = size;
  int originalAddr = addr;
  sprintf(buffer, "[writeToAddr] To write %d data from %d to %d\n", originalSize, originalAddr, originalAddr + originalSize - 1);
  logData(buffer);
  flushLog();

  if (addr < USER_BASE_ADDR) {
    // write to kernel memory, call kernelPanic()
    printf(buffer, "[writeToAddr] Writting to kernel space is not allowed.\n");
    logData(buffer);
    flushLog();
    kernelPanic(thread, addr);
    return;
  } else if (addr >= ALL_MEM_SIZE) {
    // address out of range
    sprintf(buffer, "[writeToAddr] Address is out of range.\n");
    logData(buffer);
    flushLog();
    return;
  }


  if (bitToPrint > 0) {
    char* _dataPtr = data;
    char* partOfData;
    sprintf(buffer, "[writeToAddr] {line: %d} Show frist %d numbers of data: ", __LINE__, bitToPrint);
    logData(buffer);
    flushLog();

    for (int i = 0; i < bitToPrint; i++) {
      if (i == bitToPrint - 1) {
        sprintf(buffer, "%d\n", *_dataPtr++);
      } else {
        sprintf(buffer, "%d, ", *_dataPtr++);
      }
      logData(buffer);
      flushLog();
    }
  }

  char* dataPtr = data;
  sprintf(buffer, "[Line] %d\n", __LINE__);
  logData(buffer);
  flushLog();

  int x = 5;
  sprintf(buffer, "[Line] %d\n", __LINE__);
  logData(buffer);
  flushLog();

  char partOfData[bitToPrint];
  sprintf(buffer, "[Line] %d\n", __LINE__);
  logData(buffer);
  flushLog();

  while (size > 0) {
    // not in stack or heap
    if (addr > thread->heapBottom && addr < thread->stackTop) {
      sprintf(buffer, "[Line] %d, kernel panic\n", __LINE__);
      logData(buffer);
      flushLog();
      kernelPanic(thread, addr);
    }

    int vpn = addr / PAGE_SIZE;
    int offset = addr % PAGE_SIZE;
    int pfn = thread->VPNToPFN[vpn].physicalFrameNumber;
    int memoryIdx = pfn * PAGE_SIZE + offset;

    if (size <= 4096) {
      sprintf(buffer, "[writeToAddr] {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}, {addr: %d}, {dataPtr: 0x%x}, {size: %d}.\n", vpn, offset, pfn, memoryIdx, addr, *(dataPtr), size);
      logData(buffer);
      flushLog();
    }

    // sprintf(buffer, "[writeToAddr] {memoryIdx: %d}\n", memoryIdx);
    // logData(buffer);
    // flushLog();

    // write ont bit
    // bug 
    sprintf(buffer, "[Line] %d\n", __LINE__);
    logData(buffer);
    flushLog();

    sprintf(buffer, "%d\n", *dataPtr);
    logData(buffer);
    flushLog();

    RAW_SYSTEM_MEMORY_ACCESS[memoryIdx] = *dataPtr;
    sprintf(buffer, "[Line] %d\n", __LINE__);
    logData(buffer);
    flushLog();

    if (originalSize - size < bitToPrint) {
      partOfData[originalSize - size] = *dataPtr;
    }

    if (x-- > 0) {
      sprintf(buffer, "[writeToAddr] {memory: 0x%x}, {dataPtr: 0x%x}.\n", RAW_SYSTEM_MEMORY_ACCESS[memoryIdx], *(dataPtr));
      logData(buffer);
      flushLog();

      sprintf(buffer, "[writeToAddr] {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}, {addr: %d}, {dataPtr: 0x%x}.\n", vpn, offset, pfn, memoryIdx, addr, *(dataPtr));
      logData(buffer);
      flushLog();
    }

    dataPtr++;

    addr++;
    size--;
  }
  sprintf(buffer, "[Line] %d\n", __LINE__);
  logData(buffer);
  flushLog();
  // memcpy(SYSTEM_MEMORY + addr - USER_BASE_ADDR, data, size);

  if (bitToPrint > 0) {
    sprintf(buffer, "[writeToAddr] Show frist %d numbers of data that have been written into memory: ", bitToPrint);
    logData(buffer);
    flushLog();
    sprintf(buffer, "[Line] %d\n", __LINE__);
    logData(buffer);
    flushLog();
    for (int i = 0; i < bitToPrint; i++) {
      if (i == bitToPrint - 1) {
        sprintf(buffer, "%d\n", partOfData[i]);
      } else {
        sprintf(buffer, "%d, ", partOfData[i]);
      }
      logData(buffer);
      flushLog();
    }
  }

  sprintf(buffer, "[writeToAddr] Wrote %d data from %d to %d\n\n", originalSize, originalAddr, originalAddr + originalSize - 1);
  logData(buffer);
  flushLog();
}

void readFromAddr(Thread* thread, int addr, int size, void* outData) {
  char buffer[1024];
  int originalSize = size;
  int originalAddr = addr;
  sprintf(buffer, "[readFromAddr] To write %d data from %d to %d\n", originalSize, originalAddr, originalAddr + originalSize - 1);
  logData(buffer);
  flushLog();

  if (addr < USER_BASE_ADDR) {
    // read to kernel memory, call kernelPanic()
    printf(buffer, "Writting to kernel space is not allowed.\n");
    logData(buffer);
    flushLog();
    kernelPanic(thread, addr);
    return;
  } else if (addr >= ALL_MEM_SIZE) {
    // address out of range
    sprintf(buffer, "Address is out of range.\n");
    logData(buffer);
    flushLog();
    return;
  }

  // memcpy(outData, SYSTEM_MEMORY + addr - USER_BASE_ADDR, size);
  char partOfData[bitToPrint];
  char* outDataPtr = outData;

  int x = bitToPrint;

  while (size > 0) {
    // not in stack or heap
    if (addr > thread->heapBottom && addr < thread->stackTop) {
      kernelPanic(thread, addr);
    }

    int vpn = addr / PAGE_SIZE;
    int offset = addr % PAGE_SIZE;
    int pfn = thread->VPNToPFN[vpn].physicalFrameNumber;
    int memoryIdx = pfn * PAGE_SIZE + offset;

    // 5242879
    // sprintf(buffer, "[readFromAddr] {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}.\n", vpn, offset, pfn, memoryIdx);
    // logData(buffer);
    // flushLog();

    // write ont bit
    *outDataPtr = RAW_SYSTEM_MEMORY_ACCESS[memoryIdx];
    if (x-- > 0) {
      sprintf(buffer, "[readFromAddr] %d.\n", RAW_SYSTEM_MEMORY_ACCESS[memoryIdx]);
      logData(buffer);
      flushLog();

      sprintf(buffer, "[readFromAddr] {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}, {addr: %d}.\n", vpn, offset, pfn, memoryIdx, addr);
      logData(buffer);
      flushLog();
    }
    // partOfData[originalSize - size] = RAW_SYSTEM_MEMORY_ACCESS[memoryIdx];
    outDataPtr++;

    addr++;
    size--;
  }

  sprintf(buffer, "[readFromAddr] Wrote %d data from %d to %d\n\n", originalSize, originalAddr, originalAddr + originalSize - 1);
  logData(buffer);
  flushLog();
}

char* getCacheFileName(Thread* thread, int addr) {
  return NULL;
}