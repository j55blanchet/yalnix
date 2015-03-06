// KernelStart.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Implements KernelStart().

#include "DataStructures.h"
#include "KernelGlobals.h"
#include "Traps.h"
#include "Utility.h"

#define DEFAULT_PROCESS "InitProcess"
#define IDLE_PROCESS    "IdleProcess"

void KernelStart(char** cmd_args, unsigned int pmem_size, UserContext* u_context) {
  TracePrintf(TRACE_COMMENT, "Executing KernelStart()...\n");

  // This variable is 0 if InitProcess is not running. In this case, we load InitProcess.
  // Otherwise (when we return to KernelStart by manipulating program counter),
  // InitProcess is already running, so we load IdleProcess.
  static int init_process_running = 0;

  // Check arguments to determine the first process.
  // This variable is 1 if no argument specifies the initial process. In this case, we load InitProcess.
  // Otherwise some specified user program replaces InitProcess.
  int default_process;
  if( cmd_args[0] == NULL ) {
    default_process = 1;
    TracePrintf(TRACE_COMMENT, "KernelStart(): default process will run.\n");
  } else {
    default_process = 0;
    TracePrintf(TRACE_COMMENT, "KernelStart(): argument process will run: %s.\n", cmd_args[0]);
  }
  
  // Initialize global variables.
  PMEM_SIZE = pmem_size;
  TracePrintf(TRACE_VERBOSE, "KernelStart(): PMEM_SIZE: 0x%x\n", PMEM_SIZE);
  // In particular, malloc things before frame tables are examined.
  frame_table     = (fte_t*)calloc(sizeof(fte_t), FRAME_TABLE_SIZE);
  assert(frame_table);
  r0_page_table   = (pte_t*)calloc(sizeof(pte_t), R0_PAGE_TABLE_SIZE);
  assert(r0_page_table);
  pcb_t* init_pcb = (pcb_t*)calloc(sizeof(pcb_t), 1);
  assert(init_pcb);
  pcb_t* idle_pcb = (pcb_t*)calloc(sizeof(pcb_t), 1);
  assert(idle_pcb);
  pcb_array = (pcb_t**)calloc(sizeof(pcb_t*), pcb_array_size);
  assert(pcb_array);

  // also, initize all queues
  InitQueue(&READY);
  InitQueue(&RUNNING);
  InitQueue(&SLEEPING);
  InitQueue(&WAITING);

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
    for(i = 0; i < TRAP_VECTOR_SIZE/*16*/; i++) {
      // Undefined traps point to this handler which will abort the process.
      if (trap_vector_table[i] == NULL)
	trap_vector_table[i] = &HandleTrapUndefined;
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
  { int i;
    for(i = 0; i < FRAME_TABLE_SIZE; i++) {
      if(i < frame_addr_to_id(DOWN_TO_PAGE(KERNEL_DATA_START)) ) {
	// Kernel text segment.
	frame_table[i].valid = 1;
	frame_table[i].prot  = PROT_READ | PROT_EXEC;
	frames_used++;
      } else if( i < frame_addr_to_id(UP_TO_PAGE(KERNEL_DATA_END)) ) {
	// Kernel data segment.
	frame_table[i].valid = 1;
	frame_table[i].prot  = PROT_READ | PROT_WRITE;
	frames_used++;
      } else if( i < frame_addr_to_id(kernel_break) ) {
	// Kernel heap segment (parts used already).
	frame_table[i].valid = 1;
	frame_table[i].prot  = PROT_READ | PROT_WRITE;
      } else if( i < frame_addr_to_id(KERNEL_STACK_BASE) /*hardware.h*/)  {
	// Kernel heap segment (parts not used yet).
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

  // Build virtual memory page table for region 0. (directly mapped to physical memory)
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

  // Enable virtual memory.
  // (Documentation 3.2.4., 3.2.6.).
  WriteRegister(REG_PTBR0, (unsigned int) r0_page_table);
  WriteRegister(REG_PTLR0, R0_PAGE_TABLE_SIZE);
  WriteRegister(REG_PTBR1, (unsigned int) init_pcb->r1_page_table);
  WriteRegister(REG_PTLR1, R1_PAGE_TABLE_SIZE);
  WriteRegister(REG_VM_ENABLE, 1);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  TracePrintf(TRACE_COMMENT, "KernelStart(): virtual memory enabled.\n");
  TracePrintf(TRACE_VERBOSE,
	      "KernelStart():\n"  TRACE_N
	      "  REG_PTBR0: %p\n" TRACE_N
	      "  REG_PTLR0: %d\n" TRACE_N
	      "  REG_PTBR1: %p\n" TRACE_N
	      "  REG_PTLR1: %d\n", 
	      (void*) ReadRegister(REG_PTBR0), ReadRegister(REG_PTLR0),
	      (void*) ReadRegister(REG_PTBR1), ReadRegister(REG_PTLR1));

  // Flush TLB. (again)
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

  // Create and run init process.
  // Arguments to init is passed from cmd_args.
  {
    init_pcb = InitPCB(-1);

    // InitProcess shall be the first running process.
    AddToQueue(init_pcb, &RUNNING);

    int i;
    for(i = 0; i < R0_STACK_PAGE_TABLE_SIZE; i++) {
      init_pcb->r0_stack_page_table[i].valid = 1;
      init_pcb->r0_stack_page_table[i].prot  = PROT_READ | PROT_WRITE;
      init_pcb->r0_stack_page_table[i].pfn   = frame_addr_to_id(KERNEL_STACK_BASE) + i;
      if( idle_pcb->r0_stack_page_table[i].pfn == ERROR ) {
	TracePrintf(TRACE_WRONG, "KernelStart(): cannot find free frame for InitProcess.\n");
	Halt();
      }
    }
  }
  TracePrintf(TRACE_COMMENT, "KernelStart(): kernel stack page table written for first process.\n");

  // Idle process.
  {
    idle_pcb = InitPCB(-1);

    // idle will be in line to run (but init will run first)
    AddToQueue(idle_pcb, &READY);

    int i;
    for(i = 0; i < R0_STACK_PAGE_TABLE_SIZE; i++) {
      idle_pcb->r0_stack_page_table[i].valid = 1;
      idle_pcb->r0_stack_page_table[i].prot  = PROT_READ | PROT_WRITE;
      idle_pcb->r0_stack_page_table[i].pfn   = FindFreeFrame(PROT_READ | PROT_WRITE); // this creates a new kernel stack for idle
      if( idle_pcb->r1_page_table[i].pfn == ERROR ) {
	TracePrintf(TRACE_WRONG, "KernelStart(): cannot find free frame for IdleProcess.\n");
	Halt();
      }
    }
	
    // JBB. Here, we will make a special call to KernelContextSwitch(), in order to get
    // a KernelContext to save into the idle_pcb. We need some way to indicate to MY_KCS
    // that this is a special call (and is not to be treated normally). Maybe we give it
    // a special address (eg, a void* for 2nd PCB of 1, for example??)
    //
    
    // copy init_pcb's stack (the current stack) into idle_pcb's stack (currently empty)
    CopyKernelStack(idle_pcb, init_pcb);
    
    // Copies kernel state into the idle process's pcb
    LoadKernelContext(idle_pcb);
    
    // now, we need to conditionally transform ourselves into either init process or idle process
    if (!init_process_running) {
      // start init process
      TracePrintf(TRACE_COMMENT, "KernelStart(): Init process continuing in KernelStart\n");
      
      // Load program.
      int rv;
      if( default_process ) {
	rv = LoadProgram(DEFAULT_PROCESS, cmd_args /* cmd_args[0] == NULL */, init_pcb);
      } else {
	rv = LoadProgram(cmd_args[0], cmd_args, init_pcb);
      }
      // JHL. What do we do if LoadProgram() fails?
      // It shouldn't but...
      if( rv == ERROR ) {
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "KernelStart(): LoadProgram() for init returns ERROR\n");
	Halt();
      } else if( rv == KILL ) {
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "KernelStart(): LoadProgram() for init returns KILL\n");
	Halt();
      }
      
      // copy the init proc user context into the stack so we actually run init!
      memcpy(u_context, &init_pcb->u_context, sizeof(UserContext));

      // JHL. Why is changing address space necessary here?
      ChangeAddressSpace(init_pcb);
      
      init_process_running = 1;
      TracePrintf(TRACE_COMMENT, "KernelStart(): returning to init process!\n");

      // JHL.
      // Immediately initialize the Idle Process?
      ContextSwitch(READY.head, &READY, &READY);
    } else {
      // start idle process (we need to copy the kernel stack now)
      TracePrintf(TRACE_COMMENT, "KernelStart(): Idle process continuing in KernelStart\n");
      
      // Change address space to IdleProcess's. (includes kernel stack)
      ChangeAddressSpace(idle_pcb);
      
      // Load program.
      int rv;
      char* args[1];
      args[0] = NULL;

      rv = LoadProgram(IDLE_PROCESS, args, idle_pcb);
      // JHL. What do we do if LoadProgram() fails?
      // It shouldn't but...
      if( rv == ERROR ) {
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "KernelStart(): LoadProgram() for idle returns ERROR\n");
	Halt();
      } else if( rv == KILL ) {
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "KernelStart(): LoadProgram() for idle returns KILL\n");
	Halt();
      }
      
      // save the user context generated via LoadProgram into the stack (so that we run it!)
      memcpy(u_context, &idle_pcb->u_context, sizeof(UserContext));
      ChangeAddressSpace(idle_pcb);
      
      TracePrintf(TRACE_COMMENT, "KernelStart(): returning to idle process!\n");
    } 
  }
  
  TracePrintf(TRACE_COMMENT, "Leaving KernelStart()...\n");
}

// End of KernelStart.c
