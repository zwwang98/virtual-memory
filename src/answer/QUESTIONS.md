1. Describe your design of the page table and frame tables. How and why did you store your data the way you did?
   page tables
   1. 1792 length array for 7M user space
   2. index - virtual page number in that thread
   3. value - corresponding physical frame number

   frame tables
   4. 1792 length array for 7M user space
   5. index - physical frame number
   6. value - virtual page number and thread id

2. Describe your algorithm for finding the frame associated with a page.
   1. use the page table to find the PFN
   2. if the PFN is given, find the frame

3. When two user processes both need a new frame at the same time, how are races avoided?
  1. A lock over physical frame table is needed when one thread is scanning the frame table 

4. Why did you choose the data structure(s) that you did for representing virtual-to-physical mappings?
  1. 

5. When a frame is required but none is free, some frame must be evicted. Describe your code for choosing a frame to evict.
  clock algorithm
  accessed, dirty, present

1. When a process P obtains a frame that was previously used by a process Q, how do you adjust the page table (and any other data structures) to reflect the frame Q no longer has?
  1. 

2. A page fault in process P can cause another process Q's frame to be evicted.  How do you ensure that Q cannot access or modify the page during the eviction process?  How do you avoid a race between P evicting Q's frame and Q faulting the page back in?
3. Suppose a page fault in process P causes a page to be read from the file system or swap.  How do you ensure that a second process Q cannot interfere by e.g. attempting to evict the frame while it is still being read in?
4.  A single lock for the whole VM system would make synchronization easy, but limit parallelism.  On the other hand, using many locks complicates synchronization and raises the possibility for deadlock but allows for high parallelism.  Explain where your design falls along this continuum and why you chose to design it this way.
