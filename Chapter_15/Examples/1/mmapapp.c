#include
#include
#include
#include
#include
#include
#include
<stdlib.h>
<stdio.h>
<unistd.h>
<string.h>
<sys/mman.h>
<sys/types.h>
<sys/wait.h>
int main() {
int fd = -1;
int sz = 4096;
int st = 0;
char *m;
pid_t pid;
m=mmap(NULL, sz, PROT_READ| PROT_WRITE, MAP_SHARED|
MAP_ANONYMOUS, fd, 0);
}
f)
if ((pid=fork()) == 0)
sprintf(m, “Hello: from child pid %d”, getpid());
else {
printf(“Hello from Parent. Pid %d\n”, getpid());
wait(&st);
printf(“Relaying msg from child %s”, m);
}
exit(0);

