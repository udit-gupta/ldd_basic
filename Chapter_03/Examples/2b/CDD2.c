// Example# 3.2b:  Simple Char Driver with Statically allocated Major#  
//                works only in 2.6  (Exactly Similar .. to Example# 3.1)

//      using dev_t, struct cdev (2.6)	(example 3.1b)
//	using myCDD structure		(here..)

#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <generated/utsrelease.h>
#else
#include <linux/utsrelease.h>
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");   	//  Kernel isn't tainted .. but doesn't 
			 	//  it doesn't matter for SUSE anyways :-(  

#define CDD		"CDD2"

#define CDDMAJOR	34
#define CDDMINOR	0	// 2.6
#define CDDNUMDEVS	1	// 2.6

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
char CDD_buf[4096];
#endif

struct CDDdev_struct {
  unsigned int counter;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	char *CDD_storage;
#else
	char CDD_storage[4096];
#endif
	struct cdev cdev;
	dev_t	devno;
};


static struct CDDdev_struct thisCDD[1];

static int CDD_open (struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	thisCDD->CDD_storage=CDD_buf;
#endif

	// MOD_INC_USE_COUNT;
	return 0;
}

static int CDD_release (struct inode *inode, struct file *file)
{
	// MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t CDD_read (struct file *file, char *buf, 
size_t count, loff_t *ppos)
{
	int len, err;
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
	dev_t devno;

	//  Step 1a of 2:  create/populate device numbers
	devno = MKDEV(CDDMAJOR, CDDMINOR);

	//  Step 1b of 2:  request/reserve Major Number from Kernel
	i = register_chrdev_region(devno,CDDNUMDEVS,CDD);
	if (i < 0) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i;}

	//  Step 2a of 2:  initialize thisCDD->cdev struct
	cdev_init(&(thisCDD->cdev), &CDD_fops);

	//  Maintain state "local" to driver
        thisCDD->cdev.owner = THIS_MODULE;
        thisCDD->cdev.ops = &CDD_fops;
        thisCDD->devno = devno;

	//  Step 2b of 2:  register device with kernel
        i = cdev_add(&(thisCDD->cdev), devno, CDDNUMDEVS);
        if (i) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i; }

	printk(KERN_ALERT "Char Dev Initialized. Major#=%d", CDDMAJOR);

	return 0;
}

static void CDD_exit(void)
{
	// try with and without new-line => buffer flush convention.
	// printk(KERN_ALERT "Char Dev Uninitialized. Major#=%d", CDDMAJOR);
	printk(KERN_ALERT "Char Dev Uninitialized. Major#=%d\n", CDDMAJOR);
 
	//  Step 1 of 2:  unregister device with kernel
	cdev_del(&(thisCDD->cdev));

	//  Step 2 of 2:  Release request/reserve of Major Number from Kernel
	unregister_chrdev_region(thisCDD->devno, CDDNUMDEVS);

}

module_init(CDD_init);
module_exit(CDD_exit);

