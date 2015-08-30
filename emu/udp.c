
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include "udp.h"
#include "udpproto.h"

static int sock;
static struct sockaddr_in servaddr;

void udpInit(char *hostname) {
//	struct sockaddr_in cliaddr;
	struct hostent *server;
	server=gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"Error looking up %s\n", hostname);
		exit(1);
	}

	sock=socket(AF_INET,SOCK_DGRAM,0);
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);
	servaddr.sin_port=htons(7531);
}

void udpSendDisplay(Display *d) {
	int x, y;
	TamaUdpData packet;
	packet.type=TAMAUDP_IMAGE;
	for (y=0; y<32; y++) {
		for (x=0; x<48; x++) {
			packet.d.disp.pixel[y][x]=d->p[y][x];
		}
	}
	packet.d.disp.icons=htons(d->icons);
	sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}
