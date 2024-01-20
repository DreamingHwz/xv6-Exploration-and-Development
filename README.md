# Operating-System Projects

This repository showcases a variety of operating system projects I've worked on, fortunately guided by Professor [Remzi Arpaci-Dusseau](http://www.cs.wisc.edu/~remzi). These projects cover enhancements to Memcached, new system calls added to xv6, a Unix shell, scheduling and virtual memory in xv6, a multi-threaded merge sort, kernel threads, and a distributed file system.

## Adding Commands to Memcached

### Description

Some valuable additions have been made to Memcached, enhancing its capabilities to support multiplication and division commands. Now, you can perform operations like `mult x 4` and `div x 100` after setting the variable `x`. These changes are detailed as follows:

- In the file `proto_text.c`, replaced `short incr` in the function `static void process_arithmetic_command` and added code to recognize the tokens `mult` and `div.`
- In the `thread.c` file, replaced `short incr` in the function `enum delta_result_type add_delta`.
- In both `memcached.c` and `memcached.h`, replaced `const short incr` in the functions `enum delta_result_type do_add_delta` and `enum delta_result_type add_delta`. Also, added cases for `incr` to implement the functionality of multiplication and division.

## Adding System Calls to xv6 System

### Description

In this project, two new system calls have been added to the xv6 operating system, providing enhanced functionality:

- `int trace(const char *pathname)`: A system call to trace a pathname.
- `int getcount(void)`: A system call to retrieve a count.

## Shell

### Description

This repository features a Unix shell, including a file named "wish.c." The shell supports a range of essential commands, making it a versatile tool for interacting with the system. The supported commands include:

- `exit`: To exit the shell.
- `cd`: For changing the current directory.
- `path`: To set the path for executable files.
- `if`: Conditional statements like `if command == 0 THEN command fi`.
- Other executable commands located in `/bin`, such as `ls` and `vim`.

To run the shell, follow these steps:

- Compile it with `gcc wish.c -o wish`.
- For interactive mode, use `./wish`.
- For batch mode, use `./wish batch.txt`.
- Enter commands after the prompt `wish>`. For redirection, use the `>` character.

## Scheduling and Virtual Memory in xv6

### Description

This project focuses on changing the scheduling policy to a simple priority-based scheduler, setting some pages to read-only or read-and-write, and introducing a Null-pointer Dereference.

**Introduction:**
Four system calls have been added:

- `settickets`: Changes `myproc()->tickets` to an `int number`, which is either 1 (high) or 0 (low) in this project.
- `getpinfo`: Fills the status of the currently used processes into `struct pstat * p`, which is not a null pointer.
- `mprotect`: Changes the protection bits in page table entries for a specified range of pages.
- `munprotect`: Similar to `mprotect`, but changes the protection bit from read-only to read-and-write.

To add a system call, you may need to edit `usys.S`, `syscall.c`, `sysfile.c`, `sysproc.c`, `syscall.h`, and `user.h`.

## Parallel Sort

### Introduction

This project presents a multi-threaded merge sort implementation using a custom data structure:

```c
typedef struct {
    int key;
    char value[96];
} record_t;
typedef record_t* record_ptr;
```

**Key Functions:**
* `record_ptr inputMmap(char* name)`: Maps the input file into the address space and allows access to bytes of the input file via pointers.
* `record_ptr* initRecords(record_ptr mapped)`: Initializes an array of pointers to each record.
* `void multithread_sort()`: Performs a multi-threaded sort, with each thread handling a portion of the records array.
* `void output(record_ptr* records, char* filename)`: Outputs the records array to a file.

**Usage:**
```
gcc -Wall -Werror -pthread -O psort.c -o psort
./psort input output
```

**Authors:** Skylar Hou, Yuxin Liu

## Kernel Threads

### Description

This project introduces kernel threads to the xv6 operating system, enhancing its concurrency and process management capabilities.

**Overview:**
* `clone`: This system call creates a new kernel thread that shares the calling process's address space, with the new process using its stack as the user stack.
* `join`: This system call waits for a child thread that shares the address space with the calling process to exit.
* `thread_create` and `thread_join` create a user stack and a child thread.
* A simple spin lock is implemented using the atomic function `fetch_and_add`.
* `wait` and `exit` have been modified to work with child processes that do not share the address space with the current process.
* `growproc` is changed to grow all child processes that share the same address space with the current process. Some files have been modified to integrate system calls.
Adding kernel threads to xv6. 

**Authors:** Skylar Hou, Yuxin Liu


## Distributed File System

### Description

This repository includes a distributed file system with a file server containing an on-disk file system and a client library, which offers a range of features:

**Overview:**
* `int MFS_Init(char *hostname, int port)`: Find the server exporting the file system.
* `int MFS_Lookup(int pinum, char *name)`: Look up the inode number of an entry by name.
* `int MFS_Stat(int inum, MFS_Stat_t *m)`: Return file information for a given inode.
* `int MFS_Write(int inum, char *buffer, int offset, int nbytes)`: Write data to a file specified by inum.
* `int MFS_Read(int inum, char *buffer, int offset, int nbytes)`: Read data from a file specified by inum to buffer.
* `int MFS_Creat(int pinum, int type, char *name)`: Create a file of a specified type in the parent directory of pinum.
* `int MFS_Unlink(int pinum, char *name)`: Delete a file with a given name in the parent directory of pinum.
* `int MFS_Shutdown()`: Tell the server to exit.

**Authors:** Skylar Hou, Yuxin Liu