/* memory leaks from thread library? */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
volatile int global;

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

   int i, thread_pid, join_pid;
   for(i = 0; i < 10; i++) {
      global = 1;
      thread_pid = thread_create(worker, 0, 0);
      assert(thread_pid > 0);
      join_pid = thread_join();
      assert(join_pid == thread_pid);
      assert(global == 5);
//	  printf(1, "%d\n", (uint)sbrk(0));
      assert((uint)sbrk(0) < (150 * 4096) && "shouldn't even come close");
   }

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg1, void *arg2) {
   assert(global == 1);
   global += 4;
   exit();
}

