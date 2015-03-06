#include "programs/UserUtility.h"

#define TTY_PRINTF 1 // Use TtyPrintf() to print. Else use TtyWrite() directly.

int pid;

void recurse(int tty, int depth) {

  putsArgs("SmallTest[%d] >> I am calling TtyPrintf()\n", pid);
  if( TTY_PRINTF ) {
    TtyPrintf(tty, "SmallTest[%d] >> recurse() at depth %d.\n", pid, depth);
  } else {
    TtyWrite(tty, "SmallTest >> in recurse().\n", 27);
  }
  putsArgs("SmallTest[%d] >> TtyPrintf() returns here.\n", pid);

  if( depth > 0 ) {
    recurse(tty, depth - 1);
  }
}

void iterate(int tty, int limit) {
  int i;
  for(i = 0; i < limit; i++) {
    puts("SmallTest >> TtyPrintf() is called here.\n");
    if( TTY_PRINTF ) {
      TtyPrintf(tty, "SmallTest[%d] >> iterate() at loop %d.\n", pid, i);
    } else {
      TtyWrite(tty, "SmallTest >> in iterate().\n", 27);
    }
    puts("SmallTest >> TtyPrintf() returns here.\n");
  }
}

int main2(void) {

  pid = GetPid();

  putsArgs("SmallTest[%d] >> I am calling TtyPrintf()\n", pid);
  if( TTY_PRINTF ) {
    TtyPrintf(0, "SmallTest[%d] >> I am running...\n", pid);
  } else {
    TtyWrite(0, "SmallTest >> I am running...\n", 29);
  }
  putsArgs("SmallTest[%d] >> TtyPrintf() returns here.\n", pid);

  //  puts("SmallTest >> malloc() is called here.\n");
  //  void* ptr = (void*) malloc(1);
  //  putsArgs("SmallTest >> malloc() returns %p.\n", ptr);
  
  int tty;
  int parent = Fork();
  if( parent ) {
    tty = 0;
  } else {
    tty = 1;
  }

  if( parent ) {
    malloc(1);
  } else {
    free(malloc(1));    
  }

  pid = GetPid();
  recurse(tty, 0x4);
  // iterate(0, 0x4);

  putsArgs("SmallTest[%d] >> I am calling TtyPrintf()\n", pid);
  if( TTY_PRINTF ) {
    TtyPrintf(tty, "SmallTest[%d] >> I am exiting...\n", pid);
  } else {
    TtyWrite(tty, "SmallTest >> I am exiting...\n", 29);
  }
  putsArgs("SmallTest[%d] >> TtyPrintf() returns here.\n", pid);

  { int status;
    Wait(&status);
  }

  Exit(0);
}

int main(void) {

  int size = 0x1000;
  
  void* ptr;
  int tty = 0;

  while(1) {
    TtyPrintf(tty, "I will malloc() 0x%x bytes.\n", size);
    ptr = (void*)malloc(size);
    TtyPrintf(tty, "I now have pointer at %p.\n", ptr);
    size += 0x100;
    free(ptr);
    TtyPrintf(tty, "I have freed the pointer.\n");

    if( size > 0x3000 || size < 0x800 ) {
      break;
    }
  }

  Exit(0);
}

int main3(void) {
  void* ptr = (void*)malloc(0x2000);
  TtyPrintf(0, "ptr @ %p.\n", ptr);
  free(ptr);
  Exit(0);
}
