/*
 * udpchatsvrmu.c
 *
 *  Created on: Oct 18, 2018
 *      Author: pournami
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE		50
#define IP_LEN			16
#define MAX_CLIENT		10
#define DEBUG

#define MAX(a, b) (((a) > (b))? (a): (b))

int startServer(char *port);
int processUdpChatClient(int sock_fd);

int main(int argc, char *argv[])
{
	char *port = "9191";
	int ret;

	printf("CMPE 207 HW4 udpchatsvr Pournami Puthenpurayil Rajan 669\n");
	switch (argc)
	{
	case 1:
			break;
	case 2:
			port = argv[1];
			break;
	default:
			printf("Error: Usage is ./[executable] [port]\n");
			return -1;
	}
	ret = startServer(port);
	if (ret < 0)
	{
		printf("Error (startServer): server failed\n");
		return -1;
	}
	return 0;
}

/**
 * startServer - to start the server and process client requests
 * port: Portnumber in which the server is running
 * return 0 (success) -1 (error)
 */
int startServer(char *port)
{
	int sock_fd, sock_bind, sock_close, ret;
	struct sockaddr_in serv_addr;

	/* Create Socket */
	sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0)
	{
		printf("Error (socket): %s\n", strerror(errno));
		return -1;
	}
	DEBUG("socket()\n");
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_port			= htons(atoi(port));
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	/* Bind Socket */
	sock_bind = bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (sock_bind < 0)
	{
		printf("Error (bind): %s\n", strerror(errno));
		return -1;
	}
	DEBUG("bind()\n");
	ret = processUdpChatClient(sock_fd);
	if (ret < 0)
	{
		printf("Error (processUdpChatClient): client processing failed\n");
		return -1;
	}
	return 0;
}

/*
 * processUdpChatClient() - process the chat client
 * sock_fd: socket file descriptor
 * return 0 (success) -1 (error)
 */
int processUdpChatClient(int sock_fd)
{
	char *recv_buffer	= malloc(BUFFER_SIZE * sizeof(char));
	char *send_buffer	= malloc(BUFFER_SIZE * sizeof(char));
	unsigned int client_len = sizeof(struct sockaddr);
	struct sockaddr_in in_addr[MAX_CLIENT];
	int client_conn = 0;
	struct sockaddr_in client_addr;
	int nfds = MAX(sock_fd, STDIN_FILENO) + 1;
	long unsigned int line_len;
	int sock_read, sock_write;
	char client_ip[IP_LEN];
	fd_set rfds, afds;
	const char *res;
	bool found = false;
	time_t now;

	FD_ZERO(&afds);
	FD_SET(sock_fd, &afds);
	FD_SET(STDIN_FILENO, &afds);
	memset(&client_addr, 0, sizeof(client_addr));
	memset(&in_addr, 0, sizeof(in_addr));
	while (1)
	{
		memcpy(&rfds, &afds, sizeof(rfds));
		memset(send_buffer, 0, BUFFER_SIZE);
		if (select(nfds, &rfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *)0) < 0)
		{
			printf("Error (select): %s\n", strerror(errno));
			return -1;
		}
		if (FD_ISSET(sock_fd, &rfds))
		{
			found = false;
			memset(recv_buffer, 0, BUFFER_SIZE);
			sock_read = recvfrom(sock_fd, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
			if (sock_read < 0)
			{
				printf("Error (recvfrom): %s\n", strerror(errno));
				return -1;
			}
			/* save the client address */
			for (int i = 0; i < client_conn; i++)
			{
				if(!memcmp(&in_addr[i], &client_addr, sizeof(in_addr[i])))
				{
					found = true;
					break;
				}
			}
			if (!found && client_conn < MAX_CLIENT)
			{
				memcpy(&in_addr[client_conn], &client_addr, sizeof(client_addr));
				client_conn++;
			}
			/* convert IP address from binary to text */
			res = inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, IP_LEN);
			if (!res)
			{
				printf("Error (inet_ntop): %s\n", strerror(errno));
				return -1;
			}
			now = time(NULL);
			printf("%s, %s [%d]: %s", strtok(ctime(&now), "\n"), client_ip, ntohs(client_addr.sin_port), recv_buffer);
			/* broadcast it to all other clients */
			for (int i = 0; i < client_conn; i++)
			{
				if(!memcmp(&in_addr[i], &client_addr, sizeof(in_addr[i])))
					continue;
				strcpy(send_buffer, recv_buffer);
				sock_write = sendto(sock_fd, send_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&in_addr[i], sizeof(struct sockaddr));
				if (sock_write < 0)
				{
					printf("Error (sendto): %s\n", strerror(errno));
					return -1;;
				}
			}
		}
		if (FD_ISSET(STDIN_FILENO, &rfds))
		{
			getline(&send_buffer, &line_len, stdin);
			for (int i = 0; i < client_conn; i++)
			{
					sock_write = sendto(sock_fd, send_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&in_addr[i], sizeof(struct sockaddr));
					if (sock_write < 0)
					{
						printf("Error (sendto): %s\n", strerror(errno));
						return -1;;
					}
			}
		}
	}
	free(send_buffer);
	free(recv_buffer);
	return 0;
}
