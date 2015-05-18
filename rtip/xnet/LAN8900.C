/***********************************************************************   */
/*                                                                         */
/*   MODULE:     lan8900.c                                                 */
/*   DATE:       5/7/96                                                    */
/*   PURPOSE:    Crystal Semiconductor CS8900 driver for pSOS/x86          */
/*   PROGRAMMER: Quentin Stephenson                                        */
/*                                                                         */
/*----------------------------------------------------------------------   */
/*                                                                         */
/*               Copyright 1996, Crystal Semiconductor Corp.               */
/*                      ALL RIGHTS RESERVED                                */
/*                                                                         */
/*                                                                         */
/***********************************************************************   */
/*                                                                         */
/*    Change Log:                                                          */
/*   9/15/99  James Ayres added support for the RDY4TXNow                  */
/*            interrupt.  All changes marked with @jla                     */
/*   10/06/00 Melody Lee. Modified for CAD-UL compiler.                    */
/*            All changes marked with @kml                                 */
/***********************************************************************   */

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_LAN89X0)         /* Cirrus Crystal 8900/8920 */

#include "lan8900.h"

#define DEBUG_LAN8900  0
#define DISPLAY_OUTPUT 0

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    */
/* Note on naming conventions used in this driver.                        */
/*                                                                        */
/* All routines and global variables in this driver start with a lower    */
/* case "cs", which stands for Crystal Semiconductor.  This identifies    */
/* these globally existing objects as belonging to this driver.           */
/*                                                                        */
/* All variables which contain an address instead of a value, start       */
/* with a lower case "p", which stands for pointer.  If a global          */
/* variable contains an address then it starts with "cs_p".               */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    */

RTIP_BOOLEAN cs_open(PIFACE pi);
void         cs_close(PIFACE pi);
int          cs_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN cs_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN cs_statistics(PIFACE  pi);
RTIP_BOOLEAN cs_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
/* Global variables                                                       */
CS_PARMS csparms[CFG_NUM_CS];

EDEVTABLE KS_FAR cs_device =
{
     cs_open, cs_close, cs_xmit, cs_xmit_done,
     NULLP_FUNC, cs_statistics, cs_setmcast,
     /*OS*/ /* was      LAN_CS89X0_DEVICE, "LAN_CS89X0", MINOR_0, ETHER_IFACE, */
     CS89X0_DEVICE, "CS89X0", MINOR_0, ETHER_IFACE,
     SNMP_DEVICE_INFO(CFG_OID_CS89X0, CFG_SPEED_CS89X0)
     CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
     CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
     IOADD(0x300), EN(0x0), EN(0xa)
};

#endif  /* DECLARING_DATA */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern CS_PARMS csparms[CFG_NUM_CS];
extern EDEVTABLE KS_FAR cs_device;
#endif

/* ********************************************************************   */
/* Prototypes of internal routines                                        */
/* ********************************************************************   */

void          csDelay( unsigned long usec );
void          lan_pre_isr(int minor_no);
void          lan_isr(int minor_number);
RTIP_BOOLEAN  csVerifyChip(PCS_PARMS cs);
RTIP_BOOLEAN  csResetChip(PCS_PARMS cs);
RTIP_BOOLEAN  csInitFromEEPROM(PIFACE pi);
int           csReadEEPROM(PCS_PARMS cs, word Offset, word *pValue );
/*OS*/ /* RTIP_BOOLEAN  csInitFromBSPDefs(void); PCS_PARMS cs */
/*OS*/ RTIP_BOOLEAN  csInitFromBSPDefs(PCS_PARMS cs);
char *        csHexWord( char *pChar, word *pWord );
char *        csHexByte( char *pChar, unsigned char *pByte );
RTIP_BOOLEAN  csInitInterrupt(PIFACE pi);
word          csRequestTransmit(PCS_PARMS cs, DCU msg, int length);
void          csCopyTxFrame(PCS_PARMS cs, DCU msg, int length);
void          csProcessISQ(PCS_PARMS cs);
void          csReceiveEvent(PCS_PARMS cs, word RxEvent );
void          csBufferEvent(PIFACE pi, word BufEvent);
void          csTransmitEvent(PIFACE pi);
word          csReadPacketPage(PCS_PARMS cs, word Offset);
void          csWritePacketPage(PCS_PARMS cs, word Offset, word Value);

/* ********************************************************************   */
/* MEMORY MANAGMENT                                                       */
/* ********************************************************************   */

PCS_PARMS iface_to_cs(PIFACE pi)
{
int cs_off;

    cs_off = pi->minor_number;
    if (cs_off >= CFG_NUM_CS)
    {
        DEBUG_ERROR("iface_to_cs() - pi->minor_number, CFG_NUM_SMCX =",
            EBS_INT2, pi->minor_number, CFG_NUM_SMCX);
        return((PCS_PARMS)0);
    }

    return((PCS_PARMS) &csparms[cs_off]);
}

void csDelay( unsigned long usec )
{
unsigned long i;
    for (i=0; i<usec; i++)
        io_delay();
}

/* ********************************************************************   */
/* OPEN routines.                                                         */
/* ********************************************************************   */

/***********************************************************************   */
/*  cs_open()                                                              */
/*                                                                         */
/*  This routine initializes the network interface driver.                 */
/*  If the driver is successfully initialized, then TRUE                   */
/*  driver's hardware address is returned, else FALSE is returned.         */
/*                                                                         */
/*  Returns TRUE if successful, FALSE if not                               */
/***********************************************************************   */

