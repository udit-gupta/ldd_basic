#include <stdio.h>
#include <stdlib.h>
#include <vga.h>

int main()
{
	char *term=malloc(60);

	term=getenv("TERM");
	if (!(strcmp(term,"xterm"))) {
		fprintf(stderr,"TERM == \"%s\" .. press CTRL.ALT.F[2-7], and run from text mode console.\n", term);
		exit(1);
	}

	vga_init();
	vga_setmode(G320x200x256);
	vga_setcolor(4);
	vga_drawpixel(10, 10);
	
	sleep(5);
	vga_setmode(TEXT);
	
	return EXIT_SUCCESS;
}
