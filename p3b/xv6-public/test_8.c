/* join, not wait, should handle threads */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg1, void *arg2);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   void *stack, *p = malloc(PGSIZE * 2);
   assert(p != NULL);
   if((uint)p % PGSIZE)
     stack = p + (PGSIZE - (uint)p % PGSIZE);
   else
     stack = p;

   int arg1 = 42, arg2 = 24;
   int clone_pid = clone(worker, &arg1, &arg2, stack);
   assert(clone_pid > 0);

   sleep(250);
   assert(wait() == -1);

   void *join_stack;
   int join_pid = join(&join_stack);
   assert(join_pid == clone_pid);
   assert(stack == join_stack);
   assert(global == 2);

   printf(1, "TEST PASSED\n");
   free(p);
   exit();
}

void
worker(void *arg1, void *arg2) {
   int tmp1 = *(int*)arg1;
   int tmp2 = *(int*)arg2;
   assert(tmp1 == 42);
   assert(tmp2 == 24);
   assert(global == 1);
   global++;
   exit();
}

