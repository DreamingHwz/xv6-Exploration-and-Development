/* nested thread user library functions */
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
void nested_worker(void *arg1, void *arg2);

int
main(int argc, char *argv[])
{
   ppid = getpid();

   int arg1 = 35;
   int arg2 = 42;
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
   assert(arg1_int == 35);
   assert(arg2_int == 42);
   assert(global == 2);
   global++;
   exit();
}

void
worker(void *arg1, void *arg2) {
   int arg1_int = *(int*)arg1;
   int arg2_int = *(int*)arg2;
   assert(arg1_int == 35);
   assert(arg2_int == 42);
   assert(global == 1);
   global++;
   int nested_thread_pid = thread_create(nested_worker, &arg1_int, &arg2_int);
   int nested_join_pid = thread_join();
   assert(nested_join_pid == nested_thread_pid);
   exit();
}

