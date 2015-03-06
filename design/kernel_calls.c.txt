// kernel_calls.c
//
// Julien Blanchet and Jae Heon Lee.

// Note. This is NOT a source file, at least at this point.
//
// Contains pseudocode for implementing kernel calls.
//
// Relevant documentation: 4.1.

// Hints: Class (2013-10-07) 3., 5.5.
int    // Parent gets child's pid; child gets 0. If error, parent gets ERROR.
Fork
(void)
{
  // Create a new pcb and copy parent's pcb onto it (except a few parts..).
  // Assign it a new pid (and increment the pids used so far).
  // Assign the parent's pid as its ppid.
 
  // In a new page table, clone valid bits and permissions.
  // Going through parent's page table,
  for( /* each valid entry */) {
    // Obtain a free frame from the list of free frames.
    // Find invalid ptes and map them to free frames. Use the address of these new ptes as
    // destinations for copying the content of the parent's frame into the corresponding new frame.
    // Let the child's relevant pte point to this frame.
    // Mark temporary pte as invalid again.
  }
  // Also for the kernel stack page table.

  // Load return values differently for parent (child's pid) and child (0).
  // pcb is placed to the ready queue.

  // Clone parent's kernel context.
}

// Hints: Class (2013-10-07) 3., 5.6.
int               // If success, no return; returns ERROR otherwise.
Exec
(char*  filename, // Program to run.
 char** argvec)   // JHL. Are these each null-terminated strings + one null pointer?
{
  // Copy important information to some buffer.
  // Such information includes pid, ppid, argvec, etc., as well as arguments to new program.

  // Free all frames and mark all ptes as invalid.

  // Load image of the new program at filename to this process. Feed arguments.
  LoadProgram( /*...*/ );
  // We're assuming that it'll find free frames for its contents and udpate the page table.
}

void
Exit
(int status)
{
  if( /* is not orphan */ ) { // Check ppid for orphanhood?
    /* PCB */->status = status;
  }
  // Orphans don't leave status information behind.

  // JHL. Check for children and mark them as orphans?
  // Let them check for orphanhood upon their own demise?

  // Free (almost) all resources. Stack, heap, what else?

  if( /* is initial process */ ) {
    Halt();
  }
}

int               // If successful, child's pid. Otherwise ERROR.
Wait
(int* status_ptr) // Child's exit status.
{
  // Search the queue of dead processes to find children.
  while( /* child's information is not found */ ) {
    // Block.
  }
  // At this point, child's information is found.

  *status_ptr = /* dead process's PCB */->status;
    int child_pid = /* dead process's PCB */->pid;

  // Destory the dead process information.

  return child_pid;
}

int    // pid
GetPid
(void)
{
  return /* my PCB */->pid;
}
// JHL. Is this it?

int          // 0 if successful; ERROR otherwise.
Brk
(void* addr) // The next lower limit (rounded up to multiples of PAGESIZE) 
{
  // JHL. Should error check be here? Should it be left to trap handlers?
  if( /* addr is not valid */ ) {
    TracePrintf( /* (int) level */, "Brk(): invalid address request, %p", addr);
    return ERROR;
  }

  if( /* enough space */ ) {
    // Make space (physical memory) available to the program.
    // Reset break to addr.

    if( /* successful */ ) {
      return 0;
    } else {
      TracePrintf( /* (int) level */, "Brk(): allocation unsuccessful: %p", addr);
      return ERROR;
    }

  } else {
    TracePrintf( /* (int) level */, "Brk(): insufficient space: %p", addr);
    return ERROR;
  }
}

