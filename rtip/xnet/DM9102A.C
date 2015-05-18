/* DM9102A.C - DAVICOM DM9102A                                                 */
/*             PCI single chip fast ethernet NIC controller                    */
/*                                                                             */
/* EBS - RTIP                                                                  */
/*                                                                             */
/* Copyright EBSNet Inc, 2000                                                  */
/* All rights reserved.                                                        */
/* This code may not be redistributed in source or linkable object form        */
/* without the consent of its author.                                          */
/*                                                                             */
/* Technical contacts:                                                         */
/*         Shane Titus               Tom Van Oudenaren                         */
/*         shane@ebsnetinc.com       tom@ebsnetinc.com                         */
/*                                                                             */

#undef  DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"
#include "rtipapi.h"

#if (INCLUDE_DAVICOM)

#define CFG_DAVICOM_PCI 1 /* tbd */

#include "dm910x.h"
#if (CFG_DAVICOM_PCI)
    #include "pci.h"
#endif

#define DEBUG_DAVICOM 0

/* ********************************************************************     */
void davicom_interrupt(int minor_no);
void davicom_pre_interrupt(int minor_no);
void rtos32_hook_dav_interrupt(int irq, int minor);
void rtos32_unhook_dav_interrupt(int minor);
void timeout(void * vsc);
unsigned short davicom_read_srom_word(PDAVICOM_SOFTC softc, int offset);
void delay_usec(int no_usecs);
RTIP_BOOLEAN dm_auto_neg_via_phy(PDAVICOM_SOFTC softc);
word dm_phy_read(PDAVICOM_SOFTC softc, byte phy_addr, byte reg_no);
void dm_phy_write(PDAVICOM_SOFTC softc, byte phy_addr, byte reg_no, word data_write);

/* ********************************************************************     */
RTIP_BOOLEAN davicom_open(PIFACE pi);
void         davicom_close(PIFACE pi);
int          davicom_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN davicom_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN davicom_statistics(PIFACE  pi);
RTIP_BOOLEAN davicom_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************      */
/* GLOBAL DATA                                                               */
/* ********************************************************************      */
/* DAVICOM_SOFTC KS_FAR davicom_softc[CFG_NUM_DAVICOM];                      */
PDAVICOM_SOFTC KS_FAR davicom_softc[CFG_NUM_DAVICOM];

EDEVTABLE KS_FAR davicom_device =
{
     davicom_open, davicom_close, davicom_xmit, davicom_xmit_done,
     NULLP_FUNC, davicom_statistics, davicom_setmcast,
     DAVICOM_DEVICE, "DAVICOM", MINOR_0, ETHER_IFACE,
     SNMP_DEVICE_INFO(CFG_OID_DAVICOM, CFG_SPEED_DAVICOM)
     CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
     CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
     EN(0x300), EN(0x0), EN(5)
};
#endif  /* DECLARING_DATA */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************      */
/* EXTERNS                                                                   */
/* ********************************************************************      */
#if (!BUILD_NEW_BINARY)
extern PDAVICOM_SOFTC KS_FAR davicom_softc[CFG_NUM_DAVICOM];
extern EDEVTABLE KS_FAR davicom_device;
#endif

/* ********************************************************************      */
/* DEFINES                                                                   */
/* ********************************************************************      */
#define off_to_davicom_softc(X)  \
    (X) >= CFG_NUM_DAVICOM ? (PDAVICOM_SOFTC)0 : davicom_softc[(X)]
#define iface_to_davicom_softc(X) \
    (X)->minor_number >= CFG_NUM_DAVICOM ? (PDAVICOM_SOFTC)0 : davicom_softc[(X)->minor_number]

/* ********************************************************************      */
/* davicom_close() - close a device                                          */
/*                                                                           */
/* Perform device driver specific processing to disable sending and          */
/* receiving packets.                                                        */
/*                                                                           */
/* Inputs:                                                                   */
/*   pi - interface structure of the device to close                         */
/*                                                                           */
/* Returns: nothing                                                          */
/*                                                                           */

