#include <stdio.h>
#include <fcntl.h>

int main() 
{
	int c;

	int fd;

	fd=open("myfile.txt",O_RDONLY);

	fprintf(stderr,"MyPID==%d\n",getpid());
	c=getchar();

	return 0;
}
