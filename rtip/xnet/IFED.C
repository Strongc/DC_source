/*                   
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
 *  $Id: ifed.c,v 1.1 2003/07/02 19:09:12 sarah Exp $
 */

#undef  DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_ED)
#include "ifed.h"

RTIP_BOOLEAN ed_open(PIFACE pi);
void    ed_close(PIFACE pi);
int     ed_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN ed_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
void    ed_proc_interrupts(PIFACE pi);
RTIP_BOOLEAN ed_statistics(PIFACE  pi);
RTIP_BOOLEAN ed_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
ED_SOFTC KS_FAR softc[CFG_NUM_ED];
/* 
 * Interrupt conversion table for WD/SMC ASIC.
 * (IRQ* are defined in icu.h.)
 */

KS_GLOBAL_CONSTANT int KS_FAR ed_intr_mask[8] = 
{
    IRQ9,
    IRQ3,
    IRQ5,
    IRQ7,
    IRQ10,
    IRQ11,
    IRQ15,
    IRQ4
};

/* Interrupt conversion table for 585/790 Combo.   */
KS_GLOBAL_CONSTANT int KS_FAR ed_790_intr_mask[8] = 
{
    0,
    IRQ9,
    IRQ3,
    IRQ5,
    IRQ7,
    IRQ10,
    IRQ11,
    IRQ15
};

KS_GLOBAL_CONSTANT byte KS_FAR test_pattern[32] = "THIS is A memory TEST pattern";

EDEVTABLE KS_FAR ed_device = 
{
     ed_open, ed_close, ed_xmit, ed_xmit_done,
     NULLP_FUNC, ed_statistics, ed_setmcast, 
     NE2000_DEVICE, "NE2000", MINOR_0, ETHER_IFACE, 
     SNMP_DEVICE_INFO(CFG_OID_NE2000, CFG_SPEED_NE2000)
     CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
     CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
     IOADD(0x300), EN(0x0), EN(5)
};

#endif  /* DECLARING_DATA || BUILD_NEW_BINARY */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
/* Interrupt conversion table for WD/SMC ASIC.   */
KS_EXTERN_GLOBAL_CONSTANT int KS_FAR ed_intr_mask[8]; 

/* Interrupt conversion table for 585/790 Combo.   */
KS_EXTERN_GLOBAL_CONSTANT int KS_FAR ed_790_intr_mask[8];

extern ED_SOFTC KS_FAR softc[CFG_NUM_ED];
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR test_pattern[32];
extern EDEVTABLE KS_FAR ed_device;
#endif /* !BUILD_NEW_BINARY */

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define ED_TASK_INTERRUPT 0 /* process interrupt at task layer */
                            /* NOTE: if ever turn this on need to make   */
                            /*       sure interrupt task is started      */
                            /*       (see tc_interrupt_xxx in pollos.c,  */
                            /*        osport.c and tasks.c)              */
#define MASK_CONTROLLER   0 /* mask interrupt thru interrupt controller */
                            /* i.e. call ks_mask_isr_on and ks_mask_isr_off   */

#define DISPLAY_ERROR   0   /* display error during interrupt */
#define DEBUG_RING      0

#if (DEBUG_RING)
int recovery_count = 0;     /* count of ring buffer overflows */
#endif


/* ********************************************************************   */
#define ETHER_MAX_LEN   (ETHERSIZE+4)       /* includes header */
#define CRC_LEN         4

#define EROUND  ((sizeof(struct _ether) + 3) & ~3)
#define EOFF    (EROUND - sizeof(struct _ether))

/* ********************************************************************   */
void ed_getmcaf(PFBYTE mclist, int bytesmclist, PFBYTE af);
void ed_reset(PIFACE pi, PED_SOFTC sc);
void ed_interrupt(int minor_no);

#if (INCLUDE_SMC8XXX)
int     ed_probe_WD80x3(PIFACE pi, PED_SOFTC sc);
#endif
#if (INCLUDE_3C503)
int     ed_probe_3Com(PIFACE pi, PED_SOFTC sc);
#endif
#if (INCLUDE_ED_NE2000)
int     ed_probe_Novell(PIFACE pi, PED_SOFTC sc);
#endif
void    ed_stop(PED_SOFTC sc);

#define addr2w(X) (word)_FP_OFF(X)

void        ed_init(PIFACE pi, PED_SOFTC sc);
#if (INCLUDE_ED_NE2000)
void        ed_pio_readmem(PED_SOFTC sc, word src, PFBYTE dst, int amount);
void        ed_pio_writemem(PED_SOFTC sc, PFBYTE src, word dst, int len);
void        ed_pio_write_pkt(PIFACE pi, PED_SOFTC sc, PFBYTE packet, int length, word dst);
#endif
INLINE PFBYTE ed_ring_copy(PED_SOFTC sc, PFBYTE src, PFBYTE dst, int amount);
RTIP_BOOLEAN    ed_ring_to_dcu(PED_SOFTC sc, PFBYTE src, DCU msg, word total_len);
void        ring_overflow_recovery1(PED_SOFTC sc);
void        ring_overflow_recovery2(void KS_FAR *vp);
void        ring_overflow_recovery3(PED_SOFTC sc);
void        ed_proc_in_isr(PIFACE pi, byte isr);
INLINE void ed_get_packet(PIFACE pi, PED_SOFTC sc, PFBYTE buf, int len, 
                          RTIP_BOOLEAN rint);
static void ed_read_ring_pkt(PIFACE pi, PED_SOFTC sc, RTIP_BOOLEAN rint);
void ed_proc_interrupts(PIFACE pi);

/* ********************************************************************   */
/* MEMORY MANAGMENT                                                       */
/* ********************************************************************   */
#if (CFG_NUM_ED == 1) 
#define iface_to_softc(X) (PED_SOFTC) &softc[0]
#define off_to_softc(X)   (PED_SOFTC) &softc[0]

#else
PED_SOFTC iface_to_softc(PIFACE pi) 
{
int softc_off;
    
    softc_off = pi->minor_number;
    if (softc_off >= CFG_NUM_ED) 
    {
        DEBUG_ERROR("iface_to_softc() - pi->minor_number, CFG_NUM_ED =",
            EBS_INT2, pi->minor_number, CFG_NUM_ED);
        return((PED_SOFTC)0);
    }

    return((PED_SOFTC) &softc[softc_off]);
}

PED_SOFTC off_to_softc(int softc_off) 
{
    if (softc_off >= CFG_NUM_ED) 
    {
        DEBUG_ERROR("off_to_softc() - softc_off, CFG_NUM_ED =",
            EBS_INT2, softc_off, CFG_NUM_ED);
        return((PED_SOFTC)0);
    }

    return((PED_SOFTC) &softc[softc_off]);
}
#endif

/* ********************************************************************   */
/* Probing and Initialization routines.                                   */
/* ********************************************************************   */
/* set board values to either values set by application or default values */
/* specified by device table                                              */
void set_ed_vals(PIFACE pi, PED_SOFTC sc)
{
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
dword ltemp;
#endif
    /* set values based on global variable set by application if they are    */
    /* not set to their initial value otherwise                              */
    /* set them to default values from the device table                      */

    /* io base address   */
    sc->ia_iobase = pi->io_address;

#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
    /* shared memory address   */
    ltemp = (dword)pi->mem_address;
    ltemp = ltemp << 4;
    phys_to_virtual( &(sc->ia_maddr), ltemp);
#endif

    /* interrupt number   */
    sc->ia_irq = pi->irq_val; 
}

/* ********************************************************************   */
/*  Determine if the device is present.                                   */
RTIP_BOOLEAN edprobe(PIFACE pi, PED_SOFTC sc)
{
#if (INCLUDE_SMC8XXX)
    if (pi->pdev->device_id == SMC8XXX_DEVICE) 
    {
        /* WD settings   */
        sc->cf_flags = 0;       /* If 3com set to ED_FLAGS_DISABLE_TRANCEIVER */
                                /* to use AUI   */
        sc->ia_msize = 0;       /* if non zero override calulated value 3com/wd */

        set_ed_vals(pi, sc);
        if (ed_probe_WD80x3(pi, sc))
            return (TRUE);
    }
#endif

#if (INCLUDE_3C503)
    if (pi->pdev->device_id == EL3_DEVICE) 
    {
        /* 3COM settings   */
        sc->cf_flags = 0;       /* If 3com set to ED_FLAGS_DISABLE_TRANCEIVER */
                                /* to use AUI   */
        sc->ia_msize = 0;       /* if non zero override calulated value wd */

        set_ed_vals(pi, sc);
        if (ed_probe_3Com(pi, sc))
            return (TRUE);
    }
#endif

#if (INCLUDE_ED_NE2000)
    /* ******   */
    sc->cf_flags = 0;   /* If 3com set to ED_FLAGS_DISABLE_TRANCEIVER */
                        /* to use AUI   */
    sc->ia_msize = 0;   /* if non zero override calulated value 3com/wd */

    set_ed_vals(pi, sc);

#if (INCLUDE_PCMCIA)
    if ( (pi->pdev->device_id == NE2000_PCMCIA_DEVICE) &&
         card_is_ne2000(0, sc->ia_irq, sc->ia_iobase))  /* 1st parm is socket */
    {
        if (ed_probe_Novell(pi, sc))
            return (TRUE);
        pcmctrl_card_down(0);
    }
#endif

    if ( (pi->pdev->device_id == NE2000_DEVICE) &&
         ed_probe_Novell(pi, sc))
        return (TRUE);
#endif      /* INCLUDE_ED_NE2000 */

    /* all probes failed   */
    return (FALSE);
}

