// set tabstop=2 number nohlsearch

// Example# 2.4
//   module_param .. works for 2.6

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

extern int sub_doprintk(int, void *);
extern int howmany;
extern char *whom;

static int sub2_init(void){
	// module init message
        printk(KERN_ALERT "2470:2.4: sub2 initialized!\n");

	// call function in another module.
	sub_doprintk(howmany,whom);
	return 0;
}

static void sub2_exit(void){
	// module exit message
        printk(KERN_ALERT "2470:2.4: sub2 destroyed!\n");
}

module_init(sub2_init);
module_exit(sub2_exit);
