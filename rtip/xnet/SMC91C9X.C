/*                                                                      */
/* SMC91C9X.C - SMC 91C9X Device Driver                                 */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/* Module description:                                                  */

#undef  DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#if (INCLUDE_SMC91C9X)
#include "smc91c9x.h"

/* ********************************************************************   */
extern EthernetCtrlGetMacAddr_C(unsigned short*,
							  unsigned short*,
							  unsigned short*);	//JLA(IO), get MAC address from EthernetCtrl
#define DEBUG_SMC 0                     // FKA !!!!!!!!!
#define ZERO 0

/* ********************************************************************   */
RTIP_BOOLEAN smc91c9x_open(PIFACE pi);
void         smc91c9x_close(PIFACE pi);
int          smc91c9x_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN smc91c9x_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN smc91c9x_statistics(PIFACE  pi);
RTIP_BOOLEAN smc91c9x_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
_91_PARMS KS_FAR _91parms[CFG_NUM_SMCX];

EDEVTABLE KS_FAR smc91c9x_device =
{
    smc91c9x_open, smc91c9x_close, smc91c9x_xmit, smc91c9x_xmit_done,
    NULLP_FUNC, smc91c9x_statistics, smc91c9x_setmcast,
    SMC91C9X_DEVICE, "SMC91C9X", MINOR_0, ETHER_IFACE,
    SNMP_DEVICE_INFO(CFG_OID_SMC9X, CFG_SPEED_SMC9X)
    CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS,
    CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT,
    IOADD(0x300), EN(0), EN(5)
};

#endif  /* DECLARING_DATA|| BUILD_NEW_BINARY */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern _91_PARMS KS_FAR _91parms[CFG_NUM_SMCX];
extern EDEVTABLE KS_FAR smc91c9x_device;
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
/* for non PCMCIA applications, we may need to assign a MAC address. The FEAST ISA 110
   reference design has no EEPROM and must be assigned. The 100 has an EEPROM which should
   initialize the MAC address but you may want to override it*/
#define ASSIGN_MAC_ADDR 1 /* set to 1 to assign a MAC address */
#define MAC0 0x1200  /* This address goes out on the wire as 00123456789A */
#define MAC1 0x5634
#define MAC2 0x9A78

/* Set this to 1 to include 100 Base T support (do PHY probe and initialization)  */
#define PHY_SUPPORT 1
#if(PHY_SUPPORT)
unsigned char DetectPHY(P91_PARMS smc);
unsigned PHYAccess(unsigned char PHYAdd, unsigned char RegAdd,
                   unsigned char OPCode, unsigned xwData, P91_PARMS smc);
#endif

#define ZERO 0
#if (ZERO)
void dumpAllRegisters(P91_PARMS smc);
#endif

#define FILTER_SMC_INPUT  0 /* If true toss broadcasts not for us */

/* [i_a] settings for the driver   */
#define SMC_DO_MMU_RESET   0
#define SMC_LOOP_ON_ALLOC  0			//JLA IO Technologies A/S, 2005-08-17
										//In order to avoid infinite loop in line 777
										//sometimes when enabling and disabling network

/* ********************************************************************   */
void smc91c9x_pre_interrupt(int minor_no);
void smc91c9x_interrupt(int n);
void smc91c9x_getmcaf(PFBYTE mclist, int bytesmclist, PFBYTE af);
int  smc91c9x_xmit_2(PIFACE pi, DCU msg, int length);

/*******************************************************************   */
/***** Structure for making arrays of register and value pairs. ****   */
/*******************************************************************   */
typedef struct
{
  unsigned short int   reg;
  unsigned short int value;
} register_and_value_t;
/* ********************************************************************   */
/* I/O ROUTINES                                                           */
/* ********************************************************************   */

#define SMC91C9XWRITEBYTE(REG, VAL) OUTBYTE((smc->ia_iobase+(REG)), (VAL))
#define SMC91C9XREADBYTE(REG)       INBYTE(smc->ia_iobase+(REG))

#if (!KS_LITTLE_ENDIAN)
word wswap(word w)
{
word w1, w2;
    w1 = w >> 8;
    w2 = w << 8;
    return((word) w1|w2);
}
#endif

#if (ALWAYS_16BIT)
#define SMC91C9XREPOUT(p, l) outsw((IOADDRESS)(smc->ia_iobase+DATA_8_BK2), \
                                   (PFWORD) (p), l)
#define SMC91C9XREPIN(p, l) insw((IOADDRESS)(smc->ia_iobase+DATA_8_BK2), \
                                 (PFWORD) (p), l)
#else
#define SMC91C9XREPOUT(p, l)    \
if (smc->is_16bit)              \
    outsw((IOADDRESS)(smc->ia_iobase+DATA_8_BK2), (PFWORD) (p), l);\
else                            \
    outsb((IOADDRESS)(smc->ia_iobase+DATA_8_BK2), (PFBYTE) (p), l*2);

#define SMC91C9XREPIN(p, l)     \
if (smc->is_16bit)              \
    insw((IOADDRESS)(smc->ia_iobase+DATA_8_BK2), (PFWORD) (p), l);\
else                            \
    insb((IOADDRESS)(smc->ia_iobase+DATA_8_BK2), (PFBYTE) (p), l*2);
#endif


#if (ALWAYS_16BIT)
#define SMC91C9XREADWORD(REG) INWORD(smc->ia_iobase+(REG))
#define SMC91C9XREADDWORD(REG) INDWORD(smc->ia_iobase+(REG))
#define SMC91C9XWRITEWORD(REG, VAL) OUTWORD((smc->ia_iobase+(REG)), (VAL))
#else
#define SMC91C9XWRITEWORD(REG, VAL) smc91c9xwriteword(smc, REG, VAL)
void smc91c9xwriteword(P91_PARMS smc, word REG, word VAL)
{
    SMC91C9XWRITEBYTE(REG, VAL&0xff);
    SMC91C9XWRITEBYTE(REG+1, VAL>>8);
}

#define SMC91C9XREADWORD(REG) smc91c9xreadword(smc, REG)

word smc91c9xreadword(P91_PARMS smc,word REG)
{
word lo, hi;
    lo = (word) SMC91C9XREADBYTE(REG);
    hi = (word) SMC91C9XREADBYTE(REG+1);
    return((word)(lo | (hi <<8)));
}
#endif

/* ********************************************************************   */
/* MEMORY MANAGMENT                                                       */
/* ********************************************************************   */

P91_PARMS iface_to_p91(PIFACE pi)
{
int p91_off;

    p91_off = pi->minor_number;
    if (p91_off >= CFG_NUM_SMCX)
    {
        DEBUG_ERROR("iface_to_p91() - pi->minor_number, CFG_NUM_SMCX =",
            EBS_INT2, pi->minor_number, CFG_NUM_SMCX);
        return((P91_PARMS)0);
    }

    return((P91_PARMS) &_91parms[p91_off]);
}

/* ********************************************************************   */
/* MULTICAST                                                              */
/* ********************************************************************   */

/* smc91c9x_setmcast() -
   Takes an interface structures a contiguous array
   of bytes containing N IP multicast addresses and n, the number
   of addresses (not number of bytes).
   Copies the bytes to the driver structures multicast table and
   calls reset to set the multicast table in the board.
*/

