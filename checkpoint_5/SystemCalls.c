// SystemCalls.c
//
// Julien Blanchet and Jae Heon Lee.
//
// See comments in SystemCalls.h for functionality details.
// (Comments in this file concern implementation details).

#include "Traps.h"

// Wait().
int HandleWait(int* status) {
  pcb_t* proc = RUNNING.head;

  // This is programming error.
  if( proc->num_children < 0 ) {
    TracePrintf(TRACE_WRONG, "HandleWait(): process #%d reports %d children.\n",
		proc, proc->num_children);
    Halt();
  }

  // This can happen if proc calls Wait() without having any children first.
  if( proc->num_children == 0 ) {
    return ERROR;
  }

  // This can happen if proc calls Wait() before any of its children dies.
  if( proc->dead_children_head == NULL ) {
    ContextSwitch(READY.head, &READY, &WAITING);
  }

  // At this point, at least one child is dead. Collect its soul. (first verify ptr)
  if (CheckUserPointer(status, PROT_READ | PROT_WRITE)){
    *status = proc->dead_children_head->status;
  }else{
    TracePrintf(TRACE_USER_WARNING, "HandleWait: Was passed invalid UserPointer. Not saving status\n");
  }
  proc->num_children--;

  // Remove the soul from the list of dead children.
  proc->dead_children_head = proc->dead_children_head->next;
  if( proc->dead_children_head == NULL ) {
    proc->dead_children_tail = NULL;
  }

  return SUCCESS;
}


// Fork().
//
// 1. Prepare a PCB for the new process.
//    JHL. PCBs live in kernel heap, right?
// 2. Initialize the PCB.
// 3. Copy the entire address space.
// 4. Copy the contents of the address space in region 1.
// 5. Add child to READY queue.
// 6. Set return values.
// 7. Freeze kernel context and copy content of address space in kernel stack.
//
// Returns child PCB (upon success) or NULL (upon failure).
pcb_t* HandleFork(void) {
  // 1. Prepare a PCB.
  // 2. Initialize child's PCB.
  pcb_t* parent = RUNNING.head;
  pcb_t* child  = (pcb_t*)malloc(sizeof(pcb_t));
  assert(child);
  memcpy(child, parent, sizeof(pcb_t));
  child->pid  = NEXT_PID;
  child->ppid = parent->pid;
  child->prev = NULL; // Not in any queue yet.
  child->next = NULL;
  child->num_children = 0;
  child->dead_children_head = NULL;
  child->dead_children_tail = NULL;
  RegisterPCB(child);
  parent->num_children++;

  // 3. Copy address space.
  { int i;
    int j;

    // R1 page table:
    for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
      if( child->r1_page_table[i].valid ) {
	// Find a new frame for each valid frame.
	child->r1_page_table[i].pfn = FindFreeFrame(child->r1_page_table[i].prot);

	// If there are insufficient free frames, return failure.
	if( ERROR == child->r1_page_table[i].pfn ) {
	  TracePrintf(TRACE_CRITICAL, "HandleFork(): insufficient memory for process #%d to create new process #%d.\n", parent->pid, child->pid);

	  // First free all frames allocated for child.
	  for(j = 0; j < i; j++) {
	    if( child->r1_page_table[j].valid ) {
	      frame_table[child->r1_page_table[j].pfn].valid = 0;
	      frame_table[child->r1_page_table[j].pfn].prot  = PROT_NONE;
	    }
	  }
	  // Proceed to cleanup.
	  goto fail_fin;
	}

      }
    }

    // R0 stack page table.
    for(i = 0; i < R0_STACK_PAGE_TABLE_SIZE; i++) {
      child->r0_stack_page_table[i].pfn = FindFreeFrame(child->r0_stack_page_table[i].prot /* PROT_READ | PROT_WRITE */);
      
      // If there are insufficient free frames, return failure.
      if( ERROR == child->r0_stack_page_table[i].pfn ) {
	TracePrintf(TRACE_CRITICAL, "HandleFork(): insufficient memory for process #%d to create new process #%d.\n", parent->pid, child->pid);

	// First free all frames allocated for child.
	for(j = 0; j < i; j++) {
	  if( child->r0_stack_page_table[j].valid ) {
	    frame_table[child->r0_stack_page_table[j].pfn].valid = 0;
	    frame_table[child->r0_stack_page_table[j].pfn].prot  = PROT_NONE;
	  }
	}
	// Proceed to cleanup.
	goto fail_r1_page_table;
      }
    }
  }
  
  // 4. Copy contents of address space in region 1.
  { int i;
    unsigned char PAGE_BUFFER[PAGESIZE];
    for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
      if( child->r1_page_table[i].valid ) {
	int prot = child->r1_page_table[i].prot;
	parent->r1_page_table[i].prot = PROT_READ;
	child ->r1_page_table[i].prot = PROT_WRITE;
	memcpy(PAGE_BUFFER, VMEM_1_BASE + i*PAGESIZE, PAGESIZE);
	ChangeAddressSpace(child);
	memcpy(VMEM_1_BASE + i*PAGESIZE, PAGE_BUFFER, PAGESIZE);
	ChangeAddressSpace(parent);
	parent->r1_page_table[i].prot = prot;
	child ->r1_page_table[i].prot = prot;
      }
    }
  }

  // 5. Add child to READY queue.
  AddToQueue(child, &READY);

  // 6. Set return values.
  // Note that this doesn't actually set return values.
  // HandleTrapKernel() must act upon these values to set actual return values.
  //  parent->u_context.regs[0] = child->pid;
  //  child ->u_context.regs[0] = 0;
  // JHL.
  // Maybe I don't need this part, insofar as I return child's PCB.
  // JBB. Agreed

  // 7. Freeze kernel context and kernel stack.
  CopyKernelStack(child, parent);
  LoadKernelContext(child);

  return child;
  // Failure cleanup below:

 fail_r1_page_table:
  { int i;
    for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
      if( child->r1_page_table[i].valid ) {
	frame_table[child->r1_page_table[i].pfn].valid = 0;
	frame_table[child->r1_page_table[i].pfn].prot  = PROT_NONE;
      }
    }
  }
 fail_fin:
  free(child);
  parent->u_context.regs[0] = ERROR;
  return NULL;
}

