#include <stdbool.h>
#include "tests/utils.h"
#include "tests/heap/nonThreadedTests.h"
#include "tests/heap/singleThreadedTests.h"
#include "tests/heap/multiThreadedTests.h"
#include "tests/stack/nonThreadedTests.h"
#include "tests/stack/singleThreadedTests.h"
#include "tests/stack/multiThreadedTests.h"
#include "pagingTests.h"
#include "unity.h"
#include "system.h"

// Phase 1
#define RUN_NON_THREADED_HEAP_TESTS
#define RUN_NON_THREADED_STACK_TESTS

// Phase 1.1
#define RUN_SINGLE_THREADED_HEAP_TESTS
#define RUN_SINGLE_THREADED_STACK_TESTS

// Phase 2 
#define RUN_MULTI_THREADED_HEAP_TESTS
#define RUN_MULTI_THREADED_STACK_TESTS

// Phase 3 4 5
#define RUN_PAGING_TESTS

// Phase ....
//#define EXTRA_LONG_RUNNING_TESTS


extern bool panicExpected;

void setUp() {
    systemInit();
    panicExpected = false;
}

void tearDown() {
    systemShutdown();
}

int main(void) {
    UNITY_BEGIN();
    // RUN_TEST(testWriteIntoKernelFails);
    #ifdef RUN_NON_THREADED_HEAP_TESTS
    RUN_TEST(testReadHeapFullPage); // use 1 page
    RUN_TEST(testReadHeapAcrossTwoPages);  // use 2 page
    RUN_TEST(testReadHeapMiddleOfPage);  // use 1/2 page + 1/4 page
    RUN_TEST(testReadHeapPartialPage);  // use 3/4 page
    RUN_TEST(testReadAllHeapMem);  // use all heap
    #endif
    #ifdef RUN_NON_THREADED_STACK_TESTS
    RUN_TEST(testReadStackFullPage);
    RUN_TEST(testReadStackAcrossTwoPages);
    RUN_TEST(testReadStackMiddleOfPage);
    RUN_TEST(testReadStackPartialPage);
    RUN_TEST(testReadAllStackMem);
    #endif
    #ifdef RUN_SINGLE_THREADED_HEAP_TESTS
    RUN_TEST(testSingleThreadedWriteHeapAndReadFullPage);
    RUN_TEST(testSingleThreadedWriteHeapAndReadAcrossTwoPages);
    RUN_TEST(testSingleThreadedWriteHeapAndReadMiddleOfPage);
    RUN_TEST(testSingleThreadedWriteHeapAndReadPartial);
    RUN_TEST(testSingleThreadedWriteHeapBeyondEndOfPage);  // due to kernel panic, some frame is not released
    RUN_TEST(testSingleThreadedReadAllHeapMem);

    #endif
    #ifdef RUN_SINGLE_THREADED_STACK_TESTS
    RUN_TEST(testSingleThreadedWriteStackAndReadFullPage);
    RUN_TEST(testSingleThreadedWriteStackAndReadAcrossTwoPages);
    RUN_TEST(testSingleThreadedWriteStackAndReadMiddleOfPage);
    RUN_TEST(testSingleThreadedWriteStackAndReadPartial);
    RUN_TEST(testSingleThreadedWriteStackBeyondEndOfPage);  // bug
    RUN_TEST(testSingleThreadedWriteStackAndReadAllStack);
    #endif
    #ifdef RUN_MULTI_THREADED_HEAP_TESTS
    RUN_TEST(testMultiThreadedWriteHeapAndReadFullPage);
    RUN_TEST(testMultiThreadedWriteHeapAndReadAcrossTwoPages);
    RUN_TEST(testMultiThreadedWriteHeapAndReadMiddleOfPage);
    RUN_TEST(testMultiThreadedWriteHeapAndReadPartial);
    #endif
    #ifdef RUN_MULTI_THREADED_STACK_TESTS
    RUN_TEST(testMultiThreadedWriteStackAndReadFullPage);
    RUN_TEST(testMultiThreadedWriteStackAndReadAcrossTwoPages);
    RUN_TEST(testMultiThreadedWriteStackAndReadMiddleOfPage);
    RUN_TEST(testMultiThreadedWriteStackAndReadPartial);
    #endif
    #ifdef RUN_PAGING_TESTS
    RUN_TEST(testDataPagedInCorrectly);
    // RUN_TEST(testDataPagedOutCorrectly);
    #endif
    #ifdef EXTRA_LONG_RUNNING_TESTS
    RUN_TEST(testMultiThreadedReadAllHeapMemory);
    RUN_TEST(testMultiThreadedReadAllStackMemory);
    #endif

    return UNITY_END();

}
