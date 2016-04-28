// 
// 
// 
//  Refer to http://www.computer-engineering.org/ps2mouse/ for details of the
//  mouse protocol.
//
//
//
//
// Example# 10.8:  Simple Char Driver 
//		with Static or Dynamic Major# as requested at run-time.

//                create a directory entry in /proc
//		  create a readwrite proc entry for this driver.
//                works only in 2.6 (.. a la Example# 3.2)

//      using dev_t, struct cdev (2.6)          (example 3.1b)
//      using myCDD structure                   (example 3.2b)
//      using file->private_data structure      (example 3.3b)
//      using container_of() macro              (example 3.4b)
//      using Static/Dynamic Major#             (example 3.5b)
//      using Static/Dynamic Major# as a param  (example 3.6b)
//      creating a read-only /proc entry        (example 4.4b)
//      creating a readwrite /proc entry        (example 4.5b)
//      interrupt handler+workqueue				(here)

#include <linux/version.h>	// KERNEL_VERSION
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h> // irqreturn_t
#include <linux/irq.h>  // can_return_irq()
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <asm/io.h>			// inb()
#include <linux/moduleparam.h>          // for module_param

#include <linux/proc_fs.h>              // for proc entry.
#include <linux/sched.h>                // for current->pid

#include <linux/slab.h> 
MODULE_LICENSE("GPL");   /*  Kernel isn't tainted .. but doesn't 
			     it doesn't matter for SUSE anyways :-( 
			*/

#define QUEUE_NAME  "CDDqueue"
#define PROC_BUFLEN 1024
#define CDD_MOUSEIRQ 12

#define CDD		"CDD2"
#define myCDD  		"myCDD"
#define MYDEV  		"CDD"

#define CDDMAJOR  	32
#define CDDMINOR  	0	// 2.6
#define CDDNUMDEVS  	1	// 2.6

#define CDD_PROCLEN     32

/* catchall for now
 *  */

#ifndef MYIRQFLAGS
  #define MYIRQFLAGS IRQF_SHARED
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
/*
 *  *  initialize a work item's function pointer
 *   *  */
#define PREPARE_WORK(_work, _func)   \
 do {                                \
      (_work)->func = (_func);       \
    } while (0)

#define PREPARE_DELAYED_WORK(_work, _func)  \
   PREPARE_WORK(&(_work)->work, (_func))
#endif

irqreturn_t irq_handler(int irq, void *dev_id,struct pt_regs *regs); 

int dev_id = 128;

struct movement {
    int x;  
    int y;  
    int z;  
    int one;    //0x60  
    int two;    // 0x61 
    int three; //0x62
    int four;   //0x63  
    int five;   // 0x64 
    int incx;
    int incy;
};

static unsigned int CDDmajor = CDDMAJOR;

static unsigned int CDDparm = CDDMAJOR;
module_param(CDDparm,int,0);

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

static dev_t   firstdevno;

struct CDD2proc_struct {
	char CDD2_procname[CDD_PROCLEN + 1];
	char CDD2_procflag;
	int	 CDD2_used;
};
	atomic_t CDD2_leftcounter;
	atomic_t CDD2_rightcounter;

struct CDD2dev_struct {
	unsigned int CDD2_counter;
	// union {
	//	char *CDD2_storage;
		char *CDD2_procvalue; 
	//}
	unsigned int CDD2_command;
	struct cdev  *CDD2_cdev;
	struct CDD2proc_struct *CDD2_proc;
	struct semaphore *CDD2_sem;
	struct workqueue_struct *CDD2_wq;
	struct work_struct 	CDD2_wk;
    struct movement move;
};

static struct CDD2dev_struct *CDD2_dev;

