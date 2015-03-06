// Utility.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Utility functions.

#include "Utility.h"

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
  if (index >= FRAME_TABLE_SIZE || frame_table[index].valid == 0)
    return ERROR;
  else
    frame_table[index].valid = 1;
}

void InitQueue(queue_t* QUEUE){
  QUEUE->size = 0;
  QUEUE->head = '\0';
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
    segfault();
    Halt();
  } else if( QUEUE->head != proc) {
    pcb_t* item;
    for(item = QUEUE->head->next; item != QUEUE->head; item = item->next) {
      if( item == proc ) {
	goto proc_in_queue;
      }
    }
    TracePrintf(TRACE_WRONG, "RemoveFromQueue(): cannot find process from queue.\n");
    Halt();
  }

 proc_in_queue:

  proc->next->prev = proc->prev;
  proc->prev->next = proc->next;
  if( QUEUE->head == proc ) {
    QUEUE->head = NULL;
  }

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
  printf("  code   = 0x%x(%s)\n", c->code  , s_code);
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
    default:
      s_code = "Undefined";
      break;
    }
    break;
  case TRAP_CLOCK:
    s_vector = "TRAP_CLOCK";
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
    break;
  case TRAP_TTY_RECEIVE:
    s_vector = "TRP_TTY_RECEIVE";
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

// End of Utility.c
