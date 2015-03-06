// Traps.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Implement trap handlers.
// See Traps.h for information.
//
// JHL. We might break up these into separate files later on.

#include "Traps.h"

// Helper method for TRAP_KERNEL:YALNIX_EXIT.
int ParentIsWAITING(pcb_t* parent, pcb_t* current) {
  if( current == NULL ) {
    return 0;
  }
  if( current == parent ) {
    return 1;
  }
  return ParentIsWAITING(parent, current->next);
}
void WakeUpWaitingParent(int ppid) {
  if( ppid < 0 ) {
    TracePrintf(TRACE_WRONG, "A process with ppid=%d exited.\n", ppid);
    Halt();
  }
  pcb_t* parent = pcb_array[ppid];
  if( ParentIsWAITING(parent, WAITING.head) ) {
    RemoveFromQueue(parent, &WAITING);
    AddToQueue(parent, &READY);
  }
}

void HandleTrapKernel     (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_KERNEL(0x%x)\n", u_context->code & 0xf);
  //TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapKernel is partially complete.\n");

  //TraceUserContext(TRACE_TRAP, u_context);

  switch( u_context->code ) {
  case YALNIX_GETPID:
    u_context->regs[0] = RUNNING.head->pid;
    return;
    
  case YALNIX_DELAY:
    {
      TraceUserContext(TRACE_TRAP, u_context);
      int clock_ticks = u_context->regs[0];
      int result = HandleDelay(clock_ticks, u_context);
      u_context->regs[0] = result;
    }
    return;
    
  case YALNIX_BRK:
    {
      void* requested_addr = (void*) u_context->regs[0];
      int result = HandleBrk(requested_addr);
      u_context->regs[0] = result;
    }
    return;
    
  case YALNIX_FORK:
    {
      pcb_t* child = HandleFork();
      if( child == NULL ) {
	u_context->regs[0] = ERROR;
	return;
      }
      if( child == RUNNING.head ) {
	// Child.
	u_context->regs[0] = 0;
      } else {
	// Parent.
	u_context->regs[0] = child->pid;
      }
    }
    return;
    
  case YALNIX_EXEC:
    {
      char* progName = (char*) u_context->regs[0];
      
      // check input string
      if (!CheckUserString(progName, PROT_READ, MAX_PROGRAM_NAME_LENGTH)){
        TracePrintf(TRACE_USER_ERROR, "UserProgram passed invalid pointer to Exec program name\n");
        goto kill_process;
      }
      
      TracePrintf(TRACE_VERBOSE, "HandleTrapKernel(): Exec() called with '%s'\n", progName);
      
      char** ptr = (char**)u_context->regs[1];
      if (ptr != NULL){
          if (!CheckUserPointer(ptr, PROT_READ)){
            TracePrintf(TRACE_USER_ERROR, "UserProgram passed invalid argument array pointer %p to Exec\n", ptr);
            goto returnError;
          }
          for(; *ptr != NULL; ptr++) {
            here();
            // check argument strings
            TracePrintf(TRACE_VERBOSE, "HandleTrapKernel(): Exec checking argument pointer %p pointing to %p\n", ptr, *ptr);
            if (!CheckUserString(*ptr, PROT_READ, MAX_PROGRAM_NAME_LENGTH)){
                TracePrintf(TRACE_USER_ERROR, "UserProgram passed invalid pointer as Exec argument\n");
                goto kill_process;
            }
	        TracePrintf(TRACE_VERBOSE, "  - Arg '%s'\n", *ptr);
          }
      }
      here();
      int rv = HandleExec((char*) u_context->regs[0], (char**) u_context->regs[1]);
      if( rv == SUCCESS ) {
	    memcpy(u_context, &RUNNING.head->u_context, sizeof(UserContext));
	    ChangeAddressSpace(RUNNING.head);
	    return;
      } else if( rv == ERROR ) {
        returnError:
	    u_context->regs[0] = ERROR;
	    return;
      } else /* rv == KILL */ {
	    TracePrintf(TRACE_CRITICAL, "HandleTrapKernel(): LoadProgram() reurns KILL.\n");
	    // KILL implies that we must abort the process.
	    // Continue to case YALNIX_EXIT.
	    // Let rv = KILL.
	    u_context->regs[0] = KILL;
      }
    }
    // break; // Continue to case YALNIX_EXIT.

  case YALNIX_EXIT:
    kill_process:
    if( RUNNING.head->pid == 1 ) {
      TracePrintf(TRACE_CRITICAL, "Initial process exits. OS halts.\n");
      TracePrintf(TRACE_CRITICAL, "Return value: %d\n", (int) u_context->regs[0]);
      Halt();
    } else {
      TracePrintf(TRACE_VERBOSE, "===============================================\n");
      TracePrintf(TRACE_VERBOSE, "HandleTrapKernel(): Exit(%d) called for process #%d.\n",
		  (int) u_context->regs[0], RUNNING.head->pid);
      pcb_t* proc = RUNNING.head;

      // Is my parent waiting for me?
      WakeUpWaitingParent(proc->ppid);

      RemoveFromQueue(proc, &RUNNING);
      KillPCB(proc, (int) u_context->regs[0]);

      // Now run another program.
      if( READY.head == NULL ) { // This should never happen.
	    TracePrintf(TRACE_WRONG, "No process available to run.\n");
	    Halt();
      }
      ContextSwitch(READY.head, &READY, NULL);
    }
    return;

  case YALNIX_WAIT:
    u_context->regs[0] = HandleWait((int*) u_context->regs[0]);
    return;
  case YALNIX_TTY_READ:
  case YALNIX_TTY_WRITE:
  case YALNIX_SEM_INIT:
  case YALNIX_SEM_UP:
    break;
  case YALNIX_SEM_DOWN:
    // I got tired of typing these. There are others that we will implement that we havn't yet
    TracePrintf(TRACE_UNIMPLEMENTED_WARNING, "Recieved unimplemented Trap Kernel\n");
    TraceUserContext(TRACE_TRAP, u_context);
    break;
  case YALNIX_NOP:
    TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapKernel(): NOP.\n");
    TraceUserContext(TRACE_UNIMPLEMENTED_CRITICAL, u_context);
    Halt();
    break;
  default:
    TracePrintf(TRACE_UNIMPLEMENTED_WARNING, "Received unimplemented or invalid Trap Kernel\n");
    TraceUserContext(TRACE_TRAP, u_context);
    break;
  }
}

