KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CDDparm := $(CDDparm)
CDDparm := 0

obj-m := CDD2.o

all: clean run 	
	CDDparm=34 make clean run;
	-ls -l /dev/CDD2*
	@make clean
	# @make --no-print-directory clean
	# -ls -l /dev/CDD2*

run: load CDD2app
	# @ [ -c /dev/CDD2 ] && { echo "Hello World" > /dev/CDD2;};
	# @ [ -c /dev/CDD2 ] && { cat < /dev/CDD2; };
	# @ [ -c /dev/CDD2 ] && { echo "Hello World" > /dev/CDD2;};
	# @ [ -c /dev/CDD2 ] && { echo "Hello World" > /dev/CDD2;};
	# @ [ -c /dev/CDD2 ] && { cat < /dev/CDD2; };
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	./CDD2app;

load: CDD2.o 
	-su -c "{ insmod ./CDD2.ko CDDparm=$(CDDparm);} || \
		{ echo CDDparm is not set;} ";

CDD2.o:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

###
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
###

CDD2app:
	-gcc -o CDD2app CDD2app.c;

unload:
	-su -c "rmmod CDD2; rm -f /dev/CDD2;"

clean: unload
	-@rm -fr *.o CDD2*.o CDD2*.ko .CDD2*.* CDD2*.*.* CDD2app .tmp_versions [mM]odule*