static void got_movement(struct work_struct *wk) {

	struct CDD2dev_struct *thisCDD=CDD2_dev;
    struct movement *move = &thisCDD->move;
    u8  x=0, y=0, temp, temp1;
    static int first = 0;

	if (&thisCDD->CDD2_wk!=wk) return;
	
    // yv xv ys xs 1 MB RB LB
    if  ((thisCDD->CDD2_counter % 4) == 0)
    {

        printk(KERN_INFO "%0#x counter=%d\n",move->one,
				thisCDD->CDD2_counter);
        move->incx=move->incy=0;
        temp1 = move->one;
        if ((temp1 & 0xe0) == 0xe0 || (temp1 & 0xf0) == 0xf0) {
            thisCDD->CDD2_command++;
            return;
        }
        else
            thisCDD->CDD2_command = 0;

        if ((move->one & 0x08) != 0) {
            if ((move->one & 0x02) != 0) // press right button
                atomic_inc(&CDD2_rightcounter);
            else
                if ((move->one & 0x1) != 0) // press left button
                    atomic_inc(&CDD2_leftcounter);
        }

        if (move->one & 0x10) // decx
            move->incx = 1;
        else
            if (move->one & 0x20) // decy
                move->incy = 1;
    }
    else
        if  ((thisCDD->CDD2_counter % 4) == 1)    // x movement
        {
            if (thisCDD->CDD2_command != 0)
                return;
            temp = move->one;

            if (first == 0)
            {
                x = temp;
                first = 1;
            }
            else
                if (move->incx)
                    x -= temp; // left
                else
                    x += temp; // right

                // if (x < 0) x = 0;
        }
    else
        if ((thisCDD->CDD2_counter % 4) == 2) // y movement
        {
            if (thisCDD->CDD2_command != 0)
                return;

            temp = move->one;
            if (first == 1)
            {
                y = temp;
                first++;
            }
            else
                if  (move->incy)
                    y -= temp; // down
                else
                    y += temp; // up

                // if (y < 0) y = 0;
        }
    else
        if  ((thisCDD->CDD2_counter%4)==3)    // z movement
        {
            if  (thisCDD->CDD2_command != 0)
                thisCDD->CDD2_command = 0;
                move->z = move->one;

            //printk(KERN_ALERT "%d -- %x %d %d %d %d right=%d left=%d.\n",
            //      counter,(int) move->one,move->incy,x,
            //      y,move->z);
        }
    thisCDD->CDD2_counter++;
    if (thisCDD->CDD2_counter == 4) thisCDD->CDD2_counter = 0;
    printk(KERN_INFO "right mouse button clicked=%d \
            \n left mouse button clicked=%d\n",
            atomic_read(&CDD2_rightcounter),
            atomic_read(&CDD2_leftcounter));
}


irqreturn_t irq_handler(int irq, void *dev_id,struct pt_regs *regs)
{
    static int initialize = 0;
	struct CDD2dev_struct *thisCDD=CDD2_dev;
    
    int *id=(int *)dev_id;

    if (*id!=128)
        return IRQ_HANDLED;
    //  printk(KERN_INFO "interrupt id=%d irq=%d\n",*id,irq);

    rmb();  
    thisCDD->move.one=inb(0x60); // left btn press 9. release 8, right btn press 10, release 8
    //  move.two=inb(0x61);
    //  move.three=inb(0x62);
    //  move.four=inb(0x63);
    //  move.five=inb(0x64);

    if  (initialize==0) {
        INIT_WORK(&thisCDD->CDD2_wk, got_movement);
        initialize=1;
    }
    else    
        PREPARE_WORK(&thisCDD->CDD2_wk, got_movement);

    queue_work(thisCDD->CDD2_wq,&thisCDD->CDD2_wk);
    return IRQ_HANDLED;
}


static int CDD_open (struct inode *inode, struct file *file)
{
        struct CDD2dev_struct *thisCDD=CDD2_dev;
     // struct CDD2dev_struct *thisCDD=
     // container_of(inode->i_cdev, struct CDD2dev_struct, CDD2_cdev);

		// only to maintain consistency with legacy, thus far
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
    struct CDD2dev_struct *thisCDD=file->private_data;

	if( thisCDD->CDD2_counter <= 0 ) return 0;

	err = copy_to_user(buf,thisCDD->CDD2_procvalue,thisCDD->CDD2_counter);

	if (err != 0) return -EFAULT;

	len = thisCDD->CDD2_counter;
	thisCDD->CDD2_counter = 0;
	return len;
}

static ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
{
	int err;
        struct CDD2dev_struct *thisCDD=file->private_data;

	memset(thisCDD->CDD2_procvalue,0,PROC_BUFLEN);
	err = copy_from_user(thisCDD->CDD2_procvalue,buf,count);
	if (err != 0) return -EFAULT;

	count=strlen(thisCDD->CDD2_procvalue); 
	thisCDD->CDD2_procvalue[count]=0;
	thisCDD->CDD2_counter += count;
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
	struct CDD2dev_struct *thisCDD=CDD2_dev;
	struct CDD2proc_struct *usrsp=thisCDD->CDD2_proc;


	if (*eof) { *eof=0; return 0; }
	else if (usrsp->CDD2_procflag) 
	{
		*eof = 1;
		usrsp->CDD2_procflag=0;
		return(sprintf(buf, "Hello..I got \"%s\"\n",thisCDD->CDD2_procvalue));
  }
  else
	{
		*eof = 1;
		return(sprintf(buf, "Hello from process %d\n", (int)current->pid));
	}
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
        struct CDD2dev_struct *thisCDD=CDD2_dev;

        length = (length<CDD_PROCLEN)? length:CDD_PROCLEN;

        if (copy_from_user(thisCDD->CDD2_procvalue, buf, length))
                return -EFAULT;

        thisCDD->CDD2_procvalue[length-1]=0;
        thisCDD->CDD2_proc->CDD2_procflag=1;
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
    struct CDD2dev_struct *thisCDD;
	unsigned long irqflags=0;

	CDDmajor = CDDparm;

	CDD2_dev=(struct CDD2dev_struct *)
			kmalloc(sizeof(struct CDD2dev_struct),GFP_KERNEL);

	thisCDD=CDD2_dev;

	// using thisCDD from this point on.
	// allocate memory
    thisCDD->CDD2_procvalue=kmalloc(PROC_BUFLEN,GFP_ATOMIC);
    thisCDD->CDD2_cdev=(struct cdev *)kmalloc(sizeof(struct cdev),GFP_ATOMIC);
    thisCDD->CDD2_proc=(struct CDD2proc_struct *)
			kmalloc(sizeof(struct CDD2proc_struct),GFP_ATOMIC);
    thisCDD->CDD2_sem=(struct semaphore *)kmalloc(sizeof(struct semaphore),GFP_ATOMIC);

    thisCDD->CDD2_wq=create_workqueue(QUEUE_NAME);

	// initialize
	thisCDD->CDD2_counter=0;

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
		
       	//  Step 2a of 2:  initialize thisCDD->CDD2_cdev struct
       	cdev_init(thisCDD->CDD2_cdev, &CDD_fops);

       	//  Step 2b of 2:  register device with kernel
       	thisCDD->CDD2_cdev->owner = THIS_MODULE;
       	thisCDD->CDD2_cdev->ops = &CDD_fops;
        i = cdev_add(thisCDD->CDD2_cdev, firstdevno, CDDNUMDEVS);
        if (i) { printk(KERN_ALERT "Error (%d) adding %s\n",i,CDD); return i;}

        // Create the necessary proc entries
        proc_mydev = proc_mkdir(MYDEV,0);

				#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  				proc_hello = proc_create("hello", 0777, proc_mydev, &proc_fops);
				#else
  				proc_hello = create_proc_entry(HELLO,0,proc_mydev);
  				proc_hello->read_proc = read_hello;
  				proc_hello->write_proc = write_hello;
				#endif

				#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
  				proc_hello->owner = THIS_MODULE;
				#endif

				#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
					irqflags=IRQF_SHARED;
				#else
					irqflags=SA_SHIRQ;
				#endif

	#ifdef CAN_REQUEST_IRQ
	// #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
		if (!can_request_irq(CDD_MOUSEIRQ, irqflags))
			free_irq(CDD_MOUSEIRQ,(void*)&dev_id);
	#endif

		// setup interrupt handler
		return request_irq(CDD_MOUSEIRQ,
			(irq_handler_t) irq_handler,
			irqflags,
			(char *)"test_mouse_irq_handler",
			(void*) &dev_id);

	return 0;
}

static void CDD_exit(void)
{
	unsigned long irqflags=0;

		// release resources in reverse order

        struct CDD2dev_struct *thisCDD=CDD2_dev;

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
		irqflags=IRQF_SHARED;
	#else
		irqflags=SA_SHIRQ;
	#endif

		// irq
 

  #ifdef CAN_REQUEST_IRQ
	// #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
		if (!can_request_irq(CDD_MOUSEIRQ, irqflags))
	#endif
			free_irq(CDD_MOUSEIRQ,&dev_id);

		// proc entry
        if (proc_hello) remove_proc_entry (CDD, proc_mydev);
        if (proc_mydev) remove_proc_entry (MYDEV, 0);

        //  Step 1 of 2:  unregister device with kernel
        cdev_del(thisCDD->CDD2_cdev);

        //  Step 2a of 2:  create/populate device numbers
        firstdevno = MKDEV(CDDmajor, CDDMINOR);

        //  Step 2b of 2:  Release request/reserve of Major Number from Kernel
        unregister_chrdev_region(firstdevno, CDDNUMDEVS);

		// workqueue
	    destroy_workqueue(thisCDD->CDD2_wq);

		kfree(thisCDD->CDD2_sem);
		kfree(thisCDD->CDD2_proc);
		kfree(thisCDD->CDD2_cdev);
		kfree(thisCDD->CDD2_procvalue);
		kfree(CDD2_dev);

	if (CDDmajor != CDDparm) 
		printk(KERN_ALERT "kernel unassigned major number: %d from CDD\n", CDDmajor);

}

module_init(CDD_init);
module_exit(CDD_exit);

