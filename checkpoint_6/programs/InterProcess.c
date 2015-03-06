// InterProcess.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Tests inter-process synchronization and communication operations.

#include "programs/UserUtility.h"

#define BUFFER_SIZE 64
#define MESSAGE     "hello world"

void lock_and_delay(int lock) {
  Acquire(lock);
  Delay(3);
  Release(lock);
}

void pipe_read(int pipe, char* buffer) {
  puts("InterProcess-pipe_read: I will try to read.\n");
  int rv = PipeRead(pipe, buffer, BUFFER_SIZE);
  putsArgs("InterProcess-pipe_read: I read, '%s' (rv==%d).\n", buffer, rv);
}

void pipe_write(int pipe) {
  puts("InterProcess-pipe_write: I will try to write.\n");
  int rv = PipeWrite(pipe, MESSAGE, strlen(MESSAGE));
  putsArgs("InterProcess-pipe_write: I write, '%s' (rv==%d).\n", MESSAGE, rv);
}

int main(void) {
  puts("InterProcess is running...\n");

  int lock;
  if( SUCCESS != LockInit(&lock) ) {
    puts("InterProcess: I failed to create a lock.\n");
    Exit(-1);
  }
  putsArgs("InterProcess: lock's ID is %d.\n", lock);

  int pipe;
  if( SUCCESS != PipeInit(&pipe) ) {
    puts("InterProcess: I failed to create a pipe.\n");
    Exit(-2);
  }
  
  char buffer[BUFFER_SIZE];
  memset(buffer, '\0', BUFFER_SIZE);

  puts("InterProcess: I will Fork() into two process.\n");

  if( 0 == Fork() ) {
    pipe_read(pipe, buffer);
    Exit(0);
  }

  Delay(1);

  pipe_write(pipe);

  //  puts("InterProcess: I will try to acquire my lock.\n");
  //  Acquire(lock);
  //  puts("InterProcess: I have acquired my lock.\n");
  //  Release(lock);

  WaitAll();

  puts("InterProcess: I will free my resources.\n");
  Reclaim(lock);
  Reclaim(pipe);

  puts("InterProcess is exiting...\n");
  Exit(0);
}

// End of InterProcess.c
