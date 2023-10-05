#include "fs.h"


int init_disk(void* mapped, int size, disk_t * disk){
    block_t * block_mapped = (block_t *) mapped;
    
    disk->super_ptr = (super_t *)block_mapped;
    disk->inode_bitmap_ptr = (bitmap_t *)(block_mapped + disk->super_ptr->inode_bitmap_addr);
    disk->data_bitmap_ptr = (bitmap_t *)(block_mapped + disk->super_ptr->data_bitmap_addr);
    disk->inode_region_ptr = (inode_block_t *)(block_mapped + disk->super_ptr->inode_region_addr);
    disk->data_region_ptr = block_mapped + disk->super_ptr->data_region_addr;

    disk->inode_bitmap_len = disk->super_ptr->inode_bitmap_len;
    disk->data_bitmap_len = disk->super_ptr->data_bitmap_len;
    disk->inode_region_len = disk->super_ptr->inode_region_len;
    disk->data_region_len = disk->super_ptr->data_region_len;
    
    disk->disk_len = size/UFS_BLOCK_SIZE;
}

int get_inode_ptr (disk_t * disk, int inum, inode_t ** inode){
    int bitint = ((int *)disk->inode_bitmap_ptr)[inum/32];
    bitint >>= (31 - inum%32);
    bitint &= 0x1;
    if (bitint == 0) return -1;
    *inode = ((inode_t *)disk->inode_region_ptr) + inum;
    return 0;
}

int find_free_block (bitmap_t * bitmap, int bitmap_len){
    int i = 0;
    for (; i < 32 * bitmap_len * (UFS_BLOCK_SIZE / sizeof(int)); i++){
        int bitint = ((int *)bitmap)[i/32];
        bitint >>= (31 - i%32);  
        bitint &= 0x1;
        if (bitint == 0) return i;
    }
    return -1;
}

int flip_bit (bitmap_t * bitmap, int i){
    int bitint = ((int *)bitmap)[i/32];
    bitint >>= (31 - i%32);  
    bitint &= 0x1;
    if (bitint == 0) {
        ((int *)bitmap)[i/32] |= 0x1 << (31 - i%32);
    }
    else {
        ((int *)bitmap)[i/32] &= ~(0x1 << (31 - i%32));
    }
    return 0;
}

int fs_lookup(disk_t * disk, int pinum, char *name){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, pinum, &inode);
    if (rc == -1) return -1;

    int i = 0;
    dir_ent_t * ptr;
    int inum;
    for (; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) break;
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1) continue;
            if (strcmp(name, ptr[j].name) == 0){
                inum = ptr[j].inum;
                return inum;
            }
        }
    }

    return -1;
}

int fs_stat(disk_t * disk, int inum, MFS_Stat_t *m){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, inum, &inode);
    if (rc == -1) return -1;

    (*m).size = inode->size;
    (*m).type = inode->type;
    
    return 0;
}

int fs_write(disk_t * disk, int inum, char *buffer, int offset, int nbytes){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, inum, &inode);

    if (rc == -1) return -1;                            // invalid inum
    if (nbytes > 4096 || nbytes < 0) return -1;         // invalid nbytes
    if (inode->type == UFS_DIRECTORY) return -1;         // not a regular file
    if (offset > inode->size * UFS_BLOCK_SIZE || offset < 0) return -1;   // invalid offset

    int block_num = offset/UFS_BLOCK_SIZE;
    int block_addr = inode->direct[block_num];

    if (block_addr == -1){
        int dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
        flip_bit(disk->data_bitmap_ptr, dnum);
        inode->direct[block_num] = dnum;
        block_addr = dnum;
        inode->size += sizeof(dir_ent_t);
    }

    char * block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
    
    if ((offset + nbytes)/UFS_BLOCK_SIZE == block_num){
        memcpy(block_ptr + offset%UFS_BLOCK_SIZE, buffer, nbytes);
    }
    else{
        memcpy(block_ptr + offset%UFS_BLOCK_SIZE, buffer, UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE);
        
        if (block_num + 1 >= DIRECT_PTRS) return -1;
        block_addr = inode->direct[block_num + 1];
        if (block_addr == -1){
            int dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
            flip_bit(disk->data_bitmap_ptr, dnum);
            inode->direct[block_num + 1] = dnum;
            block_addr = dnum;
            inode->size += sizeof(dir_ent_t);
        }
        block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
        memcpy(block_ptr, buffer, nbytes - (UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE));
    }

    return 0;
}

