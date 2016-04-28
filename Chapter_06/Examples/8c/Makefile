all: run clean

run: sparse
	./sparse
	ls -l myfile.dat
	du -k myfile.dat

sparse: sparse.c

clean:
	rm -f myfile.dat sparse