/*
 * Generic probe routine for testing for the existance of a DS8390.  Must be
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
int ed_probe_generic8390(PED_SOFTC sc)
{
    if ((INBYTE(sc->nic_addr + ED_P0_CR) &
         (ED_CR_RD2 | ED_CR_TXP | ED_CR_STA | ED_CR_STP)) != 
         (ED_CR_RD2 | ED_CR_STP))
        return (0);
    if ((INBYTE(sc->nic_addr + ED_P0_ISR) & ED_ISR_RST) != ED_ISR_RST)
        return (0);

    return (1);
}
    
#if (INCLUDE_SMC8XXX)
/* Probe and vendor-specific initialization routine for SMC/WD80x3 boards.   */
int ed_probe_WD80x3(PIFACE pi, PED_SOFTC sc)
{
int  i;
word j;
word memsize;
byte isa16bit;
int  iptr;
byte sum;

    sc->asic_addr = sc->ia_iobase;
    sc->nic_addr = (word)(sc->asic_addr + (word) ED_WD_NIC_OFFSET);
    sc->is790 = 0;

#ifdef TOSH_ETHER
    OUTBYTE(sc->asic_addr + ED_WD_MSR, ED_WD_MSR_POW);
    ks_sleep(2);    /* 10000 usec */
#endif

    /*
     * Attempt to do a checksum over the station address PROM.  If it
     * fails, it's probably not a SMC/WD board.  There is a problem with
     * this, though: some clone WD boards don't pass the checksum test.
     * Danpex boards for one.
     */
    for (sum = 0, j = 0; j < 8; ++j)
        sum = (byte) (sum + INBYTE((sc->asic_addr + (word) ED_WD_PROM + (word) j)));

    if (sum != ED_WD_ROM_CHECKSUM_TOTAL) 
    {
        /*
         * Checksum is invalid.  This often happens with cheap WD8003E
         * clones.  In this case, the checksum byte (the eighth byte)
         * seems to always be zero.
         */
        if (INBYTE(sc->asic_addr + ED_WD_CARD_ID) != ED_TYPE_WD8003E ||
            INBYTE(sc->asic_addr + ED_WD_PROM + 7) != 0)
            return (0);
    }

    /* Reset card to force it into a known state.   */
#ifdef TOSH_ETHER
    OUTBYTE(sc->asic_addr + ED_WD_MSR, ED_WD_MSR_RST | ED_WD_MSR_POW);
#else
    OUTBYTE(sc->asic_addr + ED_WD_MSR, ED_WD_MSR_RST);
#endif
    ks_sleep(2);    /* 100 usec */
    OUTBYTE(sc->asic_addr + ED_WD_MSR,
        INBYTE(sc->asic_addr + ED_WD_MSR) & ~ED_WD_MSR_RST);
    /* Wait in the case this card is reading it's EEROM.   */
    ks_sleep(2);    /* 5000 usec */

    sc->vendor = ED_VENDOR_WD_SMC;
    sc->type = INBYTE(sc->asic_addr + ED_WD_CARD_ID);

    /* Set initial values for width/size.   */
    memsize = 8192;
    isa16bit = 0;
    switch (sc->type) 
    {
    case ED_TYPE_WD8003S:
    case ED_TYPE_WD8003E:
    case ED_TYPE_WD8003EB:
    case ED_TYPE_WD8003W:
        break;

    case ED_TYPE_SMC8216C:
    case ED_TYPE_SMC8216T:
        sc->is790 = 1;
    case ED_TYPE_WD8013EBT:
    case ED_TYPE_WD8013W:
    case ED_TYPE_WD8013WC:
    case ED_TYPE_WD8013EBP:
    case ED_TYPE_WD8013EPC:
        memsize = 16384;
        isa16bit = 1;
        break;

    case ED_TYPE_WD8013EP:      /* also WD8003EP */
        if (INBYTE(sc->asic_addr + ED_WD_ICR) & ED_WD_ICR_16BIT) 
        {
            memsize = 16384;
            isa16bit = 1;
        }
        break;

#ifdef TOSH_ETHER
    case ED_TYPE_TOSHIBA1:
    case ED_TYPE_TOSHIBA4:
        memsize = 32768;
        isa16bit = 1;
        break;
#endif
    default:
        break;
    }

    /*
     * Make some adjustments to initial values depending on what is found
     * in the ICR.
     */
    if (isa16bit && (sc->type != ED_TYPE_WD8013EBT) &&
#ifdef TOSH_ETHER
        (sc->type != ED_TYPE_TOSHIBA1) && (sc->type != ED_TYPE_TOSHIBA4) &&
#endif
        ((INBYTE(sc->asic_addr + ED_WD_ICR) & ED_WD_ICR_16BIT) == 0)) 
    {
        isa16bit = 0;
        memsize = 8192;
    }

#ifdef ED_DEBUG
    DEBUG_ERROR("type,isa16bit = ", EBS_INT2, sc->type, isa16bit);
    DEBUG_ERROR("memsize,id_msize =", EBS_INT2, memsize, sc->ia_msize);
    for (i = 0; i < 8; i++)
    {
        DEBUG_ERROR("x -> x", i, EBS_INT2, INBYTE(sc->asic_addr + i));
    }
#endif
    /* Allow the user to override the autoconfiguration.   */
    if (sc->ia_msize)
        memsize = (word)sc->ia_msize;
    /*
     * (Note that if the user specifies both of the following flags that
     * '8-bit' mode intentionally has precedence.)
     */
    if (sc->cf_flags & ED_FLAGS_FORCE_16BIT_MODE)
        isa16bit = 1;
    if (sc->cf_flags & ED_FLAGS_FORCE_8BIT_MODE)
        isa16bit = 0;

    /*
     * Check 83C584 interrupt configuration register if this board has one
     * XXX - We could also check the IO address register.  But why
     * bother... if we get past this, it *has* to be correct.
     */
    if (sc->is790) 
    {
        int x;

        /* Assemble together the encoded interrupt number.   */
        OUTBYTE(sc->ia_iobase + ED_WD790_HWR,
            INBYTE(sc->ia_iobase + ED_WD790_HWR) | ED_WD790_HWR_SWH);
        x = INBYTE(sc->ia_iobase + ED_WD790_GCR);
        iptr = ((x & ED_WD790_GCR_IR2) >> 4) |
               ((x & (ED_WD790_GCR_IR1|ED_WD790_GCR_IR0)) >> 2);
        OUTBYTE(sc->ia_iobase + ED_WD790_HWR,
            INBYTE(sc->ia_iobase + ED_WD790_HWR) & ~ED_WD790_HWR_SWH);

        /*
         * Translate it using translation table, and check for
         * correctness.
         */
        if (ed_790_intr_mask[iptr] != sc->ia_irq) 
        {
            DEBUG_ERROR("irq != board configured irq:irq, board irq = ", EBS_INT2, 
                sc->ia_irq - 1, ed_790_intr_mask[iptr] - 1);
            return (0);
        }

        /* Enable the interrupt.   */
        OUTBYTE(sc->ia_iobase + ED_WD790_ICR,
            INBYTE(sc->ia_iobase + ED_WD790_ICR) | ED_WD790_ICR_EIL);
    } 
    else if (sc->type & ED_WD_SOFTCONFIG) 
    {
        /* Assemble together the encoded interrupt number.   */
        iptr = (INBYTE(sc->ia_iobase + ED_WD_ICR) & ED_WD_ICR_IR2) |
               ((INBYTE(sc->ia_iobase + ED_WD_IRR) &
                (ED_WD_IRR_IR0 | ED_WD_IRR_IR1)) >> 5);

        /*
         * Translate it using translation table, and check for
         * correctness.
         */
        if (ed_intr_mask[iptr] != sc->ia_irq) 
        {
            DEBUG_ERROR("irq != board configured irq:irq, board irq = ", EBS_INT2, 
                sc->ia_irq - 1, ed_intr_mask[iptr] - 1);
            return (0);
        }

        /* Enable the interrupt.   */
        OUTBYTE(sc->ia_iobase + ED_WD_IRR,
            INBYTE(sc->ia_iobase + ED_WD_IRR) | ED_WD_IRR_IEN);
    }

    sc->isa16bit = isa16bit;
    sc->mem_shared = TRUE;
    sc->ia_msize = memsize;
    sc->mem_start = sc->ia_maddr;

    sc->tx_page_start = ED_WD_PAGE_OFFSET;
    sc->rec_page_start = (word) (sc->tx_page_start + (word) ED_TXBUF_SIZE);
    sc->rec_page_stop = (word) (sc->tx_page_start + memsize / (word) ED_PAGE_SIZE);
    sc->mem_ring = sc->mem_start + sc->rec_page_start * ED_PAGE_SIZE;
    sc->mem_size = (dword)memsize;
    sc->mem_end = (PFBYTE)((dword)sc->mem_start + (dword)memsize);

    /* Get station (ethernet) address from on-board ROM.   */
    for (i = 0; i < ETH_ALEN; ++i)
        pi->addr.my_hw_addr[i] = INBYTE(sc->asic_addr + ED_WD_PROM + i);

    /* Set upper address bits and 8/16 bit access to shared memory.   */
    if (isa16bit) 
    {
        if (sc->is790) 
        {
            sc->wd_laar_proto = INBYTE(sc->asic_addr + ED_WD_LAAR) &
                                ~ED_WD_LAAR_M16EN;
        } 
        else 
        {
            sc->wd_laar_proto = ED_WD_LAAR_L16EN | 
                                ((int)(kvtop(sc->mem_start) >> 19) & 
                                 ED_WD_LAAR_ADDRHI);
        }
        OUTBYTE(sc->asic_addr + ED_WD_LAAR, 
             sc->wd_laar_proto | ED_WD_LAAR_M16EN);
    } 
    else  
    {
        if (((sc->type & ED_WD_SOFTCONFIG) ||
#ifdef TOSH_ETHER
            (sc->type == ED_TYPE_TOSHIBA1) ||
            (sc->type == ED_TYPE_TOSHIBA4) ||
#endif
            (sc->type == ED_TYPE_WD8013EBT)) && (!sc->is790)) 
        {
            sc->wd_laar_proto = ((int)(kvtop(sc->mem_start) >> 19) & 
                                 ED_WD_LAAR_ADDRHI);
            OUTBYTE(sc->asic_addr + ED_WD_LAAR, sc->wd_laar_proto);
        }
    }

    /*
     * Set address and enable interface shared memory.
     */
    if (!sc->is790) 
    {
#ifdef TOSH_ETHER
        OUTBYTE(sc->asic_addr + ED_WD_MSR + 1, (int)(kvotp(sc->mem_start) >> 8) & 0xe0) | 4);
        OUTBYTE(sc->asic_addr + ED_WD_MSR + 2, (int)(kvtop(sc->mem_start) >> 16) & 0x0f);
        sc->wd_msr_proto = ED_WD_MSR_POW;
#else
        sc->wd_msr_proto = (byte)((kvtop(sc->mem_start) >> 13) & ED_WD_MSR_ADDR);
#endif
        sc->cr_proto = ED_CR_RD2;
    } 
    else 
    {
        OUTBYTE(sc->asic_addr + 0x04, INBYTE(sc->asic_addr + 0x04) | 0x80);
        OUTBYTE(sc->asic_addr + 0x0b,
             (int)((kvtop(sc->mem_start) >> 13) & 0x0f) |
             (int)((kvtop(sc->mem_start) >> 11) & 0x40) |
             (INBYTE(sc->asic_addr + 0x0b) & 0xb0));
        OUTBYTE(sc->asic_addr + 0x04,
             INBYTE(sc->asic_addr + 0x04) & ~0x80);
        sc->wd_msr_proto = 0x00;
        sc->cr_proto = 0;
    }
    OUTBYTE(sc->asic_addr + ED_WD_MSR,
         sc->wd_msr_proto | ED_WD_MSR_MENB);

    (void) INBYTE(0x84);
    (void) INBYTE(0x84);
    DEBUG_LOG("mem_start, memsize", LEVEL_3, DINT2, sc->mem_start, memsize);

    /* Now zero memory and verify that it is clear.   */
    tc_memset(sc->mem_start, 0, memsize);

    for (j = 0; j < memsize; ++j)
    {
        if (sc->mem_start[j]) 
        {                  
            DEBUG_ERROR("failed to clear shared memory at ", DINT1,
                kvtop(sc->mem_start + j), 0 );

            /* Disable 16 bit access to shared memory.   */
            OUTBYTE(sc->asic_addr + ED_WD_MSR, sc->wd_msr_proto);
            if (isa16bit)
                OUTBYTE(sc->asic_addr + ED_WD_LAAR, sc->wd_laar_proto);
            (void) INBYTE(0x84);
            (void) INBYTE(0x84);
            return (0);
        }
    }   

    /*
     * Disable 16bit access to shared memory - we leave it disabled
     * so that 1) machines reboot properly when the board is set 16
     * 16 bit mode and there are conflicting 8bit devices/ROMS in
     * the same 128k address space as this boards shared memory,
     * and 2) so that other 8 bit devices with shared memory can be
     * used in this 128k region, too.
     */
    OUTBYTE(sc->asic_addr + ED_WD_MSR, sc->wd_msr_proto);
    if (isa16bit)
        OUTBYTE(sc->asic_addr + ED_WD_LAAR, sc->wd_laar_proto);
    (void) INBYTE(0x84);
    (void) INBYTE(0x84);

    sc->ia_iosize = ED_WD_IO_PORTS;
    return (1);
}
#endif

