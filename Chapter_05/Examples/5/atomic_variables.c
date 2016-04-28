// Example# 5.5 .. simple *atomic variables* example (main.c)
// type: atomic_t; functions: atomic_inc, atomic_dec, atomic_read, atomic_set
//   main.c

// See Documentation/atomic_ops.txt
// See Documentation/volatile-considered-harmful.txt

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

#include <linux/sched.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define HELLO "hello"
#define MYDEV "MYDEV"

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char proc_hello_value[132];
	char proc_hello_flag;
	
	atomic_t proc_hello_open;
	atomic_t proc_hello_curcount;
	atomic_t proc_hello_reads;
	atomic_t proc_hello_writes;
	atomic_t proc_hello_counter;
};

static struct proc_hello_data hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

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
	struct proc_hello_data *usrsp=&hello_data;
	int n=0, length;

	if (*eof!=0) { *eof=0; return 0; }

	length = atomic_read(&usrsp->proc_hello_counter);
	atomic_inc(&usrsp->proc_hello_open);
	atomic_inc(&usrsp->proc_hello_reads);
	atomic_inc(&usrsp->proc_hello_curcount);

	if (offset) { n=0; }
	else if (usrsp->proc_hello_flag) {
		usrsp->proc_hello_flag=0;
		n=sprintf(buf, "Hello .. I got \"%s\"\n", 
				usrsp->proc_hello_value); 
	}
	else
		n=sprintf(buf, "Hello from process %d\n", 
				(int)current->pid);

	atomic_dec(&usrsp->proc_hello_curcount);

  *eof = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
    if(n)
      *ppos=length;
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
	int err=0;
	struct proc_hello_data *usrsp=&hello_data;

	atomic_inc(&usrsp->proc_hello_open);
	atomic_inc(&usrsp->proc_hello_writes);
	atomic_inc(&usrsp->proc_hello_curcount);

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	err=copy_from_user(usrsp->proc_hello_value, buf, length); 

	// check for copy_from_user error here 
	if (err) 	{
        	printk(KERN_ALERT "2470:5.5: return fault!\n");
		return -EFAULT;
	}

	usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_flag=1;

	atomic_dec(&usrsp->proc_hello_curcount);
	atomic_set(&usrsp->proc_hello_counter,length);

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

static int my_init (void) 
{

	atomic_set(&hello_data.proc_hello_counter,0);
	atomic_set(&hello_data.proc_hello_curcount,0);
	atomic_set(&hello_data.proc_hello_open,0);
	atomic_set(&hello_data.proc_hello_reads,0);
	atomic_set(&hello_data.proc_hello_writes,0);

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

	hello_data.proc_hello_flag=0;

        // module init message
        printk(KERN_ALERT "2470:5.5: main initialized!\n");
	return 0;
}

static void my_exit (void) {

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        printk(KERN_ALERT "2470:5.5: proc_hello_open=%d!\n", 
		atomic_read(&hello_data.proc_hello_open));
        printk(KERN_ALERT "2470:5.5: proc_hello_reads=%d!\n", 
		atomic_read(&hello_data.proc_hello_reads));
        printk(KERN_ALERT "2470:5.5: proc_hello_writes=%d!\n", 
		atomic_read(&hello_data.proc_hello_writes));

        // module exit message
        printk(KERN_ALERT "2470:5.5: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
