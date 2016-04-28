// #set tabstop=2 number nohlsearch
// Example# 2.2 .. Simple "Hello World!" module example 

// Change Synopsis 	(top down)
//	macro LINUX_VERSION_CODE
//	macro KERNEL_VERSION(x,y,z)
//	macro	UTS_RELEASE
//
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <generated/utsrelease.h>
#else
#include <linux/utsrelease.h>
#endif


MODULE_AUTHOR("Me"); 
MODULE_LICENSE("GPL"); 	/* kernel isn't tainted .. SUSE noop */

static int hello_init(void){
	int ver = LINUX_VERSION_CODE;
	char *ver2 = UTS_RELEASE;

	printk(KERN_ALERT "Hello, world\n");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	printk(KERN_ALERT "   Kernel Version \"%s\", internally .. \"%d\"\n",ver2, ver);
#else
	printk(KERN_ALERT "   2.4 Kernel Version \"%s\", internally .. \"%d\"\n",ver2, ver);
#endif

	return 0;
}

static void hello_exit(void){
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
