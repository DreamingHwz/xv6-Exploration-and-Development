#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


int
main(int argc, char *argv[])
{
   struct pstat st;
  
  if(getpinfo(&st) == 0)
  {
   printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  }
  else
  {
   printf(1, "XV6_SCHEDULER\t FAILED\n");
  }
   exit();
}
