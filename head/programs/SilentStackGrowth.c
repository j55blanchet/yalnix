// SilentStackGrowth.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Like StackOverflow.c, but architected to have stack silently grow into heap 
// (NOTE: heap refuses to grow any further)

#include "include/hardware.h"
#include "KernelGlobals.h"


#define ARRAY_SIZE 65536 // 2^16


void neverEndingRecursiveFunction(int);

int main(void) {
  neverEndingRecursiveFunction(1);
  return 0;
}

void neverEndingRecursiveFunction(int depth){

    char stackFillingArray[ARRAY_SIZE];
    void* heapFillingArary = (void*) malloc(ARRAY_SIZE);

    TracePrintf(TRACE_USERLAND, 
                "SilentStackGrowth: depth:%d, stackArray:%p heapArray:%p\n", 
                depth, 
                stackFillingArray,
                heapFillingArary);
                
    neverEndingRecursiveFunction(depth + 1);
}

// SilentStackGrowth.c