void davicom_close(PIFACE pi)                     /*__fn__*/
{
PDAVICOM_SOFTC softc;
int i;

    if (pi->minor_number >= CFG_NUM_DAVICOM)
    {
        set_errno( ENUMDEVICE );
        return;
    }

    softc = iface_to_davicom_softc(pi);
    if (!softc)
    {
        set_errno(ENUMDEVICE);
        return;
    }


    /* issue stop receive command     */
    WRITE_CR(DC_MODE_CR6, 0);

    /* stop interrupts   */
    WRITE_CR(DC_ISR_MASK_CR7, 0);

    /* free receive buffers     */
    for (i = 0; i < DC_RX_RING_SIZE; i++)
    {
        os_free_packet( softc->rx_dcus[i] );
    }

}

#if (CFG_DAVICOM_PCI)
/* *********************************************************************      */
/* PCI INITIALIZATION                                                         */
/* *********************************************************************      */
RTIP_BOOLEAN davicom_pci_init(PIFACE pi, PDAVICOM_SOFTC softc)
{
    unsigned char return_code;
    unsigned char BusNum;
    unsigned char DevFncNum;
    unsigned char default_irq;
    unsigned char byte_read;
    unsigned short word_read;
    unsigned long  dword_read;
    int Index;

    if (rtpci_bios_present())
    {
        /*                                                                          */
        /*  Find and initialize the first specified (Vendor, Device)PCI device .    */
        /*  Since auto-negotiation (DANAS (bit 10, BCR32 == 0) and                  */
        /*  ASEL (bit 1, BCR2 == 1)) is the default, it is assumed                  */
        /*  that sw need do nothing to force auto-negotiation to occur.             */
        /*  NOTE:  Add auto negotiation based on 3/1/99 talk with Nortel. VK        */
        /*                                                                          */
        /*  The index in the Find PCI Device indicates the instance of the          */
        /*  device to search for.                                                   */
        /*  The minor device number should indicate the instance of the device,     */
        /*    therefore index is set equal to minor device number.                  */
        /*                                                                          */
#        if (DEBUG_DAVICOM)
            DEBUG_ERROR("davicom_pci_init: PCI Index. minor_number=", EBS_INT1, pi->minor_number, 0);
#        endif

        Index = pi->minor_number;
#        if (DEBUG_DAVICOM)
            DEBUG_ERROR("davicom_pci_init: PCI Index. Index =", EBS_INT1, Index, 0);
#        endif

        return_code = rtpci_find_device(DC_DEVICE_ID, DC_VENDOR_ID,
                                        Index, &BusNum, &DevFncNum);
        if (return_code == RTPCI_K_SUCCESSFUL)
        {
            /*  Set the interrupt line based on the value in the               */
            /*  PCI Interrupt Line Register.                                   */
            /*  Note:  This writes a byte into an int location.  Any issues    */
            /*         here?                                                   */
            /*                                                                 */
#            if (DEBUG_DAVICOM)
                DEBUG_ERROR("davicom_pci_init: PCI Device found", 0, 0, 0);
#            endif

            /* *******************************************************      */
            /* read the irq                                                 */
            return_code = rtpci_read_byte(BusNum, DevFncNum, 
                                          RTPCI_REG_INT_LINE, &byte_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
#            if (DEBUG_DAVICOM)
                DEBUG_ERROR("davicom_pci_init: IRQ =", EBS_INT1, byte_read, 0);
#            endif

                if (byte_read == RTPCI_INT_LINE_NOVAL)
                {
                    if (pi->irq_val != -1)
                        default_irq = (unsigned char)pi->irq_val;
                    else
                        default_irq = CFG_DAVICOM_PCI_IRQ;

                    DEBUG_ERROR("davicom_pci_init: error: no IRQ assigned by BIOS, using ", EBS_INT1, default_irq, 0);

                    return_code = rtpci_write_byte(BusNum, DevFncNum, 
                                                   RTPCI_REG_INT_LINE, 
                                                   default_irq);
                    if (return_code != RTPCI_K_SUCCESSFUL)
                        softc->dav_irq = default_irq;
                }
                else
                    softc->dav_irq = byte_read;

            }
            else
                return(FALSE);  /* INTERRUPT LINE Register read failed */

            /*  Set PCI Latency Timer to 32                               */
/*          rtpci_write_byte(BusNum, DevFncNum, RTPCI_REG_LTNCY_TMR, 32); */

            /* *******************************************************                      */
            /*  Read the I/O Base Register or the Memory Mapped I/O                         */
            return_code = rtpci_read_dword(BusNum, DevFncNum, RTPCI_REG_IOBASE, 
                                          &dword_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                if (dword_read == RTPCI_IOBASE_NOVAL)
                {
                     DEBUG_ERROR("davicom_pci_init: IOBASE not avail", NOVAR, 0, 0);

                    /* if no address is present in the I/O base register,      */
                    /* set the default I/O base address based on the           */
                    /* user input i/o address (in demo/test programs)          */
                    /* or the default value from xnconf.h.                     */
                    if (pi->io_address != 0)
                        dword_read = pi->io_address;
                    return_code = rtpci_write_dword(BusNum, DevFncNum, 
                                                   RTPCI_REG_IOBASE, 
                                                   dword_read);
                    if (return_code == RTPCI_K_SUCCESSFUL)
                        softc->base_addr = (IOADDRESS)dword_read;
                    else
                        return(FALSE);  /* I/O BASE Register Write Failed */
                }
                else
                {
                    softc->base_addr = (IOADDRESS) (dword_read & RTPCI_M_IOBASE_L);
#                   if (DEBUG_DAVICOM)
                        DEBUG_ERROR("davicom_pci_init. base_addr=", DINT1, softc->base_addr, 0);
#                   endif
                }
            }

            /* *******************************************************      */
            /*  Write PCI Command Register enabling Bus Mastering           */
            /*  (BMEM) and enabling I/O accesses (IOEN).                    */
            return_code = rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_CMD, 
                                          &word_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                /* set I/O Space Access Enable (0x1) and       */
                /* Bus Master Enable(0x4)                      */
                word_read |= RTPCI_M_CMD_IOEN | RTPCI_M_CMD_BMEM;
                return_code = rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_CMD, word_read);
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /* COMMAND Register read/write failed */
        }
        else
        {
#          if (DEBUG_DAVICOM)
              DEBUG_ERROR("davicom_pci_init: rtpci_device_found failed", NOVAR, 0, 0);
#          endif
           return(FALSE);      /* No PCI Device detected. */
        }
    }
    else
    {
#        if (DEBUG_DAVICOM)
            DEBUG_ERROR("davicom_pci_init: no PCI BIOS present", NOVAR, 0, 0);
#        endif
        return(FALSE);      /* No PCI BIOS present. */
    }

    return(TRUE);   /* PCI Device Successfully initialized */
}
#endif /* CFG_DAVICOM_PCI */

