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
#include <signal.h>
// #undef _POSIX_SOURCE

// Mutex scope lock
#define SCOPELOCK(scope_mutex, scope)         \
    {                                         \
        pthread_mutex_lock((scope_mutex));    \
        {scope;};                             \
        pthread_mutex_unlock((scope_mutex));  \
    } \

#define MAXNAME 256
#define MAXFILES 1024
#define MAX_MSG_SIZE 1024
#define NUM_DEVICES 2

#define TRUE 1
#define FALSE 0
#define PORT 4000
#define MAX_CLIENTS_THREADS 5
#define NORMAL_COLOR  "\x1B[0m"
#define GREEN  "\x1B[32m"
#define BLUE  "\x1B[34m"
#define USERS_DIR "users/"
#define USERS_DIR_LENGTH sizeof(USERS_DIR)

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
    int devices[NUM_DEVICES];
    char userid[MAXNAME];
    struct file_info files[MAXFILES];
    int logged_in;
};

typedef enum {
    true = TRUE,
    false = FALSE
} bool;

#define CLIENT_SIZE sizeof(struct client)
#define FILE_INFO_SIZE sizeof(struct file_info)


/*
Show commands help.
*/
void client_help();

// LINKED LIST OF CLIENTS

struct node {
    pthread_t threads[NUM_DEVICES];
    int sockets[NUM_DEVICES];
    struct client* client;
    struct node* next;
    struct node* prev;
};

struct client* make_client(char* username);
bool client_has_more_than_one_device_connected(struct node* clientnode);
bool try_connect_client_device(struct node* clientnode, int socketfd);
void disconnect_client_device(struct node* clientnode, int socketfd);
void attach_thread_to_clientnode(struct node* node, pthread_t tid);
int get_right_socket_by_thread_index(struct node* clientnode, pthread_t tid);
struct node* make_node();
void insert_client(struct node* linkedlist, struct client* client);
void remove_client(struct node* linkedlist, struct client* client);
struct client* find_client_by_userid(struct node* linkedlist, char* userid);
struct node* find_node_by_userid(struct node* linkedlist, char* userid);
struct client* find_client_by_threadid(struct node* linkedlist, pthread_t threadid);
struct node* find_node_by_threadid(struct node* linkedlist, pthread_t threadid);

#endif /*DROPBOXUTIL_H*/