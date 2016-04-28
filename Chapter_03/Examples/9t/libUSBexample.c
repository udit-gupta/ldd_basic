#include <usb.h>
#include <stdio.h>
#include <time.h>

// #define read x read x


/* the device's vendor and product id */
#define MY_VID 0x1307
#define MY_PID 0x0165

/* the device's endpoints */
#define EP_IN 0x81
#define EP_OUT 0x02

#define BUF_SIZE 12

usb_dev_handle *open_dev(void);

usb_dev_handle *open_dev(void)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  for(bus = usb_get_busses(); bus; bus = bus->next)
    {
      for(dev = bus->devices; dev; dev = dev->next)
        {
          if(dev->descriptor.idVendor == MY_VID
             && dev->descriptor.idProduct == MY_PID)
            {
              printf("Found Specified Device\n");  
              return usb_open(dev);
            }
           else
           {
               printf("Didn't Find Device\n");  
           }
        }
    }
  return NULL;
}

int main(void)
{
  usb_dev_handle *dev = NULL; /* the device handle */
  char tmp[BUF_SIZE] = "";
  int response;

	snprintf(tmp,BUF_SIZE,"Hello World!\n");
  usb_init(); /* initialize the library */
  usb_find_busses(); /* find all busses */
  usb_find_devices(); /* find all connected devices */


	if(!(dev = open_dev()))
	{
  	printf("error: device not found!\n");
  	system("read x");
  	return 0;
  }

  if(usb_set_configuration(dev, 1) < 0)
    {
      printf("error: setting config 1 failed\n");
      usb_close(dev);
      system("read x");
      return 0;
    }

  if(usb_claim_interface(dev, 0) < 0)
    {
      printf("error: claiming interface 0 failed\n");
      usb_close(dev);
      system("read x");
      return 0;
    }
  
  if((response=usb_bulk_write(dev, EP_OUT, tmp, sizeof(tmp), 5000))
     != sizeof(tmp))
	{
		printf("Arduino says: %d\n",response);
	}

  if(usb_bulk_read(dev, EP_IN, tmp, sizeof(tmp), 5000)
     != sizeof(tmp))
    {
      printf("error: bulk read failed\n");
      system("read x");
    }

  usb_release_interface(dev, 0);
  usb_close(dev);
  

  system("read x");
  return 0;
}