RTIP_BOOLEAN smc91c9x_setmcast(PIFACE pi)       /* __fn__ */
{
word mcaf[4];   /* multicast address filter */
P91_PARMS smc;
int i;

    smc = iface_to_p91(pi);
    if (!smc)
        return(FALSE);

    /* if more multicast addresses than can filter, listen to all   */
    /* multicast addresses                                          */
    if (pi->mcast.lenmclist > 64)
    {
        /* listen to all multicasts (keep receive mode enabled)   */
        SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
        SMC91C9XWRITEWORD(RCR_4_BK0, PRMSC_B204|RXEN_B804|STRIP_B904);
    }
    else
    {
        /* do not listen to all multicasts (keep receive mode enabled)   */
        SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);      /* Select bank 0 */
        SMC91C9XWRITEWORD(RCR_4_BK0, RXEN_B804|STRIP_B904); /* Enable rcvs. strip CRC */

        /* Set multicast filter on chip.   */
        SMC91C9XWRITEWORD(BSR_E_BKALL, BANK3);  /* Select bank 3 */
        smc91c9x_getmcaf((PFBYTE) pi->mcast.mclist,
                         pi->mcast.lenmclist * ETH_ALEN, (PFBYTE)mcaf);
        for (i = 0; i < 4; i++)
            SMC91C9XWRITEWORD(i*2, mcaf[i]);
    }
    return(TRUE);
}


/* ********************************************************************   */
/* OPEN and CLOSE ROUTINES                                                */
/* ********************************************************************   */

void smc91c9x_close(PIFACE pi)                          /*__fn__*/
{
P91_PARMS smc;

    smc = iface_to_p91(pi);
    if (!smc)
        return;

    /* Mask off all interrupts   */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);  /* Select bank 2 */
    SMC91C9XWRITEBYTE(IMR_D_BK2, 0);
    /* Reset the mmu   */
    SMC91C9XWRITEWORD(MMU_0_BK2,MMU_RESET);
}



/* ********************************************************************   */
/* set board values to either values set by application or default values */
/* specified at top of file                                               */

void set_smcx_vals(PIFACE pi, P91_PARMS sc)
{
    sc->ia_iobase = pi->io_address;
    sc->ia_irq    = pi->irq_val;
}


RTIP_BOOLEAN smc91c9x_open(PIFACE pi)                           /*__fn__*/
{
PFWORD p;
int i;
P91_PARMS smc;
word wtemp;
int rev;
int chip;
#if (!KS_LITTLE_ENDIAN)
word w;
#endif
#if (INCLUDE_PCMCIA)
int smc_socket_number = 0;  /* init to keep compiler happy */
#endif
#if(PHY_SUPPORT)
unsigned char phyaddr;
unsigned int phydata;
#endif


    i = pi->minor_number;           /* Offset from first entry of same type */

    smc = iface_to_p91(pi);
    if (!smc)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    /* Find a parameter structure to use   */
    tc_memset(smc, 0, sizeof(_91parms[i]));

    /* set up parameters here     */
    /* TBD - Refine this later    */
    smc->board_no = i;

    /* set up pointers between iface and softc structures   */
    smc->pi = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(smc->stats);

#if (INCLUDE_PCMCIA)
    /* Check the slots to find the smc card   */
    /* Must be done before set_smcx_vals()    */
    if (pi->pdev->device_id == SMC91C9X_PCMCIA_DEVICE)
    {
        smc_socket_number = 1;          /* value for pcmcia_card_type(1) == 3 */
        if (pcmcia_card_type(0) == 3)
            smc_socket_number = 0;
        else if (pcmcia_card_type(1) != 3)
        {
            DEBUG_ERROR("Can't find smc card", NOVAR, 0, 0);
            set_errno(EPROBEFAIL);
            return(FALSE);
        }
    }
#endif
    set_smcx_vals(pi, smc);

#if (INCLUDE_PCMCIA)
    if ((pi->pdev->device_id == SMC91C9X_PCMCIA_DEVICE) &&
         !pcmcia_card_is_smc(smc_socket_number, smc->ia_irq, smc->ia_iobase, (PFBYTE) (pi->addr.my_hw_addr) ))  /* 1st parm is socket */
    {
        DEBUG_ERROR("card_is_smc failed", NOVAR, 0, 0);
        set_errno(EPROBEFAIL);
        return(FALSE);
    }
#endif

    /* Mask off all interrupts   */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);  /* Select bank 2 */
    SMC91C9XWRITEBYTE(IMR_D_BK2, 0);

    /* Reset the mmu   */
    SMC91C9XWRITEWORD(MMU_0_BK2,MMU_RESET);

    /* get revision from chip:
       0x33 = 91C92
       0x40 = 91C94
       0x41 = 91C94R1
       0x71 = 91C100
     */
    {
#if (ZERO)
      char *chiplist[] =
      {
        "??", /* 0 */
        "??", /* 1 */
        "??", /* 2 */
        "90/92", /* 3 */
        "94", /* 4 */
        "95", /* 5 */
        "96", /* 6 */
        "100", /* 7 */
        "100FD", /* 8 */
        "110", /* 9 */
        "??", /* A */
        "??", /* B */
        "??", /* C */
        "??", /* D */
        "??", /* E */
        "??"  /* F */
      };
#endif

      SMC91C9XWRITEWORD(BSR_E_BKALL, BANK3);  /* Select bank 3 */
      rev = SMC91C9XREADWORD(0x0A /* REVISION_BK3 */ );

      chip = (rev >> 4) & 0x0F;
#if (ZERO)
      rev &= 0x0F;
      /* set ifDescr to correct value including revision code!   */
      tc_sprintf(pi->pdev->device_name
                ,"SMC91C%s%c%d%s"
                ,chiplist[chip]
                ,(rev ? 'R' : '\0')
                ,rev
                ,(pi->pdev->device_id == SMC91C9X_PCMCIA_DEVICE ? " PCMCIA" : "")
                );

      DEBUG_ERROR("SMC CHIP NAME = "
                 ,STR1, pi->pdev->device_name, 0);

      DEBUG_ERROR("SMC CHIP REVISION [0x40=91C94, 0x41=91C94R1, 0x33=91C92] = "
                 ,EBS_INT1, rev & 0x00FF, 0);
#endif
    }

    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK1);  /* Select bank 1 */

    /*Read the configuration  register.   */
    wtemp = SMC91C9XREADWORD(CFG_0_BK1);

    if (wtemp & SMC_BIT7)
        smc->is_16bit = 1;
    /* set SMC to use TP, *not* AUI!   */
#if (ZERO)
/* This may be required for some ??     */
    /* set SMC to use TP, *not* AUI!    */
    {
      word st;

      st = SMC91C9XREADWORD(CFG_0_BK1);
      DEBUG_ERROR("SMC Configuration Register = ", EBS_INT1, st, 0);
      st &= ~AUI_B810;                     /* bit 8 = AUI interface bit */
      SMC91C9XWRITEWORD(CFG_0_BK1, st);

      DEBUG_ERROR("Using TP interface", NOVAR, 0, 0);
    }
#endif

    /* show memory layout of SMC chip (MIR) and set minimum TX memory to 2K   */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
    wtemp = SMC91C9XREADWORD(MIR_8_BK0);    /* MIR Memory Information Register */
#if (DEBUG_SMC)
    DEBUG_ERROR("MIR - Memory Information Register: Free Mem Avail, Mem Size = ", EBS_INT2, (wtemp & 0xFF00U), (wtemp & 0x00FFU) * 256);
