// KernelGlobals.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Defines (as opposed to declare or initialize) global variables for Yalnix kernel.
//
// See KernelGlobals.h for information.

#include "KernelGlobals.h"

void* KERNEL_DATA_START;
void* KERNEL_DATA_END;
void* kernel_break;
unsigned int PMEM_SIZE;
void (*trap_vector_table[TRAP_VECTOR_SIZE]) (UserContext*);
fte_t* frame_table;
pte_t* r0_page_table;
queue_t RUNNING;
queue_t READY;
queue_t CLOCK_WAITING;
int pid_count = 0;
int ticks = 0;

int init_is_started = 0;
// End of KernelGlobals.c
