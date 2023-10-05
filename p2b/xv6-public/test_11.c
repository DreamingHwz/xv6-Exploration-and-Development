#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
   int ppid = getpid();

   if (fork() == 0) {     

      int *p = (int *)atoi(argv[1]);
      printf(1, "%d\n", *p);

      printf(1, "XV6_VM\t FAILED\n");
      
      kill(ppid);
      
      exit();
   } else {
      wait();
   }

   printf(1, "XV6_VM\t SUCCESS\n");
   exit();
}
