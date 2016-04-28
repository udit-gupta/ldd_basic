// Example# 5.2c:  Simple Char Driver 
//		with Static or Dynamic Major# as requested at run-time.
//		  create a read-only proc entry for this driver.
//		works only in 2.6 (.. a la Example# 3.2)

//      using dev_t, struct cdev (2.6)	  (example 3.1b)
//      using myCDD structure		   (example 3.2b)
//      using file->private_data structure      (example 3.3b)
//      using container_of() macro	      (example 3.4b)
//      using Static/Dynamic Major#	     (example 3.5b)
//      using Static/Dynamic Major# as a param  (example 3.6b)
//      creating a read-only /proc entry	(here)(example 5.2c)


#include <linux/version.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>
#include <linux/device.h>   // device_create class_create


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/moduleparam.h>
#endif

#include <linux/proc_fs.h>	      // for proc entry.
#include <linux/sched.h>		// for current->pid

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( 
			*/

#define CDD		"CDD2"
#define CDDMAJOR  	32
#define CDDMINOR  	0	// 2.6
#define CDDNUMDEVS  	1	// 2.6
#define MYDEV "MYDEV"

static unsigned int CDDmajor = CDDMAJOR;

static unsigned int CDDparm = CDDMAJOR;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	module_param(CDDparm,int,0);
#endif

static struct proc_dir_entry *proc_CDD2;
static struct proc_dir_entry *proc_mydev;

// static struct cdev cdev;
// static unsigned int counter = 0;
// static char string [132];

dev_t   firstdevno;
struct CDDdev_struct {
  unsigned int    counter;
  char	    *CDD_storage;
  struct cdev     cdev;
};

static struct CDDdev_struct CDD_dev;

static int CDD_open (struct inode *inode, struct file *file)
{
  struct CDDdev_struct *thisCDD=
  	container_of(inode->i_cdev, struct CDDdev_struct, cdev);

   file->private_data=thisCDD;

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


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int eof[1];

static ssize_t read_hello(struct file *file, char *buf,
size_t len, loff_t *ppos)
#else
static int read_hello (char *buf, char **start, off_t offset,
    int len, int *eof, void *unused)
#endif
{
	int n=0;
  if (*eof!=0) { *eof=0; return 0; }

	n=sprintf(buf,"Hello from Process %d\n", (int)current->pid);

	*eof = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  if(n)
  	*ppos=len;
#endif

	return n;
}

static struct class *CDD_class;
static struct CDDdev {
  const char *name;
  umode_t mode;
  const struct file_operations *fops;
} CDDdevlist[] =  {
  [0] = { "CDD2",0666, &CDD_fops },
};


static char *CDD_devnode(struct device *dev, umode_t *mode)
{
  if (mode && CDDdevlist[MINOR(dev->devt)].mode)
    *mode = CDDdevlist[MINOR(dev->devt)].mode;
  return NULL;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static const struct file_operations proc_fops =
{
 .owner = THIS_MODULE,
 .read  = read_hello,
};
#endif

static int CDD_init(void)
{
	int i;
	struct CDDdev_struct *thisCDD=&CDD_dev;
	thisCDD->CDD_storage=vmalloc(4096);

	CDDmajor = CDDparm;

	if (CDDmajor) {
		//  Step 1a of 2:  create/populate device numbers
   	firstdevno = MKDEV(CDDmajor, CDDMINOR);

   	//  Step 1b of 2:  request/reserve Major Number from Kernel
   	i = register_chrdev_region(firstdevno,CDDNUMDEVS,CDD);
		if (i < 0) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i;}
	}
	else {
    //  Step 1c of 2:  Request a Major Number Dynamically.
    i = alloc_chrdev_region(&firstdevno, CDDMINOR, CDDNUMDEVS, CDD);
    if (i < 0) { printk(KERN_ALERT "Error (%d) adding CDD", i); return i;}
    CDDmajor = MAJOR(firstdevno);
		printk(KERN_ALERT "kernel assigned major#: %d to CDD\n", CDDmajor);
	}

  CDD_class = class_create(THIS_MODULE, "CDD");
  if (IS_ERR(CDD_class))
    return PTR_ERR(CDD_class);

  CDD_class->devnode = CDD_devnode;
  device_create(CDD_class, NULL, MKDEV(CDDmajor, 0), NULL, CDD);

	//  Step 2a of 2:  initialize thisCDD->cdev struct
  cdev_init(&(thisCDD->cdev), &CDD_fops);

  //  Step 2b of 2:  register device with kernel
  thisCDD->cdev.owner = THIS_MODULE;
  thisCDD->cdev.ops = &CDD_fops;
  i = cdev_add(&(thisCDD->cdev), firstdevno, CDDNUMDEVS);
  if (i) { printk(KERN_ALERT "Error (%d) adding %s\n",i,CDD); return i;}

  proc_mydev = proc_mkdir(MYDEV,0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  proc_CDD2 = proc_create("hello", 0777, proc_mydev, &proc_fops);
#else
  proc_CDD2 = create_proc_entry(HELLO,0,proc_mydev);
  proc_CDD2->read_proc = read_hello;
  proc_CDD2->write_proc = write_hello;
#endif


  #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
      proc_CDD2->owner = THIS_MODULE;
  #endif      

	return 0;
}

static void CDD_exit(void)
{
 	struct CDDdev_struct *thisCDD=&CDD_dev;
			
	vfree(thisCDD->CDD_storage);

 	if (proc_CDD2) remove_proc_entry (CDD, 0);

	//  Step 1 of 2:  unregister device with kernel
 	cdev_del(&(thisCDD->cdev));

  device_destroy(CDD_class, MKDEV(CDDmajor, 0));
  class_destroy(CDD_class);

 	//  Step 2b of 2:  Release request/reserve of Major Number from Kernel
	unregister_chrdev_region(firstdevno, CDDNUMDEVS);

	if (CDDmajor != CDDparm) 
		printk(KERN_ALERT "kernel unassigned major#: %d from CDD\n",CDDmajor);
}

module_init(CDD_init);
module_exit(CDD_exit);
