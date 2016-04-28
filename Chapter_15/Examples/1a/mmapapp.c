#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
	int fd = -1;
	int sz = 4096;
	int st = 0;
	char *m;
	pid_t pid;

	char c;

	m=mmap(NULL, sz, PROT_READ| PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, fd, 0);

	fprintf(stderr,"Enter to continue: (PID==%d)",getpid());
  c=getchar();

	if ((pid=fork()) == 0)
		sprintf(m, "Hello: from child pid %d\n", getpid());
	else {
		printf("Hello from Parent. Pid %d\n", getpid());
		wait(&st);
		printf("Relaying msg from child %s", m);
	}
exit(0);
}
