// BigStack.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Program that grows a big stack to test TRAP_MEMORY

#include "include/hardware.h"
#include "KernelGlobals.h"

int test_depth = 100;


int initialDataArray[2048];

void deepRecursiveFunction(int depth);

int main(void) {

  deepRecursiveFunction(test_depth);
  
  return 0;
}

void deepRecursiveFunction(int depth){
    initialDataArray[depth] = 1;
    int stackFillingArray[512];
    //int heapMalloc = malloc(1024);
    if (depth <= 0){
        TracePrintf(TRACE_USERLAND, "BigStack reached bottom!\n");
        return;
    }
    TracePrintf(TRACE_USERLAND, "BigStack at depth %d, stackArray at %p\n", depth, stackFillingArray);
    deepRecursiveFunction(depth - 1);
    //free(heapMalloc);
}

// End of BigStack.c
