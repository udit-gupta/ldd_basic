// Example# 6.9b:  Simple Char Driver 
//		with Static or Dynamic Major# as requested at run-time.

//		create a directory entry in /proc
//		create a read-only proc entry for this driver.
//		create multiple devices .. same major#, many minor#
//			.. uses simplified access to device fields (2.6)
//		works only in 2.6 (.. a la Example# 3.2)

//      using dev_t, struct cdev (2.6)          (example 3.1b)
//      using myCDD structure                   (example 3.2b)
//      using file->private_data structure      (example 3.3b)
//      using container_of() macro              (example 3.4b)
//      using Static/Dynamic Major#             (example 3.5b)
//      using Static/Dynamic Major# as a param  (example 3.6b)
//      creating a read-only /proc entry        (example 4.4b)
//      creating a readwrite /proc entry        (example 4.5b)
//      using semaphore, spinlock and atomic_t  (example 5.5b)
//		using multiple devices (minor#s)		(example 6.5b)
//		using llseek() 							(example 6.6b)
//		using poll() 							(example 6.7b)
//		using O_WRONLY() flag					(example 6.8b)
//		using proc_dir_entry->data			(here)

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>

#include <linux/moduleparam.h>          // for module_param

#include <linux/proc_fs.h>              // for proc entry.
#include <linux/sched.h>                // for current->pid

#include <linux/poll.h>                	// for poll() functions and macros

// #include <linux/config.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>            	// for sys_unlink and sys_mknod


struct class *class_simple_create(char *, char *);
struct class *class_create(char *, char *)

class_simple_device_add(struct class *, dev_t, char *, char *);
class_device_create(struct class *, char *, dev_t, char *, char *);

class_simple_device_remove(dev_t);
class_simple_destroy(struct class *);
class_device_destroy(struct class *, dev_t);
class_destroy(struct class *);

struct class_simple *pClass;
struct class *pClass;

