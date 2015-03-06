// InitProcess.c

#include "include/hardware.h"
#include "KernelGlobals.h"
#include "Utility.h"

int main(void) {
  TracePrintf(TRACE_USERLAND, "InitProcess is running...\n");

  TracePrintf(TRACE_USERLAND, "InitProcess: my pid is %d\n", GetPid());

  int i = 1;
  while(i) {
    TracePrintf(TRACE_USERLAND, "InitProcess in loop %d.\n", i++);
    Pause();
    TracePrintf(TRACE_USERLAND, "InitProcess will Delay() for %d seconds.\n", 3);
    Delay(3);
    TracePrintf(TRACE_USERLAND, "InitProcess has woken up from Delay().\n"
		TRACE_N "InitProcess will now malloc() 0x2000 bytes");
    char* ptr = (char*)malloc(0x2000);
    TracePrintf(TRACE_USERLAND, "InitProcess: mallocked bytes begin at %p\n", ptr);
  }

  TracePrintf(TRACE_USERLAND, "InitProcess is exiting...\n");
}

// End of InitProcess.c
