# set tabstop=2 number nohlsearch

OBJ1 := sub
OBJ2 := main
OBJ3 := sub2

LDDM := ch25

#  create one .o .. from multiple .o files
#  .. do not need the .ko rule 
#  .. the kernel will auto create .ko from the obj-m line below.
obj-m := $(LDDM).o
$(LDDM)-objs := $(OBJ1).o $(OBJ2).o $(OBJ3).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

LDD := /lib/modules/$(shell uname -r)/kernel/drivers/ldd/

all: load
	@make -s clean

run:  load

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
	# su -c "insmod ./$(LDDM).ko" 
	su -c "insmod ./$(LDDM).ko whom=Universe" 
	nm ./$(LDDM).ko
	cat /proc/kallsyms | egrep "sub.*_do|howmany|whom"

loadall: compile
	-su -c "mkdir -p $(LDD); cp $(LDDM).ko $(LDD); depmod -A";
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "modprobe $(LDDM) howmany=2 whom=Universe" 

unload: 
	-su -c "rmmod $(LDDM);";

unloadall: 
	-su -c "modprobe -r $(LDDM);";
	-@su -c "rm -f $(LDD)/*; rmdir $(LDD);"

clean: unload
	-@rm -fr $(LDDM).* .$(LDDM).* $(OBJ1).*o $(OBJ2).*o $(OBJ3).*o $(OBJ1).*.* .$(OBJ1).* $(OBJ2).*.* $(OBJ2).*.* .$(OBJ2).* .$(OBJ3).* .tmp_versions* [mM]odule*
