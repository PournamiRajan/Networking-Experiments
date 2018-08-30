/*
 * server.c
 *
 *  Created on: Aug 29, 2018
 *      Author: pournami
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define PORT 8088

int main()
{
	int sock_id, sock_opt, sock_bind, sock_listen, sock_conn, sock_read, sock_write;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = malloc(40);

    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_id < 0) {
    	perror("socket creation failed");
    	return -1;
    }
    sock_opt = setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                      &opt, sizeof(opt));
    if (sock_opt < 0) {
    	perror("socket option failed");
    	return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    sock_bind = bind(sock_id, (struct sockaddr *)&address, sizeof(address));
    if (sock_bind < 0) {
        perror("socket bind failed");
        return -1;
    }
    sock_listen = listen(sock_id, 3);
    if (sock_listen < 0) {
        perror("socket listen failed");
        return -1;
    }
    sock_conn = accept(sock_id, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (sock_conn < 0) {
        perror("socket accept failed");
        return -1;
    }

    while (1)
    {
    	sock_read = read(sock_conn, buffer, sizeof(buffer));
        if (sock_read < 0) {
        	perror("socket read failed");
        	return -1;
        }
        printf("server read %s\n", buffer);
        for (int i = 0; i < 1024; i++)
        {
        	if (islower(buffer[i]))
        		buffer[i] = toupper(buffer[i]);
        }
        sock_write = send(sock_conn, buffer, sizeof(buffer), 0);
		if (sock_write < 0) {
			perror("socket write failed");
			return -1;
		}
		printf("server sent %s\n", buffer);
    }
    close(sock_id);
    return 0;
}
