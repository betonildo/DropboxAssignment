#include "server_tcp.h"

void* receiveCommand(void* clientSocket);
struct file_list getUsersFileList(char* userDir);
struct file_list push_file_info(struct file_list list, struct file_info file);

int thread_array_push(pthread_t* threadArray, pthread_t t, int index);
block allocateBlock(uint size);
int readFromSockToBlock(int sock, block* b);
void clearBlock(block* b);
char* concat(char* str1, char* str2);
void assignString(char* lhs, char* rhs);
struct file_info createFileInfoFromStats(struct dirent* entry, struct stat* sb);

int main(int argc, char *argv[])
{
	int sockfd, n;
	socklen_t clilen;
	int thread_index = 0;
	pthread_t* threadArray = (pthread_t*)calloc(MAX_CLIENTS_THREADS, sizeof(pthread_t));
	
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
			// n = write(newsockfd, response, strlen(response));
			pthread_t thread;
			pthread_create(&thread, NULL, receiveCommand, (void*)&newsockfd);
			thread_index = thread_array_push(threadArray, thread, thread_index);
		}
		else {
			printf("max number of clients connected\n");
		}
	}

	close(sockfd);
	pthread_exit(0);
	return 0;
}

int thread_array_push(pthread_t* threadArray, pthread_t t, int index) {
	index = index % MAX_CLIENTS_THREADS;
	threadArray[index] = t;
	return ++index;
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
			if (strcmp(cmd.data, "list") == 0) {
				printf("'list' command received\n");
				struct file_list list = getUsersFileList("test/");
				uint i;
				for (i = 0; i < list.length; i++) {
					
					uint bytesWriten = write(newsockfd, (void*)&list.files[i], sizeof(struct file_info));
					// TODO: monitor if problem occured
				}
				// simulate close
				struct file_info file;
				bzero((void*)&file, sizeof(struct file_info));
				write(newsockfd, (void*)&file, sizeof(struct file_info));
			}
			clearBlock(&cmd);
		}	
	}
	
	printf("Closing socket: %d\n", newsockfd);
	/* write in the socket */
	// n = write(newsockfd, response, strlen(response));
	close(newsockfd);
}

struct file_list getUsersFileList(char* userDir) {
	char home[] = "users/";
	char finalDir[256];
	bzero((void*)finalDir, sizeof(finalDir));
	strcat(finalDir, home);
	strcat(finalDir, userDir);
	
	struct file_list list;
	bzero((void*)&list, sizeof(struct file_list));
	
	printf("userdir: %s\n", finalDir);
	DIR* dir;
	struct dirent* entry;

	if ((dir = opendir(finalDir)) == NULL) {
		perror("opendir() error");
	}
	else {
		while((entry = readdir(dir)) != NULL) {
			
			// only process files
			if(entry->d_type == DT_REG) {
				
				struct stat sb;
				bzero((void*)&sb, sizeof(sb));
				
				char filePath[256];
				bzero((void*)filePath, sizeof(filePath));
				strcat(filePath, finalDir);
				strcat(filePath, entry->d_name);
				
				if (stat(filePath, &sb) == -1) {
					perror("Error get file stats\n");
					break;
				}
				
				
				switch (sb.st_mode & S_IFMT) {
				case S_IFBLK:  printf("block device\n");            break;
				case S_IFCHR:  printf("character device\n");        break;
				case S_IFDIR:  printf("directory\n");               break;
				case S_IFIFO:  printf("FIFO/pipe\n");               break;
				case S_IFLNK:  printf("symlink\n");                 break;
				case S_IFREG:{
						
						// entry we want a files list
						// using stack to reserve the value, it uses the same memory area
						// so strcat will use the value non-zeroed value
						struct file_info file = createFileInfoFromStats(entry, &sb);
						list = push_file_info(list, file);
					}
					break;
				case S_IFSOCK: printf("socket\n");                  break;
				default:       printf("unknown?\n");                break;
			    }
				
			
			}
		}
	}

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

char* concat(char* str1, char* str2) {
    int strTotalLength = strlen(str1) + strlen(str2) + 1;
    char* newstr = (char*) calloc(strTotalLength, sizeof(char));
    strcat(newstr, str1);
    strcat(newstr, str2);
    return newstr;
}

void assignString(char* lhs, char* rhs) {
	uint newSize = strlen(rhs) + 2;
	while(--newSize) lhs[newSize] = rhs[newSize];
}

struct file_info createFileInfoFromStats(struct dirent* entry, struct stat* sb) {
	
	struct file_info file;
	
	bzero((void*)&file, sizeof(file));
	// save file name
	strcat(file.name, entry->d_name);
	// save last modified into human readable format
	strcat(file.last_modified, asctime(gmtime(&(*sb).st_mtime)));
	// save extension
	char* extStr = strchr(entry->d_name, '.');
	if (extStr != NULL) strcat(file.extension, extStr + 1);
	//save size
	file.size = sb->st_size;
	return file;
}