/* ********************************************************************      */
/* davicom_open() - open a device                                            */
/*                                                                           */
/* Perform device driver specific processing to enable sending and           */
/* receiving packets.  Also probes for the device.                           */
/*                                                                           */
/* Inputs:                                                                   */
/*   pi - interface structure of the device to open                          */
/*                                                                           */
/* Returns: TRUE if successful, FALSE if failure                             */
/*                                                                           */

RTIP_BOOLEAN davicom_open(PIFACE pi)
{
PDAVICOM_SOFTC softc;
int  i;
unsigned short w;
PFBYTE p;

    if (davicom_softc[pi->minor_number] == NULL)
    {
       p = ks_malloc(sizeof(*softc)+16-1-4, PACKET_POOL_MALLOC, DRV_MALLOC);

       /* make sure on 16 byte boundary first         */
       while (((dword)p) & 15)
           p++;

       davicom_softc[pi->minor_number] = (PDAVICOM_SOFTC) p;
    }
    softc = iface_to_davicom_softc(pi);
    if (!softc)
    {
        set_errno(ENUMDEVICE);
        return (FALSE);
    }
    tc_memset((PFBYTE)softc, 0, sizeof(*softc));

    /* Set up the private data structure so that it points to the global interface      */
    /* structure. (address is needed later when returning packets)                      */
    softc->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(softc->stats);

    /* ************************************************************     */
#if (CFG_DAVICOM_PCI)
    /* read/write suitable values for the PCI configuration registers     */
    if (!davicom_pci_init(pi, softc))
    {
        DEBUG_ERROR("davicom_open: PCI register configuration failed", NOVAR, 0, 0);
        set_errno(EPROBEFAIL);      /* ??check for a PCI init failed error code?? */
        return(FALSE);
    }
#endif /* CFG_DAVICOM_PCI */

    /* ************************************************************      */
    /* reset the Davicom chip                                            */
    WRITE_CR(DC_SCR_CR0, DC_SCR_SW_RESET);

    /* wait at least 32 PCI clock cycles     */
    ks_sleep(2);

    /* ************************************************************      */
    /* read local Ethernet address from EEPROM                           */
    w = davicom_read_srom_word( softc, 10 );    /* was 20 */
    pi->addr.my_hw_addr[0] = (unsigned char) w;
    pi->addr.my_hw_addr[1] = (unsigned char) (w >> 8);
    w = davicom_read_srom_word( softc, 11 );    /* was 22 */
    pi->addr.my_hw_addr[2] = (unsigned char) w;
    pi->addr.my_hw_addr[3] = (unsigned char) (w >> 8);
    w = davicom_read_srom_word( softc, 12 );    /* was 24 */
    pi->addr.my_hw_addr[4] = (unsigned char) w;
    pi->addr.my_hw_addr[5] = (unsigned char) (w >> 8);
#if (DEBUG_DAVICOM)
    DEBUG_ERROR("ETHERNET ADDRESS: ", ETHERADDR, pi->addr.my_hw_addr, 0);
#endif
    /* ************************************************************      */
    /* create the Tx descriptors                                         */
    for (i=0; i < DC_TX_RING_SIZE; i++)
       softc->tx_desc[i].nxt_desc = (dword) (softc->tx_desc + ((i+1) & DC_TX_RING_MASK));

    /* create the Rx descriptors      */
    for (i=0; i < DC_RX_RING_SIZE; i++)
    {
        softc->rx_dcus[i] = os_alloc_packet_input(CFG_MAX_PACKETSIZE+4, DRIVER_ALLOC);
        if (!softc->rx_dcus[i])
        {
            DEBUG_ERROR("davicom_init: out of DCUs", NOVAR, 0, 0);
            return(set_errno(ENOPKTS));
        }

        softc->rx_desc[i].buffer = (dword) DCUTODATA(softc->rx_dcus[i]);
        softc->rx_desc[i].ctrl_flags = (CFG_MAX_PACKETSIZE+4) | (1<<24);

        softc->rx_desc[i].nxt_desc = (dword) (softc->rx_desc + ((i+1) & DC_RX_RING_MASK));
        softc->rx_desc[i].status = OWN_BIT;
    }

    /* write CR3 and CR4 to provide the starting address of each descriptor      */
    /* list                                                                      */
    WRITE_CR(DC_RX_BASE_ADDR_CR3, (dword)(softc->rx_desc));
    WRITE_CR(DC_TX_BASE_ADDR_CR4, (dword)(softc->tx_desc));

    /* ************************************************************      */
    /* Write CR0 to set global host bus operation parameters             */
    WRITE_CR( DC_SCR_CR0, 0 );

    /* ************************************************************      */
    /* hook the interrupt based up PCI values read                       */
    ks_hook_interrupt(softc->dav_irq, (PFVOID) pi, 
                      (RTIPINTFN_POINTER)davicom_interrupt,
                      (RTIPINTFN_POINTER)davicom_pre_interrupt,
                      pi->minor_number);

    /* ************************************************************      */
    /* write CR7 to mask causes of unnecessary interrupt                 */
    WRITE_CR(DC_ISR_MASK_CR7, 
        /* normal     */
        DC_IMR_NISE   |         /* Normal interrupt enable */
        DC_IMR_RXCIE  |         /* Receive complete interrupt */
/*      DC_IMR_TXDUE  |       */  /* Transmit buffer unavailable enabled */
        DC_IMR_TXCIE  |         /* Transmit complete interrupt enable */

        /* abnormal     */
        DC_IMR_AISE   |         /* Abnormal interrupt enable */
        DC_IMR_RXDUE  |         /* Receive buffer unavailable */
        DC_IMR_TXFUE);          /* Transmit fifo underrun enabled */

#define DC_STATUS_MASK (DC_ISR_RX_DONE  | DC_ISR_TX_DONE | DC_ISR_ABNORMAL | DC_ISR_TX_UNDERRUN | DC_ISR_RX_NOBUF)

#if (DEBUG_DAVICOM)
   DEBUG_ERROR("davicom_init: start rcv and xmit", NOVAR, 0, 0);
#endif

    /* ************************************************************
     * write CR6 to set global parameters and start both receive and transmit
     * processes; start receive
     * DC_MODE_TX_SC - start transmitter
     * DC_MODE_RX_RC - start receiver
     * DC_MODE_PAM - receive multicasts
     * DC_MODE_1_PKT
    */
    WRITE_CR(DC_MODE_CR6, (1<<26) |
       DC_MODE_SFT_XMIT | /* wait with transmit until all data is in fifo (disable threshold) */
       DC_MODE_1_PKT |    /* only one packet in transmit fifo */
       (3<<14)       |    /* max threshohld */
       DC_MODE_TX_SC |    /* start transmit */
/*     DC_MODE_PAM |      // receive multicasts   */
       DC_MODE_RX_RC |    /* start receive */
       0);

    /* ************************************************************     */

    softc->timer.func = timeout;   /* routine to execute every second */
    softc->timer.arg = softc;
    ebs_set_timer(&softc->timer, 1, TRUE);
    ebs_start_timer(&softc->timer);

    return(TRUE);
}

