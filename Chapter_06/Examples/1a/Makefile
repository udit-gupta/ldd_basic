## http://tldp.org/LDP/lkmpg/2.6/html/x892.html
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := char_dev.o

all: run clean

run: load char_devapp
	echo "Hello World" > /dev/char_dev;
	cat < /dev/char_dev;
	echo "Hello World" > /dev/char_dev;
	cat < /dev/char_dev;
	-./char_devapp;

load: char_dev.o
	-su -c "insmod ./char_dev.ko; mknod -m 666 /dev/char_dev c 100 0;"

char_dev.o:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

char_devapp:
	-gcc -o char_devapp char_devapp.c;

unload:
	-su -c " rmmod char_dev; \
	rm /dev/char_dev;"

clean: unload
	-@rm -fr *.o char_dev*.o char_dev*.ko .char_dev*.* char_dev*.*.* char_devapp .tmp_versions [mM]odule*

