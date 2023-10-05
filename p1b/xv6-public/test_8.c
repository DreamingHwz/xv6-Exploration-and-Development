#include "types.h"
#include "user.h"

#define PATH "pathname"

int main() {
  
  if (trace(PATH) < 0) {
    printf(1, "XV6_TEST_OUTPUT trace failed\n");
    exit();
  }

  for (int i = 0; i < 10000; i++) {
    open(PATH, 0);
  }

  int ret = getcount();
  printf(1, "XV6_TEST_OUTPUT %d\n", ret);

  exit();
}
