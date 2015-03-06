// KernelGlobals.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Declares (as opposed to define or initialize) global variables for Yalnix kernel.
// Also declares constants.

#ifndef KERNAL_GLOBALS_H
#define KERNAL_GLOBALS_H

#include "include/hardware.h"
#include "include/yalnix.h"
#include "DataStructures.h"

// ======== ======== ======== ======== ======== ======== ======== ========
// Constants.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// ERROR is defined in yalnix.h.
#ifndef NULL
#define NULL (0)
#endif
//
// -------- -------- -------- -------- -------- -------- -------- --------

// ======== ======== ======== ======== ======== ======== ======== ========
// TracePrintf() level constants.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// -t [tracefile] switch to yalnix will turn on tracing.
// If tracing level in TracePrintf() call <= level argument to yalnix,
// then TracePrintf() call will print to trace file (i.e., the lower the more important).
// (Documentation 5.7.3.).
//
// Example:
//   yalnix -t -lk 99
// (Prints all TracePrintf messages.)
//
// TracePrintf() has the following form:
//   TracePrintf(int level, char* fmt, args...);
#define TRACE_WRONG                  01 // Reflects incorrect kernel programming.
//                                      // We are interested in fixing these above all.
#define TRACE_CRITICAL               10 // Will halt the machine (intended behavior).
#define TRACE_UNIMPLEMENTED_CRITICAL 15
#define TRACE_SEVERE                 20 // Will result in severe errors.
#define TRACE_UNIMPLEMENTED_WARNING  25
#define TRACE_COMMENT                60 // Comments.
#define TRACE_VERBOSE                65 // Addendum to comments.
#define TRACE_TRAP                   80
//
// Insert between each line in multi-line TracePrintf() outputs.
#define TRACE_N "           "
//
// ======== ======== ======== ======== ======== ======== ======== ========



// ======== ======== ======== ======== ======== ======== ======== ========
// Memory address constants and variables.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// Base is the lowest address inhabited by data.
// Limit is the lowest address above the addresses inhabited by data, i.e.,
// the highest address inhabited by data.
// (Documentation 2.2.).
//
// Boundaries for kernel text.
//
// Kernel text base in both physical memory and (later) virtual memory.
#define KERNEL_TEXT_START 0x20000 // Base.
// JHL. I don't think this number is defined or documented anywhere.
// I came up with this number from experimenting.
//
// Limit is KERNEL_DATA_START.
//
// Boundaries for kernel data.
extern void* KERNEL_DATA_START; // Base.
extern void* KERNEL_DATA_END;   // Limit.
// Initialized in SetKernelData().
//
// Break should be a multiple of PAGESIZE (Documentation 3.2.1.).
// Use UP_TO_PAGE() macro defined in hardware.h.
extern void* kernel_break;
// Initialized in SetKernelData() but changed often.
//
// Size of physical memory.
extern unsigned int PMEM_SIZE;
// Initialized in KernelStart() from information from hardware.
//
// Stack pointer should point to stack's limit - 4, where stack's limit is
// the address above the highest address usable in page allocated as stack.
#define INITIAL_STACK_SIZE 4
// JHL. I don't think this number is defined or documented anywhere.
// I came up with this number from experimenting.
// In hardware.h, INITIAL_STACK_FRAME_SIZE, for Linux, is defined as 0x0.
// This seems suspicious... For Solaris it's 0x40.
//
// ======== ======== ======== ======== ======== ======== ======== ========



// ======== ======== ======== ======== ======== ======== ======== ========
// Trap vector table.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// (Documentation 3.4., 4.2., hardware.h).
//
// Each trap handler method is of the form
//   void HandleTrap(UserContext*);
extern void (*trap_vector_table[TRAP_VECTOR_SIZE /*16*/]) (UserContext*);
// Initialized in KernelStart().
//
// ======== ======== ======== ======== ======== ======== ======== ========

// ======== ======== ======== ======== ======== ======== ======== ========
// Address tables.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// frame_table is an array of length (PMEM_SIZE / PAGESIZE).
// In frame_table[i], i is the frame number.
// frame_table[i] refers to frame whose base address is (PMEM_BASE+(i*PAGESIZE)).
// frame_table[i] = 0 if it is free (unused) or 1 otherwise (used).
//
// Conversion macros frame_id_to_addr() and frame_addr_to_id() are defined in DataStructures.h.
//
// Array of length (PMEM_SIZE / PAGESIZE).
#define FRAME_TABLE_SIZE (PMEM_SIZE / PAGESIZE)
extern fte_t* frame_table;
// Mallocked and initialized in KernelStart().
//
// r0_page_table is an array of length (VMEM_0_SIZE / PAGESIZE).
// In r0_page_table[i], i is the page number.
// r0_page_table[i] refers to page whose base address is (VMEM_0_BASE+(i*PAGESIZE)).
// Each entry (pte_t) in r0_page_table means:
//   u_long valid   : 1;/* page mapping is valid */
//   u_long prot    : 3;/* page protection bits */
//   u_long         : 4;/* reserved; currently unused */
//   u_long pfn     : 24;/* page frame number */
// (hardware.h).
//
// .prot can be PROT_READ | PROT_WRITE | PROT_EXEC.
// PROT_READ means that it can be read. It does NOT mean protected against reading.
// 
// Conversion macros r0_id_to_addr() and r0_addr_to_id() are defined in DataStructures.h.
// Top two (KERNEL_STACK_MAXSIZE / PAGESIZE) entries are for the kernel stack.
#define R0_STACK_PAGE_TABLE_SIZE (KERNEL_STACK_MAXSIZE / PAGESIZE)
//
// Array of length (VMEM_0_SIZE / PAGESIZE).
#define R0_PAGE_TABLE_SIZE (VMEM_0_SIZE / PAGESIZE)
extern pte_t* r0_page_table;
// Mallocked and initialized in KernelStart().
//
// r1_page_table is stored in each PCB.
// It is an array of length (VMEM_1_SIZE / PAGESIZE).
#define R1_PAGE_TABLE_SIZE (VMEM_1_SIZE / PAGESIZE)
//
//
// ======== ======== ======== ======== ======== ======== ======== ========

// ======== ======== ======== ======== ======== ======== ======== ========
// Process control blocks (PCB).
// PCB queues.
// -------- -------- -------- -------- -------- -------- -------- --------
//
extern pcb_t* process_running;
// First initialized in KernelStart().
//
// ======== ======== ======== ======== ======== ======== ======== ========

// ======== ======== ======== ======== ======== ======== ======== ========
// Process-related variables and constants.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// The total number of pids used so far.
extern int pid_count;
// Declared and initialized in KernelGlobals.c
//
// Utility macro for getting the next pid.
#define NEXT_PID (++pid_count)
// Treat NEXT_PID like a constant. It is always the next pid.
//
// ======== ======== ======== ======== ======== ======== ======== ========



// ======== ======== ======== ======== ======== ======== ======== ========
// Other variables.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// The number of TRAP_CLOCKs so far.
int ticks;
// Declared and initialized in KernelGlobals.c. Incremented in HandleTrapClock().
//
// ======== ======== ======== ======== ======== ======== ======== ========

#endif
// End of kernel_globals.h
