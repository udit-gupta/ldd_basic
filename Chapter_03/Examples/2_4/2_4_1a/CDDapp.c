/*  CDDapp.c */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define CMD1 1
#define CMD2 2
#define MYNUM 0x88888888
#define MYSTR "Eureka! This is a real discovery!!"

main() {
	int fd, len, wlen;

	int c;
	char str[128];
	// char str[1];
	int num, rnum;

	strcpy(str, MYSTR);

	// open 
	if((fd = open("/dev/CDD", O_RDWR | O_APPEND)) == -1) {
		fprintf(stderr,"ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

	// write 
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

	// read 
	if ((len = read(fd, str, 12)) == -1) {
		fprintf(stderr,"ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%s\n", str);


	fprintf(stderr,"Waiting for Input\n");
	c=getchar();
	close(fd);
}
