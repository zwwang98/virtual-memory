1. Describe your design of the page table and frame tables. How and why did you store your data the way you did?
   page tables
   1. 2048 length page table entry array
   2. index - virtual page number in that thread
   3. value - page table entry containing information including corresponding physical frame number, accessed bit, present bit, and dirty bit.

   frame tables
   1. 2048 length frame table entry array
   2. index - physical frame number
   3. value - frame table entry containing information including the thread holding the frame, a bool indicating whether this frame is dirty.

2. Describe your algorithm for finding the frame associated with a page.
   1. Given a vpn, find the associated virtual page entry in the thread's page table.
   2. In the associated virtual page entry, find the associated pyhsical frame number.
      The pfn is initialized as -1 when a process is created.
      So if the pfn is not -1, it means that there is a frame allocated for that page.
   3. Given pfn, if it is not -1, find the associated physical frame entry in the global frame table.
      Thus we found the frame.

3. When two user processes both need a new frame at the same time, how are races avoided?
  A lock over physical frame table is needed when one thread is scanning the frame table so that at any moment, only one process is accessing the frame table,
  thus no two threads will have a chance to access the same frame.

4. Why did you choose the data structure(s) that you did for representing virtual-to-physical mappings?
  Because the relationship between virtual page and physical frame is one-to-one mapping. One virtual page has either no frame or one frame.
  And we are doing virtualization, we want to virtualize the whole memory for every process.
  So my idea is to give every process a page-to-pfn map, and use another frame array to store each frame's information.

5. When a frame is required but none is free, some frame must be evicted. Describe your code for choosing a frame to evict.
  Use clock algorithm to choose the frame to be evicted.
  1. Loop through the frame table, find the first frame that is not accessed recently.
  2. If there is no un-accessed frame after loop through all frames once, choose the start frame to be evicted.

6. When a process P obtains a frame that was previously used by a process Q, how do you adjust the page table (and any other data structures) to reflect the frame Q no longer has?
  Change 
  1. In process, modify the process' page-to-frame table, remove the mapping between the frame and the page holding the frame previously.
  2. In the global frame table, clear everything in that frame, make it un-accessed, un-dirty, and not-owned-by-any-process.

7. A page fault in process P can cause another process Q's frame to be evicted.  How do you ensure that Q cannot access or modify the page during the eviction process?  How do you avoid a race between P evicting Q's frame and Q faulting the page back in?
  Add a lock over the whole frame table or let each frame has its own lock, so that no other process could access or modify the page during the eviction process.

8. Suppose a page fault in process P causes a page to be read from the file system or swap.  How do you ensure that a second process Q cannot interfere by e.g. attempting to evict the frame while it is still being read in?
  Add a lock over the whole frame table or let each frame has its own lock, so that no other process could not evict the frame.

9.  A single lock for the whole VM system would make synchronization easy, but limit parallelism.  On the other hand, using many locks complicates synchronization and raises the possibility for deadlock but allows for high parallelism.  Explain where your design falls along this continuum and why you chose to design it this way.
