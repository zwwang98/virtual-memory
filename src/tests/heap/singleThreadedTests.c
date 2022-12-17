#include <stdlib.h>
#include "unity.h"
#include "thread.h"
#include "utils.h"
#include "threadedTestUtil.h"
#include "memory.h"

extern int PAGE_SIZE;
extern bool panicExpected;
extern const int USER_BASE_ADDR;
extern const int STACK_END_ADDR;

void testSingleThreadedWriteHeapAndReadFullPage() {
    TestReadWriteInfoList *list = getTestList(1);
    list->readWriteInfoList[0] = createTestReadWriteInfo(PAGE_SIZE, PAGE_SIZE, 0, true);
    pthread_create(&(list->thread->thread), NULL, writeAndReadMem, list);
    destroyThread(list->thread);
    TEST_ASSERT_EQUAL_MEMORY(list->readWriteInfoList[0].inputData, list->readWriteInfoList[0].outputData, PAGE_SIZE);
    destroyTestReadWriteInfoList(list);
    free(list);
}

void testSingleThreadedWriteHeapAndReadAcrossTwoPages() {
    TestReadWriteInfoList *list = getTestList(1);
    list->readWriteInfoList[0] = createTestReadWriteInfo(PAGE_SIZE*2, PAGE_SIZE*2, 0, true);
    pthread_create(&(list->thread->thread), NULL, writeAndReadMem, list);
    destroyThread(list->thread);
    TEST_ASSERT_EQUAL_MEMORY(list->readWriteInfoList[0].inputData, list->readWriteInfoList[0].outputData, PAGE_SIZE);
    destroyTestReadWriteInfoList(list);
    free(list);
}

void testSingleThreadedWriteHeapAndReadMiddleOfPage() {
    TestReadWriteInfoList *list = getTestList(2);
    list->readWriteInfoList[0] = createTestReadWriteInfo(PAGE_SIZE/2, PAGE_SIZE/2, 0, true);
    list->readWriteInfoList[1] = createTestReadWriteInfo(PAGE_SIZE/4, PAGE_SIZE/4, 0, true);
    pthread_create(&(list->thread->thread), NULL, writeAndReadMem, list);
    destroyThread(list->thread);

    TEST_ASSERT_EQUAL_MEMORY(list->readWriteInfoList[0].inputData,
            list->readWriteInfoList[0].outputData, PAGE_SIZE/2);
    TEST_ASSERT_EQUAL_MEMORY(list->readWriteInfoList[1].inputData,
                             list->readWriteInfoList[1].outputData, PAGE_SIZE/4);
    destroyTestReadWriteInfoList(list);
    free(list);
}

void testSingleThreadedWriteHeapAndReadPartial() {
    TestReadWriteInfoList *list = getTestList(1);
    int randomOffset = rand()%PAGE_SIZE;
    list->readWriteInfoList[0] = createTestReadWriteInfo(PAGE_SIZE, PAGE_SIZE, randomOffset, true);
    pthread_create(&(list->thread->thread), NULL, writeAndReadMem, list);
    destroyThread(list->thread);
    TEST_ASSERT_EQUAL_MEMORY(list->readWriteInfoList[0].inputData+randomOffset,
            list->readWriteInfoList[0].outputData, PAGE_SIZE-randomOffset);
    destroyTestReadWriteInfoList(list);
    free(list);
}

void testSingleThreadedReadAllHeapMem() {
    char buffer[1024];
    sprintf(buffer, "\n[testSingleThreadedReadAllHeapMem] Test starts.\n");
    logData(buffer);
    flushLog();

    TestReadWriteInfoList *list = getTestList(1);
    int size = STACK_END_ADDR - USER_BASE_ADDR;
    list->readWriteInfoList[0] = createTestReadWriteInfo(size, size, 0, true);
    pthread_create(&(list->thread->thread), NULL, writeAndReadMem, list);
    destroyThread(list->thread);
    TEST_ASSERT_EQUAL_MEMORY(list->readWriteInfoList[0].inputData,
                             list->readWriteInfoList[0].outputData, size);
    destroyTestReadWriteInfoList(list);
    free(list);

    sprintf(buffer, "\n[testSingleThreadedReadAllHeapMem] Test ends.\n");
    logData(buffer);
    flushLog();
}

void* beyondEndOfPageThreadWrite(void* input) {
    char buffer[1024];
    sprintf(buffer, "\n[beyondEndOfPageThreadWrite] Starts.\n");
    logData(buffer);
    flushLog();

    Thread* thread = input;
    int addr = allocateHeapMem(thread, PAGE_SIZE/2);
    void* data = createRandomData(PAGE_SIZE);
    writeToAddr(thread, addr, PAGE_SIZE, data);
    free(data);

    sprintf(buffer, "\n[beyondEndOfPageThreadWrite] Ends.\n");
    logData(buffer);
    flushLog();
    return NULL;
}
void testSingleThreadedWriteHeapBeyondEndOfPage() {
    char buffer[1024];
    sprintf(buffer, "\n[testSingleThreadedWriteHeapBeyondEndOfPage] Test starts.\n");
    logData(buffer);
    flushLog();

    sprintf(buffer, "\n[testSingleThreadedWriteHeapBeyondEndOfPage] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    panicExpected = true;
    Thread* thread = createThread();

    sprintf(buffer, "\n[testSingleThreadedWriteHeapBeyondEndOfPage] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    pthread_create(&(thread->thread), NULL, beyondEndOfPageThreadWrite, thread);

    sprintf(buffer, "\n[testSingleThreadedWriteHeapBeyondEndOfPage] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    destroyThread(thread);

    sprintf(buffer, "\n[testSingleThreadedWriteHeapBeyondEndOfPage] {line: %d}.\n", __LINE__);
    logData(buffer);
    flushLog();

    sprintf(buffer, "\n[testSingleThreadedWriteHeapBeyondEndOfPage] Test ends.\n");
    logData(buffer);
    flushLog();
}

