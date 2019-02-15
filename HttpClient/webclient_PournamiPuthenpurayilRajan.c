/*
 * webclient_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Sep 9, 2018
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

#define PORT		"http"
#define PATH_SIZE	30
#define HOST_SIZE	30
#define BUFFER_SIZE 13312 /* 1024 *13 */
#define DEBUG

int main(int argc, char *argv[])
{
	int addr_info, sock_fd, sock_conv, sock_conn, sock_read, sock_write, sock_shut, sock_close;
	char path[PATH_SIZE], host[HOST_SIZE];
	struct addrinfo hints, *serv_info, *rp;
	struct  sockaddr_in sock_addr;
	
	if (argc > 2 || argc < 2)
	{
		printf("Number of arguments should be 2\n");
		return -1;
	}
	int success				= 0;
	char *send_msg1			= "GET ";
	char *send_msg2			= argv[1];
	char *send_msg3			= " HTTP/1.0\r\n";
	char *send_msg4			= "\r\n";
	int send_msg_size		= strlen(send_msg1)+strlen(send_msg2)+strlen(send_msg3)+strlen(send_msg4)+1;
	char *buffer			= malloc(sizeof(char) * BUFFER_SIZE);
	char *recv_msg			= buffer;
	int buffer_len			= BUFFER_SIZE;
	char *print_msg			= "CMPE 207 HW1 webclient Pournami Puthenpurayil Rajan 669";
	char send_msg[send_msg_size];

	printf("%s\n",print_msg);
	strcpy(send_msg, send_msg1);
	strcat(send_msg, send_msg2);
	strcat(send_msg, send_msg3);
	strcat(send_msg, send_msg4);
	DEBUG("Final msg to send: %s\n",send_msg);
	/* reference : code at the end of the hw question pdf */
	if (sscanf(argv[1], "http://%99[^/]/%199[^\n]", host, path) == 2)
		success = 1;/* http://hostname/page	*/
	else if (sscanf(argv[1], "http://%99[^/]/[^\n]", host) == 1)
		success = 1;  /* http://hostname/ */
	else if (sscanf(argv[1], "http://%99[^\n]", host) == 1)
		success = 1;  /* http://hostname */
	if (success == 0)
	{
		printf("Error: url extraction\n");
		return -1;
	}
	/* Get server info */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= AF_INET;
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= 0;
	hints.ai_flags		= 0;
	addr_info			= getaddrinfo(host, PORT, &hints, &serv_info);
	DEBUG("Addrinfo = %d\n",addr_info);
	if (addr_info != 0)
	{
		printf("Error: %s \n", gai_strerror(addr_info));
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
	/* Send the http request to server */
	sock_write = write(sock_fd, send_msg, strlen(send_msg));
	if (sock_write < 0)
	{
		printf("Error: %s\n", strerror(errno));
		return -1;
	}
	DEBUG("Socket write complete\n");
	/* Shut down the socket for writes*/
	sock_shut = shutdown(sock_fd, SHUT_WR);
	if (sock_shut < 0)
	{
		printf("Error: %s \n", strerror(errno));
		return -1;
	}
	DEBUG("Socket shutdown \n");
	/* Since response is bitstream, read the response until eof */
	while(1)
	{
		/* Read response and store it in buffer */
		sock_read = read(sock_fd, buffer, buffer_len);
		if (sock_read < 0)
		{
			printf("Error: %s\n", strerror(errno));
			return -1;
		}
		buffer		+= sock_read;
		buffer_len	-= sock_read;
		/* if eof is reached, break out of loop */
		if (sock_read == 0)
			break;
	}
	buffer++;
	*buffer = '\0';
	DEBUG("Socket read complete\n");
	/* Print the http response */
	printf("%s", recv_msg);
	/* Close the socket */
	sock_close = close(sock_fd);
	if (sock_close < 0)
	{
		printf("Error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}
