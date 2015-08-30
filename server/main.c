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

#include "../emu/udpproto.h"

#define MAXCLIENT 128

typedef struct {
	uint32_t lastSeq; //-1 if not in use, seq otherwise
	uint8_t display[32*48];
	uint16_t icons;
} TamaDisp;


typedef struct {
	uint32_t currSeq;
	uint32_t noTamas;
	TamaDisp tama[MAXCLIENT];
} ShmData;

struct sockaddr_in clientaddr[MAXCLIENT];
volatile ShmData *shm;
int sock;

void handleTamaPacket(int id, TamaUdpData *d, int len) {
	int x, y, p;
	if (d->type==TAMAUDP_IMAGE) {
		p=0;
		for (x=0; x<48; x++) {
			for (y=0; y<48; y++) {
				shm->tama[id].display[p++]=d->d.disp.pixel[y][x];
			}
		}
		shm->tama[id].icons=d->d.disp.icons;
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
	
	shmfd=shm_open("/tamahive", O_RDWR|O_TRUNC|O_CREAT, 0666);
	if (shmfd<0) {
		perror("creating shm");
		exit(1);
	}
	ftruncate(shmfd, sizeof(ShmData));
	shm=mmap(NULL, sizeof(ShmData), PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (shm==NULL) {
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

	while(1) {
		al=sizeof(claddr);
		l=recvfrom(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *) &claddr, &al);
		t=-1;
		for (i=0; i<MAXCLIENT; i++) {
			if (shm->tama[i].lastSeq!=-1 && memcmp(&claddr, &clientaddr[i], al)==0) {
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
			if (i>shm->noTamas && i<MAXCLIENT) shm->noTamas=i;
			if (i<MAXCLIENT) {
				//Okay, init tama struct.
				memcpy(&clientaddr[i], &claddr, al);
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


