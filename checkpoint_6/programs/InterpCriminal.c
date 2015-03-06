


#include "programs/UserUtility.h"
#define IDK 2

void verifyError(int code, char* msg){
    if (code != ERROR){
        TracePrintf(TRACE_USERLAND, msg);
        Exit(ERROR);
    }
}


int main(void){
    int numTests = 0;

    // bad pointers while initializing locks, cvars, pipes
    numTests += 1;
    if(Fork() == 0){
        int rv;
        
        rv = PipeInit((int*) &main);
        verifyError(rv, "Succeeded in initializing pipe at main\n");
        
        rv = LockInit((int*) NULL);
        verifyError(rv, "Succeeded in initilized pipe at NULL\n");
        
        rv = CvarInit((int*) -81);
        verifyError(rv, "Succeeded in initializing Cvar at -81\n");
        
        puts("Passed test for bad pointers while initializing locks, cvars, pipes\n");
        Exit(SUCCESS);
    }
    
    // bad buffer pointers (applicable only to pipes)
    numTests += 1;
    if(Fork() == 0){
        int rv;
   
        int pipe;
        PipeInit(&pipe);
        
        rv = PipeRead(pipe, &Fork, 20);
        verifyError(rv, "Was able to PipeRead into a buffer at &Fork\n");
        
        rv = PipeWrite(pipe, NULL, 10);
        verifyError(rv, "Was able to PipeWrite from buffer at NULL\n");
        
        Reclaim(pipe);
        
        puts("Passed test for bad buffer pointers\n");
        Exit(SUCCESS);
    }
    
    // too long buffer length arguments
    numTests += 1;
    if (Fork() == 0){
        int rv;
        int pipe;
        char buffer[100];
        
        PipeInit(&pipe);
        
        rv = PipeRead(pipe, buffer, 4000);
        verifyError(rv, "Was able to PipeRead into a buffer with too short length\n");
        
        rv = PipeWrite(pipe, buffer, 4000);
        verifyError(rv, "Was able to PipeWrite from buffer with too short length\n");
    
        puts("Passed test for too long buffer length arguments\n");
        Exit(SUCCESS);
    }

    // unititialized locks, cvars, and pipes
    numTests += 1;
    if (Fork() == 0){
        int uninitialized = -1;
        char buf[10];
        int rv;
        
        rv = PipeRead(uninitialized, buf, sizeof(buf));
        verifyError(rv, "able to PipeRead uninit pipe\n");
        
        rv = PipeWrite(uninitialized, buf, sizeof(buf));
        verifyError(rv, "able to PipeWrite uninit pipe\n");
        
        rv = CvarSignal(uninitialized);
        verifyError(rv, "able to CvarSignal uninit cvar\n");
        
        rv = CvarBroadcast(uninitialized);
        verifyError(rv, "able to broadcase uninit cvar\n");
        
        {
            int lock; LockInit(&lock);
            int cvar; CvarInit(&cvar);
            
            rv = CvarWait(cvar, uninitialized);
            verifyError(rv, "able to cvarwait w/ uninit lock\n");
            
            rv = CvarWait(uninitialized, lock);
            verifyError(rv, "able to cvarwait uninit cvar\n");
            
            Reclaim(lock);
            Reclaim(cvar);
        }
        
        rv = Acquire(uninitialized);
        verifyError(rv, "able to acquire uninit lock\n");
        
        rv = Release(uninitialized);
        verifyError(rv, "able to release uninit lock\n");
        
        rv = Reclaim(uninitialized);
        verifyError(rv, "able to realcim uninit interp\n");
        
        puts("Passed test for unititialized locks, cvars, and pipes\n");
        Exit(SUCCESS);
    }
    
    numTests += 1;
    if (Fork() == 0){
        non_descendant_tries_to_read();
        puts("Passed test for descendant tries to read");
    }
    
    numTests += 1;
    if (Fork() == 0){
        child_tries_to_reclaim();
        puts("Passed test for child tries to reclaim interp\n");
    }


    int testsSucceeded = 0;
    int testsFailed = 0;

    int exitCode;
    while(Wait(&exitCode) != ERROR){
        putsArgs("%d out of %d tests have terminated!\n", testsSucceeded + testsFailed + 1, numTests);
        if (exitCode == SUCCESS)
            testsSucceeded += 1;
        else if (exitCode == ERROR)
            testsFailed += 1;
    }
    
    putsArgs("%d / %d tests succeeded\n", testsSucceeded, numTests);
    putsArgs("%d / %d tests failed\n", testsFailed, numTests);

    return (testsFailed == 0 ? SUCCESS : ERROR);
}





