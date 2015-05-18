/*
 * RTIP Device driver for ne2000/ne1000/83902.
 *
 * Device driver for National Semiconductor DS8390/WD83C690 based ethernet
 * adapters.
 *
 * Copyright (c) 1994 Charles Hannum.
 *
 * Copyright (C) 1993, David Greenman.  This software may be used, modified,
 * copied, distributed, and sold, in both source and binary form provided that
 * the above copyright and these terms are retained.  Under no circumstances is
 * the author responsible for the proper functioning of this software, nor does
 * the author assume any responsibility for damages incurred with its use.
 *
 * Currently supports the Western Digital/SMC 8003 and 8013 series, the SMC
 * Elite Ultra (8216), the 3Com 3c503, the NE1000 and NE2000, and a variety of
 * similar clones.
 *
 *  $Id: ne2000.c,v 1.1 2003/07/02 19:09:17 sarah Exp $
 */

#undef  DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_NE2000)
#include "ne2000.h"

RTIP_BOOLEAN ne2000_open(PIFACE pi);
void         ne2000_close(PIFACE pi);
int          ne2000_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN ne2000_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN ne2000_statistics(PIFACE  pi);
RTIP_BOOLEAN ne2000_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
NE2000_SOFTC KS_FAR ne2000softc[CFG_NUM_NE2000];

KS_CONSTANT byte KS_FAR ne2000_test_pattern[32] = "THIS is A memory TEST pattern";

EDEVTABLE KS_FAR ne2000_device =
{
     ne2000_open, ne2000_close, ne2000_xmit, ne2000_xmit_done,
     NULLP_FUNC, ne2000_statistics, ne2000_setmcast,
     NE2000_DEVICE, "NE2000", MINOR_0, ETHER_IFACE,
     SNMP_DEVICE_INFO(CFG_OID_NE2000, CFG_SPEED_NE2000)
     CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
     CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
     IOADD(0x300), EN(0x0), EN(5)
};
#endif  /* DECLARING_DATA */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern NE2000_SOFTC KS_FAR ne2000softc[CFG_NUM_NE2000];
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ne2000_test_pattern[32];
extern EDEVTABLE KS_FAR ne2000_device;
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define NEW_WAIT 1

#define DISPLAY_ERROR   0   /* display error during interrupt */
#define DEBUG_RING      0


#define FIX_RING_PTRS 1 /* SHOULD BE SET TO 1 */

#if (DEBUG_RING)
int recovery_count = 0;     /* count of ring buffer overflows */
#endif

/* ********************************************************************   */
#define ETHER_MAX_LEN   (ETHERSIZE+4)       /* includes header */
#define CRC_LEN         4

#define EROUND  ((sizeof(struct _ether) + 3) & ~3)
#define EOFF    (EROUND - sizeof(struct _ether))

/* ********************************************************************   */
void ne2000_getmcaf(PFBYTE mclist, int bytesmclist, PFBYTE af);
void ne2000_reset(PIFACE pi, PNE2000_SOFTC sc);
void ne2000_pre_interrupt(int minor_no);
void ne2000_interrupt(int minor_no);

int  ne2000_probe_Novell(PIFACE pi, PNE2000_SOFTC sc);
void ne2000_stop(PNE2000_SOFTC sc);

void    ne2000_init(PIFACE pi, PNE2000_SOFTC sc);
void    ne2000_pio_readmem(PNE2000_SOFTC sc, word src, PFBYTE dst, int amount);
RTIP_BOOLEAN ne2000_pio_write_pkt(PIFACE pi, PNE2000_SOFTC sc, PFCBYTE packet, int length, word dst);
word    ne2000_ring_copy(PNE2000_SOFTC sc, word src, PFBYTE dst, word amount);
void    ne2000_ring_overflow_recovery2(void KS_FAR *vp);
void    ne2000_get_packet(PIFACE pi, PNE2000_SOFTC sc, word nic_buf, int len, RTIP_BOOLEAN rint);
void    ne2000_read_ring_pkt(PIFACE pi, PNE2000_SOFTC sc, RTIP_BOOLEAN rint);
void    ne2000_proc_interrupts(PIFACE pi);

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_ne2000(int minor_number)
{
    return(xn_device_table_add(ne2000_device.device_id,
                        minor_number,
                        ne2000_device.iface_type,
                        ne2000_device.device_name,
                        SNMP_DEVICE_INFO(ne2000_device.media_mib,
                                         ne2000_device.speed)
                        (DEV_OPEN)ne2000_device.open,
                        (DEV_CLOSE)ne2000_device.close,
                        (DEV_XMIT)ne2000_device.xmit,
                        (DEV_XMIT_DONE)ne2000_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ne2000_device.proc_interrupts,
                        (DEV_STATS)ne2000_device.statistics,
                        (DEV_SETMCAST)ne2000_device.setmcast));
}

int xn_bind_ne2000_pcmcia(int minor_number)
{
/*int i;   */

    return(xn_device_table_add(NE2000_PCMCIA_DEVICE,
                        minor_number,
                        ne2000_device.iface_type,
                        "NE2000 PCMCIA DEVICE",
                        SNMP_DEVICE_INFO(ne2000_device.media_mib,
                                         ne2000_device.speed)
                        (DEV_OPEN)ne2000_device.open,
                        (DEV_CLOSE)ne2000_device.close,
                        (DEV_XMIT)ne2000_device.xmit,
                        (DEV_XMIT_DONE)ne2000_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ne2000_device.proc_interrupts,
                        (DEV_STATS)ne2000_device.statistics,
                        (DEV_SETMCAST)ne2000_device.setmcast));
}

#if (INCLUDE_SMC8041_PCMCIA)
int xn_bind_smc8041_pcmcia(int minor_number)
{
    return(xn_device_table_add(SMC8041_PCMCIA_DEVICE,
                        minor_number,
                        ne2000_device.iface_type,
                        "SMC8041 PCMCIA DEVICE",
                        SNMP_DEVICE_INFO(ne2000_device.media_mib,
                                         ne2000_device.speed)
                        (DEV_OPEN)ne2000_device.open,
                        (DEV_CLOSE)ne2000_device.close,
                        (DEV_XMIT)ne2000_device.xmit,
                        (DEV_XMIT_DONE)ne2000_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ne2000_device.proc_interrupts,
                        (DEV_STATS)ne2000_device.statistics,
                        (DEV_SETMCAST)ne2000_device.setmcast));
}
#endif  /* INCLUDE_SMC8041_PCMCIA */

/* ********************************************************************   */
/* MEMORY MANAGMENT                                                       */
/* ********************************************************************   */
#if (CFG_NUM_NE2000 == 1)
#define iface_to_softc(X) (PNE2000_SOFTC) &ne2000softc[0]
#define off_to_softc(X)   (PNE2000_SOFTC) &ne2000softc[0]
#else
static PNE2000_SOFTC iface_to_softc(PIFACE pi)
{
int softc_off;
    softc_off = pi->minor_number;
    if (softc_off >= CFG_NUM_NE2000)
    {
        DEBUG_ERROR("iface_to_softc() - pi->minor_number, CFG_NUM_NE2000 =",
            EBS_INT2, pi->minor_number, CFG_NUM_NE2000);
        return((PNE2000_SOFTC)0);
    }

    return((PNE2000_SOFTC) &ne2000softc[softc_off]);
}

static PNE2000_SOFTC off_to_softc(int softc_off)
{
    if (softc_off >= CFG_NUM_NE2000)
    {
        DEBUG_ERROR("off_to_softc() - softc_off, CFG_NUM_NE2000 =",
            EBS_INT2, softc_off, CFG_NUM_NE2000);
        return((PNE2000_SOFTC)0);
    }
    return((PNE2000_SOFTC) &ne2000softc[softc_off]);
}
#endif

