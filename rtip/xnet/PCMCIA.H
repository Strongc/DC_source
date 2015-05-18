/*****************************************************************************
*Filename: PCMCIA.H - Defines & structures for PCMCIA
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren, 1993-2000
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
* Description:
*
*
*
*
****************************************************************************/

#ifndef __PCMCIA_ERTFS_H__
#define __PCMCIA_ERTFS_H__ 1

/* Constants used by pcmctrl.c   */

/* Memory regions which are used to map the CIS onto the host memory
   space. The first value is the 24 Bit linear physical address. The
   second value is this address a represented as a pointer in hosts
   logical address space. These values are peculiar to the cirrus logic
   6710, 6720 and intel 82365 compatible pcmcia bus controllers.
*/

/* WINDOW_0 - Used to access the CIS of slot 0   */

#define ISA_MEM_WINDOW_0   0xD0000L     /* ISA 24 bit physical address */
/*#define ISA_MEM_WINDOW_0   0xC8000L   */

/* WINDOW_1 - Used to access the CIS of slot 1   */
#define ISA_MEM_WINDOW_1   0xCC000L     /* ISA 24 bit physical address */

/* Which interrupt do we use for the pcmcia card event register. This
   value is peculiar to the cirrus logic 6710, 6720 and intel 82365
   compatible pcmcia bus controllers. */
#define MGMT_INTERRUPT 10

#define NSOCKET 2





#define PCMCIA_NO_DEVICE        0
#define PCMCIA_ATA_DEVICE       1
#define PCMCIA_LINKSYS_DEVICE   2
#define PCMCIA_3C589_DEVICE     3
#define PCMCIA_SMC_DEVICE       4
#define PCMCIA_SRAM_DEVICE      5
#define PCMCIA_SYMBOL_DEVICE    6
#define PCMCIA_INTERSIL_DEVICE  7
#define PCMCIA_AGERE_DEVICE     8
#define PCMCIA_TCFE574_DEVICE   9


/* File PCMCIA.C:   */
RTIP_BOOLEAN pcmcia_card_is_ata(int socket, IOADDRESS register_file_address,
        int interrupt_number, byte pcmcia_cfg_opt_value);
RTIP_BOOLEAN pcmcia_card_is_sram(int socket);
int  pcmcia_card_type(int socket);

#if (INCLUDE_RTIP)
RTIP_BOOLEAN pcmcia_card_is_linksys(int socket, int irq, int iobase);
RTIP_BOOLEAN pcmcia_card_is_smc(int socket, int irq, int iobase, PFBYTE phys_addr);
RTIP_BOOLEAN pcmcia_card_is_xircom(PIFACE pi, int socket, int irq, int iobase);
RTIP_BOOLEAN pcmcia_card_is_3com(int socket, int irq, int iobase, byte *phys_addr);
RTIP_BOOLEAN pcmcia_card_is_smc8041(int socket, int irq, int iobase);
RTIP_BOOLEAN card_is_ne2000(int socket, int irq, int iobase);
RTIP_BOOLEAN card_is_3com(int socket, int irq, int iobase, byte *phys_addr);

#if (INCLUDE_PCMCIA_PRISM)
RTIP_BOOLEAN card_is_symbol(int socket, int irq, int iobase, PFBYTE phys_addr);
RTIP_BOOLEAN card_is_intersil(int socket, int irq, int iobase, PFBYTE phys_addr);
RTIP_BOOLEAN card_is_agere(int socket, int irq, int iobase, PFBYTE phys_addr);

RTIP_BOOLEAN check_if_symbol(int socket);
RTIP_BOOLEAN check_if_intersil(int socket);
RTIP_BOOLEAN check_if_agere(int socket);
#endif

#if (INCLUDE_TCFE574_PCMCIA)
RTIP_BOOLEAN card_is_tcfe574(int socket, int irq, int iobase, byte *phys_addr);
#endif
#endif  /* INCLUDE_RTIP */

/* File PCMCTRL.C/PCC850.C:   */
void  mgmt_isr(int arg);
RTIP_BOOLEAN pcmctrl_init(void);
void pcmctrl_close(void);
void pcmctrl_map_3com(int socket, int irq, int iobase);
void pcmctrl_map_linksys_regs(int socket, int irq, int iobase);
void pcmctrl_map_ata_regs(int socket, IOADDRESS ioaddr, int interrupt_number);
RTIP_BOOLEAN pcmctrl_card_is_installed(int socket, RTIP_BOOLEAN apply_power);
RTIP_BOOLEAN pcmctrl_card_installed(int socket);
void pcmctrl_device_power_up(int socket);
void pcmctrl_card_down(int socket);
void pcmctrl_enable_io_mode(int socket);
void pcmctrl_put_cis_byte(int socket, dword offset, byte c);
byte pcmctrl_get_cis_byte(int socket, dword offset);
void pcmctrl_map_3com(int socket, int irq, int iobase);
RTIP_BOOLEAN pcmctrl_card_power_up(int socket);
void pcmctrl_device_power_up(int socket);
PFBYTE pd67xx_map_cis(int socket , dword offset);
PFBYTE pd67xx_map_sram(int socket , dword offset);


#endif