RTIP_BOOLEAN cs_open(PIFACE pi)
{
PCS_PARMS cs;
int       i;

#if (DEBUG_LAN8900)
    DEBUG_ERROR("cs_open: called", NOVAR, 0, 0);
#endif

    cs = iface_to_cs(pi);
    if (!cs)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    /* Initialize the device structure.   */
    tc_memset(cs, 0, sizeof(CS_PARMS));

    cs->pi = pi;

    cs->csInMemoryMode = FALSE;  /* Must start out using IO mode */

    cs->io_address = pi->io_address;
    cs->mem_address = pi->mem_address;

     /* Verify that it is the correct chip   */
     if (!csVerifyChip(cs))
        return FALSE;

#if (DEBUG_LAN8900)
    DEBUG_ERROR("csVerifyChip done", NOVAR, 0, 0);
#endif

    /* Reset the chip   */
    if ( !csResetChip(cs) )
    {
        set_errno(EPROBEFAIL);
        return FALSE;
    }

#if (DEBUG_LAN8900)
    DEBUG_ERROR("csReset done", NOVAR, 0, 0);
#endif

#    if (BSP_CS8900_EEPROM == 1)

    /* Initialize using data in the EEPROM   */
    if ( !csInitFromEEPROM(pi) )
    {
        set_errno(EIFACEOPENFAIL);
        return FALSE;
    }
#if (DEBUG_LAN8900)
    DEBUG_ERROR("csInitFromEEPROM done", NOVAR, 0, 0);
#endif


#    else  /* BSP_CS8900_EEPROM == 0 */

       /* Initialize using defines in the BSP.H file   */
/*OS*/ /*       if ( !csInitFromBSPDefs() )  */ 
/*OS*/        if ( !csInitFromBSPDefs(cs) )  
       {
            set_errno(EIFACEOPENFAIL);
            return FALSE;
        }
#if (DEBUG_LAN8900)
        DEBUG_ERROR("csInitFromBSPDefs done", NOVAR, 0, 0);
#endif

#    endif  /* BSP_CS8900_EEPROM */

    /* Initialize the config and control registers   */
    csWritePacketPage(cs, PKTPG_RX_CFG, RX_CFG_RX_OK_IE );
    csWritePacketPage(cs, PKTPG_RX_CTL, RX_CTL_RX_OK_A|RX_CTL_IND_A|RX_CTL_BCAST_A);
    csWritePacketPage(cs, PKTPG_TX_CFG, TX_CFG_ALL_IE );
    /* @jla added to support rdy4tx interrupt   */
    csWritePacketPage(cs, PKTPG_BUF_CFG, BUF_CFG_RDY4TX_IE );

    /* copy the hardware (i.e. ethernet) address setup by csInitFromxxx   */
    /* to the interface structure                                         */
    for (i = 0; i < ETH_ALEN/2; i++)
    {
        pi->addr.my_hw_addr[i*2] = (byte)cs->csHardwareAddr[i];
        pi->addr.my_hw_addr[i*2+1] = (byte)(cs->csHardwareAddr[i] >> 8);
    }

#if (DEBUG_LAN8900)
    DEBUG_ERROR("ETHERNET ADDRESS: ", ETHERADDR, pi->addr.my_hw_addr, 0);
#endif

    /* Put hardware address into the Individual Address register   */
    csWritePacketPage(cs, PKTPG_IND_ADDR,   cs->csHardwareAddr[0] );
    csWritePacketPage(cs, PKTPG_IND_ADDR+2, cs->csHardwareAddr[1] );
    csWritePacketPage(cs, PKTPG_IND_ADDR+4, cs->csHardwareAddr[2] );

    /* Initialize the interrupt   */
    if ( csInitInterrupt(pi) == FALSE )
    {
        DEBUG_ERROR("csInitInterrupt failed", NOVAR, 0, 0);
        set_errno(EIFACEOPENFAIL);
        return FALSE;
    }

#if (DEBUG_LAN8900)
    DEBUG_ERROR("csInitInterrupt done", NOVAR, 0, 0);
#endif
    return TRUE;  /* Successfull initialization! */
}


/****************************************************************************   */
/*  csVerifyChip()                                                              */
/*                                                                              */
/*  This routine verifies that the Crystal Semiconductor chip is present        */
/*  and that it is a CS8900.                                                    */
/*                                                                              */
/*  IMPORTANT!  This routine will fail if the IO base address programmed        */
/*              in the chip's EEPROM does not match the IO base address         */
/*              specified with BSP_CS8900_IO_BASE in the BSP.H file.            */
/*                                                                              */
/****************************************************************************   */

RTIP_BOOLEAN csVerifyChip(PCS_PARMS cs)
{
word revison_type;

    /* See if the PacketPage Pointer port contains the correct signature   */
    if ( (INWORD(PORT_PKTPG_PTR)&SIGNATURE_PKTPG_PTR) != SIGNATURE_PKTPG_PTR )
       return FALSE;

    /* Verify that the chip is a Crystal Semiconductor chip   */
    if ( csReadPacketPage(cs, PKTPG_EISA_NUM) != EISA_NUM_CRYSTAL )
       return FALSE;

    revison_type = csReadPacketPage(cs, PKTPG_PRODUCT_ID);
    cs->chip_type = revison_type &~ PROD_REV_MASK;

#if (DEBUG_LAN8900)
    DEBUG_ERROR("chip_type = ", EBS_INT1, cs->chip_type, 0);
#endif

    return TRUE;
}


/****************************************************************************   */
/*  csResetChip()                                                               */
/*                                                                              */
/*  This routine resets the chip and initializes it for 16-bit operation.       */
/*                                                                              */
/****************************************************************************   */

RTIP_BOOLEAN csResetChip(PCS_PARMS cs)
{
long x;

    /* Issue a reset command to the chip   */
    csWritePacketPage(cs, PKTPG_SELF_CTL, SELF_CTL_RESET );

    /* Delay for 125 micro-seconds   */
    csDelay( 125 );

    /* Transition SBHE to switch chip from 8-bit to 16-bit   */
    INBYTE( PORT_PKTPG_PTR   );
    INBYTE( PORT_PKTPG_PTR+1 );
    INBYTE( PORT_PKTPG_PTR   );
    INBYTE( PORT_PKTPG_PTR+1 );

    /* Wait until the EEPROM is not busy   */
    for ( x=0; x<MAXLOOP; x++ )
       if ( !(csReadPacketPage(cs, PKTPG_SELF_ST)&SELF_ST_SI_BUSY) )
          break;
    if ( x == MAXLOOP ) return FALSE;

#if (SUPPORTS_CS8920)
{
    if (cs->chip_type != PROD_ID_CS8900)
    {
        /* Hardware problem requires PNP registers to be reconfigured after a reset   */
        LAN_OUTWORD(PORT_PKTPG_PTR, PKTPG_INTNUM_20);
        LAN_OUTBYTE(PORT_PKTPG_DATA, cs->csIntNumber);
        LAN_OUTBYTE(PORT_PKTPG_DATA_1, 0);

        LAN_OUTWORD(PORT_PKTPG_PTR, PKTPG_MEME_BASE_20);
        LAN_OUTBYTE(PORT_PKTPG_DATA, (cs->mem_address >> 8) & 0xff);
        LAN_OUTBYTE(PORT_PKTPG_DATA_1, (cs->mem_address >> 24) & 0xff);
    }
}
#endif

    /* Wait until initialization is done   */
    for ( x=0; x<MAXLOOP; x++ )
       if ( csReadPacketPage(cs, PKTPG_SELF_ST)&SELF_ST_INIT_DONE )
          break;
    if ( x == MAXLOOP ) return FALSE;

    return TRUE;
}


#if (BSP_CS8900_EEPROM == 1)