#if (!CFG_NE2000_HW)
/* ********************************************************************   */
/* Probing and Initialization routines.                                   */
/* ********************************************************************   */
/* set board values to either values set by application or default values */
/* specified by device table                                              */

void set_ne2000_vals(PIFACE pi, PNE2000_SOFTC sc)
{
    /* set values based on global variable set by application if they are   */
    /* not set to their initial value otherwise                             */
    /* set them to default values from the device table                     */
    /* io base address                                                      */
    sc->ia_iobase = pi->io_address;
    sc->ia_irq    = pi->irq_val;
}
#endif

/*
 * Generic probe routine for testing for the existence of a DS8390.  Must be
 * called after the NIC has just been reset.  This routine works by looking at
 * certain register values that are gauranteed to be initialized a certain way
 * after power-up or reset.  Seems not to currently work on the 83C690.
 *
 * Specifically:
 *
 *  Register                    reset bits  set bits
 *  Command Register (CR)       TXP, STA    RD2, STP
 *  Interrupt Status (ISR)              RST
 *  Interrupt Mask (IMR)        All bits
 *  Data Control (DCR)              LAS
 *  Transmit Config. (TCR)      LB1, LB0
 *
 * We only look at the CR and ISR registers, however, because looking at the
 * others would require changing register pages (which would be intrusive if
 * this isn't an 8390).
 *
 * Return 1 if 8390 was found, 0 if not.
 */
#if (CFG_NE2000_PROBE)
int ne2000_probe_generic8390(PNE2000_SOFTC sc)
{
    if ((INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR)) &
         (NE2000_CR_RD2 | NE2000_CR_TXP | NE2000_CR_STA | NE2000_CR_STP)) !=
         (NE2000_CR_RD2 | NE2000_CR_STP))
        return (0);
    if ((INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR)) & NE2000_ISR_RST) != NE2000_ISR_RST)
        return (0);

    return (1);
}

/*
 * Probe and vendor-specific initialization routine for NE1000/2000 boards.
 */
int ne2000_probe_Novell(PIFACE pi, PNE2000_SOFTC sc)
{
word memsize, n;
byte romdata[16], tmp;
byte test_buffer[32];
IOADDRESS nic_addr;
IOADDRESS asic_addr;

    sc->asic_addr = (IOADDRESS) (sc->ia_iobase + (word) NE2000_NOVELL_ASIC_OFFSET);
    asic_addr = sc->asic_addr; /* used in the macro to calculate register pointers */
    ASSIGN_ASIC_ADDR(NE2000_NOVELL_DATA,  asic_addr_NE2000_NOVELL_DATA)
    ASSIGN_ASIC_ADDR(NE2000_NOVELL_RESET, asic_addr_NE2000_NOVELL_RESET)

    sc->nic_addr = (IOADDRESS)(sc->ia_iobase + NE2000_NOVELL_NIC_OFFSET);
    nic_addr = sc->nic_addr; /* used in the macro to calculate register pointers */
    ASSIGN_REG_ADDR(NE2000_P0_BNRY,nic_addr_NE2000_P0_BNRY)
    ASSIGN_REG_ADDR(NE2000_P0_CNTR0,nic_addr_NE2000_P0_CNTR0)
    ASSIGN_REG_ADDR(NE2000_P0_CNTR1,nic_addr_NE2000_P0_CNTR1)
    ASSIGN_REG_ADDR(NE2000_P0_DCR,nic_addr_NE2000_P0_DCR)
    ASSIGN_REG_ADDR(NE2000_P0_CNTR2,nic_addr_NE2000_P0_CNTR2)
    ASSIGN_REG_ADDR(NE2000_P0_CR,nic_addr_NE2000_P0_CR)
    ASSIGN_REG_ADDR(NE2000_P0_IMR,nic_addr_NE2000_P0_IMR)
    ASSIGN_REG_ADDR(NE2000_P0_ISR,nic_addr_NE2000_P0_ISR)
    ASSIGN_REG_ADDR(NE2000_P0_NCR,nic_addr_NE2000_P0_NCR)
    ASSIGN_REG_ADDR(NE2000_P0_PSTART,nic_addr_NE2000_P0_PSTART)
    ASSIGN_REG_ADDR(NE2000_P0_PSTOP,nic_addr_NE2000_P0_PSTOP)
    ASSIGN_REG_ADDR(NE2000_P0_RBCR0,nic_addr_NE2000_P0_RBCR0)
    ASSIGN_REG_ADDR(NE2000_P0_RBCR1,nic_addr_NE2000_P0_RBCR1)
    ASSIGN_REG_ADDR(NE2000_P0_RCR,nic_addr_NE2000_P0_RCR)
    ASSIGN_REG_ADDR(NE2000_P0_RSAR0,nic_addr_NE2000_P0_RSAR0)
    ASSIGN_REG_ADDR(NE2000_P0_RSAR1,nic_addr_NE2000_P0_RSAR1)
    ASSIGN_REG_ADDR(NE2000_P0_RSR,nic_addr_NE2000_P0_RSR)
    ASSIGN_REG_ADDR(NE2000_P0_TBCR0,nic_addr_NE2000_P0_TBCR0)
    ASSIGN_REG_ADDR(NE2000_P0_TBCR1,nic_addr_NE2000_P0_TBCR1)
    ASSIGN_REG_ADDR(NE2000_P0_TCR,nic_addr_NE2000_P0_TCR)
    ASSIGN_REG_ADDR(NE2000_P0_TPSR,nic_addr_NE2000_P0_TPSR)
    ASSIGN_REG_ADDR(NE2000_P0_TSR,nic_addr_NE2000_P0_TSR)
    ASSIGN_REG_ADDR(NE2000_P1_CR,nic_addr_NE2000_P1_CR)
    ASSIGN_REG_ADDR(NE2000_P1_CURR,nic_addr_NE2000_P1_CURR)
    ASSIGN_REG_ADDR(NE2000_P1_MAR0,nic_addr_NE2000_P1_MAR0)
    ASSIGN_REG_ADDR(NE2000_P1_MAR1,nic_addr_NE2000_P1_MAR1)
    ASSIGN_REG_ADDR(NE2000_P1_MAR2,nic_addr_NE2000_P1_MAR2)
    ASSIGN_REG_ADDR(NE2000_P1_MAR3,nic_addr_NE2000_P1_MAR3)
    ASSIGN_REG_ADDR(NE2000_P1_MAR4,nic_addr_NE2000_P1_MAR4)
    ASSIGN_REG_ADDR(NE2000_P1_MAR5,nic_addr_NE2000_P1_MAR5)
    ASSIGN_REG_ADDR(NE2000_P1_MAR6,nic_addr_NE2000_P1_MAR6)
    ASSIGN_REG_ADDR(NE2000_P1_MAR7,nic_addr_NE2000_P1_MAR7)
    ASSIGN_REG_ADDR(NE2000_P1_PAR0,nic_addr_NE2000_P1_PAR0)
    /* XXX - do Novell-specific probe here   */
    /* Reset the board.                      */
    tmp = INBYTE_NE2000(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_RESET));
    /*
     * I don't know if this is necessary; probably cruft leftover from
     * Clarkson packet driver code. Doesn't do a thing on the boards I've
     * tested. -DG [note that a _outp(0x84, 0) seems to work here, and is
     * non-invasive...but some boards don't seem to reset and I don't have
     * complete documentation on what the 'right' thing to do is...so we do
     * the invasive thing for now.  Yuck.]
     */
    OUTBYTE_NE2000(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_RESET), tmp);
    ks_sleep(2);    /* 5000 usec */
    /*
     * This is needed because some NE clones apparently don't reset the NIC
     * properly (or the NIC chip doesn't reset fully on power-up)
     * XXX - this makes the probe invasive! ...Done against my better
     * judgement.  -DLG
     */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STP);
    ks_sleep(2);    /* 5000 usec */

    /* Make sure that we really have an 8390 based board.   */
    if (!ne2000_probe_generic8390(sc))
    {
        return (0);
    }

    /* 8k of memory plus an additional 8k if 16-bit.   */
    memsize = (word) (8192 + (word)sc->isa16bit * 8192);

    /* NIC memory doesn't start at zero on an NE board.   */
    /* The start address is tied to the bus width.        */
    sc->mem_start = (word)(8192 + sc->isa16bit * 8192);
    sc->tx_page_start = (word) (memsize / NE2000_PAGE_SIZE);
    sc->mem_size = memsize;
    sc->mem_end = (word)(sc->mem_start + memsize);

    sc->rec_page_start = (word) (sc->tx_page_start + NE2000_TXBUF_SIZE);
    sc->rec_page_stop = (word) (sc->tx_page_start + memsize / NE2000_PAGE_SIZE);

    sc->mem_ring = (word)(sc->mem_start + NE2000_PAGE_SIZE * NE2000_TXBUF_SIZE);
    /*
     * Test the ability to read and write to the NIC memory.  This has the
     * side affect of determining if this is an NE1000 or an NE2000.
     */
    /*
     * This prevents packets from being stored in the NIC memory when the
     * readmem routine turns on the start bit in the CR.
     */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RCR), NE2000_RCR_MON);

    /* Temporarily initialize DCR for byte operations.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_FT1 | NE2000_DCR_LS);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTART), 8192 / NE2000_PAGE_SIZE);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTOP), 16384 / NE2000_PAGE_SIZE);
    sc->isa16bit = 0;
    /*
     * Write a test pattern in byte mode.  If this fails, then there
     * probably isn't any memory at 8k - which likely means that the board
     * is an NE2000.
     */
    ne2000_pio_write_pkt((PIFACE) 0, sc, ne2000_test_pattern, sizeof(ne2000_test_pattern), 8192);
