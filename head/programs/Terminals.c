// Terminals.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Driver for testing terminal I/O functions.

#include "hardware.h"
#include "KernelGlobals.h"

#define SHORT_STRING "hello world\n"
#define LONG_STRING							\
  "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" \
  "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111" \
  "22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222" \
  "33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333" \
  "44444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444" \
  "55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555" \
  "66666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666" \
  "77777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777" \
  "88888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888" \
  "99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999" \
  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
  "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" \
  "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc" \
  "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd" \
  "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" \
  "hello world\n"

int main2(void) {
  TracePrintf(TRACE_USERLAND, "Terminals is running...\n");

  char c;
  int rv = TtyRead(0, &c, 1);

  Exit(rv);
}

int main(void) {
  char string[2048] = "   >> " SHORT_STRING;

  TtyWrite(0, string, strlen(string));

  /*Fork();
  Fork();
  Fork();
  Fork();
  Fork();
  Fork();
  // Fork(); // This call causes OS to run out of frames. But why can't we handle this gracefully?

  int pid = GetPid();
  string[0] = pid / 10 + '0';
  string[1] = pid % 10 + '0';

  int rv = TtyWrite(0, string, strlen(string));

  int status;
  while( ERROR != Wait(&status) );
  if( GetPid() != 1 ) {
    Exit(rv);
  }

  char buffer[64];
  memset(buffer, '\0', sizeof(buffer));
  rv = TtyRead(0, buffer, sizeof(buffer));

  TracePrintf(TRACE_USERLAND, "Process #%d: tty reads, '%s'.\n", GetPid(), buffer);
    */
    
  Exit(1);
}

// End of Terminals.c
