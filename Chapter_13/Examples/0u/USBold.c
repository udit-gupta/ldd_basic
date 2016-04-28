// vi set tabstop=2
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>

#define BUFSIZE 4096
#define MAXPKTSIZE 512


// USB generic starts here ----

struct CDDdev 
{
	struct usb_device 		*USBdev;				/* usb device for this device */
	struct usb_interface 	*intf; 					/* interface for this device */
	unsigned char	    		minor;					/* minor value */
	unsigned char 				*bulk_in_buf; 	/* the buffer to in data */
	size_t	    			 		bulk_in_sz;  		/* the size of the in buffer */
	__u8				   				bulk_in_EPaddr; /* bulk in endpoint address */
	__u8					 				bulk_out_EPaddr;/* bulk out endpoint address */
	struct kref     			kref;	 					/* module references counter */
  void 									*buf;
	struct urb 				    *urb;
};


extern int USBprobe(struct usb_interface *, const struct usb_device_id *);
extern void USBdisconnect(struct usb_interface *);
extern struct file_operations CDD_fops;
// extern usb_complete_t CDD_write_bulk_callback(struct urb *urb, struct pt_regs *regs);
extern void CDD_write_bulk_callback(struct urb *urb);

static struct usb_device_id USBtable[] =
{
	{ USB_DEVICE(USBVENDORID,USBPRODUCTID) },
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, USBtable);

static struct usb_driver USBdriver =
{
	.name = "USBdriver",
	.probe = USBprobe,
	.disconnect = USBdisconnect,
	.id_table = USBtable,
};
//static struct usb_device *USBdevice;
static struct usb_class_driver USBclass;

extern char *CDD_Buffer;


static struct CDDdev CDDdev[1];

#define MIN(a,b) (((a)<=(b))?(a):(b))
 
static void set_bulk_address (struct CDDdev *dev,struct usb_interface *intf)
{
	struct usb_endpoint_descriptor *endpoint;
	struct usb_host_interface *intf_desc;
	int i;

	intf_desc = intf->cur_altsetting;
	for (i = 0; i < intf_desc->desc.bNumEndpoints; ++i) {
	  endpoint = &intf_desc->endpoint[i].desc;

	  /* check for bulk endpoint */
	  if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) 
		== USB_ENDPOINT_XFER_BULK){

			/* bulk in */
			if(endpoint->bEndpointAddress & USB_DIR_IN) {
		    dev->bulk_in_EPaddr = endpoint->bEndpointAddress;
		    dev->bulk_in_sz = endpoint->wMaxPacketSize;
		    dev->bulk_in_buf = kmalloc(dev->bulk_in_sz,GFP_KERNEL);
		    if (!dev->bulk_in_buf)
			  printk(KERN_ERR "Could not allocate bulk buffer");
			}

			/* bulk out */
			else
		    dev->bulk_out_EPaddr = endpoint->bEndpointAddress; 
	  }
	}
}

int USBprobe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int err=0;

	struct CDDdev *dev=CDDdev;

	dev->USBdev = interface_to_usbdev(intf);

	USBclass.name = "CDD2";
	USBclass.fops = &CDD_fops;
	err = usb_register_dev(intf, &USBclass);
	if (err < 0)
	  printk(KERN_ERR "CDDUSB: No Minor# for device.");
	else
	{
	  printk(KERN_ALERT "CDDUSB: Got Minor#: %d\n",intf->minor);

		/* save our data pointer in this interface device */
		usb_set_intfdata(intf, dev->USBdev);
		set_bulk_address (dev, intf);
	}

	return err;
}

void USBdisconnect(struct usb_interface *intf)
{
	// needs locking at this time to prevent rare condition of 
	// conflicting with the open call, in order to reset ptr to intf
	usb_set_intfdata(intf, NULL);
	usb_deregister_dev(intf, &USBclass);
	printk(KERN_INFO "CDDUSB: Minor disconnected: %d\n", intf->minor);
}

