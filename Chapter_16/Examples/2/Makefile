all: run clean

run:  
	-mkdir /tmp/ramdisk 			# create a mount point:
	-mke2fs /dev/ram0 				# mount the ramdisk:
	-mount /dev/ram0 /tmp/ramdisk	# create a filesystem:
	df -k /tmp/ramdisk
	ls -l /tmp/ramdisk
	cp /etc/passwd /tmp/ramdisk
	cp /etc/hosts /tmp/ramdisk
	ls -l /tmp/ramdisk

clean: 
	-umount /dev/ram0
	-rm -fr /tmp/ramdisk
