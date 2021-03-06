#ifndef __pci_h__
#define __pci_h__
#include <typedef.h>
#define MAX_BUS 2  // 256
#define MAX_DEV 32 // 32
#define MAX_FUN 8  // 8
#define PCI_MSG_ERROR       -1
#define PCI_MSG_AVALIABLE   0x80
#define PCI_MSG_SUCCESSFUL  0
#define PCI_PORT_ADDR 0xCF8
#define PCI_PORT_DATA 0xCFC



#define CONFIG_ADDR(bus,device,fn,offset)\
                       (\
                       (((uint32_t)(bus) &0xff) << 16)|\
                       (((uint32_t)(device) &0x3f) << 11)|\
                       (((uint32_t)(fn) &0x07) << 8)|\
                       ((uint32_t)(offset) &0xfc)|0x80000000)

typedef struct pci {
	uint8_t fun;
	uint8_t dev;
	uint8_t bus;
	uint8_t rsvd;
}pci_t;

typedef struct stpci
{	uint16_t 	vid;
   	uint16_t 	did;
    	uint16_t 	cmd;
    	uint16_t 	sts;
    	uint8_t  	rid;
    	uint8_t  	pi;
    	uint8_t  	scc;
    	uint8_t  	bcc;
	uint8_t		cls;
    	uint8_t		lt;
    	uint8_t     	headtyp;
    	uint8_t     	BIST;
    	uint32_t    	bar0;
    	uint32_t    	bar1; 
    	uint32_t    	bar2;
    	uint32_t    	bar3;
    	uint32_t    	bar4;
    	uint32_t    	bar5;
	uint32_t    	ccisp;
    	uint16_t    	svid;
    	uint16_t    	sid;
	uint32_t    	exprom;  
    	uint16_t    	capptr;
	uint16_t    	rsv0;
    	uint8_t     	intln;
    	uint8_t     	intpn;
    	uint8_t     	mingnt;
    	uint8_t     	maxlat; 

}stpci_t;

uint32_t read_pci_config_addr(int bus,int dev,int fun, int offset);
void write_pci_config_addr(int bus,int dev,int fun, int offset,int data);
uint32_t pci_read_config_dword(int bus,int dev,int fun, int offset);
void pci_write_config_dword(int bus,int dev,int fun, int offset,int data);

void *pci_info(void *buffer,int max_bus);
uint32_t pci_scan_bcc(int bcc);
uint32_t pci_scan_bcc_scc(int bcc,int scc);
uint32_t pci_scan_bcc_scc_prog(int bcc,int scc,int prog);
uint32_t pci_scan_vendor(short vendor);
uint32_t pci_check_vendor(int bus,int dev, int fun, short vendor);

unsigned long pci_size(unsigned long base, unsigned long mask);

#endif
