#include "fs.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PA(direct, va) ((direct[(va) >> 12] << 12) | ((va) & 0xfff))

int ls(disk_t *disk, int inode_num) {
    inode_t * inode;
    if (get_inode_ptr(disk, inode_num, &inode) == -1) {
        return -1;
    }
    dir_ent_t* ptr;
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) break;
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        for (int j = 0; j < inode->size/sizeof(dir_ent_t); j ++){
            printf("%d\t%s\n", ptr[j].inum, ptr[j].name);
        }
    }
    printf("\n");
    return 0;
}

// build file structure;
int test_1(disk_t *disk) {
    int foo = fs_creat(disk, 0, MFS_DIRECTORY, "foo");
    foo = fs_lookup(disk, 0, "foo");
    printf("lookup: %d\n", foo);
    int bar = fs_creat(disk, foo, MFS_DIRECTORY, "bar");
    bar = fs_lookup(disk, foo, "bar");
    printf("lookup: %d\n", bar);
    ls(disk, 0);
    ls(disk, foo);
    return (foo == -1) || (bar == -1);
}

// create regular files
int test_2(disk_t *disk) {
    int foo = fs_lookup(disk, 0, "foo");
    int one = fs_creat(disk, foo, MFS_REGULAR_FILE, "one");
    one = fs_lookup(disk, foo, "one");
    ls(disk, 0);
    ls(disk, foo);
    return (foo == -1) || (one == -1);
}

// remove file
int test_3(disk_t *disk) {
    int foo = fs_lookup(disk, 0, "foo");
    int bar = fs_lookup(disk, foo, "bar");
    int two = fs_creat(disk, foo, MFS_REGULAR_FILE, "two");
    two = fs_lookup(disk, foo, "two");
    int rt = fs_unlink(disk, foo, "one");
    int one = fs_creat(disk, bar, MFS_REGULAR_FILE, "one");
    one = fs_lookup(disk, bar, "one");
    ls(disk, 0);
    ls(disk, foo);
    ls(disk, bar);
    return (foo == -1) || (bar == -1) || (two == -1) || (rt == -1) || (one == -1);
}
// write and load file
int test_4(disk_t *disk) {
    int foo = fs_lookup(disk, 0, "foo");
    int two = fs_lookup(disk, foo, "two");
    int rt1 = fs_write(disk, two, "Hello world\n", 0, 13);
    int rt2 = fs_write(disk, two, "file system!\n", 6, 14);
    char buffer[32];
    int rt3 = fs_read(disk, two, buffer, 0, 21);
    printf("%s\n", buffer);
    return (foo == -1) || (two == -1) || (rt1 == -1) || (rt2 == -1) || (rt3 == -1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }
    char *img_name = argv[1];
    int fd;
    if ((fd = open(img_name, O_RDWR, 0600)) == -1) {
        return -1;
    }
    struct stat buf;
    fstat(fd, &buf);

    void *disk_begin = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (disk_begin == MAP_FAILED) {
        return -1;
    }

    // loading file system
    disk_t disk;
    init_disk(disk_begin, buf.st_size, &disk);

    if (test_1(&disk) < 0) {
        printf("test 1 failed!");
        return -1;
    }
    printf("test 1 passed!\n");

    if (test_2(&disk) < 0) {
        printf("test 2 failed!");
        return -1;
    }
    printf("test 2 passed!\n");

    if (test_3(&disk) < 0) {
        printf("test 3 failed!");
        return -1;
    }
    printf("test 3 passed!\n");

    if (test_4(&disk) < 0) {
        printf("test 4 failed!");
        return -1;
    }
    printf("test 4 passed!\n");


    // sync and close
    msync(disk_begin, buf.st_size, MS_SYNC);
    if (munmap(disk_begin, buf.st_size) < 0) {
        return -1;
    }
    close(fd);
    return 0;
}
