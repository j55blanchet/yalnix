// InitProcess.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Default initial process.

#include "include/hardware.h"
#include "include/yalnix.h"
#include "KernelGlobals.h"
#include "Utility.h"

int main(void) {
  TracePrintf(TRACE_USERLAND, "InitProcess is running...\n");

  TracePrintf(TRACE_USERLAND, "InitProcess: my pid is %d\n", GetPid());

  int i = 1;
  
  int pid = Fork();
  TracePrintf(TRACE_USERLAND, "InitProcess has forked\n");
  if (pid){
    TracePrintf(TRACE_USERLAND, "InitProcess parent has child %d\n", pid);
    
    while(1){
        TracePrintf(TRACE_USERLAND, "InitProcess PARENT looping %d\n", pid);
        Delay(1);
    }
    
    return 0;
  }
  else{
    while(1){
        TracePrintf(TRACE_USERLAND, "InitProcess CHILD looping %d\n", pid);
        TracePrintf(TRACE_USERLAND, "\tHas pid %d\n", GetPid());
        Pause();
    }
    
    return 0;
  }

  TracePrintf(TRACE_USERLAND, "MYSTERIOUSLY, InitProcess is exiting...\n");
}

// End of InitProcess.c
