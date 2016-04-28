   #define MY_PCI_DOMAIN_NUM 0x0001
   #define MY_PCI_BUS_NUM 0x00

   struct pci_bus my_pci_bus = NULL;
   struct pci_sysdata my_pci_sd;

   memset (&my_pci_sd, 0 sizeof (my_pci_sd));
   my_pci_sd.domain = MY_PCI_DOMAIN_NUM;
   my_pci_sd.node = -1;

   memset (&my_pci_ops, 0, sizeof (my_pci_ops));
   my_pci_ops.read = my_pci_read;
   my_pci_ops.write = my_pci_write;

   my_pci_bus = pci_scan_bus(MY_PCI_BUS_NUM, &my_pci_ops, NULL);

   if (my_pci_bus)
   {
      printk (KERN_INFO "Successfully created MY PCI bus");
   }
