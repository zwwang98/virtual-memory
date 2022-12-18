#include <stdlib.h>
#include "pagingTests.h"
#include "memory.h"
#include "thread.h"
#include "utils.h"
#include "unity.h"
#include "unistd.h"

extern const int PAGE_SIZE;
extern const int USER_BASE_ADDR;

void testDataPagedOutCorrectly() {
    char buffer[1024];
    sprintf(buffer, "\n\n[testDataPagedOutCorrectly] Tests start.\n");
    logData(buffer);
    flushLog();

    Thread* thread1 = createThread();
    sprintf(buffer, "[testDataPagedOutCorrectly] {line: %d} {thread1: %d}.\n", __LINE__, thread1->threadId);
    logData(buffer);
    flushLog();

    void *data = createRandomData(PAGE_SIZE);
    int addr = allocateAndWriteHeapData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    int savedAddr = addr;
    while (addr !=-1) {
        addr = allocateAndWriteHeapData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    }

    sprintf(buffer, "\n\n[testDataPagedOutCorrectly] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    addr = allocateAndWriteStackData(thread1, data, PAGE_SIZE, PAGE_SIZE);

    while (addr !=-1) {
        addr = allocateAndWriteStackData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    }

    sprintf(buffer, "\n\n[testDataPagedOutCorrectly] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    Thread* thread2 = createThread();
    void* data2 = createRandomData(PAGE_SIZE);
    addr = allocateAndWriteHeapData(thread2, data2, PAGE_SIZE, PAGE_SIZE);

    char* fileName = getCacheFileName(thread1, savedAddr);
    FILE* file = fopen(fileName, "r+");
    void* fileData = malloc(PAGE_SIZE);

    sprintf(buffer, "[testDataPagedOutCorrectly] {line: %d}, {fileName: %s}, {thread1 id: %d}, {thread2: %d}.\n", __LINE__, fileName, thread1->threadId, thread2->threadId);
    logData(buffer);
    flushLog();

    if (access(fileName, F_OK) == 0) {
        sprintf(buffer, "[testDataPagedOutCorrectly] {line: %d}, {fileName: %s} exists.\n", __LINE__, fileName);
        logData(buffer);
        flushLog();
    } else {
        sprintf(buffer, "[testDataPagedOutCorrectly] {line: %d}, {fileName: %s} does not exist.\n", __LINE__, fileName);
        logData(buffer);
        flushLog();
    }

    fread(fileData, 1,  PAGE_SIZE, file);

    sprintf(buffer, "\n\n[testDataPagedOutCorrectly] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    TEST_ASSERT_EQUAL_MEMORY(data, fileData, PAGE_SIZE);
    fclose(file);
    destroyThread(thread1);
    destroyThread(thread2);
    free(data);
    free(data2);

    sprintf(buffer, "[testDataPagedOutCorrectly] Tests end.\n\n");
    logData(buffer);
    flushLog();
}

void testDataPagedInCorrectly() {
    char buffer[1024];
    sprintf(buffer, "\n\n[testDataPagedInCorrectly] Tests start.\n");
    logData(buffer);
    flushLog();

    Thread* thread1 = createThread();
    void *data = createRandomData(PAGE_SIZE);

    // fill thread1's heap
    sprintf(buffer, "[testDataPagedInCorrectly] {thread: %d} allocateAndWriteHeapData() starts.\n", thread1->threadId);
    logData(buffer);
    flushLog();
    int addr = allocateAndWriteHeapData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    int savedAddr = addr;
    while (addr !=-1) {
        addr = allocateAndWriteHeapData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    }
    sprintf(buffer, "[testDataPagedInCorrectly] {thread: %d} allocateAndWriteHeapData() ends.\n", thread1->threadId);
    logData(buffer);
    flushLog();

    // fill thread1's stack
    sprintf(buffer, "[testDataPagedInCorrectly] {thread: %d} allocateAndWriteStackData() starts.\n", thread1->threadId);
    logData(buffer);
    flushLog();
    addr = allocateAndWriteStackData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    while (addr !=-1) {
        addr = allocateAndWriteStackData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    }
    sprintf(buffer, "[testDataPagedInCorrectly] {thread: %d} allocateAndWriteStackData() ends.\n", thread1->threadId);
    logData(buffer);
    flushLog();

    Thread* thread2 = createThread();
    void* data2 = createRandomData(PAGE_SIZE);

    // use thread2's to take one frame, so that one frame havs to be evicted from existing frames,
    // and all existing frames are owned by thread1, so thread1 have to evict one frame, and it will be vpn256 based on current clock algorithm
    sprintf(buffer, "[testDataPagedInCorrectly] {thread: %d} allocateAndWriteHeapData() starts.\n", thread2->threadId);
    logData(buffer);
    flushLog();
    allocateAndWriteHeapData(thread2, data2, PAGE_SIZE, PAGE_SIZE);
    sprintf(buffer, "[testDataPagedInCorrectly] {thread: %d} allocateAndWriteHeapData() ends.\n", thread2->threadId);
    logData(buffer);
    flushLog();

    sprintf(buffer, "[testDataPagedInCorrectly] {savedAddr: %d}.\n", savedAddr);
    logData(buffer);
    flushLog();

    void* readData = malloc(PAGE_SIZE);
    readFromAddr(thread1, savedAddr, PAGE_SIZE, readData);

    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE);
    destroyThread(thread1);
    destroyThread(thread2);
    free(data);
    free(data2);
    free(readData);

    sprintf(buffer, "[testDataPagedInCorrectly] Tests ends.\n");
    logData(buffer);
    flushLog();
}