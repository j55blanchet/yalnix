// Traps.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Implement trap handlers.
// See Traps.h for information.
//
// JHL. We might break up these into separate files later on.

#include "Traps.h"

void HandleTrapKernel     (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_KERNEL\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapKernel is not implemented.\n");

  Halt();
}
void HandleTrapClock      (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_CLOCK\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapClock() is incomplete.\n");

  ticks++;
}
void HandleTrapIllegal    (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_ILLEGAL\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapIllegal is not implemented.\n");

  Halt();
}

void HandleTrapMemory     (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_MEMORY\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapMemory() is incomplete.\n");
  PrintUserContext(u_context);

  switch( u_context->code ) {
  case YALNIX_MAPERR: // Address not mapped.
    
    break;
  case YALNIX_ACCERR: // Invalid permissions.
    break;
  case 11:
    // JHL. I don't recall 11 defined as a code for TRAP_MEMORY.
    // But sometimes this occurs.
    TracePrintf(TRACE_COMMENT, "HandleTrapMemory(): code=11\n");
  default:
    TracePrintf(TRACE_WRONG, "HandleTrapMemory(): invalid code\n");
    break;
  }

  // JHL. Temporary measure.
  Halt();
}

void HandleTrapMath       (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_MATH\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapMath is not implemented.\n");
  Halt();
}
void HandleTrapTtyReceive (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_TTY_RECEIVE\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapTtyReceive is not unimplement.\n");
  
  Halt();
}
void HandleTrapTtyTransmit(UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_TTY_TRANSMIT\n");  
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapTtyWrite is not implemented.\n");
  Halt();
}
void HandleTrapUndefined  (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_CLOCK\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapUndefinedl is not implemented.\n");
  Halt();
}

// End of Traps.c
