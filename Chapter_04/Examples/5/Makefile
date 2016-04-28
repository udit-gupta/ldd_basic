all: run clean

run:
	strace -f -o strace-f.out ls -l
	strace -c -o strace-c.out ls -l

clean:
	rm -f strace??.out
