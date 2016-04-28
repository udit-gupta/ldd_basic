// Example 13.1:  to demo reads from /proc file using USB interfaces
//    		 main.c

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/list.h>

#include <linux/slab.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <generated/utsrelease.h>
#else
#include <linux/utsrelease.h>
#endif

#include "main.h"

#define Log(a,b) printk(a,b)
#define LOG(a) printk(a)

extern struct mutex usb_bus_list_lock;
static struct list_head* list_usbdev;
LIST_HEAD(myUSBlist);

typedef struct USBdevinfo {
    int len;
    unsigned int hub:1;
    char* topology;
    char* bandwidth;
    char* descriptor;

    struct list_head list;
} USBdevinfo_t;

void free_usbdev_node(USBdevinfo_t*);

MODULE_LICENSE("GPL");

static int rc;
static char devnum;

static struct proc_dir_entry *proc_hello;

static const char *class_decode(const int class)
{       
        int ix;
        
        for (ix = 0; clas_info[ix].class != -1; ix++)
                if (clas_info[ix].class == class)
                        break;
        return clas_info[ix].class_name;
}       
        
static char *usb_dump_device_strings(char *start, char *end,
                                     struct usb_device *dev)
{       
        if (start > end)
                return start;
        if (dev->manufacturer)          
                start += sprintf(start, format_string_manufacturer,
                                 dev->manufacturer);
        if (start > end)                
                goto out;
        if (dev->product)
                start += sprintf(start, format_string_product, dev->product);
        if (start > end)
                goto out;
 out:
        return start;
}

static char *usb_dump_device_descriptor(char *start, char *end,
                                const struct usb_device_descriptor *desc)
{
        u16 bcdUSB = le16_to_cpu(desc->bcdUSB);
        u16 bcdDevice = le16_to_cpu(desc->bcdDevice);

        if (start > end) return start;
        start += sprintf(start, format_device1,
                          bcdUSB >> 8, bcdUSB & 0xff,
                          desc->bDeviceClass,
                          class_decode(desc->bDeviceClass),
                          desc->bDeviceSubClass,
                          desc->bDeviceProtocol,
                          desc->bMaxPacketSize0,
                          desc->bNumConfigurations);
        if (start > end) return start;
        start += sprintf(start, format_device2,
                         le16_to_cpu(desc->idVendor),
                         le16_to_cpu(desc->idProduct),
                         bcdDevice >> 8, bcdDevice & 0xff);
        return start;
}


static char *usb_dump_desc(char *start, char *end, struct usb_device *dev)
{
        if (start > end)
                return start;

        start = usb_dump_device_descriptor(start, end, &dev->descriptor);

        if (start > end)
                return start;

        start = usb_dump_device_strings(start, end, dev);
        return start;
}

static ssize_t getUSBinfo(struct list_head* device_info_list,
    struct usb_device *usbdev, struct usb_bus *bus,
    int level, int index, int count)
{
    char *scratch_buf;
    int ix;
    int len, ret = 0, cnt = 0;
    int parent_devnum = 0;
    char *data_end, *speed;
    USBdevinfo_t *new;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,5)
		// from /usr/src/linux/drivers/usb/core/devices.c
    struct usb_device *childdev = NULL;
