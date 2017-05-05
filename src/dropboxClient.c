#include "dropboxClient.h"

#define PORT 4000
#define MAX_MSG_SIZE 1024

int main(int argc, char *argv[])
{
    // init helper
    if (argc < 4) {
        fprintf(stderr,"usage: %s username host port\n", argv[0]);
        return -1;
    }

    // Get client parameters
    char* username = argv[1];
    char* host = argv[2];
    int port = atoi(argv[3]);

    // connect and get the socket
    int client_socket = connect_server(host, port);

    // define command buffer
    byte cmdBuffer[MAX_MSG_SIZE];

	/* read from the socket */
    while(TRUE) {

        // write a new command to server
        printf("> ");
        bzero(cmdBuffer, MAX_MSG_SIZE);
        fgets(cmdBuffer, MAX_MSG_SIZE, stdin);
        // remove \n from of the last character
        cmdBuffer[strlen(cmdBuffer) - 1] = 0;

        // list command write command and wait for response
        if (strcmp(cmdBuffer, "list") == 0) {
            // send command to socket ...
            n = write(client_socket, cmdBuffer, strlen(cmdBuffer));
            if (n < 0) printf("ERROR writing to socket\n");
            // 
            struct file_info file;
            // ... and wait for response
            while(TRUE) {
                // TODO: discover endianess and change to local endianess
                bzero(&file, FILE_INFO_SIZE);
                n = read(client_socket, &file, FILE_INFO_SIZE);
                // if socket was closed, don't lock
                if (n <= 0 || strlen(file.last_modified) == 0) break;
                printf("%d %s %s %s\n", file.size, file.name, file.extension, file.last_modified);
            }
        }
        // exit send command and break
        else if (strcmp(cmdBuffer, "exit") == 0) {
            // send command to socket
            n = write(client_socket, cmdBuffer, strlen(cmdBuffer));
            break;
        }
    }
    
	close(sockfd);
    return 0;
}

int connect_server(char* host, int port) {

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
    if (argc < 2) {
		
        return -1;
    }
	
    // resolve address or ip
	server = gethostbyname(host);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
    // fill sockaddr_in struct to make a connect parameter
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	
    struct sockaddr* saddr_converted = (struct sockaddr*)&serv_addr;
	if (connect(sockfd, saddr_converted, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
	}

    return sockfd
}