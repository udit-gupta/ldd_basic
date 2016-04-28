MYUSBVendorID = 090c
MYUSBProductID = 1000
MYUSBVolumeLabel = MYLDD

##  Do not edit following line
MYUSBIDString = $(MYUSBVendorID):$(MYUSBProductID)

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
##  Do not edit following lines

obj-m := main.o
main-objs := CDD2.o USB.o

all: clean run 
	make -s clean

run: load CDD2app
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	./CDD2app;

load: compile
	-su -c "insmod ./main.ko; mknod -m 666 /dev/CDD2 c 32 0;"
	ls -l /dev/CDD2
	lsmod | grep main
	-cat /proc/devices | grep CDD

compile:
	@echo "Check VendorID & ProductID in Makefile"
	@lsusb | egrep --color=ALWAYS "ID $(MYUSBIDString)[ 	]*$(MYUSBString)" 
	KCPPFLAGS="-DUSBVENDORID=0x$(MYUSBVendorID) -DUSBPRODUCTID=0x$(MYUSBProductID)" \
	$(MAKE) -C $(KDIR) M=$(PWD) modules

###
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
###

CDD2app:
	-gcc -o CDD2app CDD2app.c;

unload:
	-su -c " rmmod main;"
	-rm -fr /dev/CDD2;

clean: unload
	-@rm -fr *.ko *.o CDD2app .tmp_versions [mM]odule* *.mod.c

mem:
	ls -l /dev | egrep "^c" | egrep "[ 	][ 	]*1," | sort +5.0n -6.0
