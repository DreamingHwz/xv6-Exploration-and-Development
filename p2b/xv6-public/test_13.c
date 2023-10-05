#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "pstat.h"

int
main(int argc, char *argv[]) {
  
  char* p1_file = "P1.txt";
  char* p2_file = "P2.txt";

  struct stat f1, f2;
  struct pstat pstat;

  int pid1, pid2;
  int fd1, fd2;

  fd1 = open(p1_file, O_CREATE | O_WRONLY);
  if (fd1 < 0) {
    printf(2, "Open %s failed\n", p1_file);
    exit();
  }
 
  fd2 = open(p2_file, O_CREATE | O_WRONLY);
  if (fd2 < 0) {
    printf(2, "Open %s failed\n", p2_file);
    exit();
  }

  pid1 = fork();
  if (pid1 < 0) {
    printf(2, "Fork child process 1 failed\n");
  } else if (pid1 == 0) { // child process 1
    settickets(1);
    while(1)
      printf(fd1, "A");
  } 
  
  pid2 = fork();
  if (pid2 < 0) {
    printf(2, "Fork child process 2 failed\n");
    exit();
  } else if (pid2 == 0) { // child process 2
    settickets(0);
    while (1)  
      printf(fd2, "A");
  }

  sleep(1000);
  getpinfo(&pstat);
  kill(pid1);
  kill(pid2);

  wait();
  wait();

  fstat(fd1, &f1);
  fstat(fd2, &f2);
  // compare file size made by child process
  if (f1.size > f2.size) {
    printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  }

  close(fd1);
  close(fd2);
  
  unlink(p1_file);
  unlink(p2_file);

  exit();
}