/*  ne2000_pio_writemem(sc, ne2000_test_pattern, 8192, sizeof(ne2000_test_pattern));   */
    ne2000_pio_readmem(sc, 8192, (PFBYTE)test_buffer, sizeof(test_buffer));

    if (!tc_comparen(ne2000_test_pattern, test_buffer, sizeof(test_buffer)))
    {
        /* not an NE1000 - try NE2000   */
#if (KS_LITTLE_ENDIAN)  /*DM: 9-27-02: added conditional and big endian case */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_WTS | NE2000_DCR_FT1 | NE2000_DCR_LS);
#else
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_WTS | NE2000_DCR_FT1 | NE2000_DCR_LS | NE2000_DCR_BOS);
#endif
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTART), 16384 / NE2000_PAGE_SIZE);
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTOP), (32768l / NE2000_PAGE_SIZE) );
        sc->isa16bit = 1;

        /*
         * Write a test pattern in word mode.  If this also fails, then
         * we don't know what this board is.
         */
        ne2000_pio_write_pkt((PIFACE) 0, sc, ne2000_test_pattern, sizeof(ne2000_test_pattern), 16384);
/*      ne2000_pio_writemem(sc, ne2000_test_pattern, 16384, sizeof(ne2000_test_pattern));   */
        ne2000_pio_readmem(sc, 16384, (PFBYTE)test_buffer, sizeof(test_buffer));

        if (!tc_comparen(ne2000_test_pattern, test_buffer, sizeof(test_buffer)))
            return (0); /* not an NE2000 either */
    }

    /* 8k of memory plus an additional 8k if 16-bit.   */
    memsize = (word) (8192 + (word)sc->isa16bit * 8192);

    /* NIC memory doesn't start at zero on an NE board.   */
    /* The start address is tied to the bus width.        */
    sc->mem_start = (word)(8192 + sc->isa16bit * 8192);
    sc->tx_page_start = (word) (memsize / NE2000_PAGE_SIZE);

    sc->mem_size = memsize;
    sc->mem_end = (word)(sc->mem_start + memsize);

    sc->rec_page_start = (word) (sc->tx_page_start + NE2000_TXBUF_SIZE);
    sc->rec_page_stop = (word) (sc->tx_page_start + memsize / NE2000_PAGE_SIZE);

    sc->mem_ring = (word)(sc->mem_start + NE2000_PAGE_SIZE * NE2000_TXBUF_SIZE);

    ne2000_pio_readmem(sc, 0, romdata, 16);
    for (n = 0; n < ETH_ALEN; n++)
    {
        if (sc->isa16bit)
            pi->addr.my_hw_addr[n] = (byte) ((word *)romdata)[n];
        else
            pi->addr.my_hw_addr[n] = romdata[n];
    }

    /* Clear any pending interrupts that might have occurred above.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR), 0xff);

    return (1);
}
#else
/*
 * Probe and vendor-specific initialization routine for NE1000/2000 boards.
 */
int ne2000_probe_Novell(PIFACE pi, PNE2000_SOFTC sc)
{
word n;
byte romdata[16], tmp;
IOADDRESS nic_addr;
IOADDRESS asic_addr;

    sc->asic_addr = (IOADDRESS) (sc->ia_iobase + (word) NE2000_NOVELL_ASIC_OFFSET);
    asic_addr = sc->asic_addr; /* used in the macro to calculate register pointers */
    ASSIGN_ASIC_ADDR(NE2000_NOVELL_DATA,  asic_addr_NE2000_NOVELL_DATA)
    ASSIGN_ASIC_ADDR(NE2000_NOVELL_RESET, asic_addr_NE2000_NOVELL_RESET)

    sc->nic_addr = (IOADDRESS)(sc->ia_iobase + NE2000_NOVELL_NIC_OFFSET);
    nic_addr = sc->nic_addr; /* used in the macro to calculate register pointers */
    ASSIGN_REG_ADDR(NE2000_P0_BNRY,nic_addr_NE2000_P0_BNRY)
    ASSIGN_REG_ADDR(NE2000_P0_CNTR0,nic_addr_NE2000_P0_CNTR0)
    ASSIGN_REG_ADDR(NE2000_P0_CNTR1,nic_addr_NE2000_P0_CNTR1)
    ASSIGN_REG_ADDR(NE2000_P0_DCR,nic_addr_NE2000_P0_DCR)
    ASSIGN_REG_ADDR(NE2000_P0_CNTR2,nic_addr_NE2000_P0_CNTR2)
    ASSIGN_REG_ADDR(NE2000_P0_CR,nic_addr_NE2000_P0_CR)
    ASSIGN_REG_ADDR(NE2000_P0_IMR,nic_addr_NE2000_P0_IMR)
    ASSIGN_REG_ADDR(NE2000_P0_ISR,nic_addr_NE2000_P0_ISR)
    ASSIGN_REG_ADDR(NE2000_P0_NCR,nic_addr_NE2000_P0_NCR)
    ASSIGN_REG_ADDR(NE2000_P0_PSTART,nic_addr_NE2000_P0_PSTART)
    ASSIGN_REG_ADDR(NE2000_P0_PSTOP,nic_addr_NE2000_P0_PSTOP)
    ASSIGN_REG_ADDR(NE2000_P0_RBCR0,nic_addr_NE2000_P0_RBCR0)
    ASSIGN_REG_ADDR(NE2000_P0_RBCR1,nic_addr_NE2000_P0_RBCR1)
    ASSIGN_REG_ADDR(NE2000_P0_RCR,nic_addr_NE2000_P0_RCR)
    ASSIGN_REG_ADDR(NE2000_P0_RSAR0,nic_addr_NE2000_P0_RSAR0)
    ASSIGN_REG_ADDR(NE2000_P0_RSAR1,nic_addr_NE2000_P0_RSAR1)
    ASSIGN_REG_ADDR(NE2000_P0_RSR,nic_addr_NE2000_P0_RSR)
    ASSIGN_REG_ADDR(NE2000_P0_TBCR0,nic_addr_NE2000_P0_TBCR0)
    ASSIGN_REG_ADDR(NE2000_P0_TBCR1,nic_addr_NE2000_P0_TBCR1)
    ASSIGN_REG_ADDR(NE2000_P0_TCR,nic_addr_NE2000_P0_TCR)
    ASSIGN_REG_ADDR(NE2000_P0_TPSR,nic_addr_NE2000_P0_TPSR)
    ASSIGN_REG_ADDR(NE2000_P0_TSR,nic_addr_NE2000_P0_TSR)
    ASSIGN_REG_ADDR(NE2000_P1_CR,nic_addr_NE2000_P1_CR)
    ASSIGN_REG_ADDR(NE2000_P1_CURR,nic_addr_NE2000_P1_CURR)
    ASSIGN_REG_ADDR(NE2000_P1_MAR0,nic_addr_NE2000_P1_MAR0)
    ASSIGN_REG_ADDR(NE2000_P1_MAR1,nic_addr_NE2000_P1_MAR1)
    ASSIGN_REG_ADDR(NE2000_P1_MAR2,nic_addr_NE2000_P1_MAR2)
    ASSIGN_REG_ADDR(NE2000_P1_MAR3,nic_addr_NE2000_P1_MAR3)
    ASSIGN_REG_ADDR(NE2000_P1_MAR4,nic_addr_NE2000_P1_MAR4)
    ASSIGN_REG_ADDR(NE2000_P1_MAR5,nic_addr_NE2000_P1_MAR5)
    ASSIGN_REG_ADDR(NE2000_P1_MAR6,nic_addr_NE2000_P1_MAR6)
    ASSIGN_REG_ADDR(NE2000_P1_MAR7,nic_addr_NE2000_P1_MAR7)
    ASSIGN_REG_ADDR(NE2000_P1_PAR0,nic_addr_NE2000_P1_PAR0)
    /* Reset the board.   */
    tmp = INBYTE_NE2000(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_RESET));
    OUTBYTE_NE2000(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_RESET), tmp);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STP);
    ks_sleep(2);    /* 5000 usec */
