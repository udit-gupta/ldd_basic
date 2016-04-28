MESSAGES := "/var/log/messages"
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CDDparm := $(CDDparm)
CDDparm := 0

PROCDIR := CDD

obj-m := CDD2.o

all: run 
	(cat /proc/interrupts | grep -i mouse; \
  echo "sleeping 5"; \
	sleep 5; \
	cat /proc/interrupts | grep -i mouse; \
  echo "sleeping 10"; \
	sleep 10; make clean;)
	

run: CDD2 CDD2app
	-tail -f ${MESSAGES} &
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	echo "Hello World" > /dev/CDD2;
	cat < /dev/CDD2;
	./CDD2app;
	cat < /proc/$(PROCDIR)/hello;
	echo "1234" > /proc/$(PROCDIR)/hello;
	cat < /proc/$(PROCDIR)/hello;
	cat < /proc/$(PROCDIR)/hello;

load: CDD2.o 
	-su -c "insmod ./CDD2.ko CDDparm=$(CDDparm);";

CDD2: load 
	-su -c "mknod -m 666 /dev/CDD2 c $(shell grep CDD2 /proc/devices | sed 's/CDD2//') 0;"


compile CDD2.o:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

###
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
###

CDD2app:
	-gcc -o CDD2app CDD2app.c;

unload:
	-su -c "rmmod CDD2;"
	-su -c "rm -f /dev/CDD2;"
	-pkill -o -f "tail -f ${MESSAGES}"

clean: unload
	-@rm -fr *.o CDD2*.o CDD2*.ko .CDD2*.* CDD2*.*.* CDD2app .tmp_versions [mM]odule*

