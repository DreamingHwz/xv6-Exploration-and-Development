#ifndef __fs_h__
#define __fs_h__

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ufs.h"
#include "mfs.h"
#include <string.h>
#include <sys/mman.h>

#define DIRECT_PTRS (30)

typedef struct __block{
    int block[1024];
} block_t;

typedef struct {
	int bits[UFS_BLOCK_SIZE / sizeof(int)];
} bitmap_t;

typedef struct {
	inode_t inodes[UFS_BLOCK_SIZE / sizeof(inode_t)];
} inode_block_t;

typedef struct {
	dir_ent_t entries[UFS_BLOCK_SIZE / sizeof(dir_ent_t)];
} dir_block_t;

// presumed: block 0 is the super block
typedef struct __disk {
    super_t* super_ptr;    // super block address
    bitmap_t* inode_bitmap_ptr; // block address (in blocks)
    int inode_bitmap_len;  // in blocks
    bitmap_t* data_bitmap_ptr;  // block address (in blocks)
    int data_bitmap_len;   // in blocks
    inode_block_t* inode_region_ptr; // block address (in blocks)
    int inode_region_len;  // in blocks
    block_t* data_region_ptr;  // block address (in blocks)
    int data_region_len;   // in blocks
    int disk_len;          // in blocks
} disk_t;


int init_disk(void* mapped, int size, disk_t * disk);
int fs_lookup(disk_t * disk, int pinum, char *name);
int fs_stat(disk_t * disk, int inum, MFS_Stat_t *m);
int fs_write(disk_t * disk, int inum, char *buffer, int offset, int nbytes);
int fs_read(disk_t * disk, int inum, char *buffer, int offset, int nbytes);
int fs_creat(disk_t * disk, int pinum, int type, char *name);
int fs_unlink(disk_t * disk, int pinum, char *name);
int fs_shutdown(int fd, disk_t * disk);
int get_inode_ptr (disk_t * disk, int inum, inode_t ** inode);
int find_free_block (bitmap_t * bitmap, int bitmap_len);
int flip_bit (bitmap_t * bitmap, int i);

#endif // __ufs_h__