int               // 0 if successful; ERROR otherwise.
Delay
(int clock_ticks) // The duration (in clock ticks) for which this process shall be blocked.
{
  // Check validity of argument.
  if( clock_ticks < 0 ) {
    return ERROR;
  }

  // Return immediately if waiting for 0 ticks.
  if( clock_ticks == 0 ) {
    return 0;
  }
  // At this point, clock_ticks > 0.

  // Change, in PCB, information about how this process handles TRAP_CLOCK.
  /* PCB */->ticks_remaining = clock_ticks;
  // Move this process to the queue of processes blocked waiting for TRAP_CLOCK.
  // Dequeue from queue of ready processes and put it to the runnning process.
  // Context switch the two processes.

  return 0;
}

int            // String copied onto buf if successful (not necessarily == len); ERROR otherwise.
TtyRead
(int   tty_id, // Tty.
 void* buf,    // Buffer to which string shall be copied. 
 int   len)    // Maximum length of byte which shall be copied.
{
  char buf_region_0 [len];

  // JHL. What's the relationship between this function and TtyRecieve()?
  // Should this (like TtyWrite()) also support more bytes than TERMINAL_MAX_LINE?
  // (void*) buf in TtyReceive() is restricted to reside in reigion 0. I'm assuming that the same
  // is not true for this function?

  // Not supporting more than TERMINAL_MAX_LINE bytes:

  // Check for valid argument
  if( len < 0 ||
      tty_id < 0 ||
      tty_id >= NUM_TERMINALS) {
    TracePrintf( /* (int) level */, "TtyRead() fails: tty %d at %p for %d bytes", tty_id, buf, len);
    return ERROR:
  }

  // Move PCB from running to this list: processes_waiting_tty_receive[tty_id]
  // blockProcessAndStartReady(); // at this point, blocked until tty receive interrupt
  
  int bytes_received = TtyReceive(tty_id, (void*) buf_region_0, TERMINAL_MAX_LINE /* Cf. 3.3.1. */);
  
  if( bytes_received == ERROR ) {
    TracePrintf( /* (int) level */, "TtyRead() fails: tty $d at %p for %d bytes", tty_id, buf, len);
    return ERROR;
  } 

  int bytes_to_write = bytes_received;

  if (bytes_to_write > len)
      bytes_to_write = len;

  memcpy(buf, buf_region_0, bytes_to_write); // memcpy is INFALLIABLE!!!!!!
  
  // At this point, string has been copied to buf (or error occurred).
  return bytes_to_write /* length copied */;
}

int            // Number of bytes written to tty.
TtyWrite
(int   tty_id, // Tty.
 void* buf,    // Buffer storing string to write.
 int   len)    // Length of string.
{
  // JHL. Argument validity checking?
  // If checking addr, make sure to check the whole string, from addr to addr+len.

  // Somehow obtain buffer in region 0. Cf. 3.3.1.
  void* buf_region_0 = /* buffer in region 0 */;

  // More bytes than TERMINAL_MAX_LINE should be supported.
  int current_len = len; // JHL. Copy in case original len may be needed?
  int transmit_len;      // Length for use during a single call to TtyTransmit().
  int success_len = 0;   // Length of bytes successfully transmitted so far.
  int retval;
  while( current_len > 0 ) {
    if( current_len > TERMINAL_MAX_LINE ) {
      transmit_len = TERMINAL_MAX_LINE;
    } else {
      transmit_len = current_len;
    }

    memset(buf_region0, buf, transmit_len);
    retval = TtyTransmit(tty_id, buf_region0 + success_len /* unsent so far */, transmit_len);
    if( retval == ERROR ) {
      TracePrintf(/* (int) level */,
		  "TtyWrite() fails: tty $d at %p for %d bytes (%d bytes successful)",
		  tty_id, buf, len, success_len);
      return ERROR;
    }

    // Block process until TRAP_TTY_TRANSMIT is handled.
    // JHL. TtyTransmit() returns immediately, but it is only after receiving this trap that
    // bytes are actually transmitted. I suspect that bad things might occur if we try to
    // change bytes at buf_region_0 before transmission is successful.

    // At this point, retval != ERROR.
    if( retval != transmit_len ) {
      // What if this is so?
      // transmit_len = retval; ?
    }

    success_len += transmit_len; // JHL. Is this duplicate bookkeeping?
    current_len -= transmit_len;
  }


}

