/*
 * udpchatclient_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Oct 18, 2018
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
#define IP_LEN		16
#define DEBUG

#define MAX(a,b) (((a) > (b))? (a):(b))

int getAddrInfo(char *host, char *port, struct addrinfo **serv_info);
void startChat(int sock_fd, char *server_ip, struct addrinfo *rp);
int udpSocket(char *host, char *serverport);

int main(int argc, char *argv[])
{
	char *servername = "localhost";
	char *serverport = "9191";
	int ret = 0;

	printf("CMPE 207 HW4 udpchatclient Pournami Puthenpurayil Rajan 669\n");
	switch (argc)
	{
	case 1:
			break;
	case 2:
			servername = argv[1];
			break;
	case 3:
			servername = argv[1];
			serverport = argv[2];
			break;
	default:
			printf("Error: Usage is ./<executable> <hostname> [port]\n");
			return -1;
	}
	ret = udpSocket(servername, serverport);
	if (ret < 0)
	{
		printf("Error (processClient): Client processing failed\n");
		return -1;
	}
	return 0;
}

/**
 * udpSocket - to create a udp client socket
 * host: server host address
 * serverport: port number of server
 * returns 0(success) -1(error)
 */
int udpSocket(char *host, char *serverport)
{
	int addr_info, sock_fd;
	struct addrinfo *serv_info, *rp;

	/* Get server info */
	addr_info = getAddrInfo(host, serverport, &serv_info);
	if (addr_info != 0)
	{
		printf("Error (getaddrinfo): %s\n", gai_strerror(addr_info));
		return -1;
	}
	/* create a client socket from the list of addrinfo*/
	for (rp = serv_info; rp != NULL; rp = rp->ai_next)
	{
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock_fd < 0)
			continue;
		break;
	}
	if (rp == NULL)
	{
		printf("No address in the list was a success\n");
		return -1;
	}
	DEBUG("Socket created\n");
	freeaddrinfo(serv_info);
	startChat(sock_fd, host, rp);
	return 0;
}

void startChat(int sock_fd, char *server_ip, struct addrinfo *rp)
{
	char *recv_buffer = malloc(BUFFER_SIZE * sizeof(char));
	char *send_buffer = malloc(BUFFER_SIZE * sizeof(char));
	int nfds = MAX(sock_fd, STDIN_FILENO) + 1;
	char *busy_msg = "<<server busy>>\n";
	struct sockaddr_in serv_addr;
	int serv_len = sizeof(serv_addr);
	fd_set rfds, afds;
	long unsigned int line_len;
	int sock_read, sock_write;
	const char *res;
	time_t now;

	FD_ZERO(&afds);
	FD_SET(sock_fd, &afds);
	FD_SET(STDIN_FILENO, &afds);
	while (1)
	{
		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(nfds, &rfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0) < 0)
		{
			printf("Error (select): %s\n", strerror(errno));
			return;
		}
		if (FD_ISSET(sock_fd, &rfds))
		{
			memset(recv_buffer, 0, BUFFER_SIZE);
			sock_read = recvfrom(sock_fd, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &serv_len);
			if (sock_read < 0)
			{
				printf("Error (recvfrom): %s\n", strerror(errno));
				return;
			}
			now = time(NULL);
			printf("%s, %s [%d] : %s", strtok(ctime(&now), "\n"), server_ip, ntohs(serv_addr.sin_port), recv_buffer);
			if (!strcmp(recv_buffer, busy_msg))
				break;
		}
		if (FD_ISSET(STDIN_FILENO, &rfds))
		{
			memset(send_buffer, 0, BUFFER_SIZE);
			getline(&send_buffer, &line_len, stdin);
			sock_write = sendto(sock_fd, send_buffer, BUFFER_SIZE, 0, rp->ai_addr, rp->ai_addrlen);
			if (sock_write < 0)
			{
				printf("Error (sendto): %s \n", strerror(errno));
				return;
			}
		}
	}
	free(send_buffer);
	free(recv_buffer);
	return;
}

/**
 * getAddrInfo - get end point information
 * host: host address of server
 * port: port number of server
 * serv_info: to store the server information
 * returns addr_info
 */
int getAddrInfo(char *host, char *port, struct addrinfo **serv_info)
{
	struct addrinfo hints;
	int addr_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= AF_INET;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= 0;
	hints.ai_flags		= 0;
	addr_info		= getaddrinfo(host, port, &hints, serv_info);
	return addr_info;
}