#endif

    /*OS*/ /* ATTENTION! Memory Configuration Register doesn't exist in 91C111.
      It's address is used for new Receive/PHY Control register.
    */
    wtemp = SMC91C9XREADWORD(MCR_A_BK0);    /* MCR Memory Configuration Register */
#if (DEBUG_SMC)
    DEBUG_ERROR("MIR - Memory Configuration Register: M, Reserved For TRansmit = ", EBS_INT2, (wtemp >> 9) & 0x0007, (wtemp & 0x00FFU));
#endif

#if(ZERO)
    /*
     * [i_a] reserve 8*256*1 = 2KByte for transmission packets.
     * However, when we included this code, it was intended to get the SMC chip happy
     * while working with extreme high incoming traffic loads. Well... it's didn't help
     * a darn bit so we got this outa here asap as the buffer would only get reduced
     * to zero faster. So RX Overrun just happens sooner.
     * You could use this in test situations to allocate so much you can't receive
     * large packets anymore...
     */
    SMC91C9XWRITEWORD(MCR_A_BK0, (wtemp & 0xFF00U) | 8 /* 2K = 8 * 256 * M=1 */ );

    wtemp = SMC91C9XREADWORD(MCR_A_BK0);    /* MCR Memory Configuration Register */
    DEBUG_ERROR("MIR - (Modified) Memory Configuration Register: M, Reserved For TRansmit = ", EBS_INT2, (wtemp >> 9) & 0x0007, (wtemp & 0x00FFU));
#endif



#if (DEBUG_SMC)
   DEBUG_ERROR("Not reloading from eeprom", NOVAR, 0, 0);
#endif
   SMC91C9XWRITEWORD(BSR_E_BKALL, BANK1);  /* Select bank 1 */

#if (ZERO)
    /* tbd                                                          */
    /* this causes warm reset to fail, i.e. if just restart program */
    /* again, this code caused problems but taking this out also    */
    /* causes probe to succeed when it should not                   */
    /* Reload parameters from eeprom                                */
    /* tbd - if no eeprom provided we can configure it manually.    */
    SMC91C9XWRITEWORD(CNTRL_C_BK1,LOAD_B11C);
    /* Wait while the load finishes   */
    ks_sleep(2);
    if (SMC91C9XREADWORD(CNTRL_C_BK1) & LOAD_B11C)
    {
        DEBUG_ERROR("RELOAD FAILED - CONTINUE ANYWAY", NOVAR, 0, 0);
        return(FALSE);  /* Still high. not responding */
    }
#endif
    if (pi->pdev->device_id == SMC91C9X_PCMCIA_DEVICE)
    {
        /* Set the chip's ethernet address for pcmcia
           we read it from the CIS */
        p = (PFWORD) (pi->addr.my_hw_addr);
#if (!KS_LITTLE_ENDIAN)
/* TRY SWAPPING   */
        w = wswap(*p++);
        SMC91C9XWRITEWORD(IAR0_4_BK1, w);
        w = wswap(*p++);
        SMC91C9XWRITEWORD(IAR2_6_BK1, w);
        w = wswap(*p);
        SMC91C9XWRITEWORD(IAR4_8_BK1, w);
#else
        SMC91C9XWRITEWORD(IAR0_4_BK1, *p++);
        SMC91C9XWRITEWORD(IAR2_6_BK1, *p++);
        SMC91C9XWRITEWORD(IAR4_8_BK1, *p);
#endif
    }
    else
    {
#if (ASSIGN_MAC_ADDR)
        //SMC91C9XWRITEWORD(IAR0_4_BK1, MAC0);
        //SMC91C9XWRITEWORD(IAR2_6_BK1, MAC1);
        //SMC91C9XWRITEWORD(IAR4_8_BK1, MAC2);
        unsigned short mac0, mac1, mac2;			//JLA (IO), get MAC address
        EthernetCtrlGetMacAddr_C(&mac0, &mac1, &mac2);//from EthernetCtrl
        SMC91C9XWRITEWORD(IAR0_4_BK1, mac0);
        SMC91C9XWRITEWORD(IAR2_6_BK1, mac1);
        SMC91C9XWRITEWORD(IAR4_8_BK1, mac2);

#endif

        /* Read the chip's ethernet address   */
        p = (PFWORD) (pi->addr.my_hw_addr);
        *p++ = SMC91C9XREADWORD(IAR0_4_BK1);
        *p++ = SMC91C9XREADWORD(IAR2_6_BK1);
        *p   = SMC91C9XREADWORD(IAR4_8_BK1);
    }
#if(PHY_SUPPORT)
     if ((chip == 7) || (chip == 8) || (chip == 9)) /*100 base t chips must use PHY */
     {
        SMC91C9XWRITEWORD(BSR_E_BKALL, BANK3);  /* Select bank 3 */
        /*OS*/ /* ATTENTION! The inetrnal PHY address is 00000 in 91C111 */
        phyaddr = 0x00000; /*OS*/ /*was DetectPHY(smc); */
        if (phyaddr == 0xff)
      {
          set_errno(EPROBEFAIL);
          return(FALSE);
      }
/* Read in the PHY control register. This register should initialize to an isolated
   state with autonegotiate and full duplex. All we do here is reset the isolation bit
   to enable. If your PHY does not initialize , you can write your desired setup here. */
        phydata = PHYAccess(phyaddr,PHY_CR,OPRead,0,smc);
        phydata &=~PHY_CR_ISOLATE; /* turn off isolate bit */
        PHYAccess(phyaddr,PHY_CR,OPWrite,phydata,smc);

  phydata = PHYAccess(phyaddr,PHY_SR,OPRead,0,smc);
  phydata = PHYAccess(phyaddr,PHY_SR,OPRead,0,smc);

     }
#endif

////FKAs////////////////////////////////////////////////////////////////////////

 SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);      /* Select bank 0 */
 SMC91C9XWRITEWORD(0x000a,0x1010);  //0x10dc



///////////////////////////////////////////////////////////////////////////////

    /* Write transmit control register   */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
    SMC91C9XWRITEWORD(CNTRL_C_BK1,(TEENABLE_B51C|LEENABLE_B71C));

    /* Write receive control register   */
    SMC91C9XWRITEWORD(RCR_4_BK0, RXEN_B804|STRIP_B904); /* Enable rcvs strip crc*/
    ks_hook_interrupt(smc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)smc91c9x_interrupt,
                      (RTIPINTFN_POINTER) smc91c9x_pre_interrupt,
                      _91parms[i].board_no);

#if (ZERO)
  dumpAllRegisters(smc);
#endif

    /* Mask interrupts on   */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);  /* Select bank 2 */
    SMC91C9XWRITEBYTE(IMR_D_BK2,(IMR_RCVINT|IMR_TXINT|IMR_OVR|IMR_EPH));
    return (TRUE);
}

/* ********************************************************************   */
/* STATISTICS                                                             */
/* ********************************************************************   */

