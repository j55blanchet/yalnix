// Waiter.c
//
// Julien Blanchet and Jae Heon Lee.

#include "include/hardware.h"
#include "KernelGlobals.h"

#define NUM_CHILDREN 5

int main(void) {
  TracePrintf(TRACE_USERLAND, "Waiter-p: I am running...\n");

  int i;
  int status;
  int pid;
  int rv;

  rv = Wait(&status);
  TracePrintf(TRACE_USERLAND, "Waiter-p: with no children, Wait() returns %d.\n", rv);

  TracePrintf(TRACE_USERLAND, "Waiter-p: I will now have %d children.\n", NUM_CHILDREN);
  for(i = 0; i < NUM_CHILDREN; i++) {
    if( Fork() == 0 ) {
      pid = GetPid();
      TracePrintf(TRACE_USERLAND, "Waiter-c-%d: I will sleep for %d ticks.\n", pid, i);
      Delay(i);
      TracePrintf(TRACE_USERLAND, "Waiter-c-%d: I am now awake.\n", pid);
      Exit(i);
    }
  }
  for(i = 0; i < NUM_CHILDREN; i++) {
    Wait(&status);
    TracePrintf(TRACE_USERLAND, "Waiter-p: my child's last word was '%d.'\n", status);
  }

  TracePrintf(TRACE_USERLAND, "Waiter-p: I will now demonstrate the absence of cascading termination.\n");
  if( Fork() == 0 ) {
    if( Fork() == 0 ) {
      TracePrintf(TRACE_USERLAND, "Waiter-c-c: I will sleep for %d ticks.\n", NUM_CHILDREN);
      Delay(i);
      TracePrintf(TRACE_USERLAND, "Waiter-c-c: I am now awake, but no one is waiting on me.\n");
      Exit(NUM_CHILDREN);
    }
    TracePrintf(TRACE_USERLAND, "Waiter-c-p: I will exit immediately.\n");
    Exit(2*NUM_CHILDREN);
  }

  Wait(&status);
  TracePrintf(TRACE_USERLAND, "Waiter-p: my child's last word was '%d.'\n", status);

  rv = Wait(&status);
  TracePrintf(TRACE_USERLAND, "Waiter-p: with no children, Wait() returns %d.\n", rv);

  TracePrintf(TRACE_USERLAND, "Waiter-p: I will now exit.\n");
  Exit(3*NUM_CHILDREN);
}

// End of Waiter.c
