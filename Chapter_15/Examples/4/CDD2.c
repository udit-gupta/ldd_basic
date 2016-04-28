// Example# 3.3b:  Simple Char Driver with Statically allocated Major#  
//                works only in 2.6  (Exactly Similar .. to Example# 3.1)

//      using dev_t, struct cdev (2.6)		(example 3.1b)
//	using myCDD structure			(example 3.2b)
//	using file->private_data structure	(here)

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

#define CDDMAJOR	32
#define CDDMINOR	0	// 2.6
#define CDDNUMDEVS	1	// 2.6

dev_t	firstdevno;
struct CDDdev_struct {
        unsigned int counter;
        char CDD_storage [132];
	struct cdev cdev;
};

static struct CDDdev_struct myCDD;

static int CDD_open (struct inode *inode, struct file *file)
{
	struct CDDdev_struct *thisCDD;
	thisCDD=&myCDD;

	file->private_data=thisCDD;
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
	struct CDDdev_struct *thisCDD=&myCDD;

	//  Step 1a of 2:  create/populate device numbers
	firstdevno = MKDEV(CDDMAJOR, CDDMINOR);

	//  Step 1b of 2:  request/reserve Major Number from Kernel
	i = register_chrdev_region(firstdevno,1,CDD);
	if (i < 0) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i;}

	//  Step 2a of 2:  initialize thisCDD->cdev struct
	cdev_init(&(thisCDD->cdev), &CDD_fops);

	//  Maintain state "local" to driver
        thisCDD->cdev.owner = THIS_MODULE;
        thisCDD->cdev.ops = &CDD_fops;

	//  Step 2b of 2:  register device with kernel
        i = cdev_add(&(thisCDD->cdev), firstdevno, CDDNUMDEVS);
        if (i) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i; }

	//	sys_mknod(CDD,0644,firstdevno);

	return 0;
}

static void CDD_exit(void)
{
	struct CDDdev_struct *thisCDD=&myCDD;
	
	//  Step 1 of 2:  unregister device with kernel
	cdev_del(&(thisCDD->cdev));

	//  Step 2 of 2:  Release request/reserve of Major Number from Kernel
	unregister_chrdev_region(firstdevno, CDDNUMDEVS);
}

module_init(CDD_init);
module_exit(CDD_exit);

