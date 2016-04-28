all: clean main 
	-./main
	@make -s clean

vgalib-driver: vgalib-driver.c
	gcc -O3 -o vgalib-driver -lvga vgalib-driver.c

vgalib-driver2: vgalib-driver2.c
	gcc -O3 -o vgalib-driver2 -lvgagl -lvga vgalib-driver2.c

clean:
	-rm -f main main.o vgalib-driver vgalib-driver.o vgalib-driver2 vgalib-driver2.o

	
