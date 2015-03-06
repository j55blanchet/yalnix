// ContextSwitch.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Handles context switching between two processes (within the kernel mode).
//
// Relevant documentation: 5.1., hardware.h:377-423.

#include "ContextSwitch.h"
#include "KernelGlobals.h"

// This function corresponds to MyKCS in documentation.
KernelContext* ContextSwitchHelper(KernelContext* k_context, void*/* pcb_t* */_current, void*/* pcb_t* */_next) {
  pcb_t* current = (pcb_t*) _current;
  pcb_t* next    = (pcb_t*) _next;

  // 5.1.2.: copy the kernel context into the current process's PCB and return a pointer
  // to a kernel context it had earlier saved in the next process's PCB.
  //
  // hardware.h:
  // The "KernelContext *" returned by MyKCS should point to a copy of the kernel context
  // of the new process to run.
  //
  // JHL. A copy of? Should we malloc() a new KernelContext and memcpy() next's
  // KernelContext?

  // JHL. WORK IN PROGRESS.

  // save current proc's kernel context
  memcpy(&current->k_context, k_context, sizeof(KernelContext));

  ChangeAddressSpace(next);

  // Return the next's frozen kernel context memory.
  memcpy(k_context, &next->k_context, sizeof(KernelContext));

  return &next->k_context;
}



// Currently running process is appended to the end of queue TO.
// next starts running.
void ContextSwitch(pcb_t* next, queue_t* FROM, queue_t* TO) {
  pcb_t* current = RUNNING.head;

  RemoveFromQueue(next, FROM);
  RemoveFromQueue(current, &RUNNING);
  AddToQueue(current, TO);
  AddToQueue(next, &RUNNING);

  TracePrintf(TRACE_VERBOSE, "Calling KernelContextSwitch()...\n");
  if( ERROR == KernelContextSwitch(ContextSwitchHelper, current, next) ) {
    TracePrintf(TRACE_WRONG, "ContextSwitch(): error in KernelContextSwitch().\n");
    Halt();
  }

  TracePrintf(TRACE_VERBOSE, "Leaving ContextSwitch()...\n");
}

// Helper Function to embed the current kernelContext into the pcb (given as first argument)
KernelContext*
GetKernelContextHelper
(KernelContext* k_context, void*/*pcb_t**/ proc, void*/*pcb_t**/ null) {
  memcpy(&((pcb_t*)proc)->k_context, k_context, sizeof(KernelContext));
  return k_context;
}

KernelContext*
CopyKernelStackHelper
(KernelContext* k_context, void*/*pcb_t**/ _target_idle, void*/*pcb_t**/ _source_init) {
  
  pcb_t* target_idle = (pcb_t*) _target_idle;
  pcb_t* source_init = (pcb_t*) _source_init;

  // copy the kernel stack
  unsigned char kernel_stack_buffer[R0_STACK_PAGE_TABLE_SIZE * PAGESIZE];
  memcpy(kernel_stack_buffer, KERNEL_STACK_BASE, R0_STACK_PAGE_TABLE_SIZE * PAGESIZE);
  ChangeAddressSpace(target_idle);
  memcpy(KERNEL_STACK_BASE, kernel_stack_buffer, R0_STACK_PAGE_TABLE_SIZE * PAGESIZE);
  ChangeAddressSpace(source_init);

  return k_context;
}

void CopyKernelStack(pcb_t* target_idle, pcb_t* source_init){
  if( ERROR == KernelContextSwitch(CopyKernelStackHelper, target_idle, source_init) ) {
    TracePrintf(TRACE_WRONG, "CopyKernelStack(): error in KernelContextSwitch().\n");
    Halt();
  }
}

void LoadKernelContext(pcb_t* proc) {
  if( ERROR == KernelContextSwitch(GetKernelContextHelper, proc, NULL) ) {
    TracePrintf(TRACE_WRONG, "LoadKernelContext(): error in KernelContextSwitch().\n");
    Halt();
  }
}

// End of ContextSwitch.c
