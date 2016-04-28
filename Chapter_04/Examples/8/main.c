// Example 4.8	:  using void *data;
//		:  main.c

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
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
	int  proc_hello_rcounter;
	int  proc_hello_wcounter;
};

// variable declaration is global in scope
static struct proc_hello_data hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int eof[1];

static int open_hello (struct inode *inode, struct file *file)
{
	file->private_data = (void *)&hello_data;
	return 0;
}
	
static ssize_t read_hello(struct file *file, char *buf,
size_t len, loff_t *ppos)
{
	struct proc_hello_data *usrsp=(struct proc_hello_data *)file->private_data;
#else
static int read_hello (char *buf, char **start, off_t offset, 
		int len, int *eof, void *data) 
{
	struct proc_hello_data *usrsp=(struct proc_hello_data *)data;
#endif
	
	// increment reader' counter
	usrsp->proc_hello_rcounter++;

	if (*eof!=0) { *eof=0; return 0; }
	else if (usrsp->proc_hello_flag) {
		usrsp->proc_hello_flag=0;
		snprintf(buf,len,
				"Hello .. I got \"%s\": (reads=%d .. writes=%d)\n", 
				usrsp->proc_hello_value,
				usrsp->proc_hello_rcounter,
				usrsp->proc_hello_wcounter
				); 
	}
	else
		snprintf(buf,len, "Hello from process %d\n", (int)current->pid);

	*eof = 1;
	return(strlen(buf));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int write_hello (struct file *file,const char * buf, 
    size_t count, loff_t *ppos) 
{
	struct proc_hello_data *usrsp=(struct proc_hello_data *)file->private_data;
#else
static int write_hello (struct file *file,const char * buf, 
		unsigned long count, void *data) 
{
	struct proc_hello_data *usrsp=(struct proc_hello_data *)data;
#endif
	int length=count;
	// (struct proc_hello_data *)proc_hello->data;

	// increment writer' counter
	usrsp->proc_hello_rcounter++;

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	if (copy_from_user(usrsp->proc_hello_value, buf, length)) 
		return -EFAULT;

	usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_flag=1;
	return(length);
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static const struct file_operations proc_fops = {
	// .owner = THIS_MODULE,
	.open  = open_hello,
	.read  = read_hello,
 	.write = write_hello,
 };
#endif

static int my_init (void) {

	proc_mydev = proc_mkdir(MYDEV,0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
  proc_hello = proc_create(HELLO, 0777, proc_mydev, &proc_fops);
#else
	proc_hello = create_proc_entry(HELLO,0,proc_mydev);
	proc_hello->read_proc = read_hello;
	proc_hello->write_proc = write_hello;

	// use the internal buffer here
	proc_hello->data = (void *)&hello_data;
#endif

	// initialize use flag/indicator
	hello_data.proc_hello_flag=0;

	// initialize reader/writer counter
	hello_data.proc_hello_rcounter=0;
	hello_data.proc_hello_wcounter=0;

    #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
        proc_hello->owner = THIS_MODULE;
    #endif      

	hello_data.proc_hello_flag=0;

        // module init message
        printk(KERN_ALERT "2470:4.8: main initialized!\n");
	return 0;
}

static void my_exit (void) {
	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

        // module exit message
        printk(KERN_ALERT "2470:4.8: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
