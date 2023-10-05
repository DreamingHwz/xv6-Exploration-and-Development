/* clone copies file descriptors, but doesn't share */
#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "x86.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
volatile uint newfd = 0;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg1, void *arg2);

int main(int argc, char *argv[])
{
   ppid = getpid();
   void *stack, *p = malloc(PGSIZE * 2);
   assert(p != NULL);
   if((uint)p % PGSIZE)
     stack = p + (PGSIZE - (uint)p % PGSIZE);
   else
     stack = p;

   int fd = open("tmp", O_WRONLY|O_CREATE);
   assert(fd == 3);
   int clone_pid = clone(worker, 0, 0, stack);
   assert(clone_pid > 0);
   while(!newfd);
   assert(write(newfd, "goodbye\n", 8) == -1);
   printf(1, "TEST PASSED\n");

   void *join_stack;
   int join_pid = join(&join_stack);
   assert(join_pid == clone_pid);
   free(p);
   exit();
}

void
worker(void *arg1, void *arg2) {
   assert(write(3, "hello\n", 6) == 6);
   xchg(&newfd, open("tmp2", O_WRONLY|O_CREATE));
   exit();
}
