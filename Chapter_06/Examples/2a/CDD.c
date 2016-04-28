// Example# 6.2a:  Simple Char Driver with Static or Dynamic Major# 
//		  as requested at install-time.
//		  create a directory entry in /proc
//                create a read-only proc entry for this driver.
//                works only in 2.6 (.. a la Example# 3.2)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>

#include <linux/moduleparam.h>          // for module_param

#include <linux/proc_fs.h>              // for proc entry.
#include <linux/sched.h>                // for current->pid

MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( 
			*/

#define CDD		"CDD"
#define myCDD		"CDD"

#define CDDMAJOR  	32
#define CDDMINOR  	0	// 2.6
#define CDDNUMDEVS  	1	// 2.6

#define CDD_PROCLEN	32

static unsigned int counter = 0;
static char string [132];

static unsigned int CDDmajor = CDDMAJOR;

static unsigned int CDDparm = CDDMAJOR;
module_param(CDDparm,int,0);

static struct proc_dir_entry *proc_CDD;
static struct proc_dir_entry *proc_mydev;

struct CDD_procdata {
	char CDD_procname[CDD_PROCLEN + 1];
	char CDD_procvalue[132];
	char CDD_procflag;
};

static struct CDD_procdata CDD_data;

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
	struct CDD_procdata *usrsp=&CDD_data;
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
	struct CDD_procdata *usrsp=&CDD_data;

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
 .write = write_hello
};
#endif

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
	proc_mydev = proc_mkdir(myCDD,0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  proc_CDD = proc_create("hello", 0777, proc_mydev, &proc_fops);
#else
  proc_CDD = create_proc_entry(HELLO,0,proc_mydev);
  proc_CDD->read_proc = read_hello;
  proc_CDD->write_proc = write_hello;
#endif


#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
	proc_CDD->owner = THIS_MODULE;
#endif

	return 0;
}

static void CDD_exit(void)
{
	if (proc_CDD) { 
	   remove_proc_entry (CDD, proc_mydev);
		
	   if (proc_mydev) {
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

