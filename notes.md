## How to run tests
```
cmake .
cmake --build .
./VirtualMemFrameworkC
```

## Tips
1. Read everything under `./src`.
2. `./src/main.c` holds all tests. To turn on the test, de-comment them.
   Tests are in approximately order.
3. `./answer/memory.c`
   1. page size.
      ```
        // 4k is the size of a page
        const int PAGE_SIZE= 4*1024;
      ```
   2. Everything you store on behalf on threads and the page tables must fit in this 8M memory.
      ```
        // A total of 8M exists in memory
        const int ALL_MEM_SIZE = 8*1024*1024;
      ```
    3. So 1M for kernel memory from 0-1M.
      ```
        // USER Space starts at 1M
        const int USER_BASE_ADDR = 1024*1024;
      ```
    4. So in total 2M for stack. The bottom is 8M and it goes up at most to 6M.
      ```
        // Stack starts at 8M and goes down to 6M
        const int STACK_END_ADDR = 6*1024*1024;
      ```
    5. 1M kernel memory for 2048 pages. So each page has 512 Bytes.
       1M / 2*1024 pages = 1024 * 1024 Bytes / 2 * 1024 pages = 512 Bytes / page
      ```
        // There are total of 2048 pages
        const int NUM_PAGES = 2*1024;
      ```
    6. Heap memory starts from 1M to 6M. In total 5M.
    7. System total memory 8M = 1M kernel memory + 5M heap + 2M stack.

## Solution
### `RUN_NON_THREADED_HEAP_TESTS` and `RUN_NON_THREADED_STACK_TESTS`
1. After these two tests turned on, we got:
   ```terminal
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:22:testWriteIntoKernelFails:FAIL: You get this far you haven't had a kernel panic
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:33:testReadHeapFullPage:FAIL: Memory Mismatch. Byte 0 Expected 0xF7 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:48:testReadHeapAcrossTwoPages:FAIL: Memory Mismatch. Byte 0 Expected 0xB1 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:64:testReadHeapMiddleOfPage:FAIL: Memory Mismatch. Byte 0 Expected 0xC9 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:85:testReadHeapPartialPage:FAIL: Memory Mismatch. Byte 0 Expected 0x82 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:109:testReadAllHeapMem:FAIL: Memory Mismatch. Byte 0 Expected 0x47 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:33:testReadStackFullPage:FAIL: Memory Mismatch. Byte 0 Expected 0xFB Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:49:testReadStackAcrossTwoPages:FAIL: Memory Mismatch. Byte 0 Expected 0xEC Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:65:testReadStackMiddleOfPage:FAIL: Memory Mismatch. Byte 0 Expected 0x19 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:86:testReadStackPartialPage:FAIL: Memory Mismatch. Byte 0 Expected 0x26 Was 0x00
    /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:110:testReadAllStackMem:FAIL: Memory Mismatch. Byte 0 Expected 0x69 Was 0x00
   ```

   ```
   /Users/a9527/NEU/cs5600/VirtualFrameworkC/src/main.c:22:testWriteIntoKernelFails:FAIL: You get this far you haven't had a kernel panic
   ```
   This is because we do not have a kernel panic when it is needed.

2. How to Pass Keynel Memory Test Case?
   1. The method `void kernelPanic(const Thread* thread, int addr)` in `./utils.c` is how we invoke kernel panic.
   2. When we need a kernel panic?
      Whenever we are writting to the kernel memory.
   3. Where is kernel memory?
      1. Two kinds of memory - kernel memory and user memory.
      2. In `./answer/memory.c`, we could know that
        - the user memory starts at `1M`.
          ```
          // USER Space starts at 1M
          const int USER_BASE_ADDR = 1024*1024;
          ```
        - the total memory is from 0-8M
        - so the kernel memory is from 0-1M
   4. After we know why we need a kernel panic and where is kernel panic, we could start to add it into our system.
      1. In method `writeToAddr()` from `./answer/memory.c`, add an if statement to check if addr is in the range of kernel memory.
         If so, call a `kernelPanic()`. Same code should be added into the method `readFromAddr()` because reading from kernel memory is also not allowed.