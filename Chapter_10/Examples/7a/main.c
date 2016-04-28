// Example 10.7a	:  workqueue
//		:  main.c
//  

#define thisIRQ 12
//	#define thisIRQ 12				// psmouse IRQ

// select or non-shared IRQ handler
// #define NONSHARED_IRQHANDLER
#define SHARED_IRQHANDLER

// #define thisDELAY 50
// delay of 3-sec
#define thisDELAY 3*HZ


#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/sched.h>

#include <linux/wait.h> 	 // wait queue
#include <linux/workqueue.h> // workqueue

#include <linux/irq.h>		 // irq defs
#include <linux/interrupt.h>

#include <linux/slab.h>  // since 2.6.29
// see http://lwn.net/Articles/319686/

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 1024
#define HELLO "hello"
#define MYDEV "MYDEV"

#define init_MUTEX(sem)         sema_init(sem, 1)
#define init_MUTEX_LOCKED(sem)  sema_init(sem, 0)

#ifdef NONSHARED_IRQHANDLER
		#define MYIRQNUMBER thisIRQ
		#define MYIRQFLAGS 0		// not shared 
#endif

#ifdef SHARED_IRQHANDLER
	#define MYIRQNUMBER thisIRQ
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
		#define MYIRQFLAGS IRQF_SHARED	// shared 
	#else
		#define MYIRQFLAGS SA_SHIRQ // shared */
	#endif
#endif

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


struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;				// device internal buffer
	unsigned int proc_hello_length;		// device buffer used-length
	char proc_hello_flag;				// flag 
	// work & workq related entries
	struct workqueue_struct *proc_hello_wkq;
	struct delayed_work *wk;
	// waitq related entries
    wait_queue_head_t *proc_hello_wqh;
    wait_queue_t *proc_hello_wq;
	// datadir for transfer
	char datadir; // 0/1 represents read/write (similar to stdin/out)
};


/*  just for quick reference
struct delayed_work {
    struct work_struct work;
    struct timer_list timer;
};
*/

unsigned long irqflags=0;
int dev_id = 128;
char *devbuf;		// device representation

static void hello_work_proc(struct work_struct *work);
static void wait_hello(void);
static struct proc_hello_data *hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

// here is where we setup the interrupt handler
irqreturn_t irq_handler(int irq, void *dev_id,struct pt_regs *regs)
{
	static int initialized=0;
    struct proc_hello_data *usrsp=hello_data;

    int *id=(int *)dev_id;

    if (*id!=128) {
		printk(KERN_INFO "interrupt id=%d irq=%d\n",*id,irq);
        return IRQ_HANDLED;
	}
	
    mb();

	// INIT_DELAYED_WORK anyways calls INIT_WORK internally
	if (!initialized++) 
		INIT_DELAYED_WORK(hello_data->wk, hello_work_proc); 
	else
		PREPARE_DELAYED_WORK(hello_data->wk, hello_work_proc); 

	if (! delayed_work_pending(usrsp->wk)) {
    	queue_delayed_work(usrsp->proc_hello_wkq, usrsp->wk, thisDELAY);
    	printk(KERN_ALERT "2470:10.7a: workqueue scheduled!\n");
	}
    return IRQ_HANDLED;
}

// here is where we do the work of doing the IO transfer
static void hello_work_proc(struct work_struct *work) {
	struct proc_hello_data *m=hello_data;

	if (&m->wk->work==work)  {
			
		if(m->datadir && m->proc_hello_flag) {

			//
			// doing the write transfer here.
			// => transfer from driver-buffer to device.
			//

			memcpy(devbuf,m->proc_hello_value,m->proc_hello_length);
			printk("(write-stage) workqueue .. value=\"%s\"\n", 
	   			m->proc_hello_value); 
			memset(m->proc_hello_value,0,m->proc_hello_length+2);
			m->proc_hello_flag=0;
		}
		else	{

			//
			// doing the read transfer here.
			// => transfer from device to driver-buffer.
			//

			memcpy(m->proc_hello_value,devbuf,m->proc_hello_length);
			printk("(read-stage) workqueue .. value=\"%s\"\n", 
	   			m->proc_hello_value); // 
			memset(devbuf,0,PROC_HELLO_BUFLEN);
			m->proc_hello_flag=0;
		}
	}
	else {
		printk("Unexpected Address Mismatch: Bailing Out => Doing Nothing!\n");

		
	}
}
	