#if (INCLUDE_3C503)
/* Probe and vendor-specific initialization routine for 3Com 3c503 boards.   */
int ed_probe_3Com(PIFACE pi, PED_SOFTC sc)
{
word i;
word memsize;
byte isa16bit;


    sc->asic_addr = (word)(sc->ia_iobase + (word) ED_3COM_ASIC_OFFSET);
    sc->nic_addr = (word)(sc->ia_iobase + ED_3COM_NIC_OFFSET);

    /*
     * Verify that the kernel configured I/O address matches the board
     * configured address.
     *
     * This is really only useful to see if something that looks like the
     * board is there; after all, we are already talking to it at that
     * address.
     */
    switch (INBYTE(sc->asic_addr + ED_3COM_BCFR)) 
    {
    case ED_3COM_BCFR_300:
        if (sc->ia_iobase != 0x300)
            return (0);
        break;
    case ED_3COM_BCFR_310:
        if (sc->ia_iobase != 0x310)
            return (0);
        break;
    case ED_3COM_BCFR_330:
        if (sc->ia_iobase != 0x330)
            return (0);
        break;
    case ED_3COM_BCFR_350:
        if (sc->ia_iobase != 0x350)
            return (0);
        break;
    case ED_3COM_BCFR_250:
        if (sc->ia_iobase != 0x250)
            return (0);
        break;
    case ED_3COM_BCFR_280:
        if (sc->ia_iobase != 0x280)
            return (0);
        break;
    case ED_3COM_BCFR_2A0:
        if (sc->ia_iobase != 0x2a0)
            return (0);
        break;
    case ED_3COM_BCFR_2E0:
        if (sc->ia_iobase != 0x2e0)
            return (0);
        break;
    default:
        return (0);
    }

    /*
     * Verify that the kernel shared memory address matches the board
     * configured address.
     */
    switch (INBYTE(sc->asic_addr + ED_3COM_PCFR)) 
    {
    case ED_3COM_PCFR_DC000:
        if (kvtop(sc->ia_maddr) != 0xdc000ul)
            return (0);
        break;
    case ED_3COM_PCFR_D8000:
        if (kvtop(sc->ia_maddr) != 0xd8000ul)
            return (0);
        break;
    case ED_3COM_PCFR_CC000:
        if (kvtop(sc->ia_maddr) != 0xcc000ul)
            return (0);
        break;
    case ED_3COM_PCFR_C8000:
        if (kvtop(sc->ia_maddr) != 0xc8000ul)
            return (0);
        break;
    default:
        return (0);
    }

    /*
     * Reset NIC and ASIC.  Enable on-board transceiver throughout reset
     * sequence because it'll lock up if the cable isn't connected if we
     * don't.
     */
    OUTBYTE(sc->asic_addr + ED_3COM_CR, ED_3COM_CR_RST | ED_3COM_CR_XSEL);

    /* Wait for a while, then un-reset it.   */
    ks_sleep(2);    /* 50 usec */

    /*
     * The 3Com ASIC defaults to rather strange settings for the CR after a
     * reset - it's important to set it again after the following _outp
     * (this is done when we map the PROM below).
     */
    OUTBYTE(sc->asic_addr + ED_3COM_CR, ED_3COM_CR_XSEL);

    /* Wait a bit for the NIC to recover from the reset.   */
    ks_sleep(2);    /* 5000 usec */

    sc->vendor = ED_VENDOR_3COM;
    sc->mem_shared = TRUE;
    sc->cr_proto = ED_CR_RD2;

    /*
     * Hmmm...a 16bit 3Com board has 16k of memory, but only an 8k window
     * to it.
     */
    memsize = 8192;

    /*
     * Get station address from on-board ROM.
     *
     * First, map ethernet address PROM over the top of where the NIC
     * registers normally appear.
     */
    OUTBYTE(sc->asic_addr + ED_3COM_CR, ED_3COM_CR_EALO | ED_3COM_CR_XSEL);

    for (i = 0; i < ETH_ALEN; ++i)
        pi->addr.my_hw_addr[i] = INBYTE(sc->nic_addr + i);

    /*
     * Unmap PROM - select NIC registers.  The proper setting of the
     * tranceiver is set in ed_init so that the attach code is given a
     * chance to set the default based on a compile-time config option.
     */
    OUTBYTE(sc->asic_addr + ED_3COM_CR, ED_3COM_CR_XSEL);

    /* Determine if this is an 8bit or 16bit board.   */

    /* Select page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_PAGE_0 | ED_CR_STP);

    /*
     * Attempt to clear WTS bit.  If it doesn't clear, then this is a
     * 16-bit board.
     */
    OUTBYTE(sc->nic_addr + ED_P0_DCR, 0);

    /* Select page 2 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_PAGE_2 | ED_CR_STP);

    /* The 3c503 forces the WTS bit to a one if this is a 16bit board.   */
    if (INBYTE(sc->nic_addr + ED_P2_DCR) & ED_DCR_WTS)
        isa16bit = 1;
    else
        isa16bit = 0;

    /* Select page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P2_CR, ED_CR_RD2 | ED_CR_PAGE_0 | ED_CR_STP);

    sc->mem_start = (PFBYTE)sc->ia_maddr;
    sc->mem_size = memsize;
    sc->mem_end = sc->mem_start + memsize;

    if (isa16bit) 
    {
        sc->tx_page_start = ED_3COM_TX_PAGE_OFFSET_16BIT;
        sc->rec_page_start = ED_3COM_RX_PAGE_OFFSET_16BIT;
        sc->rec_page_stop = (word) ((memsize / (word) ED_PAGE_SIZE) + 
                            (word) ED_3COM_RX_PAGE_OFFSET_16BIT);
        sc->mem_ring = sc->mem_start;
    } 
    else 
    {
        sc->tx_page_start = ED_3COM_TX_PAGE_OFFSET_8BIT;
        sc->rec_page_start =
            ED_TXBUF_SIZE + ED_3COM_TX_PAGE_OFFSET_8BIT;
        sc->rec_page_stop =
        (word) (memsize / (word) ED_PAGE_SIZE + (word) ED_3COM_TX_PAGE_OFFSET_8BIT);
        sc->mem_ring = sc->mem_start + (word) (ED_PAGE_SIZE * ED_TXBUF_SIZE);
    }

    sc->isa16bit = isa16bit;

    /*
     * Initialize GA page start/stop registers.  Probably only needed if
     * doing DMA, but what the Hell.
     */
    OUTBYTE(sc->asic_addr + ED_3COM_PSTR, sc->rec_page_start);
    OUTBYTE(sc->asic_addr + ED_3COM_PSPR, sc->rec_page_stop);

    /* Set IRQ.  3c503 only allows a choice of irq 3-5 or 9.   */
    switch (sc->ia_irq) 
    {
    case IRQ9:
        OUTBYTE(sc->asic_addr + ED_3COM_IDCFR, ED_3COM_IDCFR_IRQ2);
        break;
    case IRQ3:
        OUTBYTE(sc->asic_addr + ED_3COM_IDCFR, ED_3COM_IDCFR_IRQ3);
        break;
    case IRQ4:
        OUTBYTE(sc->asic_addr + ED_3COM_IDCFR, ED_3COM_IDCFR_IRQ4);
        break;
    case IRQ5:
        OUTBYTE(sc->asic_addr + ED_3COM_IDCFR, ED_3COM_IDCFR_IRQ5);
        break;
    default:
        DEBUG_ERROR("invalid irq must be 3-5 or 9 for 3c503 but it = ", EBS_INT1, 
            sc->ia_irq - 1, 0);
        return (0);
    }

    /*
     * Initialize GA configuration register.  Set bank and enable shared
     * mem.
     */
    OUTBYTE(sc->asic_addr + ED_3COM_GACFR,
        ED_3COM_GACFR_RSEL | ED_3COM_GACFR_MBS0);

    /*
     * Initialize "Vector Pointer" registers. These gawd-awful things are
     * compared to 20 bits of the address on ISA, and if they match, the
     * shared memory is disabled. We set them to 0xffff0...allegedly the
     * reset vector.
     */
    OUTBYTE(sc->asic_addr + ED_3COM_VPTR2, 0xff);
    OUTBYTE(sc->asic_addr + ED_3COM_VPTR1, 0xff);
    OUTBYTE(sc->asic_addr + ED_3COM_VPTR0, 0x00);

    /* Zero memory and verify that it is clear.   */
    tc_memset((PFBYTE)(sc->mem_start), 0, memsize);

    for (i = 0; i < memsize; ++i)
        if (sc->mem_start[i]) 
        {
            DEBUG_ERROR("failed to clear shared memory at ", DINT1,
                kvtop(sc->mem_start + i), 0);
            return (0);
        }

    sc->ia_msize = memsize;
    sc->ia_iosize = ED_3COM_IO_PORTS;
    return (1);
}
#endif

