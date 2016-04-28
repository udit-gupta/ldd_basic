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
extern int sub_doprintk(int howmany, void * whom);

// function .. exported to kernel
int sub_doprintk(int howmany, void * whom){
	int i;
	for (i = 0; i < howmany; i++)
		printk(KERN_ALERT "(%d) Hello, %s\n", i, (char *)whom);
	return 0;
}
