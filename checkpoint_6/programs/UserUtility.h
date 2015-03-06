// UserUtility.h
//
// Julien Blanchet and Jae Heon Lee.

#ifndef USER_UTILITY_H
#define USER_UTILITY_H

#include "include/hardware.h"
#include "KernelGlobals.h"

#define PUTS_NAME "Process #%d >> "

#define puts(string) \
    TracePrintf(TRACE_USERLAND, PUTS_NAME string, GetPid())
#define putsArgs(string, ...) \
    TracePrintf(TRACE_USERLAND, PUTS_NAME string, GetPid(), __VA_ARGS__)


#define panic(string) ({ \
TracePrintf(TRACE_USERLAND, PUTS_NAME string, GetPid()); \
Exit(-1);\
})


#define WaitAll() {				\
    int status;					\
    while(ERROR != Wait(&status));		\
  }

#endif
// End of UserUtility.h
