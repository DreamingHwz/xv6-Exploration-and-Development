#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#define PGSIZE 4096


int 
main(void){
    const uint PAGES_NUM = 5;
    
    // Allocate 5 pages
    char *ptr = sbrk(PGSIZE * PAGES_NUM * sizeof(char));
    munprotect(ptr, PAGES_NUM);
    ptr[PGSIZE * 3] = 0xAA;
    printf(1, "XV6_TEST_OUTPUT TEST PASS\n");
    exit();
}
