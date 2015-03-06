// data_structures.h
//
// Julien Blanchet and Jae Heon Lee.

// Note. This is NOT a source file, at least at this point.
//
// Contains pseudocode for data structures.

/* We can verify these later */
// #define USER_ADDRESS_SPACE_SIZE (VMEM_1_LIMIT - VMEM_1_BASE)
// #define USER_PAGE_TABLE_ENTRIES (USER_ADDRESS_SPACE_SIZE / PAGESIZE)

// #define USER_PTE_INDEX(addr) ()

// pcb_t represents each process.
typedef struct pcb {
  int pid;  //
  int ppid; // Parent's process id.

  int status; // Exit status for parent's collection.

  int ticks_remaining; // For Delay().
  //int blocker_id;      // If blocked, for lock, cvar, or pipe of this id.

  // Page table.
  struct pte page_table[(VMEM_1_LIMIT - VMEM_1_BASE) / PAGESIZE];
  struct pte kernel_stack_page_table[/* (kernel stack size) / PAGESIZE */];

  int stack_lowest_pte_index; // the lowest pte allocated for the stack
  int user_break_pte_index; // the highest pte allocated for the heap

  // store context information. UserContext will be updated with each interrupt
  // UserContext is pointer, KernalContext is embedded within the struct
  UserContext*  u_context;
  KernelContext k_context;

  /* All PCB lists are doubly linked without sentinels. These
  linked must be nullified when PCB is in a place that can contain only
  one PCB (for example, in running, and while blocked for transmission to a tty terminal */
  struct pcb* next;
  struct pcb* prev;
} pcb_t;

// frame_t represents frames in physical memory.
typedef struct {
  void* pmem_addr_base; // Each is of size PAGESIZE.
  
  frame_t* next; // Singly linked.
} frame_t;

// Note. All locks, cvars, and pipes share ids.

// lock_t represents each lock.
typedef struct lock {
  int id;

  pcb_t* owner;   // If status == UNACQUIRED, owner is meaningless.
  pcb_t* waiters; // Head of list of processes waiting for this lock.
  enum {
    UNACQUIRED, ACQUIRED
  } status;

  // Embedded linked list.
  struct lock* prev;
  struct lock* next;
} lock_t;

// cvar_t represents each cvar.
typedef struct cvar {
  int id; //

  pcb_t* owner;   // If status == UNACQUIRED, owner is meaningless.
  pcb_t* waiters; // Head of list of processes waiting for this cvar.
  lock_t* lock;   // Lock associated with this cvar. (filled upon CvarWait)
  
  // enum {
  //   WAITING, NOT_WAITING
  // } status;

  // Embedded linked list.
  struct cvar* prev;
  struct cvar* next;
} cvar_t;

// pipe_t represents each pipe.
// JHL. This isn't required for undergraduate project?
// JBB. Not sure. Let's not worry about it for now
typedef struct pipe {
  int id;

  int piper_pid; // Process which created this pipe.
  int pipee_pid; // Process to which this pipe was created.

  // JHL. Should there be a buffer? A pointer to a buffer?
  // JBB. Depending on if we want the pipe to have a fixed
  // limit or not, we'll have to have either a FIFO queue
  // or a static buffer, i think.

  struct pipe* prev;
  struct pipe* next;
} pipe_t;
