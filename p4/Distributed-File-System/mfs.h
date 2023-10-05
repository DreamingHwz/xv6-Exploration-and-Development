#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)


#define MFS_INIT (1)
#define MFS_LOOKUP (2)
#define MFS_STAT (3)
#define MFS_WRITE (4)
#define MFS_READ (5)
#define MFS_CREAT (6)
#define MFS_UNLINK (7)
#define MFS_SHUTDOWN (8)


typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;


typedef struct {
    int type;
    int inum;
    int offset;
    int nbytes;
    int ftype;
    int result;
    char name[28];
    char buffer[4096];
    MFS_Stat_t m;
} message_t;


int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int offset, int nbytes);
int MFS_Read(int inum, char *buffer, int offset, int nbytes);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__