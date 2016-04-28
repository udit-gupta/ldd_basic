VENDOR := 0x0781
MFGR := "sandisk"
BUS := $(shell lsusb | grep -i $(MFGR) | grep BUS)

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := pen_info

obj-m := $(OBJ).o

all: clean load
#	-@make clean

run:  load unload

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
	@make check
	
unload: 
	-su -c "rmmod $(OBJ)"
	-modprobe usb-storage; sync; sync; sync; sleep 2;
#	-mount /media/EMERALD
	dmesg | tail -10

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*

check:
	-lsusb -d $(VENDOR): -v | grep EP
