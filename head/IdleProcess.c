// IdleProcess.c

#include "include/hardware.h"
#include "KernelGlobals.h"

// JHL.
// TracePrintf() doesn't print anything.
// Can userland processes only speak to TTY?
//
// Charles says TracePrintf() should work...

int main(void) {
  TracePrintf(TRACE_USERLAND, "IdleProcess is running...\n");

  int i = 1;
  while(i) {
    TracePrintf(TRACE_USERLAND, "IdleProcess in loop %d.\n", i++);
    Pause();
  }
}

// End of IdleProcess.c
