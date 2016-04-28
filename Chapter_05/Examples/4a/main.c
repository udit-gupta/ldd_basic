// Example# 5.4 .. simple *kfifo* example (main.c)
//   main.c

// init:
// 	kfifo_alloc

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <generated/utsrelease.h>
#else
#include <linux/utsrelease.h>
#endif

#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kfifo.h>

#include <linux/sched.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_KFIFOLEN 64
#define HELLO "hello"
#define MYDEV "MYDEV"

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
	struct kfifo *proc_hello_kfifo;
#else
	struct kfifo proc_hello_kfifo;
#endif
	spinlock_t proc_hello_sp;
};

static struct proc_hello_data hello_data;
static struct proc_dir_entry *proc_hello, *proc_mydev;


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

	int length=len,kfifolen=0;
	struct proc_hello_data *usrsp=&hello_data;
	int ret=0;

  if (*eof!=0) { *eof=0; return 0; }

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	kfifolen = kfifo_len(&(hello_data.proc_hello_kfifo));
	printk(KERN_INFO "queue len: %u\n", kfifolen);

	if (length) 
	{
		memset(usrsp->proc_hello_value,0,PROC_HELLO_LEN);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
		ret=kfifo_get(usrsp->proc_hello_kfifo, usrsp->proc_hello_value,length);
#else
		ret=kfifo_get(&(usrsp->proc_hello_kfifo), usrsp->proc_hello_value); 
#endif

		if (offset) { ret=0; }
		else if (usrsp->proc_hello_flag) {
			usrsp->proc_hello_flag=0;
			ret=sprintf(buf, "Hello .. I got \"%s\"\n", usrsp->proc_hello_value); 
		}
		else
			ret=sprintf(buf, "Hello from process %d\n", (int)current->pid);
	}
	
  printk(KERN_ALERT "2470:5.4: '%d' read from fifobuf!\n", length);
	*eof = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		if(ret)
			*ppos=length;
#endif
	return ret;
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
	int ret=0, err=0;
	struct proc_hello_data *usrsp=&hello_data;

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

  err = kfifo_from_user(&(hello_data.proc_hello_kfifo),buf,count,&length);

	// check for copy_from_user error here
	if (err) 
		return -EFAULT;

	memset(usrsp->proc_hello_value,0,PROC_HELLO_LEN);
	ret=usrsp->proc_hello_flag=1;
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

	hello_data.proc_hello_value=kmalloc(PROC_HELLO_LEN,GFP_KERNEL);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
   hello_data.proc_hello_sp=SPIN_LOCK_UNLOCKED;
#else
	 spin_lock_init(&hello_data.proc_hello_sp);
#endif

	// initialize and allocate kfifo
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
	hello_data.proc_hello_kfifo	=
		kfifo_alloc(PROC_HELLO_KFIFOLEN, GFP_KERNEL,
				&(hello_data.proc_hello_sp)); // spinlock
#else
	rc=kfifo_alloc(&(hello_data.proc_hello_kfifo),PROC_HELLO_KFIFOLEN,
				GFP_KERNEL);
	if (rc!=0) {
		if (proc_hello) remove_proc_entry (HELLO, proc_mydev);
		if (proc_mydev) remove_proc_entry (MYDEV, 0);
		return -EFAULT;
	}
#endif
			
	hello_data.proc_hello_flag=0;

	// module init message
	printk(KERN_ALERT "2470:5.4: main initialized!\n");
	return 0;
}

static void my_exit (void) {

	kfifo_reset(&(hello_data.proc_hello_kfifo)); // redundant ?
	kfifo_free(&(hello_data.proc_hello_kfifo));

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:5.4: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
