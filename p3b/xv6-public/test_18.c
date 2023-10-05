/* test lock correctness using nested threads */
#include "types.h"
#include "user.h"


#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 0;
lock_t lock;
int num_threads = 30;
int loops = 10;


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

   lock_init(&lock);

   int i;
   for (i = 0; i < num_threads; i++) {
      int thread_pid = thread_create(worker, 0, 0);
      assert(thread_pid > 0);
   }

   for (i = 0; i < num_threads; i++) {
      int join_pid = thread_join();
      assert(join_pid > 0);
   }

   assert(global == num_threads * 2);

   printf(1, "TEST PASSED\n");
   exit();
}

void worker2(void *arg1,void *arg2){
    int j;
    lock_acquire(&lock);
    for(j=0;j<50;j++);
    global++;
    lock_release(&lock);
    exit();
    
}
void
worker(void *arg1, void *arg2) {
   
    lock_acquire(&lock);
    int j;
    for(j = 0; j < 50; j++); // take some time
    global++;
    lock_release(&lock);

    int nested_thread_pid = thread_create(worker2, 0, 0);
    assert(nested_thread_pid > 0);
    int nested_join_pid = thread_join();
    assert(nested_join_pid > 0);
    
    assert(nested_thread_pid==nested_join_pid);

   exit();
}







