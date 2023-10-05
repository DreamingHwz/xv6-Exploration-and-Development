#include "types.h"
#include "user.h"

#define PATH "pathname"

int main() {
  int iter = 10;

  if (trace(PATH) < 0) {
    printf(1, "XV6_TEST_OUTPUT trace failed\n");
    exit();
  }

  int ret = fork();
  if (ret < 0) {
    printf(1, "XV6_TEST_OUTPUT fork failed\n");
    exit();
  } else if (ret == 0) {
    for (int i = 0; i < iter; i++) {
      open(PATH, 0);
    }
    exit();
  } else {
    wait();
  }

  int ret1 = getcount();

  for (int i = 0; i < 2*iter; i++) {
    open(PATH, 0);
  }
  int ret2 = getcount();
  
  printf(1, "XV6_TEST_OUTPUT %d %d\n", ret1, ret2);

  exit();
}
