#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"


#define PGSIZE 4096


int main(void) {
    const uint PAGES_NUM = 1;
    
    char *ptr = sbrk(PAGES_NUM * PGSIZE);
    printf(1, "XV6_TEST_OUTPUT %d\n", mprotect(ptr, -10));
    printf(1, "XV6_TEST_OUTPUT %d\n", mprotect(ptr, PAGES_NUM + 10));
    
    exit();
}
