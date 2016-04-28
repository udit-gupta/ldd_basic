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
	size_t	    			 		bulk_out_sz;  	/* the size of the in buffer */
	unsigned char 				*bulk_out_buf; 	/* the buffer to in data */
	struct kref     			kref;	 					/* module references counter */
	struct urb 				    *urb;
	int										dir;
	int										count;
	void	*								buf;
};

extern struct file_operations CDD_fops;

extern int registerUSBforCDD(void);
extern int unregisterUSBforCDD(void);
extern int USBprobe(struct usb_interface *, const struct usb_device_id *);

extern int readCDDfromUSB(int minor, int count);
extern int writeCDDtoUSB(int minor, size_t count);

extern void CDD_bulk_callback(struct urb *);
extern void USBdisconnect(struct usb_interface *);

static void getUSBBulkEPInfo(struct CDDdev *);

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
static struct usb_class_driver USBclass;

extern char *CDD_Buffer;

static struct CDDdev CDDdev[1];

#define MIN(a,b) (((a)<=(b))?(a):(b))
 
static void getUSBBulkEPInfo(struct CDDdev *dev)
{
	struct usb_interface *intf=dev->intf;

	struct usb_endpoint_descriptor *endpt;
	struct usb_host_interface *ifdesc;

	int i;

	ifdesc = intf->cur_altsetting;
	for (i = 0; i < ifdesc->desc.bNumEndpoints; ++i) {
	  endpt = &ifdesc->endpoint[i].desc;

	  /* check for bulk endpoint */
	  if ((endpt->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) 
		== USB_ENDPOINT_XFER_BULK){

			/* bulk in */
			if(endpt->bEndpointAddress & USB_DIR_IN) {
		    dev->bulk_in_EPaddr = endpt->bEndpointAddress;
		    dev->bulk_in_sz = endpt->wMaxPacketSize;
		    dev->bulk_in_buf = kmalloc(dev->bulk_in_sz,GFP_KERNEL);
		    if (!dev->bulk_in_buf)
			  	printk(KERN_ERR "Could not allocate bulk in buffer");
			}

			/* bulk out */
			else
			{
		    dev->bulk_out_EPaddr = endpt->bEndpointAddress; 
		    dev->bulk_out_sz = endpt->wMaxPacketSize;
		    dev->bulk_out_buf = kmalloc(dev->bulk_out_sz,GFP_KERNEL);
		    if (!dev->bulk_out_buf)
			  	printk(KERN_ERR "Could not allocate bulk out buffer");
			}
	  }
	}
}

int USBprobe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int err=0;

	struct CDDdev *dev=CDDdev;

	dev->USBdev = interface_to_usbdev(intf);
	dev->intf 	= intf;

	USBclass.name = "CDD2";
	USBclass.fops = &CDD_fops;
	err = usb_register_dev(dev->intf, &USBclass);
	if (err < 0)
	  printk(KERN_ERR "CDDUSB: No Minor# for device.");
	else
	{
	  printk(KERN_ALERT "CDDUSB: Got Minor#: %d\n",dev->intf->minor);

		/* save our device/data pointer for this interface device */
		usb_set_intfdata(dev->intf, dev->USBdev);
		getUSBBulkEPInfo(dev);
	}

	return err;
}

void USBdisconnect(struct usb_interface *intf)
{
	struct CDDdev *dev=CDDdev;

	// needs locking at this time to prevent rare timing condition/conflict
	// with open call, in order to reset ptr to intf
	// usb_unlink_urb(dev->urb);
	// dev->urb=NULL;
	if(dev->bulk_in_buf)
		kfree(dev->bulk_in_buf);
	if(dev->bulk_out_buf)
		kfree(dev->bulk_out_buf);

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
	// struct CDDdev *dev=CDDdev;
	// usb_kill_urb(dev->urb);
	
	usb_deregister(&USBdriver);
	return 0;
}


//
// CDD
//

