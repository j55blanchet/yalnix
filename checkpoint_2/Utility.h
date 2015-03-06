// Utility.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Header for utility functions.

#ifndef UTILITY_H
#define UTILITY_H

#include "KernelGlobals.h"

// Test return value from malloc().
#define assert(ptr) { \
    if( ptr == NULL ) { \
      TracePrintf(TRACE_CRITICAL, "Fatal error: malloc() returns NULL to " #ptr " in %s:%d.\n", __FILE__, __LINE__); \
      Halt(); \
    } \
  }

// Print user context.
void PrintUserContext(UserContext*);
void TraceUserContext(int /*level*/, UserContext*);

// Returns the index of a free frame.
// This frame will be marked as used.
// Returns ERROR upon failure.
int FnidFreeFrame(int PROT_CODE /*3-bit protection type code in fte_t*/);
// Precondition: frame_table has been initialized.

#endif
// End of Utility.h
