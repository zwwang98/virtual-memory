1. allow to malloc PageListNode (allocateStackMem() and allocateHeapMem() in memory.c)?


void readFromAddr(Thread* thread, int addr, int size, void* outData);
 1. find virtual page number given addr
 2. read page by page until reading all size data
    during the process, convert the virtual page number into physical page number

void writeToAddr(const Thread* thread, int addr, int size, const void* data);
 1. find virtual page number given addr
 2. write page by page until writing all size data
    during the process, convert the virtual page number into physical page number and assign new page if needed


int allocateHeapMem(Thread *thread, int size); or int allocateStackMem(Thread *thread, int size);
 1. allocate virtual memory of size to given thread
 2. no need to assign physical frame for now
    instead, assign new physical frame during the process of writting if needed

getCacheFileName()
 1. the cache file contains the data of one page of certian thread,
    so the file needs at least two identifiers - threadID and pageID(vpn)
 2. so the cache file name could be "xxx-${threadID}-${pageID}"