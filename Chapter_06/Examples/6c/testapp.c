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

#define MYSTR "Eureka!"
#define LONGSTR "This is a long string! ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789"

main() {
	int fd, len, wlen;
	char str[128];
	int num, rnum;

	strcpy(str, MYSTR);

	// open 
	if((fd = open("/tmp/CDD2.out", O_CREAT|O_WRONLY)) == -1) {
		fprintf(stderr,"ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

	// write 
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

	lseek(fd,4*1024*1024,SEEK_END);

	// write 
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

  close(fd);
}