#if (CFG_FORCE_NE1000)
    /* initialize DCR for byte operations.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_FT1 | NE2000_DCR_LS);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTART), 8192 / NE2000_PAGE_SIZE);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTOP), 16384 / NE2000_PAGE_SIZE);
    sc->isa16bit = 0;
#define MEMSIZE  8192
#define MEMSTART 8192
#else
    /* not an NE1000 - try NE2000   */
#if (KS_LITTLE_ENDIAN)  /*DM: 9-27-02: added conditional and big endian case */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_WTS | NE2000_DCR_FT1 | NE2000_DCR_LS);
#else
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_WTS | NE2000_DCR_FT1 | NE2000_DCR_LS | NE2000_DCR_BOS);
#endif
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTART), 16384 / NE2000_PAGE_SIZE);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTOP), (32768l / NE2000_PAGE_SIZE) );
    sc->isa16bit = 1;
#define MEMSIZE  16384
#define MEMSTART 16384
#endif
    sc->mem_start = MEMSTART;
    sc->mem_size = MEMSIZE;
    sc->mem_end = (word)((long)MEMSTART + (long)MEMSIZE);

    /* XMIT Page offset is the begining of ram to the NE
       RCV Page offset is xmit offset + size of xmit ring
       RCV End is Begining of ram + size of rem */
    sc->tx_page_start = (word) (MEMSTART / NE2000_PAGE_SIZE);
    sc->rec_page_start = (word)((MEMSTART / NE2000_PAGE_SIZE) + NE2000_TXBUF_SIZE);
    sc->rec_page_stop =  (word) ((MEMSTART / NE2000_PAGE_SIZE) + MEMSIZE / NE2000_PAGE_SIZE);
    sc->mem_ring = MEMSTART + NE2000_PAGE_SIZE * NE2000_TXBUF_SIZE;
    ne2000_pio_readmem(sc, 0, romdata, 16);
    for (n = 0; n < ETH_ALEN; n++)
        pi->addr.my_hw_addr[n] = romdata[n*(sc->isa16bit+1)];

    /* Clear any pending interrupts that might have occurred above.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR), 0xff);

    return (1);
}
#endif /* if (CFG_NE2000_PROBE) */

/* ********************************************************************   */
/* OPEN and CLOSE routines.                                               */
/* ********************************************************************   */

void ne2000_close(PIFACE pi)                     /*__fn__*/
{
PNE2000_SOFTC sc;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_PVOID(pi);
#endif
    sc = iface_to_softc(pi);
    if (!sc)
        return;
    ebs_stop_timer(&(sc->ne2000_timer_info));
    ne2000_stop(sc);
}

#if (INCLUDE_PCMCIA)
int  pcmcia_card_type(int socket);
#endif

/* ********************************************************************   */
/* Install interface into kernel networking data structures.              */
/* Returns TRUE if successful, FALSE if not                               */
RTIP_BOOLEAN ne2000_open(PIFACE pi)
{
PNE2000_SOFTC sc;
#if (INCLUDE_PCMCIA)
int ne2000_socket_number;
#endif

    sc = iface_to_softc(pi);
    if (!sc)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    tc_memset((PFBYTE) sc, 0, sizeof(*sc));

    /* set up pointers between iface and ne2000_softc structures   */
    sc->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(sc->stats);

#if (INCLUDE_PCMCIA)
    /* Check the slots to find the smc card   */
    /* Must be done before set_smcx_vals()    */
    if ((pi->pdev->device_id == NE2000_PCMCIA_DEVICE) || 
        (pi->pdev->device_id == SMC8041_PCMCIA_DEVICE))
    {
        ne2000_socket_number = 1;           /* pcmcia_card_type(1) == 4 */
        if (pcmcia_card_type(0) == 4)
            ne2000_socket_number = 0;
        else if (pcmcia_card_type(1) != 4)
        {
            DEBUG_ERROR("Can't find NE2000 card", NOVAR, 0, 0);
            set_errno(EPROBEFAIL);
            return(FALSE);
        }
    }
#endif

    /* determine which device and initialize   */
#if (CFG_NE2000_HW)
    sc->ia_iobase = CFG_NE2000_IOBASE;
    sc->ia_irq    = CFG_NE2000_IRQ;
#else
    set_ne2000_vals(pi, sc);
#endif

#if (INCLUDE_PCMCIA)
   if ( ((pi->pdev->device_id == NE2000_PCMCIA_DEVICE) || (pi->pdev->device_id == SMC8041_PCMCIA_DEVICE)) &&
     card_is_ne2000(ne2000_socket_number, sc->ia_irq, sc->ia_iobase))   /* 1st parm is socket */
    {
        if (!ne2000_probe_Novell(pi, sc))
        {
            pcmctrl_card_down(ne2000_socket_number);
            return (FALSE);
        }
    }
#endif
    if (!(ne2000_probe_Novell(pi, sc)))
    {
        DEBUG_ERROR("ne2000probe failed", NOVAR, 0, 0);
        set_errno(EPROBEFAIL);
        return(FALSE);
    }
    /* Set interface to stopped condition (reset).   */
    ne2000_stop(sc);

    /* start a timer to check for ring overflow recovery    */
    /* NOTE: if timer already started, will not start again */
    sc->ne2000_timer_info.func = ne2000_ring_overflow_recovery2;
                                    /* routine to execute if timeout   */
    sc->ne2000_timer_info.arg = (void KS_FAR *)(dword)pi->minor_number;
                                    /* arg to recovery2 routine   */
    ebs_set_timer(&(sc->ne2000_timer_info), 1, FALSE);
    ebs_start_timer(&(sc->ne2000_timer_info));

    /* Initialize ifnet structure.   */
    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)ne2000_interrupt,
                      (RTIPINTFN_POINTER)ne2000_pre_interrupt,
                      pi->minor_number);

    ne2000_reset(pi, sc);
    return(TRUE);
}