/****************************************************************************   */
/*  csInitFromEEPROM()                                                          */
/*                                                                              */
/*  This routine initializes the chip using configuration information           */
/*  obtained from the EEPROM attached to the chip.  This routine also reads     */
/*  the interrupt level and hardware address from the EEPROM and saves them     */
/*  in the csHardwareAddr and csIntNumber global variables.                     */
/*                                                                              */
/****************************************************************************   */

RTIP_BOOLEAN csInitFromEEPROM(PIFACE pi)
{
    word SelfStatus;
    word ISAConfig;
    word MemBase;
    word BusCtl;
    word AdapterConfig;
    word SelfCtl;
    word XmitCtl;
    PCS_PARMS cs;
#if (SUPPORTS_CS8920)
    word irq;
#endif

    cs = iface_to_cs(pi);
    if (!cs)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    /* Verify that the EEPROM is present and OK   */
    SelfStatus = csReadPacketPage(cs,  PKTPG_SELF_ST );
    if ( !((SelfStatus & SELF_ST_EEP_PRES) && (SelfStatus & SELF_ST_EEP_OK)) )
       return FALSE;

    /* Get ISA configuration from the EEPROM   */
    if ( csReadEEPROM(cs, EEPROM_ISA_CFG,&ISAConfig) == FALSE )
       return FALSE;

    /* If memory mode is enabled   */
    if ( ISAConfig & ISA_CFG_MEM_MODE )
    {
       /* Get memory base address from EEPROM   */
       if ( csReadEEPROM(cs, EEPROM_MEM_BASE,&MemBase) == FALSE )
          return FALSE;

       MemBase &= MEM_BASE_MASK;  /* Clear unused bits */
        /* If external logic is present for address decoding   */
       if ( csReadPacketPage(cs, PKTPG_SELF_ST) & SELF_ST_EL_PRES )
       {
          /* Program the external logic to decode address bits SA20-SA23   */
          csWritePacketPage(cs, PKTPG_EEPROM_CMD, (word)
                             ((MemBase>>12) | EEPROM_CMD_ELSEL));
       }

       /* Setup chip for memory mode   */
       csWritePacketPage(cs, PKTPG_MEM_BASE,
                          (word)((MemBase<<8) & 0xFFFF) );
       csWritePacketPage(cs, PKTPG_MEM_BASE+2, (word)(MemBase>>8) );
       BusCtl = BUS_CTL_MEM_MODE;
       if ( ISAConfig & ISA_CFG_USE_SA )
          BusCtl |= BUS_CTL_USE_SA;
       csWritePacketPage(cs, PKTPG_BUS_CTL, BusCtl );

       /* Setup global variables for memory mode   */
       cs->cs_pPacketPage = (word *)(((unsigned long)MemBase)<<8);
       cs->csInMemoryMode = TRUE;  /* We are in memory mode now! */
    }

    /* If IOCHRDY is enabled then clear the bit in the BusCtl register   */
    BusCtl = csReadPacketPage(cs,  PKTPG_BUS_CTL );
    if ( ISAConfig & ISA_CFG_IOCHRDY )
       csWritePacketPage(cs, PKTPG_BUS_CTL,
                          (word)(BusCtl & ~BUS_CTL_IOCHRDY) );
    else
       csWritePacketPage(cs, PKTPG_BUS_CTL,
                          (word)(BusCtl | BUS_CTL_IOCHRDY) );

    /* Save the interrupt number to be initialized later   */
    cs->csIntNumber = ISAConfig & ISA_CFG_IRQ_MASK;

#if (DEBUG_LAN8900)
    DEBUG_ERROR("set irq number(1): ", EBS_INT1, cs->csIntNumber, 0);
#endif

#if (SUPPORTS_CS8920)
    /* If this is a CS8900 then no pnp soft   */
    if (cs->chip_type != PROD_ID_CS8900)
    {
        /* Check if the ISA IRQ has been set    */
        irq = (word)(csReadPacketPage(cs, PKTPG_INTNUM_20) & 0xff);
        if (irq != 0 && irq < CS8920_NO_INTS)
        {
            if (!cs->csIntNumber)
            {
                cs->csIntNumber = irq;
#if (DEBUG_LAN8900)
                DEBUG_ERROR("set irq number(2): ", EBS_INT1, cs->csIntNumber, 0);
#endif
            }
#if (DEBUG_LAN8900)
            DEBUG_ERROR("IRQ is ", EBS_INT1, cs->csIntNumber, 0);
#endif
        }
        else
        {
            cs->csIntNumber = pi->irq_val;
#if (DEBUG_LAN8900)
            DEBUG_ERROR("use pi->irq_val for irq", EBS_INT1, cs->csIntNumber, 0);
#endif
        }
    }
    else
#endif /* (SUPPORTS_CS8920) */
    {
        if ( cs->csIntNumber == 3 )
           cs->csIntNumber = 5;
        else
           cs->csIntNumber += 10;
#if (DEBUG_LAN8900)
        DEBUG_ERROR("set irq number(3): ", EBS_INT1, cs->csIntNumber, 0);
#endif
    }

    /* Get adapter configuration from the EEPROM   */
    if ( csReadEEPROM(cs, EEPROM_ADPTR_CFG,&AdapterConfig) == FALSE )
       return FALSE;

    /* Set the Line Control register to match the media type      */
/*  if ( (AdapterConfig & ADPTR_CFG_MEDIA) == ADPTR_CFG_10BASET ) */
    {
        DEBUG_ERROR("lan8900: set to 10 Base T", DINT2, AdapterConfig,
            ADPTR_CFG_MEDIA);
        DEBUG_ERROR("lan8900:    10BaseT", DINT1, ADPTR_CFG_10BASET, 0);
        csWritePacketPage(cs, PKTPG_LINE_CTL, LINE_CTL_10BASET );
    }
/*  else                                                           */
/*  {                                                              */
/*      DEBUG_ERROR("lan8900: set to AUI", NOVAR, 0, 0);           */
/*      csWritePacketPage(cs, PKTPG_LINE_CTL, LINE_CTL_AUI_ONLY ); */
/*  }                                                              */

    /* Set the BSTATUS/HC1 pin to be used as HC1    */
    /* HC1 is used to enable the DC/DC converter    */
    SelfCtl = SELF_CTL_HC1E;

    /* If the media type is 10Base2   */
    if ( (AdapterConfig & ADPTR_CFG_MEDIA) == ADPTR_CFG_10BASE2 )
    {
       /* Enable the DC/DC converter                 */
       /* If the DC/DC converter has a low enable    */
       if ( (AdapterConfig & ADPTR_CFG_DCDC_POL) == 0 )
          /* Set the HCB1 bit, which causes the HC1 pin to go low   */
          SelfCtl |= SELF_CTL_HCB1;
    }
    else  /* Media type is 10BaseT or AUI */
    {
       /* Disable the DC/DC converter                 */
       /* If the DC/DC converter has a high enable    */
       if ( (AdapterConfig & ADPTR_CFG_DCDC_POL) != 0 )
          /* Set the HCB1 bit, which causes the HC1 pin to go low   */
          SelfCtl |= SELF_CTL_HCB1;
    }
    csWritePacketPage(cs, PKTPG_SELF_CTL, SelfCtl );

    /* If media type is 10BaseT   */
    if ( (AdapterConfig & ADPTR_CFG_MEDIA) == ADPTR_CFG_10BASET )
    {
       /* Get transmission control from the EEPROM   */
       if ( csReadEEPROM(cs, EEPROM_XMIT_CTL,&XmitCtl) == FALSE )
          return FALSE;

       /* If full duplex mode then set the FDX bit in TestCtl register   */
       if ( XmitCtl & XMIT_CTL_FDX )
          csWritePacketPage(cs, PKTPG_TEST_CTL, TEST_CTL_FDX );
    }

   /* Get Individual Address from the EEPROM   */
   if ( csReadEEPROM(cs, EEPROM_IND_ADDR_H,&cs->csHardwareAddr[0]) == FALSE )
      return FALSE;
   if ( csReadEEPROM(cs, EEPROM_IND_ADDR_M,&cs->csHardwareAddr[1]) == FALSE )
      return FALSE;
   if ( csReadEEPROM(cs, EEPROM_IND_ADDR_L,&cs->csHardwareAddr[2]) == FALSE )
      return FALSE;

    return TRUE;
}


