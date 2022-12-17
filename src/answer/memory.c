#include "thread.h"
#include "./utils.h"
#include "FrameTable.h"
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

// frame table
FrameEntry PFNTable[NUM_PAGES];

// https://stackoverflow.com/questions/23400097/c-confused-on-how-to-initialize-and-implement-a-pthread-mutex-and-condition-vari
// initialize a mutex lock
// lock - pthread_mutex_lock(&lock);
// unlok - pthread_mutex_unlock(&lock);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// some variables help for debug
int bitToPrint = 5;
int startIdx = 2088960;
int startLogPageNum = 0;

/**
 * @brief Debug-helper function. Print out all unused frame intervals.
 */
void printOutAllUnusedFrameIntervals(Thread *thread);

/**
 * @brief Debug-helper function. Print out all used frame associated with the thread id.
 */
void printOutAllUsedFrameThreadId(Thread *thread);

/**
 * @brief Evict given thread's given page from given memory pointer into disc.
 * 
 * @param thread Given thread.
 * @param vpn The virtual page number of the page to be evicted.
 * @param memory Given memory pointer.
 */
void evictPage(Thread *thread, int vpn, void *memory);

/**
 * @brief Load given thread's given page from disc into memory.
 * 
 * @param thread Given thread.
 * @param vpn The virtual page number of the page to be evicted.
 * @param memory Given memory pointer.
 */
void loadPage(Thread *thread, int vpn, void *memory);

/**
 * @brief Given thread and vpn, return the expected file name of the file storing the data in this page.
 * 
 * @param thread Given thread.
 * @param vpn Given virtual page number.
 * @return char* The expected file name of the file storing the data in this page.
 */
char* outPageName(Thread *thread, int vpn) {
  char fileName[1024];
  snprintf(fileName, sizeof(fileName), "page_{thread:%d}_{vpn:%d}", thread->threadId, vpn);
  return fileName;
}

/**
 * @brief Check the existence of page in disc given thread and vpn.
 * 
 * @param thread Given thread.
 * @param vpn Given virtual page number of given thread.
 * @return true If the data of that page has been swapped into disc before.
 * @return false If the data of that page has never been swapped into disc before.
 */
bool isPageInDisc(Thread *thread, int vpn) {

}

