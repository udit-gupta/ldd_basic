MFGR := "sandisk"
VENDOR := 0x781
BUS := $(shell lsusb | grep -i $(MFGR) | grep BUS)

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := pen

obj-m := $(OBJ).o

all: run 
	-@make clean check

run:  load penapp
	-./penapp

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
###  
###

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	-if df -Pk | grep /media/EMERALD;  then \
		umount /media/EMERALD; \
		rmmod usb-storage; \
	 fi
	su -c "insmod ./$(OBJ).ko";sleep 1;
	-lsmod | grep pen
	dmesg | tail -10
	-ls -ld /sys/class/usb /sys/class/usb/pen?
	
unload: 
	-su -c "rmmod $(OBJ)"
	-modprobe usb-storage; sync; sync; sync; sleep 2;
#	-mount /media/EMERALD
	dmesg | tail -10

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule* ./penapp

check:
	-lsusb -d $(VENDOR):
	-lsusb -d $(VENDOR): -v | grep EP