RTIP_BOOLEAN smc91c9x_statistics(PIFACE pi)                       /*__fn__*/
{
#if (INCLUDE_KEEP_STATS)
PETHER_STATS p;
P91_PARMS smc;

    smc = iface_to_p91(pi);
    if (!smc)
        return(FALSE);

   p = (PETHER_STATS) &(smc->stats);
   UPDATE_SET_INFO(pi,interface_packets_in, p->packets_in)
   UPDATE_SET_INFO(pi,interface_packets_out, p->packets_out)
   UPDATE_SET_INFO(pi,interface_bytes_in, p->bytes_in)
   UPDATE_SET_INFO(pi,interface_bytes_out, p->bytes_out)
   UPDATE_SET_INFO(pi,interface_errors_in, p->errors_in)
   UPDATE_SET_INFO(pi,interface_errors_out, p->errors_out)
   UPDATE_SET_INFO(pi,interface_packets_lost, p->packets_lost)
#else
    ARGSUSED_PVOID(pi)
#endif
   return(TRUE);
}

/* ********************************************************************   */
/* XMIT ROUTINES                                                          */
/* ********************************************************************   */
/* For the smc91c9x driver, for one packet xmit, the first interrupt      */
/* which causes in the first call to smc91c9x_xmit_done,                  */
/* specifies the alloc is available;                                      */
/* the second interrupt, i.e. the second interrupt which causes           */
/* the second call to smc91c9x_xmit_done, specifies the                   */
/* xmit is complete                                                       */
/*                                                                        */
/* Returns TRUE if xmit done or error                                     */
/* Returns FALSE if xmit not done; if it is not done when the             */
/*   next xmit interrupt occurs, smc91c9x_xmit_done will be called again  */
/*                                                                        */
RTIP_BOOLEAN smc91c9x_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
P91_PARMS smc;
#if (!SMC_LOOP_ON_ALLOC)
int ret;
#endif

    smc = iface_to_p91(pi);
    if (!smc)
        return(TRUE);

    DEBUG_LOG("semc91c9x_xmit_done(); success [input] = ", LEVEL_3,
                  EBS_INT1, success, 0);
    if (success)
    {
#if !(SMC_LOOP_ON_ALLOC)
        if (smc->wait_alloc)
        {
#if (DEBUG_SEND_NO_BLOCK)
            DEBUG_ERROR("smc91c9x_xmit_done: wait_alloc done", NOVAR, 0, 0);
#endif
            smc->wait_alloc = FALSE; /* [i_a] bugfix: RESET alloc request
                                      * once it has been resolved. Or the SMC
                                      * driver will REtrigger the transmission
                                      * of this same packet multiple times
                                      * erroneously!
                                      */
            ret = smc91c9x_xmit_2(pi, msg, DCUTOPACKET(msg)->length);

            if (!pi->xmit_status)
            {
                pi->xmit_status = ret;
            }
            return !!ret;

            /* [i_a] if xmit_2 fails, we won't wait for a xmit signal as it'll never
             * come (network failure!). We just need to discard the message asap.
             *
             * When to wait? if xmit_2() is OK (0) --> return !!retval
             * Otherwise don't wait & throw msg away.
             *
             * we also set 'pi->xmit_status' as it has been set by smc91c9x_xmit() to 0
             * but might be set to some errorvalue now as transmission has been deferred.
             * We only set 'pi->xmit_status' if it's still '0' (OK)...
             */
        }
        else
#endif /* #if (SMC_LOOP_ON_ALLOC) */
        {
#if (DEBUG_SEND_NO_BLOCK)
            DEBUG_LOG("smc91c9x_xmit_done: xmit done", LEVEL_3, NOVAR, 0, 0);
#endif
            /* Update total number of successfully transmitted packets.     */
            smc->stats.packets_out++;
            smc->stats.bytes_out += DCUTOPACKET(msg)->length;
        }
    }
    else        /* failure */
    {
#if (DEBUG_SEND_NO_BLOCK)
        DEBUG_ERROR("smc91c9x_xmit_done: failure", NOVAR, 0, 0);
#endif

#if !(SMC_LOOP_ON_ALLOC)
        if (smc->wait_alloc)
        {
            smc->wait_alloc = FALSE; /* [i_a] bugfix: RESET alloc request
                                      * once it has been resolved, even when
                                      * it failed. Once we get here, it's probably
                                      * better to reboot the system as the driver
                                      * has gone insane by now???
                                      */
            smc->diag_alloc_isr_fail += 1;
            /* Bad news. reset the mmu.     */
#if SMC_DO_MMU_RESET
            /* [i_a] RESETting the MMU will cause the SMC chip to loose track of what's happening.
             * Meanwhile we will happily continue to receive/transmit packets that are (being) stored
             * anywhere in the SMC cache. By resetting the MMU we'll have lost our 'ALLOC' requested
             * space for packets that are to be transmitted, while the SMC chip receiver can
             * bloody well overwrite the same cache byrtes we're filling up too.
             *
             * In other words: MMU_RESET should only be done when terminating regular operations in the
             * sense that the machine will reboot or re-initialize the TCP/IP stack. NO PACKET may be
             * written to the SMC chip during an MMU_RESET as it may well harm an incoming packet.
             * This implies MMU_RESET is a VERY DANGEROUS command inside the interrupt handler...
             */
            SMC91C9XWRITEWORD(MMU_0_BK2,MMU_RESET);
#else
            if (!pi->xmit_status)
            {
              pi->xmit_status = ENETDOWN;
            }
#endif
            smc->stats.errors_out++;
        }
        else
#endif /* #if SMC_LOOP_ON_ALLOC */
        {
            smc->stats.errors_out++;
        }
    }
    return(TRUE);       /* xmit done */
}


/* ********************************************************************   */
/* ********************************************************************   */
int smc91c9x_xmit(PIFACE pi, DCU msg)
{
byte  int_status;
#if (!SMC_LOOP_ON_ALLOC)
byte  btemp;
#endif
int   length;
word  wtemp;
int   i;
P91_PARMS smc;

    smc = iface_to_p91(pi);
    if (!smc)
        return(ENUMDEVICE);

    length = DCUTOPACKET(msg)->length;

    /* make sure packet is within legal size   */
    if (length < ETHER_MIN_LEN)
        length = ETHER_MIN_LEN;
    if (length > ETHERSIZE)
    {
        DEBUG_ERROR("xmit - length is too large", NOVAR, 0, 0);
        length = ETHERSIZE;       /* what a terriable hack! */
    }

    /* Make sure TXEN is on   */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
    wtemp = SMC91C9XREADWORD(TCR_0_BK0);    /* Transmit control register */
    wtemp |= TXEN_B000;
    SMC91C9XWRITEWORD(TCR_0_BK0, wtemp);   /* Transmit control register */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);      /* Select bank 2 */

    smc->diag_alloc_tries += 1;

    SMC91C9XWRITEWORD(MMU_0_BK2,ALLOCTX(length+6));/* issue alloc pkt command */
                                                               /* The +6 is for the     */
                                                               /* status, length and    */
                                                               /* Control words         */
    for (i = 0; i < N_ALLOC_POLLS; i++)            /* poll for completes */
    {
        int_status = SMC91C9XREADBYTE(ISR_C_BK2);
        if (int_status & ISR_ALLOC)
        {
            smc->diag_alloc_poll_suc += 1;
            break;
        }

    }
#if (SMC_LOOP_ON_ALLOC)
   if (!(int_status & ISR_ALLOC))
   {
    /* [i_a] Peter suggested a different type of fix: remove the 'wait_alloc'
       * variable completely and just sit and wait here for half a sec to see
       * if the SMC chip signals ready. If not, network is down anyhow.
       */



      for (;;i++)            /* poll for completes */
      {
        int_status = SMC91C9XREADBYTE(ISR_C_BK2);
        if (int_status & ISR_ALLOC)
        {
          smc->diag_alloc_poll_suc += 1;
          break;
        }

        {
          DEBUG_ERROR("ALLOC REQUEST pending (indefinitely)?", EBS_INT1, i, 0);
          ks_sleep((word)(ks_ticks_p_sec() >> 2)); /* 0.25 secs waiting */
        }
      }
    }
