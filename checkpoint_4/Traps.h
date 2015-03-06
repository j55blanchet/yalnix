// Traps.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Trap handlers and system call handlers.

#ifndef TRAPS_H
#define TRAPS_H

#include "include/hardware.h"
#include "KernelGlobals.h"
#include "Utility.h"

// -------- -------- -------- -------- -------- -------- -------- --------
// Trap handlers.
void HandleTrapKernel     (UserContext*);
void HandleTrapClock      (UserContext*);
void HandleTrapIllegal    (UserContext*);
void HandleTrapMemory     (UserContext*);
void HandleTrapMath       (UserContext*);
void HandleTrapTtyReceive (UserContext*);
void HandleTrapTtyTransmit(UserContext*);
void HandleTrapUndefined  (UserContext*);

// -------- -------- -------- -------- -------- -------- -------- --------
// System call handlers.

// Handles Fork().
// Semantics is similar to fork() in standard environments.
//
// Returns child's PCB upon success.
// Returns NULL otherwise.
pcb_t* HandleFork(void);

// Handles Exec().
//
// argv** is a NULL-terminated array of NULL-terminated strings.
// By convention argv[0] is equal to filename, but this is not required.
int HandleExec(char* filename, char** argv);

// Delay().
//
// Sleeps for ticks number of TRAP_CLOCK interrupts (ticks).
int HandleDelay(int ticks, UserContext*);

// Brk().
//
// If request for break adjustment is legitimate, adjusts break. Returns SUCCESS.
// Otherwise returns ERROR.
int HandleBrk(void* addr);

// Wait().
int HandleWait(int* status);

#endif
// End of Traps.h
