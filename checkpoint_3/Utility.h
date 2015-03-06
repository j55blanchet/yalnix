// Utility.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Header for utility functions.

#ifndef UTILITY_H
#define UTILITY_H

#include "KernelGlobals.h"

// Test return value from malloc().
// Halt program upon failure.
#define assert(ptr) { \
    if( ptr == NULL ) { \
      TracePrintf(TRACE_CRITICAL, "Fatal error: malloc() returns NULL to " #ptr " in %s:%d.\n", __FILE__, __LINE__); \
      Halt(); \
    } \
  }

#define segfault() { \
  char* segfault_ptr = NULL; \
  *segfault_ptr = *segfault_ptr; \
  }

// Print user context.
void PrintUserContext(UserContext*);
void TraceUserContext(int /*level*/, UserContext*);
void TraceRegs(int level, UserContext*);

// Returns the index of a free frame.
// This frame will be marked as used.
// Returns ERROR upon failure.
// Precondition: frame_table has been initialized.
int FindFreeFrame(int PROT_CODE /*3-bit protection type code in fte_t*/);

// Release a frame for later use
// return ERROR if the given index is invalid for freeing
int FreeFrame(int index);

// Changes the address space into the new process's page table.
void ChangeAddressSpace(pcb_t* new_process);

void InitQueue(queue_t* QUEUE);
int AddToQueue(pcb_t*, queue_t*);
int RemoveFromQueue(pcb_t*, queue_t*);

#endif
// End of Utility.h
