#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#define SLEEP 5

void* fn(void* argval){
    fprintf(stderr,"Sleeping '%d' .. \n",SLEEP);
    sleep(SLEEP);
    fprintf(stderr,"Inside fn with message \"%s\".\n",argval);

		pthread_detach(pthread_self());
}

int main(){
    pthread_t tid;
    int st, retval;
	  struct sched_param param;
    char* targ=(char*)malloc(sizeof(char)*10);

    /*
      For processes scheduled under SCHED_OTHER scheduling policy
      sched_priority is not used in scheduling decisions 
      (it must be specified as 0).
    */

	  param.sched_priority = 0;
	  pthread_setschedparam(pthread_self(),SCHED_OTHER, &param);

    strcpy(targ,"abcd");
    pthread_create(&tid, 0, fn, (void*)targ);

    retval = pthread_join(tid, (void **)&st);

    printf("Exiting ... \n");
    return 0;
}