/* Reset interface.   */
void ne2000_reset(PIFACE pi, PNE2000_SOFTC sc)
{
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupt flags */

    sp = ks_splx();     /* Disable interrupts */
    ne2000_stop(sc);
    ne2000_init(pi, sc);
    ks_spl(sp);
}

/* Take interface offline.   */
void ne2000_stop(PNE2000_SOFTC sc)
{
int n = 5000;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_PVOID(sc);
#endif

    /* Stop everything on the interface, and select page 0 registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STP);

    /*
     * Wait for interface to enter stopped state, but limit # of checks to
     * 'n' (about 5ms).  It shouldn't even take 5us on modern DS8390's, but
     * just in case it's an old one.
     */
    while (((INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR)) & NE2000_ISR_RST) == 0) && --n);
}

/* Initialize device.    */
void ne2000_init(PIFACE pi, PNE2000_SOFTC sc)
{
int i;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupt flags */
byte mcaf[8];   /* multicast address filter */

    /*
     * Initialize the NIC in the exact order outlined in the NS manual.
     * This init procedure is "mandatory"...don't change what or when
     * things happen.
     */

    sp = ks_splx();     /* Disable interrupts */

    /* This variable is used below - don't move this assignment.   */
    sc->next_packet = (word) (sc->rec_page_start + 1);

    /* Set interface for page 0, remote DMA complete, stopped.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STP);

    if (sc->isa16bit)
    {
#if (KS_LITTLE_ENDIAN)  /*DM: 9-27-02: added conditional and big endian case */
        /*
         * Set FIFO threshold to 8, No auto-init Remote DMA, byte
         * order=80x86 (little endian), word-wide DMA xfers,
         */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_FT1 | NE2000_DCR_WTS | NE2000_DCR_LS);
#else
        /*
         * Set FIFO threshold to 8, No auto-init Remote DMA, byte
         * order=68000 (big endian), word-wide DMA xfers,
         */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_FT1 | NE2000_DCR_WTS | NE2000_DCR_LS | NE2000_DCR_BOS);
#endif
    }
    else
    {
        /* Same as above, but byte-wide DMA xfers.   */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_DCR), NE2000_DCR_FT1 | NE2000_DCR_LS);
    }

    /* Clear remote byte count registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR0), 0);

    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR1), 0);

    /* Tell RCR to do nothing for now.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RCR), NE2000_RCR_MON);

    /* Place NIC in internal loopback mode.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TCR), NE2000_TCR_LB0);

    /* Initialize transmit/receive (ring-buffer) page start.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TPSR), sc->tx_page_start);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTART), sc->rec_page_start);

    /* Initialize receiver (ring-buffer) page stop and boundary.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_PSTOP), sc->rec_page_stop);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_BNRY), sc->rec_page_start);

    /*
     * Clear all interrupts.  A '1' in each bit position clears the
     * corresponding flag.
     */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR), 0xff);

    /*
     * Enable the following interrupts: receive/transmit complete,
     * receive/transmit error, and Receiver OverWrite.
     *
     * Counter overflow and Remote DMA complete are *not* enabled.
     */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_IMR), NORMAL_NE2000_INTS);

    /* Program command register for page 1.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_1 | NE2000_CR_STP);

    /* Copy out our station (ethernet) address.   */
    for (i = 0; i < ETH_ALEN; ++i)
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_PAR0) + i, pi->addr.my_hw_addr[i]);

    /* Set current page pointer to next_packet (initialized above).   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_CURR), sc->next_packet);


    /* Set multicast filter on chip.            */
    /* If none needed lenmclist will be zero    */
    ne2000_getmcaf((PFBYTE) pi->mcast.mclist, pi->mcast.lenmclist,
                mcaf);

    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR0), mcaf[0]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR1), mcaf[1]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR2), mcaf[2]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR3), mcaf[3]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR4), mcaf[4]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR5), mcaf[5]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR6), mcaf[6]);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_MAR7), mcaf[7]);


    /* ====================                    */
    /* Program command register for page 0.    */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STP);

    /* set broadcast mode                     */
    /* NOTE: promiscous mode is not supported */
/*  i = NE2000_RCR_AB;   */

    /* Accept multicasts and broadcasts   */
    i = NE2000_RCR_AB | NE2000_RCR_AM;
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RCR), i);

    /* Take interface out of loopback.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TCR), 0);
    /* Fire up the interface.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    ks_spl(sp);
}

/* ********************************************************************   */
/* MULTICAST                                                              */
/* ********************************************************************   */
/* setmcast() -
   Takes an interface structures a contiguous array
   of bytes containing N IP multicast addresses and n, the number
   of addresses (not number of bytes).
   Copies the bytes to the driver structures multicast table and
   calls reset to set the multicast table in the board.
*/

RTIP_BOOLEAN ne2000_setmcast(PIFACE pi)     /* __fn__ */
{
PNE2000_SOFTC sc;

    sc = iface_to_softc(pi);
    if (!sc)
        return(FALSE);

    /* Call reset to load the multicast table   */
    ne2000_reset(pi, sc);
    return(TRUE);
}

/* ********************************************************************   */
/* XMIT routines.                                                         */
/* ********************************************************************   */

/* ********************************************************************   */
RTIP_BOOLEAN ne2000_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PNE2000_SOFTC sc;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_PVOID(pi);
#endif

    sc = iface_to_softc(pi);
    if (!sc)
        return(TRUE);

    if (!success)
    {
        sc->stats.errors_out++;
        sc->stats.tx_other_errors++;
    }
    else
    {
        /* Update total number of successfully transmitted packets.   */
        sc->stats.packets_out++;
        sc->stats.bytes_out += DCUTOPACKET(msg)->length;
    }
    return(TRUE);
}

/* ********************************************************************      */
/* This routine actually starts the transmission on the interface.           */
/* ********************************************************************      */
/* Transmit. a packet over the packet driver interface.                      */
/*                                                                           */
/* This routine is called when a packet needs sending. The packet contains a */
/* full ethernet frame to be transmitted. The length of the packet is        */
/* provided.                                                                 */
/*                                                                           */
/* The address of this function must be placed into the "devices" table in   */
/* iface.c either at compile time or before a device open is called.         */
/*                                                                           */
/* Non packet drivers should behave the same way.                            */
/*                                                                           */

int ne2000_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
PNE2000_SOFTC sc;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupt flags */
int length;
int ret_val;

    sc = iface_to_softc(pi);
    if (!sc)
        return(ENUMDEVICE);
    if (sc->do_recovery)
        return(ENETDOWN);

    /* make sure packet is within legal size   */
    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)
        length = ETHER_MIN_LEN;

    if (length > ETHERSIZE)
    {
        DEBUG_ERROR("ne2000xmit - length is too large: length = ",
            EBS_INT1, length, 0);
        length = ETHERSIZE;       /* what a terriable hack! */
    }

    /* Send requires interrupts disabled. So the ISR does not change
       the register back out from under us (particularly the dma
       pointer) */

    sp = ks_splx();     /* Disable interrupts */

    /* Set NIC to page 0 registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    /* mask off all isr's   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_IMR), 0);
    ks_spl(sp);

    ret_val = 0;
    if (ne2000_pio_write_pkt(pi, sc, DCUTODATA(msg), length, sc->mem_start))
    {
        /* Set NIC for page 0 register access.   */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);
        /* Set TX buffer start page.   */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TPSR), sc->tx_page_start);
        /* Set TX length.   */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TBCR0), length);
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TBCR1), length >> 8);
        /* Set page 0, remote DMA complete, transmit packet, and *start*.   */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_TXP | NE2000_CR_STA);
    }
    else
    {
        ret_val = ENETDOWN;
    }

    sp = ks_splx();     /* Disable interrupts */

    /* Set NIC to page 0 registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);
    /* Turn ints back on   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_IMR), NORMAL_NE2000_INTS);
    ks_spl(sp);

    return(ret_val);
}


/* ********************************************************************   */
/* INTERRUPT routines.                                                    */
/* ********************************************************************   */
void ne2000_pre_interrupt(int minor_no)
{
PNE2000_SOFTC sc;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_INT(minor_no)
#endif

    sc = off_to_softc(minor_no);

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq)
}