int fs_read(disk_t * disk, int inum, char *buffer, int offset, int nbytes){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, inum, &inode);

    if (rc == -1) return -1;                            // invalid inum
    if (nbytes > 4096 || nbytes < 0) return -1;         // invalid nbytes
    if (offset > inode->size || offset < 0) return -1;   // invalid offset
    if (offset % sizeof(dir_ent_t) != 0 || nbytes % sizeof(dir_ent_t) != 0)
        return -1;

    int block_num = offset/UFS_BLOCK_SIZE;
    int block_addr = inode->direct[block_num];

    char * block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
    
    if ((offset + nbytes)/UFS_BLOCK_SIZE == block_num){
        memcpy(buffer, block_ptr + offset%UFS_BLOCK_SIZE, nbytes);  //TODO
    }
    else{
        memcpy(buffer, block_ptr + offset%UFS_BLOCK_SIZE, UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE);
        
        if (block_num + 1 >= DIRECT_PTRS) return -1;
        block_addr = inode->direct[block_num + 1];
        if (block_addr == -1) return -1;
        block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
        memcpy(buffer, block_ptr, nbytes - (UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE));
    }
    return 0;
}

int fs_creat(disk_t * disk, int pinum, int type, char *name){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, pinum, &inode);
    if (rc == -1) return -1;
    if (strlen(name) > 28) return -1;
 
    dir_ent_t * ptr;
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) break;
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1) continue;
            if (strcmp(name, ptr[j].name) == 0) return 0;
        }
    }
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) {
            int dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
            flip_bit(disk->data_bitmap_ptr, dnum);
            inode->direct[i] = dnum;
        }
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1 && strcmp(ptr[j].name, "..") != 0) {
                int new_inum;

                new_inum = find_free_block(disk->inode_bitmap_ptr, disk->inode_bitmap_len);
                if (new_inum == -1) return -1; 
                flip_bit(disk->inode_bitmap_ptr, new_inum);
                ptr[j].inum = new_inum;
                strcpy(ptr[j].name, name);

                inode_t * new_inode;
                rc = get_inode_ptr(disk, ptr[j].inum, &new_inode);
                if (rc == -1) return rc;

                new_inode->type = type;
                new_inode->size = 0;
                for (int k = 0; k < DIRECT_PTRS; k ++){
                    new_inode->direct[k] = -1;
                }
                if (new_inode->type == UFS_DIRECTORY){
                    int new_dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
                    if (new_dnum == -1) return -1; 
                    flip_bit(disk->data_bitmap_ptr, new_dnum);

                    new_inode->size = 2 * sizeof(dir_ent_t);
                    new_inode->direct[0] = new_dnum + disk->super_ptr->data_region_addr;

                    dir_ent_t * entry_ptr = (dir_ent_t *)((block_t *)disk->super_ptr + new_inode->direct[0]); 

                    strcpy(entry_ptr->name, ".");
                    entry_ptr->inum = new_inum;

                    strcpy((entry_ptr + 1)->name, "..");
                    (entry_ptr + 1)->inum = pinum;

                    for (int t = 2; t < UFS_BLOCK_SIZE / sizeof(dir_ent_t); t ++)
                        (entry_ptr + t)->inum = -1;
                }
                inode->size += sizeof(dir_ent_t);
                return 0;
            }
        }
    }
    return -1;
}

int fs_unlink(disk_t * disk, int pinum, char *name){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, pinum, &inode);
    if (rc == -1) return -1;
    // if (strlen(name) > 28) return -1;
 
    dir_ent_t * ptr;
    inode_t * cinode;
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) break;
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);

        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1) continue;
            if (strcmp(name, ptr[j].name) == 0) {
                int cinum = ptr[j].inum;
                int dnum;
                rc = get_inode_ptr(disk, cinum, &cinode);
                if (rc == -1) return -1;

                if (cinode->type == UFS_DIRECTORY){
                    dir_ent_t * pptr;
                    for (int k = 0; k < DIRECT_PTRS; k ++){
                        if (cinode->direct[k] != -1) {
                            pptr = (dir_ent_t *)((block_t *)disk->super_ptr + cinode->direct[k]);
                            for (int t = 0; t < UFS_BLOCK_SIZE / sizeof(dir_ent_t); t ++){
                                if (pptr[t].inum != -1) return -1;
                            }
                        }
                    }
                }
                for (int k = 0; k < DIRECT_PTRS; k ++){
                    if (cinode->direct[k] != -1) {
                        dnum = cinode->direct[k] - disk->super_ptr->data_region_addr;
                        flip_bit(disk->data_bitmap_ptr, dnum);
                    }
                }
                flip_bit(disk->inode_bitmap_ptr, cinum);
                ptr[j].inum = -1;
                inode->size -= sizeof(dir_ent_t);
            }
        }
    }
    
    return 0;
}

int fs_shutdown(int fd, disk_t * disk){
    fsync(fd);
    munmap((void *)disk, disk->disk_len * UFS_BLOCK_SIZE);
    close(fd);
    // exit(0);
}