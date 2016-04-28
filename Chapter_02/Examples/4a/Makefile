# set tabstop=2 number nohlsearch

OBJ1 := sub
OBJ2 := main
OBJ3 := sub2

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

LDD := /lib/modules/$(shell uname -r)/kernel/drivers/ldd/

obj-m := $(OBJ1).o $(OBJ2).o $(OBJ3).o

all: clean load 
	@make -s clean

all2: loadall clean
	@make -s clean

run:  load
	lsmod | egrep "main|sub"

runall:  loadall unload

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  
###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 
###  
###

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "insmod ./$(OBJ1).ko" 
	su -c "insmod ./$(OBJ2).ko howmany=2 whom=Universe" 
	su -c "insmod ./$(OBJ3).ko" 
	lsmod | grep $(OBJ1)
	cat /proc/kallsyms | egrep "sub.*_do|howmany|whom"
	tail /lib/modules/`uname -r`/modules.dep


loadall: compile
	-su -c "mkdir -p $(LDD); cp main.ko sub.ko sub2.ko $(LDD); depmod -A";
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "modprobe $(OBJ2) howmany=2 whom=Universe" 
	su -c "insmod ./$(OBJ3).ko" 
	-lsmod | egrep "$(OBJ1)|$(OBJ2)|$(OBJ3)"
	tail /lib/modules/`uname -r`/modules.dep

unload: 
	-su -c "rmmod $(OBJ3);";
	-su -c "rmmod $(OBJ2);";
	-su -c "rmmod $(OBJ1);";
	-lsmod | egrep "$(OBJ1)|$(OBJ2)|$(OBJ3)"

unloadall: 
	-su -c "modprobe -r $(OBJ3);";
	-lsmod | egrep "$(OBJ1)|$(OBJ2)|$(OBJ3)"

clean: unload
	-@su -c "rm -f $(LDD)/*; [ -d $(LDD) ] && { rmdir $(LDD); };"
	-@rm -fr $(OBJ1).*o $(OBJ2).*o $(OBJ3).*o $(OBJ1).*.* $(OBJ3).*.* .$(OBJ1).* $(OBJ2).*.* .$(OBJ2).* .$(OBJ3).* .tmp_versions* [mM]odule*