/* handle interrupts - this is the interrupt routine   */
void ne2000_interrupt(int minor_no)   /*__fn__*/
{
PNE2000_SOFTC sc;
PIFACE pi;
byte isr;
int  error;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_INT(minor_no);
#endif

    sc = off_to_softc(minor_no);
    if (!sc)
    {
        DEBUG_ERROR("ne2000_interrupt: no sc-can't enable interrupts", NOVAR, 0, 0);
        return;
    }
    pi = sc->iface;
    if (!pi)
    {
        DRIVER_MASK_ISR_ON(sc->ia_irq)
        return;
    }
    ks_enable();

    /* Set NIC to page 0 registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    /* get interrupt status register   */
    isr = INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR));

    /* Loop until there are no more new interrupts.   */
    while (isr)
    {
        /*
         * Reset all the bits in the interrupt status register that we
         * are 'acknowledging' by writing a '1' to each bit position
         * that was set. (Writing a '1' *clears* the bit.)
         */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR), isr);

        /* ******   */
        /*
         * Handle transmitter interrupts.  Handle these first because
         * the receiver will reset the board under some conditions.
         */
        if (isr & (NE2000_ISR_PTX | NE2000_ISR_TXE)) /* pkt xmit or xmit error */
        {
            int collisions = INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_NCR)) & 0x0f;

            /*
             * Check for transmit error.  If a TX completed with an
             * error, we end up throwing the packet away.  Really
             * the only error that is possible is excessive
             * collisions, and in this case it is best to allow the
             * automatic mechanisms of TCP to backoff the flow.  Of
             * course, with UDP we're screwed, but this is expected
             * when a network is heavily loaded.
             */
            error = INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TSR));
            if ( (isr & NE2000_ISR_TXE) ||
                 (error & (NE2000_TSR_CDH | NE2000_TSR_CRS)) )
            {
                /* if transmit aborted (NE2000_TSR_ABT) due to excessive collisions,   */
                /* if collisions is 0 it really is 16 (excessive collisions)           */
                if (error & NE2000_TSR_ABT)
                {
#                    if (DISPLAY_ERROR)
                    if (collisions == 0)
                    {
                        /*
                         * When collisions total 16, the P0_NCR
                         * will indicate 0, and the TSR_ABT is
                         * set.
                         */
                        collisions = 16;
                    }
#                    endif
#                    if (DISPLAY_ERROR)
                        DEBUG_ERROR("ne2000_interrupt - transmit error - collisions = ",
                            EBS_INT1, collisions, 0);
#                    endif

                    sc->stats.collision_errors++;
                }

                /* Update output errors counter.   */
                sc->stats.errors_out++;

                pi->xmit_status = ENETDOWN;
            }
            else        /* no transmit error */
            {
                /* check for out of window collsions   */
                if (INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TSR)) & NE2000_TSR_OWC)
                    sc->stats.owc_collision++;
                if (collisions == 1)
                    sc->stats.one_collision++;
                else if (collisions >= 1)
                    sc->stats.multiple_collisions++;
                if (INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TSR)) & NE2000_TSR_CRS)
                    sc->stats.tx_carrier_errors++;
            }
            /* signal IP layer or driver that send is done   */
            ks_invoke_output(pi, 1);
        }       /* pkt xmit or xmit error */

        /* ******                         */
        /* Handle receiver interrupts.    */
        if (isr & (NE2000_ISR_PRX | NE2000_ISR_RXE | NE2000_ISR_OVW))
        {
            /*
            * Overwrite warning.  In order to make sure that a
            * lockup of the local DMA hasn't occurred, we reset
            * and re-init the NIC.  The NSC manual suggests only a
            * partial reset/re-init is necessary - but some chips
            * seem to want more.  The DMA lockup has been seen
            * only with early rev chips - Methinks this bug was
            * fixed in later revs.  -DG
            */
            if (isr & NE2000_ISR_OVW)
            {
                sc->stats.rx_overwrite_errors++;
                sc->stats.rx_other_errors++;
                sc->stats.errors_in++;
#                if (DISPLAY_ERROR || DEBUG_RING)
                    DEBUG_ERROR("warning - receiver ring buffer overrun",
                        NOVAR, 0, 0);
#                endif

                /* Recover from ring buffer overflow.                    */
                /* disable sends and receives while recovery in progress */
                sc->do_recovery = ks_get_ticks();

                /* disable all interrupts   */
                OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_IMR), 0);

                /* save whether xmit is in progress   */
                sc->tpx = INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR)) & NE2000_CR_TXP;

                /* STOP command to NIC; also set to page 0   */
                OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_STP | NE2000_CR_PAGE_0);
                /* Timer task will do the rest   */
            }
            else
            {
                /* Process Receiver Error.  One or more of:   */
                /*    CRC error,                              */
                /*    frame alignment error                   */
                /*    FIFO overrun, or                        */
                /*    missed packet.                          */
                /*                                            */
                if (isr & NE2000_ISR_RXE)
                {
                    sc->stats.errors_in++;
                    sc->stats.rx_other_errors++;
#                    if (DISPLAY_ERROR)
                        DEBUG_ERROR("receive error = ", EBS_INT1,
                        INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RSR), 0);
#                    endif
                }

                /* Go get the packet(s).                        */
                /* - Doing this on an error is dubious          */
                /*   because there shouldn't be any data to get */
                /*   (we've configured the interface to not     */
                /*   accept packets with errors).               */
                /*                                              */
                /* process the receiver interrupt               */
                if (!sc->do_recovery)
                {
                    ne2000_read_ring_pkt(pi, sc, TRUE);
                }
            }
        }           /* end receive interrupts */

        /*
         * Return NIC CR to standard state: page 0, remote DMA
         * complete, start (toggling the TXP bit off, even if was just
         * set in the transmit routine, is *okay* - it is 'edge'
         * triggered from low to high).
         */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

        /*
         * If the Network Talley Counters overflow, read them to reset
         * them.  It appears that old 8390's won't clear the ISR flag
         * otherwise - resulting in an infinite loop.
         */
        if (isr & NE2000_ISR_CNT)
        {
            (void) INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CNTR0));
            (void) INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CNTR1));
            (void) INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CNTR2));
        }

        /* get isr for next loop   */
        isr = INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR));
    }       /* end of while(isr) */

    DRIVER_MASK_ISR_ON(sc->ia_irq);
}


/* ********************************************************************   */
/*
 * Retreive packet from shared memory and send to the next level up via
 * the input list.
 */