#define HELLO "hello world"
#define BUFFER_SIZE 64

int grandchild(int pipe_to_child1) {
  char buffer[BUFFER_SIZE];
  memset(buffer, '\0', sizeof(char) * BUFFER_SIZE);

  PipeRead(pipe_to_child1, buffer, BUFFER_SIZE);
  //putsArgs("Grandchild: I read '%s' from child1 to show that the pipe works.\n", buffer);

  //puts("Grandchild: I will sleep for 5 seconds, then exit.\n");
  Delay(5);
  
  return IDK;
}

int child1(int pipe_to_parent) {
  int pipe_to_grandchild;
  PipeInit(&pipe_to_grandchild);

  if( 0 == Fork() ) {
    Exit(grandchild(pipe_to_grandchild));
  }

  PipeWrite(pipe_to_parent, &pipe_to_grandchild, sizeof(int));
  //putsArgs("Child1: parent, pipe I have with grandchild is %d.\n", pipe_to_grandchild);

  PipeWrite(pipe_to_grandchild, HELLO, strlen(HELLO));
  //putsArgs("Child1: I said '%s' to grandchild to show that the pipe works.\n", HELLO);

  //puts("Child1: I will sleep for 5 seconds, then exit.\n");
  Delay(2);

  Reclaim(pipe_to_grandchild);
  WaitAll();
  
  return IDK;
}

int child2(int pipe_to_parent) {
  int pipe_to_nephew;
  
  PipeRead(pipe_to_parent, &pipe_to_nephew, sizeof(int));
  //putsArgs("Child2: parent says that child1 talks to grandchild through pipe #%d.\n", pipe_to_nephew);

  char c;
  int r = PipeRead(pipe_to_nephew, &c, sizeof(char));
  putsArgs("Child2: when I try to read from that pipe, I get PipeRead() returns %d (buffer: '%c').\n", r, c);
  verifyError(r, "Able to read pipe of brother\n");
  
  
  return (SUCCESS);
}

int non_descendant_tries_to_read(void) {
  //puts("Parent: I will demonstrate that non-descendnats cannot read from a pipe.\n");
  //puts("Parent: Child 1 will bear a grandchild and talk to it with a pipe.\n");
  //puts("Parent: Child 1 will also let me know of the pipe id.\n");
  //puts("Parent: I will let child 2 know of that pipe id.\n");
  //puts("Parent: Child 2 will try to use that pipe.\n");
    
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
  //putsArgs("Parent: child1 says that it's talking to grandchild through pipe #%d.\n", pipe_between_child1_and_grandchild);
  //puts("Parent: I will let child2 know of this.\n");
  PipeWrite(pipe_to_child2, &pipe_between_child1_and_grandchild, sizeof(int));

  //puts("Parent: I will exit.\n");

  Reclaim(pipe_to_child1);
  Reclaim(pipe_to_child2);

  int exitCode;
  while(Wait(&exitCode) != ERROR){
    if (exitCode != IDK){
        WaitAll();
        Exit(exitCode);
    }
  }

  //puts("This shouldn't happen!");
  Exit(ERROR);
}

int child_tries_to_reclaim(void) {
  int pipe;
  int lock;
  int cvar;
  PipeInit(&pipe);
  LockInit(&lock);
  CvarInit(&cvar);

  //putsArgs("Parent: I made a pipe, a lock, and a cvar, id is %d, %d, and %d.\n", pipe, lock, cvar);

  if( 0 == Fork() ) {
    int r;
    
    r = Reclaim(pipe);
    //putsArgs("Child: I tried to reclaim the pipe whose id is %d.\n", pipe);
    //putsArgs("Child: I only got return value %d.\n", r);
    verifyError(r, "Child was able to reclaim parent's pipe");
    
    r = Reclaim(lock);
    //putsArgs("Child: I tried to reclaim the lock whose id is %d.\n", lock);
    //putsArgs("Child: I only got return value %d.\n", r);
    verifyError(r, "Child was able to reclaim parent's lock");

    r = Reclaim(cvar);
    //putsArgs("Child: I tried to reclaim the cvar whose id is %d.\n", cvar);
    //putsArgs("Child: I only got return value %d.\n", r);
    verifyError(r, "Child was able to reclaim parent's cvar");
    
    //puts("Child: I will now exit.\n");
    Exit(SUCCESS);
  }

  Reclaim(pipe);
  Reclaim(lock);
  Reclaim(cvar);

  //puts("Parent: I will exit.\n");
  int exitCode;
  Wait(&exitCode);
  if (exitCode != SUCCESS)
    Exit(ERROR);
}
