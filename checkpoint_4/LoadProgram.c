// LoadProgram.c
//
// Julien Blanchet and Jae Heon Lee.
// Based on template provided by CS 58 staff at ~cs58/yalnix/sample/template.c
//
// Loads an executable image onto a current process (making the current process
// become this new process).

// ==>> This is a TEMPLATE for how to write your own LoadProgram function.
// ==>> Places where you must change this file to work with your kernel are
// ==>> marked with "==>>".  You must replace these lines with your own code.
// ==>> You might also want to save the original annotations as comments.

#include <fcntl.h>
#include <unistd.h>
#include <hardware.h>
#include <load_info.h>
// ==>> #include anything you need for your kernel here
#include "DataStructures.h"
#include "KernelGlobals.h"

/*
 *  Load a program into an existing address space.  The program comes from
 *  the Linux file named "name", and its arguments come from the array at
 *  "args", which is in standard argv format.  The argument "proc" points
 *  to the process or PCB structure for the process into which the program
 *  is to be loaded. 
 */
int
LoadProgram(char *name, char *args[], pcb_t* proc) 
// ==>> Declare the argument "proc" to be a pointer to your PCB or
// ==>> process descriptor data structure.  We assume you have a member
// ==>> of this structure used to hold the cpu context 
// ==>> for the process holding the new program.  
{
  TracePrintf(TRACE_COMMENT, "Entering LoadProgram() with \"%s\"\n", name);

  int fd;
  int (*entry)();
  struct load_info li;
  int i;
  char *cp;
  char **cpp;
  char *cp2;
  int argcount;
  int size;
  int text_pg1;
  int data_pg1;
  int data_npg;
  int stack_npg;
  long segment_size;
  char *argbuf;

  char my_name[PCB_NAME_LENGTH];
  strncpy(my_name, name, PCB_NAME_LENGTH-1);
  my_name[PCB_NAME_LENGTH-1] = '\0';
  
  /*
   * Open the executable file 
   */
  if ((fd = open(name, O_RDONLY)) < 0) {
    TracePrintf(0, "LoadProgram: can't open file '%s'\n", name);
    return ERROR;
  }

  if (LoadInfo(fd, &li) != LI_NO_ERROR) {
    TracePrintf(0, "LoadProgram: '%s' not in Yalnix format\n", name);
    close(fd);
    return (-1);
  }

  if (li.entry < VMEM_1_BASE) {
    TracePrintf(0, "LoadProgram: '%s' not linked for Yalnix\n", name);
    close(fd);
    return ERROR;
  }

  /*
   * Figure out in what region 1 page the different program sections
   * start and end
   */
  text_pg1 = (li.t_vaddr - VMEM_1_BASE) >> PAGESHIFT;
  data_pg1 = (li.id_vaddr - VMEM_1_BASE) >> PAGESHIFT;
  data_npg = li.id_npg + li.ud_npg;
  /*
   *  Figure out how many bytes are needed to hold the arguments on
   *  the new stack that we are building.  Also count the number of
   *  arguments, to become the argc that the new "main" gets called with.
   */
  size = 0;
  for (i = 0; args[i] != NULL; i++) {
    TracePrintf(3, "counting arg %d = '%s'\n", i, args[i]);
    size += strlen(args[i]) + 1;
  }
  argcount = i;

  TracePrintf(2, "LoadProgram: argsize %d, argcount %d\n", size, argcount);
  
  /*
   *  The arguments will get copied starting at "cp", and the argv
   *  pointers to the arguments (and the argc value) will get built
   *  starting at "cpp".  The value for "cpp" is computed by subtracting
   *  off space for the number of arguments (plus 3, for the argc value,
   *  a NULL pointer terminating the argv pointers, and a NULL pointer
   *  terminating the envp pointers) times the size of each,
   *  and then rounding the value *down* to a double-word boundary.
   */
  cp = ((char *)VMEM_1_LIMIT) - size;

  cpp = (char **)
    (((int)cp - 
      ((argcount + 3 + POST_ARGV_NULL_SPACE) *sizeof (void *))) 
     & ~7);

  /*
   * Compute the new stack pointer, leaving INITIAL_STACK_FRAME_SIZE bytes
   * reserved above the stack pointer, before the arguments.
   */
  cp2 = (caddr_t)cpp - INITIAL_STACK_FRAME_SIZE;



  TracePrintf(1, "prog_size %d, text %d data %d bss %d pages\n",
	      li.t_npg + data_npg, li.t_npg, li.id_npg, li.ud_npg);


  /* 
   * Compute how many pages we need for the stack */
  stack_npg = (VMEM_1_LIMIT - DOWN_TO_PAGE(cp2)) >> PAGESHIFT;

  TracePrintf(1, "LoadProgram: heap_size %d, stack_size %d\n",
	      li.t_npg + data_npg, stack_npg);


  /* leave at least one page between heap and stack */
  if (stack_npg + data_pg1 + data_npg >= MAX_PT_LEN) {
    close(fd);
    return ERROR;
  }

  /*
   * This completes all the checks before we proceed to actually load
   * the new program.  From this point on, we are committed to either
   * loading succesfully or killing the process.
   */

  /*
   * Set the new stack pointer value in the process's exception frame.
   */

  // ==>> Here you replace your data structure proc
  // ==>> proc->context.sp = cp2;
  proc->u_context.sp = cp2;

  /*
   * Now save the arguments in a separate buffer in region 0, since
   * we are about to blow away all of region 1.
   */
  cp2 = argbuf = (char *)malloc(size);
  // ==>> You should perhaps check that malloc returned valid space
  if( cp2 == NULL ) {
    TracePrintf(TRACE_SEVERE, "LoadProgram(): fails because malloc() returns NULL.\n");
    return ERROR;
  }
  for (i = 0; args[i] != NULL; i++) {
    TracePrintf(3, "saving arg %d = '%s'\n", i, args[i]);
    strcpy(cp2, args[i]);
    cp2 += strlen(cp2) + 1;
  }

  /*
   * Set up the page tables for the process so that we can read the
   * program into memory.  Get the right number of physical pages
   * allocated, and set them all to writable.
   */

  // ==>> Throw away the old region 1 virtual address space of the
  // ==>> curent process by freeing
  // ==>> all physical pages currently mapped to region 1, and setting all 
  // ==>> region 1 PTEs to invalid.
  // ==>> Since the currently active address space will be overwritten
  // ==>> by the new program, it is just as easy to free all the physical
  // ==>> pages currently mapped to region 1 and allocate afresh all the
  // ==>> pages the new program needs, than to keep track of
  // ==>> how many pages the new process needs and allocate or
  // ==>> deallocate a few pages to fit the size of memory to the requirements
  // ==>> of the new process.
  { int i;
    int pfn;
    for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
      if( proc->r1_page_table[i].valid == 1 ) {
	proc->r1_page_table[i].valid = 0;
	proc->r1_page_table[i].prot  = PROT_NONE;
	pfn = proc->r1_page_table[i].pfn;
	proc->r1_page_table[i].pfn   = 0;

	// Free frame in frame table.
	frame_table[pfn].valid = 0;
	frame_table[pfn].prot  = PROT_NONE;
      }
    }
  }
  // ==>> Allocate "li.t_npg" physical pages and map them starting at
  // ==>> the "text_pg1" page in region 1 address space.  
  // ==>> These pages should be marked valid, with a protection of 
  // ==>> (PROT_READ | PROT_WRITE).
  { int i;
    int pfn;
    for(i = text_pg1; i < text_pg1 + li.t_npg; i++) {
      // JHL. I'm assuming that text_pg1 through text_pg1+li.t_npg are all unused.
      if( proc->r1_page_table[i].valid == 1 ) {
	TracePrintf(TRACE_WRONG, "LoadProgram(): overwriting text on page %d in use\n.", i);
	Halt();
      }
      proc->r1_page_table[i].valid = 1;
      proc->r1_page_table[i].prot  = PROT_READ | PROT_WRITE;
      proc->r1_page_table[i].pfn   = FindFreeFrame(PROT_READ | PROT_WRITE);
      if( proc->r1_page_table[i].pfn == ERROR ) {
	// JHL.
	// How do we handle if error occurs here, i.e., insufficient frames?
	// Free all used frames before failing?
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL,
		    "LoadProgram(): cannot find %d of %d frames required for writing text.\n",
		    i, li.t_npg);
	return KILL;
      }
    }
  }
  // ==>> Allocate "data_npg" physical pages and map them starting at
  // ==>> the  "data_pg1" in region 1 address space.  
  // ==>> These pages should be marked valid, with a protection of 
  // ==>> (PROT_READ | PROT_WRITE).
  { int i;
    int pfn;
    for(i = data_pg1; i < data_pg1 + data_npg; i++) {
      // JHL. I'm assuming that data_pg1 through data_pg1+data_npg are all unused.
      if( proc->r1_page_table[i].valid == 1 ) {
	TracePrintf(TRACE_WRONG, "LoadProgram(): overwriting data on page %d in use.\n", i);
	Halt();
      }
      proc->r1_page_table[i].valid = 1;
      proc->r1_page_table[i].prot  = PROT_READ | PROT_WRITE;
      proc->r1_page_table[i].pfn   = FindFreeFrame(PROT_READ | PROT_WRITE);
      if( proc->r1_page_table[i].pfn == ERROR ) {
	// JHL.
	// How do we handle if error occurs here, i.e., insufficient frames?
	// Free all used frames before failing?
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL,
		    "LoadProgram(): cannot find %d of %d frames required for data.\n",
		    i, li.t_npg);
	return KILL;
      }
    }
  }

  /*
   * Allocate memory for the user stack too.
   */
  // ==>> Allocate "stack_npg" physical pages and map them to the top
  // ==>> of the region 1 virtual address space.
  // ==>> These pages should be marked valid, with a
  // ==>> protection of (PROT_READ | PROT_WRITE).
  { int i;
    for(i = R1_PAGE_TABLE_SIZE - stack_npg - 1; i < R1_PAGE_TABLE_SIZE; i++) {
      // JHL. I'm assuming that data_pg1 through data_pg1+data_npg are all unused.
      if( proc->r1_page_table[i].valid == 1 ) {
	TracePrintf(TRACE_WRONG, "LoadProgram(): overwriting stack on page %d in use.\n", i);
	Halt();
      }
      proc->r1_page_table[i].valid = 1;
      proc->r1_page_table[i].prot  = PROT_READ | PROT_WRITE;
      proc->r1_page_table[i].pfn   = FindFreeFrame(PROT_READ | PROT_WRITE);
      if( proc->r1_page_table[i].pfn == ERROR ) {
	// JHL.
	// How do we handle if error occurs here, i.e., insufficient frames?
	// Free all used frames before failing?
	TracePrintf(TRACE_UNIMPLEMENTED_CRITICAL,
		    "LoadProgram(): cannot find %d of %d frames required for data.\n",
		    i, li.t_npg);
	return KILL;
      }
    }
  }

  // JHL. Test print.
  /*
  {
    int i;
    for(i = 0; i < R1_PAGE_TABLE_SIZE; i++) {
      TracePrintf(TRACE_COMMENT, "***r1_page_table[%d]: valid[%d] prot[%d]\n",
		  i, proc->r1_page_table[i].valid, proc->r1_page_table[i].prot);
    }
  }
  */
  // JHL. End.

  /*
   * All pages for the new address space are now in the page table.  
   * But they are not yet in the TLB, remember!
   */
  // JHL. Do we flush so that new address space doesn't get confused?
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  /*
   * Read the text from the file into memory.
   */
  lseek(fd, li.t_faddr, SEEK_SET);
  segment_size = li.t_npg << PAGESHIFT;
  if (read(fd, (void *) li.t_vaddr, segment_size) != segment_size) {
    close(fd);
    // ==>> KILL is not defined anywhere: it is an error code distinct
    // ==>> from ERROR because it requires different action in the caller.
    // ==>> Since this error code is internal to your kernel, you get to define it.
    TracePrintf(TRACE_SEVERE, "LoadProgram(): cannot load %s: error reading text.\n", name);
    return KILL;
  }

  /*
   * Read the data from the file into memory.
   */
  lseek(fd, li.id_faddr, 0);
  segment_size = li.id_npg << PAGESHIFT;

  if (read(fd, (void *) li.id_vaddr, segment_size) != segment_size) {
    close(fd);
    TracePrintf(TRACE_SEVERE, "LoadProgram(): cannot load %s: error reading data.\n", name);
    return KILL;
  }

  /*
   * Now set the page table entries for the program text to be readable
   * and executable, but not writable.
   */

  // ==>> Change the protection on the "li.t_npg" pages starting at
  // ==>> virtual address VMEM_1_BASE + (text_pg1 << PAGESHIFT).  Note
  // ==>> that these pages will have indices starting at text_pg1 in 
  // ==>> the page table for region 1.
  // ==>> The new protection should be (PROT_READ | PROT_EXEC).
  // ==>> If any of these page table entries is also in the TLB, either
  // ==>> invalidate their entries in the TLB or write the updated entries
  // ==>> into the TLB.  It's nice for the TLB and the page tables to remain
  // ==>> consistent.
  { int i;
    int BASE = r1_addr_to_id(VMEM_1_BASE + (text_pg1 << PAGESHIFT));
    for(i = BASE; i < BASE + li.t_npg; i++) {
      // These are already in use. Otherwise we're donig something wrong.
      if( proc->r1_page_table[i].valid == 0) {
	TracePrintf(TRACE_WRONG, "LoadProgram(): touching unintended memory at page %d.\n", i);
	Halt();
      }
      proc->r1_page_table[i].prot = PROT_READ | PROT_EXEC;
    }
  }

  close(fd);			/* we've read it all now */

  /*
   * Zero out the uninitialized data area
   */
  bzero(li.id_end, li.ud_end - li.id_end);

  /*
   * Set the entry point in the exception frame.
   */
  // ==>> Here you should put your data structure (PCB or process)
  // ==>>  proc->context.pc = (caddr_t) li.entry;
  proc->u_context.pc = (void*) li.entry;

  /*
   * Now, finally, build the argument list on the new stack.
   */

  // save the name of the process into the PCB
  // TracePrintf(TRACE_VERBOSE, "LoadProgram: attempting to save name information into the PCB! (Name is '%s'(len=%d))\n", name, strlen(name));
  memcpy(proc->pname, my_name, sizeof(char) * PCB_NAME_LENGTH);


#ifdef LINUX
  memset(cpp, 0x00, VMEM_1_LIMIT - ((int) cpp));
#endif



  *cpp++ = (char *)argcount;		/* the first value at cpp is argc */
  cp2 = argbuf;
  for (i = 0; i < argcount; i++) {      /* copy each argument and set argv */
    *cpp++ = cp;
    strcpy(cp, cp2);
    cp += strlen(cp) + 1;
    cp2 += strlen(cp2) + 1;
  }
  free(argbuf);
  *cpp++ = NULL;			/* the last argv is a NULL pointer */
  *cpp++ = NULL;			/* a NULL pointer for an empty envp */

  TracePrintf(TRACE_COMMENT, "Leaving LoadProgram() with SUCCESS...\n");
  return SUCCESS;
}

// End of LoadProgram.c