/****************************************************************************   */
/*  csReadEEPROM()                                                              */
/*                                                                              */
/*  Read the specified offset within the EEPROM.                                */
/*                                                                              */
/****************************************************************************   */

int csReadEEPROM(PCS_PARMS cs, word Offset, word *pValue )
{
long x;

    /* Ensure that the EEPROM is not busy   */
    for ( x=0; x<MAXLOOP; x++ )
       if ( !(csReadPacketPage(cs, PKTPG_SELF_ST)&SELF_ST_SI_BUSY) )
          break;
    if ( x == MAXLOOP ) return FALSE;

    /* Issue the command to read the offset within the EEPROM   */
    csWritePacketPage(cs, PKTPG_EEPROM_CMD,
                       (word)(Offset | EEPROM_CMD_READ) );

    /* Wait until the command is completed   */
    for ( x=0; x<MAXLOOP; x++ )
       if ( !(csReadPacketPage(cs, PKTPG_SELF_ST)&SELF_ST_SI_BUSY) )
          break;
    if ( x == MAXLOOP ) return FALSE;

    /* Get the EEPROM data from the EEPROM Data register   */
    *pValue = csReadPacketPage(cs,  PKTPG_EEPROM_DATA );

    return TRUE;
}


#else  /* BSP_CS8900_EEPROM == 0 */


/****************************************************************************   */
/*  csInitFromBSPDefs()                                                         */
/*                                                                              */
/*  This routine initializes the chip using definitions in the BSP.H file.      */
/*  This routine also converts the hardware address string specified by the     */
/*  BSP_CS8900_IND_ADDR definition and stores it in the csHardwareAddr          */
/*  global variable.  The defined interrupt level is stored in the              */
/*  cs->csIntNumber.                                                            */
/*                                                                              */
/****************************************************************************   */

/*OS*/ /* RTIP_BOOLEAN csInitFromBSPDefs(void) */
/*OS*/ RTIP_BOOLEAN csInitFromBSPDefs(PCS_PARMS cs)
{
    word BusCtl;
    word SelfCtl;
    char *pChar;

#    if (BSP_CS8900_MEM_MODE == 1)

       /* If external logic is present for address decoding   */
       if ( csReadPacketPage(cs, PKTPG_SELF_ST) & SELF_ST_EL_PRES )
       {
          /* Program the external logic to decode address bits SA20-SA23   */
          csWritePacketPage(cs, PKTPG_EEPROM_CMD,
                (word)((BSP_CS8900_MEM_BASE>>20) | EEPROM_CMD_ELSEL) );
       }

       /* Setup chip for memory mode   */
       csWritePacketPage(cs, PKTPG_MEM_BASE,   BSP_CS8900_MEM_BASE & 0xFFFF );
       csWritePacketPage(cs, PKTPG_MEM_BASE+2, BSP_CS8900_MEM_BASE>>16 );
       BusCtl = BUS_CTL_MEM_MODE;
#       if (BSP_CS8900_USE_SA == 1)
          BusCtl |= BUS_CTL_USE_SA;
#       endif
       csWritePacketPage(cs, PKTPG_BUS_CTL, BusCtl );

       /* Setup global variables for memory mode   */
       cs->cs_pPacketPage = (word *)BSP_CS8900_MEM_BASE;
       cs->csInMemoryMode = TRUE;  /* We are in memory mode now! */

#    endif  /* BSP_CS8900_MEM_MODE */

    /* If IOCHRDY is enabled then clear the bit in the BusCtl register   */
    BusCtl = csReadPacketPage(cs,  PKTPG_BUS_CTL );
#    if (BSP_CS8900_IOCHRDY == 1)
       csWritePacketPage(cs, PKTPG_BUS_CTL, BusCtl & ~BUS_CTL_IOCHRDY );
#    else
       csWritePacketPage(cs, PKTPG_BUS_CTL, BusCtl | BUS_CTL_IOCHRDY );
#    endif

    /* Save the interrupt number to be initialized later   */
    cs->csIntNumber = BSP_CS8900_IRQ;
#if (DEBUG_LAN8900)
    DEBUG_ERROR("set irq number(4): ", EBS_INT1, cs->csIntNumber, 0);
#endif

    /* Set the Line Control register to match the media type   */
#    if (BSP_CS8900_MEDIA_TYPE == TEN_BASE_T )
       csWritePacketPage(cs, PKTPG_LINE_CTL, LINE_CTL_10BASET );
#    else
       csWritePacketPage(cs, PKTPG_LINE_CTL, LINE_CTL_AUI_ONLY );
#    endif

    /* Set the BSTATUS/HC1 pin to be used as HC1    */
    /* HC1 is used to enable the DC/DC converter    */
    SelfCtl = SELF_CTL_HC1E;

    /* If the media type is 10Base2   */
#    if (BSP_CS8900_MEDIA_TYPE == TEN_BASE_2 )
       /* Enable the DC/DC converter                 */
       /* If the DC/DC converter has a low enable    */
#       if (BSP_CS8900_DCDC_POL == LOW)
          /* Set the HCB1 bit, which causes the HC1 pin to go low   */
          SelfCtl |= SELF_CTL_HCB1;
#      endif
#    else  /* Media type is 10BaseT or AUI */
       /* Disable the DC/DC converter                 */
       /* If the DC/DC converter has a high enable    */
#       if (BSP_CS8900_DCDC_POL == HIGH)
          /* Set the HCB1 bit, which causes the HC1 pin to go low   */
          SelfCtl |= SELF_CTL_HCB1;
#       endif
#    endif  /* BSP_CS8900_MEDIA_TYPE */
    csWritePacketPage(cs, PKTPG_SELF_CTL, SelfCtl );

    /* If media type is 10BaseT   */
#    if (BSP_CS8900_MEDIA_TYPE == TEN_BASE_T )
       /* If full duplex mode then set the FDX bit in TestCtl register   */
#       if (BSP_CS8900_FDX == 1 )
          csWritePacketPage(cs, PKTPG_TEST_CTL, TEST_CTL_FDX );
#       endif
#    endif

    /* Convert and save the Individual Address string   */
#if 0 /*OS*/
    pChar = BSP_CS8900_IND_ADDR;
    pChar = csHexWord( pChar, &cs->csHardwareAddr[0] );
    if ( pChar == CS_NULL ) return FALSE;
    pChar = csHexWord( pChar, &cs->csHardwareAddr[1] );
    if ( pChar == CS_NULL ) return FALSE;
    pChar = csHexWord( pChar, &cs->csHardwareAddr[2] );
    if ( pChar == CS_NULL ) return FALSE;
#else /*OS*/ /* next 3 lines instead */
    cs->csHardwareAddr[0] = CS8900A_ETH_ADDR_LO & 0xffff;
    cs->csHardwareAddr[1] = (CS8900A_ETH_ADDR_LO>>16) & 0xffff;
    cs->csHardwareAddr[2] = CS8900A_ETH_ADDR_HI & 0xffff;
#endif /*OS*/

    return TRUE;
}