#if (INCLUDE_ED_NE2000)
/*
 * Probe and vendor-specific initialization routine for NE1000/2000 boards.
 */
int ed_probe_Novell(PIFACE pi, PED_SOFTC sc)
{
word memsize, n;
byte romdata[16], tmp;
byte test_buffer[32];
dword ltemp;

    sc->asic_addr = (IOADDRESS)(sc->ia_iobase + (word) ED_NOVELL_ASIC_OFFSET);
    sc->nic_addr = (IOADDRESS)(sc->ia_iobase + ED_NOVELL_NIC_OFFSET);

    /* XXX - do Novell-specific probe here   */

    /* Reset the board.   */
#ifdef GWETHER
    OUTBYTE(sc->asic_addr + ED_NOVELL_RESET, 0);
    ks_sleep(2);    /* 120 usec */
#endif /* GWETHER */
    tmp = INBYTE(sc->asic_addr + ED_NOVELL_RESET);

    /*
     * I don't know if this is necessary; probably cruft leftover from
     * Clarkson packet driver code. Doesn't do a thing on the boards I've
     * tested. -DG [note that a _outp(0x84, 0) seems to work here, and is
     * non-invasive...but some boards don't seem to reset and I don't have
     * complete documentation on what the 'right' thing to do is...so we do
     * the invasive thing for now.  Yuck.]
     */
    OUTBYTE(sc->asic_addr + ED_NOVELL_RESET, tmp);
    ks_sleep(2);    /* 5000 usec */

    /*
     * This is needed because some NE clones apparently don't reset the NIC
     * properly (or the NIC chip doesn't reset fully on power-up)
     * XXX - this makes the probe invasive! ...Done against my better
     * judgement.  -DLG
     */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_PAGE_0 | ED_CR_STP);

    ks_sleep(2);    /* 5000 usec */

    /* Make sure that we really have an 8390 based board.   */
    if (!ed_probe_generic8390(sc))
        return (0);

    sc->vendor = ED_VENDOR_NOVELL;
    sc->mem_shared = FALSE;
    sc->cr_proto = ED_CR_RD2;
    sc->ia_msize = 0;

    /*
     * Test the ability to read and write to the NIC memory.  This has the
     * side affect of determining if this is an NE1000 or an NE2000.
     */

    /*
     * This prevents packets from being stored in the NIC memory when the
     * readmem routine turns on the start bit in the CR.
     */
    OUTBYTE(sc->nic_addr + ED_P0_RCR, ED_RCR_MON);

    /* Temporarily initialize DCR for byte operations.   */
    OUTBYTE(sc->nic_addr + ED_P0_DCR, ED_DCR_FT1 | ED_DCR_LS);

    OUTBYTE(sc->nic_addr + ED_P0_PSTART, 8192 / ED_PAGE_SIZE);
    OUTBYTE(sc->nic_addr + ED_P0_PSTOP, 16384 / ED_PAGE_SIZE);

    sc->isa16bit = 0;

    /*
     * Write a test pattern in byte mode.  If this fails, then there
     * probably isn't any memory at 8k - which likely means that the board
     * is an NE2000.
     */
    ed_pio_writemem(sc, test_pattern, 8192, sizeof(test_pattern));
    ed_pio_readmem(sc, 8192, (PFBYTE)test_buffer, sizeof(test_buffer));

    if (!tc_comparen(test_pattern, test_buffer, sizeof(test_buffer))) 
    {
        /* not an NE1000 - try NE2000   */

#if (KS_LITTLE_ENDIAN)  /*DM: 9-27-02: added conditional and big endian case */
        OUTBYTE(sc->nic_addr + ED_P0_DCR, ED_DCR_WTS | ED_DCR_FT1 | ED_DCR_LS);
#else
        OUTBYTE(sc->nic_addr + ED_P0_DCR, ED_DCR_WTS | ED_DCR_FT1 | ED_DCR_LS | ED_DCR_BOS);
#endif
        OUTBYTE(sc->nic_addr + ED_P0_PSTART, 16384 / ED_PAGE_SIZE);
        OUTBYTE(sc->nic_addr + ED_P0_PSTOP, (32768l / ED_PAGE_SIZE) );

        sc->isa16bit = 1;

        /*
         * Write a test pattern in word mode.  If this also fails, then
         * we don't know what this board is.
         */
        ed_pio_writemem(sc, test_pattern, 16384, sizeof(test_pattern));
        ed_pio_readmem(sc, 16384, (PFBYTE)test_buffer, sizeof(test_buffer));

        if (!tc_comparen(test_pattern, test_buffer, sizeof(test_buffer)))
            return (0); /* not an NE2000 either */

        sc->type = ED_TYPE_NE2000;
    } 
    else 
    {
        sc->type = ED_TYPE_NE1000;
    }
    
    /* 8k of memory plus an additional 8k if 16-bit.   */
    memsize = (word) (8192 + (word)sc->isa16bit * 8192);

    /* NIC memory doesn't start at zero on an NE board.   */
    /* The start address is tied to the bus width.        */
    ltemp  = (dword)(8192 + sc->isa16bit * 8192);
    sc->mem_start = (PFBYTE)(ltemp);
    sc->tx_page_start = (word) (memsize / ED_PAGE_SIZE);

#ifdef GWETHER
    {
        int x, i, mstart = 0;
        char pbuf0[ED_PAGE_SIZE], pbuf[ED_PAGE_SIZE], tbuf[ED_PAGE_SIZE];

        for (i = 0; i < ED_PAGE_SIZE; i++)
            pbuf0[i] = 0;

        /* Search for the start of RAM.   */
        for (x = 1; x < 256; x++) 
        {
            ed_pio_writemem(sc, pbuf0, x * ED_PAGE_SIZE, ED_PAGE_SIZE);
            ed_pio_readmem(sc, x * ED_PAGE_SIZE, tbuf, ED_PAGE_SIZE);
            if (tc_comparen(pbuf0, tbuf, ED_PAGE_SIZE)) 
            {
                for (i = 0; i < ED_PAGE_SIZE; i++)
                    pbuf[i] = 255 - x;
                ed_pio_writemem(sc, pbuf, x * ED_PAGE_SIZE, ED_PAGE_SIZE);
                ed_pio_readmem(sc, x * ED_PAGE_SIZE, tbuf, ED_PAGE_SIZE);
                if (tc_comparen(pbuf, tbuf, ED_PAGE_SIZE)) 
                {
                    mstart = x * ED_PAGE_SIZE;
                    memsize = ED_PAGE_SIZE;
                    break;
                }
            }
        }

        if (mstart == 0) 
        {
            DEBUG_ERROR("cannot find start of RAM", NOVAR, 0, 0);
            return (0);
        }

        /* Search for the end of RAM.   */
        for (x = (mstart / ED_PAGE_SIZE) + 1; x < 256; x++) 
        {
            ed_pio_writemem(sc, pbuf0, x * ED_PAGE_SIZE, ED_PAGE_SIZE);
            ed_pio_readmem(sc, x * ED_PAGE_SIZE, tbuf, ED_PAGE_SIZE);
            if (tc_comparen(pbuf0, tbuf, ED_PAGE_SIZE)) 
            {
                for (i = 0; i < ED_PAGE_SIZE; i++)
                    pbuf[i] = 255 - x;
                ed_pio_writemem(sc, pbuf, x * ED_PAGE_SIZE, ED_PAGE_SIZE);
                ed_pio_readmem(sc, x * ED_PAGE_SIZE, tbuf, ED_PAGE_SIZE);
                if (tc_comparen(pbuf, tbuf, ED_PAGE_SIZE))
                    memsize += ED_PAGE_SIZE;
                else
                    break;
            } else
                break;
        }

        DEBUG_ERROR("RAM start, size", DINT2, mstart, (dword)memsize);

        sc->mem_start = (PFBYTE)mstart;
        sc->tx_page_start = mstart / ED_PAGE_SIZE;
    }
#endif /* GWETHER */

    sc->mem_size = memsize;
    sc->mem_end = sc->mem_start + memsize;

    sc->rec_page_start = (word) (sc->tx_page_start + ED_TXBUF_SIZE);
    sc->rec_page_stop = (word) (sc->tx_page_start + memsize / ED_PAGE_SIZE);

    sc->mem_ring = sc->mem_start + ED_PAGE_SIZE * ED_TXBUF_SIZE;

    ed_pio_readmem(sc, 0, romdata, 16);
    for (n = 0; n < ETH_ALEN; n++)
        pi->addr.my_hw_addr[n] = romdata[n*(sc->isa16bit+1)];

    /* Clear any pending interrupts that might have occurred above.   */
    OUTBYTE(sc->nic_addr + ED_P0_ISR, 0xff);

    sc->ia_iosize = ED_NOVELL_IO_PORTS;
    return (1);
}
#endif
 
/* ********************************************************************   */
/* OPEN and CLOSE routines.                                               */
/* ********************************************************************   */

void ed_close(PIFACE pi)                     /*__fn__*/
{
PED_SOFTC sc;

#if (CFG_NUM_ED == 1)
    ARGSUSED_PVOID(pi);
#endif
    sc = iface_to_softc(pi);
    if (!sc)
        return;
    ebs_stop_timer(&(sc->ed_timer_info));
    ed_stop(sc);
}

