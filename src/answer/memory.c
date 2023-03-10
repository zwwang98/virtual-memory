#include "thread.h"
#include "./utils.h"
#include "FrameTable.h"
#include <string.h>

#define FILENAME_TEMPLATE ".page_thread_%d_vpn_%d"

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

// total 256 4KB pages for kernel space (256 * 4KB = 1MB)
const int NUM_KERNEL_SPACE_PAGES = NUM_PAGES / 8;

// total 1792 4KB pages for user space (1792 * 4KB = 7MB)
const int NUM_USER_SPACE_PAGES = NUM_PAGES * 7 / 8;

// Each page has 512 Bytes to store its meta data.
// 1M / 2*1024 pages = 1024 * 1024 Bytes / 2 * 1024 pages = 512 Bytes / page
const int SPACE_PER_PAGE = 512;

// 8MB memory
char RAW_SYSTEM_MEMORY_ACCESS[8 * 1024 * 1024];
void *SYSTEM_MEMORY = RAW_SYSTEM_MEMORY_ACCESS;

// frame table
FrameEntry PFNTable[NUM_PAGES];

void initPFNTable() {
  for (int pfn = 0; pfn < NUM_PAGES; pfn++) {
    PFNTable[pfn].thread = NULL;
    PFNTable[pfn].isUsed = false;
    PFNTable[pfn].dirty = false;
    PFNTable[pfn].vpn = 0;
  }
}

// https://stackoverflow.com/questions/23400097/c-confused-on-how-to-initialize-and-implement-a-pthread-mutex-and-condition-vari
// initialize a mutex lock
// lock - pthread_mutex_lock(&lock);
// unlok - pthread_mutex_unlock(&lock);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// clock algorithm pointer
int clockPtr = 256;

// some variables help for debug
const int bitToPrint = 5;
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
 * @brief Choose frame to evict using clock algorithm.
 * 
 * @param thread Given thread.
 * @return int The vpn of the page of given thread to be evicted.
 */
int choosePageToEvict(Thread *thread) {
  char buffer[1024];
  sprintf(buffer, "\n\n[choosePageToEvict] Starts\n");
  logData(buffer);
  flushLog();

  while (clockPtr < 2048) {
    if (thread->VPNToPFN[clockPtr].present && !thread->VPNToPFN[clockPtr].accessed) {
      break;
    }
    clockPtr++;
  }

  if (clockPtr == 2048) {
    clockPtr = 256;
  }

  sprintf(buffer, "[choosePageToEvict] Ends. Choose {vpn: %d} to evict.\n\n", clockPtr);
  logData(buffer);
  flushLog();

  return clockPtr;
}

/**
 * @brief Given thread and vpn, return the expected file name of the file storing the data in this page.
 * 
 * @param threadId Id of given thread.
 * @param vpn Given virtual page number.
 * @return char* The expected file name of the file storing the data in this page.
 */
char* outPageName(int threadId, int vpn) {
  static char fileName[1024];
  snprintf(fileName, sizeof(fileName), FILENAME_TEMPLATE, threadId, vpn);
  return fileName;
}

/**
 * @brief Evict given thread's given page from given memory pointer into disc.
 * 
 * @param pfn The number of the frame to be evicted.
 */
void evictPage(int pfn) {
  char buffer[1024];
  sprintf(buffer, "\n\n[evictPage] Starts evict frame {pfn: %d}.\n", pfn);
  logData(buffer);
  flushLog();

  Thread* thread = PFNTable[pfn].thread;
  int threadId = thread->threadId;
  int vpn = PFNTable[pfn].vpn;

  static char cacheFileName[1024];
  snprintf(cacheFileName, sizeof(cacheFileName), FILENAME_TEMPLATE, threadId, vpn);

  sprintf(buffer, "[evictPage] {filename: %s}, {present: %d}\n", cacheFileName, thread->VPNToPFN[vpn].present);
  logData(buffer);
  flushLog();

  thread->VPNToPFN[vpn].present = false;

  void* memoryPtr = (SYSTEM_MEMORY + (pfn - 1) * PAGE_SIZE);

  sprintf(buffer, "[evictPage] {filename: %s}, {present: %d}, {memoryPtrIdx: %d}\n", cacheFileName, thread->VPNToPFN[vpn].present, (pfn - 1) * PAGE_SIZE);
  logData(buffer);
  flushLog();

  sprintf(buffer, "[evictPage] Print out first 5 data at {memoryPtr: %d}: ", (pfn - 1) * PAGE_SIZE);
  logData(buffer);
  flushLog();
  char* dataPtr = (char*) memoryPtr;
  for (int i = 0; i < 5; i++) {
    sprintf(buffer, "%d, ", *dataPtr++);
    logData(buffer);
    flushLog();
  }
  sprintf(buffer, "\n");
  logData(buffer);
  flushLog();

  FILE *file = fopen(cacheFileName, "w");
  // https://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm
  // write data in {memory} into {file}
  fwrite(memoryPtr, PAGE_SIZE, 1, file);
  fclose(file);

  sprintf(buffer, "[evictPage] Ends\n\n");
  logData(buffer);
  flushLog();
}