int HandleDelay(int delay_ticks, UserContext* u_context){
  if (delay_ticks < 0) {
    return ERROR;
  }
  if (delay_ticks == 0) {
    return SUCCESS;
  }
  
  RUNNING.head->wakeup_time = ticks + delay_ticks;
  TracePrintf(TRACE_VERBOSE, "HandleDelay() at tick %d: putting process #%d to sleep until tick %d\n",
	      ticks, RUNNING.head->pid, RUNNING.head->wakeup_time);

  memcpy(&RUNNING.head->u_context, u_context, sizeof(UserContext));
  ContextSwitch(READY.head, &READY, &SLEEPING);
  memcpy(u_context, &RUNNING.head->u_context, sizeof(UserContext));

  return SUCCESS;
}

int HandleExec(char* filename, char** argv) {
  if (argv == NULL){
    TracePrintf(TRACE_USER_WARNING, "HandleExec: passed invalid arguement array pointer\n");
    return ERROR;
  }

  int pid = RUNNING.head->pid;
  int ppid = RUNNING.head->ppid;
  //TracePrintf(TRACE_COMMENT, "HandleExec: before load program, pid#%d, ppid#%d\n", pid, ppid);
  int rv = LoadProgram(filename, argv, RUNNING.head);
  pid = RUNNING.head->pid;
  ppid = RUNNING.head->ppid;
  TracePrintf(TRACE_COMMENT, "HandleExec: after load program, pid#%d, ppid#%d\n", pid, ppid);
  
  if( rv == SUCCESS ) {
    return SUCCESS;
  }

  if( rv == ERROR ) {
    return ERROR;
  }

  /* rv == KILL */
  // The process must be killed.
  return KILL;
}

