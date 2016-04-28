MYUSBVendorID = 090c
MYUSBProductID = 1000
MYUSBVolumeLabel = MYLDD

##  Do not edit following 3 lines
MYUSBIDString = $(MYUSBVendorID):$(MYUSBProductID)
# EP_IN=$(shell lsusb -d $(MYUSBIDString) -v | egrep "EP.*IN" | sed -e 's/.*bEndpointAddress  *//' -e 's/EP.*IN.*//')
# EP_OUT=$(shell lsusb -d $(MYUSBIDString) -v | egrep "EP.*OUT" | sed -e 's/.*bEndpointAddress  *//' -e 's/EP.*OUT.*//')

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
##  Do not edit following lines

obj-m := main.o
main-objs := CDD2.o USB.o

all: clean run 
	make -s clean
	sleep 3
	@echo "see /usr/src/linux/Documentation/usb/proc_usb_info.txt"

prep: 
	-umount /media/$(MYUSBVolumeLabel) 
	-{ lsmod | grep usb_storage; } &&  { modprobe -r usb-storage; }

reset:
	-modprobe usb-storage

run: load CDD2app
	clear
	#-@echo "EP_IP==$(EP_IN) EP_OUT==$(EP_OUT)"
	-@sleep 2
	-echo "Hello World" > /dev/CDD2;
	-cat < /dev/CDD2;
	-echo "Hello World 2" > /dev/CDD2;
	-cat < /dev/CDD2;
	-@sync; sync; sync; sleep 1;
	-./CDD2app;

load: prep compile
	-su -c "insmod ./main.ko"
	dmesg | tail
	ls -l /dev/CDD2
	-lsmod | grep main
	-cat /proc/devices | grep CDD

compile:
	@echo "Check VendorID & ProductID in Makefile"
	@lsusb -d $(MYUSBIDString)| egrep --color=ALWAYS "ID $(MYUSBIDString)[ 	]*$(MYUSBString)" 
	@echo "using EP_IN=$(EP_IN) and EP_OUT=$(EP_OUT)"
	KCPPFLAGS="-DUSBVENDORID=0x$(MYUSBVendorID) -DUSBPRODUCTID=0x$(MYUSBProductID) -DEP_IN=$(EP_IN) -DEP_OUT=$(EP_OUT)" \
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
	-@rm -fr *.ko *.o CDD2app .tmp_versions [mM]odule* *.mod.c l2 libUSBexample 3u_201* /tmp/lsusb_d_v
	#-@modprobe usb-storage

mem:
	ls -l /dev | egrep "^c" | egrep "[ 	][ 	]*1," | sort +5.0n -6.0

libUSBexample: libUSBexample.c
	cc libUSBexample.c -lusb -o libUSBexample

l2: l2.c
	-@umount /media/MYLDD
	-modprobe -r usb-storage
	rm -f core* l2
	cc l2.c -lusb -o l2
	-./l2
	-[ -f "core.*" ] &&  { gdb ./l2 `ls -1 core*`; }

examples_xusb: examples_xusb.c
	-@umount /media/MYLDD 
	-modprobe -r usb-storage
	rm -f core* examples_xusb
	cc -o examples_xusb -L/lib /lib/libusb-1.0.so.0 -lusb examples_xusb.c
	-./examples_xusb
	-[ -f "core.*" ] &&  { gdb ./examples_xusb `ls -1 core*`; }
