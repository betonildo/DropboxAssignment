#include "utils.h"

#define PORT 4000
#define MAX_MSG_SIZE 1024

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
    char buffer[MAX_MSG_SIZE];
    if (argc < 2) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }
	
	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        exit(-1);
	}

    printf("Enter the message: ");
    bzero(buffer, MAX_MSG_SIZE);
    fgets(buffer, MAX_MSG_SIZE, stdin);
    buffer[strlen(buffer) - 1] = 0;
	/* write in the socket */
	n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
		printf("ERROR writing to socket\n");

    struct file_info file;
    uint fileInfoSize = sizeof(struct file_info);
	/* read from the socket */
    while(TRUE) {
        bzero((void*)&file, fileInfoSize);
        n = read(sockfd, (void*)&file, fileInfoSize);
        if (n <= 0) break;
        int allZero = TRUE;
        byte* data = (byte*)&file;
        int i;
        for (i = 0; i < fileInfoSize; i++) {
            if (data[i] != 0) {
                allZero = FALSE;
                break;
            }
        }
        if (allZero == TRUE) break;
        printf("%d %s %s %s\n", file.size, file.name, file.extension, file.last_modified);
    }
    if (n < 0) 
		printf("ERROR reading from socket\n");

    
    
	close(sockfd);
    return 0;
}