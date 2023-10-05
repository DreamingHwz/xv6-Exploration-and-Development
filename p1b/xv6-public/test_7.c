#include "types.h"
#include "user.h"

int main() {
  char filename[257];
  for(int i = 0; i < 256; i++) {
    filename[i] = 'a';
  }
  filename[256] = '\0';
  int ret = trace(filename);
  printf(1, "XV6_TEST_OUTPUT %d\n", ret);
  exit();
}
