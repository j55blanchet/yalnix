// StackOverflow.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Program grows both heap and stack in attempt to get them to collide

#include "programs/UserUtility.h"

void neverEndingRecursiveFunction(int);

int main(void) {
  neverEndingRecursiveFunction(1);
  return 0;
}

void neverEndingRecursiveFunction(int depth){
  putsArgs("Depth %d.\n", depth);
  neverEndingRecursiveFunction(depth+1);
}

// StackOverflow.c
