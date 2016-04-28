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

#define LEN 1024*1024*128

int main() {
	int fd, len, wlen;
	char str[128];
	int num, rnum;

	// open 
	if((fd = open("/tmp/CDD2.out", O_CREAT|O_WRONLY|O_TRUNC)) == -1) {
		fprintf(stderr,"ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

	// write 
	str[0]='H';
	str[1]=0;
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

	lseek(fd,LEN,SEEK_END);

	// write 
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

  close(fd);
}
