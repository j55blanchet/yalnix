#include "programs/UserUtility.h"

#define HELLO "hello world"
#define BUFFER_SIZE 64

int grandchild(int pipe_to_child1) {
  char buffer[BUFFER_SIZE];
  memset(buffer, '\0', sizeof(char) * BUFFER_SIZE);

  PipeRead(pipe_to_child1, buffer, BUFFER_SIZE);
  putsArgs("Grandchild: I read '%s' from child1 to show that the pipe works.\n", buffer);

  puts("Grandchild: I will sleep for 5 seconds, then exit.\n");
  Delay(5);
}

int child1(int pipe_to_parent) {
  int pipe_to_grandchild;
  PipeInit(&pipe_to_grandchild);

  if( 0 == Fork() ) {
    Exit(grandchild(pipe_to_grandchild));
  }

  PipeWrite(pipe_to_parent, &pipe_to_grandchild, sizeof(int));
  putsArgs("Child1: parent, pipe I have with grandchild is %d.\n", pipe_to_grandchild);

  PipeWrite(pipe_to_grandchild, HELLO, strlen(HELLO));
  putsArgs("Child1: I said '%s' to grandchild to show that the pipe works.\n", HELLO);

  puts("Child1: I will sleep for 5 seconds, then exit.\n");
  Delay(5);

  Reclaim(pipe_to_grandchild);
  WaitAll();
}

int child2(int pipe_to_parent) {
  int pipe_to_nephew;
  
  PipeRead(pipe_to_parent, &pipe_to_nephew, sizeof(int));
  putsArgs("Child2: parent says that child1 talks to grandchild through pipe #%d.\n", pipe_to_nephew);

  char c;
  int r = PipeRead(pipe_to_nephew, &c, sizeof(char));
  putsArgs("Child2: when I try to read from that pipe, I get PipeRead() returns %d (buffer: '%c').\n", r, c);
}

int non_descendant_tries_to_read(void) {
  puts("Parent: I will demonstrate that non-descendnats cannot read from a pipe.\n");
  puts("Parent: Child 1 will bear a grandchild and talk to it with a pipe.\n");
  puts("Parent: Child 1 will also let me know of the pipe id.\n");
  puts("Parent: I will let child 2 know of that pipe id.\n");
  puts("Parent: Child 2 will try to use that pipe.\n");

  int pipe_to_child1;
  int pipe_to_child2;

  PipeInit(&pipe_to_child1);
  PipeInit(&pipe_to_child2);

  if( 0 == Fork() ) {
    Exit(child1(pipe_to_child1));
  }
  if( 0 == Fork() ) {
    Exit(child2(pipe_to_child2));
  }

  int pipe_between_child1_and_grandchild;
  PipeRead(pipe_to_child1, &pipe_between_child1_and_grandchild, sizeof(int));
  putsArgs("Parent: child1 says that it's talking to grandchild through pipe #%d.\n", pipe_between_child1_and_grandchild);
  puts("Parent: I will let child2 know of this.\n");
  PipeWrite(pipe_to_child2, &pipe_between_child1_and_grandchild, sizeof(int));

  puts("Parent: I will exit.\n");

  Reclaim(pipe_to_child1);
  Reclaim(pipe_to_child2);

  WaitAll();
  Exit(0);
}

int child_tries_to_reclaim(void) {
  int pipe;
  int lock;
  int cvar;
  PipeInit(&pipe);
  LockInit(&lock);
  CvarInit(&cvar);

  putsArgs("Parent: I made a pipe, a lock, and a cvar, id is %d, %d, and %d.\n", pipe, lock, cvar);

  if( 0 == Fork() ) {
    int r;
    
    r = Reclaim(pipe);
    putsArgs("Child: I tried to reclaim the pipe whose id is %d.\n", pipe);
    putsArgs("Child: I only got return value %d.\n", r);
    
    r = Reclaim(lock);
    putsArgs("Child: I tried to reclaim the pipe whose id is %d.\n", lock);
    putsArgs("Child: I only got return value %d.\n", r);

    r = Reclaim(cvar);
    putsArgs("Child: I tried to reclaim the pipe whose id is %d.\n", cvar);
    putsArgs("Child: I only got return value %d.\n", r);
    
    puts("Child: I will now exit.\n");
    Exit(r);
  }

  puts("Parent: I will exit.\n");
  
  Reclaim(pipe);
  WaitAll();
  Exit(0);
}

int main(void) {



  // Non-descendant will try to read from a pipe.
  puts("InterpCriminal: only a pipe's owner and its descendants can use pipes.\n");
  if( 0 == Fork() ) {
    non_descendant_tries_to_read();
  }
  WaitAll();

  // A child will try to reclaim a pipe.
  puts("InterpCriminal: only a interp's owner can reclaim the interp.\n");
  if( 0 == Fork() ) {
    child_tries_to_reclaim();
  }
  WaitAll();

  Exit(0);
}