/****************************************************************************      */
/*  csHexWord()                                                                    */
/*                                                                                 */
/*  This routine converts a sequence of hex characters to the 16-bit value         */
/*  that they represent.  The address of the first hex character is passed         */
/*  in the pChar parameter and the address just beyond the last hex                */
/*  character is returned.  The 16-bit variable pointed to by the pWord            */
/*  parameter is updated with the converted 16-bit value.  If an error             */
/*  occurred then CS_NULL is returned.                                             */
/*                                                                                 */
/****************************************************************************      */

char *csHexWord( char *pChar, word *pWord )
{
    union
    {
       word word;
       unsigned char  byte[2];
    } Value;

    /* Get the value of the first hex byte   */
    pChar = csHexByte( pChar, &Value.byte[0] );
    if ( pChar==CS_NULL || *pChar==0 ) return CS_NULL;

    /* Get the value of the second hex byte   */
    pChar = csHexByte( pChar, &Value.byte[1] );
    if ( pChar==CS_NULL ) return CS_NULL;

    /* Save value of the hex word   */
    *pWord = Value.word;

    return pChar;
}


/****************************************************************************      */
/*  csHexByte()                                                                    */
/*                                                                                 */
/*  This routine converts a sequence of hex characters to the 8-bit value          */
/*  that they represent.  There may be zero, one or two hex characters and         */
/*  they may be optionally terminated with a colon or a zero byte.                 */
/*  The address of the first hex character is passed in the pChar parameter        */
/*  and the address just beyond the last hex character is returned.                */
/*  The 8-bit variable pointed to by the pByte parameter is updated with           */
/*  the converted 8-bit value.  If an error occurred then CS_NULL is returned.     */
/*                                                                                 */
/****************************************************************************      */

char *csHexByte( char *pChar, unsigned char *pByte )
{
    int x;

    /* Inititalize the byte value to zero   */
    *pByte = 0;

    /* Process two hex characters   */
    for ( x=0; x<2; x++,pChar++ )
    {
       /* Stop early if find a colon or end of string   */
       if ( *pChar==':' || *pChar==0 ) break;

       /* Convert the hex character to a value   */
       if ( *pChar >= '0' && *pChar <= '9' )
          *pByte = (*pByte * 16) + *pChar - '0';
       else if ( *pChar >= 'a' && *pChar <= 'f' )
          *pByte = (*pByte * 16) + *pChar - 'a' + 10;
       else if ( *pChar >= 'A' && *pChar <= 'F' )
          *pByte = (*pByte * 16) + *pChar - 'A' + 10;
       else return CS_NULL;  /* Illegal character */
    }

    /* Skip past terminating colon   */
    if ( *pChar == ':' ) pChar++;

    return pChar;
}

#endif  /* BSP_CS8900_EEPROM */


/****************************************************************************   */
/*  csInitInterrupt()                                                           */
/*                                                                              */
/*  This routine initializes the chip, interrupt descriptor table and the       */
/*  programmable interrupt controller for the interrupt level contained in      */
/*  in the csIntNumber.                                                         */
/*                                                                              */
/*  If the interrupt level is not valid then FALSE is returned.                 */
/*                                                                              */
/****************************************************************************   */