void ne2000_get_packet(PIFACE pi, PNE2000_SOFTC sc, word nic_buf, int len, RTIP_BOOLEAN rint)
{
DCU msg;
PFBYTE msgdata;
int tmp_amount;

    /* Allocate a packet to write the ethernet packet in.   */
    msg = os_alloc_packet_input(len, DRIVER_ALLOC);
    if (!msg)
    {
        DEBUG_ERROR("ne2000_get_packet() - out of DCUs", NOVAR, 0, 0);
        if (rint)
            sc->stats.packets_lost++;
        return;
    }

    /* write ethernet header to packet   */
    msgdata = DCUTODATA(msg);
    ne2000_pio_readmem(sc, nic_buf, msgdata, sizeof(struct _ether));
    nic_buf = (word)(nic_buf + sizeof(struct _ether));
    msgdata += sizeof(struct _ether);
    DCUTOPACKET(msg)->length = len;

    /* Pull packet off interface and put into packet.   */
    len -= sizeof(struct _ether);

    /* Handle wrap here   */
    if (nic_buf + len > sc->mem_end)
    {
        tmp_amount = sc->mem_end - nic_buf;
        /* Copy amount up to end of NIC memory.   */
        ne2000_pio_readmem(sc, nic_buf, msgdata, tmp_amount);
        len -= tmp_amount;
        nic_buf = sc->mem_ring;
        msgdata += tmp_amount;
    }
    ne2000_pio_readmem(sc, nic_buf, msgdata, len);

    if (rint)
    {
        /* Pull packet off interface and put into packet.   */
        sc->stats.packets_in++;
        sc->stats.bytes_in += len;
#if (RTIP_VERSION > 24)
        ks_invoke_input(pi,msg);
#else
        os_sndx_input_list(pi, msg);
        ks_invoke_input(pi);
#endif
    }
    else
        os_free_packet(msg);
}

/* ********************************************************************   */
/* Ethernet interface receiver interrupt.                                 */
void ne2000_read_ring_pkt(PIFACE pi, PNE2000_SOFTC sc, RTIP_BOOLEAN rint)
{
word boundary;
#if (!KS_LITTLE_ENDIAN)
word w;
#endif
int  len;
struct ne2000_ring packet_hdr;
word nic_packet_ptr;
#if (FIX_RING_PTRS)
RTIP_BOOLEAN ring_ptr_corrupt;
#endif

    /* Set NIC to page 1 registers to get 'current' pointer.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_1 | NE2000_CR_STA);

    /*
     * 'sc->next_packet' is the logical beginning of the ring-buffer - i.e.
     * it points to where new data has been buffered.  The 'CURR' (current)
     * register points to the logical end of the ring-buffer - i.e. it
     * points to where additional new data will be added.  We loop here
     * until the logical beginning equals the logical end (or in other
     * words, until the ring-buffer is empty).
     */
    while (sc->next_packet != INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_CURR)))
    {
        /* Get pointer to this buffer's header structure.   */
        nic_packet_ptr = (word)(sc->mem_ring +
                                (sc->next_packet - sc->rec_page_start) *
                                NE2000_PAGE_SIZE);

        /*
         * The byte count includes a 4 byte header that was added by
         * the NIC.
         */
        ne2000_pio_readmem(sc, nic_packet_ptr, (PFBYTE) &packet_hdr,
                           sizeof(packet_hdr));
#if (KS_LITTLE_ENDIAN)
        len = (int)packet_hdr.count;
#else
        /* on the interface, length is in INTEL byte order so on   */
        /* MOTOROLA system we swap bytes in the word               */
        w = (word)packet_hdr.count;
        w = (w>>8) | (w<<8);
        len = (int) w;
/*      len = (packet_hdr.count>>8) | (packet_hdr.count<<8);   */
#endif
#if (FIX_RING_PTRS)
        if (len > ETHER_MAX_LEN)
        {
            /*
             * Length is a wild value.  There's a good chance that
             * this was caused by the NIC being old and buggy.
             * The bug is that the length low byte is duplicated in
             * the high byte.  Try to recalculate the length based
             * on the pointer to the next packet.
             *
             * NOTE: sc->next_packet is pointing at the current
             * packet.
             */
            len &= NE2000_PAGE_SIZE - 1;
            if (packet_hdr.next_packet >= sc->next_packet)
            {
                len += (packet_hdr.next_packet - sc->next_packet) *
                       NE2000_PAGE_SIZE;
            }
            else
            {
                len += ((packet_hdr.next_packet - sc->rec_page_start) +
                       (sc->rec_page_stop - sc->next_packet)) * 
                        NE2000_PAGE_SIZE;
            }
        }

        /* check if ring pointers are corrupt   */
        ring_ptr_corrupt = TRUE;
        if (packet_hdr.next_packet >= sc->rec_page_start &&
            packet_hdr.next_packet < sc->rec_page_stop)
            ring_ptr_corrupt = FALSE;

        if (len > (sizeof(struct _ether)+CRC_LEN) &&
            len <= ETHER_MAX_LEN && !ring_ptr_corrupt)
        {
#endif /* FIX_RING_PTRS */
            /* Go get packet from shared memory, put it in a DCU and send it to
               input exchange. */
            /* Adjust the pointer to the data to point past the header and
               Subtract the size of the CRC from the length */
            if (len <= ETHER_MAX_LEN)
            {
                ne2000_get_packet(pi, sc,
                    (word)(nic_packet_ptr + sizeof(struct ne2000_ring)),
                    len - CRC_LEN, rint);
            }
#if (FIX_RING_PTRS)
        }
        else
        {
            if (rint)
            {
                if (!ring_ptr_corrupt)
                    sc->stats.rx_frame_errors++;
                else
                {
                    /* Really BAD.  The ring pointers are corrupted.   */
                    /* Just try to continue                            */
                    DEBUG_ERROR("NIC memory corrupt - invalid packet length ", EBS_INT1,  len, 0);
                    /* ne2000_reset(pi, sc);   */
                    sc->stats.rx_other_errors++;
                }
                sc->stats.errors_in++;
            }
            return;
        }
#endif /* FIX_RING_PTRS */
        /* Update next packet pointer.   */
        sc->next_packet = packet_hdr.next_packet;

        /*
         * Update NIC boundary pointer - being careful to keep it one
         * buffer behind (as recommended by NS databook).
         */
        boundary = (word) (sc->next_packet - 1);
        if (boundary < sc->rec_page_start)
            boundary = (word) (sc->rec_page_stop - 1);

        /* Set NIC to page 0 registers to update boundary register.   */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P1_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_BNRY), boundary);

        /*
         * Set NIC to page 1 registers before looping to top (prepare
         * to get 'CURR' current pointer).
         */
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_1 | NE2000_CR_STA);
    }
}

/* ********************************************************************   */
/* RING BUFFER OVERFLOW RECOVERY                                          */
/* ********************************************************************   */

/* perform recovery for ring buffer overflow as suggested by             */
/* National Semiconductor DP8390D/NS32490D NIC Network Interface         */
/* Controller (September 1992) Document                                  */
/* NOTE: the rest of the recovery definatly cannot be done during        */
/* the interrupt due to the delay within the recovery code, therefore,   */
/* it is broken off in this routine just in case recovery ever attempted */
/* at the interrupt level                                                */
void ne2000_ring_overflow_recovery2(void KS_FAR *vp)        /*__fn__*/
{
int resend;
PNE2000_SOFTC sc;

#if (CFG_NUM_NE2000 == 1)
    ARGSUSED_PVOID(vp);
    sc = (PNE2000_SOFTC) &ne2000softc[0];
#else
    sc = (PNE2000_SOFTC) &ne2000softc[(int)vp];
#endif

    if (!sc || !sc->do_recovery)
        return;

    /* we have to wait at least 1.6 miliseconds to do ring overflow
       here we assume that 2 ticks is at least 1.6 milliseconds to
       save space on calculating time elapsed
       */
    if (!sc->do_recovery || ( (ks_get_ticks() - sc->do_recovery) < 2))
    {
        ebs_start_timer(&(sc->ne2000_timer_info));
        return;
    }

#if (DEBUG_RING)
        DEBUG_ERROR("ring buffer recovery 2 start", NOVAR, 0, 0);
#endif


#if (DEBUG_RING)
    DEBUG_ERROR("ring buffer recovery 2 start", NOVAR, 0, 0);
#endif

    /* clear the remote byte count registers   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR0), 0);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR1), 0);

    /* set resend variable   */
    if (!sc->tpx)
        resend = 0;
    else
    {
        if ( INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR)) & (NE2000_ISR_PTX | NE2000_ISR_TXE) )
            resend = 0;
        else
            resend = 1;
    }

    /* set to mode 1 loopback   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TCR), NE2000_TCR_LB0);

    /* issue start command   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_STA);

    /* remove one or more packets from the receive buffer   */
    ne2000_read_ring_pkt(sc->iface, sc, FALSE);

    /* set to page 0 in case ne2000_read_ring_pkt changed page   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_PAGE_0);

    /* reset the overwrite warning   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR), NE2000_ISR_OVW);

    /* Take interface out of loopback.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_TCR), 0);

    /* if resend is 1, reissue xmit command   */
    if (resend)
        OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_TXP | NE2000_CR_STA);

    /* enable interrupts again   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_IMR), NORMAL_NE2000_INTS);

#if (DEBUG_RING)
    recovery_count++;
    DEBUG_ERROR("ring buffer recovery done", EBS_INT1, recovery_count, 0);
#endif

    /* do not clear flag until recovery done; xmit and receive are disable   */
    /* via this flag until the recovery is done                              */
    sc->do_recovery = 0;
}

