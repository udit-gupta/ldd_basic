#include <stdio.h>
#include <malloc.h>

int main() {
	int c;

	char *buf;

	buf=malloc(8*1024*1024);

	printf("Hello World!\n");

	fprintf(stderr,"Enter a char => (PID==%d)",getpid());
	c=getchar();
	free(buf);
}
