/*  CDDapp.c */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define CMD1 1
#define CMD2 2
#define MYNUM 0x88888888
#define MYSTR "Eureka!"

main() {
	int fd, len, wlen;
	char str[128];
	int num, rnum, i=0;

	strcpy(str, MYSTR);

	// open 
	if((fd = open("/dev/CDD2", O_RDWR)) == -1) {
		fprintf(stderr,"ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

	// write 
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

	// init local buffer.
	while (i++<16)	str[i]=0;
	
	// read 
	// if ((len = read(fd, str, sizeof(str))) == -1) {
	lseek(fd,0,0);
	if ((len = read(fd, str, 6)) == -1) {
		fprintf(stderr,"ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stderr, "%s", str);
	printf("\n");

	close(fd);
}
