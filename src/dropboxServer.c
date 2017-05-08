#include "dropboxServer.h"

int sockfd;
struct node* clientslist;
pthread_mutex_t clients_mutex;

void* receiveCommand(void* clientSocket);
struct file_list getUsersFileList(char* username);
struct file_list push_file_info(struct file_list list, struct file_info file);
struct file_info createFileInfoFromStats(struct dirent* entry, struct stat* fileStat);
void signal_callback_handler(int signum);

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: %s <port>\n", argv[0]);
		return -1;
	}

	// register SIGINT 
	signal(SIGINT, signal_callback_handler);
	signal(SIGSEGV, signal_callback_handler);
	signal(SIGQUIT, signal_callback_handler);

	int port = atoi(argv[1]);

	// first thing create users dir
	create_users_dir();

	// start clients linked list
	clientslist = make_node();
	
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR opening socket\n");
		exit(errno);
	}

	// reuse address and port
	int option = 2;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding\n");
		exit(errno);
	}
	
	listen(sockfd, MAX_CLIENTS_THREADS);
	printf("listening on port: %d\n", port);

	socklen_t clilen = sizeof(struct sockaddr_in);

	while (TRUE) {
		int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
		if (newsockfd >= 0) {
			pthread_t thread;
			pthread_create(&thread, NULL, receiveCommand, (void*)&newsockfd);
		}
	}

	close(sockfd);
	pthread_exit(0);
	return 0;
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum)
{
   printf("Caught signal %d\n",signum);
   // Cleanup and close up stuff here
   close(sockfd);
   // Terminate program
   exit(signum);
}

void create_users_dir() {
	// opendir()
	DIR* dir = opendir(USERS_DIR);
	// dir already exist
	if (dir) closedir(dir);
	// Directory does not exist, so create it with own user permition permition
	else if (ENOENT == errno) {
		// create dir and store status of creation
		int status = mkdir(USERS_DIR, 0700);
		// check for error
		if (status < 0) {
			perror("mkdir() error");
			exit(errno);
		}
	}
	/* opendir() failed for some other reason. */
	else {
		perror("opendir() error");
		exit(errno);
	}
}

void* receiveCommand(void* clientSocket) {
	
	int newsockfd = *(int*)clientSocket;

	// receive user name
	char username[256] = "";
	int usernameLength = read(newsockfd, username, sizeof(username));
	if (!user_dir_exist(username)) create_userhome_dir(username);

	
	// allocate client structure and insert it on connected clients
	struct node* clientnode = NULL;
	bool is_connected = false;
	
	// search on linked list must be locked because is a shared resource
	SCOPELOCK(&clients_mutex, {
		clientnode = find_node_by_userid(clientslist, username);
		if (clientnode == NULL) {
			clientnode = make_node();
		}

		if (clientnode->client == NULL) {
			clientnode->client = make_client(username);
			insert_client(clientslist, clientnode->client);
			printf("username: %s\n", clientnode->client->userid);
		}
		
		is_connected = try_connect_client_device(clientnode, newsockfd);
		attach_thread_to_clientnode(clientnode, pthread_self());

		printf("username: %s\n", clientnode->client->userid);
		printf("socketfd: %d\n", get_right_socket_by_thread_index(clientslist, pthread_self()));
	});


	// send response to client
	if (is_connected) {
		char response[] = "connected";
		int bytesWritten = write(newsockfd, response, sizeof(response));
	}
	else {
		// exit thread
		char response[] = "devices connected limit reached";
		int bytesWritten = write(newsockfd, response, sizeof(response));
		return;
	}

	// define command buffer
	char cmd[1024] = "";
	
	/* wait for commands */
	while(TRUE) {
		int bytesRead = read(newsockfd, cmd, sizeof(cmd));
		// socket was closed
		if (bytesRead == 0) break;
		//TODO: handle problem occured
		else if (bytesRead < 0) break;
		// TODO: detect if the socket is disconnected to close this thread
		else if (bytesRead > 0) {			
			if (strcmp(cmd, "list") == 0) list_files();			
			if (strcmp(cmd, "exit") == 0) break;
			bzero(cmd, sizeof(cmd));
		}
	}

	// if client don't have a device connected
	// remove it from
	if (!client_has_more_than_one_device_connected(clientnode)) {
		SCOPELOCK(&clients_mutex, {	
			remove_client(clientslist, clientnode->client);
		});
	}
	else {
		printf("Disconnecting: %d\n", newsockfd);
		disconnect_client_device(clientnode, newsockfd);
	}
	close(newsockfd);
}

