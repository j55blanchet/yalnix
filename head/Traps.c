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
  if( current == WAITING.head ) {
    return 0;
  }
  if( current == parent ) {
    return 1;
  }

  return ParentIsWAITING(parent, current->next);
}
void WakeUpWaitingParent(int ppid) {
  if( ppid <= 0 ) { // This shouldn't be possible, since init process exiting is caught in HandleTrapKernel().
    TracePrintf(TRACE_WRONG, "A process with ppid=%d exited.\n", ppid);
    Halt();
  }
  pcb_t* parent = pcb_array[ppid];

  if( parent != NULL && WAITING.head != NULL ) {
    if( parent == WAITING.head || ParentIsWAITING(parent, WAITING.head->next) ) {
      RemoveFromQueue(parent, &WAITING);
      AddToQueue(parent, &READY);
    }
  }
}

void HandleTrapKernel     (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_KERNEL(0x%x)\n", u_context->code & 0xff);
  //TraceUserContext(TRACE_TRAP, u_context);

  switch( u_context->code ) {
  case YALNIX_GETPID:
    u_context->regs[0] = RUNNING.head->pid;
    return;
    
  case YALNIX_DELAY:
    {
      int clock_ticks = u_context->regs[0];
      int result = HandleDelay(clock_ticks, u_context);
      u_context->regs[0] = result;
    }
    return;
    
  case YALNIX_BRK:
    //      TraceUserContext(TRACE_TRAP, u_context);
    u_context->regs[0] = HandleBrk((void*)u_context->regs[0]);
    return;
    
  case YALNIX_FORK:
    {
      pcb_t* child = HandleFork();
      if( child == NULL ) {
        u_context->regs[0] = ERROR;
	TracePrintf(TRACE_SEVERE, "HandleTrapKernel(): Fork() fails.\n");
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
        KillRunningProcess();
        return;
      }
      
      TracePrintf(TRACE_VERBOSE, "HandleTrapKernel(): Exec() called with '%s'\n", progName);
      
      char** ptr = (char**)u_context->regs[1];
      if (ptr != NULL){
        if (!CheckUserPointer(ptr, PROT_READ)){
          TracePrintf(TRACE_USER_ERROR, "UserProgram passed invalid argument array pointer %p to Exec\n", ptr);
          goto returnError;
        }
        for(; *ptr != NULL; ptr++) {
            // check argument strings
            TracePrintf(TRACE_VERBOSE, "HandleTrapKernel(): Exec checking argument pointer %p pointing to %p\n", ptr, *ptr);
            if (!CheckUserString(*ptr, PROT_READ, MAX_PROGRAM_NAME_LENGTH)){
                TracePrintf(TRACE_USER_ERROR, "UserProgram passed invalid pointer as Exec argument\n");
                KillRunningProcess();
                return;
            }
            TracePrintf(TRACE_VERBOSE, "  - Arg '%s'\n", *ptr);
        }
      }
      int rv = HandleExec((char*) u_context->regs[0], (char**) u_context->regs[1]);
      if( rv == SUCCESS ) {
        memcpy(u_context, &RUNNING.head->u_context, sizeof(UserContext));
        ChangeAddressSpace(RUNNING.head);
        return;
      } 
      else if( rv == ERROR ) {
        returnError:
        u_context->regs[0] = ERROR;
        return;
      } 
      else /* rv == KILL */ {
        TracePrintf(TRACE_CRITICAL, "HandleTrapKernel(): LoadProgram() reurns KILL.\n");
        KillRunningProcess();
        return;
      }
    }

  case YALNIX_EXIT:
    // TracePrintf(TRACE_VERBOSE, "===============================================\n");
    TracePrintf(TRACE_VERBOSE, "HandleTrapKernel(): Exit(%d) called for process #%d.\n",
	    (int) u_context->regs[0], RUNNING.head->pid);
    KillProcess(RUNNING.head, &RUNNING, u_context->regs[0]);
    return;

  case YALNIX_WAIT:
    u_context->regs[0] = HandleWait((int*) u_context->regs[0]);
    return;
  case YALNIX_TTY_READ:
    {
      int   tty_id = (int)   u_context->regs[0];
      void* buffer = (void*) u_context->regs[1];
      int   length = (int)   u_context->regs[2];
      u_context->regs[0] = HandleTtyRead(tty_id, buffer, length);
    }
    return;
  case YALNIX_TTY_WRITE:
    //    TraceUserContext(TRACE_VERBOSE, u_context);
    {
      int   tty_id = (int)   u_context->regs[0];
      void* buffer = (void*) u_context->regs[1];
      int   length = (int)   u_context->regs[2];
      here();
      if( HandleTtyWrite(tty_id, buffer, length) == ERROR ) {
        u_context->regs[0] = ERROR;
      } else {
        u_context->regs[0] = length;
      }
      here();
      PrintAllQueues();
      here();
    }
    return;
  case YALNIX_SEM_INIT:
    TracePrintf(TRACE_WRONG, "HandleTrapKernel(): Yalnix does not support semaphores.\n");
    TraceUserContext(TRACE_TRAP, u_context);
    Halt();
    break;
  case YALNIX_SEM_UP:
    TracePrintf(TRACE_WRONG, "HandleTrapKernel(): Yalnix does not support semaphores.\n");
    TraceUserContext(TRACE_TRAP, u_context);
    Halt();
    break;
  case YALNIX_SEM_DOWN:
    TracePrintf(TRACE_WRONG, "HandleTrapKernel(): Yalnix does not support semaphores.\n");
    TraceUserContext(TRACE_TRAP, u_context);
    Halt();
    break;
  case YALNIX_LOCK_INIT:
    u_context->regs[0] = HandleLockInit((int*) u_context->regs[0]);
    return;
  case YALNIX_LOCK_ACQUIRE:
    u_context->regs[0] = HandleAcquire((int) u_context->regs[0]);
    return;
  case YALNIX_LOCK_RELEASE:
    u_context->regs[0] = HandleRelease((int) u_context->regs[0]);
    return;
  case YALNIX_CVAR_INIT:
    u_context->regs[0] = HandleCvarInit((int*) u_context->regs[0]);
    return;
  case YALNIX_CVAR_WAIT:
    u_context->regs[0] = HandleCvarWait((int) u_context->regs[0], (int) u_context->regs[1]);
    return;
  case YALNIX_CVAR_SIGNAL:
    u_context->regs[0] = HandleCvarSignal((int) u_context->regs[0]);
    return;
  case YALNIX_CVAR_BROADCAST:
    u_context->regs[0] = HandleCvarBroadcast((int) u_context->regs[0]);
    return;
  case YALNIX_PIPE_INIT:
    u_context->regs[0] = HandlePipeInit((int*) u_context->regs[0]);
    return;
  case YALNIX_PIPE_READ:
    u_context->regs[0] = HandlePipeRead((int)   u_context->regs[0] /* id */    ,
					(void*) u_context->regs[1] /* buffer */,
					(int)   u_context->regs[2] /* length */);
    return;
  case YALNIX_PIPE_WRITE:
    u_context->regs[0] = HandlePipeWrite((int)   u_context->regs[0] /* id */    ,
					 (void*) u_context->regs[1] /* buffer */,
					 (int)   u_context->regs[2] /* length */);
    return;
  case YALNIX_RECLAIM:
    u_context->regs[0] = HandleReclaim((int) u_context->regs[0]);
    return;
  case YALNIX_NOP:
    TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "HandleTrapKernel(): NOP.\n");
    TraceUserContext(TRACE_UNIMPLEMENTED_CRITICAL, u_context);
    Halt();
    break;
  default:
    TracePrintf(TRACE_UNIMPLEMENTED_WARNING, "HandleTrapKernel(): unimplemented or invalid TRAP_KERNEL.\n");
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

  // JHL. Addition to improve scheduling by ignoring idle process until inevitable.
  //
  // If idle process is about to run, make sure that no one else is waiting.
  // (1) Put idle to the end of the queue.

  
  if( READY.head != NULL && READY.head->pid == 2 ) {
    pcb_t* idle = READY.head;
    RemoveFromQueue(idle, &READY);
    AddToQueue(idle, &READY);
  }
  // (2) If idle is still the first, then don't call ContextSwitch().
  if( READY.head != NULL && READY.head->pid == 2 ) {
    TracePrintf(TRACE_VERBOSE, "HandleTrapClock(): I will skip a turn for idle.\n");
    return;
  }
  // JHL. End.
  

  // At this point, we are committed to calling ContextSwitc().
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
  KillRunningProcess();
}