/* ********************************************************************   */
/* Install interface into kernel networking data structures.              */
RTIP_BOOLEAN ed_open(PIFACE pi)
{
PED_SOFTC sc;

    sc = iface_to_softc(pi);
    if (!sc)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    tc_memset((PFBYTE) sc, 0, sizeof(*sc));

    /* set up pointers between iface and ed_softc structures   */
    sc->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(sc->stats);

    /* determine which device and initialize    */
    if (!edprobe(pi, sc))
    {
        DEBUG_ERROR("edprobe failed", NOVAR, 0, 0);
        set_errno(EPROBEFAIL);
        return(FALSE);
    }

    /* Set interface to stopped condition (reset).   */
    ed_stop(sc);

    /* Initialize ifnet structure.   */
    sc->if_flags = 0; 
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
    /*
     * Set default state for LINK0 flag (used to disable the tranceiver
     * for AUI operation), based on compile-time config option.
     */
    switch (sc->vendor) 
    {
    case ED_VENDOR_3COM:
        if (sc->cf_flags & ED_FLAGS_DISABLE_TRANCEIVER)
            sc->if_flags |= IFF_LINK0;
        break;
    case ED_VENDOR_WD_SMC:
        if ((sc->type & ED_WD_SOFTCONFIG) == 0)
            break;
        if ((INBYTE(sc->asic_addr + ED_WD_IRR) & ED_WD_IRR_OUT2) == 0)
            sc->if_flags |= IFF_LINK0;
        break;
    }

    switch (sc->vendor) 
    {
    case ED_VENDOR_WD_SMC:
        if ((sc->type & ED_WD_SOFTCONFIG) == 0)
            break;
    case ED_VENDOR_3COM:
        if (sc->if_flags & IFF_LINK0)
        {
            DEBUG_LOG(" aui", LEVEL_3, NOVAR, 0, 0);
        }
        else
        {
            DEBUG_LOG(" bnc", LEVEL_3, NOVAR, 0, 0);
        }
        break;
    }
#endif

    /* start a timer to check for ring overflow recovery    */
    /* NOTE: if timer already started, will not start again */
    sc->ed_timer_info.func = ring_overflow_recovery2; /* routine to execute if timeout */
    sc->ed_timer_info.arg = (void KS_FAR *)(dword)pi->minor_number; 
    ebs_set_timer(&sc->ed_timer_info, 1, FALSE);
    ebs_start_timer(&sc->ed_timer_info);

    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi, (RTIPINTFN_POINTER)ed_interrupt, (RTIPINTFN_POINTER) 0, pi->minor_number);
           
    ed_reset(pi, sc);

    return(TRUE);
}
 
/* Reset interface.   */
void ed_reset(PIFACE pi, PED_SOFTC sc)
{
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupt flags */

    sp = ks_splx();     /* Disable interrupts */

    ed_stop(sc);
    ed_init(pi, sc);

    ks_spl(sp);
}
 
/* Take interface offline.   */
void ed_stop(PED_SOFTC sc)
{
int n = 5000;
 
#if (CFG_NUM_ED == 1)
    ARGSUSED_PVOID(sc);
#endif

    /* Stop everything on the interface, and select page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STP);

    /*
     * Wait for interface to enter stopped state, but limit # of checks to
     * 'n' (about 5ms).  It shouldn't even take 5us on modern DS8390's, but
     * just in case it's an old one.
     */
    while (((INBYTE(sc->nic_addr + ED_P0_ISR) & ED_ISR_RST) == 0) && --n);
}


/* Initialize device.    */
void ed_init(PIFACE pi, PED_SOFTC sc)
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
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STP);

    if (sc->isa16bit) 
    {
#if (KS_LITTLE_ENDIAN)  /*DM: 9-27-02: added conditional and big endian case */
        /*
         * Set FIFO threshold to 8, No auto-init Remote DMA, byte
         * order=80x86 (little endian), word-wide DMA xfers,
         */
        OUTBYTE(sc->nic_addr + ED_P0_DCR, ED_DCR_FT1 | ED_DCR_WTS | ED_DCR_LS);
#else
        /*
         * Set FIFO threshold to 8, No auto-init Remote DMA, byte
         * order=68000 (big endian), word-wide DMA xfers,
         */
        OUTBYTE(sc->nic_addr + ED_P0_DCR, ED_DCR_FT1 | ED_DCR_WTS | ED_DCR_LS | ED_DCR_BOS);
#endif
    } 
    else 
    {
        /* Same as above, but byte-wide DMA xfers.   */
        OUTBYTE(sc->nic_addr + ED_P0_DCR, ED_DCR_FT1 | ED_DCR_LS);
    }

    /* Clear remote byte count registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_RBCR0, 0);
    OUTBYTE(sc->nic_addr + ED_P0_RBCR1, 0);

    /* Tell RCR to do nothing for now.   */
    OUTBYTE(sc->nic_addr + ED_P0_RCR, ED_RCR_MON);

    /* Place NIC in internal loopback mode.   */
    OUTBYTE(sc->nic_addr + ED_P0_TCR, ED_TCR_LB0);

    /* Initialize transmit/receive (ring-buffer) page start.   */
    OUTBYTE(sc->nic_addr + ED_P0_TPSR, sc->tx_page_start);
    OUTBYTE(sc->nic_addr + ED_P0_PSTART, sc->rec_page_start);

    /* Set lower bits of byte addressable framing to 0.   */
    if (sc->is790)
        OUTBYTE(sc->nic_addr + 0x09, 0);

    /* Initialize receiver (ring-buffer) page stop and boundary.   */
    OUTBYTE(sc->nic_addr + ED_P0_PSTOP, sc->rec_page_stop);
    OUTBYTE(sc->nic_addr + ED_P0_BNRY, sc->rec_page_start);

    /*
     * Clear all interrupts.  A '1' in each bit position clears the
     * corresponding flag.
     */
    OUTBYTE(sc->nic_addr + ED_P0_ISR, 0xff);

    /*
     * Enable the following interrupts: receive/transmit complete,
     * receive/transmit error, and Receiver OverWrite.
     *
     * Counter overflow and Remote DMA complete are *not* enabled.
     */
    OUTBYTE(sc->nic_addr + ED_P0_IMR, NORMAL_ED_INTS);

    /* Program command register for page 1.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_1 | ED_CR_STP);

    /* Copy out our station (ethernet) address.   */
    for (i = 0; i < ETH_ALEN; ++i)
        OUTBYTE(sc->nic_addr + ED_P1_PAR0 + i, pi->addr.my_hw_addr[i]);

    /* Set current page pointer to next_packet (initialized above).   */
    OUTBYTE(sc->nic_addr + ED_P1_CURR, sc->next_packet);


    /* Set multicast filter on chip.            */
    /* If none needed lenmclist will be zero    */
    ed_getmcaf((PFBYTE) pi->mcast.mclist, pi->mcast.lenmclist, 
                mcaf);
    for (i = 0; i < 8; i++)
        OUTBYTE(sc->nic_addr + ED_P1_MAR0 + i, mcaf[i]);


    /* ====================                    */
    /* Program command register for page 0.    */
    OUTBYTE(sc->nic_addr + ED_P1_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STP);

    /* set broadcast mode                                    */
    /* NOTE: promiscous mode and multicast are not supported */
/*  i = ED_RCR_AB;   */

    /* Accept multicasts and broadcasts   */
    i = ED_RCR_AB | ED_RCR_AM;
    OUTBYTE(sc->nic_addr + ED_P0_RCR, i);

    /* Take interface out of loopback.   */
    OUTBYTE(sc->nic_addr + ED_P0_TCR, 0);
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
    /*
     * If this is a 3Com board, the tranceiver must be software enabled
     * (there is no settable hardware default).
     */
    switch (sc->vendor) 
    {
        byte x;
    case ED_VENDOR_3COM:
        if (sc->if_flags & IFF_LINK0)
            OUTBYTE(sc->asic_addr + ED_3COM_CR, 0);
        else
            OUTBYTE(sc->asic_addr + ED_3COM_CR, ED_3COM_CR_XSEL);
        break;
    case ED_VENDOR_WD_SMC:
        if ((sc->type & ED_WD_SOFTCONFIG) == 0)
            break;
        x = INBYTE(sc->asic_addr + ED_WD_IRR);
        if (sc->if_flags & IFF_LINK0)
            x &= ~ED_WD_IRR_OUT2;
        else
            x |= ED_WD_IRR_OUT2;
        OUTBYTE(sc->asic_addr + ED_WD_IRR, x);
        break;
    }
#endif
    /* Fire up the interface.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);

    ks_spl(sp);
}
 
/* ********************************************************************   */
/* MULTICAST                                                              */
/* ********************************************************************   */
/* Edsetmcast() - 
   Takes an interface structures a contiguous array
   of bytes containing N IP multicast addresses and n, the number 
   of addresses (not number of bytes). 
   Copies the bytes to the driver structures multicast table and
   calls reset to set the multicast table in the board.
*/

RTIP_BOOLEAN ed_setmcast(PIFACE pi)     /* __fn__ */
{
PED_SOFTC sc;

    sc = iface_to_softc(pi);
    if (!sc)
        return(FALSE);

    /* Call reset to load the multicast table   */
    ed_reset(pi, sc);
    return(TRUE);
}

/* ********************************************************************   */
/* XMIT routines.                                                         */
/* ********************************************************************   */

/* ********************************************************************   */
RTIP_BOOLEAN ed_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PED_SOFTC sc;

#if (CFG_NUM_ED == 1) 
    ARGSUSED_PVOID(pi);
#endif

    sc = iface_to_softc(pi);
    if (!sc)
        return(FALSE);

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

