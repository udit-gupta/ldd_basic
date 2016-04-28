KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := bytestream-example

obj-m := $(OBJ).o

all: prep run clean

prep:
	cp -f /usr/src/linux/samples/kfifo/bytestream-example.c \
			./bytestream-example.c

run: load 

compile:
	more /usr/src/linux/include/linux/kfifo.h
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	-su -c "insmod ./$(OBJ).ko";
	-sleep 1
	-dmesg | tail -40f 

unload: 
	-su -c "rmmod $(OBJ)"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
