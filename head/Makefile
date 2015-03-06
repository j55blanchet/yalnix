#
#	Sample Makefile for Yalnix kernel and user programs.
#	
#	Prepared by Sean Smith and Adam Salem and various Yalnix developers
#	of years past...
#
#	You must modify the KERNEL_SRCS and KERNEL_OBJS definition below to be your own
#	list of .c and .o files that should be linked to build your Yalnix kernel.
#
#	You must modify the USER_SRCS and USER_OBJS definition below to be your own
#	list of .c and .o files that should be linked to build your Yalnix user programs
#
#	The Yalnix kernel built will be named "yalnix".  *ALL* kernel
#	Makefiles for this lab must have a "yalnix" rule in them, and
#	must produce a kernel executable named "yalnix" -- we will run
#	your Makefile and will grade the resulting executable
#	named "yalnix".
#

#make all will make all the kernel objects and user objects
ALL = $(KERNEL_ALL) $(USER_APPS)
KERNEL_ALL = yalnix

#List all kernel source files here. 
KERNEL_SRCS = KernelGlobals.c KernelStart.c SetKernelData.c SetKernelBrk.c Traps.c Utility.c LoadProgram.c ContextSwitch.c SystemCalls.c
#List the objects to be formed form the kernel source files here.  Should be the same as the prvious list, replacing ".c" with ".o"
KERNEL_OBJS = KernelGlobals.o KernelStart.o SetKernelData.o SetKernelBrk.o Traps.o Utility.o LoadProgram.o ContextSwitch.o SystemCalls.o
#List all of the header files necessary for your kernel
KERNEL_INCS = KernelGlobals.h DataStructures.h Traps.h Utility.h ContextSwitch.h


#List all user programs here.
USER_APPS = InitProcess IdleProcess programs/Forker programs/Execker programs/Fun programs/Waiter programs/EvilExec programs/Terminals programs/Criminal programs/BigStack programs/StackOverflow programs/SilentStackGrowth programs/PureStackOverflow programs/InterProcess programs/LedyardBridge programs/InterpCriminal programs/test/bigstack programs/test/forktest programs/test/torture programs/test/zero programs/SmallTest programs/IdleScheduling
#List all user program source files here.  SHould be the same as the previous list, with ".c" added to each file
USER_SRCS = InitProcess.c IdleProcess.c programs/Forker.c programs/Execker.c programs/Fun.c programs/Waiter.c programs/EvilExec.c programs/Terminals.c programs/Criminal.c programs/BigStack.c  programs/StackOverflow.c programs/SilentStackGrowth.c programs/PureStackOverflow.c programs/InterProcess.c programs/LedyardBridge.c programs/InterpCriminal.c programs/test/bigstack.c programs/test/forktest.c programs/test/torture.c programs/test/zero.c programs/SmallTest.c programs/IdleScheduling.c
#List the objects to be formed form the user  source files here.  Should be the same as the prvious list, replacing ".c" with ".o"
USER_OBJS = InitProcess.o IdleProcess.o programs/Forker.o programs/Execker.o programs/Fun.o programs/Waiter.o programs/EvilExec.o programs/Terminals.o programs/Criminal.o programs/BigStack.o  programs/StackOverflow.o programs/SilentStackGrowth.o programs/PureStackOverflow.o programs/InterProcess.o programs/LedyardBridge.o programs/InterpCriminal.o programs/test/bigstack.o programs/test/forktest.o programs/test/torture.o programs/test/zero.o programs/SmallTest.o programs/IdleScheduling.o
#List all of the header files necessary for your user programs
USER_INCS = include/hardware.h KernelGlobals.h programs/UserUtility.h

#write to output program yalnix
YALNIX_OUTPUT = yalnix

STUDENT_ARGS = -g -Wall -Wextra -pedantic #-fdiagnostics-color=always # JHL. Commented out because GCC doesn't understand it.

#
#	These definitions affect how your kernel is compiled and linked.
#       The kernel requires -DLINUX, to 
#	to add something like -g here, that's OK.
#



#Set additional parameters.  Students generally should not have to change this next section

#Use the gcc compiler for compiling and linking
CC = gcc

DDIR58 = /net/class/cs58/yalnix
LIBDIR = $(DDIR58)/lib
INCDIR = $(DDIR58)/include
ETCDIR = $(DDIR58)/etc

# any extra loading flags...
LD_EXTRA = 

KERNEL_LIBS = $(LIBDIR)/libkernel.a $(LIBDIR)/libhardware.so

# the "kernel.x" argument tells the loader to use the memory layout
# in the kernel.x file..
KERNEL_LDFLAGS = $(LD_EXTRA) -L$(LIBDIR) -lkernel -lelf -Wl,-T,$(ETCDIR)/kernel.x -Wl,-R$(LIBDIR) -lhardware
LINK_KERNEL = $(LINK.c)

#
#	These definitions affect how your Yalnix user programs are
#	compiled and linked.  Use these flags *only* when linking a
#	Yalnix user program.
#

USER_LIBS = $(LIBDIR)/libuser.a
ASFLAGS = -D__ASM__
CPPFLAGS= -m32 -fno-builtin -I. -I$(INCDIR) -g -DLINUX 


##########################
#Targets for different makes
# all: make all changed components (default)
# clean: remove all output (.o files, temp files, LOG files, TRACE, and yalnix)
# count: count and give info on source files
# list: list all c files and header files in current directory
# kill: close tty windows.  Useful if program crashes without closing tty windows.
# $(KERNEL_ALL): compile and link kernel files
# $(USER_ALL): compile and link user files
# %.o: %.c: rules for setting up dependencies.  Don't use this directly
# %: %.o: rules for setting up dependencies.  Don't use this directly

all: $(ALL)	

clean:
	rm -f *.o programs/*.o *~ programs/*~ TTYLOG* TRACE trace.txt $(YALNIX_OUTPUT) $(USER_APPS)  core.* core DISK rm -f programs/test/*.o programs/test/*~

count:
	wc $(KERNEL_SRCS) $(USER_SRCS)

list:
	ls -l *.c *.h

kill:
	killall yalnixtty yalnixnet yalnix

no-core:
	rm -f core.*

$(KERNEL_ALL): $(KERNEL_OBJS) $(KERNEL_LIBS) $(KERNEL_INCS)
	$(LINK_KERNEL) -o $@ $(KERNEL_OBJS) $(KERNEL_LDFLAGS) $(STUDENT_ARGS)

$(USER_APPS): $(USER_OBJS) $(USER_INCS)
	$(ETCDIR)/yuserbuild.sh $@ $(DDIR58) $@.o

edit:
	emacs Makefile
