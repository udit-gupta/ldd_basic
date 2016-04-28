// Example# 10.7 .. simple *completion* example (main.c)
//   main.c

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
// #include <linux/utsrelease.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/completion.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 1024
#define HELLO "hello"
#define MYDEV "MYDEV"

#define init_MUTEX(sem)         sema_init(sem, 1)
#define init_MUTEX_LOCKED(sem)  sema_init(sem, 0)

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
	int  proc_hello_length;
	
	struct semaphore *proc_hello_sem;
	struct completion *proc_hello_completion;
	struct completion *proc_hello_wrcompletion;
};

static char *devbuf; 	// internal/simulation of device
static struct proc_hello_data hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;


static void transfer_hello (int write) {
    struct proc_hello_data *m=&hello_data;

    if(write) {

		//
		// doing the write transfer here.
		// => transfer from driver-buffer to device.
		//

		// memset(devbuf,0,m->proc_hello_length+2); // initialize devbuf
	
		// simulate the transfer
       	memcpy(devbuf,m->proc_hello_value,m->proc_hello_length);
		printk("(write-stage) workqueue .. value=\"%s\",\"%s\",%i\n",
			m->proc_hello_value, devbuf,m->proc_hello_length);

		// reset m->proc_hello_value
		memset(m->proc_hello_value,0,m->proc_hello_length+2);

		m->proc_hello_flag=0;
		complete(m->proc_hello_wrcompletion);
     }
     else  {

		//
		// doing the read transfer here.
		// => transfer from device to driver-buffer.
		//

		// initialize  m->proc_hello_value
		memset(m->proc_hello_value,0,m->proc_hello_length+2);

		memcpy(m->proc_hello_value,devbuf,m->proc_hello_length);
		printk("(read-stage) workqueue .. value=\"%s\"\n",
			m->proc_hello_value); //

		// reset devbuf
		// memset(devbuf,0,PROC_HELLO_BUFLEN);
		m->proc_hello_flag=1;
    }
	// signal completion
	complete(m->proc_hello_completion);
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
	struct proc_hello_data *usrsp=&hello_data;


	printk(KERN_ALERT "2470:10.7: process %i (%s) about to sleep\n",
		current->pid, current->comm);

	wait_for_completion(hello_data.proc_hello_wrcompletion);

	transfer_hello(0);

	wait_for_completion(hello_data.proc_hello_completion);

	printk(KERN_ALERT "2470:10.7: process %i (%s) is now awake\n",
		current->pid, current->comm);

	if (*eof) { *eof=0; return 0; }
	else if (usrsp->proc_hello_flag) {
	  *eof = 1;
		usrsp->proc_hello_flag=0;
		return(sprintf(buf,
				"Hello .. I got \"%s\"\n", 
				usrsp->proc_hello_value)); 
	}
	else
	{
	  *eof = 1;
		return(sprintf(buf,
				"Hello from process %d\n", 
				(int)current->pid));
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
	int err=0;
	struct proc_hello_data *usrsp=&hello_data;

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;
	printk(KERN_ALERT "2470:10.7: process %i (%s) got length=%i\n",
		current->pid, current->comm, length);

    // semaphore decr
    if (down_interruptible(hello_data.proc_hello_sem))
    	return -ERESTARTSYS;

	err=copy_from_user(usrsp->proc_hello_value, buf, length); 

    // semaphore incr
    up(hello_data.proc_hello_sem);

	// check for copy_from_user error (upon sem release)
	if (err)  
		return -EFAULT;

	usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_length=length;
	usrsp->proc_hello_flag=1;

	transfer_hello(1);

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

	devbuf=(char *)
		kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);
	hello_data.proc_hello_value=(char *) 
		kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);
	hello_data.proc_hello_sem=(struct semaphore *) 
		kmalloc(sizeof(struct semaphore),GFP_KERNEL);
	hello_data.proc_hello_completion=(struct completion *) 
		kmalloc(sizeof(struct completion),GFP_KERNEL);
	hello_data.proc_hello_wrcompletion=(struct completion *) 
		kmalloc(sizeof(struct completion),GFP_KERNEL);

	init_MUTEX(hello_data.proc_hello_sem);
	init_completion(hello_data.proc_hello_completion);
	init_completion(hello_data.proc_hello_wrcompletion);

	hello_data.proc_hello_flag=0;

        // module init message
        printk(KERN_ALERT "2470:10.7: main initialized!\n");
	return 0;
}

static void my_exit (void) {

	kfree(hello_data.proc_hello_sem);
	kfree(hello_data.proc_hello_wrcompletion);
	kfree(hello_data.proc_hello_completion);
	kfree(hello_data.proc_hello_value);
	kfree(devbuf);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:10.7: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
