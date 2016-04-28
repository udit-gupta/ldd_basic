// CDD.h
//
// Linux Device Drivers
// Enhanced Char Driver



#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include <linux/errno.h>
#include<linux/moduleparam.h>

#include<linux/fs.h>
#include<linux/mm.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>
#include<linux/semaphore.h>
#include<linux/spinlock.h>
#include<linux/ioctl.h>
#include<linux/proc_fs.h>
#include <linux/sched.h>                // for current->pid
#include <linux/poll.h>                 // for poll() functions and macros

#include<linux/slab.h>

#include<linux/pci.h>

//-----------------------------------------------------------

#define CDD "CDD"
#define CDDMAJOR 0
#define CDDMINOR 0
#define CDDNUMDEVS 4

#define CDD1 16
#define CDD2 64
#define CDD3 128
#define CDD4 256

#define CDD1_NAME "CDD16"
#define CDD2_NAME "CDD64"
#define CDD3_NAME "CDD128"
#define CDD4_NAME "CDD256"

#define PCI "pci"

#define NUM_OPEN 1
#define BUF_ALLOC 2
#define BUF_USED 3

#define BUF_LEN 128

#define PROC_BUFFER 300

//-----------------------------------------------------------

MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");

//-----------------------------------------------------------

struct CDDdev_struct {
   unsigned buf_len;
   unsigned int CDD_eof;
   unsigned int CDD_fpos;
   unsigned int open_count;	//number of times open() has been called
   char *buffer;	//size depends on minor number
   struct cdev CDD_cdev;
   int CDD_devno;
   struct semaphore CDD_sem;	//semaphore for reads/writes
   struct semaphore open_block;
   spinlock_t CDD_lock;
};

//-----------------------------------------------------------

//from chardd.c
int cdd_init(void);
void cdd_exit(void);

//from cdd_methods.c
int cdd_open(struct inode *inode, struct file *file);
int cdd_release(struct inode *inode, struct file *file);
int cdd_read(struct file *file, char *buf, size_t count, loff_t *ppos);
int cdd_write(struct file *file, const char *buf, size_t count, loff_t *ppos);
int cdd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int cdd_llseek(struct file *file, loff_t offset, int whence);

//from cdd_proc.c
int CDD_proc_create(void);
int CDD_proc_destroy(void);
