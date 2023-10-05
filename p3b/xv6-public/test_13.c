/* multiple threads with some depth of function calls */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;
int num_threads = 30;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg1, void *arg2);

unsigned int fib(unsigned int n) {
   if (n == 0) {
      return 0;
   } else if (n == 1) {
      return 1;
   } else {
      return fib(n - 1) + fib(n - 2);
   }
}


int
main(int argc, char *argv[])
{
   ppid = getpid();

   assert(fib(28) == 317811);

   int arg1 = 11, arg2 = 22;

   for (int i = 0; i < num_threads; i++) {
      int thread_pid = thread_create(worker, &arg1, &arg2);
      assert(thread_pid > 0);
   }

   for (int i = 0; i < num_threads; i++) {
      int join_pid = thread_join();
      assert(join_pid > 0);
   }

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg1, void *arg2) {
   int tmp1 = *(int*)arg1;
   int tmp2 = *(int*)arg2;
   assert(tmp1 == 11);
   assert(tmp2 == 22);
   assert(global == 1);
   assert(fib(2) == 1);
   assert(fib(3) == 2);
   assert(fib(9) == 34);
   assert(fib(15) == 610);
   exit();
}