#else
    if (!(int_status & ISR_ALLOC))
    {
      /* Polling didn't produce. Wait for in interrupt     */
      ks_disable();

      /* Enable the alloc interrupt and wait for it     */
      btemp = SMC91C9XREADBYTE(IMR_D_BK2);
      btemp |= IMR_ALLOC;
      SMC91C9XWRITEBYTE(IMR_D_BK2, btemp);

      /* the first wait is for the alloc (the second wait is for the send      */
      /* to complete)                                                          */
      smc->wait_alloc = TRUE;

      /* [i_a] enable interrupts only if we're sure 'wait_alloc' has been set too!   */
      ks_enable();

      return(0);      /* wait for interrupt */
    }
#endif

    return(smc91c9x_xmit_2(pi, msg, length));
}

int smc91c9x_xmit_2(PIFACE pi, DCU msg, int length)
{
PFBYTE packet;
P91_PARMS smc;
byte  btemp;
union /* [i_a] bugfix: packet[length+xxxx] write would write beyond
       * allocated space (spurious as packets are allocated from pools
       * which have fixed sized chunks inside - as far as I can see...) */
{
    byte b[2];
    word w;
} padding;

    smc = iface_to_p91(pi);
    if (!smc)
        return(ENUMDEVICE);

    packet = DCUTODATA(msg);

    /* If we get here we think we have a packet   */
    btemp = SMC91C9XREADBYTE(ALLOC_3_BK2); /* Allocation register */
    if (btemp & SMC_BIT7)
    {
        smc->diag_alloc_alloc_reg_fail += 1;
        /* Bad news. reset the mmu.   */
#if SMC_DO_MMU_RESET
        /* [i_a] RESETting the MMU will cause the SMC chip to loose track of what's happening.
         * Meanwhile we will happily continue to receive/transmit packets that are (being) stored
         * anywhere in the SMC cache. By resetting the MMU we'll have lost our 'ALLOC' requested
         * space for packets that are to be transmitted, while the SMC chip receiver can
         * bloody well overwrite the same cache bytes we're filling up too.
         *
         * In other words: MMU_RESET should only be done when terminating regular operations in the
         * sense that the machine will reboot or re-initialize the TCP/IP stack. NO PACKET may be
         * written to the SMC chip during an MMU_RESET as it may well harm an incoming packet.
         * This implies MMU_RESET is a VERY DANGEROUS command inside the interrupt handler...
         */
        SMC91C9XWRITEWORD(MMU_0_BK2,MMU_RESET);
#endif
        smc->stats.errors_out++;
        return(ENETDOWN);
    }
    /* Set the packet number in tx area.   */
    ks_disable();
    /* [i_a] make sure the PNR & PTR registers are set without the interrupt handler messing things up.
     * So we disable interrupts here as a safety measure. It's not mandatory (SMC documentation)
     * though I feel it's necessary anyway...
     */

    SMC91C9XWRITEWORD(PNR_2_BK2, btemp);
    /* Set the pointer register so we can write to it   */
    SMC91C9XWRITEWORD(PTR_6_BK2, WRITE_XMIT(0));
    ks_enable();

    /* Write the packet memory ..                                         */
    /* Write the status word  (0) into 91C9X memory. increment pointer    */
    SMC91C9XWRITEWORD(DATA_8_BK2, 0);
    /* [i_a] the 'padding' union was introduced as I've found that rtip would overwrite
     * the next byte of another packet if this packet happens to be of a size identical to
     * the packet-size of the pool it's located in (e.g. 128 byte packet in the 128 byte pool)
     * packet[length+1] = 0
     * would nuke the neighbouring packet in a nice way that's extremely hard to track down
     * as the bug can't be easily reproduced.
     *
     * So now we don't overwrite the word beyond the packet but do a nice&clean construct
     * + i/o write or the last word to be written to the smc chip.
     */
    if (length & 0x0001) /* odd/even length? */
    {
        /* Flip these in big endian mode   */
#if (KS_LITTLE_ENDIAN)          /* INTEL */
        padding.b[0] = packet[length-1];
        padding.b[1] = SMC_BIT5;
#else
        padding.b[0] = SMC_BIT5;
        padding.b[1] = packet[length-1];
#endif

        length = (word)(length + 1);    /* This many bytes */
    }
    else
    {
        padding.b[0] = 0;
        padding.b[1] = 0;

        length = (word)(length + 2);    /* Add control word */
    }
    /* Write the length into 91C9X memory. increment pointer     */
    SMC91C9XWRITEWORD(DATA_8_BK2, (word)(length+4));

    SMC91C9XREPOUT(packet, ((length-2)>>1));  /* Burst transfer */
    SMC91C9XWRITEWORD(DATA_8_BK2, padding.w);

    SMC91C9XWRITEWORD(MMU_0_BK2,MMU_NQTX); /* move pkt to xmit queue */

    return(0);      /* Ok if we get here */
}

/* ********************************************************************   */
/* INTERRUPT ROUTINES                                                     */
/* ********************************************************************   */

#if (FILTER_SMC_INPUT)

/* ********************************************************************       */
typedef struct
{
  word length;
  word stored_length;
  struct
  {
    ETHER ether;
    union
    {
      ARPPKT arp;
      struct
      {
        IPPKT ip;
        BYTE extra[32]; /* enough to fit TCP/UDP/ICMP/IGMP packet (we hope) */
      } ip;
    } pkt;
  } data;
} ETH_FILTER_T;



/* ********************************************************************        */
/* Received packet filter.                                                     */
/*                                                                             */
/* This routine is called when an ethernet packet is received to filter out    */
/* unwanted packets. Only ARP and IP packets aimed at us are accepted.         */
/* Return -1 to indicate filter, 0 for packet accept.                          */
/*                                                                             */
int smc91c9x_filter(PIFACE pi, ETH_FILTER_T *msg)
{
  PETHER pe;
  word pkt_type;
  int ret = 0;

  pe = &msg->data.ether;
#if (SHOW_SNIFF_DATA)
  DEBUG_LOG("smcx filter - eth src", LEVEL_3, ETHERADDR, pe->eth_source, 0);
  DEBUG_LOG("smcx filter - eth dst", LEVEL_3, ETHERADDR, pe->eth_dest, 0);
#endif

  pkt_type = pe->eth_type;
  switch (pkt_type)
  {
  case EIP_68K:
    {
      PIPPKT  pip;

      pip = &msg->data.pkt.ip.ip;

      if (tc_cmp4(pi->addr.my_ip_addr, pip->ip_dest, IP_ALEN))
      {
        ret = 0;   /* accept ip packets addressed to us */
      }
      else
      {
        ret = -1;
      }
    }
    break;

  case EARP_68K:
    {
      PARPPKT pa;

      /* Toss arp requests if they are not for us   */
      pa = &msg->data.pkt.arp;

      if ((pa->ar_opcode == ARPREQ_68K) && !tc_cmp4(pi->addr.my_ip_addr, pa->ar_tpa, IP_ALEN) )
      {
        ret = -1;
      }
      else
      {
        ret = 0;
      }
    }
    break;

  case ERARP_68K:
    /* assume these rarely happen so let it pass   */
    ret = 0;
    break;

  default:

    ret = -1;
    break;
  }

  return ret;
}

