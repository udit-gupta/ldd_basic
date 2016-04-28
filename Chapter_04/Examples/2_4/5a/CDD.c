// Example# 4.5a:  Simple Char Driver with Static or Dynamic Major# 
//		  as requested at install-time.

//		  create a directory entry in /proc
//                create a readwrite proc entry for this driver.
//                works in 2.4 and 2.6 (.. a la Example# 3.2)

//      using myCDD structure                   (example 3.2a)
//      using file->private_data structure      (example 3.3a)
//      using static/dynamic major#             (example 3.5a)
//      using static/dynamic major#; with parm  (example 3.6a)
//      creating a read-only proc entry         (example 4.4a)
//      creating a readwrite proc entry         (here)

#include <linux/version.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	#include <linux/moduleparam.h> 	// for module_param
#endif

#include <linux/proc_fs.h>              // for proc entry.
#include <linux/sched.h>                // for current->pid

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( 
			*/

#define CDD		"CDD"
#define myCDD		"myCDD"

#define CDDMAJOR  	32
#define CDDMINOR  	0	// 2.6
#define CDDNUMDEVS  	1	// 2.6

#define CDD_PROCLEN	32

static unsigned int CDDmajor = CDDMAJOR;

static unsigned int CDDparm = CDDMAJOR;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	module_param(CDDparm,int,0);
#else
	MODULE_PARM(CDDparm,"i");
#endif

static struct proc_dir_entry *proc_CDD;
static struct proc_dir_entry *proc_myCDD;

struct CDDproc_struct {
        char CDD_procname[CDD_PROCLEN + 1];
        char CDD_procvalue[132];
        char CDD_procflag;
};

static struct CDDproc_struct CDD_proc;

struct CDDdev_struct {
        unsigned int counter;
        char CDD_storage [132];
};

static struct CDDdev_struct CDD_dev;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int CDD_open (struct inode *inode, struct file *file)
#else
static int CDD_open (struct file *file)
#endif
{
        struct CDDdev_struct *thisCDD=&CDD_dev;

        file->private_data=thisCDD;

	// MOD_INC_USE_COUNT;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int CDD_release (struct inode *inode, struct file *file)
#else
static int CDD_release (struct file *file)
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


static int readproc_CDD(char *buf, char **start,
        off_t offset, int len, int *eof, void *unused)
{
        struct CDDproc_struct *usrsp=&CDD_proc;

        *eof = 1;

	if (offset) { return 0; }
	else if (usrsp->CDD_procflag) {
	   usrsp->CDD_procflag=0;
	   return(sprintf(buf, "Hello..I got \"%s\"\n",usrsp->CDD_procvalue));
        }
        else
           return(sprintf(buf, "Hello from process %d\n", (int)current->pid));
}


static int writeproc_CDD (struct file *file,const char *buf,
                unsigned long count, void *data)
{
        int length=count;
        struct CDDproc_struct *usrsp=&CDD_proc;

        length = (length<CDD_PROCLEN)? length:CDD_PROCLEN;

        if (copy_from_user(usrsp->CDD_procvalue, buf, length))
                return -EFAULT;

        usrsp->CDD_procvalue[length-1]=0;
        usrsp->CDD_procflag=1;
        return(length);
}


static int CDD_init(void)
{
	int i;

	CDDmajor = CDDparm;

       	//  Step 1:  register with kernel
       	i = register_chrdev(CDDmajor, CDD, &CDD_fops);
	if (i < 0) {
		printk(KERN_ALERT "CDD:Could not get major number:\n");
		return -EINVAL;
	}

	CDDmajor = i;
	if (CDDmajor != CDDparm) 
		printk(KERN_ALERT "kernel assigned major number: %d to CDD\n", CDDmajor);

        // Create the necessary proc entries
        proc_myCDD = proc_mkdir(myCDD,0);

        proc_CDD =   create_proc_entry(CDD,0,proc_myCDD);
        proc_CDD->read_proc = readproc_CDD;
        proc_CDD->write_proc = writeproc_CDD;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
        proc_CDD->owner = THIS_MODULE;
#endif

	return 0;
}

static void CDD_exit(void)
{
        if (proc_CDD) { 
	   remove_proc_entry (CDD, proc_myCDD);
		
	   if (proc_myCDD) {
	   	remove_proc_entry (myCDD, 0);
	   }
	
	}

        //  Step 1:  Release request/reserve of Major Number from Kernel
        unregister_chrdev(CDDmajor,CDD);

	if (CDDmajor != CDDparm) 
		printk(KERN_ALERT "kernel unassigned major number: %d from CDD\n", CDDmajor);
}

module_init(CDD_init);
module_exit(CDD_exit);

