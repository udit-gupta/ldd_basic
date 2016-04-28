// Example 4.1:  to demo reads from /proc file
//    		 main.c

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/netdevice.h>

MODULE_LICENSE("GPL");

// Parameters: ifName - interface name
static char *ifName = "p1p1"; //"eth0","wlan0"(ubuntu),"p2p1"(fedora)
//
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,99)
   module_param(ifName, charp, 0); 
#else
   MODULE_PARM(ifName, "s");
#endif
//

static struct proc_dir_entry *proc_hello;

static int read_hello (char *buf, char **start, off_t offset, 
			int len, int *eof, void *unused) {

  int i,c=0;

  struct net_device *netDevice;
  *eof = 1;

  netDevice = dev_get_by_name(&init_net, ifName);

  if (netDevice == NULL){
    sprintf(buf,"'%s' not present. Please verify.\n",ifName);
    return(strlen(buf));
  }
  // netDevice address is received; print MAC address from netDevice
  c=sprintf(buf,"Net-device '%s' MAC address = ", ifName);

  for (i = 0; i < netDevice->addr_len; i++)
    sprintf(&(buf[c+(3*i)]), "%02X:", (unsigned int)(netDevice->dev_addr[i]) );

		c=sprintf(&(buf[c+(3*i)]),"\nRx packets=%ld Tx packets=%ld\nRx bytes=%ld Tx bytes=%ld\nRx errors=%ld Tx errors=%ld\nRx dropped=%ld Tx dropped=%ld\nRx Queues=%d Tx Queues = %d\ncollisions=%ld\nMTU=%d bytes\nMax frames per queue allowed = %ld\nInterrupt IRQ number= %d\n",
				netDevice->stats.rx_packets, netDevice->stats.tx_packets,
        netDevice->stats.rx_bytes, netDevice->stats.tx_bytes,
        netDevice->stats.rx_errors, netDevice->stats.tx_errors,
        netDevice->stats.rx_dropped, netDevice->stats.tx_dropped,
				netDevice->num_rx_queues, netDevice->num_tx_queues,
				netDevice->stats.collisions,
				netDevice->mtu,
				netDevice->tx_queue_len,  // "Max frames per queue allowed = %d\n",
				netDevice->irq);					// "Interrupt IRQ number= %d\n",
	
  return(strlen(buf));
}

static int my_init (void) {
	proc_hello = create_proc_entry("hello",0,0);
	proc_hello->read_proc = read_hello;

    #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
        proc_hello->owner = THIS_MODULE;
    #endif      

	return 0;
}

static void my_exit (void) {
	if (proc_hello)
		remove_proc_entry ("hello", 0);
}

module_init (my_init);
module_exit (my_exit);
