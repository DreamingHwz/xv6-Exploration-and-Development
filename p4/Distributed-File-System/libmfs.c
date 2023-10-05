#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mfs.h"
#include "udp.h"


int sd, rc;
struct sockaddr_in addrSnd;

int get_port_num() {
    int MIN_PORT = 20000;
    int MAX_PORT = 40000;

    srand(time(0));

    return (rand() % (MAX_PORT - MIN_PORT) + MIN_PORT);
}


int MFS_Init(char *hostname, int port) {
    message_t *msg = malloc(sizeof(message_t));
    int port_num = get_port_num();

    sd = UDP_Open(port_num);
    if (sd < 0)
        return -1;

    rc = UDP_FillSockAddr(&addrSnd, hostname, port);
    if (rc < 0)
        return -1;

    msg->type = MFS_INIT;

    return rc;
}

int MFS_Lookup(int pinum, char *name) {
    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_LOOKUP;
    msg->inum = pinum;

    strcpy(msg->name, name);

    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    return msg->result;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_STAT;
    msg->inum = inum;

    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    m->type = msg->m.type;
    m->size = msg->m.size;
    
    return msg->result;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes) {
    if (inum < 0 || nbytes < 0 || nbytes > 4096 || buffer == NULL || offset < 0)
        return -1;

    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_WRITE;
    msg->inum = inum;
    msg->offset = offset;
    msg->nbytes = nbytes;

    memcpy(msg->buffer, buffer, nbytes);
    
    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    return msg->result;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes) {
    if (inum < 0 || nbytes < 0 || nbytes > 4096 || buffer == NULL || offset < 0)
        return -1;

    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_READ;
    msg->inum = inum;
    msg->offset = offset;
    msg->nbytes = nbytes;
    
    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    memcpy(buffer, msg->buffer,nbytes);

    return msg->result;
}

int MFS_Creat(int pinum, int type, char *name) {
    if (pinum < 0 || strlen(name) > 28)
        return -1;

    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_CREAT;
    msg->inum = pinum;
    msg->ftype = type;

    strcpy(msg->name, name);
    
    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    return msg->result;
}

int MFS_Unlink(int pinum, char *name) {
    if (pinum < 0)
        return -1;

    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_UNLINK;
    msg->inum = pinum;

    strcpy(msg->name, name);
    
    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0)
        return -1;

    return msg->result;
}

int MFS_Shutdown() {
    message_t *msg = malloc(sizeof(message_t));
    msg->type = MFS_SHUTDOWN;

    rc = UDP_Write(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0) {
        assert(rc == 0);
        free(msg);
        close(sd);
        return -1;
    }

    rc = UDP_Read(sd, &addrSnd, (char *)msg, sizeof(message_t));
    if (rc < 0) {
        assert(rc == 0);
        free(msg);
        close(sd);
        return -1;
    }

    rc = msg->result;
    assert(rc == 0);
    free(msg);
    close(sd);
    return rc;
}
