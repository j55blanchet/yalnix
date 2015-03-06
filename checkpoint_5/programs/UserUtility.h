// UserUtility.h
//
// Julien Blanchet and Jae Heon Lee.

#include "include/hardware.h"
#include "KernelGlobals.h"

#define PUTS_NAME "Process #%d >> "

#define puts(string)              TracePrintf(TRACE_USERLAND, PUTS_NAME string, GetPid())
#define puts1(string, arg1)       TracePrintf(TRACE_USERLAND, PUTS_NAME string, GetPid(), arg1)
#define puts2(string, arg1, arg2) TracePrintf(TRACE_USERLAND, PUTS_NAME string, GetPid(), arg1, arg2)

#define WaitAll() {				\
    int status;					\
    Wait(&status);				\
  }
