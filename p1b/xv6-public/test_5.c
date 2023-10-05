#include "types.h"
#include "user.h"

int main() {
  int ret = trace((char*) 0xA000);
  printf(1, "XV6_TEST_OUTPUT %d\n", ret);
  exit();
}
