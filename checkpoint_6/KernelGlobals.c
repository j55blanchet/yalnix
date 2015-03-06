// KernelGlobals.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Defines (as opposed to declare or initialize) global variables for Yalnix kernel.
// Initializes some such variables.
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
queue_t SLEEPING;
queue_t WAITING;
queue_t* WRITING/*[0..3]*/;
queue_t* WRITING_WAIT;
queue_t* READING/*[0..3]*/;
int pid_count = 0;
int ticks = 0;
pcb_t** pcb_array;
int pcb_array_size = PCB_ARRAY_INITIAL_SIZE;
char** tty_write_buffer/*[0..3]*/;
char** tty_read_buffer /*[0..3]*/;
int interp_array_size = INTERP_ARRAY_INITIAL_SIZE;
interp_t** interp_array;
int iid_count = 0;

// End of KernelGlobals.c
