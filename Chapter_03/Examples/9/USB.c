// .vimrc set tabstop=2 
// .vimrc set number
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>

#define BUFSIZE 4096
#define MAXPKTSIZE 512


// USB generic starts here ----

// my DEV - ctl blk
struct CDDdev 
{
	struct usb_device 		*USBdev;				/* usb device for this device */
	struct usb_interface 	*intf; 					/* interface for this device */
	unsigned char	    		minor;					/* minor value */
//
	struct urb 				    *urb;						/* bulk in urb */
//
	unsigned char 				*bulk_ibuf; 		/* the buffer to in data */
	size_t	    			 		bulk_isz;  			/* the size of the in buffer */
	__u8				   				bulk_iEPaddr; 	/* bulk in endpoint address */
//
	__u8					 				bulk_oEPaddr;		/* bulk out endpoint address */
	size_t	    			 		bulk_osz;  			/* the size of the in buffer */
	unsigned char 				*bulk_obuf; 		/* the buffer to in data */
	struct kref     			kref;	 					/* module references counter */
	int										dir;
	int										count;
//
	struct mutex    			USBmutex;   		/* synch IO for disconnect */
	struct usb_anchor 		submitted;    	/* cancel INFLIGHT submissions */
	bool									inFlightReads;	
};

extern struct file_operations CDD_fops;

extern int registerUSBforCDD(void);
extern int unregisterUSBforCDD(void);
extern int USBprobe(struct usb_interface *, const struct usb_device_id *);

extern int openUSBforCDD(int minor, struct file *);
extern int closeUSBforCDD(int minor, struct file *);
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
		memcpy(CDD_Buffer,dev->urb->transfer_buffer,dev->count);
	  ((char *) dev->urb->transfer_buffer)[(dev->count)+1]=0;//null terminator! here!!
	}

	/* free allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length, 
	urb->transfer_buffer, urb->transfer_dma);

	// no need for usb_free_urb()
	// It will automatically be deallocated when not in use.
	// void usb_free_urb(struct urb *urb);
	// usb_free_urb(dev->urb);
	return;
}

int openUSBforCDD(int minor, struct file *file)
{
	int		 rc=0;
	struct CDDdev *dev=CDDdev;
	struct usb_interface *intf;

	intf=usb_find_interface(&USBdriver, minor);
  if (!intf) {
    pr_err("%s - error, can't find device for minor %d\n", __func__, minor);
    rc=-ENODEV;
    goto exit;
  }

  dev=usb_get_intfdata(intf);
  if (!dev) {
    rc=-ENODEV;
    goto exit;
  }
	// TODO
  // mutex_lock(&dev->USBmutex);
	
	// TODO
	// if (!dev->bulk_ibuf) dev->bulk_ibuf=kmalloc(dev->bulk_isz,GFP_KERNEL);
	// if (!dev->bulk_obuf) dev->bulk_obuf=kmalloc(dev->bulk_osz,GFP_KERNEL);
	file->private_data=dev;
	// TODO
  // mutex_unlock(&dev->USBmutex);

exit:
  return rc;
}

int closeUSBforCDD(int minor, struct file *file)
{
	int		 rc=0;
	struct CDDdev *dev=CDDdev;
	// struct usb_interface *intf=dev->intf;

  dev=file->private_data;
  if (!dev) {
    rc=-ENODEV;
    goto exit;
  }

  /* allow the device to be autosuspended */
	
	// TODO
  mutex_lock(&dev->USBmutex);
  // if (dev->intf)
  //  usb_autopm_put_interface(dev->intf);

  usb_free_urb(dev->urb);
  //usb_put_dev(dev->udev);
	
	// TODO
	// if (dev->bulk_ibuf) kfree(dev->bulk_ibuf);
	// if (dev->bulk_obuf) kfree(dev->bulk_obuf);
  
  // kfree(dev);
  mutex_unlock(&dev->USBmutex);

  /* decrement the count on our device */
  // kref_put(&dev->kref, skel_delete);
  return 0;


exit:
  return rc;
}