void timeout(void * vsc)
{
   PDAVICOM_SOFTC softc = vsc;

   if (softc->OutOfBuffers)
   {
      int i;

      softc->OutOfBuffers = 0;

      for (i=0; i<DC_RX_RING_SIZE; i++)
      {
         PDESCRIPTOR pdesc = softc->rx_desc + i;

         if (pdesc->buffer == 0)
         {
            DCU msg;

            softc->rx_dcus[i] = msg = os_alloc_packet_input(CFG_MAX_PACKETSIZE+4, DRIVER_ALLOC);
            if (msg)
            {
               pdesc->buffer = (dword) DCUTODATA(msg);
               pdesc->status = OWN_BIT;
#if DEBUG_DAVICOM
               DEBUG_ERROR("davicom_timeout: added new receive DCU", NOVAR, 0, 0);
#endif
            }
            else
            {
#if DEBUG_DAVICOM
               DEBUG_ERROR("davicom_timeout: out of DCUs", NOVAR, 0, 0);
#endif
            }
         }
      }

      /* get receiver going again   */
#if DEBUG_DAVICOM
      DEBUG_ERROR("davicom_timeout: restart receiver stalled at ", EBS_INT1, softc->this_rx, 0);
#endif
      softc->this_rx = 0;
      WRITE_CR(DC_RX_BASE_ADDR_CR3, (dword)(softc->rx_desc));
      WRITE_CR(DC_RX_START_CR2, 0xFFFFFFFF);
   }
   ebs_start_timer(&softc->timer);
}

