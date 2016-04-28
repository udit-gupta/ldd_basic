all: run clean

LDFLAGS="-lpthread"

run: ex1
	time ./ex1 &
	-(sleep 3; ps -C ex1 -L -o pid,tid,class,rtprio,ni,pri,psr,pcpu,stat,wchan:25,comm)
	-(sleep 5; ps -C ex1 -L -o pid,tid,class,rtprio,ni,pri,psr,pcpu,stat,wchan:25,comm)
	-(sleep 5; ps -C ex1 -L -o pid,tid,class,rtprio,ni,pri,psr,pcpu,stat,wchan:25,comm)

ex1: ex1.c

clean:
	rm -f ex1
