// Traps.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Declare trap handlers.

#ifndef TRAPS_H
#define TRAPS_H

#include "include/hardware.h"
#include "KernelGlobals.h"
#include "Utility.h"

// system defined traps
void HandleTrapKernel     (UserContext*);
void HandleTrapClock      (UserContext*);
void HandleTrapIllegal    (UserContext*);
void HandleTrapMemory     (UserContext*);
void HandleTrapMath       (UserContext*);
void HandleTrapTtyReceive (UserContext*);
void HandleTrapTtyTransmit(UserContext*);
void HandleTrapUndefined  (UserContext*);

// subtraps of KernelTrap (i.e, SysCalls)
int HandleDelay(int ticks);
int HandleBrk(void* requested_addr);

#endif
// End of Traps.h
