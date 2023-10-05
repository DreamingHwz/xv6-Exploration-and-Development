# CS537-Operating-System

These are the course projects from [CS537 Introduction to Operating Systems](https://pages.cs.wisc.edu/~remzi/Classes/537/Fall2022/) guided by Professor [Remzi Arpaci-Dusseau](http://www.cs.wisc.edu/~remzi) during Fall 2022 semester at University of Wisconsin-Madison. Original project descriptions and course materials can be found on the course website. REALLY love the course and it really inspires me. Many thanks to Prof. Remzi.

## P1A
Some codes are added for the command "mult" and "div" in Memcached. Now it can implement multiplication and division commands, such as "mult x 4" and "div x 100" after setting x.

Some changes:
* In file "proto_text.c": replace with "short incr" in the function "static void process_arithmetic_command". Add codes that recognize the tokens of "mult" and "div".i
* In file "vim thread.c": replace with "short incr" in the function "enum delta_result_type add_delta".
* In file "memcached.c" & "memcached.h": replace with "const short incr" in the function "enum delta_result_type do_add_delta" and "enum delta_result_type add_delta". Add cases of "incr" to implement the function of multiplication and division.

## P1B - Adding System Calls to xv6 System
Add system call  ```int trace(const char *pathname)``` and ```int getcount(void)```.  

## P2A - Shell

A Unix shell is built. It includes a file "wish.c".

#### Description

The shell supports these commands:
* ```exit```: exit the shell
* ```cd```: change directory
* ```path```: set path for executables
* ```if```: if statement like ```if command == 0 THEN command fi```
* other executable commands in ```/bin```, such as ```ls```, ```vim```.

To run the shell:
* ```gcc wish.c -o wish```
* For interactive mode:  ```./wish```, For batch mode: ```./wish batch.txt```
* Input command after prompt ```wish>```. For redirection, use ```>``` character like ```ls > output``` (to make my shell uers happy).

#### Author
Skylar Hou 10.12.2022

## P2B - Scheduling and Virtual Memory in xv6

To change the scheduling policy to  a simple priority-based scheduler, set some pages to read-only or read-and-write, and add a Null-pointer Dereference.

#### Introduction
Four system call is added:
* ```settickets```: Change ```myproc()->tickets``` to ```int number```, which only equals to 1 (high) or 0 (low) in this project.

* ```getpinfo```: Fill current used processes status to ```struct pstat * p```, which is not a null pointer.

* ```mprotect```: Change the protection bits in PTEs of ```int len``` of pages start at ```void * addr```. The ```addr``` should be page-aligned, and smaller than process memory size and virtual memory size.

* ```munprotect```
Similar to ```mprotect``` but change the bit from read-only to read-and-write.

To add a system call, ```usys.S``` ```syscall.c``` ```sysfile.c``` ```sysproc.c``` ```syscall.h``` ```user.h``` might be editted.

#### Author
Skylar Hou


## P3A - Parallel Sort
It is a multi-thread merge sort.

#### Introduction
```
typedef struct {
    int key;
    char value[96];
} record_t;
typedef record_t* record_ptr;
```
* ```record_ptr inputMmap(char* name)```: to map the input file into the address space, then access bytes of the input file via pointers.
* ```record_ptr* initRecords(record_ptr mapped)```: init an array of ptr to each records.
* ```void multithread_sort()```: do multi-thread sort. Each thread sorts part of the records array.
* ```void output(record_ptr* records, char* filename)```: Output the records array to file.

#### Usage

```
gcc -Wall -Werror -pthread -O psort.c -o psort
./psort input output
```


#### Author

Skylar Hou, Yuxin Liu


## P3B - Kernel Threads

Adding kernel threads to xv6. 

#### Overview

* ```clone``` This call creates a new kernel thread that shares the calling process's address space. The new process uses stack as its user stack.
* ```join``` This call waits for a child thread that shares the address space with the calling process to exit.
* ```thread_create``` ```thread_join``` Create a user stack and a child thread.
* ```lock``` A simple spin lock use the atom function ```fetch_and_add```.
* ```wait``` and ```exit```are changed to work with child processes that do not share the address space with this process.
* ```growproc``` is changed to grow all the child processes that share the same address space with the current process.
* Some file is modified to add system calls.

#### Author

Yuxin Liu, Skylar Hou


## P4 - Distributed File System

A distributed file system includes a file server with the on-disk file system and client library, which supports:  
* ```int MFS_Init(char *hostname, int port)```: find the server exporting the file system.
* ```int MFS_Lookup(int pinum, char *name)```: look up the inode number of entry ```name```.
* ```int MFS_Stat(int inum, MFS_Stat_t *m)```: return the file information of ```inum```.
* ```int MFS_Write(int inum, char *buffer, int offset, int nbytes)```: write ```buffer``` with size ```nbytes``` to ```offset``` of the file specified by ```inum```. Return 0 for success, otherwise return -1.
* ```int MFS_Read(int inum, char *buffer, int offset, int nbytes)```: read ```nbytes``` of data from ```offset``` of the file specified by ```inum``` to ```buffer```. Return 0 for success, otherwise return -1.
* ```int MFS_Creat(int pinum, int type, char *name)```: create a file of ```type``` and ```name``` in parent directory of ```pinum``` .
* ```int MFS_Unlink(int pinum, char *name)```: delete a file with ```name``` in parent directory of ```pinum```.
* ```int MFS_Shutdown()```: tell server to exit.

#### Author

Skylar Hou, Yuxin Liu
