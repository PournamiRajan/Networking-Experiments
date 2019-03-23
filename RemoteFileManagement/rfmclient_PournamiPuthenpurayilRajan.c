/*
 * rfmclient_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Oct 1, 2018
 *      Author: pournami
 */

#include "rfm_PournamiPuthenpurayilRajan.h"
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

#define INPUT_SIZE		64
#define DEBUG

int processRequest(int sock_fd, struct request *req, struct response *resp);
int getAddrInfo(char *host, char *port, struct addrinfo **serv_info);
int processClient(char *servername, char *serverport);
int tcpSocket(char *host, char *serverport);
void printResponse(struct response *resp);
int getUserRequest(int sock_fd);

int main(int argc, char *argv[])
{
	char *servername = "localhost";
	char *serverport = "9091";
	int ret = 0;

	printf("CMPE 207 HW3 rfmclient Pournami Puthenpurayil Rajan 669\n");
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
			printf("Error: Usage is ./[executable] [hostname] [port]\n");
			return -1;
	}
	ret = processClient(servername, serverport);
	if (ret < 0)
	{
		printf("Error (processClient): Client processing failed\n");
		return -1;
	}
	return 0;
}

/**
 * processClient - process the client for remote file system management
 * servername: the server address
 * serverport: port number of server
 * returns 0(success) -1(error)
 */
int processClient(char *servername, char *serverport)
{
	int sock_fd, sock_close;
	int ret	= 0;

	sock_fd = tcpSocket(servername, serverport);
	if (sock_fd < 0)
	{
		printf("Error (tcpSocket): socket creation not successful\n");
		return -1;
	}
	ret = getUserRequest(sock_fd);
	if (ret < 0)
	{
		printf("Error (getUserRequest): user request not successful\n");
		return -1;
	}
	sock_close = close(sock_fd);
	if (sock_close < 0)
	{
		printf("Error (close): %s\n", strerror(errno));
		return -1;
	}
	DEBUG("Socket closed \n");
	return 0;
}

/**
 * tcpSocket - to create a tcp client socket
 * host: server host address
 * serverport: port number of server
 * returns 0(success) -1(error)
 */
int tcpSocket(char *host, char *serverport)
{
	int addr_info, sock_fd, sock_conn;
	struct *serv_info, *rp;

	/* Get server info */
	addr_info = getAddrInfo(host, serverport, &serv_info);
	if (addr_info != 0)
	{
		printf("Error (getaddrinfo): %s\n", gai_strerror(addr_info));
		return -1;
	}
	/* create a client socket and connect the socket from the list of addrinfo*/
	for (rp = serv_info; rp != NULL; rp = rp->ai_next)
	{
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock_fd < 0)
			continue;
		DEBUG("Socket created\n");
		sock_conn = connect(sock_fd, rp->ai_addr, rp->ai_addrlen);
		if (sock_conn != -1)
			break;
		close(sock_fd);
	}
	if (rp == NULL)
	{
		printf("No address in the list was a success\n");
		return -1;
	}
	freeaddrinfo(serv_info);
	DEBUG("Socket connected \n");
	return sock_fd;
}

/**
 * getUserRequest - collect user request and process it
 * sock_fd: socket file descriptor
 * returns 0(success) -1(error)
 */
int getUserRequest(int sock_fd)
{
	struct response *resp = malloc(sizeof(struct response));
	struct request *req = malloc(sizeof(struct request));
	char *user_input = malloc(sizeof(char) * INPUT_SIZE);
	long unsigned int req_size;
	char *ptr;	
	int ret;

	while (1)
	{
		memset(req, 0, sizeof(struct request));
		DEBUG("Enter command: \n");
		getline(&user_input, &req_size, stdin);
		ptr = strtok(user_input, " \n");
		strcpy(req->command, ptr);
		ptr = strtok(NULL, " \n");
		if (ptr)
			strcpy(req->path, ptr);
		ret = processRequest(sock_fd, req, resp);
		if (ret < 0)
		{
			printf("Error (processRequest): Request processing failed\n");
			return -1;
		}
		if (!strcmp(req->command, "exit"))
			break;
	}
	free(user_input);
	free(req);
	free(resp);
	return 0;
}

/**
 * processRequest - send the request and receive the response
 * sock_fd: socket file descriptor
 * req: request structure to store request
 * resp: response structure to store response
 * returns 0(success) -1(error)
 */
int processRequest(int sock_fd, struct request *req, struct response *resp)
{
	int sock_send, sock_read, sock_shut;
	int req_len = sizeof(struct request);
	int resp_len = sizeof(struct response);
	char *buffer = (char *)resp;

	/* Send the request to server */
	sock_send = send(sock_fd, req, req_len, 0);
	if (sock_send < 0)
	{
		printf("Error (send):%s\n", strerror(errno));
		return -1;
	}
	DEBUG("Request:\n");
	/* if command is exit, shutdown the socket for write */
	if (!strcmp(req->command, "exit"))
	{
		sock_shut = shutdown(sock_fd, SHUT_WR);
		if (sock_shut < 0)
		{
			printf("Error (shutdown):%s\n", strerror(errno));
			return -1;
		}
	}
	memset(resp, 0, sizeof(struct response));
	/* Read the response from server till EoF*/
	while (1)
	{
		sock_read = read(sock_fd, buffer, resp_len);
		if (sock_read < 0)
		{
			printf("Error (read):%s\n", strerror(errno));
			return -1;
		}
		buffer += sock_read;
		resp_len -= sock_read;
		/* break when the response structure is finished reading */
		if (!sock_read || resp_len <= 0)
			break;
	}
	DEBUG("Response:\n");
	resp->status_code = ntohs(resp->status_code);
	printResponse(resp);
	return 0;
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
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= 0;
	hints.ai_flags		= 0;
	addr_info		= getaddrinfo(host, port, &hints, serv_info);
	return addr_info;
}

/**
 * printResponse - to print the response of the command
 * resp: response structure which contains the command output
 */
void printResponse(struct response *resp)
{
	DEBUG("Status: %s\n", resp->status);
	DEBUG("Status code: %d\n", resp->status_code);
	printf("%s\n", resp->output);
	return;
}