/* ********************************************************************   */
/* This routine actually starts the transmission on the interface.        */
INLINE void ed_start_xmit(PED_SOFTC sc, int len)
{
    /* Set NIC for page 0 register access.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);

    /* Set TX buffer start page.   */
    OUTBYTE(sc->nic_addr + ED_P0_TPSR, sc->tx_page_start);

    /* Set TX length.   */
    OUTBYTE(sc->nic_addr + ED_P0_TBCR0, len);
    OUTBYTE(sc->nic_addr + ED_P0_TBCR1, len >> 8);

    /* Set page 0, remote DMA complete, transmit packet, and *start*.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_TXP | ED_CR_STA);
}

/*
 * Start output on interface.
 * We make two assumptions here:
 *  1) that the current priority is set to splimp _before_ this code
 *     is called *and* is returned to the appropriate priority after
 *     return
 */
void ed_start(PIFACE pi, PED_SOFTC sc, PFBYTE packet, int length)
{
PFBYTE buffer;

    /* buffer points to open buffer - only 1 slot used.   */
    buffer = sc->mem_start;

#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
    if (sc->mem_shared) 
    {
        /* Special case setup for 16 bit boards...   */
        switch (sc->vendor) 
        {
        /*
        * For 16bit 3Com boards (which have 16k of memory), we
         * have the xmit buffers in a different page of memory
         * ('page 0') - so change pages.
         */
        case ED_VENDOR_3COM:
            if (sc->isa16bit)
                OUTBYTE(sc->asic_addr + ED_3COM_GACFR,
                    ED_3COM_GACFR_RSEL);
            break;
        /*
         * Enable 16bit access to shared memory on WD/SMC
         * boards.
         */
        case ED_VENDOR_WD_SMC:
            if (sc->isa16bit)
                OUTBYTE(sc->asic_addr + ED_WD_LAAR,
                    sc->wd_laar_proto | ED_WD_LAAR_M16EN);
            OUTBYTE(sc->asic_addr + ED_WD_MSR,
                sc->wd_msr_proto | ED_WD_MSR_MENB);
            (void) INBYTE(0x84);
            (void) INBYTE(0x84);
            break;
        }

        /* copy data to next open buffer slot   */
        tc_movebytes(buffer, packet, length);
/*      buffer += length;  Remember this in future but compiler complains   */

        /* Restore previous shared memory access.   */
        switch (sc->vendor) 
        {
        case ED_VENDOR_3COM:
            if (sc->isa16bit)
            {
                OUTBYTE(sc->asic_addr + ED_3COM_GACFR,
                        ED_3COM_GACFR_RSEL | ED_3COM_GACFR_MBS0);
            }
            break;
        case ED_VENDOR_WD_SMC:
            OUTBYTE(sc->asic_addr + ED_WD_MSR,
                sc->wd_msr_proto);
            if (sc->isa16bit)
                OUTBYTE(sc->asic_addr + ED_WD_LAAR,
                    sc->wd_laar_proto);
            (void) INBYTE(0x84);
            (void) INBYTE(0x84);
            break;
        }
    } 
#if (INCLUDE_ED_NE2000)
    else
#endif
#endif
#if (INCLUDE_ED_NE2000)
        ed_pio_write_pkt(pi, sc, packet, length, addr2w(buffer));
#endif      
#if (!INCLUDE_ED_NE2000)
    ARGSUSED_PVOID(pi);
#endif
    ed_start_xmit(sc, length);
}


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
/*                                                                           */
/* Non packet drivers should behave the same way.                            */
/*                                                                           */

int ed_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
PED_SOFTC sc;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupt flags */
int length;

    sc = iface_to_softc(pi);
    if (!sc)
        return(ENUMDEVICE); 
    if (sc->do_recovery)
        return(ENETDOWN);   

    length = DCUTOPACKET(msg)->length;

    /* make sure packet is within legal size   */
    if (length < ETHER_MIN_LEN)   
        length = ETHER_MIN_LEN;   

    if (length > ETHERSIZE)
    {
        DEBUG_ERROR("edxmit - length is too large", EBS_INT1, DCUTODATA(msg), length);
        DEBUG_ERROR("edxmit - pkt = ", PKT, DCUTODATA(msg), ETHERSIZE);
        length = ETHERSIZE;       /* what a terriable hack! */
    }

    /* Send requires interrupts disabled. So the ISR does not change
       the register back out from under us (particularly the dma
       pointer) */
    sp = ks_splx();     /* Disable interrupts */

    /* Set NIC to page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);
    /* mask off all isr's   */
    OUTBYTE(sc->nic_addr + ED_P0_IMR, 0);
    ks_spl(sp);

    ed_start(pi, sc, DCUTODATA(msg), length);

    sp = ks_splx();     /* Disable interrupts */
    /* Set NIC to page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);
    /* Turn ints back on    */
    OUTBYTE(sc->nic_addr + ED_P0_IMR, NORMAL_ED_INTS);
    ks_spl(sp);

    return(0);
}


/* ********************************************************************   */
/* INTERRUPT routines.                                                    */
/* ********************************************************************   */
 
/* handle interrupts - this is the interrupt routine   */

void ed_interrupt(int minor_no)   /*__fn__*/
{
PED_SOFTC sc;
PIFACE pi;

#if (CFG_NUM_ED == 1)
    ARGSUSED_INT(minor_no);
#endif

    sc = off_to_softc(minor_no);
    if (!sc)
        return;

    pi = sc->iface;
    if (!pi)
        return;

#if (ED_TASK_INTERRUPT)
#if (!MASK_CONTROLLER)
    /* Set NIC to page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);

    /* mask off all isr's; the interrupts will be     */
    /* enabled after they are processed at task layer */
    OUTBYTE(sc->nic_addr + ED_P0_IMR, 0);
#else
    /* turn off the interrupt thru 8259 interrupt controller   */
    ks_mask_isr_off(sc->ia_irq);
#endif

    /* wake up task to process the interrupt; (task will call   */
    /* ed_proc_interrupts)                                      */
    ks_invoke_interrupt(pi);
#else
    ks_enable();  
    ed_proc_interrupts(pi);
#endif
}

/* ********************************************************************   */
/* handle interrupts - called at task level                               */
void ed_proc_interrupts(PIFACE pi)    /*__fn__*/
{
PED_SOFTC sc;
byte isr;

    sc = off_to_softc(pi->minor_number);
    if (!sc)
        return;

    /* Set NIC to page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);

    /* get interrupt status register   */
    isr = INBYTE(sc->nic_addr + ED_P0_ISR);

    /* Loop until there are no more new interrupts.   */
    while (isr)
    {
        /*
         * Reset all the bits in the interrupt status register that we 
         * are 'acknowledging' by writing a '1' to each bit position 
         * that was set. (Writing a '1' *clears* the bit.)
         */
        OUTBYTE(sc->nic_addr + ED_P0_ISR, isr);

        /* ******   */
        /*
         * Handle transmitter interrupts.  Handle these first because
         * the receiver will reset the board under some conditions.
         */
        if (isr & (ED_ISR_PTX | ED_ISR_TXE)) /* pkt xmit or xmit error */
        {
            int collisions = INBYTE(sc->nic_addr + ED_P0_NCR) & 0x0f;

            /*
             * Check for transmit error.  If a TX completed with an
             * error, we end up throwing the packet away.  Really
             * the only error that is possible is excessive
             * collisions, and in this case it is best to allow the
             * automatic mechanisms of TCP to backoff the flow.  Of
             * course, with UDP we're screwed, but this is expected
             * when a network is heavily loaded.
             */
            (void) INBYTE(sc->nic_addr + ED_P0_TSR);
            if (isr & ED_ISR_TXE) 
            {
                /* if transmit aborted (ED_TSR_ABT) due to excessive collisions,   */
                /* if collisions is 0 it really is 16 (excessive collisions)       */
                if ((INBYTE(sc->nic_addr + ED_P0_TSR) & ED_TSR_ABT) &&
                    (collisions == 0)) 
                {
#                    if (DISPLAY_ERROR)
                        /*
                         * When collisions total 16, the P0_NCR
                         * will indicate 0, and the TSR_ABT is
                         * set.
                         */
                        collisions = 16;
#                    endif
                }
#                if (DISPLAY_ERROR)
                    DEBUG_ERROR("ed_interrupt - transmit error - collisions = ", 
                        EBS_INT1, collisions, 0);
#                endif

                sc->stats.collision_errors++;

                /* Update output errors counter.   */
                sc->stats.errors_out++;
            } 
            else        /* no transmit error */
            {
                /* check for out of window collsions   */
                if (INBYTE(sc->nic_addr + ED_P0_TSR) & ED_TSR_OWC)
                    sc->stats.owc_collision++;
                if (collisions == 1)
                    sc->stats.one_collision++;
                else if (collisions >= 1)
                    sc->stats.multiple_collisions++;
                if (INBYTE(sc->nic_addr + ED_P0_TSR) & ED_TSR_CRS) 
                    sc->stats.tx_carrier_errors++;
            }

            /* signal driver that send is done   */
#            if (ED_TASK_INTERRUPT)
                OS_SET_IP_SIGNAL(PI);
#            else
                ks_invoke_output(pi, 1);
#            endif
        }       /* xmit or xmit error */

        /* ******                         */
        /* Handle receiver interrupts.    */
        if (isr & (ED_ISR_PRX | ED_ISR_RXE | ED_ISR_OVW)) 
        {
            ed_proc_in_isr(pi, isr);
        }           /* end receive interrupts */

        /*
         * Return NIC CR to standard state: page 0, remote DMA
         * complete, start (toggling the TXP bit off, even if was just
         * set in the transmit routine, is *okay* - it is 'edge'
         * triggered from low to high).
         */
        OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);

        /*
         * If the Network Talley Counters overflow, read them to reset
         * them.  It appears that old 8390's won't clear the ISR flag
         * otherwise - resulting in an infinite loop.
         */
        if (isr & ED_ISR_CNT) 
        {
            (void) INBYTE(sc->nic_addr + ED_P0_CNTR0);
            (void) INBYTE(sc->nic_addr + ED_P0_CNTR1);
            (void) INBYTE(sc->nic_addr + ED_P0_CNTR2);
        }

        /* get isr for next loop   */
        isr = INBYTE(sc->nic_addr + ED_P0_ISR);
    }       /* end of while(isr) */
#if (ED_TASK_INTERRUPT)
#if (!MASK_CONTROLLER)
        /* enable all interrupts again now that we are done   */
        OUTBYTE(sc->nic_addr + ED_P0_IMR, NORMAL_ED_INTS);
#else
        /* turn on the interrupt thru 8259 interrupt controller   */
        ks_mask_isr_on(sc->ia_irq);
#endif
#endif
}

