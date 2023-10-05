/* no exit call in nested thread, should trap at bogus address */
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

   int arg1 = 42, arg2 = 24;
   int thread_pid = thread_create(worker, &arg1, &arg2);
   assert(thread_pid > 0);

   int join_pid = thread_join();
   assert(join_pid == thread_pid);
   assert(global == 3);

   printf(1, "TEST PASSED\n");
   exit();
}

void nested_worker(void *arg1, void *arg2){
   int arg1_int = *(int*)arg1;
   int arg2_int = *(int*)arg2;
   assert(arg1_int == 42);
   assert(arg2_int == 24);
   assert(global == 2);
   global++;
   // no exit() in thread
}

void
worker(void *arg1, void *arg2) {
   int tmp1 = *(int*)arg1;
   int tmp2 = *(int*)arg2;
   assert(tmp1 == 42);
   assert(tmp2 == 24);
   assert(global == 1);
   global++;

   int nested_thread_pid = thread_create(nested_worker, &tmp1, &tmp2);
   assert(nested_thread_pid > 0);
   for(int j=0;j<10000;j++);

   int nested_join_pid = thread_join();
   assert(nested_join_pid)
   assert(nested_join_pid == nested_thread_pid);
   exit();
}

