README.txt

Computer Science 58
Operating Systems
Professor Sean W. Smith

Yalnix Project
Checkpoint 6
Julien Blanchet and Jae Heon Lee

We have achieved goals for checkpoint 6.

Disregard the one compiling warning. We are checking against a misuse in program which causes the warning.

INSTRUCTIONS:
- make          -- to build Yalnix.
- run1          -- to run Yalnix and see trace file for the Ledyard Bridge test.
- run2          -- to run Yalnix and see trace file for the boundary cases test.
- cat trace.txt -- to see trace file.
- make clean    -- to remove built programs.

APPENDIX: Notes on Pipes.

Reproduced from ./DataStructures.h:128-136.
// NOTES ON PIPES.
// - A pipe's owner is the process which calls PipeInit().
// - A pipe's owner can Reclaim() the pipe. No one else can.
// - A pipe's owner and its descendants can use the pipe, i.e., call PipeRead() and PipeWrite().
//   ==> This restricts the use of pipe to processes which can legitimately know about the pipe.
// - A second call to PipeWrite() will overwrite what is in the pipe.
// - Writing 0 bytes to a pipe will clear the pipe.
// - A call to PipeRead() will not delete what is in the pipe.
//   ==> This facilitates communication between multiple processes.

End of README.txt
