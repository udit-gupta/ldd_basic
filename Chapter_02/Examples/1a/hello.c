// set tabstop=2 number nohlsearch
// Example# 2.1 .. Simple "Hello World!" module example 

// Change Synopsis:
// 	type modifiers
// 	__init
// 	__exit

/* #define MODULE // .. no longer necessary as 
								  // kernel build system automatically defines it */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Me"); 
MODULE_LICENSE("GPL"); 	/* kernel isn't tainted .. SUSE noop */

static int __init hello_init(void){
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}

static void __exit hello_exit(void){
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
