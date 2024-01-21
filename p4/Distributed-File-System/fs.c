/**
 * This fs.c contains functions for managing an on-disk file system image. 
 * The functions include filesystem initialization, writing data to a file, 
 * reading data from a file, creating a new file, deleting an existing file, 
 * and shutting down the filesystem.
 */

#include "fs.h"

/**
 * @brief Initializes the disk structure.
 * 
 * @param mapped A pointer to the memory-mapped region containing the on-disk file system image.
 * @param size The total size of the file system image in bytes.
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @return Returns 0 on success.
 */
int init_disk(void* mapped, int size, disk_t * disk){
    block_t * block_mapped = (block_t *) mapped;
    
    // initialize pointers
    disk->super_ptr = (super_t *)block_mapped;
    disk->inode_bitmap_ptr = (bitmap_t *)(block_mapped + disk->super_ptr->inode_bitmap_addr);
    disk->data_bitmap_ptr = (bitmap_t *)(block_mapped + disk->super_ptr->data_bitmap_addr);
    disk->inode_region_ptr = (inode_block_t *)(block_mapped + disk->super_ptr->inode_region_addr);
    disk->data_region_ptr = block_mapped + disk->super_ptr->data_region_addr;

    // set lengths
    disk->inode_bitmap_len = disk->super_ptr->inode_bitmap_len;
    disk->data_bitmap_len = disk->super_ptr->data_bitmap_len;
    disk->inode_region_len = disk->super_ptr->inode_region_len;
    disk->data_region_len = disk->super_ptr->data_region_len;
    
    // calculate disk length
    disk->disk_len = size / UFS_BLOCK_SIZE;

    return 0;
}

/**
 * @brief Retrieves the inode pointer for a given inode number.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param inum The inode number.
 * @param inode A pointer to the inode_t pointer that will be set to the retrieved inode.
 * @return Returns 0 on success, -1 if the inode is not available.
 */
int get_inode_ptr (disk_t * disk, int inum, inode_t ** inode){
    // calculate the index and bit within the inode bitmap
    int bitint = ((int *)disk->inode_bitmap_ptr)[inum/32];
    bitint >>= (31 - inum%32);
    bitint &= 0x1;

    // check if the inode is available
    if (bitint == 0) {
        return -1;
    }

    // calculate the pointer to the inode
    *inode = ((inode_t *)disk->inode_region_ptr) + inum;

    return 0;
}

/**
 * @brief Finds a free block in the bitmap.
 * 
 * @param bitmap A pointer to the bitmap structure.
 * @param bitmap_len The length of the bitmap in blocks.
 * @return Returns the index of the free block, or -1 if no free block is found.
 */
int find_free_block (bitmap_t * bitmap, int bitmap_len){
    for (int i = 0; i < 32 * bitmap_len * (UFS_BLOCK_SIZE / sizeof(int)); i++){
        // calculate the index and bit within the bitmap
        int bitint = ((int *)bitmap)[i/32];
        bitint >>= (31 - i%32);  
        bitint &= 0x1;

        // if the block is free, return the index of the free block
        if (bitint == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Flips a bit in the bitmap.
 * 
 * @param bitmap A pointer to the bitmap structure.
 * @param i The index of the bit to flip.
 * @return Returns 0 on success.
 */
int flip_bit (bitmap_t * bitmap, int i){
    int bitint = ((int *)bitmap)[i/32];
    bitint >>= (31 - i%32);  
    bitint &= 0x1;

    // filp the bit
    if (bitint == 0) {
        ((int *)bitmap)[i/32] |= 0x1 << (31 - i%32);
    }
    else {
        ((int *)bitmap)[i/32] &= ~(0x1 << (31 - i%32));
    }

    return 0;
}

/**
 * @brief Looks up a file or directory in the filesystem.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param pinum The inode number of the parent directory.
 * @param name The name of the file or directory to look up.
 * @return Returns the inode number if found, or -1 if not found.
 */
int fs_lookup(disk_t * disk, int pinum, char *name){
    inode_t *inode;
    int rc = get_inode_ptr(disk, pinum, &inode);
    
    if (rc == -1) {
        return -1;    // parent inode not found
    }

    dir_ent_t * ptr;
    int inum;

    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) {
            break;
        }

        // calculate the pointer to the directory entries block
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1) {
                continue;       // empty entry
            }

            // compare names and return the inode number if found
            if (strcmp(name, ptr[j].name) == 0){
                inum = ptr[j].inum;
                return inum;
            }
        }
    }

    return -1;
}

