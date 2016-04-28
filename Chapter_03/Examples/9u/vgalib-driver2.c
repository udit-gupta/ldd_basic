#include <stdio.h>
#include <stdlib.h>
#include <vga.h>
#include <vgagl.h>

GraphicsContext *physicalscreen;
GraphicsContext *virtualscreen;
 
int main(void)
{
   int i,
       j,
       b,
       y,
       c;
	char *term=malloc(60);
 
  term=getenv("TERM");
  if (!(strcmp(term,"xterm"))) {
    fprintf(stderr,"TERM == \"%s\" .. press CTRL+ALT+F[2-7], and run from text mode console.\n", term);
    exit(1);
  }

   vga_init();
   vga_setmode(G320x200x256);
   gl_setcontextvga(G320x200x256);
   physicalscreen = gl_allocatecontext();
   gl_getcontext(physicalscreen);
   gl_setcontextvgavirtual(G320x200x256);
   virtualscreen = gl_allocatecontext();
   gl_getcontext(virtualscreen);
 
   gl_setcontext(virtualscreen);
   y = 0;
   c = 0;
   gl_setpalettecolor(c, 0, 0, 0);
   c++;
   for (i = 0; i < 64; i++)
   {
      b = 63 - i;
      gl_setpalettecolor(c, 0, 0, b);
      for (j = 0; j < 3; j++)
      {
         gl_hline(0, y, 319, c);
         y++;
      }
      c++;
   }
 
   gl_copyscreen(physicalscreen);
 
   vga_getch();
   gl_clearscreen(0);
   vga_setmode(TEXT);

	 free(term);

   return EXIT_SUCCESS;
}

