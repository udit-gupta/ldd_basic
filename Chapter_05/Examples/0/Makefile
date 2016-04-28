all: run clean

run: ex1
	(time ./ex1;) &
	vmstat 5 5 &
	sleep 30

clean:
	pkill ex1
	rm -f ex1
