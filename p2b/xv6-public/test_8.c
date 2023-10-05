#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{

   if(getpinfo((struct pstat *)1000000) == -1)
   {
    printf(1, "XV6_SCHEDULER\t SUCCESS\n");
   }
   else
   {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
   }
   
   exit();
}