#endif /* (FILTER_SMC_INPUT) */


/* ********************************************************************       */
void smc91c9x_pre_interrupt(int minor_no)
{
P91_PARMS smc;

    smc = (P91_PARMS)&(_91parms[minor_no]);
    if (!smc)
        return;

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(smc->ia_irq)
}

void smc91c9x_interrupt(int n)      /*__fn__*/
{
word save_bank, save_ptr, pkt_save, pkt_no, wtemp;
byte  int_status, imr;
word status;
word len;
DCU msg;
PFBYTE msgdata;
P91_PARMS smc;
#if (FILTER_SMC_INPUT)
ETH_FILTER_T eth_filter_data;
#endif

    smc = (P91_PARMS)&(_91parms[n]);

    save_bank = SMC91C9XREADWORD(BSR_E_BKALL);  /* Save bank reg. */
    SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);      /* Select bank 2 */
    imr = SMC91C9XREADBYTE(IMR_D_BK2);          /* Save interrupt mask */
    SMC91C9XWRITEBYTE(IMR_D_BK2, 0);            /* Mask all off */

    save_ptr = SMC91C9XREADWORD(PTR_6_BK2);     /* Save pntr reg */

    int_status = (byte)(SMC91C9XREADBYTE(ISR_C_BK2) & imr);
    {
        while (int_status &
               /* [i_a] also handle 'rx overrun' here as it may happen too if we're looping
                * already. So handle _everything_ inside this loop; no special treatment for
                * anyone!
                */
               (ISR_OVR | ISR_RCVINT | ISR_TXINT    | ISR_TXEMT    | ISR_ALLOC | ISR_EPH))
        {
            /* [i_a] process EPH condition signalling _before_ anything else as RX/TX et al might
             * just clear your precious EPH status signalling and miss on your error diagnostics
             * update (ie. counters staying at zero while you *know* bad stuff is happenin')
             */

            /* SPECIAL CONDITION     */
            if (int_status & ISR_EPH)
            {

                SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
                wtemp = SMC91C9XREADWORD(EPH_2_BK0);    /* EPH Status */
                if (wtemp & SNGL_B102)
                    smc->stats.one_collision++;
                if (wtemp & MULT_B202)
                    smc->stats.multiple_collisions++;
                if (wtemp & COL16_B402)
                    smc->stats.collision_errors++;
                if (wtemp & SQET_B502)
                    smc->stats.tx_sqe_errors++;
                if (wtemp & LATCOL_B902)
                    smc->stats.owc_collision++;
                if (wtemp & LOSTC_BA02)
                    smc->stats.tx_carrier_errors++;
                if (wtemp & EXCDEF_BB02)
                    smc->stats.tx_delayed++;
                if (wtemp & RXOVRN_BD02)
                {
                    smc->stats.rx_fifo_errors++;
                    smc->stats.rx_other_errors++;
                    smc->err_rcvephoverrun++;
                }
                if (wtemp & TXUNRN_BF02)
                {
                    smc->stats.tx_fifo_errors++;
                    smc->stats.tx_other_errors++;
                    smc->err_txunderrun++;
                }

                /* EPH status is in wtemp. Use later for link up/down processing     */
                SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);      /* Select bank 2 */
            }

            /* Receive Overrun Interrupt   */
            if (int_status & ISR_OVR)
            {
#if 1
                /* [i_a] I've added this diagnostic updating code here too as I don't trust
                 * the EPH int flag... Anyway, this code will only update the counters
                 * if the EPH int flag isn't already set (as we'd update these counters
                 * in the code section above) and won't do any harm otherwise as the
                 * 'eph_2_bk0' register will be 0(zero) if there's nothing wrong...
                 */
                if (!(int_status & ISR_EPH))
                {
                  SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
                  wtemp = SMC91C9XREADWORD(EPH_2_BK0);    /* EPH Status */
                  if (wtemp & SNGL_B102)
                      smc->stats.one_collision++;
                  if (wtemp & MULT_B202)
                      smc->stats.multiple_collisions++;
                  if (wtemp & COL16_B402)
                      smc->stats.collision_errors++;
                  if (wtemp & SQET_B502)
                      smc->stats.tx_sqe_errors++;
                  if (wtemp & LATCOL_B902)
                      smc->stats.owc_collision++;
                  if (wtemp & LOSTC_BA02)
                      smc->stats.tx_carrier_errors++;
                  if (wtemp & EXCDEF_BB02)
                      smc->stats.tx_delayed++;
                  if (wtemp & RXOVRN_BD02)
                  {
                      smc->stats.rx_fifo_errors++;
                      smc->stats.rx_other_errors++;
                      smc->err_rcvephoverrun++;
                  }
                  if (wtemp & TXUNRN_BF02)
                  {
                      smc->stats.tx_fifo_errors++;
                      smc->stats.tx_other_errors++;
                      smc->err_txunderrun++;
                  }

                  /* EPH status is in wtemp. Use later for link up/down processing     */
                  SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);      /* Select bank 2 */
                }
#endif

                SMC91C9XWRITEBYTE(IAR_C_BK2, IAR_OVR);  /* acknowledge RX Overrun */
                smc->stats.packets_lost++;
                smc->err_rcvoverrun++;
            }

            /* RECEIVE INTERRUPT     */
            else if (int_status & ISR_RCVINT)
            {
                SMC91C9XWRITEWORD(PTR_6_BK2, READ_RCV(0));  /* read from b 0 */

                /* Get status and toss bad packets     */
                status = SMC91C9XREADWORD(DATA_8_BK2);
                if (status & (RCVTOOLNG | RCVTOOSHORT | RCVBADDCRV | RCVALGNERR))
                {
                    if (status & RCVTOOLNG)
                    {
                        smc->stats.rx_frame_errors++;
                        smc->err_rcvtoolong++;
                    }
                    if (status & RCVTOOSHORT)
                    {
                        smc->stats.rx_other_errors++;
                        smc->err_rcvtooshort++;
                    }
                    if (status & RCVBADDCRV)
                    {
                        smc->stats.rx_crc_errors++;
                        smc->err_rcvbadcrc++;
                    }
                    if (status & RCVALGNERR)
                    {
                        smc->stats.rx_other_errors++;
                        smc->err_rcvalignment++;
                    }
                    goto rcv_error;
                }

                /* Get read length. -6 removes len, status and control     */
                len = SMC91C9XREADWORD(DATA_8_BK2);
                if (status & RCVODD)    /* if odd we want the control word*/
                    len -= (word)4;
                else                        /* Don't even xfer control word   */
                    len -= (word)6;

                if (len > SMC_ETHER_MAX_LEN)
                {
rcv_error:
                    DEBUG_LOG("smcx driver - receive error ", LEVEL_1, EBS_INT1, status, 0);
                    smc->stats.errors_in++;
                    goto rm_and_release;
                }

#if (FILTER_SMC_INPUT)
                eth_filter_data.length = len;
                /* determine length that can be copied to local storage   */
                if (len > sizeof(eth_filter_data.data))
                {
                  len = sizeof(eth_filter_data.data);
                }
                len &= ~0x0001; /* make sure length is EVEN */
                eth_filter_data.stored_length = len;

                /* copy header data to local store; prevent early malloc() !   */
                SMC91C9XREPIN(&eth_filter_data.data, (len>>1));  /* Burst transfer */

                smc->diag_packets_received++;
                if (smc91c9x_filter(smc->pi, &eth_filter_data) == -1)
                {
                  smc->diag_packets_dropped++;
                  goto rm_and_release;
                }
               /* data remaining in cache:   */
               len = eth_filter_data.length - len;

               /* Allocate a packet to write the ethernet packet in.     */
               msg = os_alloc_packet_input(eth_filter_data.length, DRIVER_ALLOC);
               if (!msg)
               {
                     DEBUG_ERROR("smcx driver - out of packets", NOVAR, 0, 0);
                     smc->stats.packets_lost++;
                     goto rm_and_release;
               }
               msgdata = DCUTODATA(msg);
               /* copy local (partial) ethernet frame to permanent storage;
                  * add remaining data bytes from chip cache */
               tc_movebytes(msgdata, &eth_filter_data.data, eth_filter_data.stored_length);
               if (len > 0)
               {
                  SMC91C9XREPIN(msgdata+eth_filter_data.stored_length, (len>>1));  /* Burst transfer */
               }

               /* reset 'len' to total ethernet frame size   */
               len = eth_filter_data.length;
#else /* (FILTER_SMC_INPUT) */
                    /* Allocate a packet to write the ethernet packet in.   */
                    msg = os_alloc_packet_input(len, DRIVER_ALLOC);
                    if (!msg)
                    {
                        DEBUG_ERROR("smcx driver - out of packets", NOVAR, 0, 0);
                        smc->stats.packets_lost++;
                        goto rm_and_release;
                    }
                    msgdata = DCUTODATA(msg);
                    SMC91C9XREPIN(msgdata, (len>>1));  /* Burst transfer */
#endif

               if (status & RCVODD)
                     len -= (word)1;         /* If odd 1st byte control wrd is data */
                                             /* If even we didn't read cntrl     */
               DCUTOPACKET(msg)->length = len;

               smc->stats.packets_in++;
               smc->stats.bytes_in += (word)(len -sizeof(struct _ether));

#if (RTIP_VERSION > 24)
               ks_invoke_input(smc->pi, msg);
#else
               os_sndx_input_list(smc->pi, msg);
               ks_invoke_input(smc->pi);
#endif

rm_and_release:
/* wtemp = */   SMC91C9XREADWORD(FIFO_4_BK2);
                while (SMC91C9XREADWORD(MMU_0_BK2) & SMC_BIT0)
                        ;
                /* [i_a] disable the MMU_RMRLRX+waitloop code if you want the
                 * SMC chip filling up to simulate overloaded networks.
                 */
                SMC91C9XWRITEWORD(MMU_0_BK2,MMU_RMRLRX);/* Remove and release */
                while (SMC91C9XREADWORD(MMU_0_BK2) & SMC_BIT0)
                        ;
/* wtemp = */   SMC91C9XREADWORD(FIFO_4_BK2);
            }

            /* TRANSMIT INTERRUPT     */
            else if (int_status & ISR_TXINT)
            {

                pkt_save = SMC91C9XREADBYTE(PNR_2_BK2);/* Save Packet number register */

                /* [i_a] removed the loop 0..10 as it is not 100% identical to SMC advice.
                 * Considering all, I've removed the loop altogether as the advised
                 * polling of 'int_status' occurs at the end of the big loop.
                 */
                pkt_no = SMC91C9XREADBYTE(FIFO_4_BK2); /* read pkt from Fifo reg */
                if (!(pkt_no & SMC_BIT7))
                {
                    /* [i_a] process packet in FIFO as PNR is valid   */
                    SMC91C9XWRITEWORD(PNR_2_BK2, pkt_no);  /* to pkt number */

                    /* Get status     */
                    SMC91C9XWRITEWORD(PTR_6_BK2, READ_XMIT(0)); /*  read from b 0 */
                    status = SMC91C9XREADWORD(DATA_8_BK2);
                    if (!(status & TX_SUC))
                    {
                        smc->stats.errors_out++;

                        /* Set the txena bit after an error     */
                        SMC91C9XWRITEWORD(BSR_E_BKALL, BANK0);  /* Select bank 0 */
                        wtemp = SMC91C9XREADWORD(TCR_0_BK0);    /* Transmit control register */
                        wtemp |= TXEN_B000;
                        SMC91C9XWRITEWORD(TCR_0_BK0, wtemp);    /* Transmit control register */
                        SMC91C9XWRITEWORD(BSR_E_BKALL, BANK2);  /* Select bank 2 */
                    }
                    else /* Update total number of successfully transmitted packets. */
                        smc->stats.packets_out++;

                    /* signal output has completed   */
                    ks_invoke_output(smc->pi, 1);

                    SMC91C9XWRITEWORD(MMU_0_BK2,MMU_RMSP); /* Remove the xmit pcket */
                    while (SMC91C9XREADWORD(MMU_0_BK2) & SMC_BIT0)
                            ;

                    /* Acknowledge the interrupt     */
                    SMC91C9XWRITEBYTE(IAR_C_BK2, IAR_TXINT);
                }
                SMC91C9XWRITEBYTE(PNR_2_BK2, pkt_save); /* restore Packet number register */
            }

            /* XMIT EMPTY     */
            else if (int_status & ISR_TXEMT)
            {
                /* Shouldn't happen. If does clear it and mask it off     */

                SMC91C9XWRITEBYTE(IAR_C_BK2, IAR_TXEMT);
                imr &= ~IMR_TXEMT;
            }

            /* ALLOCATE MEMORY FOR TX COMPLETE     */
            else if (int_status & ISR_ALLOC)
            {
                /* Alloc succeeded. mask off alloc int and wake sender     */

                imr &= ~IMR_ALLOC;
                /* signal output has completed   */
                ks_invoke_output(smc->pi, 1);
            }
            int_status = (byte)(SMC91C9XREADBYTE(ISR_C_BK2) & imr);
        }
    }

    /* Restore pointer imr and bank registers     */
    SMC91C9XWRITEWORD(PTR_6_BK2, save_ptr);
    SMC91C9XWRITEBYTE(IMR_D_BK2, imr);
    SMC91C9XWRITEWORD(BSR_E_BKALL, save_bank);

    DRIVER_MASK_ISR_ON(smc->ia_irq)
}

