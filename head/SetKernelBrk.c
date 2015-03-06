// SetKernelBreak.c
//
// Julien Blanchet and Jae Heon Lee.
//
// (Documentation 4.3.).

#include "KernelGlobals.h"
#include "Utility.h"

int SetKernelBrk (void* addr)
{
  TracePrintf(TRACE_COMMENT, "SetKernelBrk(): request address at %p, current kernel break at %p.\n", addr, kernel_break);

  if( ReadRegister(REG_VM_ENABLE) ) {
    here();
    here();
    here();

    if( (unsigned int) addr >= KERNEL_STACK_BASE ) {
      TracePrintf(TRACE_CRITICAL, "SetKernelBrk(): cannot grant memory at %p above KERNEL_STACK_BASE.\n", addr);
      Halt();
    } else if( addr < kernel_break ) {
      TracePrintf(TRACE_WRONG, "SetKernelBrk(): requested memory %p is below current kernel break %p.\n", addr, kernel_break);
      Halt();
    }
    
    { int i;
      int j = 0;
      for(i = r0_addr_to_id(kernel_break); i < r0_addr_to_id(addr); i++) {
	if( r0_page_table[i].valid ) { // This should not happen.
	  TracePrintf(TRACE_WRONG, "SetKernelBrk(): r0_page_table[%d] is already valid (request at %p).\n", i, addr);
	  Halt();
	}
	r0_page_table[i].valid = 1;
	r0_page_table[i].prot  = PROT_READ | PROT_WRITE;
	r0_page_table[i].pfn   = FindFreeFrame(r0_page_table[i].prot);
	if( ERROR == r0_page_table[i].pfn ) {
	  TracePrintf(TRACE_WRONG, "SetKernelBrk(): cannot grant memory request at %p: insufficient memory.\n", addr);
	  Halt();
	}
	j++;
      }
      kernel_break = (void*) UP_TO_PAGE(addr);
      TracePrintf(TRACE_COMMENT, "SetKernelBrk(): memory request at %p granted. %d frames allocated.  New kernel break at %p.\n", addr, j, kernel_break);
    }

  } else {
    // Virtual memory is not enabled yet.
    // (Documentation 4.3.1.).

    // Cannot fulfill memory request.
    // Comparison with KERNEL_DATA_END is required for DCS 58's first call to SetKernelBrk() before PMEM_SIZE is initialized.
    void* PMEM_LIMIT;
    if( (void*) PMEM_BASE + PMEM_SIZE > KERNEL_DATA_END ) {
      PMEM_LIMIT = (void*) PMEM_BASE + PMEM_SIZE;
    } else {
      PMEM_LIMIT = KERNEL_DATA_END;
    }
    if( addr > PMEM_LIMIT ) {
      TracePrintf(TRACE_SEVERE, "SetKernelBrk() cannot grant memory requested (%p) above KERNEL_DATA_END (%p) (return ERROR)\n",
    		  addr, PMEM_BASE + PMEM_SIZE);
      return ERROR;
    }

    // Here assume that malloc() will call this with valid argument.
    // Keep track of maximum extent passed on calls to SetKernelBrk().
    kernel_break = (void*) UP_TO_PAGE(addr);    

    TracePrintf(TRACE_VERBOSE, "SetKernelBrk() : completed. requested:%p\t new brk:%p\n", addr, kernel_break);
  }

  // JBB: malloc expects us to return the address of the new break.
  return 0;
}

// End of SetKernelBreak.c
