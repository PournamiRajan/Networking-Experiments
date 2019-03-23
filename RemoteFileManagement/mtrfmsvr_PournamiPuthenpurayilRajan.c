/*
 * mtrfmsvr_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Oct 3, 2018
 *      Author: pournami
 */

#include "rfm_PournamiPuthenpurayilRajan.h"
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
#include <stdio.h>
#include <pthread.h>

#define QUEUE_LEN		10
#define IP_LEN			16
#define PORT_LEN		10
#define ERR_LEN			256
#define DEBUG

void processRequest(struct request *req, struct response *resp);
void storeStatus(struct response *resp);
void handleRFSClient(int sock_fd);
int startServer(char *port);

/**
 * To store the global status of the connections
 * status_mutex: mutex to protect the structure
 * connect_count: concurrent client connection count
 * complete_count: completed client connection count
 * cmd_count: total command count
 */
struct stats_t
{
	pthread_mutex_t status_mutex;
	int connect_count;
	int complete_count;
	int cmd_count;
};
struct stats_t  stats = {PTHREAD_MUTEX_INITIALIZER, 0, 0, 0};

int main(int argc, char *argv[])
{
	char *port = "9091";
	int ret;

	printf("CMPE 207 HW3 rfmserver Pournami Puthenpurayil Rajan 669\n");
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
	int master_fd, sock_bind, sock_listen, slave_fd;
	int create_thrd;
	struct sockaddr_in serv_addr, client_addr;
	int client_len = sizeof(client_addr);
	pthread_t thread;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	/* Create Socket */
	master_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (master_fd < 0)
	{
		printf("Error (socket): %s\n", strerror(errno));
		return -1;
	}
	DEBUG("socket()\n");
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_port			= htons(atoi(port));
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	/* Bind Socket */
	sock_bind = bind(master_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (sock_bind < 0)
	{
		printf("Error (bind): %s\n", strerror(errno));
		return -1;
	}
	DEBUG("bind()\n");
	/* Listen on the socket */
	sock_listen = listen(master_fd, QUEUE_LEN);
	if (sock_listen < 0)
	{
		printf("Error (listen): %s\n", strerror(errno));
		return -1;
	}
	DEBUG("listen()\n");
	memset(&client_addr, 0, sizeof(client_addr));
	while(1)
	{
		/* Accept client requests */
		slave_fd = accept(master_fd, (struct sockaddr *)&client_addr, &client_len);
		if (slave_fd < 0)
		{
			if (errno == EINTR)
				continue;
			printf("Error (accept): %s\n", strerror(errno));
			return -1;
		}
		DEBUG("accept()\n");
		/* Create thread for each clients */
		create_thrd = pthread_create(&thread, &attr, (void * (*) (void *)) handleRFSClient, (void *)((long) slave_fd));
		if (create_thrd < 0)
		{
			printf("Error (pthread_create): %s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}

/**
 * handleRFSClient - handles each client
 * sock_fd: file descriptor of the slave socket for the client
 */
void handleRFSClient(int sock_fd)
{
	int sock_read, sock_write, sock_close;
	struct request *req = malloc(sizeof(struct request));
	struct response *resp = malloc(sizeof(struct response));
	int req_len;
	int resp_len;
	char *buffer = (char *)req;
	int threadId = pthread_self();
	char *error = malloc(sizeof(char) * ERR_LEN);

	printf("Thread %u: started\n", threadId);
	/* Increase the concurrent connection count */
	pthread_mutex_lock(&stats.status_mutex);
	stats.connect_count++;
	pthread_mutex_unlock(&stats.status_mutex);
	/*Read the client request */
	while (1)
	{
		memset(req, 0, sizeof(struct request));
		memset(resp, 0, sizeof(struct response));
		buffer = (char *)req;
		req_len = sizeof(struct request);
		resp_len = sizeof(struct response);
		while (1)
		{
			sock_read = read(sock_fd, buffer, req_len);
			if (sock_read < 0)
			{
				strerror_r(errno, error, ERR_LEN);
				printf("Error (read): %s\n", error);
				printf("Thread %u: exited due to error\n", threadId);
				return;
			}
			buffer += sock_read;
			req_len	-= sock_read;
			/* Break the loop when the request structure is read completely */
			if (!sock_read || req_len <= 0)
				break;
		}
		DEBUG("Request received\n");
		printf("Thread %u: %s %s\n", threadId, req->command, req->path);
		/* Increase the command count */
		pthread_mutex_lock(&stats.status_mutex);
		stats.cmd_count++;
		pthread_mutex_unlock(&stats.status_mutex);
		processRequest(req, resp);
		sock_write = send(sock_fd, resp, resp_len, 0);
		if (sock_write < 0)
		{
			strerror_r(errno, error, ERR_LEN);
			printf("Error (send): %s\n", error);
			printf("Thread %u: exited due to error\n", threadId);
			return;
		}
		DEBUG("Response sent\n");
		if (!strcmp(req->command, "exit"))
			break;
	}
	sock_close = close(sock_fd);
	if (sock_close < 0)
	{
		strerror_r(errno, error, ERR_LEN);
		printf("Error (close): %s\n", error);
		printf("Thread %u: exited due to error\n", threadId);
		return;
	}
	/* Increase the complete connection count  && Decrease the connection count*/
	pthread_mutex_lock(&stats.status_mutex);
	stats.connect_count--;
	stats.complete_count++;
	pthread_mutex_unlock(&stats.status_mutex);
	printf("Thread %u: exited\n", threadId);
	free(req);
	free(resp);
	free(error);
	return;
}

/**
 * processRequest - to process each request and populate response structure
 * req: request structure which contains the commands to execute
 * resp: response structure to store the result of the commands after execution
 */
void processRequest(struct request *req, struct response *resp)
{
	FILE *file;
	char *command = malloc(sizeof(char) * 100);
	char *redirect = " 2>&1";
	char *error = malloc(sizeof(char) * ERR_LEN);

	strcpy(command, req->command);
	if (strcmp(command, "cat") && strcmp(command, "rm") && strcmp(command, "stats") && strcmp(command, "exit"))
	{
		resp->status_code = htons(400);
		strcpy(resp->status, "BAD REQUEST");
		strcpy(resp->output, "Invalid Command\n");
		printf("Invalid Command: %s\n", command);
		return;
	}
	if (strlen(req->path))
	{
		strcat(command, " ");
		strcat(command, req->path);
	}
	if (!strcmp(command, "stats"))
	{
		storeStatus(resp);
		return;
	}
	strcat(command, redirect);
	file = popen(command, "r");
	if (!file)
	{
		strerror_r(errno, error, ERR_LEN);
		resp->status_code = htons(500);
		strcpy(resp->status, "SERVER INTERNAL ERROR");
		strcpy(resp->output, error);
		printf("Error (popen): %s\n", error);
		return;
	}
	fread(resp->output, OP_SIZE, 1, file);
	resp->status_code = htons(200);
	strcpy(resp->status, "SUCCESS");
	pclose(file);
	free(command);
	free(error);
	return;
}

/**
 * storeStatus - to populate the response structure if command is 'stats'
 * resp: response structure to store results of command
 */
void storeStatus(struct response *resp)
{
	char con_count[10], comp_count[10], cmd_count[10];

	pthread_mutex_lock(&stats.status_mutex);
	sprintf(con_count, "%d", stats.connect_count);
	sprintf(comp_count, "%d", stats.complete_count);
	sprintf(cmd_count, "%d", stats.cmd_count);
	pthread_mutex_unlock(&stats.status_mutex);

	resp->status_code = htons(200);
	strcpy(resp->status, "SUCCESS");
	strcpy(resp->output, "Concurrent Connections : ");
	strcat(resp->output, con_count);
	strcat(resp->output, "\n");
	strcat(resp->output,"Completed Connections : ");
	strcat(resp->output, comp_count);
	strcat(resp->output, "\n");
	strcat(resp->output, "Commands received : ");
	strcat(resp->output, cmd_count);
	strcat(resp->output, "\n");
	return;
}