/**
 * @brief Given frame number and file name, load the data in the file and put it into the frame.
 * 
 * @param pfn The number of given frame.
 * @param filename Given cache file name.
 */
void loadPage(int pfn, char* filename) {
  char buffer[1024];
  sprintf(buffer, "\n\n[loadPage] Loading page...\n");
  logData(buffer);
  flushLog();

  Thread* thread = PFNTable[pfn].thread;
  int vpn = PFNTable[pfn].vpn;
  thread->VPNToPFN[vpn].present = true;

  sprintf(buffer, "[loadPage] {filename: %s}\n", filename);
  logData(buffer);
  flushLog();

  // https://pubs.opengroup.org/onlinepubs/007904975/functions/fopen.html
  // open the given file whose name is {filename} in read mode
  FILE *file = fopen(filename, "r");
  void* memoryPtr = SYSTEM_MEMORY + (pfn - 1) * PAGE_SIZE;
  if (file) {
      // https://www.tutorialspoint.com/c_standard_library/c_function_fread.htm
      // read data in {file} into {memoryPtr}
      fread(memoryPtr, PAGE_SIZE, 1, file);
      fclose(file);
  }

  sprintf(buffer, "[loadPage] Page loaded...\n\n");
  logData(buffer);
  flushLog();
}

/**
 * @brief Allocate one frame to given page of given thread.
 * 
 * @param thraed Given thread.
 * @param vpn Given virtual page number.
 */
void allocateFrameToPage(Thread *thread, int vpn) {
  char buffer[1024];
  sprintf(buffer, "\n\n[allocateFrameToPage] Starts.\n");
  logData(buffer);
  flushLog();

  int firstEmptyPFN = NUM_KERNEL_SPACE_PAGES;
  while (firstEmptyPFN < NUM_PAGES) {
    if (!PFNTable[firstEmptyPFN].isUsed) {
      break;
    }
    firstEmptyPFN++;
  }

  if (firstEmptyPFN == NUM_PAGES) {
    sprintf(buffer, "[allocateFrameToPage] Need evict some page.\n");
    logData(buffer);
    flushLog();
    firstEmptyPFN = choosePageToEvict(thread);
    if (PFNTable[firstEmptyPFN].dirty) {
      void* memoryPtr = SYSTEM_MEMORY + (firstEmptyPFN - 1) * PAGE_SIZE;
      evictPage(firstEmptyPFN);
    }
  }

  // PFNTable[firstEmptyPFN]
  PFNTable[firstEmptyPFN].thread = thread;
  PFNTable[firstEmptyPFN].vpn = vpn;
  PFNTable[firstEmptyPFN].isUsed = true;
  PFNTable[firstEmptyPFN].dirty = false;
  // update thread's page table
  thread->VPNToPFN[vpn].physicalFrameNumber = firstEmptyPFN;
  thread->VPNToPFN[vpn].present = true;

  sprintf(buffer, "[allocateFrameToPage] Allocate {pfn: %d} to {threadId: %d} {vpn: %d}, {present: %d}.\n", firstEmptyPFN, thread->threadId, vpn, thread->VPNToPFN[vpn].present);
  logData(buffer);
  flushLog();

  sprintf(buffer, "[allocateFrameToPage] ends.\n\n");
  logData(buffer);
  flushLog();
}

// assign frames to thread to
void allocateMemory(Thread *thread, int begin, int end, int size) {
  char buffer[1024];
  sprintf(buffer, "\n[allocateMemory] Start to allocate {size: %d} memory from %d to %d to {thread: %d}.\n", size, begin, end, thread->threadId);
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

  // sprintf(buffer, "[allocateMemory] {begin: %d}, {beginPageNum: %d}, {end: %d}, {endPageNum: %d}, {curPageNum: %d}.\n", begin, beginPageNum, end, endPageNum, curPageNum);
  // logData(buffer);
  // flushLog();

  while (curPageNum <= endPageNum) {
    allocateFrameToPage(thread, curPageNum);
    curPageNum++;
  }

  pthread_mutex_unlock(&lock);
  sprintf(buffer, "[allocateMemory] {line: %d} {thread: %d} returns the lock.\n", __LINE__, thread->threadId);
  logData(buffer);
  flushLog();


  sprintf(buffer, "[allocateMemory] Finish allocating {size: %d} memory from %d to %d.\n\n", size, begin, end);
  logData(buffer);
  flushLog();

  printOutAllUnusedFrameIntervals(thread);
  printOutAllUsedFrameThreadId(thread);
}

