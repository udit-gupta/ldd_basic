# set tabstop=2 number nohlsearch

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := hello

obj-m := $(OBJ).o

all: load clean

run:  load
	lsmod | head
	cat /proc/kallsyms | grep hello

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  Alternative, antiquated syntax
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	su -c "insmod ./$(OBJ).ko"
	nm ./$(OBJ).ko
	lsmod | head
	cat /proc/kallsyms | grep hello

unload: 
	-su -c "rmmod $(OBJ)"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
