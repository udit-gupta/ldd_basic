// CDDproc.c
//
// Linux Device Drivers
//
// This file contains the functions for maintaining proc entries for CDD

#ifndef _CDD_
#define _CDD_
#include "CDD.h"
#endif

#define MAX_PROC_BUFFER 300

//------------------------------------------------------

struct proc_data {
   char buffer[MAX_PROC_BUFFER];
   int buffer_size;
   int write_flag;
};


extern struct CDDdev_struct CDDdev[CDDNUMDEVS];

//------------------------------------------------------

//static struct proc_dir_entry proc_CDD[CDDNUMDEVS];
static struct proc_dir_entry *procdir_CDD;
static struct proc_dir_entry *proc_CDD1;
static struct proc_dir_entry *proc_CDD2;
static struct proc_dir_entry *proc_CDD3;
static struct proc_dir_entry *proc_CDD4;
static struct proc_dir_entry *proc_PCI;
static struct proc_data CDDproc_data[CDDNUMDEVS];

//------------------------------------------------------

int CDD1_proc_read(char *buf, char **start, off_t offset, int len, int *eof, void *unused) {
   char temp1[50],temp2[50],temp3[50],temp4[50],temp5[50];
   struct CDDdev_struct *thisCDD;
   struct proc_data *thisproc = &(CDDproc_data[0]);

   thisCDD = &(CDDdev[0]);

   *eof = 1;

   //create proc entry only on first run
   if(!(thisproc->write_flag)) {
      thisproc->write_flag = 1;
      thisproc->buffer_size = 0;
      sprintf(temp1, "Group number: z\n");
      sprintf(temp2, "Team members: Me\n");
      sprintf(temp3, "Buffer Length - Allocated: %d characters\n", thisCDD->buf_len);
      sprintf(temp4, "Buffer Length - Used: %d characters\n", thisCDD->CDD_fpos);
      sprintf(temp5, "Open() has been called: %d times\n", thisCDD->open_count);

      thisproc->buffer_size += sprintf(thisproc->buffer, "%s%s%s%s%s", temp1, temp2, temp3, temp4, temp5);
   }

   return(sprintf(buf, thisproc->buffer));
}

//------------------------------------------------------

int CDD2_proc_read(char *buf, char **start, off_t offset, int len, int *eof, void *unused) {
   char temp1[50],temp2[50],temp3[50],temp4[50],temp5[50];
   struct CDDdev_struct *thisCDD;
   struct proc_data *thisproc = &(CDDproc_data[1]);

   thisCDD = &(CDDdev[1]);

   *eof = 1;

   //create proc entry only on first run
   if(!(thisproc->write_flag)) {
      thisproc->write_flag = 1;
      thisproc->buffer_size = 0;
      sprintf(temp1, "Group number: z\n");
      sprintf(temp2, "Team members: Me\n");
      sprintf(temp3, "Buffer Length - Allocated: %d characters\n", thisCDD->buf_len);
      sprintf(temp4, "Buffer Length - Used: %d characters\n", thisCDD->CDD_fpos);
      sprintf(temp5, "Open() has been called: %d times\n", thisCDD->open_count);

      thisproc->buffer_size += sprintf(thisproc->buffer, "%s%s%s%s%s", temp1, temp2, temp3, temp4, temp5);
   }

   return(sprintf(buf, thisproc->buffer));
}

//------------------------------------------------------

int CDD3_proc_read(char *buf, char **start, off_t offset, int len, int *eof, void *unused) {
   char temp1[50],temp2[50],temp3[50],temp4[50],temp5[50];
   struct CDDdev_struct *thisCDD;
   struct proc_data *thisproc = &(CDDproc_data[2]);

   thisCDD = &(CDDdev[2]);

   *eof = 1;

   //create proc entry only on first run
   if(!(thisproc->write_flag)) {
      thisproc->write_flag = 1;
      thisproc->buffer_size = 0;
      sprintf(temp1, "Group number: z\n");
      sprintf(temp2, "Team members: Me\n");
      sprintf(temp3, "Buffer Length - Allocated: %d characters\n", thisCDD->buf_len);
      sprintf(temp4, "Buffer Length - Used: %d characters\n", thisCDD->CDD_fpos);
      sprintf(temp5, "Open() has been called: %d times\n", thisCDD->open_count);

      thisproc->buffer_size += sprintf(thisproc->buffer, "%s%s%s%s%s", temp1, temp2, temp3, temp4, temp5);
   }

   return(sprintf(buf, thisproc->buffer));
}