int allocateHeapMem(Thread *thread, int size) {
  char buffer[1024];
  sprintf(buffer, "[allocateHeapMem] Start to allocate heap mem.\n");
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

void writeToAddr(Thread* thread, int addr, int size, const void* data) {
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

  // if (bitToPrint > 0) {
  //   char* _dataPtr = (char*) data;
  //   char* partOfData;
  //   sprintf(buffer, "[writeToAddr] {line: %d} Show frist %d numbers of data: ", __LINE__, bitToPrint);
  //   logData(buffer);
  //   flushLog();

  //   for (int i = 0; i < bitToPrint; i++) {
  //     if (i == bitToPrint - 1) {
  //       sprintf(buffer, "0x%x\n", *_dataPtr++);
  //     } else {
  //       sprintf(buffer, "0x%x, ", *_dataPtr++);
  //     }
  //     logData(buffer);
  //     flushLog();
  //   }
  // }

  char* dataPtr = (char*) data;
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

  int z = 1;
  while (size > 0 && ((USER_BASE_ADDR <= addr && addr < thread->heapBottom) || (thread->stackTop <= addr && addr < ALL_MEM_SIZE))) {
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
    int memoryIdx = (pfn - 1) * PAGE_SIZE + offset;

    if (z-- > 0) {
      sprintf(buffer, "[writeToAddr] {line: %d} {vpn: %d} {present: %d}\n", __LINE__, vpn, thread->VPNToPFN[vpn].present);
      logData(buffer);
      flushLog();
    }

    thread->VPNToPFN[vpn].accessed = 1;
    PFNTable[pfn].dirty = true;

    RAW_SYSTEM_MEMORY_ACCESS[memoryIdx] = *dataPtr;
    // sprintf(buffer, "[Line] %d\n", __LINE__);
    // logData(buffer);
    // flushLog();

    if (originalSize - size < bitToPrint) {
      partOfData[originalSize - size] = *dataPtr;
    }
    if (vpn == 256) {
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

  // if (bitToPrint > 0) {
  //   sprintf(buffer, "[writeToAddr] {Line: %d} Show frist %d numbers of data that have been written into memory: ", __LINE__, bitToPrint);
  //   logData(buffer);
  //   flushLog();
  //   for (int i = 0; i < bitToPrint; i++) {
  //     if (i == bitToPrint - 1) {
  //       sprintf(buffer, "0x%x\n", partOfData[i]);
  //     } else {
  //       sprintf(buffer, "0x%x, ", partOfData[i]);
  //     }
  //     logData(buffer);
  //     flushLog();
  //   }
  // }

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

  int z = 2;
  while (size > 0 && ((USER_BASE_ADDR <= addr && addr < thread->heapBottom) || (thread->stackTop <= addr && addr < ALL_MEM_SIZE))) {
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
    int memoryIdx = (pfn - 1) * PAGE_SIZE + offset;
    if (z-- > 0) {
      sprintf(buffer, "[readFromAddr] {line: %d} {vpn: %d} {present: %d}\n", __LINE__, vpn, thread->VPNToPFN[vpn].present);
      logData(buffer);
      flushLog();
    }
    if (z-- > 0 && !thread->VPNToPFN[vpn].present) {
      sprintf(buffer, "[readFromAddr] {line: %d} Need to assign a frame to {vpn: %d} and then load from cache\n", __LINE__, vpn);
      logData(buffer);
      flushLog();
      allocateFrameToPage(thread, vpn);

      int curPfn = thread->VPNToPFN[vpn].physicalFrameNumber;

      static char cacheFileName[1024];
      int vpn = addr / PAGE_SIZE;
      snprintf(cacheFileName, sizeof(cacheFileName), FILENAME_TEMPLATE, thread->threadId, vpn);
      
      // given frame {pfn: curPfn} and cache file name, load the data in the file into the frame
      loadPage(curPfn, cacheFileName);
    }

    thread->VPNToPFN[vpn].accessed = true;

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
  static char buffer[1024];
  sprintf(buffer, "\n\n[getCacheFileName] Start \n");
  logData(buffer);
  flushLog();

  static char cacheFileName[1024];
  int vpn = addr / PAGE_SIZE;
  snprintf(cacheFileName, sizeof(cacheFileName), FILENAME_TEMPLATE, thread->threadId, vpn);

  sprintf(buffer, "[getCacheFileName] End\n");
  logData(buffer);
  flushLog();

  return cacheFileName;
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

  for (int pfn = NUM_KERNEL_SPACE_PAGES; pfn < NUM_PAGES; pfn++) {
    // not owned by any thread
    if (PFNTable[pfn].thread == NULL) {
      continue;
    }
    allFrameUnused = false;
    // get all frame owned by current thread
    int curThreadId = PFNTable[pfn].thread->threadId;
    int j = pfn;
    while (j < NUM_PAGES) {
      if (PFNTable[j].thread && PFNTable[j].thread->threadId == curThreadId) {
        j++;
      } else {
        break;
      }
    }
    // after the while loop, frams [pfn, j-1] are owned by current thread
    sprintf(buffer, "{frames [%d, %d], thread: %d}.", pfn, j - 1, curThreadId);
    logData(buffer);
    flushLog();

    pfn = j - 1;
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