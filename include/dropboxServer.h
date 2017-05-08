//GUARD
#ifndef DROPBOXSERVER_H
#define DROPBOXSERVER_H

#include "dropboxUtil.h"

void list_files();
void create_users_dir();
int user_dir_exist(char* username);
void create_userhome_dir(char* username);

#endif /*DROPBOXSERVER_H*/