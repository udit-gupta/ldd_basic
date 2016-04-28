#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h> 

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	#include <linux/config.h>  
#endif
	
int LO_open (struct net_device *dev) {
	printk("LO_open\n");
	netif_start_queue (dev);
	return 0;
}

int LO_release (struct net_device *dev) {
	printk ("LO_release\n");
	netif_stop_queue(dev);
	return 0;
}

static int LO_xmit (struct sk_buff *skb, struct net_device *dev) {
	printk ("LO_xmit\n");
	dev_kfree_skb(skb);
	return 0;
}

int LO_init (struct net_device *dev)
{
	dev->open = LO_open;
	dev->stop = LO_release;
	dev->hard_start_xmit = LO_xmit;
	printk ("LOdev_init\n");
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	struct net_device LO = {init: LO_init};
#endif

int LO_init_module (void)
{
	int err;

	strcpy (LO.name, "LO");
	if ((err = register_netdev (&LO))) {
		printk ("ERR(%d): LO",err);
		return err;
	}
	printk ("LO Module Init\n");
	return 0;
}
	
void LO_cleanup (void)
{
	printk ("LO Module Cleanup\n");
	unregister_netdev (&LO);
	return;
}

module_init (LO_init_module);
module_exit (LO_cleanup);
