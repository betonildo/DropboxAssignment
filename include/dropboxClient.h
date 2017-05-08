#ifndef DROPBOXCLIENT_H
#define DROPBOXCLIENT_H

#include "dropboxUtil.h"

int connect_server(char* host, int port);
void send_username(char* username);
int is_connected_with_server();
void send_command(char* cmd); // EXTRA, manage commands
void list_files();
void sync_client();
void send_file(char* filename);
void get_file(char* filename);
void close_connection();

/*
Check connection status. Returns true or false
*/
int is_connected();

#endif /*DROPBOXCLIENT_H*/