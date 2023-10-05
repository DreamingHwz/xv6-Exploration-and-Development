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
    mprotect(ptr, PAGES_NUM);
    int ppid = getpid();

    if (fork() == 0) {
        // Should page fault as normally here
        ptr[2 * PGSIZE] = 0xAA;
        printf(1, "XV6_TEST_OUTPUT Seg fault failed to trigger\n");
        // Shouldn't reach here
        kill(ppid);
        exit();
    } else {
        wait();
    }

    printf(1, "XV6_TEST_OUTPUT TEST PASS\n");

    exit();
}
