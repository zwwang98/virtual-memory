/**
 * @brief PageTableEntry represents an entry for virtual page table.
 * 1. When the physicalFrameNumber is -1, there is no mapping physical frame of this virtual page.
 *    Otherwise, there is already a physical frame assigned to this virtual page.
 * 2. When physicalFameNumber is not -1,
 *    if the boolean value inMemory is true, then the corresponding physical frame is in memory,
 *    Otherwise, the corresponding physical frame has been swapped out to disc(files).
 *    In this case, we need to swap it back.
 */
typedef struct PageTableEntry {
  // whether the page is in the memory
  bool present;
  // r/w
  bool accessed;
  // write
  bool dirty;
  // the corresponding physical frame number of this virtual page
  int physicalFrameNumber;
} PageTableEntry;