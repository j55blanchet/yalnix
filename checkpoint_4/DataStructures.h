// DataStructures.h
//
// Julien Blanchet and Jae Heon Lee.
//
// Defines data structures.

#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include "include/hardware.h"
#include "Constants.h"

// ======== ======== ======== ======== ======== ======== ======== ========
// Frame table.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// Table of frames in physical memory.
//
typedef struct fte fte_t;
struct fte {
  unsigned char valid : 1;
  unsigned char prot  : 3;
};
//
// Frame table is ((fte_t*) frame_table).
// It is declared in KernelGlobals.c and initialized in KernelStart().
//
// Frame table is of size (PMEM_SIZE / PAGESIZE). There are this many frames.
// Each frame is PAGESIZE bytes long.
// frame_table[i], where i is frame id, has base (PMEM_BASE+(i*PAGESIZE)) and
// limit (PMEM_BASE+((i+1)*PAGESIZE)).
//
// Conversion macros.
#define frame_id_to_addr(id)   ((void*)(PMEM_BASE + ((id) * PAGESIZE)))
#define frame_addr_to_id(addr) ((((unsigned int)(addr)) - PMEM_BASE) / PAGESIZE)
//
// ======== ======== ======== ======== ======== ======== ======== ========



// ======== ======== ======== ======== ======== ======== ======== ========
// Page table.
// -------- -------- -------- -------- -------- -------- -------- --------
//
// Table of pages in virtual memory.
//
typedef struct pte pte_t; // struct pte is defined in hardware.h.
//
// Region 0 page table is ((pte_t*) region0_table).
// It is  declared in KernelGlobals.c and initialized in KernelStart().
// Region 1 page table is found in each PCB.
//
// Conversion macros.
#define r0_id_to_addr(id)   ((void*)(VMEM_0_BASE + ((id) * PAGESIZE)))
#define r0_addr_to_id(addr) ((((unsigned int)(addr)) - VMEM_0_BASE) / PAGESIZE)
#define r1_id_to_addr(id)   ((void*)(VMEM_1_BASE + ((id) * PAGESIZE)))
#define r1_addr_to_id(addr) ((((unsigned int)(addr)) - VMEM_1_BASE) / PAGESIZE)
//
// ======== ======== ======== ======== ======== ======== ======== ========



// ======== ======== ======== ======== ======== ======== ======== ========
// Dead process information.
// -------- -------- -------- -------- -------- -------- -------- --------
//
typedef struct soul soul_t;
struct soul {
  int pid;
  int status;
  soul_t* next;
};
//
// -------- -------- -------- -------- -------- -------- -------- --------



// ======== ======== ======== ======== ======== ======== ======== ========
// Process control block (PCB).
// -------- -------- -------- -------- -------- -------- -------- --------
//

#define R1_PAGETABLE_NUM_ENTRIES (VMEM_1_SIZE / PAGESIZE)
#define PCB_NAME_LENGTH (MAX_PROGRAM_NAME_LENGTH)
typedef struct pcb pcb_t;
struct pcb {
  int pid;    // Process id.
  int ppid;   // Parent's process id.
              // -1 if parentless.

  char pname[PCB_NAME_LENGTH]; // For convenience in programming.

  int wakeup_time;  // The number of ticks for which this process waits for KernelDelay().

  pte_t r1_page_table[R1_PAGETABLE_NUM_ENTRIES];
  pte_t r0_stack_page_table[KERNEL_STACK_MAXSIZE / PAGESIZE];

  int r1_stack_base_index;  // Lowest address in process's stack.
  int r1_break_limit_index; // Lowest address above process's heap.

  UserContext   u_context;
  KernelContext k_context;

  // Doubly linked list.
  pcb_t* prev;
  pcb_t* next;

  // Information about children.
  int num_children; /* alive */ // Used to ensure that Wait() returns immediately if no children.
  soul_t* dead_children_head;
  soul_t* dead_children_tail;
};
//
// Queues for process control blocks.
typedef struct {
  pcb_t* head;
  int size;
} queue_t;
//
// ======== ======== ======== ======== ======== ======== ======== ========

#endif
// End of DataStructures.h