static void wait_hello(void) {
	struct proc_hello_data *usrsp=hello_data;

    prepare_to_wait_exclusive(usrsp->proc_hello_wqh,
        usrsp->proc_hello_wq, TASK_INTERRUPTIBLE);

	add_wait_queue_exclusive(usrsp->proc_hello_wqh,
        usrsp->proc_hello_wq);

    while (usrsp->proc_hello_flag!=1) {
        printk(KERN_ALERT "2470:10.5a process(%d \"%s\") "
            "calling schedule ..!\n", current->pid, current->comm);
        schedule();
        printk(KERN_ALERT "2470:10.5a process(%d \"%s\") "
            "return from schedule ..!\n", current->pid, current->comm);
    }

    // process awake message
    printk(KERN_ALERT "2470:10.5a process(%d \"%s\") "
            "finished waiting ..!\n", current->pid, current->comm);

    finish_wait(usrsp->proc_hello_wqh,usrsp->proc_hello_wq);
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int eof[1];

static ssize_t read_hello(struct file *file, char *buf,
size_t len, loff_t *ppos)
#else
static int read_hello (char *buf, char **start, off_t offset,
    int len, int *eof, void *unused)
#endif
{
	struct proc_hello_data *usrsp=hello_data;
	int n=0;

	usrsp->datadir=0;

	if (*eof) { n=0; }
	else if (usrsp->proc_hello_flag) {
		*eof = 1;

		while (!usrsp->proc_hello_flag)
			wait_hello();

		n=sprintf(buf,"Hello I got \"%s\"\n", devbuf);
		usrsp->proc_hello_flag=0;
		usrsp->proc_hello_length=0;
		kfree(devbuf);
	}
	else
	{
		*eof = 1;
		n=sprintf(buf, "Hello from process %d\njiffies=%ld\n", 
				(int)current->pid,jiffies);
	}	
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
	struct proc_hello_data *usrsp=hello_data;

	length = (length<PROC_HELLO_BUFLEN)? length:PROC_HELLO_BUFLEN;

	if (copy_from_user(usrsp->proc_hello_value, buf, length)) 
		return -EFAULT;

    usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_length=length;
	usrsp->datadir=1;

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

static int my_init (void) {

	int rc=0;
	//
	// procfs related entries
	//
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

	//
	// memory initialization
	//
	hello_data=(struct proc_hello_data *)
		kmalloc(sizeof(*hello_data),GFP_KERNEL);

	hello_data->proc_hello_value=(char *)
		kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);

    hello_data->proc_hello_wqh=(wait_queue_head_t *)
        kmalloc(sizeof(wait_queue_head_t),GFP_KERNEL);

    hello_data->proc_hello_wq=(wait_queue_t *)
        kmalloc(sizeof(wait_queue_t),GFP_KERNEL);

	hello_data->wk=(struct delayed_work *)
	    kzalloc(sizeof(struct delayed_work),GFP_KERNEL);

	hello_data->proc_hello_flag=0;

	devbuf=(char *) kzalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);

	//
	// waitqueue related entries
    // => Also, what happens when the following lines is commented out
    //
    init_waitqueue_head(hello_data->proc_hello_wqh);
    init_wait(hello_data->proc_hello_wq);

	//
	// create workqueue
	// => no need for a separate memory allocation for wkq.
	// => create_workqueue sets it up
	    // hello_data->proc_hello_wkq=(struct workqueue_struct *)
		// kmalloc(sizeof(struct workqueue_struct),GFP_KERNEL);
	//
	hello_data->proc_hello_wkq=create_workqueue(HELLO);
	

	//
	// setup IRQ handler
	//
    irqflags=MYIRQFLAGS;

    // #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
    //  if (!can_request_irq(MYIRQNUMBER, irqflags))
    //       free_irq(MYIRQNUMBER,(void*)&dev_id);
    // #endif

        // setup interrupt handler
 	if ((rc=(request_irq(MYIRQNUMBER,
        	(irq_handler_t) irq_handler,
       		irqflags,
       		(char *)"MYDEV_irq_handler",
       		(void*) &dev_id)))<0) 
	{
   		printk(KERN_ALERT "2470:10.7a: intr not initialized!(RC=%d)\n",rc);
			goto err;
	}
	else {
   		printk(KERN_ALERT "2470:10.7a: intr initialized!\n");
	}

    // finally .. module init messages.
    printk(KERN_ALERT "2470:10.7a: main initialized!\n");
    printk(KERN_ALERT "2470:10.7a: memory allocated(hello_data) = %0x(%d bytes)!\n", (int) hello_data, ksize(hello_data));
    printk(KERN_ALERT "2470:10.7a: memory allocated(hello_data->wk) = %0x(%d bytes)!\n", (int) &hello_data->wk, ksize(hello_data->wk));
    printk(KERN_ALERT "2470:10.7a: memory allocated(hello_data->proc_hello_value) = %d!\n", ksize(hello_data->proc_hello_value));

	return 0;

err:
	//
	// teardown work
	//
	while (delayed_work_pending(hello_data->wk)) {
		cancel_delayed_work_sync(hello_data->wk);
        printk(KERN_ALERT "2470:10.7a: Cancelling delayed work!\n");
	}

	//
	// teardown workqueue
	//
	destroy_workqueue(hello_data->proc_hello_wkq);

	kfree(devbuf);

	kfree(hello_data->wk);

    kfree(hello_data->proc_hello_wq);
    kfree(hello_data->proc_hello_wqh);

	kfree(hello_data->proc_hello_value);
	kfree(hello_data);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

    // module exit message
    printk(KERN_ALERT "2470:10.7a: main bailing out!\n");

	return rc;
}

static void my_exit (void) {

	// teardown
		
	//
    // teardown irq
	//
/*
    #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
        if (!can_request_irq(MYIRQNUMBER, irqflags))
    #endif
*/
	free_irq(MYIRQNUMBER,&dev_id);

	//
	// teardown work
	//
	while (delayed_work_pending(hello_data->wk)) {
		cancel_delayed_work_sync(hello_data->wk);
        printk(KERN_ALERT "2470:10.7a: Cancelling delayed work!\n");
	}

	//
	// teardown workqueue
	//
	destroy_workqueue(hello_data->proc_hello_wkq);

	kfree(devbuf);

	kfree(hello_data->wk);

    kfree(hello_data->proc_hello_wq);
    kfree(hello_data->proc_hello_wqh);

	kfree(hello_data->proc_hello_value);
	kfree(hello_data);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

    // module exit message
    printk(KERN_ALERT "2470:10.7a: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
