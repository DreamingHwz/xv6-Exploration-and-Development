# Kernel Threads

Adding kernel threads to xv6. 

## Overview

* ```clone``` This call creates a new kernel thread which shares the calling process's address space. The new process uses stack as its user stack.
* ```join``` This call waits for a child thread that shares the address space with the calling process to exit.
* ```thread_create``` ```thread_join``` Create a user stack and a child thread.
* ```lock``` A simple spin lock use the atom function ```fetch_and_add```.
* ```wait``` and ```exit``` is changed to work with child processes that do not share the address space with this process.
* ```growproc``` is changed to grow all the child processes that share the same address space with the current process.
* Some file is modified to add system calls.

## Author

Skylar Hou, Yuxin Liu