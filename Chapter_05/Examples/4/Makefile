##
## NOTE: these are samples from /usr/src/linux/samples/kfifo/*.c
##
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

OBJ := kfifo

obj-m := bytestream-example.o dma-example.o inttype-example.o record-example.o

# test: compile 

all: run clean

run: cp load test-int
	
cp:
	[ -d "/usr/src/linux/samples/kfifo/" ] && { cp /usr/src/linux/samples/kfifo/*.c .; }

test-int: load writer-int reader-int
	sleep 40

writer-int:
	echo $i
	(for i in 1 2 3 4 5; do \
		echo "$${i}" | tee /proc/int-fifo; \
		sleep 2;\
	done) &
	sleep 1;   # to add to race condition for below

reader-int:
	sleep 10;
	(for i in 1 2 3 4 5; do \
		cat < /proc/int-fifo; \
		sleep 3;\
	done) &
	wait

compile:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules 

load:	compile
	@echo "try \"tail -f /var/log/messages\" in another window(as root) ..";
	- for i in bytestream-example dma-example inttype-example record-example; do \
			su -c "insmod ./$${i}.ko";\
	done
	-lsmod | grep example

unload: 
	- for i in bytestream-example dma-example inttype-example record-example; do \
			su -c "rmmod ./$${i}"; \
done
	-lsmod | grep example

clean: unload
	-@rm -f *.o *.ko *.*.* .[a-zA-Z0-9]*.* .tmp_versions/* [mM]odule*
	-@rmdir .tmp_versions*
