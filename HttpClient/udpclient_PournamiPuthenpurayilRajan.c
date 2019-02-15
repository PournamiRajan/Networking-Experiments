/*
 * udpclient_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Sep 12, 2018
 *      Author: pournami
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <time.h>

#define BUFFER_SIZE		50
#define SEND_MSG_SIZE	50
#define DEBUG

int main(int argc, char *argv[])
{
	int addr_info, sock_fd, sock_read, sock_send, sock_close;
	struct addrinfo hints, *serv_info, *rp;
	struct  sockaddr_in sock_addr;
	char send_msg[SEND_MSG_SIZE];
	time_t t;
	socklen_t *addrlen 	= malloc(sizeof(struct sockaddr));
	char *buffer 		= malloc(sizeof(char) * BUFFER_SIZE);
	char *print_msg		= "CMPE 207 HW1 udp Pournami Puthenpurayil Rajan 669";

	printf("%s\n",print_msg);
	if (argc < 4 || argc > 4)
	{
		printf("Number of arguments should be 4\n");
		return -1;
	}
	if (strlen(argv[3]) > SEND_MSG_SIZE)
	{
		printf("Message size too big\n");
		return -1;
	}
	strcpy(send_msg, argv[3]);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= AF_INET;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= 0;
	hints.ai_flags		= 0;
	addr_info			= getaddrinfo(argv[1], argv[2], &hints, &serv_info);
	if (addr_info != 0)
	{
		printf("Error: %s \n", gai_strerror(addr_info));
		return -1;
	}
	for (rp = serv_info; rp != NULL; rp = rp->ai_next)
	{
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock_fd != -1)
			break;
	}
	if (rp == NULL)
	{
        printf("No address in the list was a success\n");
        return -1;
    }
	DEBUG("Socket created\n");
    sock_send = sendto(sock_fd, send_msg, strlen(send_msg), 0, serv_info->ai_addr, sizeof(struct sockaddr));
	if (sock_send < 0)
	{
		printf("Error: %s \n", strerror(errno));
		return -1;
	}
	DEBUG("Socket send complete\n");
	sock_read = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, serv_info->ai_addr, addrlen);
	if (sock_read < 0)
	{
		printf("Error: %s \n", strerror(errno));
		return -1;
	}
	buffer[BUFFER_SIZE] = '\0';
	DEBUG("Socket read complete\n");
    freeaddrinfo(serv_info);
    time(&t);
    printf("Current local date time: %s", ctime(&t));
    printf("UDP datagram length = %d bytes\n", sock_read);
    /* Print the response message */
    printf("UDP datagram:\n");
    printf("%s\n",buffer);
    /* Close the socket */
    sock_close = close(sock_fd);
    if (sock_close < 0)
	{
		printf("Error: %s \n", strerror(errno));
		return -1;
	}
}

