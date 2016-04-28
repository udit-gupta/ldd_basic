# set tabstop=2 number nohlsearch

OBJ1 := sub
OBJ2 := main

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

LDD := /lib/modules/$(shell uname -r)/kernel/drivers/ldd/

obj-m := $(OBJ1).o $(OBJ2).o

all: load 
	@make -s clean

all2: load unload loadall 
	@make -s clean

run:  load unload

runall:  loadall unload

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  Alternative, archaic syntax
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "insmod ./$(OBJ1).ko" 
	su -c "insmod ./$(OBJ2).ko howmany=2 whom=Universe" 
	lsmod | egrep "$(OBJ1)|$(OBJ2)"
	cat /proc/kallsyms | grep sub_do
	tail /lib/modules/`uname -r`/modules.dep

loadall: compile
	-su -c "mkdir -p $(LDD); cp main.ko sub.ko $(LDD); depmod -A;";
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "modprobe $(OBJ2) howmany=2 whom=Universe" 
	lsmod | egrep "$(OBJ1)|$(OBJ2)"
	tail /lib/modules/`uname -r`/modules.dep

unload: 
	-su -c "rmmod $(OBJ2);";
	-su -c "rmmod $(OBJ1);";

unloadall: 
	-su -c "modprobe -r $(OBJ2);";
	-(sleep 2; depmod -a; sync; sync;) &
	-lsmod | egrep "sub|main"
	-tail /lib/modules/`uname -r`/modules.dep

clean: unload unloadall
	-@su -c "rm -f $(LDD)/*; [ -d $(LDD) ] && { rmdir $(LDD); };"
	-@rm -fr $(OBJ1).*o $(OBJ2).*o $(OBJ1).*.* .$(OBJ1).* $(OBJ2).*.* .$(OBJ2).* .tmp_versions* [mM]odule*
