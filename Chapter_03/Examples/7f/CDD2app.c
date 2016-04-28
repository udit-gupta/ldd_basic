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
#define MYSTR2 "Hello World 13049872138472130984721398749832174."

main() {
	int fd, len, wlen,i=0;
	char str[128];
	int num, rnum;

	strcpy(str, MYSTR);

	// open 
	if((fd = open("/dev/CDD2", O_WRONLY | O_APPEND)) == -1) {
		fprintf(stderr,"1. ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

	// write 
	wlen = strlen(str);
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"1. ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

	close(fd);

	strcpy(str, MYSTR2);
	wlen = strlen(str);

	if((fd = open("/dev/CDD2", O_WRONLY|O_APPEND)) == -1) {
		fprintf(stderr,"2. ERR:on open():%s\n",strerror(errno));
		exit(0);
	}
	if ((len = write(fd, str, wlen)) == -1) {
		fprintf(stderr,"2. ERR:on write():%s\n",strerror(errno));
		exit(1);
	}

	close(fd);

	if((fd = open("/dev/CDD2", O_RDONLY)) == -1) {
		fprintf(stderr,"3. ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

//
	i=0; while(i++<128) str[i]=0;
	// read 
	if ((len = read(fd, str, 60)) == -1) {
		fprintf(stderr,"3. ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%s\n", str);
//
	i=0; while(i++<128) str[i]=0;
	lseek(fd,5,0);
	// read 
	if ((len = read(fd, str, 2)) == -1) {
		fprintf(stderr,"3. ERR:on read():%s\n",strerror(errno));
		exit(1);
	}
	fprintf(stdout, "%s\n", str);

	close(fd);
}
