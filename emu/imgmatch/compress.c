#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define FLAG_SKIP 0x80

int main(int argc, char **argv) {
	char out[48*32];
	char line[1024];
	int p=0;
	int x,y;
	int rle=0;
	if (argc!=2) {
		printf("Usage: %s varName <inFile.lcd >>outFile.inc\n", argv[0]);
		exit(0);
	}
	for (y=0; y<32; y++) {
		fgets(line, 1023, stdin);
		for (x=0; x<48; x++) {
			if (line[x]==' ') {
				rle++;
				if (rle==127) {
					out[p++]=rle|FLAG_SKIP;
					rle=0;
				}
			} else {
				if (rle!=0) {
					out[p++]=rle|FLAG_SKIP;
					rle=0;
				}
				out[p++]=line[x];
			}
		}
	}
	printf("static const unsigned char %s_data[]={", argv[1]);
	for (x=0; x<p; x++) {
		if ((x&15)==0) {
			printf("\n\t");
		}
		printf("0x%02hhX", out[x]);
		if (x!=p-1) printf(", ");
	}
	printf("};\nconst unsigned char *%s=&%s_data[0];\n\n", argv[1], argv[1]);
	
	exit(0);
}