#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
   struct pstat st;
   int pid = getpid();
   int defaulttickets = 0;
   
   if(getpinfo(&st) == 0)
   {
    for(int i = 0; i < NPROC; i++) {
      if (st.inuse[i]) {
        if(st.pid[i] == pid) {
          defaulttickets = st.tickets[i];
         }
      }
   }
   }
  else
  {
   printf(1, "XV6_SCHEDULER\t FAILED\n");
   exit();
  }

  
  if(defaulttickets == 1)
  {
   printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  }
  else
  {
   printf(1, "XV6_SCHEDULER\t FAILED\n");
  }
   exit();
}
