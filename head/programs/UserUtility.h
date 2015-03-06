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

// TEST FUNCTION.
int TtyPrint(int tty_id, char* /*NULL-terminated*/ string /* Up to 103 chars */ ) {
  char buffer[128] = "*** Process #___>>";
  //                  012345678901234567
  //                               ^^^
  int pid = GetPid();
  buffer[13] = (pid / 100) + '0';
  buffer[14] = (pid / 10)  + '0';
  buffer[15] = (pid % 10)  + '0';

  int buffer_length = 0;
  { int i;
    for(i = 0; buffer[i] != '\0'; i++) {
      buffer_length++;
    }
  }

  int string_length = 0;
  { int i;
    for(i = 0; string[i] != '\0'; i++) {
      string_length++;
    }
  }

  { int i;
    for(i = buffer_length; i < buffer_length + string_length; i++) {
      buffer[buffer_length + i] = string[i];
    }
    buffer[buffer_length + string_length] = '\0';
  }

  TracePrintf(TRACE_USERLAND, buffer);
  TtyWrite(tty_id, buffer, buffer_length + string_length);

  return SUCCESS;
}

#endif
// End of UserUtility.h
