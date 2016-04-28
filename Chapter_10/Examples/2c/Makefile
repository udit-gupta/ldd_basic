MESSAGES := "/var/log/messages"

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

all: 
	@echo "- Another example for Debug - timing issue/condition"
	@echo "make test - to \"test\" and debug - if needed"

test: run clean

run:  load  
	-tail -f ${MESSAGES} &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	echo 123 > /proc/MYDEV/hello
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	sleep 5
	echo 123 > /proc/MYDEV/hello
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	echo 123 > /proc/MYDEV/hello

	##
	##	Test timeout .. i.e. without writer
	##
run2:  load 
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &

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
	-pkill -o -f "tail -f ${MESSAGES}"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
