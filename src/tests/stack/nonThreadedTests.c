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

void testWriteIntoKernelFails() {
    panicExpected = true;
    Thread* thread = createThread();
    void* data = createRandomData(PAGE_SIZE);
    writeToAddr(thread, 0, PAGE_SIZE, data);
    destroyThread(thread);
    TEST_FAIL_MESSAGE("You get this far you haven't had a kernel panic");
}

void testReadStackFullPage() {
    char buffer[1024];
    sprintf(buffer, "\n[testReadStackFullPage] Test starts.\n");
    logData(buffer);
    flushLog();

    int printNum = 5;

    Thread *thread = createThread();
    void *data = createRandomData(PAGE_SIZE);
    sprintf(buffer, "\n[testReadStackFullPage] Print out first %d of data: ", printNum);
    logData(buffer);
    flushLog();
    int y = 0;
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

    int addr = allocateAndWriteStackData(thread, data, PAGE_SIZE, PAGE_SIZE);
    sprintf(buffer, "[testReadStackFullPage] Stack starts at %d.\n", addr);
    logData(buffer);
    flushLog();

    void *readData = malloc(PAGE_SIZE);
    bzero(readData, PAGE_SIZE);
    readFromAddr(thread, addr, PAGE_SIZE, readData);

    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE);

    free(data);
    free(readData);
    destroyThread(thread);
    sprintf(buffer, "\n[testReadStackFullPage] Test ends.\n");
    logData(buffer);
    flushLog();
}

void testReadStackAcrossTwoPages() {
    char buffer[1024];
    sprintf(buffer, "\n[testReadStackAcrossTwoPages] Test starts.\n");
    logData(buffer);
    flushLog();

    Thread* thread = createThread();

    void* data = createRandomData(PAGE_SIZE*2);

    int printNum = 5;
    sprintf(buffer, "\n[testReadStackAcrossTwoPages] Print out first %d of data: ", printNum);
    logData(buffer);
    flushLog();
    int y = 0;
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

    int addr = allocateAndWriteStackData(thread, data, PAGE_SIZE * 2, PAGE_SIZE * 2);

    void *readData = malloc(PAGE_SIZE*2);
    bzero(readData, PAGE_SIZE*2);
    readFromAddr(thread, addr, PAGE_SIZE*2, readData);

    sprintf(buffer, "\n[testReadStackAcrossTwoPages] Print out first %d of readData: ", printNum);
    logData(buffer);
    flushLog();
    char *readDataPtr = readData;
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

    TEST_ASSERT_EQUAL_MEMORY(data, readData, PAGE_SIZE*2);

    sprintf(buffer, "\n[testReadStackAcrossTwoPages] {Line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    free(data);
    free(readData);
    destroyThread(thread);

    sprintf(buffer, "\n[testReadStackAcrossTwoPages] Test ends.\n");
    logData(buffer);
    flushLog();
}

void testReadStackMiddleOfPage() {
    Thread* thread = createThread();
    void* data = createRandomData(PAGE_SIZE/2);
    void* secondData = createRandomData(PAGE_SIZE/4);
    int addr = allocateAndWriteStackData(thread, data, PAGE_SIZE / 2, PAGE_SIZE / 2);
    int secondAddr = allocateAndWriteStackData(thread, secondData, PAGE_SIZE / 4, PAGE_SIZE / 4);

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

void testReadStackPartialPage() {
    Thread* thread = createThread();
    void* data = createRandomData(PAGE_SIZE*3/4);
    int addr = allocateAndWriteStackData(thread, data, PAGE_SIZE, PAGE_SIZE / 2);
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

void testReadAllStackMem() {
    char buffer[1024];
    sprintf(buffer, "\n[testReadAllStackMem] Test starts.\n");
    logData(buffer);
    flushLog();

    Thread *thread = createThread();
    int size = ALL_MEM_SIZE-STACK_END_ADDR;
    void *data = createRandomData(size);
    int addr = allocateAndWriteStackData(thread, data, size, size);
    void *readData = malloc(size);
    bzero(readData, size);
    readFromAddr(thread, addr, size, readData);

    TEST_ASSERT_EQUAL_MEMORY(data, readData, size);
    free(data);
    free(readData);
    destroyThread(thread);

    sprintf(buffer, "\n[testReadAllStackMem] Test ends.\n\n");
    logData(buffer);
    flushLog();
}
