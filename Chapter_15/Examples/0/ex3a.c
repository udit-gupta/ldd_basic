#include <stdio.h>
#include <malloc.h>

char buf[8*1024*1024]=" ";

int main() {
	int c;

	printf("Hello World!\n");

	fprintf(stderr,"Enter a char => (PID==%d)",getpid());
	c=getchar();
}
