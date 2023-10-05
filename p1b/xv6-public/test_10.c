#include "types.h"
#include "user.h"

#define PATH "pathname"

int main() {
  int iter = 20;  

  if (trace(PATH) < 0) {
    printf(1, "XV6_TEST_OUTPUT trace failed\n");
    exit();
  }

  for (int i = 0; i < iter; i++) {
    if (i % 2 == 0) {
      open(PATH, 0);
    } else {
      open("random", 0);
    }
  }

  int ret = getcount();
  printf(1, "XV6_TEST_OUTPUT %d\n", ret);

  exit();
}
