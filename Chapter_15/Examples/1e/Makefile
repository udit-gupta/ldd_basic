# Adapted from http://www.scs.ch/~frey/linux/mmap.example.tar
# set your module name here
# provide the following files
#
# MODULENAME.c	the driver
# MODULENAME.h  the include file
# MODULENAME_test.c	the driver test program
# MODULENAME_test	the driver test script
#
MODULENAME 	:= mmap
### should not need to change stuff below ######################


KDIR		:= /lib/modules/$(shell uname -r)/build
#KDIR		:= /exports/linux-2.6.12.2/
PWD		:= $(shell pwd)

CC		:= gcc -Wall

all: test clean

ifeq ($(KERNELRELEASE),)
compile:	$(MODULENAME)_test
compile:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
else
  obj-m	:= $(MODULENAME).o
endif

$(MODULENAME)_test: $(MODULENAME)_test.c
	$(CC) -g -o $(MODULENAME)_test $(MODULENAME)_test.c


load: compile
	-insmod ./$(MODULENAME).ko

unload:
	-rmmod $(MODULENAME)

run: load
	-su -c "mknod -m 666 /dev/node c $(shell grep mmap /proc/devices | sed 's/mmap//') 0";

test: run
	./$(MODULENAME)_test

clean: unload
	-@rm -fr *.o *.ko Modules.symvers *.mod.c .*.cmd $(MODULENAME)_test /dev/node  [mM]odule* .tmp*

