// Example# 6.5b:  Simple Char Driver 
//		with Static or Dynamic Major# as requested at run-time.

//		create a directory entry in /proc
//		  create a read-only proc entry for this driver.
//		  create multiple devices .. same major#, many minor#
//		      .. uses simplified access to device fields (2.6)
//		works only in 2.6 (.. a la Example# 3.2)


//      using dev_t, struct cdev (2.6)	  (example 3.1b)
//      using myCDD structure		   (example 3.2b)
//      using file->private_data structure      (example 3.3b)
//      using container_of() macro	      (example 3.4b)
//      using Static/Dynamic Major#	     (example 3.5b)
//      using Static/Dynamic Major# as a param  (example 3.6b)
//      creating a read-only /proc entry	(example 4.4b)
//      creating a readwrite /proc entry	(example 4.5b)
//      using semaphore, spinlock and atomic_t  (example 5.5b)
//		using multiple devices (minor#s)		(here)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>

#include <linux/moduleparam.h>	  // for module_param

#include <linux/proc_fs.h>	      // for proc entry.
#include <linux/sched.h>		// for current->pid

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( 
			*/

#define CDD		"CDD2"
#define myCDD  		"myCDD"

#define CDDMAJOR  	32
#define CDDMINOR  	0		// 2.6
#define CDDNUMDEVS  	6		// 2.6

#define CDD_PROCLEN     32

// static unsigned int counter = 0;

static unsigned int CDDmajor = CDDMAJOR;

static unsigned int CDDparm = CDDMAJOR;
module_param(CDDparm,int,0);

// static struct cdev cdev;

dev_t	firstdevno;
struct CDDdev_struct{
		atomic_t	counter;	
	char 		CDD_name[CDD_PROCLEN + 1];
	char 		CDD_storage[132];

	struct cdev 	cdev;
	dev_t		devno;

	struct semaphore CDDsem;
	spinlock_t      CDDspinlock;
	int	     CDDnumopen;
	atomic_t	CDDcuropen;
	atomic_t	CDDtotopen;
	atomic_t	CDDtotreads;
	atomic_t	CDDtotwrites;
};

static struct CDDdev_struct CDD_dev[CDDNUMDEVS];

static struct proc_dir_entry *proc_myCDD;
static struct proc_dir_entry *proc_dirCDD;

struct CDD_proc {
	char CDD_procname[CDD_PROCLEN + 1];
	char CDD_procvalue[132];
	char CDD_procflag;
};

static struct CDD_proc CDD_procdata;

static int CDD_open (struct inode *inode, struct file *file)
{
	// MOD_INC_USE_COUNT;

	struct CDDdev_struct *thisCDD=
		container_of(inode->i_cdev, struct CDDdev_struct, cdev);

	file->private_data=thisCDD;

  spin_trylock(&(thisCDD->CDDspinlock));
  thisCDD->CDDnumopen++;
  spin_unlock(&(thisCDD->CDDspinlock));

  atomic_inc(&(thisCDD->CDDcuropen));
  atomic_inc(&(thisCDD->CDDtotopen));

	return 0;
}

static int CDD_release (struct inode *inode, struct file *file)
{
	struct CDDdev_struct *thisCDD=file->private_data;

	// MOD_DEC_USE_COUNT;
	spin_lock(&(thisCDD->CDDspinlock));
	thisCDD->CDDnumopen--;
	spin_unlock(&(thisCDD->CDDspinlock));

	atomic_dec(&(thisCDD->CDDcuropen));

	return 0;
}

static ssize_t CDD_read (struct file *file, char *buf, 
size_t count, loff_t *ppos)
{
	int len, err;

	// unsigned int CDDmajor=imajor(file->f_dentry->d_inode);
	// unsigned int CDDminor=iminor(file->f_dentry->d_inode);

	struct CDDdev_struct *thisCDD=file->private_data;
	char *string=thisCDD->CDD_storage + *ppos;

	if( atomic_read(&thisCDD->counter) <= 0 ) return 0;

	// semaphore decr
	if (down_interruptible(&(thisCDD->CDDsem)))
		return -ERESTARTSYS;

	err = copy_to_user(buf,string,
		atomic_read(&thisCDD->counter));

	// semaphore incr
	up(&(thisCDD->CDDsem));

	if (err != 0) return -EFAULT;

	// increment #reads
	atomic_inc(&(thisCDD->CDDtotreads));

	len = atomic_read(&thisCDD->counter);
	atomic_set(&thisCDD->counter,0);
	return len;
}

static ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
{
	int err;

	// unsigned int CDDmajor=imajor(file->f_dentry->d_inode);
	// unsigned int CDDminor=iminor(file->f_dentry->d_inode);

	struct CDDdev_struct *thisCDD=file->private_data;
	char *string=(thisCDD->CDD_storage) + 
		atomic_read(&thisCDD->counter) + (long) *ppos;

	// semaphore decr
	if (down_interruptible(&(thisCDD->CDDsem)))
		return -ERESTARTSYS;

	err = copy_from_user(string,buf,count);

	// semaphore incr
	up(&(thisCDD->CDDsem));

	if (err != 0) return -EFAULT;

       // increment #writes
	atomic_inc(&(thisCDD->CDDtotwrites));

	atomic_add(count, &thisCDD->counter);
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
{
  off_t offset=*ppos;
#else
static int read_hello (char *buf, char **start, off_t offset,
    int len, int *eof, void *unused)
{
#endif
  struct CDD_proc *usrsp=&CDD_procdata;
  int n=0;

  if (*eof!=0) { *eof=0; return 0; }

  if (offset) { n=0; }
  else if (usrsp->CDD_procflag) {
     usrsp->CDD_procflag=0;
     n=sprintf(buf, "Hello..I got \"%s\"\n",usrsp->CDD_procvalue);
  }
  else
     n=sprintf(buf, "Hello from process %d\n", (int)current->pid);

    *eof = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
    if(n)
      *ppos=len;
#endif
    return n;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static ssize_t write_hello(struct file *file, const char __user *buf,
  size_t count, loff_t *ppos)
#else
static int write_hello (struct file *file,const char * buf,
    unsigned long count, void *data)
#endif
{
	int length=count;
	struct CDD_proc *usrsp=&CDD_procdata;

	length = (length<CDD_PROCLEN)? length:CDD_PROCLEN;

	if (copy_from_user(usrsp->CDD_procvalue, buf, length))
		return -EFAULT;

	usrsp->CDD_procvalue[length-1]=0;
	usrsp->CDD_procflag=1;
	return(length);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static const struct file_operations proc_fops =
{
 .owner = THIS_MODULE,
 .read  = read_hello,
 .write  = write_hello,
};
#endif

static int CDD_init(void)
{
	int i, errno;
	struct cdev *cdevp;
	struct CDDdev_struct *thisCDD;

	dev_t devno;

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
		printk(KERN_ALERT "kernel assigned major number: %d to CDD\n", CDDmajor);
	}
		
	// initialize and allocate the devices
	for (i=0;i<CDDNUMDEVS; i++) {
		// set our references here .. 1 of 2
		thisCDD=&(CDD_dev[i]);
		cdevp=&(thisCDD->cdev);

		// set our references here .. 2 of 2
		devno = MKDEV(CDDmajor,i);

		// start initializing our devices
		sema_init(&(thisCDD->CDDsem),1);	
		thisCDD->devno = devno;

       		//  Step 2a of 2:  initialize cdev struct
		cdev_init(cdevp, &CDD_fops);	

       		//  Step 2b of 2:  register device with kernel
       		cdevp->owner = THIS_MODULE;
       		cdevp->ops = &CDD_fops;
		errno = cdev_add(cdevp, devno, CDDNUMDEVS);
		if (errno) { printk(KERN_ALERT "Error (%d) adding %s(%d)\n",errno,CDD,i); return errno;}

		// initialize CDD_dev Semaphore
		sema_init(&(thisCDD->CDDsem),1);

		// initialize CDD_dev Semaphore
		spin_lock_init(&(thisCDD->CDDspinlock));

		// init value of counters;
		atomic_set(&(thisCDD->CDDcuropen),0);
		atomic_set(&(thisCDD->CDDtotopen),0);
		atomic_set(&(thisCDD->CDDtotreads),0);
		atomic_set(&(thisCDD->CDDtotwrites),0);
	}

  proc_dirCDD = proc_mkdir(myCDD,0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  proc_myCDD = proc_create("hello", 0777, proc_dirCDD, &proc_fops);
#else
  proc_myCDD = create_proc_entry(HELLO,0,proc_myCDD);
  proc_myCDD->read_proc = read_hello;
  proc_myCDD->write_proc = write_hello;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
  proc_myCDD->owner = THIS_MODULE;
#endif

	return 0;
}

static void CDD_exit(void)
{
	int i=0;
	struct cdev *cdevp;
	struct CDDdev_struct *thisCDD;

  if (proc_myCDD) remove_proc_entry ("hello", proc_dirCDD);
  if (proc_dirCDD) remove_proc_entry (myCDD, 0);

	// initialize and allocate the devices
	for (i=0;i<CDDNUMDEVS; i++) {
		// set our references here .. 1 of 2
		thisCDD=&(CDD_dev[i]);
		cdevp=&(thisCDD->cdev);

		printk(KERN_ALERT "CDD(%d): Stats:TotOpen=%d; NumOpen=%d; CurOpen=%d\n",
			i,
			atomic_read(&thisCDD->CDDtotopen),
			thisCDD->CDDnumopen,
			atomic_read(&thisCDD->CDDcuropen));
		printk(KERN_ALERT "CDD(%d): Stats:TotReads=%d; TotWrites=%d\n",
			i,
			atomic_read(&thisCDD->CDDtotreads),
			atomic_read(&thisCDD->CDDtotwrites));

		//  Step 1 of 2:  unregister device with kernel
		cdev_del(cdevp);
	}

	//  Step 2 of 2:  Release request/reserve of Major Number from Kernel
	unregister_chrdev_region(firstdevno, CDDNUMDEVS);

	if (CDDmajor != CDDparm) 
		printk(KERN_ALERT "kernel unassigned major number: %d from CDD\n", CDDmajor);
}

module_init(CDD_init);
module_exit(CDD_exit);

