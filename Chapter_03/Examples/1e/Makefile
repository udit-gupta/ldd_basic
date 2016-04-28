KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := CDD2.o CDD3.o

all: clean run 
	@make -s clean

run: load CDD2app
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	echo "Hello Galaxy" > /dev/CDD3;
	cat < /dev/CDD3;
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	echo "Hello Galaxy" > /dev/CDD3;
	cat < /dev/CDD3;
	./CDD2app;

load: CDD2.o CDD3.o
	-su -c "insmod ./CDD2.ko; mknod -m 666 /dev/CDD2 c 32 0;"
	-su -c "insmod ./CDD3.ko; mknod -m 666 /dev/CDD3 c 32 10;"
	ls -l /dev/CDD?
	grep CDD /proc/devices

CDD2.o:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

###
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
###

CDD2app:
	-gcc -o CDD2app CDD2app.c;

unload:
	-su -c " rmmod CDD2; \
	rm -fr /dev/CDD2;"
	-su -c " rmmod CDD3; \
	rm -fr /dev/CDD3;"

clean: unload
	-@rm -fr *.o CDD?*.o CDD?*.ko .CDD?*.* CDD?*.*.* CDD2app .tmp_versions [mM]odule*

