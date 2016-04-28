#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>

int main() {

	char *wfn="myfile.txt";
	int wfd=0;
	int n=0;
	char *buf=malloc(128);


	wfd=open(wfn,O_WRONLY|O_CREAT|O_TRUNC,0660);
	
	sprintf(buf,"The name of this file is '%s'\n.",wfn);
	n=write(wfd,buf,strlen(buf));

	lseek(wfd,32*4096000,SEEK_END);

	sprintf(buf,"The name of this file AGAIN is '%s'\n.",wfn);
	n=write(wfd,buf,strlen(buf));

	close(wfd);

	return 0;
}
	

	
