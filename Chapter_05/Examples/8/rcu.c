// Example# 5.8 .. simple *rcu* example (main.c)
//   rcu.c

// www.cs.pdx.edu/~walpole/class/cs510/spring2006/slides/15outline.pdf

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 2048
#define HELLO "hello"
#define MYDEV "MYDEV"

#define BITPOS 1

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
	
	spinlock_t proc_hello_sp;
	unsigned long proc_hello_counter;
};

static struct proc_hello_data *hello_data;
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
	int length=len;
	int n=0;

  if (*eof!=0) { *eof=0; return 0; }

	length = (length<PROC_HELLO_BUFLEN)? length:PROC_HELLO_BUFLEN;
	length = (hello_data->proc_hello_counter<length)? hello_data->proc_hello_counter:length;

	rcu_read_lock();
       	printk(KERN_ALERT "2470:5.8: in 'read' len='%d' rcu_read_lock!\n", length);

	// do something useful here
	if (offset) { n=0; }
	else if (hello_data->proc_hello_flag) {
		hello_data->proc_hello_flag=0;
		n=sprintf(buf, "Hello .. I got \"%s\"\n", 
			rcu_dereference(hello_data->proc_hello_value)); 
	}
	else
		n=sprintf(buf, "Hello from process %d\n", 
				(int)current->pid);

       	printk(KERN_ALERT "2470:5.8: '%d' in 'read' after rcu deref !\n", length);

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
	static struct proc_hello_data *old, *t;
	char *tmpbuf;

	tmpbuf=kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);
	t=kmalloc(sizeof(*hello_data),GFP_KERNEL);

	length = (length<PROC_HELLO_BUFLEN)? length:PROC_HELLO_BUFLEN;
	
	err=copy_from_user(tmpbuf, buf, length); 
       	printk(KERN_ALERT "2470:5.8: after copy_from_user!\n");

	// check for copy_from_user error here
	if (err) 
		return -EFAULT;

	// handle trailing nl char
	tmpbuf[length-1]=0;

	t->proc_hello_flag=1;
	t->proc_hello_counter=length;
	t->proc_hello_value=tmpbuf;

       	printk(KERN_ALERT "2470:5.8: '%d' before rcu spinlock\n", length);
	spin_lock(&hello_data->proc_hello_sp);
	
       	printk(KERN_ALERT "2470:5.8: got rcu\n");

	old=hello_data;
	rcu_assign_pointer(hello_data, t);
	spin_unlock(&hello_data->proc_hello_sp);

	synchronize_rcu();
       	printk(KERN_ALERT "2470:5.8: synchronize rcu\n");

	kfree(old->proc_hello_value);

	kfree(old);

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

	hello_data = kmalloc(sizeof(*hello_data),GFP_KERNEL);

	hello_data->proc_hello_value=kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);

	hello_data->proc_hello_flag=0;

        // module init message
        printk(KERN_ALERT "2470:5.8: main initialized!\n");
	return 0;
}

static void my_exit (void) {

	kfree(hello_data->proc_hello_value);

	kfree(hello_data);

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:5.8: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
