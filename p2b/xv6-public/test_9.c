#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[]){
	int pid_par = getpid();
	int tickets = 0;
	
	if(settickets(tickets) == 0)
	{
	}
	else
	{
	 printf(1, "XV6_SCHEDULER\t FAILED\n");
	 exit();
	}
	
	if(fork() == 0){
		int pid_chd = getpid();
		struct pstat st;
		if(getpinfo(&st) == 0)
		{
		}
		else
		{
		 printf(1, "XV6_SCHEDULER\t FAILED\n");
		 exit();
		}
		int tickets_par = -1,tickets_chd = -1;
		for(int i = 0; i < NPROC; i++){
      			if (st.pid[i] == pid_par){
				tickets_par = st.tickets[i];
			}
			else if (st.pid[i] == pid_chd){
				tickets_chd = st.tickets[i];
			}
		}

		printf(1, "parent: %d, child: %d\n", tickets_par, tickets_chd);

		if(tickets_chd == tickets)
		{
		 printf(1, "XV6_SCHEDULER\t SUCCESS\n");
		}
		else
		{
		 printf(1, "XV6_SCHEDULER\t FAILED\n");
		}

    exit();
	}
  	while(wait() > 0);
	exit();
}
