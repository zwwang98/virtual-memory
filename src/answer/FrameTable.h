#include "thread.h"

typedef struct FrameEntry {
  Thread* thread;
  int vpn;
  bool isUsed;
  bool dirty;
} FrameEntry;