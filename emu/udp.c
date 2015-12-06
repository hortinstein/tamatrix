/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include "udp.h"
#include "udpproto.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "ir.h"
#include "benevolentai.h"

static int sock;
static struct sockaddr_in servaddr;
static char olddisp[32][48];


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


void udpTick() {
	TamaUdpData packet;
	fd_set rfds;
	struct timeval tv;
	int r;
	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);
	tv.tv_sec=0;
	tv.tv_usec=0;
	r=select(sock+1, &rfds, NULL, NULL, &tv);
	if (r==1) {
		read(sock, &packet, sizeof(TamaUdpData));
		if (packet.type==TAMAUDP_IRSTART) {
			benevolentAiReqIrComm(packet.d.irs.type);
		} else if (packet.type==TAMAUDP_IRSTARTACK) {
			benevolentAiAckIrComm(packet.d.irs.type);
		} else if (packet.type==TAMAUDP_IRDATA) {
			irRecv(packet.d.ir.data, ntohs(packet.d.ir.dataLen));//, ntohs(packet.d.ir.startPulseLen));
		}
	}
}

void udpExit() {
	TamaUdpData packet;
	packet.type=TAMAUDP_BYE;
	sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	close(sock);
}


void udpSendIrstartReq(int type) {
	TamaUdpData packet;
	packet.type=TAMAUDP_IRSTART;
	packet.d.irs.type=type;
	sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void udpSendIrstartAck(int type) {
	TamaUdpData packet;
	packet.type=TAMAUDP_IRSTARTACK;
	packet.d.irs.type=type;
	sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void udpSendDisplay(Display *d) {
	int x, y;
	int chg=0;
	//Compare first, don't send udp packet if no change has occurred.
	for (y=0; y<32; y++) {
		if (memcmp(olddisp[y], d->p[y], 48)!=0) {
			chg=1;
			break;
		}
	}
	if (!chg) return;

	TamaUdpData packet;
	packet.type=TAMAUDP_IMAGE;
	for (y=0; y<32; y++) {
		for (x=0; x<48; x++) {
			packet.d.disp.pixel[y][x]=d->p[y][x];
			olddisp[y][x]=d->p[y][x];
		}
	}
	packet.d.disp.icons=htons(d->icons);
	sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

void udpSendIr(char *data, int len,  int startPulseLen) {
	TamaUdpData packet;
	packet.type=TAMAUDP_IRDATA;
	packet.d.ir.startPulseLen=htons(startPulseLen);
	memcpy(packet.d.ir.data, data, len);
	packet.d.ir.dataLen=htons(len);
	sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

