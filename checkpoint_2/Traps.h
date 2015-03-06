// Traps.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Declare trap handlers.

#include "include/hardware.h"
#include "KernelGlobals.h"
#include "Utility.h"

void HandleTrapKernel     (UserContext*);
void HandleTrapClock      (UserContext*);
void HandleTrapIllegal    (UserContext*);
void HandleTrapMemory     (UserContext*);
void HandleTrapMath       (UserContext*);
void HandleTrapTtyReceive (UserContext*);
void HandleTrapTtyTransmit(UserContext*);
void HandleTrapUndefined  (UserContext*);

// End of Traps.h
