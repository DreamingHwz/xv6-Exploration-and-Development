#include "types.h"
#include "user.h"

#define PATH "file.txt"

int main() {
  char buffer[100];
  
  strcpy(buffer, PATH);
  if (trace(buffer) < 0) {
    printf(1, "XV6_TEST_OUTPUT trace failed\n");
    exit();
  }

  strcpy(buffer, "random string");
  open(PATH, 0);
  open(PATH, 0);
  open(PATH, 0);

  int ret = getcount();
  printf(1, "XV6_TEST_OUTPUT %d\n", ret);
  exit();
}