// assign frames to thread to
void allocateMemory(Thread *thread, int begin, int end, int size) {
  char buffer[1024];
  sprintf(buffer, "\n[allocateMemory] Start to allocate {size: %d} memory from %d to %d.\n", size, begin, end);
  logData(buffer);
  flushLog();

  pthread_mutex_lock(&lock);
  sprintf(buffer, "[allocateMemory] {line: %d} {thread: %d} gets the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  printOutAllUnusedFrameIntervals(thread);
  printOutAllUsedFrameThreadId(thread);

  int beginPageNum = begin / PAGE_SIZE;
  int endPageNum = end / PAGE_SIZE;
  int curPageNum = beginPageNum;

  sprintf(buffer, "[allocateMemory] {begin: %d}, {beginPageNum: %d}, {end: %d}, {endPageNum: %d}, {curPageNum: %d}.\n", begin, beginPageNum, end, endPageNum, curPageNum);
  logData(buffer);
  flushLog();


  int x = bitToPrint;

  // assign frames from beginPageNum to endPageNum
  // frame 0-255 == 1MB, saved for kernel space
  for (int i = 256; i < 2048; i++) {
    while (curPageNum <= endPageNum && thread->VPNToPFN[curPageNum].physicalFrameNumber != -1) {
      sprintf(buffer, "[allocateMemory] Skip assigning frame to thread %d page {vpn: %d}, because it already has a frame  %d.\n", thread->threadId, curPageNum, thread->VPNToPFN[curPageNum]);
      logData(buffer);
      flushLog();
      curPageNum++;
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
      if (curPageNum >= startLogPageNum && x-- > 0) {
        sprintf(buffer, "[allocateMemory] {line: %d} Assign frame {pfn: %d} to thread %d page {vpn: %d}.\n", __LINE__, i, thread->threadId, curPageNum);
        logData(buffer);
        flushLog();
      }

      curPageNum++;
    }
  }

  // for some unknown reason, there are some pages not given frames
  sprintf(buffer, "[allocateMemory] {begin: %d}, {beginPageNum: %d}, {end: %d}, {endPageNum: %d}, {curPageNum: %d}.\n", begin, beginPageNum, end, endPageNum, curPageNum);
  logData(buffer);
  flushLog();

  if (curPageNum <= endPageNum) {
    sprintf(buffer, "[allocateMemory] Pages [%d, %d] are not allocated frames because there is no unused frame. We may need to evict frames.\n", curPageNum, endPageNum);
    logData(buffer);
    flushLog();
  }

  // for (int i = 0; i < 1792; i++) {
  //   if (PFNTable[i].threadId == 0) {
  //     sprintf(buffer, "[allocateMemory] {pfn: %d}, {threadId: %d}, {vpn: %d}.\n", i, PFNTable[i].threadId, PFNTable[i].vpn);
  //     logData(buffer);
  //     flushLog();
  //   }
  // }

  printOutAllUnusedFrameIntervals(thread);
  printOutAllUsedFrameThreadId(thread);

  pthread_mutex_unlock(&lock);
  sprintf(buffer, "[allocateMemory] {line: %d} {thread: %d} returns the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  sprintf(buffer, "[allocateMemory] Finish allocating {size: %d} memory from %d to %d.\n\n", size, begin, end);
  logData(buffer);
  flushLog();

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
  int existingHeapMemory = thread->heapBottom - USER_BASE_ADDR;
  int prePageExistingHeapMemory = existingHeapMemory % PAGE_SIZE;
  int prePageRemainedMemory;
  if (prePageExistingHeapMemory == 0) {
    prePageRemainedMemory = 0;
  } else {
    prePageRemainedMemory = PAGE_SIZE - prePageExistingHeapMemory;
  }
  sprintf(buffer, "[allocateHeapMem] {line: %d} {existingHeapMemory: %d}, {prePageRemainedMemory: %d}.\n", __LINE__, existingHeapMemory, prePageRemainedMemory);
  logData(buffer);
  flushLog();

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

    allocateMemory(thread, thread->heapBottom, thread->heapBottom + size - 1, size);
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
  int existingStackMemory = ALL_MEM_SIZE - thread->stackTop;
  int prePageRemainedMemory = (PAGE_SIZE - existingStackMemory) % PAGE_SIZE;
  sprintf(buffer, "[allocateStackMem] {thread->stackTop: %d}, {prePageRemainedMemory: %d}, {existingStackMemory: %d}.\n", thread->stackTop, prePageRemainedMemory, existingStackMemory);
  logData(buffer);
  flushLog();

  if (prePageRemainedMemory >= size) {
    // if remaimned memory is enough, do not need another page
    sprintf(buffer, "[allocateStackMem] Remaimned memory is enough, do not need another page.\n");
    logData(buffer);
    flushLog();
  } else {
    // else, allocate more pages
    allocateMemory(thread, thread->stackTop - size, thread->stackTop - 1, size);
  }

  // able to allocate size memory
  thread->stackTop -= size;
  sprintf(buffer, "[allocateStackMem] {line: %d}, {thread->stackTop: %d}, {prePageRemainedMemory: %d}, {existingStackMemory: %d}.\n", __LINE__, thread->stackTop, prePageRemainedMemory, existingStackMemory);
  logData(buffer);
  flushLog();

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



void writeToAddr(const Thread* thread, int addr, int size, const void* data) {
  char buffer[1024];
  
  int originalSize = size;
  int originalAddr = addr;
  sprintf(buffer, "\n[writeToAddr] To write %d data from %d to %d\n", originalSize, originalAddr, originalAddr + originalSize - 1);
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

  pthread_mutex_lock(&lock);
  sprintf(buffer, "[writeToAddr] {line: %d} {thread: %d} gets the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  if (bitToPrint > 0) {
    char* _dataPtr = data;
    char* partOfData;
    sprintf(buffer, "[writeToAddr] {line: %d} Show frist %d numbers of data: ", __LINE__, bitToPrint);
    logData(buffer);
    flushLog();

    for (int i = 0; i < bitToPrint; i++) {
      if (i == bitToPrint - 1) {
        sprintf(buffer, "%x\n", *_dataPtr++);
      } else {
        sprintf(buffer, "%x, ", *_dataPtr++);
      }
      logData(buffer);
      flushLog();
    }
  }

  char* dataPtr = data;
  sprintf(buffer, "[Line] %d\n", __LINE__);
  logData(buffer);
  flushLog();

  int x = bitToPrint;
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
      sprintf(buffer, "[writeToAddr] {line: %d} Addr is beyond valid range. KernelPanic will be invoked..\n", __LINE__);
      logData(buffer);
      flushLog();
      break;
    }

    int vpn = addr / PAGE_SIZE;
    int offset = addr % PAGE_SIZE;
    int pfn = thread->VPNToPFN[vpn].physicalFrameNumber;
    int memoryIdx = pfn * PAGE_SIZE + offset;
    if (size <= 2048 && x-- > 0) {
      sprintf(buffer, "[writeToAddr] {line: %d} {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}, {addr: %d}, {dataPtr: 0x%x}.\n", __LINE__, vpn, offset, pfn, memoryIdx, addr, *(dataPtr));
      logData(buffer);
      flushLog();
    }

    RAW_SYSTEM_MEMORY_ACCESS[memoryIdx] = *dataPtr;
    // sprintf(buffer, "[Line] %d\n", __LINE__);
    // logData(buffer);
    // flushLog();

    if (originalSize - size < bitToPrint) {
      partOfData[originalSize - size] = *dataPtr;
    }
    // if ((originalSize - size >= startIdx) && x-- > 0) {
    if (size <= 2048 && x-- > 0) {
      sprintf(buffer, "[writeToAddr] {line: %d}, {memory: 0x%x}, {dataPtr: 0x%x}.\n", __LINE__, RAW_SYSTEM_MEMORY_ACCESS[memoryIdx], *(dataPtr));
      logData(buffer);
      flushLog();

      sprintf(buffer, "[writeToAddr] {line: %d} {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}, {addr: %d}, {dataPtr: 0x%x}.\n", __LINE__, vpn, offset, pfn, memoryIdx, addr, *(dataPtr));
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
    sprintf(buffer, "[writeToAddr] {Line: %d} Show frist %d numbers of data that have been written into memory: ", __LINE__, bitToPrint);
    logData(buffer);
    flushLog();
    for (int i = 0; i < bitToPrint; i++) {
      if (i == bitToPrint - 1) {
        sprintf(buffer, "%x\n", partOfData[i]);
      } else {
        sprintf(buffer, "%x, ", partOfData[i]);
      }
      logData(buffer);
      flushLog();
    }
  }

  pthread_mutex_unlock(&lock);
  sprintf(buffer, "[writeToAddr] {line: %d} {thread: %d} returns the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  if (size > 0) {
    sprintf(buffer, "[writeToAddr] Have data out of range. Call kernelPanic().\n");
    logData(buffer);
    flushLog();
    kernelPanic(thread, addr);
  }

  sprintf(buffer, "[writeToAddr] Wrote %d data from %d to %d\n\n", originalSize, originalAddr, originalAddr + originalSize - 1);
  logData(buffer);
  flushLog();
}

void readFromAddr(Thread* thread, int addr, int size, void* outData) {
  char buffer[1024];
  int originalSize = size;
  int originalAddr = addr;
  sprintf(buffer, "[readFromAddr] To read %d data from %d to %d\n", originalSize, originalAddr, originalAddr + originalSize - 1);
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

  pthread_mutex_lock(&lock);
  sprintf(buffer, "[readFromAddr] {line: %d} {thread: %d} gets the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  // memcpy(outData, SYSTEM_MEMORY + addr - USER_BASE_ADDR, size);
  char partOfData[bitToPrint];
  char* outDataPtr = outData;

  int x = bitToPrint;

  while (size > 0) {
    // not in stack or heap, stop reading
    if (addr > thread->heapBottom && addr < thread->stackTop) {
      sprintf(buffer, "[readFromAddr] {line: %d} Addr is beyond valid range. KernelPanic will be invoked..\n", __LINE__);
      logData(buffer);
      flushLog();
      break;
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
    // if ((originalSize - size >= startIdx) && x-- > 0) {
    if (size <= 2048 && x-- > 0) {
      sprintf(buffer, "[readFromAddr] {memory: 0x%x}.\n", RAW_SYSTEM_MEMORY_ACCESS[memoryIdx]);
      logData(buffer);
      flushLog();

      sprintf(buffer, "[readFromAddr] {vpn: %d}, {offset: %d}, {pfn: %d}, {memoryIdx: %d}, {addr: %d}, {memory: 0x%x}.\n", vpn, offset, pfn, memoryIdx, addr, RAW_SYSTEM_MEMORY_ACCESS[memoryIdx]);
      logData(buffer);
      flushLog();
    }
    // partOfData[originalSize - size] = RAW_SYSTEM_MEMORY_ACCESS[memoryIdx];
    outDataPtr++;

    addr++;
    size--;
  }

  pthread_mutex_unlock(&lock);
  sprintf(buffer, "[readFromAddr] {line: %d} {thread: %d} returns the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  // do not read all size data, because some data is out of range
  if (size > 0) {
    sprintf(buffer, "[readFromAddr] Have data out of range. Call kernelPanic().\n");
    logData(buffer);
    flushLog();
    kernelPanic(thread, addr);
  }

  sprintf(buffer, "[readFromAddr] Read %d data from %d to %d\n\n", originalSize, originalAddr, originalAddr + originalSize - 1);
  logData(buffer);
  flushLog();
}

char* getCacheFileName(Thread* thread, int addr) {
  int vpn = addr / PAGE_SIZE;
  return outPageName(thread, vpn);
}

void printOutAllUnusedFrameIntervals(Thread* thread) {
  char buffer[1024];
  sprintf(buffer, "\n[printOutAllUnusedFrameIntervals] {line: %d} {thread: %d} Start to print out unused frame: ", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  int preUnusedPFN = 256;
  bool allFrameOccupied = true;

  int i = 256;
  int unusedFrameIntervalStart = -1;
  int unusedFrameIntervalEnd = -1;
  while (i < 2048) {
    // pfn i is used
    while (i < 2048 && PFNTable[i].isUsed) {
      i++;
    }
    // pfn i is not used
    unusedFrameIntervalStart = i;
    while (i < 2048 && !PFNTable[i].isUsed) {
      i++;
      allFrameOccupied = false;
    }
    unusedFrameIntervalEnd = i - 1;
    if (!allFrameOccupied) {
      sprintf(buffer, "[%d, %d], ", unusedFrameIntervalStart, unusedFrameIntervalEnd);
      logData(buffer);
      flushLog();
    }
  }

  if (!allFrameOccupied) {
    sprintf(buffer, "\n[printOutAllUnusedFrameIntervals] Not all frames (pfn 256-2047, user space) are used.\n");
    logData(buffer);
    flushLog();
  } else {
    sprintf(buffer, "\n[printOutAllUnusedFrameIntervals] All frames (pfn 256-2047, user space) are used.\n");
    logData(buffer);
    flushLog();
  }

  sprintf(buffer, "[printOutAllUnusedFrameIntervals] Finish printing out unused frame:\n");
  logData(buffer);
  flushLog();
}

void printOutAllUsedFrameThreadId(Thread* thread) {
  char buffer[1024];
  sprintf(buffer, "\n[printOutAllUsedFrameThreadId] {line: %d} {thread: %d} Start to print out used frame: ", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();

  int preUnusedPFN = 256;
  bool allFrameUnused = true;

  int i = 256;
  while (i < 2048) {
    int preThreadId = PFNTable[i].threadId;
    int preFrameId = i;
    // pfn i is used
    while (i < 2048 && PFNTable[i].isUsed && PFNTable[i].threadId == preThreadId) {
      allFrameUnused = false;
      i++;
    }
    // after the while loop, [preFrameId,i-1] is used by same thread
    if (preFrameId <= i - 1) {
      sprintf(buffer, "[printOutAllUsedFrameThreadId] {line: %d} frame[%d, %d] is used by {threadId: %d}.\n", __LINE__, preFrameId, i - 1, preThreadId);
      logData(buffer);
      flushLog();
    }

    // sprintf(buffer, "[printOutAllUsedFrameThreadId] {line: %d}, {i: %d}.\n", __LINE__, i);
    // logData(buffer);
    // flushLog();

    // pfn i is not used
    while (i < 2048 && !PFNTable[i].isUsed) {
      // sprintf(buffer, "[printOutAllUsedFrameThreadId] {line: %d}, {i: %d}.\n", __LINE__, i);
      // logData(buffer);
      // flushLog();
      i++;
    }
  }

  sprintf(buffer, "\n");
  logData(buffer);
  flushLog();

  if (allFrameUnused) {
    sprintf(buffer, "[printOutAllUsedFrameThreadId] All frames (pfn 256-2047, user space) are unused.\n");
    logData(buffer);
    flushLog();
  } else {
    sprintf(buffer, "[printOutAllUsedFrameThreadId] Not all frames (pfn 256-2047, user space) are unused.\n");
    logData(buffer);
    flushLog();
  }

  sprintf(buffer, "[printOutAllUsedFrameThreadId] Finish printing out used frames' threadId:\n\n");
  logData(buffer);
  flushLog();
}