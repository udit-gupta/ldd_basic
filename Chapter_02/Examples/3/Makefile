# set tabstop=2 number nohlsearch

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

all: load clean

run:  load unload

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
###  
###

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	# su -c "insmod ./$(OBJ).ko"
	# su -c "insmod ./$(OBJ).ko howmany=2 whom=World"
	# su -c "insmod ./$(OBJ).ko howmany=3 whom=Globe"
	# su -c "insmod ./$(OBJ).ko whom=Universe howmany=2"
	su -c "insmod ./$(OBJ).ko who=Globe"
	# su -c "insmod ./$(OBJ).ko whom=Globe"
	# su -c "insmod ./$(OBJ).ko howmany=4 whom=Galaxy"
	-cat /proc/kallsyms | grep '\[main\]' 

unload: 
	-su -c "rmmod $(OBJ)"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