int             // ERROR if failure.
PipeInit
(int* pipe_idp) // Pipe id is stored here.
{
  // JHL. I don't quite understand how pipes work... Yet...

  // Somehow create a new pipe.
  *pipe_idp = /* identifier */;

  return 0;
}

int             // Number of bytes read if successful.
PipeRead
(int   pipe_id, //
 void* buf,     // Read bytes will be stored here.
 int   len)     // Length of bytes to read.
{
  // Block this process and wait until pipe is filled.
  // JHL. How do we know?

  // Somehow read from pipe.
  memset(buf, /* pipe buffer? */, len);

  // What if written length != reading length?
  // Do we need special care for such cases?

  // Move the process to ready queue.
}

int             // ==len if successful; ERROR otherwise.
PipeWrite
(int   pipe_id, //
 void* buf,     // Bytes to write.
 int   len)     // Number of bytes to write.
{
  // Block until write is complete?

  // JHL. How?
}

int             // ERROR if failure.
LockInit
(int* lock_idp)
{
  // Procure space for a lock instance and initialize it.
  *lock_idp = /* identifier */;

  return 0;
}

int           // ERROR if failure.
Acquire
(int lock_id) //
{
  // Find lock from lock_id.
  // Check if its status is already acquired by this process.
  if( /* lock */->status == ACQUIRED && /* lock */->owner /* is this process */ ) {
    return ERROR;
  }

  // Race conditions could occur if interrupt could happen in kernel.
  while( /* lock */->status == ACQUIRED ) { // if-condition may suffice here.
    // put this process on lock's waiting queue (will be blocked).
    // move next process on ready queue to running
    // context switch the that process (which we just moved to runnning)
  }

  // Change its status from unacquired to acquired (by this process).
  /* lock */->status = ACQUIRED;
  /* lock */->owner  = /* this process */;

  return 0;
}

int           // ERROR if failure.
Release
(int lock_id) //
{
  // Find lock from lock_id.
  if( /* lock */->owner == /* this process */ && /* lock */->status == ACQUIRED ) {
    /* lock */->status = UNACQUIRED;

    if( /*lock*/->waiters != null ) {

      // Remove the first waiter from the queue of waiters.
      // This waiter process goes to the ready queue.

      // Changing of lock's status and owner will happen as the waiter process runs and continues Acquire().
    }

    return 0;
  } else {
    return ERROR;
  }
}

int             // ERROR if failure.
CvarInit
(int* cvar_idp)
{
  // Procure space for a cvar instance and initialize it.
  *cvar_idp = /* identifier */; // give them the lock id

  return 0;
}

// Mesa-style.
int           // ERROR if failure.
CvarSignal
(int cvar_id) //
{
  // Check for validity.
  if( /* invalid */ ) {
    return ERROR;
  }

  if (/*cvar*/->waiters == NULL){
    return 0;
  }

  pcb_t* firstWaiter = waiters;
  //

  // JHL.
  // Change the status of cvar?

  // Dequeue the first process in the waiting list.
  // Unblock this process.
}

// Mesa-style.
int           // ERROR if failure.
CvarBroadcast
(int cvar_id) //
{
  // Check for validity.

  // Dequeue all processes in the waiting list.
  // Unblock all such processes.
}

int           // ERROR if failure.
CvarWait
(int cvar_id, //
 int lock_id) // Lock associated with this cvar.
{

  // JHL. I feel that this should be done in a more graceful way
  Release(lock_id);

  
  // Enqueue this process to the list of waiting processes.
  // move_to_blocked(&(cvar->waiters))

  // Upon waking, acquire lock.
  Acquire(lock_id);
}

int      // ERROR if failure.
Reclaim
(int id) // Id for lock, cvar, or pipe.
{
  // Find the lock, cvar, or pipe from id.
  // Destroy the object  and free resources.
}
