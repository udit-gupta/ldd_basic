# BUS := $(shell lsusb | grep Delcom | sed -e 's/Device.*//' -e 's/Bus//' -e 's/00//' -e 's/ //g')
BUS := $(shell lsusb -d 0xfc5: | grep Delcom | sed -e 's/Device.*//' -e 's/Bus//' -e     's/00//' -e 's/ //g')

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := timingLED

obj-m := $(OBJ).o

all: usbmon red yellow green clean

run:  load
	### debug note; 
	### 1. recompile Kernel with CONFIG_USB_HID=m
	### 2. udevadm monitor - comment rule in line#765 for 
	###  						/lib/udev/rules.d/60-libmtp.rules 
	### 3. lsusb -t | grep HID
	### 4. ls -l /sys/devices/pci0000:00/0000:00:1d.1/usb3/
  ### 5. lsusb | grep Delcom
	-@su -c "rmmod usbhid"
	-lsusb -t | grep timing
	tree /sys/bus/usb/drivers/${OBJ}driver/

	# cd /sys/bus/usb/drivers/timingLED/3-1:1.0

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
###  
###

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	-su -c "insmod ./$(OBJ).ko";
	sleep 1


## http://biot.com/blog/usb-sniffing-on-linux
## yum -i install wireshark
## yum -i install wireshark-gnome

usbmon:
	-mount -t debugfs none_debugs /sys/kernel/debug
	-modprobe usbmon

clean: unload
	-umount /sys/kernel/debug
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*


prep: load usbmon 
	-rmmod usbhid; 
	sync; sync; sync;
	lsusb -t | grep HID; 
	echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/red; 
	echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/yellow; 
	echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/green; 
	# echo "0" > /sys/bus/usb/drivers/timingLEDdriver/$(shell lsusb | grep Delcom | sed -e 's/Device.*//' -e 's/Bus//' -e 's/00//' -e 's/ //g')-1:1.0/yellow; 
	sync; sync; sync;
	sleep 2;
	#

red: prep
	echo "1" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/red; 
	sleep 5
	echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/red; 

yellow: prep
	echo "1" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/yellow; 
	sleep 5
	echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/yellow; 

green: prep
	echo "1" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/green; 
	sleep 5
	echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/green; 

unload: 
	-echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/red; 
	-echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/yellow; 
	-echo "0" > /sys/bus/usb/drivers/timingLEDdriver/${BUS}-1:1.0/green; 
	-su -c "rmmod $(OBJ)"
	-su -c "modprobe -r usbmon"