int HandleBrk(void* requested_addr){
  int pt_request_index = r1_addr_to_id(requested_addr);
  pcb_t* cur_pcb = RUNNING.head;
  int pt_br_cur_index = cur_pcb-> r1_break_limit_index;

  // if the user process already "owns" that memory, simply return the current break
  if (pt_request_index  <  pt_br_cur_index) {
    TracePrintf(TRACE_VERBOSE, "HandleBrk(): program already has %p.\n", requested_addr);
    return SUCCESS;
  }
  
  // cannot grow heap into, or within one page of, the stack
  else if (pt_request_index >= cur_pcb -> r1_stack_base_index - 1) {
    TracePrintf(TRACE_COMMENT, "HandleBrk(): cannot heap in stack, at %p.\n", requested_addr);
    return ERROR;
  }

  // if we've gotten to this point, it's a valid request and we need to grow the stack
  TracePrintf(TRACE_VERBOSE, "HandleBrk(): will grant %p.\n", requested_addr);

  // step 1: gather necessary free frames
  int frames_required = pt_request_index - pt_br_cur_index;
  int acquired_frames [frames_required];
  int num_acquired_frames = 0;
  while(num_acquired_frames < frames_required){
    int new_frame = FindFreeFrame(PROT_READ | PROT_WRITE);

    // JBB: in future, we should change this to gracefully free the frames we've acquired, than return -1
    if (new_frame == ERROR){
      TracePrintf(TRACE_CRITICAL, "HandleBrk: Insufficent memory to grow user heap\n");
      Halt();
    }
	
    acquired_frames [num_acquired_frames] = new_frame;
    num_acquired_frames++;
  }

  TracePrintf(TRACE_VERBOSE, "HandleBrk(): %d frames found for %p.\n", num_acquired_frames, requested_addr);

  //step 2: arrange ptes
  int i;
  for(i = 0; i < frames_required; i++){
    cur_pcb->r1_page_table[i + pt_br_cur_index + 1].valid = 1;
    cur_pcb->r1_page_table[i + pt_br_cur_index + 1].prot  = PROT_READ | PROT_WRITE;
    cur_pcb->r1_page_table[i + pt_br_cur_index + 1].pfn   = acquired_frames[i];
  }

  // set new break and return to user
  cur_pcb -> r1_break_limit_index = pt_br_cur_index + frames_required;
  TracePrintf(TRACE_VERBOSE, "HandleBrk(): new break at index %d, max addr %p.\n", 
	      cur_pcb->r1_break_limit_index,
	      UP_TO_PAGE((r1_id_to_addr(cur_pcb->r1_break_limit_index))));
  return SUCCESS;

} // end of HandleBrk

int HandleTtyWrite(int tty_id, void* buffer, int length) {
  TracePrintf(TRACE_VERBOSE, "HandleTtyWrite() called with tty_id:%d, buffer:%p of length: %d.\n", tty_id, buffer, length);
  pcb_t* proc = RUNNING.head;

  if( tty_id < 0 || tty_id >= NUM_TERMINALS ) {
    TracePrintf(TRACE_USER_WARNING, "HandleTtyWrite(): Tty #%d does not exist.\n", tty_id);
    return ERROR;
  }

  if( length < 0 ) {
    TracePrintf(TRACE_USER_WARNING, "HandleTtyWrite(): Cannot write %d bytes.\n", length);
    return ERROR;
  }

  // Check the validity of buffer.
  if(!CheckUserBuffer(buffer, length, PROT_READ)) {
    return ERROR;
  }

  // Wait until my time comes.
  while( WRITING[tty_id].head != NULL ) {
    // i.e., while someone is in queue and I'm not the first in queue,
    
    here();
    ContextSwitch(READY.head, &READY, &WRITING_WAIT[tty_id]);
    here();
    // Stand in line and wait.
  }

  int transmitted = 0;

  while( transmitted < length ) {
    int requested;
    if( /*remaining*/ length - transmitted > TERMINAL_MAX_LINE ) {
      requested = TERMINAL_MAX_LINE;
    } else {
      requested = length - transmitted;
    }

    memcpy(tty_write_buffer[tty_id], buffer + transmitted, requested);
    TtyTransmit(tty_id, tty_write_buffer[tty_id], requested);
    ContextSwitch(READY.head, &READY, &WRITING[tty_id]);

    transmitted += requested;
  }

  TracePrintf(TRACE_VERBOSE, "HandleTtyWrite(): Process #%d finishes writing to tty #%d.\n", proc->pid, tty_id);

  return SUCCESS;
}

int HandleTtyRead(int tty_id, void* buffer, int length) {
  pcb_t* proc = RUNNING.head;

  if( tty_id < 0 || tty_id >= NUM_TERMINALS ) {
    TracePrintf(TRACE_USER_WARNING, "HandleTtyRead(): Tty #%d does not exist.\n", tty_id);
    return ERROR;
  }

  if( length < 0 || length > TERMINAL_MAX_LINE ) {
    TracePrintf(TRACE_USER_WARNING, "HandleTtyRead(): Cannot read %d bytes.\n", length);
    return ERROR;
  }

  // Check the validity of buffer.
  if(!CheckUserBuffer(buffer, length, PROT_READ | PROT_WRITE)) {
    return ERROR;
  }

  ContextSwitch(READY.head, &READY, &READING[tty_id]);

  int rv = TtyReceive(tty_id, tty_read_buffer[tty_id], TERMINAL_MAX_LINE);

  here();
  TracePrintf(0, "%d.\n", rv);
  here();

  int actual_length = length > rv ? rv : length;
  memcpy(buffer, tty_read_buffer[tty_id], actual_length * sizeof(char));

  return actual_length;
}

// End fof SystemCalls.c
