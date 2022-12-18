#include <stdlib.h>
#include "pagingTests.h"
#include "memory.h"
#include "thread.h"
#include "utils.h"
#include "unity.h"

extern const int PAGE_SIZE;
extern const int USER_BASE_ADDR;

void testDataPagedOutCorrectly() {
    char buffer[1024];
    sprintf(buffer, "\n\n[testDataPagedOutCorrectly] Tests start.\n");
    logData(buffer);
    flushLog();

    Thread* thread1 = createThread();
    sprintf(buffer, "[testDataPagedOutCorrectly] {thread1: %d}.\n", thread1->threadId);
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

    sprintf(buffer, "\n\n[testDataPagedOutCorrectly] {line: %d}, {fileName: %s}, {thread1 id: %d}, {thread2: %d}.\n", __LINE__, fileName, thread1->threadId, thread2->threadId);
    logData(buffer);
    flushLog();

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
    int addr = allocateAndWriteHeapData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    int savedAddr = addr;

    while (addr !=-1) {
        addr = allocateAndWriteHeapData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    }

    addr = allocateAndWriteStackData(thread1, data, PAGE_SIZE, PAGE_SIZE);

    while (addr !=-1) {
        addr = allocateAndWriteStackData(thread1, data, PAGE_SIZE, PAGE_SIZE);
    }

    Thread* thread2 = createThread();
    void* data2 = createRandomData(PAGE_SIZE);
    allocateAndWriteHeapData(thread2, data2, PAGE_SIZE, PAGE_SIZE);

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