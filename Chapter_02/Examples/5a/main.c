// set tabstop=2 number nohlsearch

// Example# 2.4
//   module_param .. works for 2.6

#define EXPORT_SYMTAB
#include <linux/version.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/moduleparam.h>
#endif

MODULE_AUTHOR("Me"); 	
MODULE_LICENSE("GPL"); 	// kernel isn't tainted .. SUSE noop

// Parameters: how many times we say hello, and some Id to whom.

char *whom = "world";
EXPORT_SYMBOL(whom);
int howmany = 1;
EXPORT_SYMBOL(howmany);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        module_param(whom, charp, 0);
        module_param(howmany, int, 0);
#else
        MODULE_PARM(whom, "s");
        MODULE_PARM(howmany, "i");
#endif



extern int sub_doprintk(int, void *);
extern int sub2_doprintk(void);

static int hello_init(void){
	// module init message
        printk(KERN_ALERT "2470-020:2.4: main initialized!\n");

	// call function in another module.
	sub2_doprintk();
	return 0;
}

static void hello_exit(void){
	// module exit message
        printk(KERN_ALERT "2470-020:2.4: main destroyed!\n");
}

module_init(hello_init);
module_exit(hello_exit);
