#include "dropboxServer.h"

void* receiveCommand(void* clientSocket);
struct file_list getUsersFileList(char* username);
struct file_list push_file_info(struct file_list list, struct file_info file);

block allocateBlock(uint size);
int readFromSockToBlock(int sock, block* b);
void clearBlock(block* b);
struct file_info createFileInfoFromStats(struct dirent* entry, struct stat* fileStat);

int main(int argc, char *argv[])
{
	int sockfd, n;
	socklen_t clilen;
	
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	bzero(&(serv_addr.sin_zero), 8);
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	listen(sockfd, MAX_CLIENTS_THREADS);
	
	clilen = sizeof(struct sockaddr_in);

	while (TRUE) {
		int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd >= 0) {
			pthread_t thread;
			pthread_create(&thread, NULL, receiveCommand, (void*)&newsockfd);
		}
	}

	close(sockfd);
	pthread_exit(0);
	return 0;
}

void* receiveCommand(void* clientSocket) {
	
	int newsockfd = *(int*)clientSocket;
	
	block cmd = allocateBlock(256);
	clearBlock(&cmd);
	
	/* wait for commands */
	while(TRUE) {
		int bytesRead = readFromSockToBlock(newsockfd, &cmd);
		// socket was closed
		if (bytesRead == 0) break;
		//TODO: handle problem occured
		else if (bytesRead < 0) break;

		// TODO: detect if the socket is disconnected to close this thread
		else if (bytesRead > 0) {
			//TODO: read commands from cmd block
			
			// 
			if (strcmp(cmd.data, "list") == 0) {
				printf("'list' command received\n");
				struct file_list list = getUsersFileList("test/");
				uint i;
				for (i = 0; i < list.length; i++) {
					void* filePointer = (void*)&list.files[i];
					uint bytesWriten = write(newsockfd, filePointer, FILE_INFO_SIZE);
					// TODO: monitor if problem occured
				}
				// simulate close
				struct file_info file;
				bzero(&file, FILE_INFO_SIZE);
				write(newsockfd, &file, FILE_INFO_SIZE);
				
				// clear memory for list of files
				free(list.files);
			}
			
			if (strcmp(cmd.data, "exit") == 0) break;
			clearBlock(&cmd);
		}
	}
	
	printf("Closing socket: %d\n", newsockfd);
	close(newsockfd);
	free(cmd.data);
}

struct file_list getUsersFileList(char* username) {
	char home[] = "users/";
	char userHome[256] = "";
	
	sprintf(userHome, "%s%s", home, username);
	
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

block allocateBlock(uint size) {
	block b;
	b.data = (byte*)calloc(size, sizeof(byte));
	b.size = size;
	return b;
}

int readFromSockToBlock(int sock, block* b) {
	return read(sock, b->data, b->size);
}

void clearBlock(block* b) {
	bzero(b->data, b->size);
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