#include <stdio.h>

int main() {
	int c;

	printf("Hello World!\n");

	fprintf(stderr,"Enter a char => (PID==%d)",getpid());
	c=getchar();
}
