#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define TRUE 1
#define FALSE 0
#define PORT 4000
#define MAX_CLIENTS_THREADS 5

void* receiveData(void* clientSocket);
int thread_array_push(pthread_t* threadArray, pthread_t t, int index);

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
			pthread_t thread;
			pthread_create(&thread, NULL, receiveData, (void*)&newsockfd);
			thread_index = thread_array_push(threadArray, thread, thread_index);
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

void* receiveData(void* clientSocket) {
	int newsockfd = *(int*)clientSocket;
	
	char buffer[256];	
	bzero(buffer, 256);
	
	/* read from the socket */
	int n = 1;
	while((n = read(newsockfd, buffer, 256)) > 0) {
		printf("Message size: %d\n", n);
		buffer[256] = 0;
		printf("Here is the message: %s\n", buffer);
		bzero(buffer, 256);
	}
	if (n < 0) printf("ERROR reading from socket");
	
	/* write in the socket */ 
	char* response = "I got your message";
	n = write(newsockfd, response, strlen(response));
	if (n < 0) 
		printf("ERROR writing to socket");

	close(newsockfd);
}
