KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := main

obj-m := $(OBJ).o

# test: compile 

all: run clean

run: test1 test2

test1:load
	echo 123 > /proc/MYDEV/hello
	cat /proc/MYDEV/hello;

test2: load 
	echo $$i
	(for i in 1 2 3 4 5; do \
		echo "Test String .. $${i}" | tee /proc/MYDEV/hello;\
		sleep 1;\
	done) &
	sleep 1;   # to add to race condition for below
	(for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14; do \
		cat < /proc/MYDEV/hello;\
		sleep 1;\
	done) 
	sleep 1;
	wait

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

###  Alternatively, you may want to use the early 2.6 syntax of
###  $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules 

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	-su -c "insmod ./$(OBJ).ko";

unload: 
	-su -c "rmmod $(OBJ)"

clean: unload
	-@rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* [mM]odule*
