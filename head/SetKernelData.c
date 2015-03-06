// SetKernelData.c
//
// Julien Blanchet and Jae Heon Lee.

#include "KernelGlobals.h"

void SetKernelData(void* _KernelDataStart, void* _KernelDataEnd)
{
  TracePrintf(TRACE_COMMENT, "Executing SetKernelData()...\n");

  // Let the machine inform us about kernel's current memory usage.
  KERNEL_DATA_START = _KernelDataStart;
  KERNEL_DATA_END   = _KernelDataEnd;

  // Before use of heap, kernel_break is set at the same level as
  // KERNEL_DATA_END.
  kernel_break      = (void*) UP_TO_PAGE(_KernelDataEnd);

  TracePrintf(TRACE_VERBOSE,
	      "SetKernelData():\n" TRACE_N
	      "  KERNEL_DATA_START at %p\n" TRACE_N
	      "  KERNEL_DATA_END   at %p\n" TRACE_N
	      "  kernel_break      at %p\n",
	      KERNEL_DATA_START, KERNEL_DATA_END, kernel_break);

  TracePrintf(TRACE_COMMENT, "Leaving SetKernelData()...\n");
}

// End of SetKernelData.c