/* ********************************************************************   */
/* handle receive interrupts - called at task level                       */
void ed_proc_in_isr(PIFACE pi, byte isr)        /*__fn__*/
{           
PED_SOFTC sc;

    sc = off_to_softc(pi->minor_number);
    if (!sc)
        return;

    /*
    * Overwrite warning.  In order to make sure that a
    * lockup of the local DMA hasn't occurred, we reset
    * and re-init the NIC.  The NSC manual suggests only a
    * partial reset/re-init is necessary - but some chips
    * seem to want more.  The DMA lockup has been seen
    * only with early rev chips - Methinks this bug was
    * fixed in later revs.  -DG
    */
    if (isr & ED_ISR_OVW) 
    {
        sc->stats.rx_overwrite_errors++;
        sc->stats.rx_other_errors++;
        sc->stats.errors_in++;
#        if (DISPLAY_ERROR || DEBUG_RING)
            DEBUG_ERROR("warning - receiver ring buffer overrun", 
                NOVAR, 0, 0);
#        endif

        /* Recover from ring buffer overflow.   */
        ring_overflow_recovery1(sc);
    } 
    else 
    {
        /* Process Receiver Error.  One or more of:    */
        /*    CRC error,                               */
        /*    frame alignment error                    */
        /*    FIFO overrun, or                         */
        /*    missed packet.                           */
        /*                                             */
        if (isr & ED_ISR_RXE) 
        {
            sc->stats.errors_in++;
            sc->stats.rx_other_errors++;
#            if (DISPLAY_ERROR)
                DEBUG_ERROR("receive error = ", EBS_INT1,  
                        INBYTE(sc->nic_addr + ED_P0_RSR), 0);
#            endif
        }

        /* Go get the packet(s).                        */
        /* - Doing this on an error is dubious          */
        /*   because there shouldn't be any data to get */
        /*   (we've configured the interface to not     */
        /*   accept packets with errors).               */
        /*                                              */
        /*   Enable 16bit access to shared memory first */
        /*   on WD/SMC boards.                          */
        /*                                              */
#if (INCLUDE_SMC8XXX)
        if (sc->vendor == ED_VENDOR_WD_SMC) 
        {
            if (sc->isa16bit)
                OUTBYTE(sc->asic_addr + ED_WD_LAAR,
                        sc->wd_laar_proto | ED_WD_LAAR_M16EN);
            OUTBYTE(sc->asic_addr + ED_WD_MSR,
                    sc->wd_msr_proto | ED_WD_MSR_MENB);
            (void) INBYTE(0x84);
            (void) INBYTE(0x84);
        }
#endif
        /* process the receiver interrupt   */
        if (!sc->do_recovery)
            ed_read_ring_pkt(pi, sc, TRUE);

#if (INCLUDE_SMC8XXX)
        /* Disable 16-bit access.   */
        if (sc->vendor == ED_VENDOR_WD_SMC) 
        {
            OUTBYTE(sc->asic_addr + ED_WD_MSR, sc->wd_msr_proto);
            if (sc->isa16bit)
                OUTBYTE(sc->asic_addr + ED_WD_LAAR, sc->wd_laar_proto);
            (void) INBYTE(0x84);
            (void) INBYTE(0x84);
        }
#endif
    }
}
    
/* ********************************************************************   */
/*
 * Given a source and destination address, copy 'amount' of a packet from the
 * ring buffer into a linear destination buffer.  Takes into account ring-wrap.
 * sc = ed info (softc)
 * src = pointer in ed ring buffer
 * dst = pointer to last mbuf in mbuf chain to copy to
 * amount = amount of data to copy
 */
INLINE PFBYTE ed_ring_copy(PED_SOFTC sc, PFBYTE src, PFBYTE dst, int amount)
{
int tmp_amount;

    /* Does copy wrap to lower addr in ring buffer?   */
    if (src + amount > sc->mem_end) 
    {
        tmp_amount = (int)(sc->mem_end - src);
        /* Copy amount up to end of NIC memory.   */
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
        if (sc->mem_shared)
        {
            tc_movebytes(dst, src, tmp_amount);
        }
#if (INCLUDE_ED_NE2000)
        else
#endif
#endif
#if (INCLUDE_ED_NE2000)
        {
            ed_pio_readmem(sc, addr2w(src), dst, tmp_amount);
        }
#endif
        amount -= tmp_amount;
        src = sc->mem_ring;
        dst += tmp_amount;
    }
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
    if (sc->mem_shared)
    {
        tc_movebytes(dst, src, amount);
    }
#if (INCLUDE_ED_NE2000)
    else
#endif
#endif
#if (INCLUDE_ED_NE2000)
        ed_pio_readmem(sc, addr2w(src), dst, amount);
#endif
    return (src + amount);
}


/* ********************************************************************   */
/*
 * Retreive packet from shared memory and send to the next level up via
 * the input list. 
 */
INLINE void ed_get_packet(PIFACE pi, PED_SOFTC sc, PFBYTE buf, int len, 
                          RTIP_BOOLEAN rint)
{
DCU msg;
PFBYTE msgdata;

    /* Allocate a packet to write the ethernet packet in.   */
    msg = os_alloc_packet_input(len, DRIVER_ALLOC);
    if (!msg)    
    {
        DEBUG_ERROR("ed_get_packet() - out of DCUs", NOVAR, 0, 0);
        if (rint)
            sc->stats.packets_lost++;
        return;
    }

    /* write ethernet header to packet   */
    msgdata = DCUTODATA(msg);
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
    if (sc->mem_shared)
        tc_movebytes(msgdata, buf, sizeof(struct _ether));
#if (INCLUDE_ED_NE2000)
    else
#endif
#endif
#if (INCLUDE_ED_NE2000)
        ed_pio_readmem(sc, addr2w(buf), msgdata, sizeof(struct _ether));
#endif
    buf += sizeof(struct _ether);
    msgdata += sizeof(struct _ether);

    DCUTOPACKET(msg)->length = len; 

    /* Pull packet off interface and put into packet.   */
    len -= (word) (sizeof(struct _ether));
    ed_ring_copy(sc, buf, msgdata, len); 

    if (rint)
    {
        /* Pull packet off interface and put into packet.   */
        sc->stats.packets_in++;
        sc->stats.bytes_in += len;  
#        if (ED_TASK_INTERRUPT)
            OS_SNDX_IP_EXCHG(pi, msg);
#        else
#if (RTIP_VERSION > 24)
            ks_invoke_input(pi,msg);   
#else
            os_sndx_input_list(pi, msg);
            ks_invoke_input(pi);   
#endif
#        endif
    }
    else
        os_free_packet(msg);
}

/* ********************************************************************   */
/* Ethernet interface receiver interrupt.                                 */
static void ed_read_ring_pkt(PIFACE pi, PED_SOFTC sc, RTIP_BOOLEAN rint)
{
word boundary;
#if (!KS_LITTLE_ENDIAN)
word w;
#endif
int  len;
struct ed_ring packet_hdr; 
PFBYTE packet_ptr;
RTIP_BOOLEAN ring_ptr_corrupt;

    /* Set NIC to page 1 registers to get 'current' pointer.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_1 | ED_CR_STA);

    /*
     * 'sc->next_packet' is the logical beginning of the ring-buffer - i.e.
     * it points to where new data has been buffered.  The 'CURR' (current)
     * register points to the logical end of the ring-buffer - i.e. it
     * points to where additional new data will be added.  We loop here
     * until the logical beginning equals the logical end (or in other
     * words, until the ring-buffer is empty).
     */
    while (sc->next_packet != INBYTE(sc->nic_addr + ED_P1_CURR)) 
    {
        /* Get pointer to this buffer's header structure.   */
        packet_ptr = sc->mem_ring + (sc->next_packet - sc->rec_page_start) * 
                     ED_PAGE_SIZE;

        /*
         * The byte count includes a 4 byte header that was added by
         * the NIC.
         */
#if (INCLUDE_SMC8XXX || INCLUDE_3C503)
        if (sc->mem_shared)
            packet_hdr = *((struct ed_ring KS_FAR *)packet_ptr);
#if (INCLUDE_ED_NE2000)
        else
#endif
#endif
#if (INCLUDE_ED_NE2000)
            ed_pio_readmem(sc, addr2w(packet_ptr), (PFBYTE) &packet_hdr, 
                           sizeof(packet_hdr));
#endif
#if (KS_LITTLE_ENDIAN)
        len = (int)packet_hdr.count;
#else
        /* on the interface, length is in INTEL byte order so on      */
        /* MOTOROLA system we swap bytes in the word                  */
        w = (word)packet_hdr.count;
        w = (w>>8) | (w<<8);
        len = (int) w;
#endif
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
            len &= ED_PAGE_SIZE - 1;
            if (packet_hdr.next_packet >= sc->next_packet) 
            {
                len += (packet_hdr.next_packet - sc->next_packet) * 
                       ED_PAGE_SIZE;
            } 
            else 
            {
                len += ( (packet_hdr.next_packet - sc->rec_page_start) +
                         (sc->rec_page_stop - sc->next_packet) ) * ED_PAGE_SIZE;
            }
        }

        /* check if ring pointers are corrupt   */
        ring_ptr_corrupt = TRUE;
        if (packet_hdr.next_packet >= sc->rec_page_start &&
            packet_hdr.next_packet < sc->rec_page_stop) 
            ring_ptr_corrupt = FALSE;

        if (len <= ETHER_MAX_LEN && !ring_ptr_corrupt)
        {
            /* Go get packet from shared memory, put it in a DCU and send it to 
               input exchange. */
            /* Adjust the pointer to the data to point past the header and
               Subtract the size of the CRC from the length */
            ed_get_packet(pi, sc, packet_ptr + sizeof(struct ed_ring),
                          len - CRC_LEN, rint);
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
                    /* ed_reset(pi, sc);   */
                    sc->stats.rx_other_errors++;
                }
                sc->stats.errors_in++;
            }
            return;
        }

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
        OUTBYTE(sc->nic_addr + ED_P1_CR, sc->cr_proto | ED_CR_PAGE_0 | ED_CR_STA);
        OUTBYTE(sc->nic_addr + ED_P0_BNRY, boundary);

        /*
         * Set NIC to page 1 registers before looping to top (prepare
         * to get 'CURR' current pointer).
         */
        OUTBYTE(sc->nic_addr + ED_P0_CR, sc->cr_proto | ED_CR_PAGE_1 | ED_CR_STA);
    }
}


/* ********************************************************************   */
/* RING BUFFER OVERFLOW RECOVERY                                          */
/* ********************************************************************   */

/* perform recovery for ring buffer overflow as suggested by     */
/* National Semiconductor DP8390D/NS32490D NIC Network Interface */
/* Controller (September 1992) Document                          */
void ring_overflow_recovery1(PED_SOFTC sc)
{
#if (DEBUG_RING)
    DEBUG_ERROR("ring buffer recovery start", NOVAR, 0, 0);
#endif

    /* disable sends and receives while recovery in progress   */
    sc->do_recovery = TRUE;

    /* disable all interrupts   */
    OUTBYTE(sc->nic_addr + ED_P0_IMR, 0);

    /* save whether xmit is in progress   */
    sc->tpx = INBYTE(sc->nic_addr + ED_P0_CR) & ED_CR_TXP;

    /* STOP command to NIC; also set to page 0   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_STP | ED_CR_PAGE_0);

#    if (ED_TASK_INTERRUPT)
        /* finish the recovery   */
        ring_overflow_recovery3(sc);
#    endif
}

