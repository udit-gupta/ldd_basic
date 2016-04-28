KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := CDD.o

all: run clean

run: load CDDapp
	echo "Hello World" > /dev/CDD;
	cat < /dev/CDD;
	echo "Hello World" > /dev/CDD;
	cat < /dev/CDD;
	-./CDDapp &
	-sleep 1; 
	-@echo "######################################################"
	-@echo "# -------------------------------------------------- #"
	-@echo "# Note: we are reading the dev buffer directly, here #";
	-cat /dev/CDD | tee CDD.out
	-pmap `ps -f | grep CDDapp | grep -v grep |  awk '{print $$2}'`
	-@echo "# -------------------------------------------------- #"
	-@echo "######################################################"
	@sync;sync;sync;sleep 3
	@sleep 10
	

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
	-su -c " rmmod CDD; \
	rm /dev/CDD;"

clean: unload
	-@rm -fr *.o CDD*.o CDD*.ko .CDD*.* CDD*.*.* CDDapp .tmp_versions [mM]odule* 

