MESSAGES := "/var/log/messages"

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

all: run clean

run:  load 
	-tail -f ${MESSAGES} &
	cat /proc/MYDEV/hello &
	cat /proc/MYDEV/hello &
	sleep 5
	echo 123 > /proc/MYDEV/hello
	cat /proc/MYDEV/hello &
	echo 123 > /proc/MYDEV/hello
	cat /proc/MYDEV/hello &
	echo 123 > /proc/MYDEV/hello
	echo 123 > /proc/MYDEV/hello
	#@(sleep 15; echo -n "remember run \"make clean\" for Ch10-5a"; ) &

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

finwait: 
	-while ps -f | egrep "cat /proc/MYDEV/hello" > /dev/null 2>&1; \
	do \
		echo 123 > /proc/MYDEV/hello; \
		sleep 1; \
		pkill -o -f "cat /proc/MYDEV/hello"; \
	done
	-pkill -o -f "tail -f ${MESSAGES}";

unload: finwait
	-su -c "rmmod $(OBJ)"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