int registerUSBforCDD(void)
{
	int err=0;

	err=usb_register(&USBdriver); // Register driver with USB

	if (err)
		printk("usb_register failed. Error# %d", err);

	return err;
}

int unregisterUSBforCDD(void)
{
	usb_deregister(&USBdriver);
	return 0;
}


//
//
// CDD
//
//

/*
int readCDDfromUSB(int minor)
{
	int rc=0, cnt=0;
	rc=usb_bulk_msg(USBdevice, usb_rcvbulkpipe(USBdevice,EP_IN),
		CDD_Buffer, BUFSIZE, &cnt, HZ*5);
	return rc;
}
*/

// http://stackoverflow.com/questions/15308770/read-usb-bulk-message-on-linux



int readCDDfromUSB(int minor, int count)
{
	int rc=0, wct=MIN(count, BUFSIZE);

	struct usb_interface *intf;
	struct usb_device *CDDdevice;
	struct CDDdev *dev=CDDdev;
	
	intf=usb_find_interface(&USBdriver, minor);
	if (!intf) 
	{
	  printk(KERN_ERR "CDD: intf not found");
		return -ENODEV;
	}

	CDDdevice=usb_get_intfdata(intf);
	if (!CDDdevice) 
	{
	  printk(KERN_ERR "CDD: CDDdevice not found");
		return -ENODEV;
	}

	// rc=usb_bulk_msg(CDDdevice, usb_rcvbulkpipe(CDDdevice,EP_IN),
	rc=usb_bulk_msg(CDDdevice, usb_rcvbulkpipe(CDDdevice,dev->bulk_in_EPaddr),
		CDD_Buffer, dev->bulk_in_sz, &cnt, HZ*5);

	if (rc)
		printk(KERN_ALERT "rc usb_bulk_msg == %d\n", rc);
	return rc;
}

// function is called exactly once, when the URB is completed.
void CDD_write_bulk_callback(struct urb *urb)
{

	/* sync/async faults are not errors */
	if(urb->status &&
			!(urb->status == -ENOENT ||
			  urb->status == -ECONNRESET ||
			  urb->status == -ESHUTDOWN)) 
		printk(KERN_ALERT "%s - nonzero write bulk status received: error %d", 
			__FUNCTION__, urb->status);

	/* free allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length, 
		 urb->transfer_buffer, urb->transfer_dma);

	// no need for usb_free_urb()
	// It will automatically be deallocated when not in use.
	// void usb_free_urb(struct urb *urb);
	return;
}

int writeCDDtoUSB(int minor, size_t count)
{
	int rc=0, wct=MIN(count, BUFSIZE);

	//rc=usb_bulk_msg(USBdevice, usb_sndbulkpipe(USBdevice, EP_OUT),
	//CDD_Buffer, wct, &wct, HZ*5);
	
	struct CDDdev *dev=CDDdev;
	
	// get MEM for URB
	dev->urb=usb_alloc_urb(0,GFP_KERNEL);
	if (!dev->urb)
	{
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc write urbmem, error %d", __FUNCTION__, rc);
		goto error;
	}

	// get MEM for DMA
	dev->buf=usb_alloc_coherent(dev->USBdev,(size_t)MAXPKTSIZE,
			GFP_KERNEL,&dev->urb->transfer_dma);
	if(!dev->buf)
	{
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc write bufmem, error %d %ld", __FUNCTION__, rc, (long int) dev->buf);
		goto error;
	}

	// CDD_Buffer has the data at this time.
	memcpy(dev->buf,CDD_Buffer,count);
	
	usb_fill_bulk_urb(dev->urb, dev->USBdev, 
			usb_sndbulkpipe(dev->USBdev,dev->bulk_out_EPaddr),
			dev->buf, count, CDD_write_bulk_callback, dev);
	dev->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	rc=usb_submit_urb(dev->urb,GFP_KERNEL);
	if (rc)
	{
		printk(KERN_ALERT "%s - failed to submit write urb, error %d", __FUNCTION__, rc);
		goto error;
	}

	rc=wct;

error:
	return rc;
}
