## Distributed File System

### Description

This repository includes a distributed file system with a file server containing an on-disk file system and a client library, which offers a range of features:

**Authors:** Skylar Hou, Yuxin Liu

**Overview:**
* `int MFS_Init(char *hostname, int port)`: Find the server exporting the file system.
* `int MFS_Lookup(int pinum, char *name)`: Look up the inode number of an entry by name.
* `int MFS_Stat(int inum, MFS_Stat_t *m)`: Return file information for a given inode.
* `int MFS_Write(int inum, char *buffer, int offset, int nbytes)`: Write data to a file specified by inum.
* `int MFS_Read(int inum, char *buffer, int offset, int nbytes)`: Read data from a file specified by inum to buffer.
* `int MFS_Creat(int pinum, int type, char *name)`: Create a file of a specified type in the parent directory of pinum.
* `int MFS_Unlink(int pinum, char *name)`: Delete a file with a given name in the parent directory of pinum.
* `int MFS_Shutdown()`: Tell the server to exit.