// http://stackoverflow.com/questions/15308770/read-usb-bulk-message-on-linux
// function is called exactly once, when the URB is completed.
void CDD_bulk_callback(struct urb *urb)
{
	struct CDDdev *dev=CDDdev;

	/* sync/async faults are not errors */
	if(urb->status &&
			!(urb->status == -ENOENT ||
			  urb->status == -ECONNRESET ||
			  urb->status == -ESHUTDOWN)) 
		printk(KERN_ALERT "%s - nonzero bulk status received: error %d", 
			__FUNCTION__, urb->status);

	// CDD_Buffer has the data at this time.
	if (!dev->dir) 		// read direction
	{
		memcpy(CDD_Buffer,dev->buf,dev->count);
	  ((char *) dev->urb->transfer_buffer)[dev->count]=0;//null terminator! here!!
	}

	/* free allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length, 
		 dev->buf, urb->transfer_dma);

	// no need for usb_free_urb()
	// It will automatically be deallocated when not in use.
	// void usb_free_urb(struct urb *urb);
	// usb_free_urb(dev->urb);
	return;
}

int readCDDfromUSB(int minor, int count)
{
	int rc=0, rct=MIN(count, BUFSIZE);

	struct CDDdev *dev=CDDdev;
	
	// get MEM for URB
	dev->urb=usb_alloc_urb(0,GFP_KERNEL);
	if (!dev->urb)
	{
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc read urbmem, error %d", 
			__FUNCTION__, rc);
		goto error;
	}

	// fill urb
	usb_fill_bulk_urb(dev->urb, dev->USBdev, 
			usb_rcvbulkpipe(dev->USBdev,dev->bulk_in_EPaddr),
			dev->urb->transfer_buffer, dev->count, CDD_bulk_callback, 
			(void *) dev);

	// get MEM for DMA
	dev->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	dev->buf=
		usb_alloc_coherent(dev->USBdev,(size_t)MAXPKTSIZE,
			GFP_KERNEL,&dev->urb->transfer_dma);
	if(!dev->buf)
	{
		usb_free_urb(dev->urb);	
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc read dmamem, error %d %ld", 
			__FUNCTION__, rc, (long int) dev->buf);
		goto error;
	}

	dev->dir=0;   		// read direction.
	dev->count=rct; // how much to read.

	// send urb
	rc=usb_submit_urb(dev->urb,GFP_KERNEL);
	if (rc)
	{
		printk(KERN_ALERT "%s - failed to submit read urb, error %d", 
			__FUNCTION__, rc);
		if(dev->urb) usb_unlink_urb(dev->urb);
		dev->urb=NULL;
		goto error;
	}

	rc=rct;
	
error:
	return rc;
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
		printk(KERN_ALERT "%s - failed to alloc write urbmem, error %d", 
			__FUNCTION__, rc);
		goto error;
	}

	// get MEM for DMA
	dev->urb->transfer_buffer=usb_alloc_coherent(dev->USBdev,
		(size_t)MAXPKTSIZE,GFP_KERNEL,&dev->urb->transfer_dma);
	if(!dev->urb->transfer_buffer)
	{
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc write dmamem, error %d %ld",
			__FUNCTION__, rc, (long int) dev->urb->transfer_buffer);
		goto error;
	}

	// CDD_Buffer has the data at this time.
	memcpy(dev->urb->transfer_buffer,CDD_Buffer,count);

	dev->dir=1;
	dev->count=count;
	
	usb_fill_bulk_urb(dev->urb, dev->USBdev, 
			usb_sndbulkpipe(dev->USBdev,dev->bulk_out_EPaddr),
			dev->urb->transfer_buffer, wct, CDD_bulk_callback, (void *) 1);
	dev->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	rc=usb_submit_urb(dev->urb,GFP_KERNEL);
	if (rc)
	{
		printk(KERN_ALERT "%s - failed to submit write urb, error %d", 
			__FUNCTION__, rc);
		goto error;
	}

	rc=wct;

error:
	return rc;
}
