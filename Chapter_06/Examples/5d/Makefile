KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# CDDparm := 1
# CDDparm := $(CDDparm)
CDDparm := 0  ## TBD
CDDparm := 32

obj-m := CDD2.o

all: clean run 
	-ls -l /dev/CDD2_? 
	dmesg | tail -20
	@make -s clean
	dmesg | tail -10
	@make -s mem

mem:
	ls -l /dev | egrep "^c" | egrep "[ 	][ 	]*1," | sort -k6.1n

run: load CDD2app runproc
	-ls -l /dev/CDD2_? 
	echo "Hello World" > /dev/CDD2_a;
	echo "Hello Universe" > /dev/CDD2_b;
	echo "Hello Galaxy" > /dev/CDD2_c;
	-echo "Hello Space" > /dev/CDD2_l;
	-echo "Hello Planet" > /dev/CDD2_m;
	-echo "Hello Moon" > /dev/CDD2_n;
	cat < /dev/CDD2_a;
	cat < /dev/CDD2_b;
	cat < /dev/CDD2_c;
	-cat < /dev/CDD2_l;
	-cat < /dev/CDD2_m;
	-cat < /dev/CDD2_n;
	./CDD2app;

runproc:
	cat < /proc/myCDD/hello;
	echo "1234" > /proc/myCDD/hello;
	cat < /proc/myCDD/hello;
	cat < /proc/myCDD/hello;

runappend: load CDD2app
	echo "Hello World" > /dev/CDD2_a;
	echo "Hello Universe" > /dev/CDD2_b;
	echo "Hello Galaxy" > /dev/CDD2_b;
	echo "Hello Space" > /dev/CDD2_l;
	echo "Hello Planet" > /dev/CDD2_m;
	echo "Hello Moon" > /dev/CDD2_n;
	cat < /dev/CDD2_a;
	cat < /dev/CDD2_b;
	cat < /dev/CDD2_c;
	cat < /dev/CDD2_l;
	cat < /dev/CDD2_m;
	cat < /dev/CDD2_n;

load: CDD2.o 
	-su -c "insmod ./CDD2.ko CDDparm=$(CDDparm);";
	-cat /proc/devices | grep CDD

CDD2.o:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

###
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
###

CDD2app:
	-gcc -o CDD2app CDD2app.c;

unload:
	-su -c "rmmod CDD2; rm -fr /dev/CDD2*;"
	-cat /proc/devices | grep CDD

clean: unload
	-@rm -fr *.o CDD2*.o CDD2*.ko .CDD2*.* CDD2*.*.* CDD2app .tmp_versions [mM]odule*

