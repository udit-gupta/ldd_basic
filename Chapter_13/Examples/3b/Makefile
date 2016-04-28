##  do a 
## 	    yum -y install libhid-devel 
##  before running this code.

##
##  see http://www.delcomproducts.com/downloads/control_delcom_gen2.zip
##

all: run clean

run: control_delcom_gen2 green red blue
	# -./control_delcom_gen2 --get

green: control_delcom_gen2
	-@./control_delcom_gen2 --green
	@sleep 2

red: control_delcom_gen2
	-@strace -o strc.out ./control_delcom_gen2 --red
	@sleep 3

blue: control_delcom_gen2
	-@./control_delcom_gen2 --blue
	@sleep 4

control_delcom_gen2: control_delcom_gen2.c
	gcc -I/usr/include -o control_delcom_gen2 -lhid control_delcom_gen2.c

## http://biot.com/blog/usb-sniffing-on-linux
## yum -i install wireshark
## yum -i install wireshark-gnome

trace: usbmon 

usbmon:
	-mount -t debugfs none_debugs /sys/kernel/debug
	modprobe usbmon
	./diffusb.sh
	# cat /sys/kernel/debug/usb/usbmon/1u > usbmon.trc

traceclean trace-clean: usbmon-clean

usbmon-clean:
	-rmmod usbmon
	-modprobe -r usbmon
	umount /sys/kernel/debug
	rm -f *.out *.trc

off: control_delcom_gen2
	-./control_delcom_gen2 --off

clean: off
	-@rm -f control_delcom_gen2 strc.out *.out *.trc
	
