
//
// CDD2app.c is an pthread application used to test Char Driver 
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define CMD1 1
#define CMD2 2
#define MYNUM 0x88888888
#define MYSTR "Eureka!"
#define MAX_LENGTH		256
#define READ_LENGTH		256

struct my_struct {
	int				fd;
	int				stop;
	char 			*read_buf;
	char 			*write_buf;
	pthread_mutex_t RW_mutex;
};

//Use to read data from driver
void *read_function(void *arg)
{

	int		i, len;

	struct my_struct	*myData = (struct my_struct *)arg;

	myData->read_buf = malloc(READ_LENGTH);

	for(i=0;;i++)
	{
     	pthread_mutex_lock( &(myData->RW_mutex) );
		if ((len = read(myData->fd, myData->read_buf, READ_LENGTH)) == -1)
		{
			fprintf(stdout,"ERR:on read():%s\n",strerror(errno));
		}
		else
		{
			fprintf(stdout, "%d - read \"%s\"  len=%d\n", i, myData->read_buf, len);
		}
		pthread_mutex_unlock(&(myData->RW_mutex));

		memset((void *)myData->read_buf, '\0', READ_LENGTH);
		sleep(1);

		if(myData->stop)
		{
			break;
		}
	}

	free(myData->read_buf);
	return NULL;
}

//Use to write data to driver
void *write_function(void *arg)
{
	int		i, wlen;
	struct my_struct *myData = (struct my_struct *)arg;

	wlen = strlen(myData->write_buf);

	for(i=0;;i++)
	{
     	pthread_mutex_lock(&(myData->RW_mutex));
		if ((wlen = write(myData->fd, myData->write_buf, wlen)) == -1) 
		{
			fprintf(stdout,"ERR:on write():%s\n",strerror(errno));
		}
		else
		{
			fprintf(stdout, "%d - write count=%d\n", i, wlen);
		}
		pthread_mutex_unlock(&(myData->RW_mutex));
		
		//write faster than read to prevent overflow
		usleep(700000);
		
		if(myData->stop)
		{
			break;
		}
	}

	return NULL;
}

int main(int argc, char *argv[]) 
{
	int			i, j, status;
	int			file_type, file_flag;

	pthread_t	readThread, writeThread;
	pthread_mutex_t RW_mutex = PTHREAD_MUTEX_INITIALIZER;

	struct my_struct	myData;
	myData.write_buf = malloc(MAX_LENGTH);
	memset((void *)myData.write_buf, '\0', MAX_LENGTH);
	myData.stop = 0;
    myData.RW_mutex = RW_mutex;

	strcpy(myData.write_buf, MYSTR);

	file_type = O_RDWR;
	file_flag = O_APPEND;

	//Check options
	if(argc == 4)
	{
		switch(atoi(argv[1]))
		{
			case 3:
				file_type = O_WRONLY;
				break;
			case 2:
				file_type = O_RDONLY;
				break;
			case 1:
			default:
				file_type = O_RDWR;
				break;
			
		}

		switch(atoi(argv[2]))
		{
			case 2:
				file_flag = O_TRUNC;
				break;
			case 1:
			default:
				file_flag = O_APPEND;
				break;
		}

		strcpy(myData.write_buf, argv[3]);
	}
	else
	{
		fprintf(stdout, " help menu..\n");
		fprintf(stdout, " ./CDD2 arg1 arg2 arg3\n");
		fprintf(stdout, "   arg1: open file type 1-RW, 2-Read only, 3-write only\n");
		fprintf(stdout, "   arg2: file write control 1-append, 2-trunc\n");
		fprintf(stdout, "   arg3: string for testing, max size=132 bytes\n");

		return 0;
	}

	fprintf(stdout, "Open device\n");

	// open device
	if((myData.fd = open("/dev/CDD2", file_type | file_flag)) == -1)
	{
		fprintf(stderr,"ERR:on open():%s\n",strerror(errno));
		free(myData.write_buf);
		return 0;	//exit(0);
	}

	fprintf(stdout, "Create read thread\n");
	if((status = pthread_create(&readThread, NULL, read_function, &myData)) != 0)
	{
		return EXIT_FAILURE;
	}

//	sleep(1);

	fprintf(stdout, "Create write thread\n");
	if((status = pthread_create(&writeThread, NULL, write_function, &myData)) != 0)
	{
		return EXIT_FAILURE;
	}

	//Waiting for user to enter key to terminate the program
	// if(getchar())
	sleep(10);

		myData.stop = 1;


  	fprintf(stdout, "Delete write thread\n");
  	status = pthread_join(writeThread, NULL);

  	fprintf(stdout, "Delete read thread\n");
 	status = pthread_join(readThread, NULL);

	free(myData.write_buf);
	close(myData.fd);

	return EXIT_SUCCESS;

}

