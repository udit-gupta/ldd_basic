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
    pthread_attr_t tattr;
	  struct sched_param param;
    char* targ=(char*)malloc(sizeof(char)*10);

    pthread_attr_init(&tattr);
	  param.sched_priority = 20;

	  // pthread_setschedparam(pthread_self(),SCHED_OTHER, &param);
	  // pthread_setschedparam(pthread_self(),SCHED_FIFO, &param);
    sleep(SLEEP);
	  pthread_setschedparam(pthread_self(),SCHED_RR, &param);

    sleep(SLEEP);
    strcpy(targ,"abcd");
    pthread_create(&tid, &tattr, fn, (void*)targ);

    retval = pthread_join(tid, (void **)&st);

    printf("Exiting ... \n");
    return 0;
}
