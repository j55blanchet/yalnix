// Utility.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Utility functions.

#include "Utility.h"

// simple check to ensure that a permission field contains desired permissions
inline int CheckProtection(int to_check, int desired_protection){
    int relevant_protection = to_check & desired_protection;
    int result = relevant_protection - desired_protection;
    return (result == 0);
}

int CheckUserString(char* string, int desired_protection, int max_bytes){
    char* stringLoc = string;
    int bytes_so_far = 0;
    while(max_bytes == -1 || bytes_so_far < max_bytes){
        if (!CheckUserPointer(stringLoc, desired_protection)){
            TracePrintf(TRACE_VERBOSE, "Invalid UserString %d bytes into string at %p, which started at %p\n", bytes_so_far, stringLoc, string);
            return 0;
        }
            
        if (*stringLoc == '\0')
            return 1;
        
        stringLoc++;
        bytes_so_far++;
    }
    TracePrintf(TRACE_VERBOSE, "Invalid UserString at %d bytes into string at %p, which started at %p\n", bytes_so_far, stringLoc, string);
    return 0;
}
int CheckUserBuffer(void* buffer, int len, int desired_protection){
    int i;
    for(i = 0; i < len; i ++){
        // used casts to avoid compiler warnings (these casts won't affect program though)
        if (!CheckUserPointer((void*) &(((char*) buffer)[i]), desired_protection)){
            return 0;
        }
    }
    return 1;
}

int CheckUserPointer(void* ptr, int desired_protection){
    // runs 3 checks:
    // - PTR is within acceptable range
    // - PTR points to valid PTE
    // - those PTE permissions contain the PROTECTIONS that are desired
    if (ptr == NULL)
        return 0;
    
    int pt_index = r1_addr_to_id(ptr);
    if (pt_index < 0 || pt_index >= R1_PAGETABLE_NUM_ENTRIES)
        return 0;
        
    pte_t* pte = &(RUNNING.head->r1_page_table[pt_index]);
    if (pte->valid == 0)
        return 0;

    return CheckProtection(pte->prot, desired_protection);
}

int FindFreeFrame(int PROT_CODE) {
  // Check legitimacy of call.
  if( frame_table == NULL ) {
    TracePrintf(TRACE_WRONG, "GetFreeFrame(): frame_table not initialied\n");
  }

  int i;
  for(i = frame_addr_to_id(KERNEL_DATA_END)/*lowest usable frame?*/; i < FRAME_TABLE_SIZE; i++) {
    if( frame_table[i].valid == 0 ) {
      frame_table[i].valid = 1;
      frame_table[i].prot  = PROT_CODE;
      return i;
    }
  }
  // At this point, frame table is full.

  TracePrintf(TRACE_SEVERE, "FindFreeFrame(): cannot find a free frame\n");
  return ERROR;
}

int FreeFrame(int index){
  if (index >= FRAME_TABLE_SIZE || frame_table[index].valid == 0) {
    TracePrintf(TRACE_WRONG, "FreeFrame(): no frame has index  %d.\n", index);
    Halt();
  }

  frame_table[index].valid = 0;
  frame_table[index].prot  = PROT_NONE;
  return SUCCESS;
}

void InitQueue(queue_t* QUEUE){
  QUEUE->size = 0;
  QUEUE->head = NULL;
}

int AddToQueue(pcb_t* proc, queue_t* QUEUE) {
  if( QUEUE->head == NULL ) {
    QUEUE->head = proc;
    proc->next  = proc;
    proc->prev  = proc;
  } else {
    proc->next              = QUEUE->head;
    proc->prev              = QUEUE->head->prev;
    QUEUE->head->prev->next = proc;
    QUEUE->head->prev       = proc;
  }
  
  QUEUE->size++;
  return SUCCESS;
}

