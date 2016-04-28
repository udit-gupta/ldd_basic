//  Example 7.2a:  
//  userspaceapp.c

//  Though non-standard, some distributions of Linux have exported the 
//  rdtsc() functionality to userspace.

//  here is a sample userspace program. save it as userspaceapp.c

//  to compile the program:  use 'make userspaceapp'
//  to run the program:  use './userspaceapp'

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <asm/msr.h>

int main(int argc, char **argv)
{
	unsigned long startl, endl;
	unsigned long long startll, endll;
	rdtscl(startl);
	sleep(1);
	rdtscl(endl);
	
	printf("start=%ld,end=%ld,elapsed=%ld\n",startl,endl,endl-startl);

	rdtscll(startll);		// long long .. 64-bit implementation
	sleep(1);
	rdtscll(endll);			// long long .. 64-bit implementation
	
	printf("start=%lld,end=%lld,elapsed=%lld\n",startll,endll,endll-startll);
}
