//  Example 7.4: main.c .. Simple standalone program

//  busy waiting using jiffies .. time_before()
//  rdtsc()
//  cpu_khz

//  cpufreq-info -f -m
//  cpuspeed -C

//
#define  thisDELAY	5	// lock the system (#seconds)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/sched.h>	// jiffies

#include <asm/msr.h>		// machine-specific registers;rdtsc()
#include <linux/delay.h>	// mdelay();udelay();

MODULE_LICENSE("GPL");

#define PROC_HELLO_LEN 8
#define HELLO "hello"
#define MYDEV "MYDEV"

// from /proc/cpuinfo   
#define MYCPUMHZ (cpu_khz/1000)

struct proc_hello_data {
	char proc_hello_name[PROC_HELLO_LEN + 1];
	char proc_hello_value[132];
	char proc_hello_flag;
};

static struct proc_hello_data hello_data;

static struct proc_dir_entry *proc_hello;
static struct proc_dir_entry *proc_mydev;

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
	volatile unsigned long long startll, endll; 
	volatile unsigned long long elapsedll;
	volatile unsigned long long diffll; // need for 64bit div on 32bit arch
	unsigned long long secll;
	int buflen=0;
	long long startjiffies,endjiffies;

	secll=0;

 	set_user_nice(current, -19);


  if (*eof!=0) { *eof=0; return 0; }
	else if (usrsp->proc_hello_flag) {
		usrsp->proc_hello_flag=0;
		buflen=sprintf(buf, "Hello .. I got \"%s\"\n", 
				usrsp->proc_hello_value); 
	}
	else {
		startjiffies=jiffies;
		endjiffies = jiffies + thisDELAY * HZ;
	
		rdtscll(startll); // e.g. long long i.e. 64-bit implementation
		while (time_before(jiffies, (unsigned long)endjiffies)) {
			// doing nothing here ..
		}
		rdtscll(endll);	  // e.g. long long i.e. 64-bit implementation
	
		diffll=elapsedll=endll-startll;

		//do_div replaces val with quotient - needed for 64bit div on 32bit arch
		do_div(diffll,(cpu_khz*1024*1024)); 

		// instead of above, try this and see if you can explain the result
		// do_div(diffll,(cpu_khz*1024)); 

		elapsedll=(elapsedll==0)?1:elapsedll;

		buflen=sprintf(buf,"Hello from "
		"process %d\n"
		"startjiffies=%ld, jiffies=%ld, diff=%ldms\n"
		"start=%lld,end=%lld, diff=%lld (cpu ticks) (==%lu sec)"
		"\ncpu_khz=%d (%d MHz)\n",
		(int)current->pid,
		(long int)startjiffies,
    (long int)jiffies,
		(long int)(jiffies-startjiffies),
		startll,
		endll,
		elapsedll,
		(unsigned long int) diffll,
		(int) cpu_khz,
		cpu_khz/(1024));
	}

	*eof = 1;
	return buflen;
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
	struct proc_hello_data *usrsp=&hello_data;

	length = (length<PROC_HELLO_LEN)? length:PROC_HELLO_LEN;

	if (copy_from_user(usrsp->proc_hello_value, buf, length)) 
		return -EFAULT;

	usrsp->proc_hello_value[length-1]=0;
	usrsp->proc_hello_flag=1;
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

	hello_data.proc_hello_flag=0;

  // module init message
  printk(KERN_ALERT "Example:7.5: main initialized!\n");
	return 0;
}

static void my_exit (void) {
	if (proc_hello)
		remove_proc_entry (HELLO, proc_mydev);
	if (proc_mydev)
		remove_proc_entry (MYDEV, 0);

  // module exit message
  printk(KERN_ALERT "Example:7.5: main destroyed!\n");
}

module_init (my_init);
module_exit (my_exit);