int smc91c9x_getLinkStatus(void)						//Added by JLA (IO Technologies)
{
  int retval = 1;

#if(PHY_SUPPORT)
  P91_PARMS smc = (P91_PARMS)&(_91parms[0]);
  unsigned char phyaddr = 0x00;
  unsigned int phydata;

  SMC91C9XWRITEWORD(BSR_E_BKALL, BANK3);  /* Select bank 3 */
  phydata = PHYAccess(phyaddr,PHY_SR,OPRead,0,smc);
  if (!(phydata & (1<<2)))
  {
	retval = 0;
  }
#endif

  return retval;
}

/* ********************************************************************   */
/* MULTICAST                                                              */
/* ********************************************************************   */
void smc91c9x_getmcaf(PFBYTE mclist, int bytesmclist, PFBYTE af)
{
byte c;
PFBYTE cp;
dword crc;
int i, len, offset;
byte bit;
int row, col;

    /*
     * Set up multicast address filter by passing all multicast addresses
     * through a crc generator, and then using the high order 6 bits as an
     * index into the 64 bit logical address filter.  The high order bit
     * selects the word, while the rest of the bits select the bit within
     * the word.
     */

/*  if (ifp->if_flags & IFF_PROMISC) {   */
/*      ifp->if_flags |= IFF_ALLMULTI;   */
/*      af[0] = af[1] = 0xffffffff;      */
/*      return;                          */
/*  }                                    */

    af[0] = af[1] = af[2] = af[3] = af[4] = af[5] = af[6] = af[7] = 0;
    for (offset = 0; offset < bytesmclist; offset += 6)
    {
        cp = mclist + offset;
        crc = 0xffffffffL;
        for (len = 6; --len >= 0;) {
            c = *cp++;
            for (i = 8; --i >= 0;) {
                if (((crc & 0x80000000L) ? 1 : 0) ^ (c & 0x01)) {
                    crc <<= 1;
                    crc ^= 0x04c11db6L | 1;
                } else
                    crc <<= 1;
                c >>= 1;
            }
        }
        /* Just want the 6 most significant bits.   */
        crc >>= 26;

        /* Turn on the corresponding bit in the filter.   */
/*      af[crc >> 5] |= 1 << ((crc & 0x1f) ^ 24);         */
        row = (int)(crc/8);
        col = (int)(crc % 8);
        bit =  (byte) (1 << col);
        af[row] |= bit;
    }
}

