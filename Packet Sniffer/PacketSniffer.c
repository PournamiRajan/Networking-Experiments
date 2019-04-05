/*
 * sniffer_PournamiPuthenpurayilRajan.c
 *
 *  Created on: Nov 11, 2018
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
#include <pcap.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>

#define ERROR_SIZE 100
#define ICMP_SIZE 100
#define DEBUG

void process_packet(unsigned char * args, const struct pcap_pkthdr *header, const unsigned char *buffer);
void process_eth_packet(const unsigned char *buffer);
void process_icmp_packet(const unsigned char *buffer);
void process_tcp_packet(const unsigned char *buffer);
void process_udp_packet(const unsigned char *buffer);
u_int8_t process_ip_packet(const unsigned char *buffer);

int option;
FILE *file;

int main(int argc, char *argv[])
{
	printf("CMPE 207 HW5 pktsniff Pournami Puthenpurayil Rajan 669\n");
	struct pcap_if *alldevices, *device, device_list[10];
	char err[ERROR_SIZE];
	struct pcap *p;
	int i = 1, device_num;

	printf("All available interfaces:\n");
	if (pcap_findalldevs(&alldevices , err))
	{
	        printf("Error (pcap_findalldevs): %s" , err);
	        return -1;
	}
	for(device = alldevices; device != NULL; device = device->next)
	{
		memcpy(&device_list[i], device, sizeof(device_list[0]));
		printf("%d. %s\n", i++, device->name);
	}
	printf("Pick any (1 to %d): ", --i);
	scanf("%d", &device_num);
	if (device_num < 1 || device_num > i)
	{
		printf("Invalid Option\n");
		return -1;
	}
	DEBUG("Device picked is %s\n", device_list[device_num].name);
	DEBUG("Opening device\n");
	p = pcap_open_live(device_list[device_num].name, 65536, 1, 0, err);
	if (!p)
	{
		printf("Error (pcap_open_live): %s\n", strerror(errno));
		return -1;
	}
	printf("Filter options:\n");
	printf("1. All\n2. ICMP\n3. TCP\n4. UDP\nEnter (1 to 4): ");
	scanf("%d", &option);
	if (option < 1 || option > 4)
	{
		printf("Invalid Option\n");
		return -1;
	}
	DEBUG("Option = %d", option);
	file = fopen("./out.txt", "w");
	pcap_loop(p , -1 , process_packet , NULL);
	fclose(file);
	return 0;
}

void process_packet(unsigned char * args, const struct pcap_pkthdr *header, const unsigned char *buffer)
{
	DEBUG("\n");
	struct ethhdr *eth_hdr = malloc(sizeof(struct ethhdr));
	struct iphdr *ip_pkt = malloc(sizeof(struct iphdr));
	int offset = sizeof(struct ethhdr);
	u_int8_t protocol;

	memset(ip_pkt, 0, sizeof(struct iphdr));
	memcpy(ip_pkt, buffer + offset, sizeof(struct iphdr));
	memcpy(eth_hdr, buffer, sizeof(struct ethhdr));

	protocol = ip_pkt->protocol;
	if (protocol != 1 && protocol != 6 && protocol != 17)
		return;
	if(option == 1 || (option == 2 && protocol == 1) || (option == 3 && protocol == 6) || (option == 4 && protocol == 17))
	{
		process_eth_packet(buffer);
		protocol = process_ip_packet(buffer);
	}

	switch (protocol)
	{
		case 1:
			DEBUG("ICMP packet\n");
			if (option == 1 || option == 2)
				process_icmp_packet(buffer);
			break;
		case 6:
			DEBUG("TCP packet\n");
			if (option == 1 || option == 3)
				process_tcp_packet(buffer);
			break;
		case 17:
			DEBUG("UDP packet\n");
			if (option == 1 || option == 4)
				process_udp_packet(buffer);
			break;
		default:
			DEBUG("Not an ICMP/TCP/UDP packet (skipping)\n");
			break;
	}
	return;
}

void process_eth_packet(const unsigned char *buffer)
{
	struct ethhdr *eth_hdr = malloc(sizeof(struct ethhdr));
	memcpy(eth_hdr, buffer, sizeof(struct ethhdr));
	fprintf(file, "-----------------------\n");
	fprintf(file, "Ethernet packet details\n");
	fprintf(file, "-----------------------\n");
	fprintf(file, "Dest Mac = ");
	fprintf(file, "%X-%X-%X-%X-%X-%X\n", eth_hdr->h_dest[0], eth_hdr->h_dest[1], eth_hdr->h_dest[2], eth_hdr->h_dest[3],
			eth_hdr->h_dest[4], eth_hdr->h_dest[5]);
	fprintf(file, "Src Mac = ");
	fprintf(file, "%X-%X-%X-%X-%X-%X\n", eth_hdr->h_source[0], eth_hdr->h_source[1], eth_hdr->h_source[2], eth_hdr->h_source[3],
			eth_hdr->h_source[4], eth_hdr->h_source[5]);
	fprintf(file, "Ethtype = 0x%04X\n", (uint16_t)ntohs(eth_hdr->h_proto));
}

u_int8_t process_ip_packet(const unsigned char *buffer)
{

	struct iphdr *ip_pkt = malloc(sizeof(struct iphdr));
	int offset = sizeof(struct ethhdr);
	struct sockaddr_in src, dest;

	memset(ip_pkt, 0, sizeof(struct iphdr));
	memcpy(ip_pkt, buffer + offset, sizeof(struct iphdr));
	memset(&src, 0, sizeof(src));
	memset(&dest, 0, sizeof(dest));
	src.sin_addr.s_addr = ip_pkt->saddr;
	dest.sin_addr.s_addr = ip_pkt->daddr;
	fprintf(file, "-----------------\n");
	fprintf(file, "IP Packet Details\n");
	fprintf(file, "-----------------\n");
	fprintf(file, "Version = %d\n", ip_pkt->version);
	fprintf(file, "Header length = %d\n", ip_pkt->ihl);
	fprintf(file, "Type of Service = %d\n", ip_pkt->tos);
	fprintf(file, "Total Length = %d\n", ntohs(ip_pkt->tot_len));
	fprintf(file, "Identification Number = %d\n", ntohs(ip_pkt->id));
	fprintf(file, "TTL = %d\n", ip_pkt->ttl);
	fprintf(file, "Protocol = %d\n", ip_pkt->protocol);
	fprintf(file, "Checksum = %d\n", ntohs(ip_pkt->check));
	fprintf(file, "Source IP = %s\n", inet_ntoa(src.sin_addr));
	fprintf(file, "Destination IP = %s\n", inet_ntoa(dest.sin_addr));
	return ip_pkt->protocol;
}

void process_icmp_packet(const unsigned char *buffer)
{
	struct icmp *icmp_pkt = malloc(sizeof(struct icmp));
	int offset = sizeof(struct ethhdr) +sizeof(struct iphdr);

	memset(icmp_pkt, 0, sizeof(struct icmp));
	memcpy(icmp_pkt, buffer + offset, sizeof(struct icmp));
	fprintf(file, "-------------------\n");
	fprintf(file, "ICMP Packet Details\n");
	fprintf(file, "-------------------\n");
	fprintf(file, "Type = %d\n", icmp_pkt->icmp_type);
	fprintf(file, "Code = %d\n", icmp_pkt->icmp_code);
	fprintf(file, "Checksum = %d\n", ntohs(icmp_pkt->icmp_cksum));
	fprintf(file, "Identifier = %d\n", icmp_pkt->icmp_id);
	fprintf(file, "Sequence Number = %d\n", icmp_pkt->icmp_seq);
	fprintf(file, "====================================================\n");
	return;
}

void process_tcp_packet(const unsigned char *buffer)
{
	struct tcphdr *tcp_pkt = malloc(sizeof(struct tcphdr));
	int offset = sizeof(struct ethhdr) +sizeof(struct iphdr);

	memset(tcp_pkt, 0, sizeof(struct tcphdr));
	memcpy(tcp_pkt, buffer, sizeof(struct tcphdr));
	fprintf(file, "------------------\n");
	fprintf(file, "TCP Packet Details\n");
	fprintf(file, "------------------\n");
	fprintf(file, "Source Port = %d\n", ntohs(tcp_pkt->source));
	fprintf(file, "Destination Port = %d\n", ntohs(tcp_pkt->dest));
	fprintf(file, "Sequence Number = %d\n", tcp_pkt->seq);
	fprintf(file, "Acknowledgment Number = %d\n", tcp_pkt->ack_seq);
	fprintf(file, "Header Length = %d\n", ntohs(tcp_pkt->doff));
    fprintf(file, "Urg Flag = %d\n", ntohs(tcp_pkt->urg));
    fprintf(file, "Ack Flag = %d\n", ntohs(tcp_pkt->ack));
    fprintf(file, "Push Flag = %d\n", ntohs(tcp_pkt->psh));
    fprintf(file, "Rst Flag = %d\n", ntohs(tcp_pkt->rst));
    fprintf(file, "Syn Flag = %d\n", ntohs(tcp_pkt->syn));
    fprintf(file, "Fin Flag = %d\n", ntohs(tcp_pkt->fin));
    fprintf(file, "Advertised Window = %d\n", ntohs(tcp_pkt->window));
    fprintf(file, "Checksum = %d\n", ntohs(tcp_pkt->check));
    fprintf(file, "Urg Pointer = %d\n", ntohs(tcp_pkt->urg_ptr));
    fprintf(file, "====================================================\n");
	return;
}

void process_udp_packet(const unsigned char *buffer)
{
	struct udphdr *udp_pkt = malloc(sizeof(struct udphdr));
	int offset = sizeof(struct ethhdr) +sizeof(struct iphdr);

	memset(udp_pkt, 0, sizeof(struct udphdr));
	memcpy(udp_pkt, buffer, sizeof(struct udphdr));
	fprintf(file, "------------------\n");
	fprintf(file, "UDP Packet Details\n");
	fprintf(file, "------------------\n");
	fprintf(file, "Source Port = %d\n", ntohs(udp_pkt->source));
	fprintf(file, "Destination Port = %d\n", ntohs(udp_pkt->dest));
	fprintf(file, "Length = %d\n", ntohs(udp_pkt->len));
	fprintf(file, "Checksum = %d\n", ntohs(udp_pkt->check));
	fprintf(file, "====================================================\n");
	return;

}
