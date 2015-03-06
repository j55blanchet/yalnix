// traps.c
//
// Julien Blanchet and Jae Heon Lee.

// Note. This is NOT a source file, at least at this point.
//
// Contains pseudocode for implementing trap-handler functions.
//
// Relevant documentation: 3.4., 4.2.


// TRAP_KERNEL
void
HandleTrapKernel
(UserContext* u_context)
{
  switch( u_context->code /* kernel call number */ ) {

    /*
     * Examples:
     *   - Creating a new process.
     *   - Allocating memory. // JHL. Is this confined to malloc() family?
     *   - Performing I/O.
     * (3.4.1.).
     */

  case /*...*/ :
    // Retrieve arguments from registers (regs[8]).
    // Invoke corresponding kernel call with the arguments.
    break;
  }
}

// TRAP_CLOCK
void
HandleTrapClock
(UserContext* u_context)
{
  // JHL. How about processes blocked for time-related operations?
  // From the list of processes blocked waiting for Delay() to return,
  for(/* each process */) {
    /* pcb */->ticks_remaining--;
    if( /* pcb */->ticks_remaning == 0 ) {
      // Move pcb to the ready queue.
    }
  }

  // "round-robin process scheduling with a CPU quantum per process of 1 clock tick."
  // i.e.:
  // Move the running process to the end of the ready queue,
  // Dequeue the ready queue and put that process to running,
  // Context-switch between the above two processes.
  if( /* ready queue has other runnable processes */ ) {
    // Context-switch to the next runnable process.
  }
}

// TRAP_ILLEGAL
void
HandleTrapIllegal
(UserContext* u_context)
{
  switch( u_context->code /* type of illegal instruction */ ) {

    /*
     * Examples:
     *   - Undefined machine language opcode.
     *   - Illegal addressing mode.
     *   - Previleged instruction from user mode.
     * (3.4.1.).
     */

  case /*...*/:
    TracePrintf( /* (int) level */, "TRAP_ILLEGAL: %s\n", /* case-specific message */);
    break;
  }
  TracePrintf(level, "Aborting process with pid %d\n", pcb->pid);

  // Abort current process. JBB: Do this by:
  // Zero out the PCB (more likely for us to catch errors that refer to this dead PCB)
  // Free the PCB*
  // Move next process in ready queue to running
}

// TRAP_MEMORY
void
HandleTrapMemory
(UserContext* u_context)
{
  switch( u_context->code /* type of disallowed memory access  */ ) {

  /* implicit requests to grow the stack go to map error */
  case YALNIX_MAPERR: /* address not mapped */

    /* test if this represents a implicit stack expansion by seeing if 
    the requsted address is between the current allocated stack memory and 
    the break for this process */
    int pte_index = /*PAGE_TABLE_INDEX*/(u_context->addr); 
    if ( pte_index > pcb-> stack_lowest_pte_index &&
         pte_index < pcb-> user_break_pte_index - 1 
         /* leave space between stack and heap so that stack does not silently
         grow into the heap */)
    {
      //if here, is implicit request to grow memory

      // Grant stack space between current stack allocation and requested address
      while(pcb -> stack_lowest_pte_index.lowest_address_in_that_pte > u_context -> addr){
        // pop frame from free frame list
        // if no free frames, abort process

        // make pte of stack_lowest_pte_index + 1 point to this frame
        // set pte to valid with RW permissions
        stack_lowest_pte_index++
      }

      return; /* Stack expansions allocation is complete*/
    }
    TracePrintf(/* (int) level */, "Invalid memory access location: stack overflow (touching heap)\n");
    break;

  case YALNIX_ACCERR:
    TracePrintf(/* (int) level */, "MEM_ACCESS_ERR: %x was accessed with invalid permissions\n", u_context->addr);
    break;
  }
  // if here, error has already been printed; abort process
}

// TRAP_MATH
void
HandleTrapMath
(UserContext* u_context)
{
  switch( u_context->code /* type of math error */ ) {
    
    /*
     * Examples:
     *   - Division by zero.
     *   - Arithmetic overflow. // JHL. Is this considered an error?
     * (3.4.1.).
     */
    // JHL. Are there other types of math errors?

  case /*...*/ :
    TracePrintf( /* (int) level */, "TRAP_MATH: %s\n", /* case-specific message */);
    break;
  }

  // Abort current process.
}

// TRAP_TTY_RECEIVE
void
HandleTrapTtyReceive
(UserContext* u_context)
{
  int tty_id = u_context->code;
  // TWO CASES:
  // case 1: nobody is waiting to receive from this terminal
  if (processes_waiting_tty_receive[tty_id] == NULL){
    char buf[TERMINAL_MAX_LINE];
    int bytes_read = TtyReceive(tty_id, buf, sizeof buf);
    if (bytes_read < 0){
      TracePrintf(/* level */, "TtyReceive() failed during TrapTtyReceive, but nobody was waiting for it anyways\n");

      // keep going
      return;
    }
  }

  // case 2: somebody IS waiting to receive form this terminal
  else{

    // move currently running process to READY queue
    // move associated process DIRECTLY into  running (so that it can immediatley recieve this input)
    // context switch to that process

  }
}

// TRAP_TTY_TRANSMIT
//
// This trap is generated upon *completion* of, not invocation of, TtyTransmit() (3.3.1., 3.4.1.).
void
HandleTrapTtyTransmit
(UserContext* u_context)
{
  int tty_id = u_context -> code;

  // something is waiting
  if (processes_waiting_tty_transmit != NULL){
    // move that process to the ready queue (that's all!)
  }

  // if a PCB isn't waiting when we receive this, that is an ERROR!
  // so we complain
  else{
    TracePrintf(/* really high level */, "Error! Received TrapTtyTransmit and no process was waiting for it\n");
    Halt();
  }
}

// Undefined traps.
//
// Trap vectors for undefined traps should point to this function.
void
HandleTrapUndefined
(UserContext* u_context)
{
  TracePrintf(/* (int) level */, "Undefined trap encountered!\n");
  Halt();
}

// End of traps.c