int RemoveFromQueue(pcb_t* proc, queue_t* QUEUE) {
  if( QUEUE->head == NULL ) {
    TracePrintf(TRACE_WRONG, "RemoveFromQueue(): tried to remove from an empty queue.\n");
    Halt();
  }
  if( QUEUE->head == proc ) {
    goto found;
  } else /* ( QUEUE->head != proc ) */ {
    pcb_t* item;
    for(item = QUEUE->head->next; item != QUEUE->head; item = item->next) {
      if( item == proc ) {
	goto found;
      }
    }
    TracePrintf(TRACE_WRONG, "RemoveFromQueue(): cannot find process from queue.\n");
    Halt();
  }

 found:
  if( QUEUE->head == proc && proc->next == proc ) { 
    // proc is only process in QUEUE.
    QUEUE->head = NULL;
  } else if( QUEUE->head == proc ) { 
    // There are other processes in QUEUE.
    QUEUE->head = proc->next;
  }
  proc->next->prev = proc->prev;
  proc->prev->next = proc->next;

  proc->next = NULL;
  proc->prev = NULL;

  QUEUE->size--;
  return SUCCESS;
}

void ChangeAddressSpace(pcb_t* new_process) {
  int i;
  
  // Change kernel stack address space. We write new_process's page table
  // into the global page table for Region 0, for the kernel stack part.
  for(i = 0; i < R0_STACK_PAGE_TABLE_SIZE; i++) {
    memcpy(&r0_page_table[r0_addr_to_id(KERNEL_STACK_BASE) + i],
	   &(new_process->r0_stack_page_table[i]), sizeof(pte_t));
  }

  // Change the page table we look for when we look at Region 1.
  WriteRegister(REG_PTBR1, (unsigned int) new_process->r1_page_table);

  // Flush TLB to avoid confusion.
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
}

void TraceRegs(int level, UserContext* u_context){
  TracePrintf(level, "Usercontext Regs:");
  int i;
  for(i=0; i<8; i++){
    TracePrintf(level, "    reg[%d]:%p\t%d\n", i, u_context->regs[i], u_context->regs[i]);
  }
}

void PrintUserContextHelperHelper(UserContext* c, char* s_vector, char* s_code) {
  printf("UserContext\n");
  printf("  vector = %d(%s)\n", c->vector, s_vector);
  printf("  code   = 0x%x(%s)\n", c->code, s_code);
  printf("  addr   = %p\n",     c->addr);
  printf("  pc     = %p\n",     c->pc);
  printf("  sp     = %p\n",     c->sp);
#ifdef LINUX
  printf("  ebp    = %p\n",     c->ebp);
#endif
  { int i;
    for(i = 0; i < GREGS; i++) {
      printf("  regs[%i]=%x\t%d\n", i, c->regs[i], c->regs[i]);
    }
  }  
}
void TraceUserContextHelperHelper(UserContext* c, char* s_vector, char* s_code, int level) {
  TracePrintf(level,
	      "UserContext\n" TRACE_N
	      "  vector = %d(%s)\n" TRACE_N
	      "  code   = 0x%x(%s)\n" TRACE_N
	      "  addr   = %p\n" TRACE_N
	      "  pc     = %p\n" TRACE_N
	      "  sp     = %p\n" TRACE_N
#ifdef LINUX
	      "  ebp    = %p\n"
#endif	      
	      , c->vector, s_vector, c->code, s_code, c->addr, c->pc, c->sp
#ifdef LINUX
	      , c->ebp
#endif
	      );
  { int i;
    for(i = 0; i < GREGS; i++) {
      TracePrintf(level, "  regs[%i]=0x%x\n", i, c->regs[i]);
    }
  }
}
void PrintUserContextHelper(UserContext* c, int TracePrintf, int level) {
  char* s_vector;
  char* s_code;
  switch( c->vector ) {
  case TRAP_KERNEL:
    s_vector = "TRAP_KERNEL";
    switch( c->code ) {
    case YALNIX_DELAY:
      s_code = "Delay()";
      break;
    case YALNIX_EXIT:
      s_code = "Exit()";
      break;
    case YALNIX_GETPID:
      s_code = "GetPid()";
      break;
    case YALNIX_NOP:
      s_code = "NOP";
      break;
    case YALNIX_BRK:
      s_code = "Brk()";
      break;
    case YALNIX_FORK:
      s_code = "Fork()";
      break;
    case YALNIX_EXEC:
      s_code = "Exec()";
      break;
    case YALNIX_WAIT:
      s_code = "Wait()";
      break;
    case YALNIX_TTY_WRITE:
      s_code = "TtyWrite()";
      break;
    case YALNIX_TTY_READ:
      s_code = "TtyRead()";
      break;
    default:
      s_code = "Undefined";
      break;
    }
    break;
  case TRAP_CLOCK:
    s_vector = "TRAP_CLOCK";
    switch( c->code ) {
    default:
      s_code = "Undefined";
      break;
    }
    break;
  case TRAP_ILLEGAL:
    s_vector = "TRAP_ILLEGAL";
    break;
  case TRAP_MEMORY:
    s_vector = "TRAP_MEMORY";
    switch( c->code ) {
    case YALNIX_MAPERR:
      s_code = "YALNIX_MAPERR";
      break;
    case YALNIX_ACCERR:
      s_code = "YALNIX_ACCERR";
      break;
    default:
      s_code = "Undefined";
      break;
    }
    break;
  case TRAP_MATH:
    s_vector = "TRAP_MATH";
    break;
  case TRAP_TTY_TRANSMIT:
    s_vector = "TRAP_TTY_TRANSMIT";
    s_code   = "";
    break;
  case TRAP_TTY_RECEIVE:
    s_vector = "TRP_TTY_RECEIVE";
    s_code   = "";
    break;
  default:
    s_vector = "Undefined";
    break;
  }

  if( TracePrintf > 0 ) {
    TraceUserContextHelperHelper(c, s_vector, s_code, level);
  } else {
    PrintUserContextHelperHelper(c, s_vector, s_code);
  }
}
void PrintUserContext(UserContext* c) {
  PrintUserContextHelper(c, 0, 0);
}
void TraceUserContext(int level, UserContext* c) {
  PrintUserContextHelper(c, 1, level);
}

