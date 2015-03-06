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
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapKernel is incomplete.\n");

  //TraceUserContext(TRACE_TRAP, u_context);

  switch( u_context->code ) {
    case YALNIX_GETPID: // JHL. Temporary. But this essentially works.
      u_context->regs[0] = RUNNING.head->pid;
      break;

    case YALNIX_DELAY:{
      TraceUserContext(TRACE_TRAP, u_context);
      int clock_ticks =  u_context->regs[0]; // need to extract this from regs
      int result = HandleDelay(clock_ticks);
      u_context->regs[0] = result;
      break;
    }

    case YALNIX_BRK:{
      void* requested_addr = (void*) u_context->regs[0]; // need to extract this from regs
      int result = HandleBrk(requested_addr);
      u_context->regs[0] = result;
      break;
    }
    
    case YALNIX_FORK:
    case YALNIX_EXEC:
    case YALNIX_EXIT:
    case YALNIX_WAIT:
    case YALNIX_TTY_READ:
    case YALNIX_TTY_WRITE:
    case YALNIX_SEM_INIT:
    case YALNIX_SEM_UP:
    case YALNIX_SEM_DOWN:
      // I got tired of typing these. There are others that we will implement that we havn't yet
      TracePrintf(TRACE_UNIMPLEMENTED_WARNING, "Recieved unimplemented Trap Kernel");
      TraceUserContext(TRACE_TRAP, u_context);
      break;

    default:
      TracePrintf(TRACE_UNIMPLEMENTED_WARNING, "Recieved unimplemented or invalid Trap Kernel");
      TraceUserContext(TRACE_TRAP, u_context);
      break;
  }
}


void HandleTrapClock(UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_CLOCK(%d)\n", ticks);
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapClock() is somewhat more complete.\n");

  // decrement ticks-remaining of all waiting processes
  pcb_t* cur_pcb = CLOCK_WAITING.head;
  if (cur_pcb){
    do {
      cur_pcb->ticks--;
      if (cur_pcb->ticks == 0){
	TracePrintf(10, "should remove process from CLOCK_WAITING right now!\n");	
	pcb_t* next = cur_pcb->next;
        RemoveFromQueue(cur_pcb, &CLOCK_WAITING);
        AddToQueue(cur_pcb, &READY);
	if (next != cur_pcb){
	  cur_pcb = next;
	  continue;
	}
      }  
      cur_pcb = cur_pcb->next;
    } while(CLOCK_WAITING.head != NULL && cur_pcb != CLOCK_WAITING.head);
  }

  ticks++;

  // Round-robin scheduling.
  if( READY.head != NULL ) {
    // Freeze the current user context and save it in the current process's memory.
    memcpy(&RUNNING.head->u_context, u_context, sizeof(UserContext));

    // JHL. This doesn't work yet.
    ContextSwitch(READY.head, &READY, &READY);

    // Melt the frozen user context from the next process's memory and copy it onto
    // the user context which hardware will look at.
    memcpy(u_context, &RUNNING.head->u_context, sizeof(UserContext));
  }
}


void HandleTrapIllegal    (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_ILLEGAL\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapIllegal is not implemented.\n");

  Halt();
}

void HandleTrapMemory     (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_MEMORY\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapMemory() is incomplete.\n");
  TraceUserContext(TRACE_TRAP, u_context);

  switch( u_context->code ) {
  case YALNIX_MAPERR: // Address not mapped.    
    break;
  case YALNIX_ACCERR: // Invalid permissions.
    break;
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
