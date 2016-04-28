// Example# 3.6a: Simple Char Driver with Major# as an passed-in value to insmod
//		  Handles both dynamic and static allocation of Major Numbers
//  		  works for both 2.4 and 2.6

//      using myCDD structure                   (example 3.2a)
//      using file->private_data structure      (example 3.3a)
//      using static/dynamic major#             (example 3.5a)
//      using static/dynamic major#; with parm  (here)

#include <linux/version.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/moduleparam.h>
#endif

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-(
			 */

#define MYMAJOR 0
#define MYDEV 	"CDD"

static unsigned int CDDmajor = 0;
static int CDDparm = 32;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	module_param(CDDparm,int,0);		// 2.6
#else
	MODULE_PARM(CDDparm,"i");		// 2.4
#endif

struct CDDdev_struct {
        unsigned int counter;
        char CDD_storage [132];
};

static struct CDDdev_struct myCDD;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int CDD_open (struct inode *inode, struct file *file)
#else
static int CDD_open (struct file *file)
#endif
{
        static struct CDDdev_struct *thisCDD=&myCDD;

        file->private_data=thisCDD;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int CDD_release(struct inode *inode, struct file *file)
#else
static int CDD_release(struct file *file)
#endif
{
	// MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t CDD_read (struct file *file, char *buf, 
size_t count, loff_t *ppos)
{
	int len, err;
        struct CDDdev_struct *thisCDD=file->private_data;

	if( thisCDD->counter <= 0 ) return 0;

	err = copy_to_user(buf,thisCDD->CDD_storage,thisCDD->counter);

	if (err != 0) return -EFAULT;

	len = thisCDD->counter;
	thisCDD->counter = 0;
	return len;
}

static ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
{
	int err;
        struct CDDdev_struct *thisCDD=file->private_data;

 	err = copy_from_user(thisCDD->CDD_storage,buf,count);
	if (err != 0) return -EFAULT;

	thisCDD->counter += count;
	return count;
}

static struct file_operations CDD_fops =
{
	// for LINUX_VERSION_CODE 2.4.0 and later 
	owner:	THIS_MODULE, 	// struct module *owner
	open:	CDD_open, 	// open method 
	read:   CDD_read,	// read method 
	write:  CDD_write, 	// write method 
	release:  CDD_release 	// release method
};

static int CDD_init(void)
{
	int i;

	// register_chrdev returns 0 when CDDparm !=0, and vice versa ..
	i = register_chrdev(CDDparm, MYDEV, &CDD_fops);
	if (i < 0) return i;
	
	CDDmajor = (i ? i:CDDparm);
	// if (i) 
	   printk(KERN_ALERT "kernel assigned major#: %d to CDD\n", CDDmajor);

	return 0;
}

static void CDD_exit(void)
{
	unregister_chrdev (CDDmajor, MYDEV);
}

module_init(CDD_init);
module_exit(CDD_exit);

