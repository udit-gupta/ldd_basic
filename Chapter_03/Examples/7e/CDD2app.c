/*  CDD2app.c */

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
	int fd, len, wlen,i=0;
	char str[128];
	int num, rnum;

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

	// initialize str
	str[i++]=0;
	str[i++]=0;
	str[i++]=0;
	str[i++]=0;
	str[i++]=0;
	
	lseek(fd,0,0);
	// read 
	if ((len = read(fd, str, 3)) == -1) {
		fprintf(stderr,"ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%s\n", str);
	lseek(fd,5,0);
	// read 
	if ((len = read(fd, str, 2)) == -1) {
		fprintf(stderr,"ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%s\n", str);

	close(fd);
}