/* ********************************************************************      */
/* INTERRUPT routines                                                        */
/* ********************************************************************      */
void davicom_pre_interrupt(int minor_no)
{
PDAVICOM_SOFTC softc;

    softc = off_to_davicom_softc(minor_no);

    if (!softc)
    {
        DEBUG_ERROR("davicom_pre_interrupt: no softc", NOVAR, 0, 0);
        return;
    }

    /* The isr will be masked on again when the strategy routine called      */
    /* from the interrupt task returns                                       */
    DRIVER_MASK_ISR_OFF(softc->dav_irq)
}


/* ********************************************************************         */
/* davicom_interrupt() - process driver interrupt                               */
/*                                                                              */
/* Processing routine for device driver interrupt.  It should process           */
/* transmit complete interrupts, input interrupts as well as any                */
/* indicating errors.                                                           */
/*                                                                              */
/* If an input packet arrives it is sent to the IP exchange (which              */
/* will signal the IP task to wake up).                                         */
/*                                                                              */
/* If an output xmit completed, it will wake up the IP tasks to                 */
/* process the output list.                                                     */
/*                                                                              */
/* Input:                                                                       */
/*   minor_number - minor number of device driver which caused the interrupt    */
/*                                                                              */
/* Returns: nothing                                                             */
/*                                                                              */

