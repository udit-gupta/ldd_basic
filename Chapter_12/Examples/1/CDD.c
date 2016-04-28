// chardd.c
//
// Linux Device Drivers
//
// This program is a simple char device driver.
// This file includes the global variables and
// init and exit functions for CDD.

#ifndef _CDD_
#define _CDD_
#include "CDD.h"
#endif

//----------------------------------------------------------

//major number can be given at runtime
static unsigned int CDDmajor = CDDMAJOR;
module_param(CDDmajor, int, 0);

//----------------------------------------------------------

static dev_t firstdevno;
struct CDDdev_struct CDDdev[CDDNUMDEVS];

struct file_operations CDD_fops = {
   .owner = THIS_MODULE,
   .llseek = cdd_llseek,
   .read = cdd_read,
   .write = cdd_write,
   // .ioctl = cdd_ioctl,
   .open = cdd_open,
   .release = cdd_release,
};

//----------------------------------------------------------

int cdd_init(void) {
   int error, i;
   dev_t devno;
   struct cdev *cdevp;
   struct CDDdev_struct *thisCDD;

   if( CDDmajor ) {
      // Major, minor numbers hard coded
      firstdevno = MKDEV(CDDmajor, CDDMINOR);

      error = register_chrdev_region(firstdevno, 1, CDD);
      if( error<0) {
         printk(KERN_ALERT "Error registering CDD: %d\n", error);
         return -error;
      }
   }
   else {
      // dynamically allocated Major, minor numbers
      error = alloc_chrdev_region(&firstdevno, CDDMINOR, CDDNUMDEVS, CDD);
      if( error<0 ) {
         printk(KERN_ALERT "Error adding CDD: %d\n", error);
         return -error;
      }

      CDDmajor = MAJOR(firstdevno);
   }

   printk(KERN_INFO "Kernel assigned major number to CDD: %d\n", CDDmajor);

   for(i=0; i<CDDNUMDEVS; i++) {
      thisCDD=&(CDDdev[i]);
      cdevp=&(thisCDD->CDD_cdev);

      devno = MKDEV(CDDmajor, i);	//create major, minor device
      thisCDD->CDD_devno = devno;

      //initialize semaphore for reads and writes
      sema_init(&(thisCDD->CDD_sem),1);
      //initialize semaphore for blocking open
      sema_init(&(thisCDD->open_block),1);
      //initialize spinlock for open() call count
      spin_lock_init(&(thisCDD->CDD_lock));
      
      cdev_init(cdevp, &CDD_fops);
      cdevp->owner = THIS_MODULE;
      cdevp->ops = &CDD_fops;

      error = cdev_add(cdevp, devno, CDDNUMDEVS);
      if(error) {
         printk(KERN_ALERT "Error %d adding %s (%d)\n", error, CDD, i);
         return error;
      }

      printk(KERN_INFO "Added minor number: %d\n", i);

   }

   //create /proc entry for CDD
   if(CDD_proc_create()) {
      printk(KERN_ALERT "Error creating /proc file\n");
      return -1;
   }

   //allocate buffer for each minor device
   CDDdev[0].buf_len = CDD1;
   CDDdev[1].buf_len = CDD2;
   CDDdev[2].buf_len = CDD3;
   CDDdev[3].buf_len = CDD4;

   //initialize counters
   for(i=0; i<CDDNUMDEVS; i++) {
      CDDdev[i].CDD_eof = 0;
      CDDdev[i].CDD_fpos = 0;
      CDDdev[i].open_count = 0;
      CDDdev[i].buffer = kmalloc(CDDdev[i].buf_len*sizeof(char),GFP_KERNEL);
   }

   return 0;
}

//----------------------------------------------------------

void cdd_exit(void) {
   int i;
   dev_t devno;
   struct cdev *cdevp;
   struct CDDdev_struct *thisCDD;


   for(i=0; i<CDDNUMDEVS; i++) {
      thisCDD=CDDdev+i;
      cdevp = &(thisCDD->CDD_cdev);

      kfree(thisCDD->buffer);	//cleanup buffer

      cdev_del(cdevp);
   }

   devno = MKDEV(CDDmajor, CDDMINOR);

   unregister_chrdev_region(devno, CDDNUMDEVS);

   if( CDDmajor != CDDMAJOR) {
      printk(KERN_INFO "kernel assigned major number for CDD removed: %d\n", CDDmajor);
   }

   CDD_proc_destroy();

}

//----------------------------------------------------------

module_init(cdd_init);
module_exit(cdd_exit);
