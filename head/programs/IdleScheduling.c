// IdleScheduling.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Simple test of round robin scheduling
// Runs for a few cycles, then blocks, leaving idle alone


#include "include/hardware.h"
#include "KernelGlobals.h"

int main(void) {

  int pid = GetPid();
  
  TracePrintf(TRACE_USERLAND, "UserProgram about to Pause(): pid=%d...\n", pid);
  Pause();
  TracePrintf(TRACE_USERLAND, "UserProgram about to Pause(): pid=%d...\n", pid);
  Pause();
  TracePrintf(TRACE_USERLAND, "UserProgram about to Delay(1): pid=%d...\n", pid);
  TracePrintf(TRACE_USERLAND, "==> Idle should run now\n");
  
  Delay(1);
  TracePrintf(TRACE_USERLAND, "UserProgram returned from Delay(1). Delaying once more pid:#%d\n", pid);
  Delay(1);
  TracePrintf(TRACE_USERLAND, "UserProgram is finished!\n");
  
  return 0;
}

// End of IdleScheduling.c