void PrintQueueHelper(queue_t* QUEUE) {
  if( QUEUE->head == NULL ) {
    TracePrintf(TRACE_VERBOSE, "  (Empty queue).\n");
  } else {
    TracePrintf(TRACE_VERBOSE, "  Head: Process #%d(ppid #%d): '%s'\n", QUEUE->head->pid, QUEUE->head->ppid, QUEUE->head->pname);
    pcb_t* pcb;
    for(pcb = QUEUE->head->next; pcb != QUEUE->head; pcb = pcb->next) {
      TracePrintf(TRACE_VERBOSE, "        Process #%d(ppid #%d): '%s'\n", pcb->pid, pcb->ppid, pcb->pname);
    }
  }
}

void PrintAllQueues(void) {
  PrintQueue(&RUNNING);
  PrintQueue(&READY);
  PrintQueue(&SLEEPING);
  PrintQueue(&WAITING);
  //  { int i;
  //    for(i = 0; i < NUM_TERMINALS; i++) {
  //      PrintQueue(&WRITING[i]);
  //    }
  //    for(i = 0; i < NUM_TERMINALS; i++) {
  //      PrintQueue(&READING[i]);
  //    }
  //  }
  PrintQueue(&WRITING[0]);
  PrintQueue(&WRITING[1]);
  PrintQueue(&WRITING[2]);
  PrintQueue(&WRITING[3]);
  PrintQueue(&READING[0]);
  PrintQueue(&READING[1]);
  PrintQueue(&READING[2]);
  PrintQueue(&READING[3]);
  PrintQueue(&WRITING_WAIT[0]);
  PrintQueue(&WRITING_WAIT[1]);
  PrintQueue(&WRITING_WAIT[2]);
  PrintQueue(&WRITING_WAIT[3]);
}

void RegisterPCB(pcb_t* pcb) {
  if( pcb->pid >= pcb_array_size ) {
    TracePrintf(TRACE_VERBOSE, "pcb_array expands to accomodtate new process # %d.\n", pcb->pid);
    pcb_array_size += PCB_ARRAY_INITIAL_SIZE;
    pcb_array = (pcb_t**)realloc(pcb_array, pcb_array_size);
    assert(pcb_array);
  }
  pcb_array[pcb->pid] = pcb;  
}


void RegisterInterp(int id, interp_t* interp) {
  if( id >= interp_array_size ) {
    TracePrintf(TRACE_VERBOSE, "interp_array expands to accomodtate new interp # %d.\n", id);
    interp_array_size += INTERP_ARRAY_INITIAL_SIZE;
    interp_array       = (interp_t**)realloc(interp_array, interp_array_size);
    assert(interp_array);
  }
  interp_array[id] = interp;
}