/**
 * @brief Retrieves file or directory information from inode.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param inum The inode number.
 * @param m A pointer to the MFS_Stat_t structure to store file or directory information.
 * @return Returns 0 on success, -1 if the inode is not found.
 */
int fs_stat(disk_t * disk, int inum, MFS_Stat_t *m){
    inode_t *inode;
    int rc;
    rc = get_inode_ptr(disk, inum, &inode);

    if (rc == -1) {
        return -1;      // inode not found
    }

    (*m).size = inode->size;
    (*m).type = inode->type;
    
    return 0;
}

/**
 * @brief Writes data to a file in the filesystem.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param inum The inode number of the file.
 * @param buffer A pointer to the buffer containing the data to write.
 * @param offset The offset within the file to start writing.
 * @param nbytes The number of bytes to write.
 * @return Returns 0 on success, -1 on failure.
 */
int fs_write(disk_t * disk, int inum, char *buffer, int offset, int nbytes){
    inode_t *inode;
    int rc = get_inode_ptr(disk, inum, &inode);

    if (rc == -1) {
        return -1;      // invalid inum
    }
    if (nbytes > UFS_BLOCK_SIZE || nbytes < 0) {
        return -1;      // invalid nbytes
    }
    if (inode->type == UFS_DIRECTORY) {
        return -1;      // not a regular file
    }
    if (offset > inode->size * UFS_BLOCK_SIZE || offset < 0) {
        return -1;      // invalid offset
    }

    int block_num = offset/UFS_BLOCK_SIZE;
    int block_addr = inode->direct[block_num];

    if (block_addr == -1){
        // allocate a new block if not already assigned
        int dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
        flip_bit(disk->data_bitmap_ptr, dnum);
        inode->direct[block_num] = dnum;
        block_addr = dnum;
        inode->size += sizeof(dir_ent_t);
    }

    char * block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
    
    if ((offset + nbytes)/UFS_BLOCK_SIZE == block_num){
        // write to the same block
        memcpy(block_ptr + offset%UFS_BLOCK_SIZE, buffer, nbytes);
    }
    else{
        // write to the current block
        memcpy(block_ptr + offset%UFS_BLOCK_SIZE, buffer, UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE);
        
        if (block_num + 1 >= DIRECT_PTRS) {
            return -1;      // no more direct pointers available
        }

        // allocate a new block for the next part of the data
        block_addr = inode->direct[block_num + 1];
        if (block_addr == -1){
            int dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
            flip_bit(disk->data_bitmap_ptr, dnum);
            inode->direct[block_num + 1] = dnum;
            block_addr = dnum;
            inode->size += sizeof(dir_ent_t);
        }

        // update block pointer and copy remaining data
        block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
        memcpy(block_ptr, buffer, nbytes - (UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE));
    }

    return 0;
}

/**
 * @brief Reads data from a file in the filesystem.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param inum The inode number of the file.
 * @param buffer A pointer to the buffer to store the read data.
 * @param offset The offset within the file to start reading.
 * @param nbytes The number of bytes to read.
 * @return Returns 0 on success, -1 on failure.
 */
int fs_read(disk_t * disk, int inum, char *buffer, int offset, int nbytes){
    inode_t *inode;
    int rc = get_inode_ptr(disk, inum, &inode);

    if (rc == -1) {
        return -1;      // invalid inum
    }
    if (nbytes > UFS_BLOCK_SIZE || nbytes < 0) {
        return -1;      // invalid nbytes
    }
    if (offset > inode->size * UFS_BLOCK_SIZE || offset < 0) {
        return -1;      // invalid offset
    }
    if (offset % sizeof(dir_ent_t) != 0 || nbytes % sizeof(dir_ent_t) != 0) {
        return -1;      // invalid alignment
    }

    int block_num = offset / UFS_BLOCK_SIZE;
    int block_addr = inode->direct[block_num];

    char * block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
    
    if ((offset + nbytes)/UFS_BLOCK_SIZE == block_num){
        // read from the same block
        memcpy(buffer, block_ptr + offset%UFS_BLOCK_SIZE, nbytes);  //TODO
    }
    else{
        // read from the current block
        memcpy(buffer, block_ptr + offset%UFS_BLOCK_SIZE, UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE);
        
        if (block_num + 1 >= DIRECT_PTRS) {
            return -1;
        }

        // read from the next block
        block_addr = inode->direct[block_num + 1];
        if (block_addr == -1) {
            return -1;
        }

        // update block pointer and copy remaining data
        block_ptr = (char *)(((block_t *)disk->super_ptr) + block_addr);
        memcpy(buffer, block_ptr, nbytes - (UFS_BLOCK_SIZE - offset%UFS_BLOCK_SIZE));
    }

    return 0;
}

