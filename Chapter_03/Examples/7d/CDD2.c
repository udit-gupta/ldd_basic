// Example# 3.6b: Simple Char Driver with Static or Dynamic Major# 
//		  as requested at install-time.
//                works only in 2.6 (.. a la Example# 3.2)

//      using dev_t, struct cdev (2.6)          (example 3.1b)
//      using myCDD structure                   (example 3.2b)
//      using file->private_data structure      (example 3.3b)
//      using container_of() macro              (example 3.4b)
//      using Static/Dynamic Major#          	(example 3.5b)
//      using Static/Dynamic Major# as a param  (example 3.6b)
//      using O_TRUNC and O_APPEND				(here .. example 3.7b)

#include <linux/version.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/moduleparam.h>
#endif

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( */

#define CDD		"CDD2"
#define CDDMAJOR  	32
#define CDDMINOR  	0	// 2.6
#define CDDNUMDEVS  	1	// 2.6

static unsigned int CDDmajor = CDDMAJOR;
static unsigned int CDDparm = CDDMAJOR;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	module_param(CDDparm,int,0);
#else
	MODULE_PARM(CDDparm,"i");
#endif

dev_t   firstdevno;
struct CDDdev_struct {
        unsigned int    counter;
        char            *CDD_storage;
        struct cdev     cdev;
};

static struct CDDdev_struct myCDD;

static int CDD_open (struct inode *inode, struct file *file)
{
        struct CDDdev_struct *thisCDD=
                container_of(inode->i_cdev, struct CDDdev_struct, cdev);

	    if ( file->f_flags & O_TRUNC )  {

        	printk(KERN_ALERT "file '%s' opened O_TRUNC\n",
            	file->f_dentry->d_name.name);

        	thisCDD->CDD_storage[0]=0;
        	thisCDD->counter=0;
    	}

    	if ( file->f_flags & O_APPEND )  {
        	printk(KERN_ALERT "file '%s' opened O_APPEND\n",
	            file->f_dentry->d_name.name);
    	}

        file->private_data=thisCDD;

	return 0;
}

static int CDD_release (struct inode *inode, struct file *file)
{
 	struct CDDdev_struct *thisCDD=file->private_data;

	if( thisCDD->counter <= 0 ) return 0; // overcome compiler warning msg.

	// MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t CDD_read (struct file *file, char *buf, 
size_t count, loff_t *ppos)
{
	int err;
 	struct CDDdev_struct *thisCDD=file->private_data;

	if( thisCDD->counter <= 0 ) return 0;

  if( *ppos >= thisCDD->counter) return 0;
  else if( *ppos + count >= thisCDD->counter)
    count = thisCDD->counter - *ppos;

  if( count <= 0 ) return 0;    
	printk(KERN_ALERT "CDD_read: count=%d\n", count);

	// bzero(buf,64);  // a bogus 64byte initialization
	memset(buf,0,64);  // a bogus 64byte initialization
	err = copy_to_user(buf,&(thisCDD->CDD_storage[*ppos]),count);
	if (err != 0) return -EFAULT;
	
	buf[count]=0;
	*ppos += count;
	return count;
}

static ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
{
	int err;
	struct CDDdev_struct *thisCDD=file->private_data;

	err = copy_from_user(&(thisCDD->CDD_storage[*ppos]),buf,count);
	if (err != 0) return -EFAULT;

	thisCDD->counter += count;
	*ppos+=count;
	return count;
}
static loff_t CDD_llseek (struct file *file, loff_t newpos, int whence)
{
  int pos;
	struct CDDdev_struct *thisCDD=file->private_data;

	switch(whence) {
		case SEEK_SET:        // CDDoffset can be 0 or +ve
			pos=newpos;
			break;
		case SEEK_CUR:        // CDDoffset can be 0 or +ve
			pos=(file->f_pos + newpos);
			break;
		case SEEK_END:        // CDDoffset can be 0 or +ve
			pos=(thisCDD->counter + newpos);
			break;
		default:
			return -EINVAL;
	}
	if ((pos < 0)||(pos>thisCDD->counter)) 
		return -EINVAL;
				
	file->f_pos = pos;
	return pos;
}

static struct file_operations CDD_fops =
{
	// for LINUX_VERSION_CODE 2.4.0 and later 
	owner:	THIS_MODULE, 	// struct module *owner
	open:	CDD_open, 	// open method 
	read:   CDD_read,	// read method 
	write:  CDD_write, 	// write method 
	llseek:  CDD_llseek,  // covered in detail in Ch6. Just for fwd ref now.
	release:  CDD_release 	// release method
};

static int CDD_init(void)
{
	int i;
  struct CDDdev_struct *thisCDD=&myCDD;

	thisCDD->CDD_storage=vmalloc(4096);

	CDDmajor = CDDparm;

	if (CDDmajor) {
        	//  Step 1a of 2:  create/populate device numbers
        	firstdevno = MKDEV(CDDmajor, CDDMINOR);

        	//  Step 1b of 2:  request/reserve Major Number from Kernel
        	i = register_chrdev_region(firstdevno,1,CDD);
        	if (i < 0) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i;}
	}
	else {
        	//  Step 1c of 2:  Request a Major Number Dynamically.
                i = alloc_chrdev_region(&firstdevno, CDDMINOR, CDDNUMDEVS, CDD);
        	if (i < 0) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i;}
                CDDmajor = MAJOR(firstdevno);
		printk(KERN_ALERT "kernel assigned major number: %d to CDD\n", CDDmajor);
	}
		

       	//  Step 2a of 2:  initialize thisCDD->cdev struct
       	cdev_init(&thisCDD->cdev, &CDD_fops);

       	//  Step 2b of 2:  register device with kernel
       	thisCDD->cdev.owner = THIS_MODULE;
       	thisCDD->cdev.ops = &CDD_fops;
        i = cdev_add(&thisCDD->cdev, firstdevno, CDDNUMDEVS);
        if (i) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i; }

        return 0;

}

static void CDD_exit(void)
{
	struct CDDdev_struct *thisCDD=&myCDD;

	vfree(thisCDD->CDD_storage);
        
	//  Step 1 of 2:  unregister device with kernel
 	cdev_del(&thisCDD->cdev);

 	//  Step 2b of 2:  Release request/reserve of Major Number from Kernel
 	unregister_chrdev_region(firstdevno, CDDNUMDEVS);

	if (CDDmajor != CDDMAJOR) printk(KERN_ALERT "kernel unassigned major number: %d from CDD\n", CDDmajor);
}

module_init(CDD_init);
module_exit(CDD_exit);

