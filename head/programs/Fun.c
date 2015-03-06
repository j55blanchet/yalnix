// Fun.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Simple extra program used as targets for other programs

#include "include/hardware.h"
#include "KernelGlobals.h"

int main(void) {

  int pid = GetPid();
  
  while(1){
    TracePrintf(TRACE_USERLAND, "Fun is running: pid=%d...\n", pid);
    Delay(1);
  }
  
}

// End of Fun.c