RTIP_BOOLEAN csInitInterrupt(PIFACE pi)
{
PCS_PARMS cs;
word w;

    cs = iface_to_cs(pi);
    if (!cs)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    if (cs->chip_type == PROD_ID_CS8900)
    {
        /* Verify that the interrupt number is vaild   */
        if ( !(cs->csIntNumber==5 || cs->csIntNumber==10 ||
               cs->csIntNumber==11 || cs->csIntNumber==12) )
           return FALSE;
    }

    /* Set the interrupt number in the chip   */
#if (SUPPORTS_CS8920)
    if (cs->chip_type != PROD_ID_CS8900)
    {
       csWritePacketPage(cs, PKTPG_INTNUM_20, (word)(cs->csIntNumber) );
    }
    else
#endif
    {
        if ( cs->csIntNumber == 5 )
           csWritePacketPage(cs, PKTPG_INT_NUM, 3 );
        else
           csWritePacketPage(cs, PKTPG_INT_NUM, (word)(cs->csIntNumber-10) );
    }
#if (DEBUG_LAN8900)
    DEBUG_ERROR("set irq in chip: ", EBS_INT1, cs->csIntNumber, 0);
#endif

    /* Enable the interrupt at the PIC   */
    ks_hook_interrupt(cs->csIntNumber, (PFVOID) pi,
                      (RTIPINTFN_POINTER)lan_isr,
                      (RTIPINTFN_POINTER)lan_pre_isr,
                      pi->minor_number);

    /* ********************************************************************   */
    /* Enable interrupt at the chip   */
    csWritePacketPage(cs, PKTPG_BUS_CTL, (word)
        (csReadPacketPage(cs, PKTPG_BUS_CTL) | BUS_CTL_INT_ENBL) );

    /* Enable reception and transmission of frames   */
    w = csReadPacketPage(cs, PKTPG_LINE_CTL);
    w &= 0x7fff;
    /* Turn on both receive and transmit operations   */
    csWritePacketPage(cs, PKTPG_LINE_CTL, (word)(w | LINE_CTL_RX_ON  |
                                                 LINE_CTL_TX_ON) );

    /* ********************************************************************   */
    /* Receive only error free packets addressed to this card                 */
    csWritePacketPage(cs, PKTPG_RX_CTL, DEF_RX_ACCEPT);

    csWritePacketPage(cs, PKTPG_RX_CFG, RX_CFG_RX_OK_IE);

    csWritePacketPage(cs, PKTPG_TX_CFG,
                      TX_CFG_TX_OK_IE | TX_CFG_OUT_WIN_IE | TX_CFG_JABBER_IE |
                      TX_CFG_ANY_COL_ENBL | TX_CFG_16_COLL_IE |
                      TX_CFG_LOSS_CRS_IE | TX_CFG_SQE_ERR_IE);

    csWritePacketPage(cs, PKTPG_BUF_CFG,
                      TX_EVENT_TX_OK | TX_EVENT_OUT_WIN | TX_EVENT_16_COLL |
                      RX_MISS_COUNT_OVRFLOW_ENBL);

    /* now that we've got our act together, enable everything   */
    csWritePacketPage(cs, PKTPG_BUS_CTL, BUS_CTL_INT_ENBL);

    return TRUE;
}

/* ********************************************************************   */
/* CLOSE routines.                                                        */
/* ********************************************************************   */
void cs_close(PIFACE pi)
{
    ARGSUSED_PVOID(pi)
}

/* ********************************************************************   */
/* XMIT routines.                                                         */
/* ********************************************************************   */

/* Returns TRUE if xmit done or error                              */
/* Returns FALSE if xmit not done; if it is not done when the      */
/*   next xmit interrupt occurs, cs_xmit_done will be called again */
/*                                                                 */
RTIP_BOOLEAN cs_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PCS_PARMS cs;
int length;

    cs = iface_to_cs(pi);
    if (!cs)
    {
        set_errno(ENUMDEVICE);
        return(TRUE);
    }

    if (success)
    {
        if (cs->wait_alloc)
        {
            cs->wait_alloc = FALSE;


            length = DCUTOPACKET(msg)->length;
            if (length < ETHER_MIN_LEN)
                length = ETHER_MIN_LEN;

            if (length > ETHERSIZE)
            {
                DEBUG_ERROR("lan8900_xmit_done - length is too large: length = ",
                    EBS_INT1, length, 0);
                length = ETHERSIZE;       /* what a terriable hack! */
            }

            /* The chip is ready for transmission now                */
            /* Copy the message to the chip to start the transmit    */
            csCopyTxFrame(cs, msg, length);

            return(FALSE);  /* xmit not done */
        }
        else
        {
            /* Update total number of successfully transmitted packets.     */
            INCR_INFO(pi, interface_packets_out)
            UPDATE_INFO(pi, interface_bytes_out, DCUTOPACKET(msg)->length)
        }
    }
    else
    {
        INCR_INFO(pi, interface_errors_out)
    }

    return(TRUE);       /* xmit done */
}

/****************************************************************************   */
/*  cs_xmit()                                                                   */
/*                                                                              */
/*  This routine is called when pNA requests that a new message (packet) be     */
/*  transmitted.  The new transmit request is placed on the transmit request    */
/*  queue.  If a transmit request is not currently in progress, then the new    */
/*  transmit request is started, else it waits its turn in the queue.           */
/*                                                                              */
/*  Returns: REPORT_XMIT_DONE if xmit is done                                   */
/*         0 if the transmit started but is not done                            */
/*         errno if an error occured                                            */
/*                                                                              */
/****************************************************************************   */

int cs_xmit(PIFACE pi, DCU msg)
{
   word BusStatus;
   PCS_PARMS cs;
   int length;

    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)
        length = ETHER_MIN_LEN;

    if (length > ETHERSIZE)
    {
        DEBUG_ERROR("cs_xmit - length is too large: length = ",
            EBS_INT1, length, 0);
        length = ETHERSIZE;       /* what a terriable hack! */
    }

    cs = iface_to_cs(pi);
    if (!cs)
        return(ENUMDEVICE);

    /* Request that a transmit be started   */
    BusStatus = csRequestTransmit(cs, msg, length);

    /* If there was an error with the transmit bid   */
    if ( BusStatus & BUS_ST_TX_BID_ERR )
    {
        DEBUG_ERROR("lan8900_xmit: bus error on xmit", NOVAR, 0, 0);
          /* Free the transmit request message                              */
          /* Remove the transmit request from the transmit request queue    */
          return EMSGSIZE;  /* Bad transmit length */
    }
    else if ( BusStatus & BUS_ST_RDY4TXNOW )
    {
          cs->wait_alloc = FALSE;

          /* The chip is ready for transmission now                */
          /* Copy the message to the chip to start the transmit    */
          csCopyTxFrame(cs, msg, length);
    }
    else
    {
        /* wait until chip is ready; cs_xmit_done will start the   */
        /* transmit                                                */
        cs->wait_alloc = TRUE;
    }

   return 0;        /* xmit started but not done */
}


/****************************************************************************   */
/*  csRequestTransmit()                                                         */
/*                                                                              */
/*  This routine requests that a transmit be started by writing a transmit      */
/*  command and a frame length to the chip.  The contents of the BusStatus      */
/*  register is returned which indicates the success of the request.            */
/*                                                                              */
/****************************************************************************   */

word csRequestTransmit(PCS_PARMS cs, DCU msg, int length)
{
    /* Request that the transmit be started after all data has been copied   */
    if ( cs->csInMemoryMode )
    {
       csWritePacketPage(cs, PKTPG_TX_CMD, TX_CMD_START_ALL );
       csWritePacketPage(cs, PKTPG_TX_LENGTH, (word)length);
    }
    else  /* In IO mode */
    {
       LAN_OUTWORD( PORT_TX_CMD, TX_CMD_START_ALL );
       LAN_OUTWORD( PORT_TX_LENGTH, (word)length);
    }

    /* Return the BusStatus register which indicates success of the request   */
    return csReadPacketPage(cs,  PKTPG_BUS_ST );
}


