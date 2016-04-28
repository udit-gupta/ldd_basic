// cdd_methods.c
//
// Linux Device Drivers
// char device driver
//
// This program has the methods for char device driver.

#ifndef _CDD_
#define _CDD_
#include "CDD.h"
#endif

//----------------------------------------------------

extern struct CDDdev_struct CDDdev[CDDNUMDEVS];

//----------------------------------------------------

int cdd_open(struct inode *inode, struct file *file) {
   struct CDDdev_struct *thisCDD;

   //get minor number
   unsigned int CDDminor = iminor(file->f_dentry->d_inode);
   thisCDD = &(CDDdev[CDDminor]);

   //if O_NONBLOCK flag given, won't block open
   if( !((file->f_flags)&O_NONBLOCK) ) {
      // obtain semaphore
      if(down_interruptible(&(thisCDD->open_block))) {
         return -ERESTARTSYS;	//couldn't obtain semaphore
      }
   }

   // CDD opened to append
   // file position to EOF
   if( (file->f_flags)&O_APPEND) {
      thisCDD->CDD_fpos = thisCDD->CDD_eof;
   }	//CDD opened to truncate, file pos set to top
   else if( (file->f_flags)&O_TRUNC) {
      thisCDD->CDD_fpos = 0;
      thisCDD->CDD_eof = 0;
   }
   else {
      thisCDD->CDD_fpos = 0;	//default case, position to top of file
   }

   spin_lock(&(thisCDD->CDD_lock));	//obtain spinlock
   thisCDD->open_count++;	//count open() calls
   spin_unlock(&(thisCDD->CDD_lock));

   file->private_data = thisCDD;

   return 0;
}

//----------------------------------------------------

int cdd_release(struct inode *inode, struct file *file) {
   struct CDDdev_struct *thisCDD = file->private_data;
   file->private_data = NULL;

   //if O_NONBLOCK flag given, won't block open
   if(!((file->f_flags)&O_NONBLOCK) ) {
      // release semaphore
      up(&(thisCDD->open_block));
   }

   return 0;
}

//----------------------------------------------------

int cdd_read(struct file *file, char *buf, size_t count, loff_t *ppos) {
   int error = 0;
   int read_len, buf_end;

   struct CDDdev_struct *thisCDD;
   thisCDD = (struct CDDdev_struct *) file->private_data;

   buf_end = thisCDD->buf_len;

   if(( (thisCDD->CDD_fpos) == buf_end) || ((thisCDD->CDD_fpos) == (thisCDD->CDD_eof)) ) {
      return 0;		//already at end of buffer or EOF
   }

   if( thisCDD->CDD_eof - thisCDD->CDD_fpos < count ) { //trying to read beyond EOF
      read_len = thisCDD->CDD_eof - thisCDD->CDD_fpos;
   }
   else {
      read_len = count;	//normal read
   }

   // obtain semaphore
   if(down_interruptible(&(thisCDD->CDD_sem))) {
      return -ERESTARTSYS;	//couldn't obtain semaphore
   }

   //copy from CDD buffer to user's buffer
   error = copy_to_user(buf, &(thisCDD->buffer[thisCDD->CDD_fpos]), read_len);
   //release semaphore on close
   up(&(thisCDD->CDD_sem));

   if(error>0) {
      thisCDD->CDD_fpos += read_len - error;	//copy_to_user returns the bytes NOT copied
      return (read_len - error);		//end of buffer
   }
   else if(error<0) {
      printk(KERN_ALERT "Error: Couldn't read data\n");
      return EBADE;
   }
   else {   // error == 0
      thisCDD->CDD_fpos += read_len;	//read full amount
      return read_len;
   }
}

//----------------------------------------------------

int cdd_write(struct file *file, const char *buf, size_t count, loff_t *ppos) {
   int error = 0;
   int write_len, buf_end;

   struct CDDdev_struct *thisCDD;
   thisCDD = (struct CDDdev_struct *) file->private_data;

   buf_end = thisCDD->buf_len;

   if(thisCDD->CDD_fpos == buf_end) {
      return 0;		//already at end of buffer
   }

   // tried to write past buffer limit
   // trim to allowed buffer limit
   if( (buf_end - thisCDD->CDD_fpos) < count) {
      write_len = buf_end - thisCDD->CDD_fpos;
   }
   else {
      write_len = count; //can write full length
   }

   // obtain semaphore
   if(down_interruptible(&(thisCDD->CDD_sem))) {
      return -ERESTARTSYS;	//couldn't obtain semaphore
   }

   //copy from user's buffer to CDD buffer
   error = copy_from_user(&(thisCDD->buffer[thisCDD->CDD_fpos]), buf, write_len);

   //release semaphore on close
   up(&(thisCDD->CDD_sem));

   if(error>0) {			//write finished early->end of buffer
      thisCDD->CDD_fpos += write_len - error;	//copy_from_user returns the bytes NOT copied
      
      if(thisCDD->CDD_fpos > thisCDD->CDD_eof) {
         thisCDD->CDD_eof = thisCDD->CDD_fpos;	//gone beyond current EOF
      }
      return (write_len - error);
   }
   else if(error<0) {
      return EBADE;
   }
   else { // error == 0
      thisCDD->CDD_eof += write_len;	//wrote full amount
      thisCDD->CDD_fpos += write_len;
      return write_len;
   }
}

//----------------------------------------------------

int cdd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
   struct CDDdev_struct *thisCDD;
   thisCDD = (struct CDDdev_struct *) filp->private_data;

   switch(cmd) {
      //return number of opens so far
      case NUM_OPEN:
         return (thisCDD->open_count);
         break;

      //return size of buffer
      case BUF_ALLOC:
         return thisCDD->buf_len;
         break;

      //return length of buffer used so far
      case BUF_USED:
         return (thisCDD->CDD_eof);
         break;

	//no default command
      default:
         return -1;

   }

}

//----------------------------------------------------

// llseek implementation.  Attempts to seek beyond EOF will
// seek to EOF.  Attempts to seek before top of file will
// seek to top of file.
int cdd_llseek(struct file *file, loff_t offset, int whence) {
   int new_pos;
   struct CDDdev_struct *thisCDD = (struct CDDdev_struct *) file->private_data;

printk(KERN_ALERT "SET %d CUR %d END %d\n", SEEK_SET, SEEK_CUR, SEEK_END);
printk(KERN_ALERT "whence: %d\n", whence);
   switch(whence) {
      case SEEK_SET:  //seek from top of file
         new_pos = offset;
         break;
      case SEEK_CUR:  //seek from current
         new_pos = thisCDD->CDD_fpos + offset;
         break;

      case SEEK_END:  //seek from end
         new_pos = thisCDD->CDD_eof + offset;
         break;

      default: //invalid
         return -EINVAL;
   }
   
   //handle limit (>end of buffer or <top of file) conditions
   if(new_pos > thisCDD->buf_len) {
      new_pos = thisCDD->buf_len;
   }
   else if(new_pos < 0) {
      new_pos = 0;
   }

   if(new_pos > thisCDD->CDD_eof) {
      thisCDD->CDD_eof = new_pos;
      thisCDD->CDD_fpos = new_pos;
   }

   thisCDD->CDD_fpos = new_pos;

   return new_pos;
}

