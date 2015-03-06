// KernelStart.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Implements KernelStart().

#include "DataStructures.h"
#include "KernelGlobals.h"
#include "Traps.h"
#include "Utility.h"

// Idle process.
void IdleProcess(void) {
  TracePrintf(TRACE_COMMENT, "Executing idle proc...\n");
  int i = 1;
  while(i) {
    printf("idle proc: loop %d\n", i); // JHL. Just to make it a little more visual.
    TracePrintf(TRACE_COMMENT, "idle proc: loop %d\n", i++);
    Pause();
  }
}

void KernelStart(char** cmd_args, unsigned int pmem_size, UserContext* u_context)
{
  TracePrintf(TRACE_COMMENT, "Executing KernelStart()...\n");

  // Check the validity of arguments (cmd_args).
  //
  // TODO.
  //
  // JHL. Or do we simply pass these to init proc and let it fail if invalid arguments?
  
  // Initialize global variables.
  PMEM_SIZE = pmem_size;
  TracePrintf(TRACE_VERBOSE, "KernelStart():\n" TRACE_N
	      "  PMEM_SIZE: 0x%x\n", PMEM_SIZE);
  // In particular, malloc things before frame tables are examined.
  frame_table     = (fte_t*)malloc(sizeof(int) * FRAME_TABLE_SIZE);
  assert(frame_table);
  r0_page_table   = (pte_t*)malloc(sizeof(pte_t) * R0_PAGE_TABLE_SIZE);
  assert(r0_page_table);
  process_running = (pcb_t*)malloc(sizeof(pcb_t));
  assert(process_running);

  // Initialize the trap vector table.
  // Initialize the register pointer for trap vector table.
  trap_vector_table[TRAP_KERNEL      /*0*/] = &HandleTrapKernel;
  trap_vector_table[TRAP_CLOCK       /*1*/] = &HandleTrapClock;
  trap_vector_table[TRAP_ILLEGAL     /*2*/] = &HandleTrapIllegal;
  trap_vector_table[TRAP_MEMORY      /*3*/] = &HandleTrapMemory;
  trap_vector_table[TRAP_MATH        /*4*/] = &HandleTrapMath;
  trap_vector_table[TRAP_TTY_RECEIVE /*5*/] = &HandleTrapTtyReceive;
  trap_vector_table[TRAP_TTY_TRANSMIT/*6*/] = &HandleTrapTtyTransmit;
  { int i;
    for(i = TRAP_TTY_TRANSMIT + 1; i < TRAP_VECTOR_SIZE/*16*/; i++) {
      // Undefined traps point to this handler which will abort the process.
      trap_vector_table[i] = *HandleTrapUndefined;
    }
  }
  WriteRegister(REG_VECTOR_BASE, (unsigned int) trap_vector_table);
  TracePrintf(TRACE_COMMENT, "KernelStart(): trap vector table initialized.\n");
  TracePrintf(TRACE_VERBOSE, "               (at %p)\n", (void*) ReadRegister(REG_VECTOR_BASE));

  // Create a list of free frames.
  // Frames already in use should not be included (i.e., between KERNEL_DATA_START and KERNEL_DATA_END).
  /* frame_table = (fte_t*)malloc(sizeof(int) * FRAME_TABLE_SIZE); */
  /* assert(frame_table); */
  int frames_used = 0;
  // JHL. It seems that it doesn't matter whether we allocate pmem address below x20000.
  // Machine wants x20000(16) and x22000(17) to be PROT_READ | PROT_EXEC.
  // Machine wants x24000 through x???? to be...
  // Why those numbers though?
  // I don't recall any documentation telling me about address x20000.
  // Maybe this is like x3000 in LC-3?
  //
  // What about text between x22000 and KERNEL_DATA_START? Are they executable?
  { int i;
    for(i = 0; i < FRAME_TABLE_SIZE; i++) {
      if(i < frame_addr_to_id(DOWN_TO_PAGE(KERNEL_TEXT_START))/*KernelGlobals.h*/ ) {
	// Unused segment.
	frame_table[i].valid = 0;
	frame_table[i].prot  = PROT_NONE;
      } else if(i < frame_addr_to_id(DOWN_TO_PAGE(KERNEL_DATA_START)) ) {
	// Kernel text segment.
	frame_table[i].valid = 1;
	frame_table[i].prot  = PROT_READ | PROT_EXEC;
	frames_used++;
      } else if( i < frame_addr_to_id(UP_TO_PAGE(KERNEL_DATA_END)) ) {
	// Kernel data segment.
	frame_table[i].valid = 1;
	frame_table[i].prot  = PROT_READ | PROT_WRITE;
	frames_used++;
      } else if( i < frame_addr_to_id(KERNEL_STACK_BASE) /*hardware.h*/)  {
	// Kernel heap segment (currently unused).
	frame_table[i].valid = 0;
	frame_table[i].prot  = PROT_NONE;
      } else if( i < frame_addr_to_id(KERNEL_STACK_LIMIT) /*hardware.h*/) {
	// Kernel stack segment.
	frame_table[i].valid = 1;
	frame_table[i].prot  = PROT_READ | PROT_WRITE;
      } else {
	// Region 1.
	frame_table[i].valid = 0;
	frame_table[i].prot  = PROT_NONE;
      }
    }
  }
  TracePrintf(TRACE_COMMENT, "KernelStart(): frame table built. %d frames in use.\n", frames_used);

  // JHL. Test print.
  /*
  {
    printf("frame_table: (PMEM_BASE = %p)", PMEM_BASE);
    int i;
    int j;
    for(i = 0, j = 0; i < FRAME_TABLE_SIZE; i++, j++) {
      if( j % 16 == 0 ) {
	printf("\n 0x%06x : ", frame_id_to_addr(i));
      }
      printf("%hhd ", frame_table[i].valid);
    }
    printf("\n");
  }
  */
  // JHL. End.

  // Build virtual memory page table for region 0.
  /* r0_page_table = (pte_t*)malloc(sizeof(pte_t) * R0_PAGE_TABLE_SIZE); */
  /* assert(r0_page_table); */
  { int i;
    for(i = 0; i < R0_PAGE_TABLE_SIZE; i++) {
      r0_page_table[i].valid = frame_table[i].valid;
      r0_page_table[i].prot  = frame_table[i].prot;
      if( r0_page_table[i].valid == 1 ) {
	r0_page_table[i].pfn = i;
      } else {
	r0_page_table[i].pfn = 0;
      }
    }
  }
  TracePrintf(TRACE_COMMENT, "KernelStart(): page table for region 0 written.\n");

  // JHL. Test print.
  /*
  {
    printf("%p\n", KERNEL_DATA_START);
    printf("%p\n", DOWN_TO_PAGE(KERNEL_DATA_START));
    printf("%u\n", frame_addr_to_id(KERNEL_DATA_START));
    printf("%u\n", frame_addr_to_id(DOWN_TO_PAGE(KERNEL_DATA_START)));

    printf("\nr0_page_table: (VMEM_0_BASE = %p)\n", VMEM_0_BASE);
    int i;
    for(i = 0; i < R0_PAGE_TABLE_SIZE; i++) {
      printf(" Addr[0x%06x] Valid[%u] Prot[%u] PFN[%u]\n", r0_id_to_addr(i),
	     r0_page_table[i].valid, r0_page_table[i].prot, r0_page_table[i].pfn);
    }
    int j;
    for(i = 0, j = 0; i < R0_PAGE_TABLE_SIZE; i++, j++) {
      if( j % 16 == 0 ) {
	printf("\n 0x%06x : ", r0_id_to_addr(i));
      }
      printf("%03d ", r0_page_table[i].pfn);
    }
    printf("\n");
  }
  */
  // JHL. End.

  // Build a PCB for idle process.
  // 
  // Build it as the running process.
  /* process_running = (pcb_t*)malloc(sizeof(pcb_t)); */
  /* assert(process_running); */
  process_running->pid    = NEXT_PID;
  process_running->ppid   = -1; // No parent process.
  process_running->status = 0;
  process_running->ticks  = 0;
  // Build virtual memory page table.
  { int i;
    for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
      process_running->r1_page_table[i].valid = 0;
      process_running->r1_page_table[i].prot  = PROT_NONE;
      process_running->r1_page_table[i].pfn   = 0;
    }
    for(i = 0; i < R0_STACK_PAGE_TABLE_SIZE; i++) {
      process_running->r0_stack_page_table[i].valid = r0_page_table[r0_addr_to_id(KERNEL_STACK_BASE) + i].valid;
      process_running->r0_stack_page_table[i].prot  = r0_page_table[r0_addr_to_id(KERNEL_STACK_BASE) + i].prot;
      process_running->r0_stack_page_table[i].pfn   = r0_addr_to_id(KERNEL_STACK_BASE) + i;
    }
  }
  TracePrintf(TRACE_COMMENT, "KernelStart(): page table for region 1 for idle proc written.\n");
  process_running->r1_stack_base_index  = VMEM_1_LIMIT;
  process_running->r1_break_limit_index = VMEM_1_BASE;
  process_running->u_context = u_context;
  //process_running->k_context; // JHL. Leave this alone?
  process_running->prev = process_running;
  process_running->next = process_running;
  // Set pc to the idle process (or what is pretending to be a process).
  u_context->pc = &IdleProcess;
  // Create a stack and set sp to it.
  {
    int new_frame_id = FindFreeFrame(PROT_ALL);
    if( new_frame_id < 0 ) {
      TracePrintf(TRACE_CRITICAL, "KernelStart(): no free frame is found for the first process.\n");
      Halt();
    }
    process_running->r1_page_table[r1_addr_to_id(VMEM_1_LIMIT) - 1].valid = 1;
    process_running->r1_page_table[r1_addr_to_id(VMEM_1_LIMIT) - 1].prot  = frame_table[new_frame_id].prot;
    process_running->r1_page_table[r1_addr_to_id(VMEM_1_LIMIT) - 1].pfn   = new_frame_id;
  }
  u_context->sp = (void*) VMEM_1_LIMIT - INITIAL_STACK_SIZE/*KernelGlobals.h, not hardware.h*/;

  // Enable virtual memory.
  // (Documentation 3.2.4., 3.2.6.).
  WriteRegister(REG_PTBR0, (unsigned int) r0_page_table);
  WriteRegister(REG_PTLR0, R0_PAGE_TABLE_SIZE);
  WriteRegister(REG_PTBR1, (unsigned int) process_running->r1_page_table);
  WriteRegister(REG_PTLR1, R1_PAGE_TABLE_SIZE);
  WriteRegister(REG_VM_ENABLE, 1);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  TracePrintf(TRACE_COMMENT, "KernelStart(): virtual memory enabled.\n");
  TracePrintf(TRACE_VERBOSE,
	      "KernelStart():\n" TRACE_N
	      "  REG_PTBR0: %p\n" TRACE_N
	      "  REG_PTLR0: %d\n" TRACE_N
	      "  REG_PTBR1: %p\n" TRACE_N
	      "  REG_PTLR1: %d\n", 
	      (void*) ReadRegister(REG_PTBR0), ReadRegister(REG_PTLR0),
	      (void*) ReadRegister(REG_PTBR1), ReadRegister(REG_PTLR1));

  // Create and run init process.
  // Arguments to init is passed from cmd_args.
  //
  // TODO (for checkpoint 3).
  //
  // JHL. Or become the first process? (Documentation 4.4.).
  // Use u_context to construct UserContext structs for idle and init.
  // How?

  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  TracePrintf(TRACE_COMMENT, "Leaving KernelStart()...\n");
}

// End of KernelStart.c
