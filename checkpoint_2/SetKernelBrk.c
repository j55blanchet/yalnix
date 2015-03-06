// SetKernelBreak.c
//
// Julien Blanchet and Jae Heon Lee.
//
// (Documentation 4.3.).

#include "KernelGlobals.h"

int SetKernelBrk (void* addr)
{
  printf("SetKernelBrk(addr=%p)\n", addr);

  if( ReadRegister(REG_VM_ENABLE) ) {
    TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL, "SetKernelBrk virtual memory functionality is unimplemented\n");

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
  }

  return 0;
}

// End of SetKernelBreak.c
