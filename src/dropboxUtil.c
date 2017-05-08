#include "dropboxUtil.h"

/*
Check connection status. Returns true or false
*/
void client_help() {
    printf("%scommands available:\n", GREEN);
    printf("%supload   <path/filename.ext> : Sends a file to users directory\n", GREEN);
    printf("%sdownload <filename.exe>      : Sends a file to users directory\n", GREEN);
    printf("%slist                         : List all files on users directory\n", GREEN);
    printf("%sget_sync_dir                 : Synchronize users directory with local /home/sync_dir_<username>\n", GREEN);
    printf("%sexit                         : Close connection and exit the client\n", GREEN);
    printf("\n\n");
}

struct client* make_client(char* username) {
    struct client* client = (struct client*)malloc(sizeof(struct client));
    bzero(client->userid, sizeof(client->userid));
    bzero(client->files, sizeof(client->files));
    sprintf(client->userid, "%s", username);
    int i;
    for (i = 0; i < NUM_DEVICES; i++) {
        client->devices[i] = false;
    }
    return client;
}

bool client_has_more_than_one_device_connected(struct node* clientnode) {
    int i, count = 0;

    for (i = 0; i < NUM_DEVICES; i++)
        if (clientnode->sockets[i] > 0) count++;

    if (count > 1) return true;
    return false;
}

bool try_connect_client_device(struct node* clientnode, int socketfd) {
    // find empty device and fill it
    int i;
    for (i = 0; i < NUM_DEVICES; i++) {
        if (clientnode->sockets[i] < 0) {
            clientnode->sockets[i] = socketfd;
            return true;
        }
    }

    return false;
}

void disconnect_client_device(struct node* clientnode, int socketfd) {
    int i;
    for (i = 0; i < NUM_DEVICES; i++)
        if (clientnode->sockets[i] == socketfd) 
            clientnode->sockets[i] = -1;
}

void attach_thread_to_clientnode(struct node* node, pthread_t tid) {
    int i;
    for (i = 0; i < NUM_DEVICES; i++)
        if (node->threads[i] == 0)
            node->threads[i] = tid;
}

int get_right_socket_by_thread_index(struct node* clientnode, pthread_t tid) {
    int i;
    for (i = 0; i < NUM_DEVICES; i++)
        if (clientnode->threads[i] == tid)
            return clientnode->sockets[i];

    return -1;
}

struct node* make_node() {
    struct node* node = (struct node*)malloc(sizeof(struct node));
    // creates a node with thread's id
    // node->threadid = pthread_self();
    node->next = NULL;
    node->prev = NULL;
    node->client = NULL;
    int i;
    for (i = 0; i < NUM_DEVICES; i++) {
        node->threads[i] = 0;
        node->sockets[i] = -1;
    }
    return node;
}

void insert_client(struct node* linkedlist, struct client* client) {
    
    struct node* next = make_node();
    next->client = client;
    next->prev = linkedlist;
    next->next = linkedlist->next;
    linkedlist->next = next;
}

void remove_client(struct node* linkedlist, struct client* client) {
    struct node* found = find_node_by_userid(linkedlist, client->userid);

    if (!found) return;
    if (found == linkedlist) {
        free(linkedlist->client);
        linkedlist->client = NULL;
        return;
    }

    if (found->next) found->next->prev = found->prev;
    if (found->prev) found->prev->next = found->next;

    free(found->client);
    free(found);
}

struct client* find_client_by_userid(struct node* linkedlist, char* userid) {
    struct node* node = find_node_by_userid(linkedlist, userid);
    if (node) return node->client;
    else return NULL;
}

struct node* find_node_by_userid(struct node* linkedlist, char* userid) {
    struct node* n = linkedlist;
    while(n != NULL && n->client != NULL && strcmp(n->client->userid, userid) != 0)
        n = n->next;
    return n;
}

struct client* find_client_by_threadid(struct node* linkedlist, pthread_t threadid) {
    struct node* node = find_node_by_threadid(linkedlist, threadid);
    if (node) return node->client;
    else NULL;
}

struct node* find_node_by_threadid(struct node* linkedlist, pthread_t threadid) {
    struct node* n = linkedlist;
    int i;
    bool found = false;

    while(n != NULL && n->client != NULL) {

        for (i = 0; i < NUM_DEVICES; i++)
            if (n->threads[i] == threadid) 
                found = true;

        if (!found) n = n->next;
        else break;
    }
    
    return n;
}