// Helper method for HandleTrapClock().
void RemoveFinishedDelays(pcb_t* head, pcb_t* current) {
  if( head == current ) { // Reached end of list.
    return;
  }
  if( ticks >= current->wakeup_time ) {
    pcb_t* next = current->next;

    RemoveFromQueue(current, &SLEEPING);
    AddToQueue(current, &READY);
    TracePrintf(TRACE_VERBOSE, "HandleTrapClock(): waking up process #%d.\n", current->pid);

    RemoveFinishedDelays(head, next);
  } else {
    RemoveFinishedDelays(head, current->next);
  }
}

void HandleTrapClock(UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "\n\nTRAP_CLOCK(%d)\n", ticks);
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapClock() is partially complete.\n");
  // TraceUserContext(TRACE_TRAP, u_context);

  pcb_t* head = SLEEPING.head;
  if( head != NULL ) {
    RemoveFinishedDelays(head, head->next);
    if( ticks >= head->wakeup_time ) {
      RemoveFromQueue(head, &SLEEPING);
      AddToQueue(head, &READY);
      TracePrintf(TRACE_VERBOSE, "HandleTrapClock(): waking up process #%d.\n", head->pid);
    }
  }

  ticks++;

  // Round-robin scheduling.
  if( READY.head != NULL ) {
    // Freeze the current user context and save it in the current process's memory.
    memcpy(&RUNNING.head->u_context, u_context, sizeof(UserContext));

    ContextSwitch(READY.head, &READY, &READY);

    // Melt the frozen user context from the next process's memory and copy it onto
    // the user context which hardware will look at.
    memcpy(u_context, &RUNNING.head->u_context, sizeof(UserContext));
  }
}


void HandleTrapIllegal    (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_ILLEGAL\n");
  TraceUserContext(TRACE_CRITICAL, u_context);
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
  TracePrintf(TRACE_TRAP, "Undefined trap.\n");
  TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapUndefined is not implemented.\n");

  Halt();
}

// End of Traps.c
