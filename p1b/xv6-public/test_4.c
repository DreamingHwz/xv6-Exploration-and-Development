#include "types.h"
#include "user.h"

int main() {
  int ret0 = getcount();
  open("random file", 0);
  open("another random file", 0);
  int ret1 = getcount();
  printf(1, "XV6_TEST_OUTPUT %d %d\n", ret0, ret1);
  exit();
}
