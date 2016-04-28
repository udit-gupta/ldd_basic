// set tabstop=2 number nohlsearch
// Example# 2.3 .. Simple Hello World example .. with arguments

// Change Synopsis (top down)
// 	moduleparam.h
//  API module_param(var,datatype,sysfs perms)
#include <linux/version.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <generated/utsrelease.h>
#else
#include <linux/utsrelease.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,99)
#include <linux/moduleparam.h>
#endif

MODULE_AUTHOR("Me"); 	
MODULE_LICENSE("GPL"); 	// kernel isn't tainted .. SUSE noop

// Parameters: how many times we say hello, and some Id to whom.
static char *whom = "hello world";
// static char *whom;
static int howmany=1;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,99)
	module_param(whom, charp, 0);
	module_param(howmany, int, 0);
#else
	MODULE_PARM(whom, "s");
	MODULE_PARM(howmany, "i");
#endif

static int hello_init(void){
	int i;
	for (i = 0; i < howmany; i++)
		printk(KERN_ALERT "(%d) Hello, %s\n", i, whom);
	return 0;
}

static void hello_exit(void){
	printk(KERN_ALERT "Goodbye, cruel %s\n", whom);
}

module_init(hello_init);
module_exit(hello_exit);
