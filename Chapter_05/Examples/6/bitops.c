// Example# 5.6 .. simple *bitops* example (main.c)
//   bitops.c

// See Documentation/atomic_ops.txt

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

#include <asm/bitops.h>

#include <linux/sched.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define PROC_HELLO_BUFLEN 8192
#define HELLO "hello"
#define MYDEV "MYDEV"

#define BITPOS 1

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char *proc_hello_value;
	char proc_hello_flag;
	
	unsigned long proc_hello_counter;
	unsigned long proc_hello_spin;
	volatile unsigned long proc_hello_bitpos;
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
	int length=len;
	struct proc_hello_data *usrsp=&hello_data;
	int n=0;

  if (*eof!=0) { *eof=0; return 0; }

	length = (length<PROC_HELLO_BUFLEN)? length:PROC_HELLO_BUFLEN;
	length = (usrsp->proc_hello_counter<length)? usrsp->proc_hello_counter:length;
       	printk(KERN_ALERT "2470:5.6: in 'read' len='%d' before test_bit !\n", length);

	if (test_bit(BITPOS,&usrsp->proc_hello_bitpos)==0) {
		if (offset) { n=0; }
		else if (usrsp->proc_hello_flag) {
			usrsp->proc_hello_flag=0;
			n=sprintf(buf, "Hello .. I got \"%s\"\n", 
					usrsp->proc_hello_value); }
		else
			n=sprintf(buf, "Hello from process %d\n", 
					(int)current->pid);
	}

       	printk(KERN_ALERT "2470:5.6: '%d' in 'read' after test_bit !\n", length);

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
	int ret=0, err=0;
	struct proc_hello_data *usrsp=&hello_data;

	length = (length<PROC_HELLO_BUFLEN)? length:PROC_HELLO_BUFLEN;
	
       	printk(KERN_ALERT "2470:5.6: '%d' before test_bit, set_bit !\n", length);
	usrsp->proc_hello_spin=0;
	
	while ((ret=test_and_set_bit(BITPOS,&usrsp->proc_hello_bitpos)) != 0) {
       		printk(KERN_ALERT "2470:5.6: '%d' ('%d') from test_and_set_bit loop.!\n", ret, (int) usrsp->proc_hello_spin++);
		schedule();
	}

	err=copy_from_user(usrsp->proc_hello_value, buf, length); 

	// check for copy_from_user error here
	if (err) 
		return -EFAULT;

	// handle trailing nl char
	usrsp->proc_hello_value[length-1]=0;

	if(test_and_clear_bit(BITPOS,&usrsp->proc_hello_bitpos)==0) {
       		printk(KERN_ALERT "2470:5.6: '%d' from test_and_clear_bit loop.!\n", ret);
		return -EFAULT;
	}
	
  printk(KERN_ALERT "2470:5.6: after clear_bit!\n");

	usrsp->proc_hello_flag=1;
	usrsp->proc_hello_counter=length;
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

	hello_data.proc_hello_value=kmalloc(PROC_HELLO_BUFLEN,GFP_KERNEL);

	hello_data.proc_hello_flag=0;
	hello_data.proc_hello_bitpos=0;

        // module init message
        printk(KERN_ALERT "2470:5.6: main initialized!\n");
	return 0;
}

static void my_exit (void) {

	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:5.6: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