/****************************************************************************   */
/*  csCopyTxFrame()                                                             */
/*                                                                              */
/*  This routine builds an ethernet header in the chip's transmit frame         */
/*  buffer and then copies the frame data from the list of message blocks       */
/*  to the chip's transmit frame buffer.  When all the data has been copied     */
/*  then the chip automatically starts to transmit the frame.                   */
/*                                                                              */
/*  The reason why this "simple" copy routine is so long and complicated is     */
/*  because all reads and writes to the chip must be done as 16-bit words.      */
/*  If a message block has an odd number of bytes, then the last byte must      */
/*  be saved and combined with the first byte of the next message block.        */
/*                                                                              */
/****************************************************************************   */

void csCopyTxFrame(PCS_PARMS cs, DCU msg, int length)
{
    word *pFrame = 0;
    word *pBuff;
    word *pBuffLimit;
    PFBYTE packet;
    int HaveExtraByte;
    union
    {
       unsigned char  byte[2];
       word word;
    } Straddle;

    packet = DCUTODATA(msg);

    /* Point pBuffLimit to just beyond the last word   */
    pBuffLimit = (word *)(packet+length);
    pBuff = (word *)packet;

    /* If there are odd bytes remaining in the buffer   */
    if ( (packet+length - packet) & 1 )
    {
        HaveExtraByte = TRUE;

         /* Point pBuffLimit to the extra byte   */
         pBuffLimit = (word *)(packet+length-1);
    }
    else  /* There is an even number of bytes remaining */
    {
        HaveExtraByte = FALSE;

        /* Point pBuffLimit to just beyond the last word   */
        pBuffLimit = (word *)(packet+length);
    }

    /* Copy the words in the buffer to the frame   */
    if ( cs->csInMemoryMode )
    {
        pFrame = cs->cs_pPacketPage + (PKTPG_TX_FRAME/2);
        while ( pBuff < pBuffLimit ) *pFrame++ = *pBuff++;
    }
    else
         while ( pBuff < pBuffLimit ) 
         {
            LAN_OUTWORD( PORT_RXTX_DATA, *pBuff );
            pBuff++;
         }

    /* If there is an extra byte left over in this buffer   */
    if ( HaveExtraByte )
       /* Save the extra byte for later   */
       Straddle.byte[0] = *(unsigned char *)pBuff;

    /* If there is an extra byte left over from the last buffer   */
    if ( HaveExtraByte )
    {
        /* Add a zero byte to make a word   */
        Straddle.byte[1] = 0;

        /* Save the last word   */
        if ( cs->csInMemoryMode )
            *pFrame = Straddle.word;
        else
            LAN_OUTWORD( PORT_RXTX_DATA, Straddle.word );
   }
}


/* ********************************************************************   */
/* INTERRUPT routines.                                                    */
/* ********************************************************************   */

/****************************************************************************   */
/*  lan_isr()                                                                   */
/*                                                                              */
/*  Network Interface Interrupt Service Routine                                 */
/*                                                                              */
/*  This routine is called whenever the chip generates                          */
/*  an interrupt.  This routine calls csProcessISQ() which handles the          */
/*  interrupt event(s).                                                         */
/*                                                                              */
/****************************************************************************   */

void lan_pre_isr(int minor_no)
{
    PCS_PARMS cs;

    cs = (PCS_PARMS)&(csparms[minor_no]);
    if (!cs)
        return;

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(cs->csIntNumber)
}

void lan_isr(int minor_number)
{
    PCS_PARMS cs;

    cs = (PCS_PARMS)&(csparms[minor_number]);

    /* Process the Interrupt Status Queue   */
    csProcessISQ(cs);

    DRIVER_MASK_ISR_ON(cs->csIntNumber)
}


/****************************************************************************   */
/*  csProcessISQ()                                                              */
/*                                                                              */
/*  This routine processes the events on the Interrupt Status Queue.  The       */
/*  events are read one at a time from the ISQ and the appropriate event        */
/*  handlers are called.  The ISQ is read until it is empty.  If the chip's     */
/*  interrupt request line is active, then reading a zero from the ISQ will     */
/*  deactivate the interrupt request line.                                      */
/*                                                                              */
/****************************************************************************   */

void csProcessISQ(PCS_PARMS cs)
{
    word Event;

    /* Read an event from the Interrupt Status Queue   */
    if ( cs->csInMemoryMode )
       Event = csReadPacketPage(cs,  PKTPG_ISQ );
    else
       Event = INWORD( PORT_ISQ );

    /* Process all the events in the Interrupt Status Queue   */
    while ( Event != 0 )
    {
       /* Dispatch to an event handler based on the register number   */
       switch ( Event & REG_NUM_MASK )
       {
          case REG_NUM_RX_EVENT:
             csReceiveEvent(cs, Event);
             break;
          case REG_NUM_TX_EVENT:
             csTransmitEvent(cs->pi);
             break;
          case REG_NUM_BUF_EVENT:
            /* @jla add for rdy4txnow interrupt  */
            csBufferEvent( cs->pi, Event );
          case REG_NUM_RX_MISS:
          case REG_NUM_TX_COL:
             /* Unused events   */
             break;
       }

       /* Read another event from the Interrupt Status Queue   */
       if ( cs->csInMemoryMode )
          Event = csReadPacketPage(cs,  PKTPG_ISQ );
       else
          Event = INWORD( PORT_ISQ );
    }
}


/****************************************************************************   */
/*  csReceiveEvent()                                                            */
/*                                                                              */
/*  This routine is called whenever a frame has been received at the chip.      */
/*  This routine gets a data buffer from pNA, copies the received frame         */
/*  into the data buffer and passes the frame up to pNA by calling pNA's        */
/*  Announce_Packet() entry point.                                              */
/*                                                                              */
/****************************************************************************   */

