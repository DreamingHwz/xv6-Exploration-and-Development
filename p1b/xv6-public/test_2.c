#include "types.h"
#include "user.h"

int main() {
  int ret1 = trace("random path");
  int ret2 = getcount();

  printf(1, "XV6_TEST_OUTPUT %d %d\n", ret1, ret2);

  exit();
}