pcb_t* InitPCB(int ppid) {
  pcb_t* pcb = (pcb_t*)calloc(sizeof(pcb_t), 1);

  pcb->pid         = NEXT_PID;
  pcb->ppid        = ppid;
  pcb->wakeup_time = 0;

  strncpy(pcb->pname, "Uninitialized", PCB_NAME_LENGTH);

  pcb->r1_stack_base_index  = -1; //r1_addr_to_id(VMEM_1_LIMIT);
  // Initialized in: LoadProgram().

  pcb->r1_break_limit_index = -1; //r1_addr_to_id(VMEM_1_BASE);
  // Initialized in: LoadProgram().

  pcb->prev = NULL;
  pcb->next = NULL;

  pcb->num_children = 0;
  pcb->dead_children_head = NULL;
  pcb->dead_children_tail = NULL;

  RegisterPCB(pcb);

  return pcb;
}

void ForgetDeadChildren(soul_t* proc) {
  if( proc == NULL ) {
    return;
  }
  ForgetDeadChildren(proc->next);
  free(proc);
}

// NOTE: no cascading termination. 
soul_t* KillPCB(pcb_t* pcb, int status) {
  // pcb->prev == pcb->next NULL implies that it is removed from a queue.
  if( pcb->prev != NULL || pcb->next != NULL ) {
    TracePrintf(TRACE_WRONG, "KillPCB(): process #%d is still in some queue.\n", pcb->pid);
    Halt();
  }

  soul_t* soul = (soul_t*) malloc(sizeof(soul_t));
  soul->pid    = pcb->pid;
  soul->status = status;
  soul->next   = NULL;

  pcb_array[pcb->pid] = NULL;

  // Free frames assigned for this process.
  int i;
  for(i = 0; i < R0_STACK_PAGE_TABLE_SIZE; i++) {
    if( pcb->r0_stack_page_table[i].valid ) {
      FreeFrame(pcb->r0_stack_page_table[i].pfn);
    }
  }
  for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
    if( pcb->r1_page_table[i].valid ) {
      FreeFrame(pcb->r1_page_table[i].pfn);
    }
  }

  ForgetDeadChildren(pcb->dead_children_head);

  // Add exit information to parent.
  pcb_t* parent = pcb_array[pcb->ppid];
  if( parent == NULL ) {
    // Parent is already dead.
    free(soul);
    return NULL;
  }

  if( parent->dead_children_head == NULL ) {
    parent->dead_children_head = soul;
    parent->dead_children_tail = soul;
  } else {
    parent->dead_children_tail->next = soul;
    parent->dead_children_tail       = soul;
  }

  return soul;
}

int KillProcess(pcb_t* proc, queue_t* QUEUE_ptr, int exit_status) {
  if( proc->pid == 1 ) {
    TracePrintf(TRACE_CRITICAL, "KillProcess(): Initial process exits. OS halts.\n" TRACE_N
		"  Return value: %d\n", exit_status);
    Halt();
  } else {
    TracePrintf(TRACE_CRITICAL, "KillProcess(): killing process #%d.\n", proc->pid);
    // Is my parent waiting for me?
    WakeUpWaitingParent(proc->ppid);
    
    RemoveFromQueue(proc, QUEUE_ptr);
    KillPCB(proc, exit_status);

    // Now run another program if RUNNING.head has been killed.
    if( RUNNING.head == NULL ) {
      if( READY.head == NULL ) { // This should never happen.
	TracePrintf(TRACE_WRONG, "No process available to run.\n");
	Halt();
      } else {
	ContextSwitch(READY.head, &READY, NULL);
      }
    }
  }
  return SUCCESS;
}

int IsInvalidIID(int id) {
  // Invalid iid.
  if( id < 0 ) {
    return 1;
  }

  // Iid is not registered.
  if( id > iid_count ) {
    WARN_USER("IID %d > than iid count\n", id);
    return 1;
  }

  // Iid has been freed.
  if( NULL == interp_array[id] ) {
    WARN_USER("IID %d has been freed\n", id);
    return 1;
  }

  return 0;
}

// End of Utility.c
