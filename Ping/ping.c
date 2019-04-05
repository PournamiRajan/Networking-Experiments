/*
 * ping_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Nov 9, 2018
 *      Author: pournami
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

void sendPing4();
unsigned short in_cksum(unsigned short *addr, int len);
void recvICMP();
void timeDiff(struct timeval *time, struct timeval *res);
int seq;

#define IP_LEN	16
#define BUFFER_SIZE	100
#define RCV_TIME_OUT	2
#define DATA_LEN 20
#define DEBUG

struct sockaddr_in serv_addr;
int sock_fd;

int main(int argc, char *argv[])
{
	char *servername = "localhost";
	int port = 0;
	int ret = 0, i = 0;
	struct timeval timeout;
	char *ip = malloc(sizeof(char) * IP_LEN);

	printf("CMPE 207 HW5 ping Pournami Puthenpurayil Rajan 669\n");
	switch (argc)
	{
	case 1:
			break;
	case 2:
			servername = argv[1];
			break;
	default:
			printf("Error: Usage is ./[executable] [hostname] \n");
			return -1;
	}
	sock_fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_fd < 0)
	{
		printf("Error (socket): %s errno = %d\n", strerror(errno), errno);
		return -1;;
	}
	timeout.tv_sec = RCV_TIME_OUT;
	timeout.tv_usec = 0;
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	struct hostent *addr = gethostbyname(servername);
	if (!addr)
	{
		printf("Error (gethostbyname):  %s (If success, could not find the address)\n", strerror(errno));
		return -1;
	}
    while (addr -> h_addr_list[i])
    {
    	ip =  inet_ntoa( *( struct in_addr*)(addr->h_addr_list[i]));
    	if (strlen(ip))
    		break;
    	i++;
    }
	inet_pton(AF_INET, ip, &serv_addr.sin_addr.s_addr);
	printf("PING %s (%s) %d bytes of data\n", addr->h_name, ip, DATA_LEN + 8);
	signal(SIGALRM, sendPing4);
	alarm(1);
	recvICMP();
	return 0;
}

/**
 * sendPing4() - send IPv4 ping packets
 */
void sendPing4()
{
	int len;
	int sock_send;
	struct icmp     *icmp;
	char *send_buff = malloc(sizeof(char) * BUFFER_SIZE);
	icmp = (struct icmp *) send_buff;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = getpid();
	icmp->icmp_seq = ++seq;
	memset(icmp->icmp_data, 0xa5, DATA_LEN);
	gettimeofday((struct timeval *) icmp->icmp_data, NULL);

	len = 8 + DATA_LEN;
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((u_short *) icmp, len);

	sock_send = sendto(sock_fd, send_buff, len, 0, (struct sockaddr *)&serv_addr,  sizeof(struct sockaddr));
	if (sock_send < 0)
	{
		printf("Error (sendto): %s errno = %d\n", strerror(errno), errno);
		return;
	}
	alarm(1);
	free(send_buff);
}

/**
 * recvICMP() - to receive icmp packets
 */
void recvICMP()
{
	char *recv_buffer = malloc(sizeof(char) * BUFFER_SIZE);
	unsigned int inaddr_len = sizeof(struct sockaddr);
	struct sockaddr_in in_addr;
	int sock_recv;
	char serv_ip[IP_LEN];
	const char *res;
	struct timeval time;
	struct icmp icmp;
	struct iphdr ip;

	while (1)
	{
		memset(recv_buffer, 0, BUFFER_SIZE);
		memset(&in_addr, 0, sizeof(in_addr));
		memset(&time, 0, sizeof(struct timeval));
		sock_recv = recvfrom(sock_fd, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&in_addr, &inaddr_len);
		if (sock_recv < 0)
		{
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN)
			{
				printf("Request timed out, Trying again\n");
				continue;
			}
			printf("Error (recvfrom): %s errno = %d\n", strerror(errno), errno);
			return;
		}
		res = inet_ntop(AF_INET, &in_addr.sin_addr, serv_ip, IP_LEN);
		if (!res)
		{
			printf("Error (inet_ntop): %s\n", strerror(errno));
			exit(-1);
		}
		memcpy(&ip, recv_buffer, sizeof(struct iphdr));
		memcpy(&icmp, recv_buffer + sizeof(struct iphdr), sock_recv - sizeof(struct iphdr));
		memcpy(&time, &icmp.icmp_data, sizeof(struct timeval));
		if (icmp.icmp_type == ICMP_ECHOREPLY)
		{
			if (icmp.icmp_id != getpid())
				continue;
			struct timeval *res = malloc(sizeof(struct timeval));
			timeDiff(&time, res);
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%0.3fms\n", sock_recv, serv_ip, icmp.icmp_seq, ip.ttl, (double)res->tv_usec/1000);
		}
	}

}

/**
 * timeDiff() - to calculate round trip time
 * time: time in icmp packet
 * res: structure to store result
 */
void timeDiff(struct timeval *time, struct timeval *res)
{
	struct timeval *now = malloc(sizeof(struct timeval));
	gettimeofday(now, NULL);
	timersub(now, time, res);

}

/**
 * in_cksum() - to calculate icmp checksum
 * addr: starting point of icmp header
 * len: length of icmp packet
 * return checksum
 */
/* reference https://github.com/iputils/iputils/blob/master/ping.c*/
unsigned short in_cksum(unsigned short *addr, int len)
{
	int	nleft = len;
	int	sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
		sum += answer;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return(answer);
}