#if (ZERO)
#define dprintf tm_printf

/*jhh [  */
void  dumpAllRegisters(P91_PARMS smc)
{
    DEBUG_ERROR("Fix printf use_ascilib ", NOVAR, 0, 0);
}
/*jhh ]  */
#endif


#if(PHY_SUPPORT)

/* GLOBAL VARIABLES                  */
/*unsigned long OUI;                 */
/*unsigned char     Model, Revision; */

void clkmdio(unsigned MGMTData,P91_PARMS smc)
{
  SMC91C9XWRITEWORD(MGMT, MGMTData);
  SMC91C9XWRITEWORD(MGMT, MGMTData | MCLK);
}

/* ---------- Routine that actually generates the MII frames ----------   */

unsigned PHYAccess(unsigned char PHYAdd, unsigned char RegAdd,
                   unsigned char OPCode, unsigned xwData, P91_PARMS smc)
{
 /* Local variables   */

 int i;
 unsigned MGMTval;

 /* Filter unused bits from input variables.   */

 PHYAdd &= 0x1F;
 RegAdd &= 0x1F;
 OPCode &= 0x03;

  MGMTval = SMC91C9XREADWORD(MGMT) & (MALL ^ 0xFFFF);

  /* Output Preamble (32 '1's)   */

  for (i=0;i<32;i++)
   clkmdio(MGMTval | MDOE | MDO, smc);

  /* Output Start of Frame ('01')   */

  for (i=0;i<2;i++)
   clkmdio(MGMTval | MDOE | i, smc);

  /* Output OPCode ('01' for write or '10' for Read)   */

  for (i=1;i>=0;i--)
   clkmdio(MGMTval | MDOE | ((OPCode>>i) & 0x01), smc );

  /* Output PHY Address   */

  for (i=4;i>=0;i--)
   clkmdio(MGMTval | MDOE | ((PHYAdd>>i) & 0x01) ,smc);

  /* Output Register Address   */

  for (i=4;i>=0;i--)
   clkmdio(MGMTval | MDOE | ((RegAdd>>i) & 0x01) ,smc);

  if (OPCode == OPRead)
  {
   /* Read Operation   */

   /* Implement Turnaround ('Z0')   */

   clkmdio(MGMTval,smc);

   /* Read Data   */

   xwData = 0;

   for (i=15;i>=0;i--)
   {
    clkmdio(MGMTval, smc);
    xwData |= (((SMC91C9XREADWORD(MGMT) & MDI) >> 1) << i);
   }

   /* Add Idle state   */

   clkmdio(MGMTval, smc);

   return (xwData);
  }
  else
  {
   /* Write Operation   */

   /* Implement Turnaround ('10')   */

   for (i=1;i>=0;i--)
    clkmdio(MGMTval | MDOE | ((2>>i) & 0x01), smc);

   /* Write Data   */

   for (i=15;i>=0;i--)
    clkmdio(MGMTval | MDOE | ((xwData>>i) & 0x01), smc);

   /* Add Idle state   */

   clkmdio(MGMTval, smc);

   return (1);
  }
}    /* End of Routine */

unsigned char DetectPHY(P91_PARMS smc)
{
    unsigned int PhyId1, PhyId2;
    unsigned char PhyAdd=0xff;
    int Count;

    for (Count=31;Count>=0;Count--)
    {
    PhyId1 = PHYAccess((unsigned char)Count, PHY_ID1, OPRead, 0, smc);
    PhyId2 = PHYAccess((unsigned char)Count, PHY_ID2, OPRead, 0, smc);

    if(
        (PhyId1 > 0x0000) &&
        (PhyId1 < 0xffff) &&
        (PhyId2 > 0x0000) &&
        (PhyId2 < 0xffff) &&
        ((PhyId1 != 0x8000) && (PhyId1 != 0x8000))
      )
      {
       PhyAdd = (unsigned char) Count;
       break;
      }
    ks_sleep((word)(ks_ticks_p_sec()/1000+1));
    }
/*    OUI =     (((unsigned long) PhyId1) << 6) | ((PhyId2 & 0xfc00) >> 10);   */
/*    Model =   (unsigned char) ( (PhyId2 & 0x03f0) >> 4 );                    */
/*    Revision =    (unsigned char) (PhyId2 & 0x000f);                         */
    return(PhyAdd);
}

#endif

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_smc91c9x(int minor_number)
{
    return(xn_device_table_add(smc91c9x_device.device_id,
                        minor_number,
                        smc91c9x_device.iface_type,
                        smc91c9x_device.device_name,
                        SNMP_DEVICE_INFO(smc91c9x_device.media_mib,
                                         smc91c9x_device.speed)
                        (DEV_OPEN)smc91c9x_device.open,
                        (DEV_CLOSE)smc91c9x_device.close,
                        (DEV_XMIT)smc91c9x_device.xmit,
                        (DEV_XMIT_DONE)smc91c9x_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)smc91c9x_device.proc_interrupts,
                        (DEV_STATS)smc91c9x_device.statistics,
                        (DEV_SETMCAST)smc91c9x_device.setmcast));
}

#if (INCLUDE_PCMCIA)
int xn_bind_smc91c9x_pcmcia(int minor_number)
{
    return(xn_device_table_add(SMC91C9X_PCMCIA_DEVICE,
                        minor_number,
                        smc91c9x_device.iface_type,
                        "SMC91C9X PCMCIA",
                        SNMP_DEVICE_INFO(smc91c9x_device.media_mib,
                                         smc91c9x_device.speed)
                        (DEV_OPEN)smc91c9x_device.open,
                        (DEV_CLOSE)smc91c9x_device.close,
                        (DEV_XMIT)smc91c9x_device.xmit,
                        (DEV_XMIT_DONE)smc91c9x_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)smc91c9x_device.proc_interrupts,
                        (DEV_STATS)smc91c9x_device.statistics,
                        (DEV_SETMCAST)smc91c9x_device.setmcast));
}
#endif

#endif      /* DECLARING_DATA */

#endif      /* INCLUDE_SMC91C9X */
