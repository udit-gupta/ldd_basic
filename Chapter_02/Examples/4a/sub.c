// set tabstop=2 number nohlsearch

// Example# 2.4  .. 

// #define EXPORT_SYMTAB
#define WHOM	"sub!"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Me"); 	
MODULE_LICENSE("GPL"); 	// kernel isn't tainted .. SUSE noop

// extern declaration of sub_doprintk() for use in EXPORT_SYMBOL
static int sub_doprintk(int howmany, void * whom);
EXPORT_SYMBOL(sub_doprintk);

// function .. exported to kernel
static int sub_doprintk(int howmany, void * whom){
	int i;
	for (i = 0; i < howmany; i++)
		printk(KERN_ALERT "(%d) Hello, %s\n", i, (char *)whom);
	return 0;
}

// init module:sub
static int sub_init(void){
	printk(KERN_ALERT "2470-020:2.4: sub initialized!\n");
	sub_doprintk(1, WHOM);
	return 0;
}

// exit module:sub
static void sub_exit(void){
	printk(KERN_ALERT "2470-020:2.4: sub destroyed!\n");
}

module_init(sub_init);
module_exit(sub_exit);
