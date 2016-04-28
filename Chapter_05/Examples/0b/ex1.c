#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include <sys/resource.h>

#define SLEEP 5

void* fn(void* argval){
		int st=-1;
		int pri=0;

		/*
			For processes scheduled under SCHED_OTHER scheduling policy
      sched_priority is not used in scheduling decisions 
			(it must be specified as 0).
		*/

	  st=pthread_setschedprio(pthread_self(),pri);

		if (st!=0) {
			fprintf(stderr,"Error: pthread_setschedprio()==%d:%s\n",
				st,strerror(st));
			return;
		}

    fprintf(stderr,"Sleeping '%d' .. \n",SLEEP);
    sleep(SLEEP);
    fprintf(stderr,"Inside fn with message \"%s\".\n",argval);

		pthread_detach(pthread_self());
}

int main(){
		int st, retval;
    pthread_t tid;
    char* targ=(char*)malloc(sizeof(char)*10);

    strcpy(targ,"abcd");
    pthread_create(&tid, 0, fn, (void*)targ);

    retval = pthread_join(tid, (void **)&st);

    printf("Exiting ... \n");
    return 0;
}
