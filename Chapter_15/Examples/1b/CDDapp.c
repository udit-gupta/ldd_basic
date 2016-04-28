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
#include <sys/mman.h>
#include <sys/wait.h>

int main() {
	int fd;
	int sz = 4096;
	int st = 0;

	char *m;
	pid_t pid;

	printf("Test mmap...\n");
	// open 
	if((fd = open("/dev/CDD", O_RDWR)) == -1) {
		fprintf(stderr,"ERR:on open():%s\n",strerror(errno));
		exit(0);
	}

	// mmap
	m = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, 0);

	if ((pid=fork()) == 0) {
		sprintf(m, "Hello from child, pid %d", getpid());

		sleep(10);
	} else {
		printf ("Hello from parent, pid %d\n", getpid());
		wait(&st);
		printf ("Replaying msg from child: %s\n", m);
		sleep(10);
		close(fd);
	}

	exit(0);
}