//------------------------------------------------------

int CDD4_proc_read(char *buf, char **start, off_t offset, int len, int *eof, void *unused) {
   char temp1[50],temp2[50],temp3[50],temp4[50],temp5[50];
   struct CDDdev_struct *thisCDD;
   struct proc_data *thisproc = &(CDDproc_data[3]);

   thisCDD = &(CDDdev[3]);

   *eof = 1;

   //create proc entry only on first run
   if(!(thisproc->write_flag)) {
      thisproc->write_flag = 1;
      thisproc->buffer_size = 0;
      sprintf(temp1, "Group number: z\n");
      sprintf(temp2, "Team members: Me\n");
      sprintf(temp3, "Buffer Length - Allocated: %d characters\n", thisCDD->buf_len);
      sprintf(temp4, "Buffer Length - Used: %d characters\n", thisCDD->CDD_fpos);
      sprintf(temp5, "Open() has been called: %d times\n", thisCDD->open_count);

      thisproc->buffer_size += sprintf(thisproc->buffer, "%s%s%s%s%s", temp1, temp2, temp3, temp4, temp5);
   }

   return(sprintf(buf, thisproc->buffer));
}

//------------------------------------------------------


// this scans all PCI devices and prints out the following for each device
// PCI_VENDOR_ID
// PCI_DEVICE_ID
// PCI_INTERRUPT_LINE
// PCI_LATENCY_TIMER
// PCI_COMMAND
int CDD_proc_PCI(char *buf, char **start, off_t offset, int len, int *eof, void *unused) {
   char temp1[128];
   u8 temp;
   int index = 1;
   struct pci_dev *cur_dev;

   *eof = 1;

   //start search
   cur_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);

   sprintf(buf, "device %d:\n", index++);

   pci_read_config_byte(cur_dev, PCI_VENDOR_ID, &temp);
   sprintf(temp1, "PCI_VENDOR_ID = %d\n", temp);
   strcat(buf, temp1);
   pci_read_config_byte(cur_dev, PCI_DEVICE_ID, &temp);
   sprintf(temp1, "PCI_DEVICE_ID = %d\n", temp);
   strcat(buf, temp1);
   pci_read_config_byte(cur_dev, PCI_INTERRUPT_LINE, &temp);
   sprintf(temp1, "PCI_INTERRUPT_LINE = %d\n", temp);
   strcat(buf, temp1);
   pci_read_config_byte(cur_dev, PCI_LATENCY_TIMER, &temp);
   sprintf(temp1, "PCI_LATENCY_TIMER = %d\n", temp);
   strcat(buf, temp1);
   pci_read_config_byte(cur_dev, PCI_COMMAND, &temp);
   sprintf(temp1, "PCI_COMMAND = %d\n", temp);
   strcat(buf, temp1);
   strcat(buf, "\n");

   //walk through PCI devices until end
   while( (cur_dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, cur_dev))) {
      sprintf(temp1, "device %d:\n", index++);
      strcat(buf, temp1);

      pci_read_config_byte(cur_dev, PCI_VENDOR_ID, &temp);
      sprintf(temp1, "PCI_VENDOR_ID = %d\n", temp);
      strcat(buf, temp1);
      pci_read_config_byte(cur_dev, PCI_DEVICE_ID, &temp);
      sprintf(temp1, "PCI_DEVICE_ID = %d\n", temp);
      strcat(buf, temp1);
      pci_read_config_byte(cur_dev, PCI_INTERRUPT_LINE, &temp);
      sprintf(temp1, "PCI_INTERRUPT_LINE = %d\n", temp);
      strcat(buf, temp1);
      pci_read_config_byte(cur_dev, PCI_LATENCY_TIMER, &temp);
      sprintf(temp1, "PCI_LATENCY_TIMER = %d\n", temp);
      strcat(buf, temp1);
      pci_read_config_byte(cur_dev, PCI_COMMAND, &temp);
      sprintf(temp1, "PCI_COMMAND = %d\n", temp);
      strcat(buf, temp1);
      strcat(buf, "\n");

   }

   return strlen(buf);
}

//------------------------------------------------------

