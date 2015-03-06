// Fun.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Simple extra program used to test exec

#include "include/hardware.h"
#include "KernelGlobals.h"

int main(void) {
  int pid = GetPid();
  TracePrintf(TRACE_USERLAND, "Fun is running: pid=%d...\n", pid);
    
  int j = 0;
  int cPid;
  if ((cPid=Fork()) == 0){
    TracePrintf(TRACE_USERLAND, "I am Fun's Baby! pid=%d\n", GetPid());
    TracePrintf(TRACE_USERLAND, "Fun Baby Delaying\n");
    Delay(2);
    TracePrintf(TRACE_USERLAND, "Fun Baby is Exiting\n");
    Exit(4);
  }
  while(1){
    TracePrintf(TRACE_USERLAND, "Fun Parent is waiting for child is running: pid=%d..childPid:%d.\n", pid, cPid);
    Wait(&main); // this should really throw the kernel for a loop! ;)
    TracePrintf(TRACE_USERLAND, "Fun Parent finish waiting for child! Now finished...\n");
    Exit(0);
  }
}

// End of Execker.c
