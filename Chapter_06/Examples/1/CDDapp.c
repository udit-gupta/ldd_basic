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

// /*    */
// IOCTL commands ... 
#define READ 0
#define WRITE 1
// 


#define MYNUM 5558
#define MYSTR "Eureka!"

main() {
	int fd, len, wlen;
	char str[128];
	unsigned long inum = MYNUM;
	unsigned long onum;

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
	if ((len = read(fd, str, sizeof(str))) == -1) {
		fprintf(stderr,"ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%s\n", str);

	// write using ioctl()
	if ((len = ioctl(fd, WRITE, (int *)inum)) == -1) {
		fprintf(stderr,"ERR:on ioctl-write():%s\n",strerror(errno));
		exit(1);
	}

	// read using ioctl()
	if ((len = ioctl(fd, READ, &onum)) == -1) {
		fprintf(stderr,"ERR:on ioctl-read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "ioctl read .. %#0x(%d).  len=%d\n", onum,onum,len);

	close(fd);
}