void HandleTrapMemory(UserContext* u_context) {
  int pte_index = r1_addr_to_id(u_context->addr);
  {
    TracePrintf(TRACE_VERBOSE, "TRAP_MEMORY:\n"
		TRACE_N "  Code      : %d (%d=MAPERR, %d=ACCERR)\n"
		TRACE_N "  Address   : index %d\t(%p) (valid? %d)\n"
		TRACE_N "  Stack Base: index %d\t(%p)\n"
		TRACE_N "  Heap Break: index %d\t(%p)\n"
		, u_context->code, YALNIX_MAPERR, YALNIX_ACCERR
		, r1_addr_to_id(u_context->addr), u_context->addr
		, RUNNING.head->r1_page_table[r1_addr_to_id(u_context->addr)].valid
		, RUNNING.head->r1_stack_base_index, r1_id_to_addr(RUNNING.head->r1_stack_base_index)
		, RUNNING.head->r1_break_limit_index, r1_id_to_addr(RUNNING.head->r1_break_limit_index)
		);
  }

  /*
  TracePrintf(TRACE_TRAP, 
              "TRAP_MEMORY(type %d at %p) cur-stack:%d, req-stack:%d, heap:%d\n",          
              u_context->code,  
              u_context->addr, 
              RUNNING.head->r1_stack_base_index, 
              pte_index, 
              RUNNING.head->r1_break_limit_index);
  */  

  switch( u_context->code ) {
  case YALNIX_MAPERR: // Address not mapped.
    if( u_context->addr == NULL ) {
      TracePrintf(TRACE_SEVERE, "HandleTrapMemory(): process #%d commits NULL pointer exception.\n",
		  RUNNING.head->pid);
      KillRunningProcess();
      return;
    }
    
    // sanity check: should be below the current stack base (otherwise, why is there a memtrap?)
    if (pte_index >= RUNNING.head->r1_stack_base_index){
      TracePrintf(TRACE_WRONG, "HandleTrapMemory(): process #%d has TRAP-MEMORY of pointer  "
		  "%p of pte index %d that is within stack limit of %d."
		  " (heap limit index %d)\n",
		  RUNNING.head->pid, 
		  u_context->addr, 
		  pte_index, 
		  RUNNING.head->r1_stack_base_index, 
		  RUNNING.head->r1_break_limit_index);
      Halt();
    }
    
    // detect if trying to grow stack into heap
    {
      int i = RUNNING.head->r1_stack_base_index - 1;
      
      // if any pte's in the range from current stack location
      // are valid, we are running into the heap
      for(;i >= pte_index - 1 /* leave 1 unmapped page */;i--){
	if (RUNNING.head->r1_page_table[i].valid){
	  TracePrintf(TRACE_SEVERE, "HandleTrapMemory(): process #%d attempting"
		      " to grow stack into heap. cur-stack:%d, "
		      "req-stack:%d, heap:%d\n", 
		      RUNNING.head->pid,
		      RUNNING.head->r1_stack_base_index, 
		      pte_index, 
		      RUNNING.head->r1_break_limit_index);
	  KillRunningProcess();
	  return;
	}
      }
    }
    
    // at this point, we can be sure that this request is an implict request 
    // to grow the stack into the heap
    {
      int i = RUNNING.head->r1_stack_base_index - 1;
      for(;i >= pte_index; i--){
	TracePrintf(TRACE_VERBOSE, "HandleTrapMemory(): mapping page %d to stack\n", i);
        
	// if we can find a free frame, give it to stack
	int frame_i = FindFreeFrame(PROT_READ | PROT_WRITE);
	if (frame_i != ERROR && frame_i > 1){
	  RUNNING.head->r1_page_table[i].valid = 1;
	  RUNNING.head->r1_page_table[i].prot = PROT_READ | PROT_WRITE;
	  RUNNING.head->r1_page_table[i].pfn = frame_i;
	}
	// no frames available: kill process
	else{
	  TracePrintf(TRACE_SEVERE, "HandleTrapMemory(): no memory available to grow stack of process #%d\n", RUNNING.head->pid);
	  KillRunningProcess();
	  return;
	}
      }
      RUNNING.head->r1_stack_base_index = pte_index;
    }
    TracePrintf(TRACE_VERBOSE, "HandleTrapMemory(): grew stack for process #%d.\n", RUNNING.head->pid);
    return;
    
  case YALNIX_ACCERR: // Invalid permissions.
    TracePrintf(TRACE_SEVERE, "HandleTrapMemory(): fatal memory access error for process #%d.\n",
		RUNNING.head->pid);
    KillRunningProcess();
    return;
  default:
    TracePrintf(TRACE_WRONG, "HandleTrapMemory(): invalid code(0x%x)\n", u_context->code);
    Halt();
  }
}

