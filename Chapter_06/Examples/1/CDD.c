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
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-(  
			     
			     they have a concept of vendor-tainted ..
                             meaning that no one can call them ... 
			 */

#define CMD0 0
#define CMD1 1

static unsigned int counter = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	static char *string;
#else
	static char string [132];
#endif
static int data;

static int CDD_open (struct inode *inode, struct file *file)
{
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
	if( counter <= 0 ) return 0;

	err = copy_to_user(buf,string,counter);

	if (err != 0) return -EFAULT;

	len = counter;
	counter = 0;
	return len;
}

static ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
{
	int err;
	err = copy_from_user(string,buf,count);
	if (err != 0) return -EFAULT;

	counter += count;
	return count;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
static long CDD_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
#else
static int CDD_ioctl(struct inode *inode, struct file *file,
unsigned int cmd, unsigned long arg)
#endif
{
	int retval = 0;

	switch ( cmd ) {
		case CMD0: // for reading data from arg into userspace
			if (!access_ok(VERIFY_READ, (void __user *) arg, sizeof(int)))
				return -EFAULT;

			// use copy_to_user for pointer references
			if (copy_to_user((int *) arg, &data, sizeof(int)))
				  return -EFAULT;
			printk("r. data==%ld\n",(unsigned long) data);
			retval=data;
			break;
		case CMD1: // for "writing" data to arg from userspace
			// if (!access_ok(VERIFY_WRITE, (void __user *) arg, sizeof(int)))
				// return -EFAULT;

			// use copy_from_user for pointer references
			//	if (copy_from_user(&data, (int *) arg, sizeof(int)))
			     // return -EFAULT;
			// otherwise, use straightforward reference.
			printk("w. data==%ld\n",(unsigned long)arg);
			retval=data=(unsigned long) arg;
			break;
		default:
			retval = -EINVAL;
	}
	return retval;
}


static struct file_operations CDD_fops =
{
	// for LINUX_VERSION_CODE 2.4.0 and later 
	owner:	THIS_MODULE, 	// struct module *owner
	open:	CDD_open, 	// open method 
	read:   CDD_read,	// read method 
	write:  CDD_write, 	// write method 
	release:  CDD_release, 	// release method
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	unlocked_ioctl:  CDD_ioctl 	// ioctl method
#else
	ioctl:  CDD_ioctl 	// ioctl method
#endif
};

static int CDD_init(void)
{
	int i;
	i = register_chrdev (32, "CDD", & CDD_fops);
	if (i != 0) return - EIO;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	string=vmalloc(4096);
#endif

	return 0;
}

static void CDD_exit(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	vfree(string);
#endif
	unregister_chrdev (32, "CDD");
}

module_init(CDD_init);
module_exit(CDD_exit);

