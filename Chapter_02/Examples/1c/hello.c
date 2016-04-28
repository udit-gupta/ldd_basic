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

//EXPORT_SYMTAB;

MODULE_AUTHOR("Me"); 
MODULE_LICENSE("GPL"); 	/* kernel isn't tainted .. SUSE noop */

static int hello_init(void){
	printk(KERN_ALERT "Hello, world (PID=%d)\n", current->pid);
	return 0;
}

static void hello_exit(void){
	printk(KERN_ALERT "Goodbye, cruel world (PID=%d)\n",current->pid);
}

EXPORT_SYMBOL(hello_init);

module_init(hello_init);
module_exit(hello_exit);