#endif

    if (level > MAX_TOPO_LEVEL) {
  		Log("Invalid level: %d\n", level);
  		return -EINVAL;
    }

    if (usbdev->parent && usbdev->parent->devnum != -1)
  		parent_devnum = usbdev->parent->devnum;

 //       Allocate new info node
    new = (USBdevinfo_t*)kmalloc(sizeof(USBdevinfo_t), GFP_KERNEL);
    if (!new) {
  		LOG("Memory allocation for new info node failed\n");
  		return -ENOMEM;
    } 
		else {
  		memset(new, 0, sizeof(USBdevinfo_t));
    }

    //Allocate scratch buffer. Since this is a recursive function better not
    //    allocate buffer space on the stack.
    scratch_buf = (char*)__get_free_pages(GFP_KERNEL, 1);
    if (!scratch_buf) {
 			ret = -ENOMEM;
  		goto cleanup_new;
    }
    /*
 		* So the root hub's parent is 0 and any device that is
 		* plugged into the root hub has a parent of 0.
 		*/
    switch (usbdev->speed) {
  		case USB_SPEED_LOW: 
					speed = "1.5"; 
					break;
  		case USB_SPEED_UNKNOWN:   /* usb 1.1 root hub code */
  		case USB_SPEED_FULL:
      		speed = "12"; 
					break;
  		case USB_SPEED_WIRELESS:  /* Wireless has no real fixed speed */
  		case USB_SPEED_HIGH:
      		speed = "480"; 
					break;
  		case USB_SPEED_SUPER:
      		speed = "5000"; 
					break;
  		default:
      		speed = "??";
    }
    len = sprintf(scratch_buf, topology_str,
  				bus->busnum, level, parent_devnum,
  					index, count, usbdev->devnum,
  					speed, usbdev->maxchild);
    /*
 		 * level = topology-tier level;
 	   * parent_devnum = parent device number;
 		 * index = parent's connector number;
 	   * count = device count at this level
     */

    new->topology = (char*) kmalloc(len, GFP_KERNEL);
    if (!new->topology) {
  		LOG("topology alloc failed\n");
  		ret = -ENOMEM;
  		goto cleanup_new;
    }
    strcpy(new->topology, scratch_buf);
    new->len += len;

    /* If this is the root hub, display the bandwidth information */
    if (level == 0) {
  		int max;

  		new->hub = 1;

  		/* high speed reserves 80%, full/low reserves 90% */
  		if (usbdev->speed == USB_SPEED_HIGH)
      		max = 800;
  		else
      		max = FRAME_TIME_MAX_USECS_ALLOC;

  		/* report "average" periodic allocation over a microsecond.
 		 	* the schedules are actually bursty, HCDs need to deal with
 		 	* that and just compute/report this average.
 		 	*/
  		len = sprintf(scratch_buf, format_bandwidth,
      			bus->bandwidth_allocated, max,
      			(100 * bus->bandwidth_allocated + max / 2) / max,
      			bus->bandwidth_int_reqs,
      			bus->bandwidth_isoc_reqs);

  		new->bandwidth = (char*) kmalloc(len, GFP_KERNEL);
  		if (!new->bandwidth) {
     		// LOG("bandwidth descr alloc failed\n");
     		ret = -ENOMEM;
     		goto cleanup_new;
  		}

  		strcpy(new->bandwidth, scratch_buf);
  		new->len += len;
    }

    data_end = usb_dump_desc(scratch_buf, scratch_buf + (2 * PAGE_SIZE) - 256, usbdev);
    len = data_end - scratch_buf;
    new->descriptor = (char*)kmalloc(len, GFP_KERNEL);
    if (!new->descriptor) {
  		// LOG("descriptor alloc failed\n");
  		ret = -ENOMEM;
  		goto cleanup_new;
    }
    strcpy(new->descriptor, scratch_buf);
    new->len += len;

    // link our new node to the list */
    list_add(&new->list, &myUSBlist);

    // free_pages((unsigned long)pages_start, 1);
    free_pages((unsigned long)scratch_buf, 1);
    
    // look at all of this device's children. 
    if (usbdev->maxchild==0) {
    	// LOG("No more children!\n");
    	ret = 0;
    } 
		else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,5)
    	for (ix = 0; ix < usbdev->maxchild; ix++) {
    		struct usb_device *childdev = usbdev->children[ix];
#else
			// from /usr/src/linux/drivers/usb/core/devices.c
			usb_hub_for_each_child(usbdev, ix, childdev) {
#endif

      	if (childdev) {
					usb_lock_device(childdev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,5)
					ret = getUSBinfo(&myUSBlist,childdev,bus,level+1,ix,++cnt);
#else
			// from /usr/src/linux/drivers/usb/core/devices.c
					ret = getUSBinfo(&myUSBlist,childdev,bus,level+1,ix-1,++cnt);
#endif
    			usb_unlock_device(childdev);
    			if (ret < 0) {
        		// Log("usb_device_dump returned negative. Level %d\n", level);
        		break;
    			}
      	}
  		}
    }
    return ret;

 cleanup_new:
    free_usbdev_node(new);
    return ret;
}


