#include "server_tcp.h"

void* receiveCommand(void* clientSocket);
file_list getUsersFileList(char* userDir);
file_list push_file_info(file_list list, struct file_info file);

int thread_array_push(pthread_t* threadArray, pthread_t t, int index);
block allocateBlock(uint size);
int readFromSockToBlock(int sock, block* b);
void clearBlock(block* b);
char* concat(char* str1, char* str2);

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

void* monitor_users_dir(char* usersDir) {

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
			clearBlock(&cmd);
		}	
	}
	
	printf("Closing socket: %d\n", newsockfd);
	/* write in the socket */
	// n = write(newsockfd, response, strlen(response));
	close(newsockfd);
}

file_list getUsersFileList(char* userDir) {
	char* home = "users/";
	char* finalDir = concat(home, userDir);
	file_list list;
	
	DIR* dir;
	struct dirent* entry;

	if ((dir = opendir(finalDir)) == NULL) {
		perror("opendir() error");
	}
	else {
		while((entry = readdir(dir)) != NULL) {
			// entry we want a files list
			struct file_info file;
			file.name = entry->d_name;
		}
	}

	return list;
}

file_list push_file_info(file_list list, struct file_info file) {
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