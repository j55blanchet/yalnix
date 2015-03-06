// major_functions.c
//
// Julien Blanchet and Jae Heon Lee.

// Note. This is NOT a source file, at least at this point.
//
// Contains pseudocode for implementing major functions.
//
// Relevant documentation: 4.3., 4.4., include/hardware.h

extern void* KERNEL_DATA_START; // The lower limit for kernel.
extern void* KERNEL_DATA_END;   // The upper limit for kernel.
extern void* kernel_break;      //

extern int PMEM_SIZE; // Size of the physical memory.

struct pte* kernel_page_table;
void* trap_vector_table[TRAP_VECTOR_SIZE];

void
SetKernelData
(void* _KernelDataStart,
 void* _KernelDataEnd)
{
  // Be informed, from hardware, about kernel's memory usage.
  KERNEL_DATA_START = _KernelDataStart;
  KERNEL_DATA_END   = _KernelDataEnd;

  // Before use of heap, kernel break KERNEL_DATA_END + 0.
  kernel_break      = _KernelDataEnd;
}

void
KernelStart
(char** cmd_args,
 unsigned int pmem_size,
 UserContext* u_context)
{
  // Check for the validity of cmd_args.

  PMEM_SIZE = pmem_size;

  // Initialize interrupt vector table.      // Function pointers.
  trap_vector_table[TRAP_KERNEL     /*0*/] = &HandleTrapKernel;
  trap_vector_table[TRAP_CLOCK      /*1*/] = &HandleTrapClock;
  trap_vector_table[TRAP_ILLEGAL    /*2*/] = &HandleTrapIllegal;
  trap_vector_table[TRAP_MEMORY     /*3*/] = &HandleTrapMemory;
  trap_vector_table[TRAP_MATH       /*4*/] = &HandleTrapMath;
  trap_vector_table[TRAP_TTY_RECEIVE/*5*/] = &HandleTrapTtyReceive;
  trap_vector_table[TRAP_TTY_WRITE  /*6*/] = &HandleTrapTtyWrite;
  // trap_vector_table[TRAP_DISK       /*7*/] = *HandleTrapDisk;
  for(int i = TRAP_TTY_WRITE + 1; i < TRAP_VECTOR_SIZE; i++) {
    // Undefined traps point to this handler which will abort the process.
    trap_vector_table[i] = *HandleTrapUndefined;
  }
  // Initialize REG_VECTOR_BASE register to point to interrupt vector table.
  WriteRegister(REG_VECTOR_BASE, trap_vector_table /* interrupt vector table */);

  // Initialize all global variables, lists, etc.

  // Build a list of free frames.
  // Make sure not to include frames already in use.
  // JHL.
  // (pmem_size / PAGESIZE) frames total? (rounded down to multiple of PAGESIZE?)
  // Or things out of KERNEL_DATA_START and KERNEL_DATA_END?

  // Enable virtual memory.
  WriteRegister(REG_VM_ENABLE, 1); // Cf. 3.2.6.

  // Create idle process.
  // Should this be through Fork()?

  // Create the first process and run.
  // Or become the first process (Cf. 4.4.).
  // Arguments to this process is in cmd_args.

  // Use u_context to construct UserContext structs for idle and initial processes.

  return;
}

int
SetKernelBrk
(void* addr)
{
  if( /* virtual memory is enabled */ ) {

    if( /* has run out of memory */ ) {
      return -1 /* Cf. 4.3.2. */;
    } else {
      void* new_break = /*round up?*/ addr;
      
      // For each page containing address between VMEM_BASE == VMEM_0_BASE and addr,
      // Check that the pages are valid
      
      // Check that no pages outside this range (except stack kernel) are valid.

      // For all pages between the current break and new_break
      // find free frames, update pte as valid and pointing to the frame.

      kernel_break = new_break;

      return 0;
    }

  } else { // Virtual memory is not enabled yet

    void* new_break = /*round up?*/ addr;

    // Here assume that malloc() will call this with valid argument.
    // Keep track of maximum extent passed on calls to SetKernelBrk().

    kernel_break = new_break;
    
  }
}
