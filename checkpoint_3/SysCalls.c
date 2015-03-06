// SysCalls.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Implement syscall handlers (called from kernel_trap)
// See Traps.h for information.

#include "KernelGlobals.h"
#include "Traps.h"
#include "include/hardware.h"
#include "DataStructures.h"

int HandleDelay(int clock_ticks){
    if (clock_ticks < 0)
	return ERROR;
    if (clock_ticks == 0)
	return 0;

    pcb_t* cur_pcb = RUNNING.head;
    cur_pcb->ticks = clock_ticks;

    ContextSwitch(READY.head, &READY, &CLOCK_WAITING);
}

int HandleBrk(void* requested_addr){
  int pt_request_index = r1_addr_to_id(requested_addr);
  pcb_t* cur_pcb = RUNNING.head;
  int pt_br_cur_index = cur_pcb-> r1_break_limit_index;

  // if the user process already "owns" that memory, simply return the current break
  if (pt_request_index  <  pt_br_cur_index) {
    TracePrintf(TRACE_VERBOSE, "HandleBrk(): program already has %p.\n", requested_addr);
    return 0;
  }
  
  // cannot grow heap into, or within one page of, the stack
  else if (pt_request_index >= cur_pcb -> r1_stack_base_index - 1) {
    TracePrintf(TRACE_COMMENT, "HandleBrk(): cannot heap in stack, at %p.\n", requested_addr);
    return ERROR;
  }

  // if we've gotten to this point, it's a valid request and we need to grow the stack
  TracePrintf(TRACE_VERBOSE, "HandleBrk(): will grant %p.\n", requested_addr);

  // step 1: gather necessary free frames
  int frames_required = pt_request_index - pt_br_cur_index;
  int acquired_frames [frames_required];
  int num_acquired_frames = 0;
  while(num_acquired_frames < frames_required){
    int new_frame = FindFreeFrame(PROT_READ | PROT_WRITE);

    // JBB: in future, we should change this to gracefully free the frames we've acquired, than return -1
    if (new_frame == ERROR){
      TracePrintf(TRACE_CRITICAL, "HandleBrk: Insufficent memory to grow user heap\n");
      Halt();
    }
	
    acquired_frames [num_acquired_frames] = new_frame;
    num_acquired_frames++;
  }

  TracePrintf(TRACE_VERBOSE, "HandleBrk(): %d frames found for %p.\n", num_acquired_frames, requested_addr);

  //step 2: arrange ptes
  int i;
  for(i = 0; i < frames_required; i++){
    cur_pcb->r1_page_table[i + pt_br_cur_index + 1].valid = 1;
    cur_pcb->r1_page_table[i + pt_br_cur_index + 1].prot  = PROT_READ | PROT_WRITE;
    cur_pcb->r1_page_table[i + pt_br_cur_index + 1].pfn   = acquired_frames[i];
  }

  // set new break and return to user
  cur_pcb -> r1_break_limit_index = pt_br_cur_index + frames_required;
  TracePrintf(TRACE_VERBOSE, "HandleBrk(): new break at index %d, max addr %p.\n", 
	      cur_pcb->r1_break_limit_index,
	      UP_TO_PAGE((r1_id_to_addr(cur_pcb->r1_break_limit_index))));
  return 0;

} // end of HandleBrk
