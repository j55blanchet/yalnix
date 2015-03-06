// EvilExec.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Try passing exec some naughty material!

#include "include/hardware.h"
#include "KernelGlobals.h"

int main(void) {
  int pid = GetPid();
  TracePrintf(TRACE_USERLAND, "EvilExec is running: pid=%d...\n", pid);
  
  char* noargs[1];
  noargs[0] = NULL;
  
  // 5 trials:
  // #1: pass null pointer as program name
  if (Fork() == 0){
    TracePrintf(TRACE_USERLAND, "EvilExec test#1: Exec(NULL, noargs) to exec. pid=%d...\n", GetPid());
    Exec(NULL, noargs);
    
    TracePrintf(TRACE_USERLAND, "EvilExec #1 Returned after Exec Call!!!\n");
    Exit(ERROR);
  }
  
  // #2: pass program name that is in invalid memory
  if (Fork() == 0){
    char i = 'X';
    char* stackLocalVariable = &i;
    stackLocalVariable += 2000; // this should suffice to exceed stack!
    TracePrintf(TRACE_USERLAND, "EvilExec test#2: pass invalid stack variable to exec. Exec(stackLocalVariable, noargs) pid=%d...\n", GetPid());
    Exec(stackLocalVariable, noargs);
    
    TracePrintf(TRACE_USERLAND, "EvilExec #2 Returned after Exec Call!!!\n");
    Exit(ERROR);
  }
  
  // #3: pass valid program name (lets say, what if we pass main()??)
  if (Fork() == 0){
    TracePrintf(TRACE_USERLAND, "EvilExec test#3: pass &main to exec. Exec((char*) &main, noargs) pid=%d...\n", GetPid());
    Exec((char*) &main, noargs);
    
    TracePrintf(TRACE_USERLAND, "EvilExec #3 Returned after Exec Call!!!\n");
    Exit(ERROR);
  }
  
  // #4: pass invalid argument array pointer
  if (Fork() == 0){
      TracePrintf(TRACE_USERLAND, "EvilExec test#4: pass bad argument array to exec.  Exec(\"programs/Fun\", ((char**) -1))  pid=%d...\n", GetPid());
        
      Exec("programs/Fun", ((char**) -1));
        
      TracePrintf(TRACE_USERLAND, "EvilExec #4 Returned after Exec Call!!!\n");
      Exit(ERROR);
  }
  
  // #5: pass invalid argument in valid argument array
  if (Fork() == 0){
      char* argv[3];
      argv[0] = "hi";
      argv[1] = (char*) -1;
      argv[2] = NULL;
      
      TracePrintf(TRACE_USERLAND, "EvilExec test#5: pass argument array with bad strings to exec. Exec(\"programs/Fun\", argv), argv[1] = (char*) -1, pid=%d...\n", GetPid());
      Exec("programs/Fun", argv);
            
      TracePrintf(TRACE_USERLAND, "EvilExec #5 Returned after Exec Call!!!\n");
      Exit(ERROR);
  }
  
  int loops = 15;
  while(loops > 0){
    loops --;
    TracePrintf(TRACE_USERLAND, "EvilExec Parent is continuing\n");
    Pause();
 } 
 return 0;
}

// End of Execker.c
