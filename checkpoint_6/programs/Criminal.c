// Criminal.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Executes illegal operations.

#include "programs/UserUtility.h"

int main(void) {
  puts("Criminal running...\n");

  if( 0 == Fork() ) {
    puts("Criminal-c: I will calcualte 1/0.\n");
    int a = 1/0;
    Exit(a);
  }

  if( 0 == Fork() ) {
    puts("Criminal-c: I will read from a null pointer.\n");
    char* a = NULL;
    char b = *a;
    Exit(*a);
  }

  if( 0 == Fork() ) {
    puts("Criminal-c: I will write to a null pointer.\n");
    char* a = NULL;
    *a = 'a';
    Exit(*a);
  }

  puts("Criminal will wait for children.\n");
  WaitAll();

  puts("Criminal exiting...\n");
  Exit(0);
}
