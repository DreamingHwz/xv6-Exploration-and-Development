# P2B - Scheduling and Virtual Memory in xv6

To change scheduling policy to  a simple priority-based scheduler, set some pages to read-only or read-and-write, and add a Null-pointer Dereference.

## Introduction
Four system call is added:
* ```settickets```: Change ```myproc()->tickets``` to ```int number```, which only equals to 1 (high) or 0 (low) in this project.

* ```getpinfo```: Fill current used processes status to ```struct pstat * p```, which is not a null pointer.

* ```mprotect```: Change the protection bits in PTEs of ```int len``` of pages start at ```void * addr```. The ```addr``` should be page-aligned, and smaller than process memory size and virtual memory size.

* ```munprotect```
Similar to ```mprotect``` but change the bit from read-only to read-and-write.

To add a system call, ```usys.S``` ```syscall.c``` ```sysfile.c``` ```sysproc.c``` ```syscall.h``` ```user.h``` might be editted.

## Author
Skylar Hou
