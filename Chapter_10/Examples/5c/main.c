// Example 10.5c	:  workqueue
//		:  main.c
//  

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>

#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/sched.h>
#include <linux/workqueue.h> // workqueue

#include <linux/slab.h>  // since 2.6.29
// see http://lwn.net/Articles/319686/

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */

/*

from <linux/kernel.h>

#define container_of(ptr, type, member) ({          \
 const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
 (type *)( (char *)__mptr - offsetof(type,member) );})

from <linux/stddefs.h>

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

*/

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 1024
#define HELLO "hello"
#define MYDEV "MYDEV"

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
	struct workqueue_struct *proc_hello_wkq;
	struct work_struct *rq;
};

static void write_hello_wk(struct work_struct *work);
static struct proc_hello_data *hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

static void write_hello_wk(struct work_struct *work) {
	struct proc_hello_data *m=hello_data;

	if (m->rq==work)  {
	
		printk("Printing from workqueue .. I got \"%s\" at jiffies=%ld\n", 
	   m->proc_hello_value, jiffies); 

	}
	else {
		printk("Unexpected Address Mismatch: Bailing Out => Doing Nothing!\n");
	}
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


	if (*eof) { n=0; }
	else if (usrsp->proc_hello_flag) {
	  *eof = 1;
		usrsp->proc_hello_flag=0;
		n=sprintf(buf,"See /var/log/messages.\n");
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
	usrsp->proc_hello_flag=1;

    queue_work(usrsp->proc_hello_wkq, usrsp->rq);

    printk(KERN_ALERT "2470:10.5c: workqueue scheduled!\n");
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

	hello_data=(struct proc_hello_data *)
		kmalloc(sizeof(*hello_data),GFP_KERNEL);

	hello_data->proc_hello_value=(char *)
		kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);

	// no need for a separate memory allocation for wkq.
	// create_workqueue sets it up
	// hello_data->proc_hello_wkq=(struct workqueue_struct *)
		// kmalloc(sizeof(struct workqueue_struct),GFP_KERNEL);
	hello_data->proc_hello_wkq=create_workqueue(HELLO);
	
	hello_data->rq=(struct work_struct *)
	    kzalloc(sizeof(struct work_struct),GFP_KERNEL);

	hello_data->proc_hello_flag=0;

	// PREPARE_WORK(hello_data->proc_hello_wkq, write_hello_wkq);
	// INIT_WORK anyways calls PREPARE_WORK internally
	INIT_WORK(hello_data->rq, write_hello_wk);

    // module init message
    printk(KERN_ALERT "2470:10.5c: main initialized!\n");
    printk(KERN_ALERT "2470:10.5c: memory allocated(hello_data) = %0x(%d bytes)!\n", (int) hello_data, ksize(hello_data));
    printk(KERN_ALERT "2470:10.5c: memory allocated(hello_data->rq) = %0x(%d bytes)!\n", (int) &hello_data->rq, ksize(hello_data->rq));
    printk(KERN_ALERT "2470:10.5c: memory allocated(hello_data->proc_hello_value) = %d!\n", ksize(hello_data->proc_hello_value));

	return 0;
}

static void my_exit (void) {
	destroy_workqueue(hello_data->proc_hello_wkq);

	kfree(hello_data->rq);
	// kfree(hello_data->proc_hello_wkq);
	kfree(hello_data->proc_hello_value);
	kfree(hello_data);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:10.5c: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
