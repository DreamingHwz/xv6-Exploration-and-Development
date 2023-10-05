#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

#include "udp.h"
#include "fs.h"
#include "mfs.h"

#define BUFFER_SIZE (1000)

// server code
int main(int argc, char *argv[]) {
    struct stat statbuf;
    void* mapped;
    int disksize;
    disk_t disk;
    int fd, sd;

    if (argc < 3) return -1;

    sd = UDP_Open(atoi(argv[1]));
    assert(sd > -1);
    fd = open(argv[2], O_RDWR, 0600);
    if (fd == 0) {
        printf("%s", "image does not exist\n");
        return -1;
    }

    fstat(fd, &statbuf);
    disksize = statbuf.st_size;
    if ((mapped = mmap(NULL, disksize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
        return -1;
    init_disk(mapped, disksize, &disk);

    // printf("fs_lookup:%d\n", fs_lookup(&disk, 1, "bar"));
    // MFS_Stat_t m;
    // fs_stat(&disk, 4, &m);
    // printf("fs_stat:%d %d\n", m.size, m.type);
    // fs_write(&disk, 4, "haha", 0, 2);
    // fs_stat(&disk, 4, &m);
    // printf("fs_stat:%d %d\n", m.size, m.type);
    
    while(1) {
        struct sockaddr_in addr;
        message_t msg;
        // printf("server:: waiting...\n");
        // message_t server_msg;
        int rc = UDP_Read(sd, &addr, (char *)&msg, sizeof(message_t));
        // printf("server:: read message [size:%d contents:(%d)]\n", rc, msg.type);

        if (rc > 0) {
            switch (msg.type) {
                case MFS_INIT:
                    break;

                case MFS_LOOKUP:
                    msg.result = fs_lookup(&disk, msg.inum, msg.name); 
                    break;

                case MFS_STAT:
                    msg.result = fs_stat(&disk, msg.inum, &msg.m); 
                    break;

                case MFS_WRITE:
                    msg.result = fs_write(&disk, msg.inum, msg.buffer, msg.offset, msg.nbytes);
                    fsync(fd); 
                    break;

                case MFS_READ:
                    msg.result = fs_read(&disk, msg.inum, msg.buffer, msg.offset, msg.nbytes);
                    break;

                case MFS_CREAT:
                    msg.result = fs_creat(&disk, msg.inum, msg.type, msg.name);
                    fsync(fd); 
                    break;

                case MFS_UNLINK:
                    msg.result = fs_unlink(&disk, msg.inum, msg.name);
                    fsync(fd); 
                    break;

                case MFS_SHUTDOWN:
                    msg.result = 0;
                    rc = UDP_Write(sd, &addr, (char *)&msg, sizeof(message_t));
                    exit(0);
                    break;
            }

            rc = UDP_Write(sd, &addr, (char *)&msg, sizeof(message_t));
            if (rc < 0) return -1;
            // printf("server:: reply\n");
        }
    }
    
    close(fd);
	close(sd);
    return 0; 
}
    


 