/**
 * PageListNode struct represents assignment of one page for one thread(process)'s heap or stack.
 */
typedef struct PageListNode {
  int startAddr;
  int endAddr;
  struct PageListNode *next;
} PageListNode;

/**
 * PageList struct represents all pages assignments for one thread(process)'s heap or stack.
 */
typedef struct PageList {
  int startAddr;
  int endAddr;
  PageListNode *head;
  PageListNode *tail;
} PageList;

/**
 * @brief Iniate a memory management page list.
 * 
 * @return PageList* 
 */
PageList *emptyPageList();

/**
 * @brief Iniate a memory management page list for stack.
 * 
 * @return PageList* 
 */
PageList *emptyStackPageList();

/**
 * @brief Iniate a memory management page list for heap.
 * 
 * @return PageList* 
 */
PageList *emptyHeapPageList();

/**
 * @brief Given page list, free all memory.
 */
void destroyPageList(PageList *pageList);