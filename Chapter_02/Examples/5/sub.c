// set tabstop=2 number nohlsearch

// Example# 2.4  .. 

// #define EXPORT_NO_SYMBOLS
#define EXPORT_SYMBOLS
#define EXPORT_SYMTAB

#include <linux/module.h>
#include <linux/kernel.h>
int sub_doprintk(int howmany, void * whom);

// MODULE_AUTHOR("Me"); 	
// MODULE_LICENSE("GPL"); 	// kernel isn't tainted .. SUSE noop

// function .. 
int sub_doprintk(int howmany, void * whom){
	int i;
	for (i = 0; i < howmany; i++)
		printk(KERN_ALERT "(%d) Hello, %s\n", i, (char *)whom);
	return 0;
}
