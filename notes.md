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