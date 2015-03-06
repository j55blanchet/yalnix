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
  for(i = frame_addr_to_id(KERNEL_TEXT_START)/*16*//*lowest usable frame?*/; i < FRAME_TABLE_SIZE; i++) {
    if( frame_table[i].valid == 0 ) {
      frame_table[i].valid = 1;
      frame_table[i].prot  = PROT_CODE;
      return i;
    }
  }
  // At this point, frame table is full.

  TracePrintf(TRACE_SEVERE, "GetFreeFrame(): cannot find a free frame\n");
  return ERROR;
}

void PrintUserContextHelperHelper(UserContext* c, char* s_vector, char* s_code) {
  printf("UserContext\n");
  printf("  vector = %d(%s)\n", c->vector, s_vector);
  printf("  code   = %d(%s)\n", c->code  , s_code);
  printf("  addr   = %p\n",     c->addr);
  printf("  pc     = %p\n",     c->pc);
  printf("  sp     = %p\n",     c->sp);
#ifdef LINUX
  printf("  ebp    = %p\n",     c->ebp);
#endif
  { int i;
    for(i = 0; i < GREGS; i++) {
      printf("  regs[%i]=%x\n", i, c->regs[i]);
    }
  }  
}
void TraceUserContextHelperHelper(UserContext* c, char* s_vector, char* s_code, int level) {
  TracePrintf(level,
	      "UserContext\n" TRACE_N
	      "  vector = %d(%s)\n" TRACE_N
	      "  code   = %d(%s)\n" TRACE_N
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
      TracePrintf(level, "  regs[%i]=%x\n", i, c->regs[i]);
    }
  }
}
void PrintUserContextHelper(UserContext* c, int TracePrintf, int level) {
  char* s_vector;
  char* s_code;
  switch( c->vector ) {
  case TRAP_KERNEL:
    s_vector = "TRAP_KERNEL";
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