void ring_overflow_recovery2(void KS_FAR *vp)   /*__fn__*/
{
#if (!ED_TASK_INTERRUPT)
PED_SOFTC sc;

    sc = (PED_SOFTC) &softc[(int)(dword)vp];

    if (!sc || !sc->do_recovery)
        return;

#if (DEBUG_RING)
        DEBUG_ERROR("ring buffer recovery 2 start", NOVAR, 0, 0);
#endif
        ring_overflow_recovery3(sc);
#endif
}

/* NOTE: the rest of the recovery definatly cannot be done during        */
/* the interrupt due to the delay within the recovery code, therefore,   */
/* it is broken off in this routine just in case recovery ever attempted */
/* at the interrupt level                                                */
void ring_overflow_recovery3(PED_SOFTC sc)      /*__fn__*/
{
int resend;
PIFACE pi;

    pi = sc->iface;

#if (DEBUG_RING)
    DEBUG_ERROR("ring buffer recovery 2 start", NOVAR, 0, 0);
#endif

    /* wait for at least 1.6 ms    */
    ks_sleep(2);            /* 1.6/ks_msec_p_tick is always < 1 but */
                            /* need to sleep 2 since sleeping 1 is   */
                            /* not reliable                          */

    /* clear the remote byte count registers   */
    OUTBYTE(sc->nic_addr + ED_P0_RBCR0, 0);
    OUTBYTE(sc->nic_addr + ED_P0_RBCR1, 0);

    /* set resend variable   */
    if (!sc->tpx)
        resend = 0;
    else
    {
        if ( INBYTE(sc->nic_addr + ED_P0_ISR) & (ED_ISR_PTX | ED_ISR_TXE) )
            resend = 0;     
        else
            resend = 1;
    }

    /* set to mode 1 loopback   */
    OUTBYTE(sc->nic_addr + ED_P0_TCR, ED_TCR_LB0);

    /* issue start command   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_STA);

    /* remove one or more packets from the receive buffer   */
    ed_read_ring_pkt(pi, sc, FALSE);

    /* set to page 0 in case ed_read_ring_pkt changed page   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_PAGE_0);

    /* reset the overwrite warning   */
    OUTBYTE(sc->nic_addr + ED_P0_ISR, ED_ISR_OVW);

    /* Take interface out of loopback.    */
    OUTBYTE(sc->nic_addr + ED_P0_TCR, 0);

    /* if resend is 1, reissue xmit command   */
    if (resend)
        OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_TXP | ED_CR_STA);

    /* enable interrupts again   */
    OUTBYTE(sc->nic_addr + ED_P0_IMR, NORMAL_ED_INTS);

#if (DEBUG_RING)
    recovery_count++;
    DEBUG_ERROR("ring buffer recovery done", EBS_INT1, recovery_count, 0);
#endif

    /* do not clear flag until recovery done; xmit and receive are disable   */
    /* via this flag until the recovery is done                              */
    sc->do_recovery = FALSE;
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

RTIP_BOOLEAN ed_statistics(PIFACE pi)                       /*__fn__*/
{
#if (INCLUDE_KEEP_STATS)
PETHER_STATS p;
PED_SOFTC sc;
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
#if (INCLUDE_ED_NE2000)
void ed_pio_readmem(PED_SOFTC sc, word src, PFBYTE dst, int amount)
{

    /* Select page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_PAGE_0 | ED_CR_STA);

    /* Round up to a word.   */
    if (amount & 1)
        ++amount;

    /* Set up DMA byte count.   */
    OUTBYTE(sc->nic_addr + ED_P0_RBCR0, amount);
    OUTBYTE(sc->nic_addr + ED_P0_RBCR1, amount >> 8);

    /* Set up source address in NIC mem.   */
    OUTBYTE(sc->nic_addr + ED_P0_RSAR0, src);
    OUTBYTE(sc->nic_addr + ED_P0_RSAR1, src >> 8);

    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD0 | ED_CR_PAGE_0 | ED_CR_STA);

    if (sc->isa16bit)
        insw( (word) (sc->asic_addr + ED_NOVELL_DATA), (PFWORD) dst, amount / 2);
    else
        insb( (word) (sc->asic_addr + ED_NOVELL_DATA), dst, amount);
}

/*
 * Stripped down routine for writing a linear buffer to NIC memory.  Only used
 * in the probe routine to test the memory.  'len' must be even.
 */
void ed_pio_writemem(PED_SOFTC sc, PFBYTE src, word dst, int len)
{
int maxwait = 100; /* about 120us */

    /* Select page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_PAGE_0 | ED_CR_STA);

    /* Reset remote DMA complete flag.   */
    OUTBYTE(sc->nic_addr + ED_P0_ISR, ED_ISR_RDC);

    /* Set up DMA byte count.   */
    OUTBYTE(sc->nic_addr + ED_P0_RBCR0, len);
    OUTBYTE(sc->nic_addr + ED_P0_RBCR1, len >> 8);

    /* Set up destination address in NIC mem.   */
    OUTBYTE(sc->nic_addr + ED_P0_RSAR0, dst);
    OUTBYTE(sc->nic_addr + ED_P0_RSAR1, dst >> 8);
        
    /* Set remote DMA write.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD1 | ED_CR_PAGE_0 | ED_CR_STA);

    if (sc->isa16bit)
        outsw((word) (sc->asic_addr + ED_NOVELL_DATA), (PFWORD) src, len / 2);
    else
        outsb((word) (sc->asic_addr + ED_NOVELL_DATA), src, len);

    /*
     * Wait for remote DMA complete.  This is necessary because on the
     * transmit side, data is handled internally by the NIC in bursts and
     * we can't start another remote DMA until this one completes.  Not
     * waiting causes really bad things to happen - like the NIC
     * irrecoverably jamming the ISA bus.
     */
    while (((INBYTE(sc->nic_addr + ED_P0_ISR) & ED_ISR_RDC) != ED_ISR_RDC) && --maxwait);
}

/*
 * Write a DCU to the destination NIC memory address using programmed I/O
 */
void ed_pio_write_pkt(PIFACE pi, PED_SOFTC sc, PFBYTE packet, int length, word dst)
{
int maxwait = 100; /* about 120us */

    /* Select page 0 registers.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD2 | ED_CR_PAGE_0 | ED_CR_STA);

    /* Reset remote DMA complete flag.   */
    OUTBYTE(sc->nic_addr + ED_P0_ISR, ED_ISR_RDC);

    /* Set up DMA byte count.   */
    OUTBYTE(sc->nic_addr + ED_P0_RBCR0, length);
    OUTBYTE(sc->nic_addr + ED_P0_RBCR1, length >> 8);

    /* Set up destination address in NIC mem.   */
    OUTBYTE(sc->nic_addr + ED_P0_RSAR0, dst);
    OUTBYTE(sc->nic_addr + ED_P0_RSAR1, dst >> 8);

    /* Set remote DMA write.   */
    OUTBYTE(sc->nic_addr + ED_P0_CR, ED_CR_RD1 | ED_CR_PAGE_0 | ED_CR_STA);

    /*
     * Transfer DCU to the NIC memory.
     * 16-bit cards require that data be transferred as words, and only
     * words, so that case requires some extra code to patch over
     * odd-length DCUs.
     */
    if (!sc->isa16bit) 
    {
        /* NE1000s are easy.   */
        outsb((word) (sc->asic_addr + ED_NOVELL_DATA), packet, length);
    } 
    else 
    {
        /* NE2000s are a bit trickier.   */
        byte savebyte[2];

        if (length > 0) 
        {
            /* Output contiguous words.   */
            if (length > 1) 
            {
                outsw((word)(sc->asic_addr + ED_NOVELL_DATA), (PFWORD) packet, length >> 1);
                packet += length & ~1;
                length &= 1;
            }

            /* If odd number of bytes, output last byte as a word with a 0 appended   */
            if (length == 1) 
            {
                savebyte[0] = *packet;
                savebyte[1] = 0;
                OUTWORD((sc->asic_addr + ED_NOVELL_DATA), *(word *)savebyte);
            }
        }
    }
        
    /*
     * Wait for remote DMA complete.  This is necessary because on the
     * transmit side, data is handled internally by the NIC in bursts and
     * we can't start another remote DMA until this one completes.  Not
     * waiting causes really bad things to happen - like the NIC
     * irrecoverably jamming the ISA bus.
     */
    while ( ((INBYTE(sc->nic_addr + ED_P0_ISR) & ED_ISR_RDC) != ED_ISR_RDC) && 
            --maxwait);

    if (!maxwait) 
    {
        DEBUG_ERROR("remote transmit DMA failed to complete", NOVAR, 0, 0);
        sc->stats.errors_out++;
        sc->stats.tx_other_errors++;
        ed_reset(pi, sc);
    }

}
#endif

/* calc values for multicast    */
void ed_getmcaf(PFBYTE mclist, int lenmclist, PFBYTE af)
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


/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_ed(int minor_number)
{
    return(xn_device_table_add(ed_device.device_id, 
                        minor_number, 
                        ed_device.iface_type,
                        ed_device.device_name,
                        SNMP_DEVICE_INFO(ed_device.media_mib, 
                                         ed_device.speed)                           
                        (DEV_OPEN)ed_device.open,
                        (DEV_CLOSE)ed_device.close,
                        (DEV_XMIT)ed_device.xmit,
                        (DEV_XMIT_DONE)ed_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ed_device.proc_interrupts,
                        (DEV_STATS)ed_device.statistics,
                        (DEV_SETMCAST)ed_device.setmcast));
}
#if (INCLUDE_PCMCIA)
int xn_bind_ed_pcmcia(int minor_number)
{
    return(xn_device_table_add(NE2000_PCMCIA_DEVICE,
                        minor_number, 
                        ed_device.iface_type,
                        "NE2000 PCMCIA",
                        SNMP_DEVICE_INFO(ed_device.media_mib, 
                                         ed_device.speed)                           
                        (DEV_OPEN)ed_device.open,
                        (DEV_CLOSE)ed_device.close,
                        (DEV_XMIT)ed_device.xmit,
                        (DEV_XMIT_DONE)ed_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)ed_device.proc_interrupts,
                        (DEV_STATS)ed_device.statistics,
                        (DEV_SETMCAST)ed_device.setmcast));
}
#endif
#endif      /* DECLARING_DATA */
#endif      /* INCLUDE_ED */