/* ********************************************************************   */
/* STATISTICS routine.                                                    */
/* ********************************************************************   */

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

RTIP_BOOLEAN ne2000_statistics(PIFACE pi)                       /*__fn__*/
{
#if (INCLUDE_KEEP_STATS)
PETHER_STATS p;
PNE2000_SOFTC sc;
#endif

#if ( (CFG_NUM_NE2000 == 1) || !INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi);
#endif

#if (INCLUDE_KEEP_STATS)
    sc = iface_to_softc(pi);
    if (!sc)
        return(FALSE);

   p = (PETHER_STATS) &(sc->stats);
   UPDATE_SET_INFO(pi,interface_packets_in, p->packets_in)
   UPDATE_SET_INFO(pi,interface_packets_out, p->packets_out)
   UPDATE_SET_INFO(pi,interface_bytes_in, p->bytes_in)
   UPDATE_SET_INFO(pi,interface_bytes_out, p->bytes_out)
   UPDATE_SET_INFO(pi,interface_errors_in, p->errors_in)
   UPDATE_SET_INFO(pi,interface_errors_out, p->errors_out)
   UPDATE_SET_INFO(pi,interface_packets_lost, p->packets_lost)
#endif
   return(TRUE);
}

/* ********************************************************************   */
/* Supporting routines.                                                   */
/* ********************************************************************   */

/*
 * Given a NIC memory source address and a host memory destination address,
 * copy 'amount' from NIC to host using Programmed I/O.  The 'amount' is
 * rounded up to a word - okay as long as mbufs are word sized.
 * This routine is currently Novell-specific.
 */
void ne2000_pio_readmem(PNE2000_SOFTC sc, word src, PFBYTE dst, int amount)
{

    /* Select page 0 registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    /* Round up to a word.   */
    if (amount & 1)
        ++amount;

    /* Set up DMA byte count.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR0), amount);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR1), amount >> 8);

    /* Set up source address in NIC mem.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RSAR0), src);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RSAR1), src >> 8);

    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD0 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    if (sc->isa16bit)
        insw(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_DATA), (unsigned short KS_FAR *) dst, amount / 2);
    else
        insb(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_DATA), dst, amount);
}

/*
 * Write a DCU to the destination NIC memory address using programmed I/O
 */
RTIP_BOOLEAN ne2000_pio_write_pkt(PIFACE pi, PNE2000_SOFTC sc, PFCBYTE packet, int length, word dst)
{
int     maxwait; /* about 120us */
byte    savebyte[2];
RTIP_BOOLEAN ret_val=TRUE;

    /* Select page 0 registers.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD2 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    /* Reset remote DMA complete flag.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR), NE2000_ISR_RDC);

    /* Set up DMA byte count.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR0), length);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RBCR1), length >> 8);

    /* Set up destination address in NIC mem.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RSAR0), dst);
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_RSAR1), dst >> 8);

    /* Set remote DMA write.   */
    OUTBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_CR), NE2000_CR_RD1 | NE2000_CR_PAGE_0 | NE2000_CR_STA);

    /*
     * Transfer DCU to the NIC memory.
     * 16-bit cards require that data be transferred as words, and only
     * words, so that case requires some extra code to patch over
     * odd-length DCUs.
     */
    if (!sc->isa16bit)
    {
        /* NE1000s are easy.   */
        outsb(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_DATA), packet, length);
    }
    else
    {
        /* NE2000s are a bit trickier.   */
        /* Output contiguous words.      */
        if (length > 1)
        {
            outsw(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_DATA), (unsigned short KS_FAR *) packet, length >> 1);
            packet += length;
            length &= 1;
            if (length)
                packet--;
        }
        /* If odd number of bytes, output last byte as a word with a 0 appended   */
        if (length == 1)
        {
            savebyte[0] = *packet;
            savebyte[1] = 0;
            OUTWORD(REF_ASIC_ADDR(asic_addr_NE2000_NOVELL_DATA), *(word *)savebyte);
        }
    }

    /*
     * Wait for remote DMA complete.  This is necessary because on the
     * transmit side, data is handled internally by the NIC in bursts and
     * we can't start another remote DMA until this one completes.  Not
     * waiting causes really bad things to happen - like the NIC
     * irrecoverably jamming the ISA bus.
     */
#if (NEW_WAIT)
    maxwait = 300;      /* about 360usec */
#else
    maxwait = 1000;
#endif
    while (maxwait--)
    {
        if ( ((INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR)) & NE2000_ISR_RDC) == NE2000_ISR_RDC))
            break;
    }

    /* If pi is zero we are probing so don't reset   */
    if (!maxwait && pi)
    {
#if (NEW_WAIT)
        /* the while() above was not enough.  Cannot afford to tie up the   */
        /* CPU longer than .5 msec, so sleep 20 msecs longer now.           */
        ks_sleep((word)(1 + ks_ticks_p_sec()/50));

        ++sc->stats.tx_delayed;
#endif
#if (NEW_WAIT)
        if ((INBYTE_NE2000(REF_REG_ADDR(nic_addr_NE2000_P0_ISR)) & NE2000_ISR_RDC) != NE2000_ISR_RDC)
#endif
        {
            DEBUG_ERROR("remote transmit DMA failed to complete", NOVAR, 0, 0);
            sc->stats.errors_out++;
            sc->stats.tx_other_errors++;
            ne2000_reset(pi, sc);
            ret_val = FALSE;
        }
    }
    return(ret_val);

}

/* calc values for multicast   */
void ne2000_getmcaf(PFBYTE mclist, int lenmclist, PFBYTE af)
{
int bytesmclist;
byte c;
PFBYTE cp;
dword crc;
int i, len, offset;
byte bit;
int row, col;

    bytesmclist = lenmclist * ETH_ALEN;

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

    /* the driver can only handle 64 addresses; if there are more than   */
    /* 64 than accept all addresses; the IP layer will filter the        */
    /* packets                                                           */
    if (lenmclist > 64)
        af[0] = af[1] = af[2] = af[3] = af[4] = af[5] = af[6] = af[7] = 0xff;
    else
    {
        af[0] = af[1] = af[2] = af[3] = af[4] = af[5] = af[6] = af[7] = 0;
        for (offset = 0; offset < bytesmclist; offset += 6)
        {
            cp = mclist + offset;
            crc = 0xffffffffL;
            for (len = 6; --len >= 0;)
            {
                c = *cp++;
                for (i = 8; --i >= 0;)
                {
                    if (((crc & 0x80000000L) ? 1 : 0) ^ (c & 0x01))
                    {
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
}
#endif      /* DECLARING_DATA */
#endif      /* INCLUDE_NE2000 */


