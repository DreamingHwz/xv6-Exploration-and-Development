#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  int i;
  int x = 0;

  if(argc != 2){
    exit();
  }
  for (i = 1; i<atoi(argv[1]); i++)
    x += i;
  exit();
  return x;
}
