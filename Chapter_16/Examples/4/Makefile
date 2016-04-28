#  sourced from http://www.linuxforu.com/2012/02/device-drivers-disk-on-ram-block-drivers/
# If called directly from the command line, invoke the kernel build system.
ifeq ($(KERNELRELEASE),)

	KERNEL_SOURCE := /usr/src/linux
	PWD := $(shell pwd)
default: clean 
	@make load | less

load: module
	@echo "see http://www.linuxforu.com/2012/02/device-drivers-disk-on-ram-block-drivers/"
	-insmod ./dor.ko
	-ls /dev/rb*
	#### step #3: Read the entire disk (/dev/rb)
	-dd if=/dev/rb of=/dev/null bs=8k
	#### step #4: Zero out the first sector of /dev/rb1
	-dd if=/dev/zero of=/dev/rb1 count=1
	#### step #5: write some text to /dev/rb1
	-echo "Hello World" > /dev/rb1
	#### step #6: read from /dev/rb1
	-xxd /dev/rb1 | head -4
	-od -bc /dev/rb1 | head -6
	#### step #7: display partition info for fdisk
	-fdisk -l /dev/rb
	#### step #8: mkfs.vfat /dev/rb3
	-mkfs.vfat /dev/rb3
	#### step #9: mount /dev/rb3
	-mkdir /tmp/DOR
	-mount -t vfat /dev/rb3 /tmp/DOR
	-df -Pk /tmp/DOR

module:
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) modules

unload:
	-umount /tmp/DOR
	-rmmod dor
	-rm -fr /tmp/DOR

clean: unload
	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) clean

# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
else

	obj-m := dor.o
	dor-y := ram_block.o ram_device.o partition.o

endif
