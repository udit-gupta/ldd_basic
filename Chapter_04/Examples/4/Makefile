KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

all: clean run
	@make -s clean

run:  clean load 
	dd if=/proc/sequence count=1
	## dd if=/proc/sequence skip=2 count=1

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

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
