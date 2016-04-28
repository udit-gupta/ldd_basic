#include <linux/netdevice.h>

int readprocNIC(char *buf, char **start, off_t offset, int len, int *eof, int *data)
{
  char tmpBuf[128];
  int i,c=0;
  char *ethernetCardName="p1p1"; //"eth0","wlan0"(ubuntu),"p2p1"(fedora)

  struct net_device *netDevice;
  *eof = 1;

  netDevice = dev_get_by_name(&init_net, ethernetCardName);

  if (netDevice == NULL){
    sprintf(buf,"'%s' not present. Please verify.\n",ethernetCardName);
    return(strlen(buf));
  }
  // netDevice address is received; print MAC address from netDevice
  c=sprintf(tmpBuf,"Net-device '%s' MAC address = ", ethernetCardName);

  for (i = 0; i < netDevice->addr_len; i++)
    sprintf(&(tmpBuf[c+(3*i)]), "%02X:", (unsigned int)(netDevice->perm_addr[i]) );
  
  sprintf(&(tmpBuf[c+(3*i)+1]),"inpkts (Rx packets)= %ld\n", 
  			"outpkts(Tx packets)= %ld\n",
				"collisions = %ld\n", 
				"Rx bytes   = %ld    Tx bytes   = %ld\n", 
  			"Rx errors  = %ld    Tx errors  = %ld\n", 
  			"Rx dropped = %ld    Tx dropped = %ld\n", 
				"MTU = %d bytes\n",
  			"Number of Rx Queues = %d\n",
  			"Number of Tx Queues = %d\n",
  			"Max frames per queue allowed = %d\n",
  			"Interrupt IRQ number= %d\n",
				netDevice->stats.rx_packets,
				netDevice->stats.tx_packets,
				netDevice->stats.collisions,
        netDevice->stats.rx_bytes, netDevice->stats.tx_bytes,
        netDevice->stats.rx_errors, netDevice->stats.tx_errors,
        netDevice->stats.rx_dropped, netDevice->stats.tx_dropped,
				netDevice->mtu,
				netDevice->num_rx_queues,
				netDevice->num_tx_queues,
				netDevice->tx_queue_len,  // "Max frames per queue allowed = %d\n",
				netDevice->irq);					// "Interrupt IRQ number= %d\n",
		buf=tmpBuf;
	
  return(strlen(buf));
}