void davicom_interrupt(int minor_no)
{
dword status;
PDAVICOM_SOFTC softc;
PDESCRIPTOR pdesc;
DCU msg;
int pkt_len;

    softc = off_to_davicom_softc(minor_no);
    if (!softc)
        return;

     status = READ_CR(DC_STATUS_ISR_CR5);
     WRITE_CR(DC_STATUS_ISR_CR5, status);

     status &= DC_STATUS_MASK;  /* keep only bits we are interested in */

     if (status & DC_ISR_RX_DONE)
         while (1)
         {
            pdesc = softc->rx_desc + softc->this_rx;
            if (pdesc->buffer == 0)
               break;
            if (pdesc->status & OWN_BIT)
               break;
            if ((pdesc->status & (ES_BIT|PLE_BIT|AE_BIT)) == 0) /* no error */
            {
                pkt_len = (pdesc->status >> 16) & 2047;
                msg = softc->rx_dcus[softc->this_rx];

                /* set up length of packet; MUST be set to actual size      */
                /* not including crc etc even if allocated a larger         */
                /* packet above                                             */
                DCUTOPACKET(msg)->length = pkt_len;

                softc->stats.packets_in++;
                softc->stats.bytes_in += pkt_len;

                /* signal IP layer that a packet is on its exchange'      */
                /* send packet from ring buffer                           */
                ks_invoke_input(softc->iface, msg);

                /* replace current DCU from ring buffer just passed to      */
                /* IP task with a new DCU                                   */
                softc->rx_dcus[softc->this_rx] = msg = os_alloc_packet_input(CFG_MAX_PACKETSIZE+4, DRIVER_ALLOC);
                if (msg)
                   pdesc->buffer = (dword) DCUTODATA(msg);
                else
                {
                   pdesc->buffer = 0; /* out of buffers */
                   DEBUG_ERROR("davicom_interrupt: out of DCUs", NOVAR, 0, 0);
                }
            }
            else /* error receive, discard */
            {
                DEBUG_ERROR("davicom_interrupt: received corrupted packet, status ", EBS_INT1, pdesc->status, 0);
            }

             pdesc->status = pdesc->buffer ? OWN_BIT : 0;

             /* point to next input DCU in ring buffer     */
             softc->this_rx = (softc->this_rx + 1) & DC_RX_RING_MASK;
          }

      /* ************************************************************     */
      if (status & DC_ISR_TX_DONE)  /* assuming no deferments; that TDES0<0> is never asserted   */
          while (1)
          {
             pdesc = softc->tx_desc + softc->this_tx;

             if ((pdesc->ctrl_flags & CI_BIT) &&  /* complete interrupt */
                ((pdesc->status & OWN_BIT) == 0)) /* we own it */
             {
#if DEBUG_DAVICOM
/*                DEBUG_ERROR("interrupt, packet sent", NOVAR, 0, 0);   */
#endif
                ks_invoke_output(softc->iface, 1);
                pdesc->ctrl_flags = 0; /* mark as free */
                softc->this_tx = (softc->this_tx + 1) & DC_TX_RING_MASK;
             }
             else
               break;
         }

#if (DC_STATUS_MASK & DC_ISR_RX_NOBUF)
      /* ************************************************************      */
      /* check receive buffer is not available for device                  */
      if (status & DC_ISR_RX_NOBUF)
      {
          /* get receiver going again   */
          softc->OutOfBuffers = 1; /* have timer fix it later */
          DEBUG_ERROR("davicom_interrupt: out of receive buffers", NOVAR, 0, 0);
      }
#endif

      /* ************************************************************      */
      /* CHECK FOR ERRORS                                                  */
      /* ************************************************************      */

      /* TXPS - DC_ISR_TX_STOPPED         */
      /* TXJT - DC_ISR_TX_JABBER_TMO      */
      /* TXFU - DC_ISR_TX_UNDERRUN        */
      /* RXDU - DC_ISR_RX_NOBUF           */
      /* RXWT - DC_ISR_RX_WATCHDOG_TMO    */
      /* RXPS - DC_ISR_TX_EARLY           */
      /* SBE  - DC_ISR_BUS_ERROR          */
#if DEBUG_DAVICOM
      if (status & DC_ISR_ABNORMAL)
      {
/*          DEBUG_ERROR("davicom_interrupt: abnormal error", NOVAR, 0, 0);   */
          softc->stats.tx_other_errors++;
      }
#endif

      if (status & DC_ISR_TX_UNDERRUN)
      {
          DEBUG_ERROR("davicom_interrupt: transmitter underrun", NOVAR, 0, 0);
      }

    DRIVER_MASK_ISR_ON(softc->dav_irq);
}

/* ********************************************************************      */
/* davicom_xmit_done() - process a completed transmit                        */
/*                                                                           */
/* This routine is called as a result of the transmit complete               */
/* interrupt occuring (see ks_invoke_output).                                */
/*                                                                           */
/* Inputs:                                                                   */
/*   pi     - interface structure                                            */
/*   DCU    - packet transmitting                                            */
/*   status - TRUE indicates the xmit completed successfully, FALSE          */
/*            indicates it did not (possible errors include                  */
/*            timeout etc)                                                   */
/*                                                                           */
/* Returns: TRUE if xmit done or error                                       */
/*          FALSE if xmit not done; if it is not done when the               */
/*                next xmit interrupt occurs, davicom_xmit_done will         */
/*                be called again                                            */
/*                                                                           */

