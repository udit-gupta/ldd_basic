#include <stdio.h>
#include <fcntl.h>

int main() 
{
	int c, fd;
	fprintf(stderr,"PID==%d\n",getpid());

	fd=open("myfile.txt",O_WRONLY|O_TRUNC|O_CREAT);
	
	c=getchar();
}	
