/*
 * client.c
 *
 *  Created on: Aug 29, 2018
 *      Author: pournami
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


#define PORT 8088

int main()
{
	int sock_id, sock_conn, sock_read, sock_write;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_id < 0) {
    	perror("socket creation failed");
     	return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}
    sock_conn = connect(sock_id, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (sock_conn < 0) {
         perror("socket connect failed");
         return -1;
    }
    while (1)
    {
    	char *hello = "Hello from client";
    	sock_write = send(sock_id, hello, strlen(hello), 0);
    	if (sock_write < 0) {
    			perror("socket write failed");
    			return -1;
    	}
    	printf("Client sent %s\n", hello);
    	sock_read = read(sock_id, buffer, sizeof(buffer));
    	if (sock_read < 0) {
    			perror("socket read failed");
    			return -1;
    	}
    	printf("Server sent %s\n", buffer);

     }
    close(sock_id);
     return 0;
}