ssize_t getUSBdevinfo(void) {
    struct usb_bus *bus;
    ssize_t total_written = 0;

    mutex_lock(&usb_bus_list_lock);

    /* print devices for all busses */
    list_for_each_entry(bus, &usb_bus_list, bus_list) {

  		/* recursive loop through all children of the root hub */
  		if (!bus->root_hub) continue;

  		usb_lock_device(bus->root_hub);
  		rc = getUSBinfo(&myUSBlist, bus->root_hub, bus, 0, 0, 0); 
  		usb_unlock_device(bus->root_hub);
  		if (rc < 0) {
				mutex_unlock(&usb_bus_list_lock);
      	return rc;
  		}
  	total_written += rc;
    }   

    mutex_unlock(&usb_bus_list_lock);
    return total_written;
}

void free_usbdev_node(USBdevinfo_t* node) {
    if(!node) return;

    if(node->topology) kfree(node->topology);
    if(node->bandwidth) kfree(node->bandwidth);
    kfree(node);
}


static int read_hello (char *buf, char **start, off_t offset, 
			int count, int *eof, void *data) {

   static char done;
   int len = 0;
   int rc;

   if(done) {
			// LOG("DONE, so returning 0. User process, go away!\n");
      done = 0;
      *eof = 1;
      return 0;
   }   

   if(offset == 0) {
			if ((rc = getUSBdevinfo())<0) {
       	printk("getUSBdevinfo PROBLEM. rc = %d\n", rc);
      }
   }
      
	while(!list_empty(&myUSBlist)) {
		USBdevinfo_t* usbdev;
    
		/* take first entry out of the list */
		list_usbdev= myUSBlist.next;
		list_del(list_usbdev);
		usbdev = list_entry(list_usbdev, USBdevinfo_t, list);
    
		Log("list-walk: found device info node %p\n", usbdev);
		*start = buf;
		len += sprintf(buf+len, "Device %d\n", devnum++);
		len += sprintf(buf+len, "topology: %s\n", usbdev->topology);
		if (usbdev->hub) {
			len += sprintf(buf+len, "bandwidth: %s\n", usbdev->bandwidth);
		}   
		len += sprintf(buf+len, "descriptor: %s\n", usbdev->descriptor);
		len += sprintf(buf+len, "-------------------------------------\n");
 
		/* free node */
		free_usbdev_node(usbdev);
    
			if(len > (count - 500)) {
			/* getting dangerously close to the end of buffer 
		   * let's return what we have
      */
			LOG("Getting close to end of buffer, return what we have");
     	break;
     }   
	}   
  
  // *eof = 1; //  END OF LIST;  *eof = 0; //  Loop again
	*eof=(list_empty(&myUSBlist)) ? 1 : 0;

	return len;
	// return(sprintf(buf,"Hello at %d\n", (int)jiffies));
}

static int my_init (void) {
	proc_hello = create_proc_entry("MyUSB",0,0);
	proc_hello->read_proc = read_hello;
	proc_hello->mode = S_IFREG | S_IRUGO;
    #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
        proc_hello->owner = THIS_MODULE;
    #endif      

	return 0;
}

static void my_exit (void) {
	if (proc_hello)
		remove_proc_entry ("MyUSB", 0);
}

module_init (my_init);
module_exit (my_exit);
