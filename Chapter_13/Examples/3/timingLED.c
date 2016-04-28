// Adapted from http://www.linuxjournal.com/article/7353
// and, https://github.com/ileitch/delcom_904008_driver/blob/master/delcom_904008.rb
 /*
  * USB LED driver - 1.1
  *
  * Copyright (C) 2004 Greg Kroah-Hartman (greg@kroah.com)
  *
  *      This program is free software; you can redistribute it and/or
  *      modify it under the terms of the GNU General Public License as
  *      published by the Free Software Foundation, version 2.
  *
  */
 
 #include <linux/kernel.h>
 #include <linux/errno.h>
 #include <linux/init.h>
 #include <linux/slab.h>
 #include <linux/module.h>
 #include <linux/usb.h>
 
 
 #define DRIVER_AUTHOR "Greg Kroah-Hartman, greg@kroah.com"
 #define DRIVER_DESC "USB LED Driver"
 
 #define VENDOR_ID       0x0fc5
 // #define PRODUCT_ID      0x1223
 #define PRODUCT_ID      0xB080
 #define INTERFACE_ID		0

 #define OFF 			0x07
 #define GREEN 		0x01
 #define YELLOW 	0x02
 #define RED 			0x04

 #define USB_CTRLMSG_BUFSZ 8
 
 /* table of devices that work with this driver */
 static struct usb_device_id id_table [] = {
         { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
         { },
 };
 MODULE_DEVICE_TABLE (usb, id_table);
 
 struct usb_led {
         struct usb_device *     udev;
         unsigned char           red;
         unsigned char           yellow;
         unsigned char           green;
 };
 
 static void change_color(struct usb_led *led)
 {
         int retval,i=0;
         unsigned char color = OFF;
         unsigned char *buffer;
 
         buffer = kmalloc(USB_CTRLMSG_BUFSZ, GFP_KERNEL);
         if (!buffer) {
                 dev_err(&led->udev->dev, "out of memory\n");
                 return;
         }
 
				 for (;i<USB_CTRLMSG_BUFSZ;) buffer[i++]=0;

				 buffer[0]=101;	 // from usbmon
				 buffer[1]=12;	 // from usbmon
				 buffer[3]=0xFF;	 // from usbmon
         if (led->yellow) {
                 color &= ~(YELLOW);
								 buffer[2]=YELLOW;
				 }
         if (led->red) {
                 color &= ~(RED);
								 buffer[2]=RED;
				 }
         if (led->green) {
                 color &= ~(GREEN);
								 buffer[2]=GREEN;
				 }

         dev_dbg(&led->udev->dev,
                 "yellow = %d, red = %d, green = %d, color = %.2x\n",
                 led->yellow, led->red, led->green, color);
 
         retval = usb_control_msg( 	led->udev,
                              			usb_sndctrlpipe(led->udev, 0),
                               			0x09,  // from usbmon
                               			0x21,  // from usbmon
                               			(0x03 * 0x100) + 0x1, // from usbmon
                               			(0x00 * 0x100) + color,
                               			buffer, 
                               			USB_CTRLMSG_BUFSZ,
                               			2000);

         if (retval)
                 dev_dbg(&led->udev->dev, "retval = %d\n", retval);

         kfree(buffer);
 }
 
 #define show_set(value) \
 static ssize_t show_##value(struct device *dev, struct device_attribute *attr, char *buf)               \
 {                                                                       \
         struct usb_interface *intf = to_usb_interface(dev);             \
         struct usb_led *led = usb_get_intfdata(intf);                   \
                                                                         \
         return sprintf(buf, "%d\n", led->value);                        \
 }                                                                       \
 static ssize_t set_##value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)    \
 {                                                                       \
         struct usb_interface *intf = to_usb_interface(dev);             \
         struct usb_led *led = usb_get_intfdata(intf);                   \
         int temp = simple_strtoul(buf, NULL, 10);                       \
                                                                         \
         led->value = temp;                                              \
         change_color(led);                                              \
         return count;                                                   \
 }                                                                       \
 static DEVICE_ATTR(value, S_IWUGO | S_IRUGO, show_##value, set_##value);
 show_set(yellow);
 show_set(red);
 show_set(green);

static int led_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
        struct usb_device *udev = interface_to_usbdev(interface);
        struct usb_led *dev = NULL;
        int retval = -ENOMEM;

				printk(">>>>>>> Step #1. probe starting! <<<<<");
        dev = kzalloc(sizeof(struct usb_led), GFP_KERNEL);
        if (dev == NULL) {
                dev_err(&interface->dev, "Out of memory\n");
                goto error_mem;
        }

        dev->udev = usb_get_dev(udev);

        usb_set_intfdata (interface, dev);

        retval = device_create_file(&interface->dev, &dev_attr_yellow);
        if (retval)
                goto error;
        retval = device_create_file(&interface->dev, &dev_attr_red);
        if (retval)
                goto error;
        retval = device_create_file(&interface->dev, &dev_attr_green);
        if (retval)
                goto error;

        dev_info(&interface->dev, "USB LED device now attached\n");
        return 0;

error:
				printk(">>>>>>> USB LED device not attached! <<<<<");
        device_remove_file(&interface->dev, &dev_attr_yellow);
        device_remove_file(&interface->dev, &dev_attr_red);
        device_remove_file(&interface->dev, &dev_attr_green);
        usb_set_intfdata (interface, NULL);
        usb_put_dev(dev->udev);
        kfree(dev);
error_mem:
        return retval;
}

static void led_disconnect(struct usb_interface *interface)
{
        struct usb_led *dev;

        dev = usb_get_intfdata (interface);

        device_remove_file(&interface->dev, &dev_attr_yellow);
        device_remove_file(&interface->dev, &dev_attr_red);
        device_remove_file(&interface->dev, &dev_attr_green);

        /* first remove the files, then set the pointer to NULL */
        usb_set_intfdata (interface, NULL);

        usb_put_dev(dev->udev);

        kfree(dev);

        dev_info(&interface->dev, "USB LED now disconnected\n");
}

static struct usb_driver led_driver = {
        .name =         "timingLEDdriver",
        .probe =        led_probe,
        .disconnect =   led_disconnect,
        .id_table =     id_table,
};

static int __init usb_led_init(void)
{
        int retval = 0;

        printk(">>>>>>>>>>>>>>>>>> usb_register");
        retval = usb_register(&led_driver);
        if (retval) {
                dev_err("usb_register failed. Error number %d", retval);
                printk(">>>>>>>>>>>>>>> usb_register failed. Error number %d", retval);
				}
        return retval;
}

static void __exit usb_led_exit(void)
{
        usb_deregister(&led_driver);
}

module_init (usb_led_init);
module_exit (usb_led_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

