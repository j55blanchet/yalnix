// ContextSwitch.h
//
// Julien Blanchet and Jae Heon Lee.

#ifndef CONTEXT_SWITCH_H
#define CONTEXT_SWITCH_H

#include "DataStructures.h"
#include "KernelGlobals.h"
#include "Utility.h"

// Running process is appended to the end of queue.
// next starts running.
void ContextSwitch
(
 pcb_t*   next,      // Process which will run next.
 queue_t* NEXT_FROM, // Queue from which next comes.
 queue_t* THIS_TO    // Queue to which current will go.
 );

void LoadKernelContext(pcb_t* proc);
void CopyKernelStack(pcb_t* target_idle, pcb_t* source_init);
#endif
// End of ContextSwitch.h
