// StackOverflow.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Program grows both heap and stack in attempt to get them to collide

#include "include/hardware.h"
#include "KernelGlobals.h"


#define ARRAY_SIZE 32768 // 2^15


void neverEndingRecursiveFunction(int);

int main(void) {
  neverEndingRecursiveFunction(1);
  return 0;
}

void neverEndingRecursiveFunction(int depth){

  char stackFillingArray[ARRAY_SIZE];
  void* heapFillingArary = (void*) malloc(ARRAY_SIZE);
  
  TracePrintf(TRACE_USERLAND, 
	      "BigStack: depth:%d, stackArray:%p heapArray:%p\n", 
	      depth, 
	      stackFillingArray,
	      heapFillingArary);
  
  neverEndingRecursiveFunction(depth + 1);
}

// StackOverflow.c