void csReceiveEvent(PCS_PARMS cs, word RxEvent )
{
    word *pFrame = 0;
    word *pBuff;
    word *pBuffLimit;
    DCU msg;
    word Length;
    union
    {
       unsigned char  byte[2];
       word word;
    } Last;

    /* Verify that it is an RxOK event   */
    if ( !(RxEvent & RX_EVENT_RX_OK) ) return;

    /* Read the header from the frame buffer   */
    if ( cs->csInMemoryMode )
    {
       pFrame = cs->cs_pPacketPage + (PKTPG_RX_LENGTH/2);
       Length = *pFrame++;
    }
    else  /* In IO mode */
    {
       INWORD( PORT_RXTX_DATA );  /* Discard RxStatus */
       Length = INWORD( PORT_RXTX_DATA );
    }

    /* Allocate a data buffer from pNA   */
    msg = os_alloc_packet_input(Length, DRIVER_ALLOC);
    if ( msg == CS_NULL )  /* If pNA doesn't have any more buffers */
    {
        DEBUG_ERROR("csReceiveEvent: alloc of DCU failed", NOVAR, 0, 0);
       /* Discard the received frame   */
       csWritePacketPage(cs, PKTPG_RX_CFG,
             (word)(csReadPacketPage(cs, PKTPG_RX_CFG) | RX_CFG_SKIP) );
       return;
    }

    DCUTOPACKET(msg)->length = Length;

    /* Setup pointers to the buffer for copying   */
    pBuff = (word *)DCUTODATA(msg);
    pBuffLimit = pBuff + (Length/2);

    /* Copy the frame from the chip to the pNA buffer   */
    if ( cs->csInMemoryMode )
       while ( pBuff < pBuffLimit ) *pBuff++ = *pFrame++;
    else
       while ( pBuff < pBuffLimit ) *pBuff++ = INWORD( PORT_RXTX_DATA );

    /* If there is an extra byte at the end of the frame   */
    if ( Length & 1 )
    {
       /* Read the byte as a word   */
       if ( cs->csInMemoryMode )
         Last.word = *pFrame;
       else
          Last.word = INWORD( PORT_RXTX_DATA );

       /* Write the byte as a byte   */
       *(DCUTODATA(msg)+DCUTOPACKET(msg)->length-1) = Last.byte[0];
    }

    /* Pass the received frame on up to pNA   */
#if (RTIP_VERSION > 24)
    ks_invoke_input(cs->pi,msg);
#else
    os_sndx_input_list(cs->pi, msg);
    ks_invoke_input(cs->pi);
#endif
}


/* @jla added this routine to send frame on rdy4tx interrupt                    */
/****************************************************************************   */
/*  csBufferEvent()                                                             */
/*                                                                              */
/*  This routine is called whenever a buffer event has occurred.                */
/*  It only handles Rdy4TXNow events.  All other buffer events are ignored.     */
/****************************************************************************   */

void csBufferEvent(PIFACE pi, word BufEvent )
{
    if ( BufEvent & BUF_EVENT_RDY4TX )
    {
        /* force cs_xmit_done routine to start transmission   */
        ks_invoke_output(pi, 1); /* signal output has completed */
    }
}


/****************************************************************************   */
/*  csTransmitEvent()                                                           */
/*                                                                              */
/*  This routine is called whenever the transmission of a frame has             */
/*  completed (either successfully or unsuccessfully).  This routine            */
/*  removes the completed transmit request from the transmit request queue.     */
/*  If there are more transmit requests waiting, then start the transmission    */
/*  of the next transmit request.                                               */
/*                                                                              */
/****************************************************************************   */

void csTransmitEvent(PIFACE pi)
{
    ks_invoke_output(pi, 1); /* signal output has completed */
}


/* ********************************************************************   */
/* UTILITY routines.                                                      */
/* ********************************************************************   */

/****************************************************************************   */
/*  csReadPacketPage()                                                          */
/*                                                                              */
/*  Reads the PacketPage register at the specified offset.                      */
/*                                                                              */
/****************************************************************************   */

word csReadPacketPage(PCS_PARMS cs, word Offset)
{
    if ( cs->csInMemoryMode )
    {
       return *(cs->cs_pPacketPage+(Offset/2));
    }
    else  /* In IO mode */
    {
#if (1)
       LAN_OUTWORD( PORT_PKTPG_PTR, Offset );
       return INWORD( PORT_PKTPG_DATA );
#else
{
word temp;
       temp = INWORD( PORT_PKTPG_DATA );
       DEBUG_ERROR("csReadPacketPage: offset, rslt = ", DINT2,
            Offset, temp);
        return(temp);
}
#endif
    }
}


/****************************************************************************   */
/*  csWritePacketPage()                                                         */
/*                                                                              */
/*  Writes to the PacketPage register at the specified offset.                  */
/*                                                                              */
/****************************************************************************   */

void csWritePacketPage(PCS_PARMS cs, word Offset, word Value)
{
    if ( cs->csInMemoryMode )
    {
       *(cs->cs_pPacketPage+(Offset/2)) = Value;
    }
    else  /* In IO mode */
    {
#if (DISPLAY_OUTPUT)
        DEBUG_ERROR("csWritePacketPage: offset, value: ", DINT2,
            Offset, Value);
#endif
       LAN_OUTWORD( PORT_PKTPG_PTR, Offset );
       LAN_OUTWORD( PORT_PKTPG_DATA, Value );
    }
}

/* ********************************************************************   */
/* MULTICAST                                                              */
/* ********************************************************************   */
/* setmcast() -
   Takes an interface structures a contiguous array
   of bytes containing N IP multicast addresses and n, the number
   of addresses (not number of bytes).
*/
RTIP_BOOLEAN cs_setmcast(PIFACE pi)     /* __fn__ */
{
    ARGSUSED_PVOID(pi)
    return(FALSE);
}

/* ********************************************************************          */
/* Statistic. return statistics about the device interface                       */
/*                                                                               */
/* This routine is called by user code that wishes to inspect driver statistics. */
/* We call this routine in the demo program. It is not absolutely necessary      */
/* to implement such a function (Leave it empty.), but it is a handy debugging   */
/* tool.                                                                         */
/*                                                                               */
/* The address of this function must be placed into the "devices" table in       */
/* iface.c either at compile time or before a device open is called.             */
/*                                                                               */
/*                                                                               */
/* Non packet drivers should behave the same way.                                */
/*                                                                               */

RTIP_BOOLEAN cs_statistics(PIFACE pi)                       /*__fn__*/
{
#if (INCLUDE_KEEP_STATS)
PCS_PARMS    sc;
#endif

#if (!INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi);
#endif

#if (INCLUDE_KEEP_STATS)
    sc = iface_to_cs(pi);
    if (!sc)
        return(FALSE);
#endif
   return(TRUE);
}

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_cs(int minor_number)
{
    return(xn_device_table_add(cs_device.device_id,
                        minor_number,
                        cs_device.iface_type,
                        cs_device.device_name,
                        SNMP_DEVICE_INFO(cs_device.media_mib,
                                         cs_device.speed)
                        (DEV_OPEN)cs_device.open,
                        (DEV_CLOSE)cs_device.close,
                        (DEV_XMIT)cs_device.xmit,
                        (DEV_XMIT_DONE)cs_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)cs_device.proc_interrupts,
                        (DEV_STATS)cs_device.statistics,
                        (DEV_SETMCAST)cs_device.setmcast));
}

#endif      /* DECLARING_DATA */
#endif      /* INCLUDE_LAN89X0 */
