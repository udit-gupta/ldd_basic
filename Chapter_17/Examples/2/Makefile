KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := LO

obj-m := $(OBJ).o

all: run check

run:  load
	lsmod | grep $(OBJ)
	ifconfig ${OBJ} 192.168.0.1 up
	ifconfig 
	

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
###  
###

load:	clean compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "insmod ./$(OBJ).ko"

unload: 
	-su -c "rmmod $(OBJ)"

clean: unload 
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*

check:
	-ping 192.168.0.1 -c 2 -i 1
