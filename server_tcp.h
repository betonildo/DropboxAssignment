//GUARD
#ifndef SERVER_TCP_H
#define SERVER_TCP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define _POSIX_SOURCE
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#undef _POSIX_SOURCE

#define TRUE 1
#define FALSE 0
#define PORT 4000
#define MAX_CLIENTS_THREADS 5
#define MAXNAME 1024;

typedef unsigned char byte;
typedef unsigned int  uint;

typedef struct {
	byte* data;
	uint  size;
} block;

typedef block string_t;

struct file_info {
    char name[MAXNAME];
    char extension[MAXNAME];
    char last_modified[MAXNAME];
    int size;
};

struct file_list {
    struct file_info* files;
    int length;
};

#endif /*SERVER_TCP_H*/