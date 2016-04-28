//  yum -y install svgalib-devel
//
//
// sourced from http://tldp.org/LDP/khg/HyperNews/get/devices/fake.html
// Also, see http://www.linuxjournal.com/article/2783
//
//
// In /dev/mem.
// - GRAPH_SIZE is the size of VGA memory, and 
// - GRAPH_BASE is the first address of VGA memory 
//
//
// 
// See, http://www.cab.u-szeged.hu/local/linux/doc/khg/node15.html
//      http://www.mjmwired.net/kernel/Documentation/mtrr.txt  // Memory-Type Range Register
//
// Also,see /etc/vga/libvga.config

#define GRAPH_BASE 0xe0040000

// GRAPH_SIZE=768k
#define GRAPH_SIZE 0xc0000
#include <stdio.h>
#include <stdlib.h>		// needed for exit()
#include <unistd.h>		// needed for exit()
#include <fcntl.h>
#include <sys/mman.h>  // needed for mmap() macros
#include <sys/user.h>  // needed for PAGE_SIZE macro
#

int main() {
	int mem_fd=0;

	unsigned char *graph_mem; // http://linux.die.net/man/3/graph_mem

	if ((mem_fd = open("/dev/mem", O_RDWR) ) < 0) {
 		printf("VGAlib: can't open /dev/mem \n");
		exit (-1);
	}

 	/* mmap graphics memory */
 	if ((graph_mem = malloc(GRAPH_SIZE + (PAGE_SIZE-1))) == NULL) {
 		printf("VGAlib: allocation error \n");
		exit (-1);
	}

	if ((unsigned long)graph_mem % PAGE_SIZE)
        graph_mem += PAGE_SIZE - ((unsigned long)graph_mem % PAGE_SIZE);

	graph_mem = (unsigned char *)mmap(
		(caddr_t)graph_mem, 
		GRAPH_SIZE,
    PROT_READ|PROT_WRITE,
    MAP_SHARED|MAP_FIXED,
    mem_fd, 
    GRAPH_BASE
	);

    if ((long)graph_mem < 0) {
        printf("VGAlib: mmap error \n");
        exit (-1);
    }

		close(mem_fd);
}
