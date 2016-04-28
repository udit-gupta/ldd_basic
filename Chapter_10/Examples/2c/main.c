// Example 10.2c	:  waitqueues
//		:  main.c

#define thisDELAY 3
#define ACK_TIMEOUT msecs_to_jiffies(thisDELAY)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>

#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 1024
#define HELLO "hello"
#define MYDEV "MYDEV"

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
	
	wait_queue_head_t *proc_hello_wqh;
};

static struct proc_hello_data *hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

int j;

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

	*eof = 1;
	
	j=jiffies;

	// can also do a lockless check if the waitqueue is 
	// currently active
	// if (waitqueue_active(usrsp->proc_hello_wqh)) {

   		// process asleep message
       	printk(KERN_ALERT "2470:10.2c process(%d \"%s\") "
			"going to sleep ..!\n", current->pid, current->comm);

	 	wait_event_interruptible_timeout(*(usrsp->proc_hello_wqh),
			usrsp->proc_hello_flag != 0, ACK_TIMEOUT);

   		// process awake message
   		printk(KERN_ALERT "2470:10.2c process(%d \"%s\") "
			"awoken ..!\n", current->pid, current->comm);
	// }

  	// process awake message
	printk(KERN_ALERT "2470:10.2c process(%d \"%s\") "
		"is awake ..!\n", current->pid, current->comm);

	usrsp->proc_hello_flag=0;

	n=sprintf(buf, "Hello .. I got \"%s\"\n", 
				usrsp->proc_hello_value); 

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

	// j=jiffies;

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	if (copy_from_user(usrsp->proc_hello_value, buf, length)) 
		return -EFAULT;

	usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_flag=1;

    // print awakening message
    printk(KERN_ALERT "2470:10.2c: awakening readers!\n");
	wake_up_interruptible(usrsp->proc_hello_wqh);
		
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
	
	hello_data->proc_hello_wqh=(wait_queue_head_t *)
		kmalloc(sizeof(wait_queue_head_t),GFP_KERNEL);
	
	// see what happens when the following line is commented out
	// 
	init_waitqueue_head(hello_data->proc_hello_wqh);

	hello_data->proc_hello_flag=0;

    // module init message
    printk(KERN_ALERT "2470:10.2c: main initialized!\n");
	return 0;
}

static void my_exit (void) {
	kfree(hello_data->proc_hello_wqh);
	kfree(hello_data->proc_hello_value);
	kfree(hello_data);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:10.2c: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
