MESSAGES := "/var/log/messages"
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

# test: compile

all: run clean

run: load 
	-tail -f ${MESSAGES} &
	-cat /proc/interrupts | grep MY
	echo 123 > /proc/MYDEV/hello
	sleep 2
	-@echo "#####"
	-@echo "what happens when the \"echo\" below is commented out ?"
	-@echo "   .. see the messages in /var/log/messages"
	-@echo "if the window appears hung use "
	-@echo "   .. in another window \"echo 1234 > /proc/MYDEV/hello\""
	-@echo "#####"
	echo 1234 > /proc/MYDEV/hello
	sleep 2
	-cat /proc/interrupts | grep MY
	cat /proc/MYDEV/hello
	sleep 2
	-@echo "#####"
	-@echo "notice the artifact of read() system call as part of cat"
	-@echo " .. there will be a second read() - till read() returns 0"
	-@echo "#####"
	-cat /proc/interrupts | grep MY

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
