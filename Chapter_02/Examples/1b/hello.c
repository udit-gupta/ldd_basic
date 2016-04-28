// set tabstop=2 number nohlsearch

// Example# 2.1 .. Simple "Hello World!" module example 

// synopsis:
// 	init:
// 	exit:
// 	printk()

/* #define MODULE // no longer necessary as 
									// kernel build system automatically defines it */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h> // for current macro

MODULE_AUTHOR("Me"); 
MODULE_LICENSE("GPL"); 	/* kernel isn't tainted .. SUSE noop */

int hello_init(void){
	printk(KERN_ALERT "Hello, world (PID=%d)\n", current->pid);
	return 0;
}

void hello_exit(void){
	printk(KERN_ALERT "Goodbye, cruel world (PID=%d)\n",current->pid);
}

module_init(hello_init);
module_exit(hello_exit);