int readCDDfromUSB(int minor, int count)
{
	int rc=0, rct=MIN(count, BUFSIZE);

	struct CDDdev *dev=CDDdev;
	
	// dev->inFlightReads=0;

	// get MEM for URB
	dev->urb=usb_alloc_urb(0,GFP_KERNEL);
	if (!dev->urb)
	{
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc read urbmem, error %d", 
			__FUNCTION__, rc);
		goto error;
	}

	dev->dir=0;   	// read direction.
	dev->count=rct; // how much to read.

	// get MEM for DMA
	dev->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	dev->urb->transfer_buffer=
		usb_alloc_coherent(dev->USBdev,(size_t)MAXPKTSIZE,
			GFP_KERNEL,&dev->urb->transfer_dma);
	if(!dev->urb->transfer_buffer)
	{
		usb_free_urb(dev->urb);	
		rc=-ENOMEM;
		printk(KERN_ALERT "%s - failed to alloc read dmamem, error %d %ld", 
			__FUNCTION__, rc, (long int) dev->urb->transfer_buffer);
		goto error;
	}
	
	// fill urb
	usb_fill_bulk_urb(dev->urb, dev->USBdev, 
			usb_rcvbulkpipe(dev->USBdev,dev->bulk_iEPaddr),
			dev->urb->transfer_buffer, 
			dev->count, CDD_bulk_callback, 
			(void *) dev);

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
	dev->count=wct;
	
	usb_fill_bulk_urb(dev->urb, dev->USBdev, 
			usb_sndbulkpipe(dev->USBdev,dev->bulk_oEPaddr),
			dev->urb->transfer_buffer, wct, CDD_bulk_callback, 
			(void *) dev);
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

/*
// --
// --

	USB subsystem related methods are here.

// --
// --
*/

static void getUSBBulkEPInfo(struct CDDdev *dev)
{
	struct usb_interface *intf=dev->intf;

	struct usb_endpoint_descriptor *endpt;
	struct usb_host_interface *ifdesc;

	size_t bufsz=0;

	int i;

	ifdesc = intf->cur_altsetting;
	for (i = 0; i < ifdesc->desc.bNumEndpoints; ++i) {
	  endpt = &ifdesc->endpoint[i].desc;

	  /* check for bulk endpoint */
	  if ((endpt->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) 
		== USB_ENDPOINT_XFER_BULK){

			/* bulk in */
			if(endpt->bEndpointAddress & USB_DIR_IN) 
			{
				bufsz=usb_endpoint_maxp(endpt);
		    dev->bulk_isz = bufsz; // endpt->wMaxPacketSize;
		    dev->bulk_iEPaddr = endpt->bEndpointAddress;
		    dev->bulk_ibuf = kmalloc(dev->bulk_isz,GFP_KERNEL);
		    if (!dev->bulk_ibuf)
			  	printk(KERN_ERR "Could not allocate bulk in buffer");
			}

			/* bulk out */
			else
			{
		    dev->bulk_oEPaddr = endpt->bEndpointAddress; 
		    dev->bulk_osz = endpt->wMaxPacketSize;
		    dev->bulk_obuf = kmalloc(dev->bulk_osz,GFP_KERNEL);
		    if (!dev->bulk_obuf)
			  	printk(KERN_ERR "Could not allocate bulk out buffer");
			}
	  }
	}
}

// --
// --

int USBprobe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int err=0;

	struct CDDdev *dev=CDDdev;

	dev->USBdev = interface_to_usbdev(intf);
	dev->intf 	= intf;

	mutex_init(&dev->USBmutex);
	init_usb_anchor(&dev->submitted);

	USBclass.name = "CDD2";
	USBclass.fops = &CDD_fops;
	err = usb_register_dev(intf, &USBclass);
	if (err < 0)
	  printk(KERN_ERR "CDDUSB: No Minor# for device.");
	else
	{
	  printk(KERN_ALERT "CDDUSB: Got Minor#: %d\n",intf->minor);

		// setup End Point Information.
		getUSBBulkEPInfo(dev);

		/* save our device/data pointer for this interface device */
		// usb_set_intfdata(intf, dev->USBdev);
		usb_set_intfdata(intf, dev);
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

	// if needed.
	// dev=usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	usb_deregister_dev(intf, &USBclass);

	mutex_lock(&dev->USBmutex);
	if(dev->bulk_ibuf)
		kfree(dev->bulk_ibuf);
	if(dev->bulk_obuf)
		kfree(dev->bulk_obuf);
	dev->intf=NULL;
	dev->bulk_ibuf=NULL;
	dev->bulk_obuf=NULL;
	mutex_unlock(&dev->USBmutex);

	usb_kill_anchored_urbs(&dev->submitted);

	printk(KERN_INFO "CDDUSB: Minor disconnected: %d\n", intf->minor);
}


// --
// --

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
