#include "callback.h"

extern void initPFNTable();

void startupCallback() {
  initPFNTable();
}
void shutdownCallback() {
  
}