int CDD1_proc_write(struct file *file, const char *buf, unsigned long count, void *data) {
   int error;
   int to_write;
   struct proc_data *thisproc = &(CDDproc_data[0]);

   //does this write go beyond the size of the proc buffer?
   //we need to stay within the size of the buffer
   if(thisproc->buffer_size+count > MAX_PROC_BUFFER) {
      to_write = MAX_PROC_BUFFER - thisproc->buffer_size;
   }
   else {
      to_write = count;
   }

   error = copy_from_user(thisproc->buffer, buf, to_write);
   thisproc->buffer_size += to_write;

   return thisproc->buffer_size;
//   return(sprintf(thisproc->buffer, "Group number: 0\n"));
}

//------------------------------------------------------

int CDD2_proc_write(struct file *file, const char *buf, unsigned long count, void *data) {
   int to_write;
   int error;
   struct proc_data *thisproc = &(CDDproc_data[1]);

   //does this write go beyond the size of the proc buffer?
   //we need to stay within the size of the buffer
   if(thisproc->buffer_size+count > MAX_PROC_BUFFER) {
      to_write = MAX_PROC_BUFFER - thisproc->buffer_size;
   }
   else {
      to_write = count;
   }

   error = copy_from_user(thisproc->buffer, buf, to_write);
   thisproc->buffer_size += to_write;

   return thisproc->buffer_size;
}

//------------------------------------------------------

int CDD3_proc_write(struct file *file, const char *buf, unsigned long count, void *data) {
   int error;
   int to_write;
   struct proc_data *thisproc = &(CDDproc_data[2]);

   //does this write go beyond the size of the proc buffer?
   //we need to stay within the size of the buffer
   if(thisproc->buffer_size+count > MAX_PROC_BUFFER) {
      to_write = MAX_PROC_BUFFER - thisproc->buffer_size;
   }
   else {
      to_write = count;
   }

   error = copy_from_user(thisproc->buffer, buf, to_write);
   thisproc->buffer_size += to_write;

   return thisproc->buffer_size;
}

//------------------------------------------------------

int CDD4_proc_write(struct file *file, const char *buf, unsigned long count, void *data) {
   int error;
   int to_write;
   struct proc_data *thisproc = &(CDDproc_data[3]);

   //does this write go beyond the size of the proc buffer?
   //we need to stay within the size of the buffer
   if(thisproc->buffer_size+count > MAX_PROC_BUFFER) {
      to_write = MAX_PROC_BUFFER - thisproc->buffer_size;
   }
   else {
      to_write = count;
   }

   error = copy_from_user(thisproc->buffer, buf, to_write);
   thisproc->buffer_size += to_write;

   return thisproc->buffer_size;
}

//------------------------------------------------------

//create /proc/CDD entry
int CDD_proc_create(void) {

   procdir_CDD = proc_mkdir(CDD,0);

   proc_CDD1 = create_proc_entry(CDD1_NAME, 0, procdir_CDD);
   proc_CDD2 = create_proc_entry(CDD2_NAME, 0, procdir_CDD);
   proc_CDD3 = create_proc_entry(CDD3_NAME, 0, procdir_CDD);
   proc_CDD4 = create_proc_entry(CDD4_NAME, 0, procdir_CDD);

   proc_CDD1->read_proc = CDD1_proc_read;
   // proc_CDD1->owner = THIS_MODULE;
   proc_CDD1->write_proc = CDD1_proc_write;

   proc_CDD2->read_proc = CDD2_proc_read;
   // proc_CDD2->owner = THIS_MODULE;
   proc_CDD2->write_proc = CDD2_proc_write;

   proc_CDD3->read_proc = CDD3_proc_read;
   // proc_CDD3->owner = THIS_MODULE;
   proc_CDD3->write_proc = CDD3_proc_write;

   proc_CDD4->read_proc = CDD4_proc_read;
   // proc_CDD4->owner = THIS_MODULE;
   proc_CDD4->write_proc = CDD4_proc_write;

   proc_PCI = create_proc_entry(PCI, 0, 0);
   proc_PCI->read_proc = CDD_proc_PCI;
   // proc_PCI->owner = THIS_MODULE;
   proc_PCI->write_proc = NULL;

   return 0;
}

//------------------------------------------------------

int CDD_proc_destroy(void) {

   remove_proc_entry(CDD1_NAME, procdir_CDD);
   remove_proc_entry(CDD2_NAME, procdir_CDD);
   remove_proc_entry(CDD3_NAME, procdir_CDD);
   remove_proc_entry(CDD4_NAME, procdir_CDD);
   
   remove_proc_entry(PCI, 0);

   //remove CDD directory in /proc
   if(procdir_CDD) {
      remove_proc_entry(CDD, 0);
   }

   return 0;
}
