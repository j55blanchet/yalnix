// Execker.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Calls Exec().

#include "include/hardware.h"
#include "KernelGlobals.h"

#define DELAY 5

int main(void) {
  TracePrintf(TRACE_USERLAND, "Execker-p is running: pid=%d...\n", GetPid());
  int rv;
  char* name = "programs/Forker";
  char* argv[2];
  argv[0] = name;
  argv[1] = NULL;

  //  TracePrintf(TRACE_USERLAND, "Execker-c: I will exec() into '%s'.\n", name);
  //  Exec(name, argv);

  if( Fork() == 0 ) {

    TracePrintf(TRACE_USERLAND, "Execker-c: I will now sleep for %d ticks.\n", DELAY);
    Delay(DELAY);
    TracePrintf(TRACE_USERLAND, "Execker-c: I am now awake.\n");
  } else {

    TracePrintf(TRACE_USERLAND, "Execker-p: I will exec() into '%s'.\n", name);    
    Exec(name, argv);
  }

  while(1) {
    TracePrintf(TRACE_USERLAND, "Execker-p/c: in infinite loop.\n");
    Pause();
  }
}

// End of Execker.c