void HandleTrapMath       (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_MATH\n");
  TraceUserContext(TRACE_CRITICAL, u_context);
  KillRunningProcess();
}

void HandleTrapTtyReceive (UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_TTY_RECEIVE\n");

  int tty_id = u_context->code;

  if( READING[tty_id].head != NULL ) {
    ContextSwitch(READING[tty_id].head, &READING[tty_id], &READY);
  } else {
    TracePrintf(TRACE_COMMENT, "HandleTrapTtyReceive(): no process was reading tty #%d.\n", tty_id);
  }
}

// WRITING[tty_id].head is always the process which has been writing to the tty[tty_id].
void HandleTrapTtyTransmit(UserContext* u_context) {
  TracePrintf(TRACE_TRAP, "TRAP_TTY_TRANSMIT\n");  

  // TraceUserContext(TRACE_TRAP, u_context);

  int tty_id = u_context->code;

  // Clear buffer (not necessary).
  // memset(tty_write_buffer[tty_id], '\0', TERMINAL_MAX_LINE);

  //  here();
  //  PrintAllQueues();
  ContextSwitch(WRITING[tty_id].head, &WRITING[tty_id], &READY);
  //  here();
  //  PrintAllQueues();

  // Make sure that, upon completing TtyTransmit() for one proess,
  // another process writing to the same tty gets a chance to run.
  if( WRITING_WAIT[tty_id].head != NULL ) {
    TracePrintf(TRACE_VERBOSE, "HandleTtyTransmit(): process #%d was waiting to write to tty #%d.\n",
		WRITING_WAIT[tty_id].head->pid, tty_id);
    ContextSwitch(WRITING_WAIT[tty_id].head, &WRITING_WAIT[tty_id], &READY);
  }
}

void HandleTrapUndefined  (UserContext* u_context) {
  TracePrintf(TRACE_WRONG, "Undefined trap: system halts.\n");
  Halt();
}

// End of Traps.c
