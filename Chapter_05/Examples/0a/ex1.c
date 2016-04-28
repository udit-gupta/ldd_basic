#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>


#include <sys/resource.h>

#define SLEEP 5

int main(){
	int which = PRIO_PROCESS;
	int priority = -20;
	// int priority = 19;

	id_t pid;
	int ret;

	pid = getpid();
	ret = getpriority(which, pid); // returns nice() value
	fprintf(stderr,"Priority of PID=%d is %d;\n",pid,ret);

	sleep(5);

	pid = getpid();
	ret = setpriority(which, pid, priority);
	ret = getpriority(which, pid);
	fprintf(stderr,"Priority of PID=%d is now %d;\n",pid,ret);

	sleep(5);
	
	priority=19;

	pid = getpid();
	ret = setpriority(which, pid, priority);
	ret = getpriority(which, pid);
	fprintf(stderr,"Priority of PID=%d is now %d;\n",pid,ret);

	sleep(5);
}
