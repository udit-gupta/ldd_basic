// see http://www.embeddedlinux.org.cn/EssentialLinuxDeviceDrivers/final/ch11lev1sec5.html

int readCDDfromUSB(void)
{
	return 0;
}

int writeCDDtoUSB(void)
{
	return 0;
}

// USB related code starts here ----
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define USB_BULKOUTEP 0x01
#define USB_BULKINEP  0x82
#define BUFSIZE 4096

extern int USBprobe(struct usb_interface *, const struct usb_device_id *);
extern void USBdisconnect(struct usb_interface *);
extern struct file_operations CDD_fops;

static struct usb_device *USBdevice;
static struct usb_class_driver USBclass;

static struct usb_device_id USBtable[] =
{
    { .match_flags =  USB_DEVICE_ID_MATCH_DEVICE,
      .idVendor =  USBVENDORID,
      .idProduct = USBPRODUCTID
		},
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

int USBprobe(struct usb_interface *intf, const struct usb_device_id *id)
{
    int err=0;

    USBdevice = interface_to_usbdev(intf);

    USBclass.name = "usb/CDD%d";
    USBclass.fops = &CDD_fops;
    if ((err = usb_register_dev(intf, &USBclass)) < 0)
    {
        /* Something prevented us from registering this driver */
        printk(KERN_ERR "Not able to get a minor for this device.");
    }
    else
        printk(KERN_INFO "CDDUSB: Minor obtained: %d\n", intf->minor);

    return err;
}

void USBdisconnect(struct usb_interface *intf)
{
		printk(KERN_INFO "CDDUSB: Minor disconnected: %d\n", intf->minor);
    usb_deregister_dev(intf, &USBclass);
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
