// Example# 3.1b:  Simple Char Driver with Statically allocated Major#  
//                works only in 2.6  (Similar .. to Example# 3.1a)


//	using dev_t, struct cdev (2.6)

#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <generated/utsrelease.h>
#else
#include <linux/utsrelease.h>
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>		// 2.6
#include <asm/uaccess.h>
#include <linux/delay.h>		// 2.6

MODULE_LICENSE("GPL");   	//  Kernel isn't tainted .. but doesn't 
			 	//  it doesn't matter for SUSE anyways :-(  

#define CDD		"CDD2"

#define CDDMAJOR	32
#define CDDMINOR	0	// 2.6
#define CDDNUMDEVS	6	// 2.6

#define ERRH(err)	\
	if (err<0) 	\
	{	\
		printk(KERN_ERR "%s - USB Error:%d\n", __func__,(int) err); \
		return err;	\
	} 

extern int registerUSBforCDD(void);
extern int openUSBforCDD(int, struct file *);
extern int closeUSBforCDD(int, struct file *);
extern int readCDDfromUSB(int,size_t);
extern int writeCDDtoUSB(int,size_t);
extern int unregisterUSBforCDD(void);
extern unsigned char *USBbuffer;
extern struct file_operations CDD_fops;

extern char *CDD_Buffer;
unsigned int counter = 0;
struct cdev cdev;
dev_t  devno;

#define BUFSZ 4096

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
char CDD_buf[BUFSZ];
char *CDD_Buffer=CDD_buf;
#else
char CDD_Buffer[BUFSZ];
#endif


static int CDD_open (struct inode *inode, struct file *file)
{
	int err=0,
			minor=iminor(file->f_dentry->d_inode);
	// MOD_INC_USE_COUNT;

  err=openUSBforCDD(minor,file);
	ERRH(err);

	return 0;
}

static int CDD_release (struct inode *inode, struct file *file)
{
	int err=0,
			minor=iminor(file->f_dentry->d_inode);
	// MOD_DEC_USE_COUNT;

  err=closeUSBforCDD(minor,file);
	ERRH(err);

	return 0;
}

static ssize_t CDD_read (struct file *file, char *buf, 
	size_t count, loff_t *ppos)
{
	int err=0, 
			rct=count,
			minor=iminor(file->f_dentry->d_inode);

	if (counter<0) 			counter=0;
	if (!counter)				return 0;

	// TODO:  Case when rct > counter
	if (rct>counter+80) rct=counter;
	
	// if (*ppos==0) 		return 0;
	if (*ppos>BUFSZ)  	return 0;
	if (*ppos>=counter) return 0;

  err=readCDDfromUSB(minor,rct);
	ERRH(err);

	msleep(250);

	err=copy_to_user(buf,&(CDD_Buffer[*ppos]),rct);
	if (err != 0) return -EFAULT;

	*ppos += rct;
	counter -= rct;
	return rct;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
static ssize_t CDD_write (struct file *file, const char __user *buf,
size_t count, loff_t *ppos)
#else
static ssize_t CDD_write (struct file *file, const char *buf, 
size_t count, loff_t *ppos)
#endif
{
	int err, 
			wct=count,
			minor=iminor(file->f_dentry->d_inode);

	if ((*ppos+count) >= BUFSZ) return -ENOMEM;

	err=copy_from_user(&(CDD_Buffer[*ppos]),buf,wct);
	if (err!=0) return -EFAULT;

	err=writeCDDtoUSB(minor,wct);
	ERRH(err);

	// while(i<wct) CDD_Buffer[i++]=0;

	*ppos += wct;
	counter += wct; 
	return wct;
}

static loff_t CDD_llseek (struct file *file, loff_t newpos, int whence)
{
  int pos;

  switch(whence) {
    case SEEK_SET:        // CDDoffset can be 0 or +ve
      pos=newpos;
      break;
    case SEEK_CUR:        // CDDoffset can be 0 or +ve
      pos=(file->f_pos + newpos);
      break;
    case SEEK_END:        // CDDoffset can be 0 or +ve
      pos=(counter + newpos);
      break;
    default:
      return -EINVAL;
  }
  if ((pos < 0)||(pos>counter)) 
    return -EINVAL;
    
  file->f_pos = pos;
  return pos;
}

struct file_operations CDD_fops =
{
	// for LINUX_VERSION_CODE 2.4.0 and later 
	owner:	THIS_MODULE, 	// struct module *owner
	open:		CDD_open, 	// open method 
	read:   CDD_read,	// read method 
	write:  CDD_write, 	// write method 
	release:CDD_release, 	// release method .. for close() system call
	llseek: CDD_llseek 	// lseek method .. for close() system call
};

static int CDD_init(void)
{
	int err;

	err = registerUSBforCDD();			// BLACKBOX for now!!
	ERRH(err);
	
	return 0;
}

static void CDD_exit(void)
{
	int err;
	
	err = unregisterUSBforCDD();			// BLACKBOX for now!!
}

module_init(CDD_init);
module_exit(CDD_exit);
