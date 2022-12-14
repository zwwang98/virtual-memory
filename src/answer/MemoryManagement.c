#include <stdlib.h>
#include "memory.h"
#include "MemoryManagement.h"

extern const int ALL_MEM_SIZE;
extern const int STACK_END_ADDR;
extern const int USER_BASE_ADDR;

PageList* emptyPageList() {
  PageList* pageList = (PageList*) malloc(sizeof(PageList));
  
  // no page assigned to an empty page list yet
  pageList->head = NULL;
  pageList->tail = NULL;

  return pageList;
}

PageList *emptyStackPageList() {
  PageList* pageList = emptyPageList();

  // stack grows from (8*1024*1024-1) to (6*1024*1024), both inclusive
  pageList->startAddr = ALL_MEM_SIZE - 1;
  pageList->endAddr = STACK_END_ADDR;
  return pageList;
}

PageList *emptyHeapPageList() {
  PageList* pageList = emptyPageList();

  // heap grows from (1*1024*1024) to (6*1024*1024-1), both inclusive
  pageList->startAddr = USER_BASE_ADDR;
  pageList->endAddr = STACK_END_ADDR - 1;
  return pageList;
}

void destroyPageList(PageList *pageList) {
  // free each internal pageListNode first
  PageListNode* ptr = pageList->head;
  while (ptr != NULL) {
    PageListNode* next = ptr->next;
    free(ptr);
    ptr = next;
  }

  // finally, free pageList
  free(pageList);
}