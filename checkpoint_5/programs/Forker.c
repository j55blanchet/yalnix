// Forker.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Calls Fork().

#include "include/hardware.h"
#include "KernelGlobals.h"

#define PARENT_LOOP_MAX 6
#define PARENT_DELAY    0

#define CHILD_LOOP_MAX  5
#define CHILD_DELAY     0

int main(void) {
  TracePrintf(TRACE_USERLAND, "Forker-p is running: pid=%d...\n", GetPid());

  int i;
  int pid = Fork();

  if( pid == ERROR ) {
    TracePrintf(TRACE_USERLAND, "Forker-p: Fork() returns ERROR\n");
    // Exit();
    
  } else if( pid ) {
    // Parent.
    TracePrintf(TRACE_USERLAND, "Forker-p(#%d): I gave birth to Forker-c: pid=%d.\n", GetPid(), pid);
    if( PARENT_DELAY ) {
      TracePrintf(TRACE_USERLAND, "Forker-p(#%d): I will now delay for %d ticks.\n", GetPid(),  PARENT_DELAY);
      Delay(PARENT_DELAY);
    }
    for(i = 1; i <= PARENT_LOOP_MAX; i++) {
      TracePrintf(TRACE_USERLAND, "Forker-p(#%d): in loop %d / %d\n", GetPid(), i, PARENT_LOOP_MAX);
      Pause();
    }
    TracePrintf(TRACE_USERLAND, "Forker-p(#%d) is exiting...\n", GetPid());
    // Exit();
    
  } else {
    // Child.
    TracePrintf(TRACE_USERLAND, "Forker-c: I am now born: pid=%d.\n", GetPid());
    if( CHILD_DELAY ) {
      TracePrintf(TRACE_USERLAND, "Forker-c: I will now delay for %d ticks.\n", CHILD_DELAY);
      Delay(CHILD_DELAY);
    }
    for(i = 1; i <= CHILD_LOOP_MAX; i++) {
      TracePrintf(TRACE_USERLAND, "Forker-c: in loop %d / %d\n", i, CHILD_LOOP_MAX);
      Pause();
    }
    TracePrintf(TRACE_USERLAND, "Forker-c is exiting...\n");
    // Exit();
  }

}

// End of Forker.c