RTIP_BOOLEAN davicom_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PDAVICOM_SOFTC softc;

    softc = iface_to_davicom_softc(pi);
    if (!softc)
    {
        set_errno(ENUMDEVICE);
        return (FALSE);
    }

    if (!success)
    {
        softc->stats.errors_out++;
        softc->stats.tx_other_errors++;
        DEBUG_ERROR("davicom_xmit_done: error", NOVAR, 0, 0);
    }
    else
    {
        /* Update total number of successfully transmitted packets.     */
        softc->stats.packets_out++;
        softc->stats.bytes_out += DCUTOPACKET(msg)->length;
#if (DEBUG_DAVICOM)
/*        DEBUG_ERROR("davicom_xmit_done: success", NOVAR, 0, 0);   */
#endif
    }

    return(TRUE);
}

/* ********************************************************************      */
/* davicom_xmit() - transmit a packet                                        */
/*                                                                           */
/* Starts transmitting packet msg.  davicom_xmit_done will be called         */
/* when transmit complete interrupt occurs.                                  */
/*                                                                           */
/* Inputs:                                                                   */
/*   pi  - interface structure of the device to open                         */
/*   msg - packet to transmit where                                          */
/*         DCUTOPACKET(msg)->length - length of packet                       */
/*         DCUTODATA(msg)           - packet                                 */
/*                                                                           */
/* Returns: REPORT_XMIT_DONE if xmit is done                                 */
/*          0 if the transmit is started but not done                        */
/*          errno if an error occured                                        */
/*                                                                           */

int davicom_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
int next_tx;
PDESCRIPTOR pdesc;
int length;
PDAVICOM_SOFTC softc;

#if (DEBUG_DAVICOM)
/*    DEBUG_ERROR("davicom_xmit called", NOVAR, 0, 0);   */
#endif

    softc = iface_to_davicom_softc(pi);
    if (!softc)
    {
        DEBUG_ERROR("davicom_xmit: softc invalid", NOVAR, 0, 0);
        set_errno(ENUMDEVICE);
        return (ENUMDEVICE);
    }

    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)
        length = ETHER_MIN_LEN;
    if (length > (ETHERSIZE+4))
    {
        DEBUG_ERROR("xmit - length is too large, truncated", NOVAR, 0, 0);
        length = ETHERSIZE+4;         /* what a terriable hack! */
    }

    /* ************************************************************      */
    /* set the packet in the transmit FIFO                               */
    next_tx = softc->next_tx;
    pdesc = &(softc->tx_desc[softc->next_tx]);

    /* make sure the next buffer is available   */
    if (pdesc->ctrl_flags != 0)
    {
        DEBUG_ERROR("davicom_xmit: send buffer not free", NOVAR, 0, 0);
        return EOUTPUTFULL;
    }

    softc->next_tx = (softc->next_tx + 1) & DC_TX_RING_MASK;  /* Wrap to zero if must */
    softc->tx_dcus[next_tx] = msg;

    pdesc->buffer = (dword) DCUTODATA(msg);

    /* ************************************************************      */
    /* CI_BIT = set completion interrupt                                 */
    /* ED_BIT = ending descriptor                                        */
    /* BD_BIT = beginning descriptor                                     */
    /* CE_BIT = chain enable                                             */
    pdesc->ctrl_flags = length | CI_BIT | ED_BIT | BD_BIT | CE_BIT;

    /* enable   */
    pdesc->status = OWN_BIT;

    /* ************************************************************      */
    /* start the transmitter                                             */
    WRITE_CR(DC_TX_START_CR1, 0xffffffff);

    return 0;
}

/* ********************************************************************     */
RTIP_BOOLEAN davicom_setmcast(PIFACE pi)
{
   PDAVICOM_SOFTC softc = iface_to_davicom_softc(pi);

    WRITE_CR(DC_MODE_CR6, (1<<26) |
       DC_MODE_SFT_XMIT | /* wait with transmit until all data is in fifo (disable threshold) */
       DC_MODE_1_PKT |    /* only one packet in transmit fifo */
       (3<<14)       |    /* max threshohld */
       DC_MODE_TX_SC |    /* start transmit */
       DC_MODE_PAM |      /* receive multicasts */
       DC_MODE_RX_RC |    /* start receive */
       0);

    return(TRUE);
}

