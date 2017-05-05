// guard
#ifndef DROPBOXUTIL_H
#define DROPBOXUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
// #define _POSIX_SOURCE
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// #undef _POSIX_SOURCE

#define MAXNAME 256
#define MAXFILES 1024

#define TRUE 1
#define FALSE 0
#define PORT 4000
#define MAX_CLIENTS_THREADS 5
#define NORMAL_COLOR  "\x1B[0m"
#define GREEN  "\x1B[32m"
#define BLUE  "\x1B[34m"

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

struct client {
    int devices[2];
    char userid[MAXNAME];
    struct file_info files[MAXFILES];
    int logged_in;
};

#define CLIENT_SIZE sizeof(struct client)
#define FILE_INFO_SIZE sizeof(struct file_info)

#endif /*DROPBOXUTIL_H*/