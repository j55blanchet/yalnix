int NUM_PAGES /*= (PMEM_SIZE / PAGESIZE) */;

void* KERNEL_DATA_START; // The lower limit for kernel.
void* KERNEL_DATA_END;   // The upper limit for kernel.
void* kernel_break;      //

int PMEM_SIZE; // Size of the physical memory.

/* Doubly linked lists without sentinels (pointers contained within PCB) */
pcb_t* process_running;   // The running process. (pcb->prev, pcb->next are NULLified for this)
pcb_t* process_ready;     // Head of list of ready processes.
//pcb_t* process_blocked; // Head of list of blocked processes.
pcb_t* processes_delayed; // Head of list of processed blocked for Delay().



pcb_t* processes_waiting_tty_receive[NUM_TERMINALS]; // array of heads of lists of processes blocked for tty receive
bool tty_transmit_lock[NUM_TERMINALS]; // disallow transmitting to terminal before TRAP_TTY_TRANSMIT finishes prior transmission
pcb_t* processes_waiting_tty_transmit[NUM_TERMINALS]; // process blocked for transmitting to each tty terminal (pcb->prev, pcb->next are NULLified for this)
pcb_t* processes_dead;      // Head of list of processes dead but not collected by parent.

// tty_ownership_entry tty_list[NUM_TERMINALS];

lock_t* locks; // Head of list of locks. (each has assoc. list of blocked PCBs)
cvar_t* cvars; // Head of list of cvars. (each has assoc. list of blocked PCBs)

int pid_current_max;        // The largest pid used so far.
int synchro_id_current_max; // The largest id for locks and cvars used so far.


// IDEA: subroutine that moves current proceses to a queue and runs next ready process
// should take pcb_t** so that it can modify the heads of the above queues (putting them to NULL when empty)




// Subroutines that we might want
void /*??*/ proc_abort(pcb_t*){}
void /*??*/ move_to_ready(pcb_t** sourceList /*so that we can nullify source list*/, pcb_t* actualPCB){}

// move running process to the specified queue, and start next ready process
void /*??*/ move_to_blocked(pcb_t** queue);

/* SHOULD have a way to quickly get lock from lock id */