/**
 * @brief Creates a new file or directory in the filesystem.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param pinum The inode number of the parent directory.
 * @param type The type of the new file or directory.
 * @param name The name of the new file or directory.
 * @return Returns 0 on success, -1 on failure.
 */
int fs_creat(disk_t * disk, int pinum, int type, char *name){
    inode_t *inode;
    int rc = get_inode_ptr(disk, pinum, &inode);

    if (rc == -1) {
        return -1;
    }
    if (strlen(name) > 28) {
        return -1;
    }
 
    dir_ent_t * ptr;

    // check if the file or directory already exists
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) {
            break;
        }
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1) {
                continue;
            }
            if (strcmp(name, ptr[j].name) == 0) {
                return 0;
            }
        }
    }

    // find an available block in the inode's direct pointer
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) {
            int dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
            flip_bit(disk->data_bitmap_ptr, dnum);
            inode->direct[i] = dnum;
        }

        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);
        
        // find an available entry in the block
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1 && strcmp(ptr[j].name, "..") != 0) {
                int new_inum;

                // find an available inode
                new_inum = find_free_block(disk->inode_bitmap_ptr, disk->inode_bitmap_len);
                if (new_inum == -1) {
                    return -1; 
                }
                flip_bit(disk->inode_bitmap_ptr, new_inum);

                // update the directory entry
                ptr[j].inum = new_inum;
                strcpy(ptr[j].name, name);

                // get and initialize the new inode
                inode_t * new_inode;
                rc = get_inode_ptr(disk, ptr[j].inum, &new_inode);
                if (rc == -1) {
                    return rc;
                }

                new_inode->type = type;
                new_inode->size = 0;
                for (int k = 0; k < DIRECT_PTRS; k ++){
                    new_inode->direct[k] = -1;
                }

                // if the new inode is a directory, set up "." and ".." entries
                if (new_inode->type == UFS_DIRECTORY){
                    int new_dnum = find_free_block(disk->data_bitmap_ptr, disk->data_bitmap_len);
                    if (new_dnum == -1) {
                        return -1; 
                    }
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

                // update the parent inode size
                inode->size += sizeof(dir_ent_t);

                return 0;
            }
        }
    }

    return -1;      // unable to find an entry
}

/**
 * @brief Unlinks a file or directory from the filesystem.
 * 
 * @param disk A pointer to the disk_t structure representing the file system disk.
 * @param pinum The inode number of the parent directory.
 * @param name The name of the file or directory to unlink.
 * @return Returns 0 on success, -1 on failure.
 */
int fs_unlink(disk_t * disk, int pinum, char *name){
    inode_t *inode;
    int rc = get_inode_ptr(disk, pinum, &inode);\

    if (rc == -1) {
        return -1;
    }
    // if (strlen(name) > 28) {
    //     return -1;
    // }
 
    dir_ent_t * ptr;
    inode_t * cinode;

    // look for the file in the parent directory
    for (int i = 0; i < DIRECT_PTRS; i ++){
        if (inode->direct[i] == -1) {
            break;
        }
        ptr = (dir_ent_t *)((block_t *)disk->super_ptr + inode->direct[i]);

        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(dir_ent_t); j ++){
            if (ptr[j].inum == -1) {
                continue;
            }
            if (strcmp(name, ptr[j].name) == 0) {
                int cinum = ptr[j].inum;
                int dnum;
                
                // get the inode pointer
                rc = get_inode_ptr(disk, cinum, &cinode);
                if (rc == -1) {
                    return -1;
                }

                // check if the child inode is a directory and if it is empty
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

                // deallocate data blocks associated with the child inode
                for (int k = 0; k < DIRECT_PTRS; k ++){
                    if (cinode->direct[k] != -1) {
                        dnum = cinode->direct[k] - disk->super_ptr->data_region_addr;
                        flip_bit(disk->data_bitmap_ptr, dnum);
                    }
                }

                // deallocate the child inode
                flip_bit(disk->inode_bitmap_ptr, cinum);

                // remove the directory entry from the parent directory
                ptr[j].inum = -1;
                inode->size -= sizeof(dir_ent_t);

                return 0;
            }
        }
    }
    
    return -1;
}

/**
 * @brief Shuts down the filesystem.
 * 
 * @param fd The file descriptor of the file system.
 * @param disk A pointer to the disk_t structure representing the file system disk.
 */
int fs_shutdown(int fd, disk_t * disk){
    fsync(fd);
    munmap((void *)disk, disk->disk_len * UFS_BLOCK_SIZE);
    close(fd);
    // exit(0);
}