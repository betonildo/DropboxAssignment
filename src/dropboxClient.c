#include "dropboxClient.h"

int client_socket = -1;

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

    // start connection
    client_socket = connect_server(host, port);

    // send user to server
    send_username(username);

    // check connection client-server
    if (!is_connected_with_server()) return -1;

    // define command buffer
    byte cmdBuffer[MAX_MSG_SIZE];

	/* read from the socket */
    while(is_connected()) {

        // write a new command to server
        printf("> ");
        bzero(cmdBuffer, MAX_MSG_SIZE);
        fgets(cmdBuffer, MAX_MSG_SIZE, stdin);
        // remove \n from of the last character
        cmdBuffer[strlen(cmdBuffer) - 1] = 0;

        // only send commands if it is connected
        if (is_connected()) send_command(cmdBuffer);
    }
    
	close(client_socket);
    return 0;
}

int connect_server(char* host, int port) {

    int sockfd = -1;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
    // resolve address or ip
	server = gethostbyname(host);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        perror("ERROR opening socket");
    
    // fill sockaddr_in struct to make a connect parameter
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	
    struct sockaddr* saddr_converted = (struct sockaddr*)&serv_addr;
	if (connect(sockfd, saddr_converted, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
	}

    return sockfd;
}

void send_username(char* username) {
    int usernameLength = strlen(username);
    int bytesWritten = write(client_socket, username, usernameLength);
    if (bytesWritten != usernameLength) printf("username was not properly sent\n");
}

int is_connected_with_server() {
    char response[256];
    int bytesRead = read(client_socket, response, sizeof(response));
    if (strcmp(response, "connected") == 0) {
        printf("%s\n", response);
        return true;
    }
    else {
        printf("%s\n", response);
        return false;
    }
}

void send_command(char* cmdBuffer) {// EXTRA, manage commands
    
    // list command write command and wait for response
    if (strcmp(cmdBuffer, "list") == 0) list_files();
    // exit send command and break
    else if (strcmp(cmdBuffer, "exit") == 0) close_connection();
    else client_help();
}



void list_files() {
    // connect and get the socket
    int bytesRead;

    // send command to socket ...
    char cmd[] = "list";
    bytesRead = write(client_socket, cmd, sizeof(cmd));
    if (bytesRead < 0) printf("ERROR writing to socket\n");
    // 
    struct file_info file;
    // ... and wait for response
    while(TRUE) {
        // TODO: discover endianess and change to local endianess
        bzero(&file, FILE_INFO_SIZE);
        bytesRead = read(client_socket, &file, FILE_INFO_SIZE);
        // if socket was closed, don't lock
        if (bytesRead <= 0 || strlen(file.last_modified) == 0) break;
        printf("%d %s %s %s\n", file.size, file.name, file.extension, file.last_modified);
    }
}

void sync_client() {

}

void send_file(char* filename) {

}

void get_file(char* filename) {

}

/*
Close socket file descriptor only if a connection was stablished
*/
void close_connection() {
    if (client_socket >= 0) close(client_socket);
    // set socket to -1 helps identify that a connection already was closed
    client_socket = -1;
}

/*
Check connection status, 
*/
int is_connected() {

    // if disconnection is already known don't execute anything
    if (client_socket < 0) return 0;

    int error_code;
    int error_code_size = sizeof(error_code);
    int status = getsockopt(client_socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    if (status != 0) {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n", strerror(status));
    }

    if (status != 0) {
        /* socket has a non zero error status */
        fprintf(stderr, "socket error: %s\n", strerror(error_code));
    }

    // 0 is connection status OK
    return status == 0;
}