#include <linux/device.h>

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( 
			*/

#define CDD		"CDD2"
#define myCDD  		"myCDD"

#define CDDMAJOR  	32
#define CDDMINOR  	0		// 2.6
#define CDDNUMDEVS  	6		// 2.6

#define CDD_PROCLEN     32

#define CDD_BUFSIZE		132

// static unsigned int counter = 0;

static unsigned int CDDmajor = CDDMAJOR;

static unsigned int CDDparm = CDDMAJOR;
module_param(CDDparm,int,0);

// static struct cdev cdev;

dev_t	firstdevno;
struct CDDdev_struct{
		atomic_t	counter;	
        char 		CDD_name[CDD_PROCLEN + 1];
        char 		CDD_storage[CDD_BUFSIZE + 1];

		struct cdev 	cdev;
		dev_t		devno;

        struct semaphore CDDsem;
        spinlock_t      CDDspinlock;
        int             CDDnumopen;
        atomic_t        CDDcuropen;
        atomic_t        CDDtotopen;
        atomic_t        CDDtotreads;
        atomic_t        CDDtotwrites;
};

static struct CDDdev_struct CDD_dev[CDDNUMDEVS];

static struct proc_dir_entry *proc_myCDD;
static struct proc_dir_entry *proc_CDD2[CDDNUMDEVS];

struct CDD_proc {
        char CDD_procname[CDD_PROCLEN + 1];
        char CDD_procvalue[132];
        char CDD_procflag;
};

static struct CDD_proc CDD_procdata[CDDNUMDEVS];

/*
// #include <init/do_mounts.h>            	// for create_dev() 
static inline int create_dev(char *name, dev_t dev)
{
        sys_unlink(name);
        return sys_mknod(name, S_IFBLK|0600, new_encode_dev(dev));
}
*/


static int CDD_open (struct inode *inode, struct file *file)
{
	// MOD_INC_USE_COUNT;

	struct CDDdev_struct *thisCDD=
		container_of(inode->i_cdev, struct CDDdev_struct, cdev);

       // truncate filelength to 0, if open was O_TRUNC
    if ( file->f_flags & O_TRUNC )  {

        printk(KERN_ALERT "file '%s' opened O_TRUNC\n", 
			file->f_dentry->d_name.name);
	
        if (down_interruptible (&(thisCDD->CDDsem)))
            return -ERESTARTSYS;

		thisCDD->CDD_storage[0]=0;
	    atomic_set(&thisCDD->counter,0);
        up (&(thisCDD->CDDsem));
    }
    if ( file->f_flags & O_APPEND )  {
        printk(KERN_ALERT "file '%s' opened O_APPEND\n", 
			file->f_dentry->d_name.name);
	}

    /* and use filp->private_data to point to the device data */
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

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

static loff_t CDD_llseek (struct file *file, loff_t CDDoffset, int whence)
{
	int len;

	// unsigned int CDDmajor=imajor(file->f_dentry->d_inode);
	// unsigned int CDDminor=iminor(file->f_dentry->d_inode);

	struct CDDdev_struct *thisCDD=file->private_data;
	// char *string=thisCDD->CDD_storage;

    loff_t newoffset, curroffset;

	curroffset = file->f_pos;
	len = atomic_read(&thisCDD->counter);

	switch(whence) {
		case SEEK_SET: 				// CDDoffset can be 0 or +ve
	
			if (CDDoffset < 0)		// Cannot seek past beginning of data
				return -EINVAL;
	
			if (CDDoffset > CDD_BUFSIZE) // Do Not Support Sparse files.
				return -EINVAL;			 // Cannot Seek Past EOF

			if (CDDoffset > len) 	 // Cannot Seek past actually used.
				return -EINVAL;			 

			newoffset = curroffset + CDDoffset;
			break;

		case SEEK_CUR: 				// CDDoffset can be +ve or -ve
			
            if (CDDoffset < 0) {	// CDDoffset is -ve
				
				if ((CDDoffset + curroffset) < 0)   // new offset cannot be < 0;
					return -EINVAL;

			}
			else {					// CDDoffset is +ve

				if ((CDDoffset + curroffset) > CDD_BUFSIZE)   // No Sparse files.
					return -EINVAL;

				if ((CDDoffset + curroffset) > len)   // Cannot seek past actual use
					return -EINVAL;
			}
				
			newoffset = curroffset + CDDoffset;
			break;	
	
		case SEEK_END:				// CDDoffset can be 0 or -ve
			
			if (CDDoffset > 0) 		// No Sparse files.
				return -EINVAL;

			if ((CDDoffset + len) < 0)   // new offset cannot be < 0;
				return -EINVAL;

			// newoffset = CDD_BUFSIZE + CDDoffset;	// technically, correct!!

			newoffset = len + CDDoffset;
			break;	

		default:
			return -EINVAL;
	}

	file->f_pos = newoffset;

	return newoffset;
}

static unsigned int CDD_poll (struct file *file, struct poll_table_struct *polltbl) 
{
	unsigned int mask=0;

	mask |= POLLIN | POLLRDNORM;
	mask |= POLLOUT | POLLWRNORM;

	return mask;
}

static struct file_operations CDD_fops =
{
	// for LINUX_VERSION_CODE 2.4.0 and later 
	owner:	THIS_MODULE, 	// struct module *owner
	open:	CDD_open, 		// open method 
	read:   CDD_read,		// read method 
	write:  CDD_write, 		// write method 
    llseek:  CDD_llseek,  	// llseek method
	poll:	CDD_poll,	  	// poll method
	release:  CDD_release 	// release method
};


static int readproc_CDD2(char *buf, char **start,
        off_t offset, int len, int *eof, void *data)
{
		int thisPROC=(int) data;
        struct CDD_proc *usrsp=&CDD_procdata[thisPROC];

        *eof = 1;

        if (offset) { return 0; }
        else if (usrsp->CDD_procflag) {
           usrsp->CDD_procflag=0;
           return(sprintf(buf, "Hello..I got \"%s\"\n",usrsp->CDD_procvalue));
        }
        else
           return(sprintf(buf, "Hello from process %d\n", (int)current->pid));

}


static int writeproc_CDD2(struct file *file,const char *buf,
                unsigned long count, void *data)
{
		int thisPROC=(int) data;
        struct CDD_proc *usrsp=&CDD_procdata[thisPROC];
        int length=count;

        length = (length<CDD_PROCLEN)? length:CDD_PROCLEN;

        if (copy_from_user(usrsp->CDD_procvalue, buf, length))
                return -EFAULT;

        usrsp->CDD_procvalue[length-1]=0;
        usrsp->CDD_procflag=1;
        return(length);
}

static int CDD_init(void)
{
	char procfs_name[80];
	// char devfs_name[80];
	int i, errno;
	struct cdev *cdevp;
	struct CDDdev_struct *thisCDD;

	dev_t devno;
  struct class_simple *CDD_class;

	CDD_class = (struct class_simple *) class_simple_create(THIS_MODULE, CDD);

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

		// finally, create the device node here.
	  // sprintf(devfs_name,"/dev/CDD_%c", 'a' + i);
		// create_dev(devfs_name,thisCDD->devno);
		class_simple_device_add(CDD_class, thisCDD->devno,
					NULL, "CDD_%c",'a' + i);
	}

        // Create the necessary proc directory
        proc_myCDD = proc_mkdir(myCDD,0);

        // Create the necessary proc entries
	for (i=0;i<CDDNUMDEVS; i++) {

		sprintf(procfs_name,"%s%d",CDD,i);
		
        proc_CDD2[i] = create_proc_entry(procfs_name,0,proc_myCDD);
        proc_CDD2[i]->read_proc = readproc_CDD2;
        proc_CDD2[i]->write_proc = writeproc_CDD2;
        proc_CDD2[i]->data = (void *)i;

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
        // proc_CDD2->owner = THIS_MODULE;
	#endif
	
	}
		
	
	return 0;
}

static void CDD_exit(void)
{
	int i=0;
	struct cdev *cdevp;
	struct CDDdev_struct *thisCDD;
	char procfs_name[80];

	// remove proc entries
	for (i=0;i<CDDNUMDEVS; i++) {
		sprintf(procfs_name,"%s%d",CDD,i);
        if (proc_CDD2[i]) remove_proc_entry (procfs_name, proc_myCDD);
	}

	// remove proc entries
   	if (proc_myCDD) remove_proc_entry (myCDD, 0);

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
