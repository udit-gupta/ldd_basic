// static on stack
#include <stdio.h>
#include <malloc.h>


int main() {

	static char buf[6*1024*1024];
	int c;

	printf("Hello World!\n");

	fprintf(stderr,"Enter a char => (PID==%d)",getpid());
	c=getchar();
}
