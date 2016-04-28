MESSAGES := "/var/log/messages"
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

all: run clean

run:  load 
	-tail -f ${MESSAGES} &
	-cat /proc/interrupts | grep MYDEV
	cat /proc/MYDEV/hello
	echo 123 > /proc/MYDEV/hello
	(sleep 10; cat /proc/MYDEV/hello)
	#
	# test .. cancelling delayed work
	echo 123 > /proc/MYDEV/hello
	cat /proc/MYDEV/hello
	#
	cat /proc/MYDEV/hello

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

unload: 
	-su -c "rmmod $(OBJ)"
	-cat /proc/interrupts | grep MYDEV
	-pkill -o -f "tail -f ${MESSAGES}"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