/* ********************************************************************      */
/* davicom_statistics() - update statistics                                  */
/*                                                                           */
/* Update statistics in interface structure which are kept in                */
/* a structure specific to this device driver                                */
/*                                                                           */
/* Returns: TRUE if successful FALSE if error                                */
/*                                                                           */

RTIP_BOOLEAN davicom_statistics(PIFACE pi)                       /*__fn__*/
{
   UPDATE_SET_INFO(pi,interface_packets_in, 0)
   UPDATE_SET_INFO(pi,interface_packets_out, 0)
   UPDATE_SET_INFO(pi,interface_bytes_in, 0)
   UPDATE_SET_INFO(pi,interface_bytes_out, 0)
   UPDATE_SET_INFO(pi,interface_errors_in, 0)
   UPDATE_SET_INFO(pi,interface_errors_out, 0)
   UPDATE_SET_INFO(pi,interface_packets_lost, 0)
   return(TRUE);
}

/*****************************************************************************      */
/* davicom_read_srom_word( DavicomSoftcType KS_FAR*, int )                          */
/*****************************************************************************      */
/*                                                                                  */

void SROMWriteClocked(PDAVICOM_SOFTC softc, dword data)
{
   WRITE_CR(DC_EX_MGMT_CR9, data | DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM);
   delay_usec(5);
   WRITE_CR(DC_EX_MGMT_CR9, data | DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM | CR9_CLOCK_TO_EEPROM);
   delay_usec(5);
   WRITE_CR(DC_EX_MGMT_CR9, data | DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM);
   delay_usec(5);
}

/* reads one word from the serial ROM offset in words     */

unsigned short davicom_read_srom_word(PDAVICOM_SOFTC softc, int offset)
{
   int i;
   word data = 0;

   WRITE_CR(DC_EX_MGMT_CR9, DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED);
   delay_usec(5);
   WRITE_CR(DC_EX_MGMT_CR9, DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM);
   delay_usec(5);

   /* write 110   */
   SROMWriteClocked(softc, CR9_DATA_IN_TO_EEPROM);
   SROMWriteClocked(softc, CR9_DATA_IN_TO_EEPROM);
   SROMWriteClocked(softc, 0);

   for (i=5; i>=0; i--)
      SROMWriteClocked(softc, (offset & (1<<i)) ? CR9_DATA_IN_TO_EEPROM : 0);

   WRITE_CR(DC_EX_MGMT_CR9, DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM);

   for (i=16; i>0; i--)
   {
      WRITE_CR(DC_EX_MGMT_CR9, DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM | CR9_CLOCK_TO_EEPROM);
      delay_usec(5);
      data = (data << 1) | ((READ_CR(DC_EX_MGMT_CR9) & CR9_DATA_OUT_FROM_EEPROM) ? 1 : 0);
      WRITE_CR(DC_EX_MGMT_CR9, DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED | CR9_CHIP_SELECT_TO_EEPROM);
      delay_usec(5);
   }

  WRITE_CR(DC_EX_MGMT_CR9, DM_SIO_ROMCTL_READ | CR9_EEPROM_SELECTED);
  return data;
}

/*****************************************************************************      */
/* UTILITY                                                                          */
/*****************************************************************************      */
void delay_usec(int no_usecs)
{
int i;

    for (i=0; i<no_usecs; i++)
        io_delay();
}

/*****************************************************************************      */
/* API - BIND                                                                       */
/*****************************************************************************      */
int xn_bind_davicom(int minor_number)
{
    return(xn_device_table_add(davicom_device.device_id,
                        minor_number,
                        davicom_device.iface_type,
                        davicom_device.device_name,
                        SNMP_DEVICE_INFO(davicom_device.media_mib,
                                         davicom_device.speed)
                        (DEV_OPEN)davicom_device.open,
                        (DEV_CLOSE)davicom_device.close,
                        (DEV_XMIT)davicom_device.xmit,
                        (DEV_XMIT_DONE)davicom_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)davicom_device.proc_interrupts,
                        (DEV_STATS)davicom_device.statistics,
                        (DEV_SETMCAST)davicom_device.setmcast));
}

#endif      /* DECLARING_DATA */
#endif      /* INCLUDE_DAVICOM */
