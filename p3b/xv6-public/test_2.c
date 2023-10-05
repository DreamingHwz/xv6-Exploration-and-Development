/* clone and play with the argument */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
volatile int arg1 = 11;
volatile int arg2 = 22;
volatile int global = 1;

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

   int clone_pid = clone(worker, (void*)&arg1, (void*)&arg2, stack);
   assert(clone_pid > 0);
   while(global != 33);
   assert(arg1 == 44);
   assert(arg2 == 55);
   printf(1, "TEST PASSED\n");
   
   void *join_stack;
   int join_pid = join(&join_stack);
   assert(join_pid == clone_pid);
   free(p);
   exit();
}

void
worker(void *arg1, void *arg2) {
   int tmp1 = *(int*)arg1;
   int tmp2 = *(int*)arg2;
   *(int*)arg1 = 44;
   *(int*)arg2 = 55;
   assert(global == 1);
   global = tmp1 + tmp2;
   exit();
}