int user_dir_exist(char* username) {

	// create string containing: users/<username>/
	char userhome[512] = "";
	sprintf(userhome, "%s%s/", USERS_DIR, username);

	DIR* dir = opendir(userhome);
	// dir already exist
	if (dir) closedir(dir);
	// Directory does not exist, so create it with own user permition permition
	else if (ENOENT == errno) return FALSE;
	/* opendir() failed for some other reason. */
	else {
		perror("opendir() error");
		exit(errno);
	}

	return TRUE;
}

void create_userhome_dir(char* username) {
	// create string containing: users/<username>/
	char userhome[512] = "";
	sprintf(userhome, "%s%s/", USERS_DIR, username);

	// create dir and store status of creation
	int status = mkdir(userhome, 0700);
	// check for error
	if (status < 0) {
		perror("mkdir() error");
		exit(errno);
	}
}

void list_files() {
	
	struct node* clientnode = find_node_by_threadid(clientslist, pthread_self());
	int socketfd = get_right_socket_by_thread_index(clientnode, pthread_self());
	struct file_list list = getUsersFileList(clientnode->client->userid);
	uint i;
	for (i = 0; i < list.length; i++) {
		void* filePointer = (void*)&list.files[i];
		uint bytesWriten = write(socketfd, filePointer, FILE_INFO_SIZE);
		// TODO: monitor if problem occured
	}
	// close list request (send zeros)
	struct file_info file;
	bzero(&file, FILE_INFO_SIZE);
	write(socketfd, &file, FILE_INFO_SIZE);
	
	// clear memory for list of files
	free(list.files);
}

struct file_list getUsersFileList(char* username) {
	
	char userHome[256] = "";	
	sprintf(userHome, "%s%s/", USERS_DIR, username);
	
	printf("USERDIR: '%s'\n", userHome);
	
	struct file_list list;
	bzero(&list, FILE_INFO_SIZE);
	
	DIR* dir = opendir(userHome);
	

	if (dir == NULL) perror("opendir() error");
	else {
		struct dirent* entry = readdir(dir);
		for (;entry != NULL; entry = readdir(dir)) {

			// create a file stat with empty memory
			struct stat fileStat;
			bzero(&fileStat, sizeof(fileStat));

			char filePath[256];
			uint copyLength = strlen(userHome) + strlen(entry->d_name) + 1;
			
			// concatenate user path with file name to get it's status
			sprintf(filePath, "%s%s", userHome, entry->d_name);			

			// get file stats
			if (stat(filePath, &fileStat) == -1) {
				perror("Error get file stats");
				break;
			}

			// only work with files
			int isFile = fileStat.st_mode & S_IFMT & S_IFREG;
			if (isFile) {
				// entry we want a files list
				// using stack to reserve the value, it uses the same memory area
				// so strcat will use the value non-zeroed value
				struct file_info file = createFileInfoFromStats(entry, &fileStat);
				list = push_file_info(list, file);
			}
		}
	}

	// close dir if it is open
	if (dir != NULL) closedir(dir);
	return list;
}

struct file_list push_file_info(struct file_list list, struct file_info file) {
	uint newLength = sizeof(struct file_info) * (list.length + 1);
	list.files = (struct file_info*) realloc(list.files, newLength);
	list.files[list.length] = file;
	list.length++;
	return list;
}

struct file_info createFileInfoFromStats(struct dirent* entry, struct stat* fileStat) {
	
	struct file_info file;
	
	bzero((void*)&file, sizeof(file));
	// save file name
	strcat(file.name, entry->d_name);
	// save last modified into human readable format
	strcat(file.last_modified, asctime(gmtime(&(*fileStat).st_mtime)));
	// save extension
	char* extStr = strchr(entry->d_name, '.');
	if (extStr != NULL) strcat(file.extension, extStr + 1);
	//save size
	file.size = fileStat->st_size;
	return file;
}