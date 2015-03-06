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

#define here() { \
  TracePrintf(TRACE_COMMENT, "******** %s:%d ********\n", __FILE__, __LINE__); \
  }

// Print user context.
void PrintUserContext(UserContext*); // JHL. Don't use this. Use TracePrintf() version of this.
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

// Check to make sure pointer is mapped to valid memory in region 1 and has desired PROTECTION
int CheckUserPointer(void* ptr, int PROTECTION);

// Return if entirity of string is within valid memory, checking at most max_bytes
// note: if max_bytes is -1, then there is no limit to string length
int CheckUserString(char* string, int desired_protection, int max_bytes);

// Changes the address space into the new process's page table.
void ChangeAddressSpace(pcb_t* new_process);

void InitQueue(queue_t* QUEUE);
int AddToQueue(pcb_t*, queue_t*);
int RemoveFromQueue(pcb_t*, queue_t*);

void PrintQueueHelper(queue_t* QUEUE);
void PrintAllQueues(void);

#define PrintQueue(QUEUE_ptr) { \
  TracePrintf(TRACE_VERBOSE, "QUEUE '%s':\n", #QUEUE_ptr); \
  PrintQueueHelper(QUEUE_ptr); \
  }

soul_t* KillPCB(pcb_t*, int status);
pcb_t* InitPCB(int ppid); // PCB is initialized.
void RegisterPCB(pcb_t*); // pcb_array[pid] is set to PCB's address.

#endif
// End of Utility.h
