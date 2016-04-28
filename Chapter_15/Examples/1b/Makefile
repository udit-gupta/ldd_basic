KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := CDD.o

all: run 
	sleep 12
	@make clean

run: load CDDapp
	echo "Hello World" > /dev/CDD;
	cat < /dev/CDD;
	echo "Hello World" > /dev/CDD;
	cat < /dev/CDD;
	-./CDDapp &
	pmap `ps -C CDDapp|grep CDDapp|awk '{print $$1}'| head -1`
	-@sleep 10

load: CDD.o
	-su -c "insmod ./CDD.ko; mknod -m 666 /dev/CDD c 32 0;"

CDD.o:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	###
	###  Alternatively, you may want to use the early 2.6 syntax of
	###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	###

CDDapp:
	-gcc -o CDDapp CDDapp.c;

unload:
	-su -c "rmmod CDD;"
	-rm -f /dev/CDD;

clean: unload
	-@rm -fr *.o CDD*.o CDD*.ko .CDD*.* CDD*.*.* CDDapp .tmp_versions [mM]odule*

