/*
 * client.c
 *	UDP+IP RAW socket
 *  Created on: 29 мар. 2018 г.
 *      Author: jake
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

unsigned short csum(unsigned short*, int);

struct udp_header{ //Заголовок UDP
	short source;
	short dest;
	short len;
	short check;
}udph;
struct ip_header{ // Заголовок IP
 	u_char ip_vhl;  /* версия << 4 | длина заголовка >> 2 */
    u_char ip_tos;  /* тип службы */
    u_short ip_len;  /* общая длина */
    u_short ip_id;  /* идентификатор */
    u_short ip_off;  /* поле фрагмента смещения */
    #define IP_RF 0x8000  /* reserved флаг фрагмента */
    #define IP_DF 0x4000  /* dont флаг фрагмента */
    #define IP_MF 0x2000  /* more флаг фрагмента */
    #define IP_OFFMASK 0x1fff /* маска для битов фрагмента */
    u_char ip_ttl;  /* время жизни */
    u_char ip_p;  /* протокол */
    u_short ip_sum;  /* контрольная сумма */
    struct in_addr ip_src,ip_dst; /* адрес источника и адрес назначения */
} iph;
#define IP_HL(ip)  (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)  (((ip)->ip_vhl) >> 4)

int main(void)
{
	int sock,i;
	int yes=1;
	char datagram[248];
	char payload_ip[256];
	struct sockaddr_in local;

	memset(datagram,0,sizeof(datagram));
	memset(payload_ip,0,sizeof(payload_ip));
	memset(&local,0,sizeof(local));

	strcat(datagram,"Hello RAW");

    //UDP header
    udph.source = htons(1234);
    udph.dest = htons(1235);
    udph.len = htons(sizeof(udph) + strlen(datagram));
    udph.check = 0;

    //IP header
    iph.ip_vhl = 4;
    iph.ip_vhl = iph.ip_vhl<<4 | 5;
    iph.ip_tos = 0;
    iph.ip_len = htons(sizeof(iph)+sizeof(udph)+strlen(datagram));
    iph.ip_id = htons(444);
    iph.ip_off = htons((0&IP_RF) | (0&IP_DF) | (0&IP_MF));
    iph.ip_ttl=64;
    iph.ip_p = 17;
    iph.ip_sum = htons(csum ((unsigned short *) &iph, sizeof(iph)));
    inet_aton("127.0.0.1",&(iph.ip_dst));
    inet_aton("127.0.0.1",&(iph.ip_src));

    // Сборка пакета
    memcpy(payload_ip,&iph,sizeof(iph));
    memcpy(payload_ip+sizeof(iph),&udph,sizeof(udph));
    memcpy(payload_ip+sizeof(iph)+sizeof(udph),datagram,strlen(datagram));

    // Открытие и настройка RAW socket
	if((sock=socket(AF_INET,SOCK_RAW,IPPROTO_UDP)) < 0)
	{
		perror("socket error");
		exit(1);
	}
	if ( setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &yes, sizeof(int)) == -1 )
		{
		  perror("setsockopt() error");
		  exit(1);
		}

	sendto(sock,payload_ip,htons(iph.ip_len),0,(struct sockaddr*)&local,sizeof(local));
	recvfrom(sock,payload_ip,sizeof(payload_ip),0,0,0);
	recvfrom(sock,payload_ip,sizeof(payload_ip),0,0,0);
	printf("%s\n",payload_ip+sizeof(iph)+sizeof(udph));

	close(sock);
	return 0;
}
