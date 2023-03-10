#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "unity.h"
#include "thread.h"
#include "memory.h"
#include "utils.h"

extern const int ALL_MEM_SIZE;
extern const int NUM_PAGES;
extern const int PAGE_SIZE;
extern const int USER_BASE_ADDR;
extern const int STACK_END_ADDR;
extern bool panicExpected;

void testWriteHeapBeyondEndOfPage() {
    panicExpected = true;
    Thread* thread = createThread();
    int addr = allocateHeapMem(thread, PAGE_SIZE/2);
    void* data = createRandomData(PAGE_SIZE);
    writeToAddr(thread, addr, PAGE_SIZE, data);
    destroyThread(thread);
}

void testReadHeapFullPage() {
    char buffer[1024];
    sprintf(buffer, "\n[testReadHeapFullPage] Test starts.\n");
    logData(buffer);
    flushLog();

    Thread *thread = createThread();
    void *data = createRandomData(PAGE_SIZE);

    int addr = allocateAndWriteHeapData(thread, data, PAGE_SIZE, PAGE_SIZE);
    sprintf(buffer, "\n[testReadHeapFullPage] {addr: %d}.\n", addr);
    logData(buffer);
    flushLog();


    void *readData = malloc(PAGE_SIZE);
    bzero(readData, PAGE_SIZE);
    readFromAddr(thread, addr, PAGE_SIZE, readData);

    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE);
    free(data);
    free(readData);
    destroyThread(thread);

    sprintf(buffer, "\n[testReadHeapFullPage] Test ends.\n\n");
    logData(buffer);
    flushLog();
}

void testReadHeapAcrossTwoPages() {
    Thread* thread = createThread();
    void* data = createRandomData(PAGE_SIZE*2);
    int addr = allocateAndWriteHeapData(thread, data, PAGE_SIZE * 2, PAGE_SIZE * 2);

    void *readData = malloc(PAGE_SIZE*2);
    bzero(readData, PAGE_SIZE*2);
    readFromAddr(thread, addr, PAGE_SIZE*2, readData);

    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE*2);
    free(data);
    free(readData);
    destroyThread(thread);
}

void testReadHeapMiddleOfPage() {
    Thread* thread = createThread();
    void* data = createRandomData(PAGE_SIZE/2);
    void* secondData = createRandomData(PAGE_SIZE/4);
    int addr = allocateAndWriteHeapData(thread, data, PAGE_SIZE / 2, PAGE_SIZE / 2);
    int secondAddr = allocateAndWriteHeapData(thread, secondData, PAGE_SIZE / 4, PAGE_SIZE / 4);

    void *readData = malloc(PAGE_SIZE/2);
    bzero(readData, PAGE_SIZE/2);
    readFromAddr(thread, addr, PAGE_SIZE/2, readData);
    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE/2);

    bzero(readData, PAGE_SIZE/2);
    readFromAddr(thread, secondAddr, PAGE_SIZE/4, readData);
    TEST_ASSERT_EQUAL_MEMORY(secondData, readData, PAGE_SIZE/4);

    free(data);
    free(secondData);
    free(readData);
    destroyThread(thread);
}

void testReadHeapPartialPage() {
    Thread* thread = createThread();
    void* data = createRandomData(PAGE_SIZE*3/4);
    int addr = allocateAndWriteHeapData(thread, data, PAGE_SIZE, PAGE_SIZE / 2);
    writeToAddr(thread, addr+(PAGE_SIZE/2), (PAGE_SIZE/4), data+(PAGE_SIZE/2));

    void *readData = malloc(PAGE_SIZE*3/4);
    bzero(readData, PAGE_SIZE*3/4);
    readFromAddr(thread, addr, PAGE_SIZE*3/4, readData);
    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE*3/4);

    bzero(readData, PAGE_SIZE*3/4);
    readFromAddr(thread, addr, PAGE_SIZE/2, readData);
    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE/2);

    bzero(readData, PAGE_SIZE*3/4);
    readFromAddr(thread, addr+(PAGE_SIZE/2), PAGE_SIZE/4, readData);
    TEST_ASSERT_EQUAL_MEMORY(data+(PAGE_SIZE/2), readData, PAGE_SIZE/4);

    free(data);
    free(readData);
    destroyThread(thread);
}

void testReadAllHeapMem() {
    char buffer[1024];
    sprintf(buffer, "\n[testReadAllHeapMem] Test starts.\n");
    logData(buffer);
    flushLog();
    int printNum = 5;

    Thread *thread = createThread();
    int size = STACK_END_ADDR - USER_BASE_ADDR;

    void *data = createRandomData(size);
    sprintf(buffer, "\n[testReadAllHeapMem] Print out first %d of data: ", printNum);
    logData(buffer);
    flushLog();
    int y = 5230589;
    char *dataPtr = data + y;
    for (int i = 0; i < printNum; i++) {
        sprintf(buffer, "0x%x, ", *(dataPtr++));
        logData(buffer);
        flushLog();
        if (i == printNum - 1) {
            sprintf(buffer, "\n\n");
            logData(buffer);
            flushLog();
        }
    }

    int addr = allocateAndWriteHeapData(thread, data, size, size);
    sprintf(buffer, "[testReadAllHeapMem] Heap starts at %d.\n", addr);
    logData(buffer);
    flushLog();


    void *readData = malloc(size);
    bzero(readData, size);
    readFromAddr(thread, addr, size, readData);

    sprintf(buffer, "[testReadAllHeapMem] Print out first %d of readData: ", printNum);
    logData(buffer);
    flushLog();
    char *readDataPtr = readData + y;
    for (int i = 0; i < printNum; i++) {
        sprintf(buffer, "0x%x, ", *(readDataPtr++));
        logData(buffer);
        flushLog();
        if (i == printNum - 1) {
            sprintf(buffer, "\n\n");
            logData(buffer);
            flushLog();
        }
    }

    TEST_ASSERT_EQUAL_MEMORY(data, readData, size);
    free(data);
    free(readData);
    destroyThread(thread);

    sprintf(buffer, "\n[testReadAllHeapMem] Test ends.\n\n");
    logData(buffer);
    flushLog();
}