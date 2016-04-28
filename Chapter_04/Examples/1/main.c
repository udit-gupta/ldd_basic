// Example 4.1:  to demo reads from /proc file
//    		 main.c

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

#include <linux/sched.h>

MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_hello;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int eof[1];

static ssize_t read_hello(struct file *file, char *buf, 
size_t len, loff_t *ppos)
#else
static int read_hello (char *buf, char **start, off_t offset, 
			int len, int *eof, void *unused) 
#endif
{
	if (*eof) { *eof=0; return 0; }

	snprintf(buf,len,"HELLO to you .. PID %d HZ=%d\n", 
      (int)current->pid, HZ);
	len=strlen(buf);
	*eof = (int)current->pid;
	return(len);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static const struct file_operations proc_fops = {
 // .owner = THIS_MODULE,
 .read  = read_hello,
};
#endif 


static int my_init (void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
// proc_create(const char *name, umode_t mode, 
// struct proc_dir_entry *parent, const struct file_operations *proc_fops)
	proc_hello = proc_create("hello", 0777, NULL, &proc_fops);
#else  
	proc_hello = create_proc_entry("hello",0,0);
	proc_hello->read_proc = read_hello;
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
	proc_hello->owner = THIS_MODULE;
#endif      

  if(proc_hello == NULL)
   return -ENOMEM;

	return 0;
}

static void my_exit (void) {
	if (proc_hello)
		remove_proc_entry ("hello", 0);
}

module_init (my_init);
module_exit (my_exit);
