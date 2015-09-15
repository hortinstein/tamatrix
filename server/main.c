#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "../emu/udpproto.h"

#define MAXCLIENT 128

typedef struct __attribute((packed)) {
	uint32_t lastSeq; //-1 if not in use, seq otherwise
	uint8_t display[32*48];
	uint16_t icons;
} TamaDisp;


typedef struct __attribute((packed)) {
	uint32_t currSeq;
	uint32_t noTamas;
	TamaDisp tama[MAXCLIENT];
} ShmData;

typedef struct {
	struct sockaddr_in addr;
	int connectedTo;
} Client;

Client client[MAXCLIENT];
volatile ShmData *shm;
int sock;

void handleTamaPacket(int id, TamaUdpData *d, int len) {
	int x, y, p;
	if (d->type==TAMAUDP_IMAGE) {
//		printf("Image for tama id %d\n", id);
		p=0;
		for (y=0; y<32; y++) {
			for (x=0; x<48; x++) {
				shm->tama[id].display[p++]=d->d.disp.pixel[y][x];
			}
		}
		shm->tama[id].icons=d->d.disp.icons;
	} else if (d->type==TAMAUDP_IRSTART) {
		//Find a tama to connect this one to.
		y=100;
		do {
			y--;
			x=rand()%(shm->noTamas);
		} while (y>0 && (x==id || (shm->currSeq - shm->tama[x].lastSeq)>100) );
		client[id].connectedTo=x;
		client[x].connectedTo=id;
		sendto(sock, d, len, 0, (struct sockaddr *)&client[x].addr, sizeof(struct sockaddr_in));
		printf("IRSTART type %d from %d to %d\n", (int)d->d.irs.type, id, x);
	} else if (d->type==TAMAUDP_IRSTARTACK) {
		y=client[id].connectedTo;
		sendto(sock, d, len, 0, (struct sockaddr *)&client[x].addr, sizeof(struct sockaddr_in));
		printf("IRSTARTACK type %d from %d to %d\n", (int)d->d.irs.type, id, y);
	} else if (d->type==TAMAUDP_IRDATA) {
		//Forward packet to connected tama
		y=client[id].connectedTo;
		sendto(sock, d, len, 0, (struct sockaddr *)&client[y].addr, sizeof(struct sockaddr_in));
		printf("IR data from %d to %d len %d spulse %d:", id, y, ntohs(d->d.ir.dataLen), ntohs(d->d.ir.startPulseLen));
		for (x=0; x<ntohs(d->d.ir.dataLen); x++) printf("%02X ", d->d.ir.data[x]);
		printf("\n");
	} else if (d->type==TAMAUDP_BYE) {
		printf("Tama %d says bye. Freeing slot.\n", id);
		shm->tama[id].lastSeq=-1;
		return; //to stop rest of the code fiddling with lastSeq
	}
	p=shm->currSeq+1;
	shm->tama[id].lastSeq=p;
	shm->currSeq=p;
}

int main(int argc, char** argv) {
	int shmfd;
	int optval=1;
	int i, l, t;
	int al;
	TamaUdpData pkt;
	struct sockaddr_in claddr, serveraddr;
	srand(time(NULL));
	
	shmfd=shmget(7531, sizeof(ShmData), IPC_CREAT|0666);
	if (shmfd<0) {
		perror("creating shm");
		exit(1);
	}
	shm=shmat(shmfd, NULL, 0);
	if (shm==(void*)-1) {
		perror("mmapping shm");
		exit(1);
	}

	sock=socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0) {
		perror("creating socket");
		exit(1);
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons(7531);

	if (bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		perror("bind to udp port");
		exit(1);
	}

	shm->currSeq=0;
	shm->noTamas=0;
	for (i=0; i<MAXCLIENT; i++) {
		shm->tama[i].lastSeq=-1;
	}

		client[0].connectedTo=1;
		client[1].connectedTo=0;

	while(1) {
		al=sizeof(claddr);
		l=recvfrom(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *) &claddr, &al);
		t=-1;
		for (i=0; i<MAXCLIENT; i++) {
			if (shm->tama[i].lastSeq!=-1 && memcmp(&claddr, &client[i].addr, al)==0) {
				t=i;
				break;
			}
		}
		if (t==-1) {
			//Create new tama entry
			//First, find unused slot.
			for (i=0; i<shm->noTamas && i<MAXCLIENT; i++) {
				if (shm->tama[i].lastSeq==-1) break;
			}
			//If this expands the noTamas, change that number.
			if (i>=shm->noTamas && i<MAXCLIENT) shm->noTamas=i+1;
			if (i<MAXCLIENT) {
				//Okay, init tama struct.
				memcpy(&client[i].addr, &claddr, al);
				shm->tama[i].lastSeq=0;
				t=i;
				printf("New connection id=%d\n", t);
			}
		}
		if (t!=-1) {
			handleTamaPacket(t, &pkt, l);
		}
	}
}


