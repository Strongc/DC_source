/*                                                                          */
/* LANCE Support for shared memory added July 15, 1997                      */
/* The driver may be configured to operate in bus master mode or            */
/* shared memory mode                                                       */
/*                                                                          */
/* EBS - VK, October 1998                                                   */
/*       Bus Master and Shared Memory are modes of the ISA bus.  For        */
/*       the PCI-based Am79C972, the driver must be in Bus Master Mode.     */
/*                                                                          */
/* To use bus master mode make sure the followin line is defined lower in   */
/* the file:                                                                */
/*  #define USE_LANCE_BUS_MASTER 1                                          */
/* To us shared memory mode make sure the following is defined lower in the */
/* file:                                                                    */
/*  #define USE_LANCE_SHARED_MEM 1                                          */
/*                                                                          */
/* Also in shared memory mode you must define a constant that points to     */
/* the shared memory. The following is the default. Change it to match      */
/* you memory map.                                                          */
/* #define LANCE_SHARED_MEM_ADDRESS   (PFBYTE)  0xD0000000                  */
/*                                                                          */
/* EBS - VK, October 1998                                                   */
/*       Added PCI support to allow the Am79C972, PCnet-FAST+ board         */
/*       to be accessed using the backwards compatibility (to 7990)         */
/*       feature of the board registers.                                    */
/*       This support included:                                             */
/*          Adding an enum, PCNET-FASTP and an entry in chip_table.         */
/*          Adding/modifying code for PCI initialization in routine         */
/*              lance_init.                                                 */
/*          Masking out DMA initialization in routines dma_init &           */
/*              dma_stop as the need for a DMA channel is ISA specific.     */
/*     - VK, November 1998                                                  */
/*       Added PCI I/O Base Register and PCI Interrupt Line Register        */
/*       default values for systems that don't assign resources to devices. */
/*                                                                          */
/*                                                                          */

/* tbd - irq, alignment, mem_start, init_etherdev, stk   */

/* lance.c: An AMD LANCE ethernet driver for linux.   */
/*
    Written 1993,1994,1995 by Donald Becker.

    Copyright 1993 United States Government as represented by the
    Director, National Security Agency.
    This software may be used and distributed according to the terms
    of the GNU Public License, incorporated herein by reference.

    This driver is for the Allied Telesis AT1500 and HP J2405A, and should work
    with most other LANCE-based bus-master (NE2100 clone) ethercards.

    The author may be reached as becker@CESDIS.gsfc.nasa.gov, or C/O
    Center of Excellence in Space Data and Information Sciences
       Code 930.5, Goddard Space Flight Center, Greenbelt MD 20771
*/

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_LANCE)
#if (CFG_AMD_PCI)
    #include "pci.h"
#endif

#define DEBUG_LANCE 0


/* ********************************************************************   */
#if (MAX_PACKETSIZE < ETHERSIZE+4)
/*SPRF#error: MAX_PACKETSIZE is too small, must be at least 1518    */
#endif


/* ********************************************************************   */
/*
                Theory of Operation

I. Board Compatibility

This device driver is designed for the AMD 79C960, the "PCnet-ISA
single-chip ethernet controller for ISA".  This chip is used in a wide
variety of boards from vendors such as Allied Telesis, HP, Kingston,
and Boca.  This driver is also intended to work with older AMD 7990
designs, such as the NE1500 and NE2100, and newer 79C961.  For convenience,
I use the name LANCE to refer to all of the AMD chips, even though it properly
refers only to the original 7990.

II. Board-specific settings

The driver is designed to work the boards that use the faster
bus-master mode, rather than in shared memory mode.  (Only older designs
have on-board buffer memory needed to support the slower shared memory mode.)

Most ISA boards have jumpered settings for the I/O base, IRQ line, and DMA
channel.  This driver probes the likely base addresses:
{0x300, 0x320, 0x340, 0x360}.
After the board is found it generates a DMA-timeout interrupt and uses
autoIRQ to find the IRQ line.  The DMA channel can be set with the low bits
of the otherwise-unused dev->mem_start value (aka PARAM1).  If unset it is
probed for by enabling each free DMA channel in turn and checking if
initialization succeeds.

The HP-J2405A board is an exception: with this board it's easy to read the
EEPROM-set values for the base, IRQ, and DMA.  (Of course you must already
_know_ the base address -- that field is for writing the EEPROM.)

  EBS-VK,10/98
  Modified driver to add PCI support for the Am79C972 PCnet-FAST+ board.
  Note that the PCI BIOS is queried for the Interrupt Line, I/O Base address,
  Device ID, and these values are written into the private LANCE structure 
  rather than doing the search and query that us necessary for ISA
  based LANCE boards.  See routine lance_init.
  DMA is currently not implemented by the PCI Specification (see pg 497,
  PCI Hardware and Software Architecture and Design).  DMA Channel will
  be defaulted to 4 (i.e., not available to the system).

III. Driver operation

IIIa. Ring buffers
The LANCE uses ring buffers of Tx and Rx descriptors.  Each entry describes
the base and length of the data buffer, along with status bits.  The length
of these buffers is set by LANCE_LOG_{RX,TX}_BUFFERS, which is log_2() of
the buffer length (rather than being directly the buffer length) for
implementation ease.  The current values are 2 (Tx) and 4 (Rx), which leads to
ring sizes of 4 (Tx) and 16 (Rx).  Increasing the number of ring entries
needlessly uses extra space and reduces the chance that an upper layer will
be able to reorder queued Tx packets based on priority.  Decreasing the number
of entries makes it more difficult to achieve back-to-back packet transmission
and increases the chance that Rx ring will overflow.  (Consider the worst case
of receiving back-to-back minimum-sized packets.)

The LANCE has the capability to "chain" both Rx and Tx buffers, but this driver
statically allocates full-sized (slightly oversized -- PKT_BUF_SZ) buffers to
avoid the administrative overhead. For the Rx side this avoids dynamically
allocating full-sized buffers "just in case", at the expense of a
memory-to-memory data copy for each packet received.  For most systems this
is a good tradeoff: the Rx buffer will always be in low memory, the copy
is inexpensive, and it primes the cache for later packet processing.  For Tx
the buffers are only used when needed as low-memory bounce buffers.

IIIB. 16M memory limitations.
For the ISA bus master mode all structures used directly by the LANCE,
the initialization block, Rx and Tx rings, and data buffers, must be
accessible from the ISA bus, i.e. in the lower 16M of real memory.
This is a problem for current Linux kernels on >16M machines. The network
devices are initialized after memory initialization, and the kernel doles out
memory from the top of memory downward.  The current solution is to have a
special network initialization routine that's called before memory
initialization; this will eventually be generalized for all network devices.
As mentioned before, low-memory "bounce-buffers" are used when needed.

IIIC. Synchronization
The driver runs as two independent, single-threaded flows of control.  One
is the send-packet routine, which enforces single-threaded use by the
dev->tbusy flag.  The other thread is the interrupt handler, which is single
threaded by the hardware and other software.

The send packet thread has partial control over the Tx ring and 'dev->tbusy'
flag.  It sets the tbusy flag whenever it's queuing a Tx packet. If the next
queue slot is empty, it clears the tbusy flag when finished otherwise it sets
the 'lp->tx_full' flag.

The interrupt handler has exclusive control over the Rx ring and records stats
from the Tx ring.  (The Tx-done interrupt can't be selectively turned off, so
we can't avoid the interrupt overhead by having the Tx routine reap the Tx
stats.)  After reaping the stats, it marks the queue entry as empty by setting
the 'base' to zero.  Iff the 'lp->tx_full' flag is set, it clears both the
tx_full and tbusy flags.

*/

/* ********************************************************************   */
/* One of these must be defined. Both may not be define                   */
#define USE_LANCE_BUS_MASTER 1
#define USE_LANCE_SHARED_MEM 0
#if (!USE_LANCE_SHARED_MEM && !USE_LANCE_BUS_MASTER)
#error
#endif
#if (USE_LANCE_SHARED_MEM && USE_LANCE_BUS_MASTER)
#error
#endif
/* For shared memory system we need to know the base address of the 
   shared memory. This value is expressed as a constant pointer to byte 
   The address selected here is ISA physical address D0000
   The IS_PM section declares a flat mode pointer to this base.. the 
   other section is a real mode segment:offset pointer. In no intel
   environments just put the memory address of the region here */
#if (USE_LANCE_SHARED_MEM)
#   if (IS_MS_PM || IS_BCC_PM || IS_HC_PM)  /* protected mode */
#       define LANCE_SHARED_MEM_ADDRESS   (PFBYTE) 0xD0000
#   elif (defined(__BORLANDC__) || defined(_MSC_VER) )
#       define LANCE_SHARED_MEM_ADDRESS   (PFBYTE)  0xD0000000
#   else
#       error - Define an address for the shared memory area of the lance card
#   endif
#endif

/* Set the number of Tx and Rx buffers, using Log_2(# buffers).
   Reasonable default values are 4 Tx buffers, and 16 Rx buffers.
   That translates to 2 (4 == 2^^2) and 4 (16 == 2^^4). */
#ifndef LANCE_LOG_TX_BUFFERS
#define LANCE_LOG_TX_BUFFERS 2
#define LANCE_LOG_RX_BUFFERS 4
#endif

#undef TX_RING_SIZE
#define TX_RING_SIZE            (1 << (LANCE_LOG_TX_BUFFERS))
#define TX_RING_MOD_MASK        (TX_RING_SIZE - 1)
/* PVO Changed from original. We will look at base as 2 16 bit registers
   so use << 13 instead of << 29 */
#define TX_RING_LEN_BITS        ((LANCE_LOG_TX_BUFFERS) << 13)

#undef RX_RING_SIZE
#define RX_RING_SIZE            (1 << (LANCE_LOG_RX_BUFFERS))
#define RX_RING_MOD_MASK        (RX_RING_SIZE - 1)
/* PVO Changed from original. We will look at base as 2 16 bit registers
   so use << 13 instead of << 29 */
#define RX_RING_LEN_BITS        ((LANCE_LOG_RX_BUFFERS) << 13)

#define PKT_BUF_SZ      1544


/* Offsets from base I/O address.   */
#define LANCE_DATA       0x10
#define LANCE_ADDR       0x12
#define LANCE_RESET      0x14
#define LANCE_BUS_IF     0x16       /* EBS-VK: PCI Bus Data Register offset */
#define LANCE_TOTAL_SIZE 0x18       /*       : 0x18-0x1f reserved on 79C972 */

#define SWAPIF(X) X

/* ********************************************************************   */
/* The LANCE Rx and Tx ring descriptors.                                  */
struct lance_rx_head 
{
    word  ladrf;            /* low order 16 bits of buffer address */
    word  status_hadrf;     /* status in hi byte hadrf in low byte */
    word  buf_length;           /* This length is 2s complement (negative)! */
    word  msg_length;           /* This length is "normal". */
};

struct lance_tx_head 
{
    word  ladrf;            /* low order 16 bits of buffer address */
    word  status_hadrf;     /* status in hi byte hadrf in low byte */
    word  length;               /* Length is 2s complement (negative)! */
    word  misc;
};

/* The LANCE initialization block, described in databook.   */
struct lance_init_block 
{
    word mode;      /* Pre-set mode (reg. 15) */
    unsigned char phys_addr[6]; /* Physical ethernet address */
    dword filter[2];            /* Multicast filter (unused). */
    /* Receive and transmit ring base, along with extra bits.   */
    word rx_ring_lo;            /* Tx and Rx ring base pointers */
    word rx_ring_hi;            /* Tx and Rx ring base pointers */
    word tx_ring_lo;            /* Tx and Rx ring base pointers */
    word tx_ring_hi;            /* Tx and Rx ring base pointers */
};

typedef struct lance_private 
{
    int     irq;                /* Interrupt  */
    word    base_addr;          /* default io base  */
    int     in_interrupt;       /* flag set if in interrupt service routine */
    byte    dev_addr[ETH_ALEN];
    dword   trans_start;
    struct  ether_statistics stats;
    PIFACE  iface;
    /* The Tx and Rx ring entries must aligned on 8-byte boundaries.   */
    byte    pad_rx[8];  /* For padding */
    struct  lance_rx_head rx_ring_core[RX_RING_SIZE];
    byte    pad_tx[8];  /* For padding */
    struct  lance_tx_head tx_ring_core[TX_RING_SIZE];
    /* core for init block   */
    struct  lance_init_block        init_block;
    struct  lance_rx_head KS_FAR *prx_ring;
    struct  lance_tx_head KS_FAR *ptx_ring;
    struct  lance_init_block KS_FAR *pinit_block;
#if (USE_LANCE_SHARED_MEM) 
    /* In bus master mode we carve up core in the shared memory and
       put the host side memory addresses in these arrays.
       the lance side addresses are in the ring buffers themselves */
    PFBYTE prx_shared_mem[RX_RING_SIZE];
    PFBYTE ptx_shared_mem[TX_RING_SIZE];
#endif
#if (USE_LANCE_BUS_MASTER)
    DCU    rx_dcus[RX_RING_SIZE];   /* Each entry in the ring has a dcu */
#endif

    int cur_rx;         /* The next free ring entry */
    int cur_tx;         /* The next free ring entry */
    int dirty_rx;       /* The ring entries to be free()ed. */
    int dirty_tx;       /* The ring entries to be free()ed. */
    int lance_dma;
    unsigned char chip_version;         /* See lance_chip_type. */
    char tx_full;
    char lock;
    int pad0, pad1;             /* Used for 8-byte alignment */
} lance_private;
typedef struct lance_private KS_FAR *PLANCE_PRIV;

#define LANCE_MUST_PAD          0x00000001
#define LANCE_ENABLE_AUTOSELECT 0x00000002
#define LANCE_MUST_REINIT_RING  0x00000004
#define LANCE_MUST_UNRESET      0x00000008
#define LANCE_HAS_MISSED_FRAME  0x00000010

static void lance_restart(PLANCE_PRIV lp, unsigned int csr0_bits, int must_reinit);
void dma_init(int channel);


/* A mapping from the chip ID number to the part number and features.
   These are from the datasheets -- in real life the '970 version
   reportedly has the same ID as the '965. */
struct lance_chip_type 
{
    dword id_number;
    char *name;
    int flags;
};

enum {OLD_LANCE = 0, PCNET_ISA=1, PCNET_ISAP=2, PCNET_ISAPA=3, PCNET_PCI=4, PCNET_VLB=5, PCNET_FASTP=6, LANCE_UNKNOWN=7};

/* Non-zero only if the current card is a PCI with BIOS-set IRQ.   */

/* ********************************************************************   */
/* DATA                                                                   */

struct lance_chip_type lance_chip_table[] = 
{
    {0x0000, "LANCE 7990",              /* Ancient lance chip.  */
        LANCE_MUST_PAD + LANCE_MUST_UNRESET},
    {0x0003, "PCnet/ISA 79C960",        /* 79C960 PCnet/ISA.  */
        LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
            LANCE_HAS_MISSED_FRAME},
    {0x2260, "PCnet/ISA+ 79C961",       /* 79C961 PCnet/ISA+, Plug-n-Play.  */
        LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
            LANCE_HAS_MISSED_FRAME},
    {0x2261, "PCnet/ISA+ 79C961A",      /* 79C961A PCnet/ISA+, Plug-n-Play.  */
        LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
            LANCE_HAS_MISSED_FRAME},
    {0x2420, "PCnet/PCI 79C970",        /* 79C970 or 79C974 PCnet-SCSI, PCI. */
        LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
            LANCE_HAS_MISSED_FRAME},
    /* Bug: the PCnet/PCI actually uses the PCnet/VLB ID number, so just call
        it the PCnet32. */
    {0x2430, "PCnet32",                 /* 79C965 PCnet for VL bus. */
        LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
            LANCE_HAS_MISSED_FRAME},
    {0x2624, "PCnet/FAST+ 79C972",      /* 79C972 PCnet-FAST+. PCI-based*/
        LANCE_MUST_REINIT_RING + LANCE_HAS_MISSED_FRAME},
    {0x0,    "PCnet (unknown)",
        LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
            LANCE_HAS_MISSED_FRAME},
};

struct lance_private lp_priv[CFG_NUM_LANCE];


/* ********************************************************************   */
/* FUNCTION DECLARATIONS                                                  */
RTIP_BOOLEAN lance_probe1(PLANCE_PRIV lp);
static void lance_init_ring(PLANCE_PRIV lp);
static void lance_rx(PLANCE_PRIV lp);
void lance_interrupt(int minor_no);
static void set_multicast_list(PLANCE_PRIV lp, int num_addrs, void *addrs);

static void lance_purge_rx_ring(PLANCE_PRIV lp);
/* ********************************************************************   */
/* MEMORY MANAGMENT                                                       */
/* ********************************************************************   */
PLANCE_PRIV iface_to_lp(PIFACE pi) 
{
int lp_off;
    
    lp_off = pi->minor_number;
    if (lp_off >= CFG_NUM_LANCE) 
    {
        DEBUG_ERROR("iface_to_lp() - pi->minor_number, CFG_NUM_LANCE =",
            EBS_INT2, pi->minor_number, CFG_NUM_LANCE);
        return((PLANCE_PRIV)0);
    }

    return((PLANCE_PRIV) &lp_priv[lp_off]);
}

PLANCE_PRIV off_to_lp(int lp_off) 
{
    if (lp_off >= CFG_NUM_LANCE) 
    {
        DEBUG_ERROR("off_to_lp() - lp_off, CFG_NUM_LANCE =",
            EBS_INT2, lp_off, CFG_NUM_LANCE);
        return((PLANCE_PRIV)0);
    }

    return((PLANCE_PRIV) &lp_priv[lp_off]);
}

/* ********************************************************************   */
/* INITIALIZATION                                                         */
/* ********************************************************************   */
/* set board values to either values set by application or default values */
/* specified by device table                                              */
void set_lance_vals(PIFACE pi, PLANCE_PRIV lp)
{

    /* set values based on global variable set by application if they are    */
    /* not set to their initial value otherwise                              */
    /* set them to default values from the device table                      */
    /* NOTE:  EBS - VK, 10/98                                                */
    /*              In the case of the PCI-based PCnet-FAST+(Am79C972),      */
    /*              the irq and base_addr will be reset in lance_init        */
    /*              based on the values read from the PCI configuration      */
    /*              registers.                                               */

    /* io base address   */
    lp->base_addr = pi->io_address; 

    /* interrupt number   */
    lp->irq = pi->irq_val; 
}

/* Assign a receive buffer to the lance   */
void give_lance_rx_buf(PLANCE_PRIV lp, int index)
{
word w;
struct lance_rx_head *ph;
    ph = lp->prx_ring + index;
    w = SWAPIF(ph->status_hadrf);
    w = (word)(w | 0x8000);
    ph->status_hadrf = SWAPIF(w); /* Hi 8 bits of addres + NIC owns buffer */
}

RTIP_BOOLEAN alloc_lance_rx_buf(PLANCE_PRIV lp, int index)
{
PFBYTE addr;
dword linear_32;
word w;
short i;
struct lance_rx_head *ph;
#if (USE_LANCE_BUS_MASTER)
DCU msg;
#endif
    
#if (USE_LANCE_SHARED_MEM)
    /* In shared mode we already have the addresses   */
    addr = lp->prx_shared_mem[index];
#endif

#if (USE_LANCE_BUS_MASTER)
    /* allocate a DCU, keep track of it and give it to the controller   */
    msg = lp->rx_dcus[index]; /* PETER - check if already alloced */
    if (!msg)
    {
        msg = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC);

        if (!msg)
        {
            DEBUG_ERROR("alloc_lance_rx_buf: out of DCUs", NOVAR, 0, 0);
            return(FALSE);
        }
        lp->rx_dcus[index] = msg;
    }
    addr = (PFBYTE) DCUTODATA(msg);
#endif

    ph = lp->prx_ring + index;
    i = -(ETHERSIZE+4);
    w = (word) i;
    ph->buf_length = SWAPIF(w); /* buffer length */
    linear_32 = kvtop(addr);
    w = (word) linear_32;
    ph->ladrf = SWAPIF(w);  /* low 16 bits of address */
    w = (word)(linear_32 >> 16);
    w = (word)(w | 0x8000);
    ph->status_hadrf = SWAPIF(w); /* Hi 8 bits of addres + NIC owns buffer */
    return(TRUE);
}

/* This sets up a transmit ring descriptor to contain the address of
   a memory area. it is use by both shared memory and bus master modes.
   the ownership of the area remains with the host */
void setup_lance_tx_buf(PLANCE_PRIV lp, int index, PFBYTE packet)
{
struct lance_tx_head *ph;
word w;
dword linear_32;

    ph = lp->ptx_ring+index;
    linear_32 = kvtop((PFBYTE)packet);
    w = (word) linear_32;
    ph->ladrf = SWAPIF(w);  /* low 16 bits of address */
    w   =   (word) (linear_32 >> 16);
    w &= 0x00ff;
    ph->status_hadrf = SWAPIF(w);
}


/* ********************************************************************   */
/* This lance probe is unlike the other board probes in 1.0.*.  The LANCE may
   have to allocate a contiguous low-memory region for bounce buffers.
   This requirement is satisfied by having the lance initialization occur
   before the memory management system is started, and thus well before the
   other probes. */

RTIP_BOOLEAN lance_init(PIFACE pi, PLANCE_PRIV lp)
{
    /*                                                                  */
    /*  EBS - VK, 10/98 Support added for PCI-based Am79C972            */
    /*                                                                  */
    /*  Configure the PCI registers.  (CFG_AMD_PCI defined in xnconf.h) */
    /*  If this is successful                                           */
    /*    call lance_probe1                                             */
    /*  else                                                            */
    /*    return an error code [indicating that the PCI init failed].   */
    /*                                                                  */
#if (CFG_AMD_PCI)
    unsigned char return_code;
    unsigned char BusNum;
    unsigned char DevFncNum;
    unsigned char default_irq;
    unsigned char byte_read;
    unsigned short word_read;
    int Index;
    struct four_bytes {
        unsigned short  wordl;
        unsigned short  wordh;
    };
    union {
        struct four_bytes two_words;
        unsigned long   cat_words;
    } d_word;

    if (rtpci_bios_present())
    {
        /*                                                                       */
        /*  Find and initialize the first specified (Vendor, Device)PCI device . */
        /*  Since auto-negotiation (DANAS (bit 10, BCR32 == 0) and               */
        /*  ASEL (bit 1, BCR2 == 1)) is the default, it is assumed               */
        /*  that sw need do nothing to force auto-negotiation to occur.          */
        /*                                                                       */
        /*  The index in the Find PCI Device indicates the instance of the       */
        /*  device to search for.                                                */
        /*  The minor device number should indicate the instance of the device,  */
        /*    therefore index is set equal to minor device number.               */
        /*                                                                       */
#        if (DEBUG_LANCE)
            DEBUG_ERROR("PCI Index. minor_number=", EBS_INT1, pi->minor_number, 0);
#        endif
        Index = pi->minor_number;
#        if (DEBUG_LANCE)
            DEBUG_ERROR("PCI Index. Index =", EBS_INT1, Index, 0);
#        endif

        return_code = rtpci_find_device(RTPCI_D_ID_AM79972, RTPCI_V_ID_AMD, 
                                        Index, &BusNum, &DevFncNum);
        if (return_code == RTPCI_K_SUCCESSFUL)
        {
            /*                                                              */
            /*  Set the interrupt line based on the value in the            */
            /*  PCI Interrupt Line Register.                                */
            /*  Note:  This writes a byte into an int location.  Any issues */
            /*         here?                                                */
            /*                                                              */
#            if (DEBUG_LANCE)
                DEBUG_ERROR("PCI Device found", 0, 0, 0);
#            endif
            return_code = rtpci_read_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE, &byte_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
#            if (DEBUG_LANCE)
                DEBUG_ERROR("Reading IntLine Reg. byteread=", EBS_INT1, byte_read, 0);
#            endif
                if (byte_read == RTPCI_INT_LINE_NOVAL)
                {
                    /* set the default interrupt register based on either the    */
                    /* user input value (for demo programs) or                   */
                    /* the default value set in xnconf.h.                        */
                    if (pi->irq_val != -1)
                        default_irq = pi->irq_val;
                    else
                        default_irq = CFG_AMD_PCI_IRQ;
#                    if (DEBUG_LANCE)
                        DEBUG_ERROR("default_irq = ", EBS_INT1, default_irq, 0);
#                    endif
                    return_code = rtpci_write_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE, default_irq);
                    if (return_code == RTPCI_K_SUCCESSFUL)
                        lp->irq = default_irq;
                    else
                        return(FALSE);
                }
                else
                    lp->irq = byte_read;
            }
            else 
                return(FALSE);  /* INTERRUPT LINE Register read failed  */

            /*                                                                            */
            /*  Let the PCI Latency Timer Register default.                               */
            /*                                                                            */
            /*                                                                            */
            /*  Read the I/O Base Register or the Memory Mapped I/O                       */
            /*  Register and store the address as the base address of the device.         */
            /*  This is a double word (32-bit register).  I cannot access it as such      */
            /*  on a 16-bit system in REAL MODE.  Hence, I am doing two word reads.       */
            /*  The 5 low bits of the register are not address bits, therefore I am       */
            /*  masking the register with the value 0xFFFF FFE0 (defined in RTPCIDEF.H).  */
            /*  For PCI devices, bits 0-7 address the max 256 bytes that the device       */
            /*  can request, bits 8&9 MBZ as these bits indicate ISA devices,             */
            /*  at least one of bits 12-15 must be 1.                                     */
            /*  Note that I am assuming that the system BIOS  is assigning the I/O space  */
            /*  system resource (see pg 755 of PCI HW & SW Architecture and Design).      */
            /*                                                                            */
            return_code = rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_IOBASE, &d_word.two_words.wordl);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                if (d_word.two_words.wordl == RTPCI_IOBASE_NOVAL)
                {
#                if (DEBUG_LANCE)
                    DEBUG_ERROR("Writing default I/O base value", 0, 0, 0);
#                endif
                    /* if no address is present in the I/O base register,   */
                    /* set the default I/O base address based on the        */
                    /* user input i/o address (in demo/test programs)       */
                    /* or the default value from xnconf.h.                  */
                    if (pi->io_address != 0)
                        d_word.two_words.wordl = pi->io_address;
                    else
                        d_word.two_words.wordl = CFG_AMD_PCI_IOBASE;
                    return_code = rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_IOBASE, d_word.two_words.wordl);
                    if (return_code == RTPCI_K_SUCCESSFUL)
                        lp->base_addr = d_word.two_words.wordl;
                    else
                        return(FALSE);  /* I/O BASE Register Write Failed   */
                }
                else
                    lp->base_addr = d_word.two_words.wordl & RTPCI_M_IOBASE_L;

#            if (DEBUG_LANCE)
                DEBUG_ERROR("lance_init. wordl=", EBS_INT1, d_word.two_words.wordl, 0);
#            endif
                
                return_code = rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_IOBASE+2, &d_word.two_words.wordh);
                if (return_code == RTPCI_K_SUCCESSFUL)
                {               
#                if (DEBUG_LANCE)
                    DEBUG_ERROR("lance_init. wordh=", EBS_INT1, d_word.two_words.wordh, 0);
#                endif
#                if (DEBUG_LANCE)
                    DEBUG_ERROR("lance_init. base_addr=", EBS_INT1, lp->base_addr, 0);
#                endif
                }
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /*  IOBASE Register read failed */

            /*                                                     */
            /*  Write PCI Command Register enabling Bus Mastering  */
            /*  (BMEM) and enabling I/O accesses (IOEN).           */
            /*                                                     */
            return_code = rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_CMD, &word_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                word_read |= RTPCI_M_CMD_IOEN | RTPCI_M_CMD_BMEM;
                return_code = rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_CMD, word_read);
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /* COMMAND Register read/write failed   */
        }
        else 
            return(FALSE);      /* No PCI Device detected.  */
    }
    else
        return(FALSE);      /* No PCI BIOS present. */

#endif  /* CFG_AMD_PCI */

    return(lance_probe1(lp));
}

RTIP_BOOLEAN lance_probe1(PLANCE_PRIV lp)
{
int i, reset_val, lance_version;
char *chipname;
PFBYTE pc;
dword linear_32;
word w;
#if (DEBUG_LANCE)
        DEBUG_ERROR("lance_probe1 entered.  base_addr", EBS_INT1, lp->base_addr, 0);
#endif
    /* Reset the LANCE.    */
    reset_val = INWORD(lp->base_addr+LANCE_RESET); /* Reset the LANCE */

    /* The Un-Reset needed is only needed for the real NE2100, and will
       confuse the HP board. */
    OUTWORD(lp->base_addr+LANCE_RESET, reset_val);

    OUTWORD(lp->base_addr+LANCE_ADDR, 0x0000); /* Switch to window 0 */
    if (INWORD(lp->base_addr+LANCE_DATA) != 0x0004)
        return FALSE;

    /* Get the version of the chip.                       */
    /* This works for Am79C972 as well.  EBS-VK, 10/98    */
    OUTWORD(lp->base_addr+LANCE_ADDR, 88);
    if (INWORD(lp->base_addr+LANCE_ADDR) != 88) 
    {
        lance_version = 0;
    } 
    else                            /* Good, it's a newer chip. */
    {
        dword chip_version = (dword) INWORD(lp->base_addr+LANCE_DATA);
        dword ltemp;
        OUTWORD(lp->base_addr+LANCE_ADDR, 89);
        ltemp = INWORD(lp->base_addr+LANCE_DATA);
        chip_version |= ltemp << 16;
#if (DEBUG_LANCE)
        DEBUG_ERROR("LANCE chip version is ", EBS_INT1, chip_version, 0);
#endif

        if ((chip_version & 0xfff) != 0x003)
        {
            return FALSE;
        }

        chip_version = (chip_version >> 12) & 0xffff;

        for (lance_version = 1; lance_chip_table[lance_version].id_number; 
             lance_version++) 
        {
            if (lance_chip_table[lance_version].id_number ==  chip_version)
                break;
        }
    }

    chipname = lance_chip_table[lance_version].name;
#if (DEBUG_LANCE)
    DEBUG_ERROR("lance_probe: chip name", STR1, chipname, 0);
#endif

    /* There is a 16 byte station address PROM at the base address.
       The first six bytes are the station address. */
    for (i = 0; i < ETH_ALEN; ++i)
    {
        lp->dev_addr[i] = lp->iface->addr.my_hw_addr[i] = INBYTE(lp->base_addr + i);
    }

    /* Make certain the data structures used by the LANCE are aligned.   */
    /* tbd: dev->priv = (void *)(((int)dev->priv + 7) & ~7);             */
#if (USE_LANCE_BUS_MASTER)
    /* Bus master mode. point at the rx_ring in the private structure
       and round down into the padding until on an 8 bit boundary */
    pc = (PFBYTE) &(lp->rx_ring_core[0]);
    while ((dword)pc & 7) pc--;
#endif
#if (USE_LANCE_SHARED_MEM)
    /* Place the shared memory resources into the shared memory area.
       The address of this area is LANCE_SHARED_MEM_ADDRESS.. defined 
       above or in xnconf.h */
    pc = LANCE_SHARED_MEM_ADDRESS;
    while ((dword)pc & 7) pc++;
#endif
    lp->prx_ring = (struct lance_rx_head KS_FAR *) pc;

#if (USE_LANCE_BUS_MASTER)
    /* Bus master mode. point at the tx_ring in the private structure
       and round down into the padding until on an 8 byte boundary */
    pc = (PFBYTE) &(lp->tx_ring_core[0]);
    while ((dword)pc & 7) pc--;
#endif
#if (USE_LANCE_SHARED_MEM)
    /* Shared mem mode. Point beyond the rx_ring in shared mem
       and round up until on an 8 byte boundary */
    pc = (PFBYTE) (pc + sizeof(lp->rx_ring_core));
    while ((dword)pc & 7) pc++;
#endif
    lp->ptx_ring = (struct lance_tx_head KS_FAR *) pc;

    /* grab a region for the init block. This code works in shared and
       non shared mode */
    /* Take ptx_ring and add TX_RING_SIZE to it (this is in TX_RING_ENTRY Chunks)   */
    lp->pinit_block = (struct lance_init_block KS_FAR *)(lp->ptx_ring + TX_RING_SIZE);

#if (USE_LANCE_SHARED_MEM)
    /* In shared memory mode carve up the core in the shared memory
       space */
    pc = (PFBYTE) (lp->pinit_block + sizeof(*lp->pinit_block));
    for (i = 0; i < RX_RING_SIZE; i++)
    {
        lp->prx_shared_mem[i] = pc;
        pc += PKT_BUF_SZ;
    }
    for (i = 0; i < TX_RING_SIZE; i++)
    {
        lp->ptx_shared_mem[i] = pc;
        pc += PKT_BUF_SZ;
    }
#endif
    lp->chip_version = (unsigned char)lance_version;

    lp->pinit_block->mode = 0x0003;     /* Disable Rx and Tx. */
    for (i = 0; i < 6; i++)
        lp->pinit_block->phys_addr[i] = lp->dev_addr[i];
    lp->pinit_block->filter[0] = 0x00000000ul;
    lp->pinit_block->filter[1] = 0x00000000ul;

    linear_32 = kvtop((PFBYTE)lp->prx_ring);
    w = (word) linear_32;
    lp->pinit_block->rx_ring_lo = SWAPIF(w);
    w = (word) (linear_32 >> 16);
    w |= RX_RING_LEN_BITS;
    lp->pinit_block->rx_ring_hi = SWAPIF(w);
    linear_32 = kvtop((PFBYTE)lp->ptx_ring);
    w = (word) linear_32;
    lp->pinit_block->tx_ring_lo = SWAPIF(w);
    w = (word) (linear_32 >> 16);
    w |= TX_RING_LEN_BITS;
    lp->pinit_block->tx_ring_hi = SWAPIF(w);

    linear_32 = kvtop((PFBYTE)lp->pinit_block);
    OUTWORD(lp->base_addr+LANCE_ADDR, 0x0001);
    INWORD(lp->base_addr+LANCE_ADDR);
    w = (word) linear_32;
    OUTWORD(lp->base_addr+LANCE_DATA, w);
    OUTWORD(lp->base_addr+LANCE_ADDR, 0x0002);
    INWORD(lp->base_addr+LANCE_ADDR);
    w = (word) (linear_32>>16);
    OUTWORD(lp->base_addr+LANCE_DATA, w);
    OUTWORD(lp->base_addr+LANCE_ADDR, 0x0000);
    INWORD(lp->base_addr+LANCE_ADDR);

    /* tbd (see origional code)   */
    lp->lance_dma = 4;          /* Native bus-master, no DMA channel needed. */
              /*    It is a bus master and CSR8 is reserved and LANCE_BUS_IF doesn't exist.   */
    if ((lance_version == PCNET_ISAP)||(lance_version == PCNET_ISAPA))      /* A plug-n-play version. */
    {
        int bus_info;
        OUTWORD(lp->base_addr+LANCE_ADDR, 8);
        bus_info = INWORD(lp->base_addr+LANCE_BUS_IF);
        lp->lance_dma = bus_info & 0x07;
        lp->irq = (bus_info >> 4) & 0x0F;
    } 
    else if (lance_version != PCNET_FASTP)
    {
        DEBUG_ERROR(" Not PCNET_ISAP oops ", NOVAR, 0, 0);
        return(FALSE);
        /* The DMA channel may be passed in PARAM1.      */
/*      if (lp->mem_start & 0x07)                        */
/*          lp->lance_dma = (int)(lp->mem_start & 0x07); */
    }

    if ((lp->lance_dma == 4) && (lance_version != PCNET_FASTP)) 
    {
#if (DEBUG_LANCE)
        DEBUG_ERROR(", no DMA needed.", NOVAR, 0, 0);
#endif
    } 
    else
    {
#if (USE_LANCE_BUS_MASTER)
#if (DEBUG_LANCE)
        DEBUG_ERROR("no call init now bus master mode. dma chnl == ", EBS_INT1,lp->lance_dma, 0);
#endif
        dma_init(lp->lance_dma);
#endif
        } 

    if (lance_chip_table[lp->chip_version].flags & LANCE_ENABLE_AUTOSELECT) 
    {
        /* Turn on auto-select of media (10baseT or BNC) so that the user
           can watch the LEDs even if the board isn't opened. */
        OUTWORD(lp->base_addr+LANCE_ADDR, 0x0002);
        OUTWORD(lp->base_addr+LANCE_BUS_IF, 0x0002);
    }

    return TRUE;
}
/* STOP the board. and read CSR12, 13 14   */
/*  OUTWORD(ioaddr+LANCE_ADDR, 0);         */
/*  OUTWORD(ioaddr+LANCE_DATA, 0x0004);    */
void load_lance(int ioaddr)
{
    OUTWORD(ioaddr+LANCE_ADDR, 12);
    OUTWORD(ioaddr+LANCE_DATA, 0x01);
    OUTWORD(ioaddr+LANCE_ADDR, 13);
    OUTWORD(ioaddr+LANCE_DATA, 0x02);
    OUTWORD(ioaddr+LANCE_ADDR, 14);
    OUTWORD(ioaddr+LANCE_DATA, 0x03);
}
static void dump_lance(int ioaddr)
{
word w;

    OUTWORD(ioaddr+LANCE_ADDR, 12);
    w = INWORD(ioaddr+LANCE_DATA);
DEBUG_ERROR("ADDR 12 == ", EBS_INT1, w, 0);
    OUTWORD(ioaddr+LANCE_ADDR, 13);
    w = INWORD(ioaddr+LANCE_DATA);
DEBUG_ERROR("ADDR 13 == ", EBS_INT1, w, 0);
    OUTWORD(ioaddr+LANCE_ADDR, 14);
    w = INWORD(ioaddr+LANCE_DATA);
DEBUG_ERROR("ADDR 14 == ", EBS_INT1, w, 0);
}
/* ********************************************************************   */
/* OPEN and CLOSE routines.                                               */
/* ********************************************************************   */
RTIP_BOOLEAN lance_open(PIFACE pi)
{
PLANCE_PRIV lp;
int ioaddr;
int i;
dword linear_32;
word w;

    lp = iface_to_lp(pi);
    if (!lp)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    tc_memset((PFBYTE) lp, 0, sizeof(*lp));

    /* set up pointers between iface and lp structures   */
    lp->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(lp->stats);

    set_lance_vals(pi, lp);

#if (USE_LANCE_SHARED_MEM)
    DEBUG_ERROR("lance using shared memory", NOVAR, 0, 0);
#endif

    if (!lance_init(pi, lp))
    {
        DEBUG_ERROR("lance init or probe failed", NOVAR, 0, 0);
        set_errno(EPROBEFAIL);
        return(FALSE);
    }
    ioaddr = lp->base_addr;

    /* set up pointers between iface and ed_softc structures   */
    lp->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(lp->stats);

    /* Reset the LANCE   */
    INWORD(ioaddr+LANCE_RESET);

    /* Un-Reset the LANCE, needed only for the NE2100.   */
    if (lance_chip_table[lp->chip_version].flags & LANCE_MUST_UNRESET)
        OUTWORD(ioaddr+LANCE_RESET, 0);

    if (lance_chip_table[lp->chip_version].flags & LANCE_ENABLE_AUTOSELECT) 
    {
        /* This is 79C960-specific: Turn on auto-select of media (AUI, BNC).   */
        OUTWORD(ioaddr+LANCE_ADDR, 0x0002);
        OUTWORD(ioaddr+LANCE_BUS_IF, 0x0002);
    }

#if (DEBUG_LANCE)
    DEBUG_ERROR("lance_open() irq =; lance_dma = ", DINT2,
           lp->irq, lp->lance_dma);
/* following line does not compile with DEBUG Flag set   */
/*  DEBUG_ERROR("lance_open() tx/rx rings = ", DINT2,    */
/*         lp->tx_ring, (int) lp->rx_ring);              */
/*                                                       */
    DEBUG_ERROR("lance_open() init = ", DINT1, lp->pinit_block, 0);
#endif

    lance_init_ring(lp);

    /* Re-initialize the LANCE, and start it when done.   */
/* TBD HERE.. InitBlock must be correct                   */
    linear_32 = kvtop((PFBYTE)lp->pinit_block);
    OUTWORD(lp->base_addr+LANCE_ADDR, 0x0001);
    w = (word) linear_32;
    OUTWORD(lp->base_addr+LANCE_DATA, w);
    OUTWORD(lp->base_addr+LANCE_ADDR, 0x0002);
    w = (word) (linear_32>>16);
    OUTWORD(lp->base_addr+LANCE_DATA, w);

    OUTWORD(ioaddr+LANCE_ADDR, 0x0004);
    OUTWORD(ioaddr+LANCE_DATA, 0x0915);

    OUTWORD(ioaddr+LANCE_ADDR, 0x0000);
    OUTWORD(ioaddr+LANCE_DATA, 0x0001);

    lp->in_interrupt = 0;
    i = 0;
    while (i++ < 100)
        if (INWORD(ioaddr+LANCE_DATA) & 0x0100)
            break;

    ks_hook_interrupt(lp->irq, (PFVOID) pi, (RTIPINTFN_POINTER)lance_interrupt, (RTIPINTFN_POINTER) 0,
                      pi->minor_number);

    /* 
     * We used to clear the InitDone bit, 0x0100, here but Mark Stockton
     * reports that doing so triggers a bug in the '974.
     */
    OUTWORD(ioaddr+LANCE_DATA, 0x0042);

#if (DEBUG_LANCE)
    DEBUG_ERROR("LANCE open after %d ticks", EBS_INT1, i, 0);
    DEBUG_ERROR("LANCE open : init block =; csr0 = ", DINT2,
           lp->pinit_block, INWORD(ioaddr+LANCE_DATA));
#endif

    /* tbd: CALLING LANCE RESTART ROUTINE IN OPEN   */
    lance_restart(lp, 0x0043, 1);

    return(TRUE);                   /* Always succeed */
}

void lance_close(PIFACE pi)                     /*__fn__*/
{
int ioaddr;
PLANCE_PRIV lp;

    lp = iface_to_lp(pi);
    if (!lp)
    {
        return;
    }

    ioaddr = lp->base_addr;

    if (lance_chip_table[lp->chip_version].flags & LANCE_HAS_MISSED_FRAME) 
    {
        OUTWORD(ioaddr+LANCE_ADDR, 112);
        /* tbd                                                  */
/*      lp->stats.rx_missed_errors = INWORD(ioaddr+LANCE_DATA); */
    }
    OUTWORD(ioaddr+LANCE_ADDR, 0);

#if (DEBUG_LANCE)
    DEBUG_ERROR("Shutting down ethercard, status was ", EBS_INT1,
           INWORD(ioaddr+LANCE_DATA), 0);
#endif

    /* We stop the LANCE here -- it occasionally polls
       memory if we don't. */
    OUTWORD(ioaddr+LANCE_DATA, 0x0004);
    lance_purge_rx_ring(lp);

}

/* ********************************************************************   */
/* RING routines.                                                         */
/* ********************************************************************   */
/* The LANCE has been halted for one reason or another (busmaster memory
   arbitration error, Tx FIFO underflow, driver stopped it to reconfigure,
   etc.).  Modern LANCE variants always reload their ring-buffer
   configuration when restarted, so we must reinitialize our ring
   context before restarting.  As part of this reinitialization,
   find all packets still on the Tx ring and pretend that they had been
   sent (in effect, drop the packets on the floor) - the higher-level
   protocols will time out and retransmit.  It'd be better to shuffle
   these skbs to a temp list and then actually re-Tx them after
   restarting the chip, but I'm too lazy to do so right now.  dplatt@3do.com
*/

static void lance_purge_tx_ring(PLANCE_PRIV lp)
{
int i;

    for (i = 0; i < TX_RING_SIZE; i++) 
    {
    }
}



static void lance_purge_rx_ring(PLANCE_PRIV lp)
{
int i;

#if (!USE_LANCE_SHARED_MEM) 
    for (i = 0; i < RX_RING_SIZE; i++) 
    {
        /* Free the packet. Then reload the ring entry   */
        if (lp->rx_dcus[i])
            os_free_packet(lp->rx_dcus[i]);
        lp->rx_dcus[i] = 0; /* PETER - zero it */
    }
#endif
}



/* Initialize the LANCE Rx and Tx rings.   */
static void lance_init_ring(PLANCE_PRIV lp)
{
int i;
struct lance_tx_head * ptx;
dword linear_32;
word w;

    lp->cur_rx = lp->cur_tx = 0;
    lp->dirty_rx = lp->dirty_tx = 0;
    for (i = 0; i < RX_RING_SIZE; i++) 
    {
        /* Allocate buffer space in bus master mode or 
           assign it in shared memory mode */
        alloc_lance_rx_buf(lp, i);
    }
    /* The Tx buffer address is filled in as needed, but we do need to clear
       the upper ownership bit. */
    for (i = 0,ptx = lp->ptx_ring; i < TX_RING_SIZE; i++, ptx++) 
    {
#if (USE_LANCE_BUS_MASTER)
    /* In bus master mode zero the descriptors   */
        ptx->ladrf = ptx->status_hadrf = 0;
#endif
#if (USE_LANCE_SHARED_MEM)
    /* In shared memory mode assign shared memory to TX ring but don't give
       lance ownership yet */
    setup_lance_tx_buf(lp, i, lp->ptx_shared_mem[i]);
#endif
    }

    lp->pinit_block->mode = 0x0000;
    for (i = 0; i < 6; i++)
        lp->pinit_block->phys_addr[i] = lp->dev_addr[i];
    lp->pinit_block->filter[0] = 0x00000000;
    lp->pinit_block->filter[1] = 0x00000000;

    linear_32 = kvtop((PFBYTE)lp->prx_ring);
    w = (word) linear_32;
    lp->pinit_block->rx_ring_lo = SWAPIF(w);
    w = (word) (linear_32 >> 16);
    w = (word) (w | RX_RING_LEN_BITS);
    lp->pinit_block->rx_ring_hi = SWAPIF(w);

    linear_32 = kvtop((PFBYTE)lp->ptx_ring);
    w = (word) linear_32;
    lp->pinit_block->tx_ring_lo = SWAPIF(w);
    w = (word) (linear_32 >> 16);
    w = (word) (w | TX_RING_LEN_BITS);
    lp->pinit_block->tx_ring_hi = SWAPIF(w);
}

/* ********************************************************************   */
/* RESTART routine.                                                       */
/* ********************************************************************   */
static void lance_restart(PLANCE_PRIV lp, unsigned int csr0_bits, int must_reinit)
{
    if (must_reinit ||
        (lance_chip_table[lp->chip_version].flags & LANCE_MUST_REINIT_RING)) 
    {
        lance_purge_tx_ring(lp);
        lance_init_ring(lp);
    }
    OUTWORD(lp->base_addr + LANCE_ADDR, 0x0000);
    OUTWORD(lp->base_addr + LANCE_DATA, csr0_bits);
}

/* ********************************************************************   */
/* XMIT routines.                                                         */
/* ********************************************************************   */
RTIP_BOOLEAN lance_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PLANCE_PRIV lp;
int ioaddr;

    lp = iface_to_lp(pi);

    if (!lp)
        return(TRUE);

    ioaddr = lp->base_addr;

    if (success)
    {
        /* Update total number of successfully transmitted packets.   */
        lp->stats.packets_out++;
        lp->stats.bytes_out += DCUTOPACKET(msg)->length; 
    }
    else
    {
        /* error   */
        OUTWORD(ioaddr+LANCE_ADDR, 0);
        DEBUG_ERROR("transmit timed out, status = ; resetting.", EBS_INT1,
               INWORD(ioaddr+LANCE_DATA), 0);
        OUTWORD(ioaddr+LANCE_DATA, 0x0004);
        lp->stats.errors_out++;
        lance_restart(lp, 0x0043, 1);
        lp->stats.errors_out++;
        lp->stats.tx_other_errors++;
    }
    return(TRUE);
}

int lance_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
PLANCE_PRIV lp;
int ioaddr;
int entry;
word w;
struct lance_tx_head *ph;
int length;

    lp = iface_to_lp(pi);
    if (!lp)
    {
        return(ENUMDEVICE); 
    }

    /* make sure packet is within legal size   */
    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)   
        length = ETHER_MIN_LEN;   

    if (length > ETHERSIZE)
    {
        DEBUG_ERROR("lance_xmit - length is too large", EBS_INT1, DCUTODATA(msg), length);
        DEBUG_ERROR("lance_xmit - pkt = ", PKT, DCUTODATA(msg), ETHERSIZE+4);
        length = ETHERSIZE;       /* what a terriable hack! */
    }

    ioaddr = lp->base_addr;

    /* Mask to ring buffer boundary.   */
    entry = lp->cur_tx & TX_RING_MOD_MASK;
    ph = lp->ptx_ring+entry;

    /* Caution: the write order is important here, set the base address
       with the "ownership" bits last. */
    ph->misc = 0;
    w = (word) -length;
    ph->length = SWAPIF(w);

#if (USE_LANCE_BUS_MASTER)
    /* In bus master mode point the lance's dma right at the core   */
    setup_lance_tx_buf(lp, entry, DCUTODATA(msg));
#endif
#if (USE_LANCE_SHARED_MEM)
    /* In shared memory mode just copy the data into shared memory   */
    tc_movebytes(lp->ptx_shared_mem[entry], DCUTODATA(msg), length);
#endif
    /* Now set the ownership to the lance   */
    w = SWAPIF(ph->status_hadrf);
    w = (word)(w | 0x8300);
    ph->status_hadrf = SWAPIF(w);

    lp->cur_tx++;

    /* Trigger an immediate send poll.   */
    OUTWORD(ioaddr+LANCE_ADDR, 0x0000);
    OUTWORD(ioaddr+LANCE_DATA, 0x0048);

    /* wait for send to complete   */
    return(0);
}

/* ********************************************************************   */
/* INTERRUPT routines.                                                    */
/* ********************************************************************   */
/* The LANCE interrupt handler.                                           */
#define ST_TINT 0x0200
#define ST_TERR 0x8000
#define ST_RINT 0x0400
#define ST_INE  0x0040
#define ST_INIT 0x0100

#define ST_BAB  0x4000
#define ST_CERR 0x2000
#define ST_MISS 0x1000
#define ST_MERR 0x0800


void lance_interrupt(int minor_no)    /*__fn__*/
{
PIFACE pi;
PLANCE_PRIV lp;
int csr0, boguscnt=10;
int must_restart;
word w;
word old_ptr;

    lp = off_to_lp(minor_no);
    if (!lp)
        return;

    pi = lp->iface;
    if (!pi)
        return;

    if (lp->in_interrupt)
    {
        DEBUG_ERROR("Re-entering the interrupt handler.", NOVAR, 0, 0);
    }

    lp->in_interrupt += 1;
    old_ptr = INWORD(lp->base_addr + LANCE_ADDR);

    OUTWORD(lp->base_addr + LANCE_ADDR, 0x00);

    csr0 = INWORD(lp->base_addr + LANCE_DATA);  /* PVOPVO */
    if (!(csr0 & (ST_TINT|ST_TERR|ST_RINT|ST_INIT)))
    {
        /* If its not a csr0 interrupt read csr4   */
        OUTWORD(lp->base_addr + LANCE_ADDR, 0x04);
        w = INWORD(lp->base_addr + LANCE_DATA);
        OUTWORD(lp->base_addr + LANCE_ADDR, 0x00);
    }

    /* **************************************************     */
    while ((csr0 = INWORD(lp->base_addr + LANCE_DATA)) & 0x8700 &&
           --boguscnt >= 0) 
    {
        /* Acknowledge all of the current interrupt sources ASAP.     */
/*        OUTWORD(lp->base_addr + LANCE_DATA, csr0 & ~0x004f);        */

        must_restart = 0;

#if (DEBUG_LANCE)
        DEBUG_ERROR("interrupt  csr0=; new csr=", DINT2,
               csr0, INWORD(lp->base_addr + LANCE_DATA));
#endif
        /* **************************************************      */
        /* Rx interrupt                                            */
        if (csr0 & 0x0400)          
        {
            lance_rx(lp);
            OUTWORD(lp->base_addr + LANCE_DATA, csr0 | (ST_INE|ST_RINT));
        }

        /* **************************************************      */
        /* Tx-done interrupt                                       */
        if (csr0 & 0x0200)   
        {
        int dirty_tx = lp->dirty_tx;

            while (dirty_tx < lp->cur_tx) 
            {
            int entry = dirty_tx & TX_RING_MOD_MASK;
             struct lance_tx_head *ph;
            word status;

                ph = lp->ptx_ring+entry;
                status = SWAPIF(ph->status_hadrf);   /* status in hi byte hadrf in low byte */
                if (status & 0x8000)
                    break;          /* It still hasn't been Txed */

                ph->status_hadrf = 0;       /* status in hi byte hadrf in low byte */
                if (status & 0x4000) 
                {
                    /* There was an major error, log it.     */
                    word err_status = SWAPIF(ph->misc);
                    lp->stats.errors_out++;
                    if (err_status & 0x0400) lp->stats.collision_errors++;
                    if (err_status & 0x0800) lp->stats.tx_carrier_errors++;
                    if (err_status & 0x1000) lp->stats.owc_collision++;
                    if (err_status & 0x4000) 
                    {
                        /* Ackk!  On FIFO errors the Tx unit is turned off!     */
                        lp->stats.tx_fifo_errors++;

                        /* Remove this verbosity later!                   */
/*                      DEBUG_ERROR("Tx FIFO error! Status = ", DINT1,    */
/*                             csr0, 0);                                  */

                        /* Restart the chip.     */
                        must_restart = 1;
                    }
                } 
                else 
                {
                    if (status & 0x1800)
                        lp->stats.one_collision++;
                    lp->stats.packets_out++;
                }

                dirty_tx++;
            }

#ifndef final_version
            if (lp->cur_tx - dirty_tx >= TX_RING_SIZE) 
            {
                DEBUG_ERROR("out-of-sync dirty pointer = vs. ", DINT2,
                       dirty_tx, lp->cur_tx);
                DEBUG_ERROR("out-of-sync: full = ", EBS_INT1, 
                       lp->tx_full, 0);
                dirty_tx += TX_RING_SIZE;
            }
#endif
            lp->dirty_tx = dirty_tx;

            /* signal xmit routine or IP task     */
            ks_invoke_output(pi, 1);
            OUTWORD(lp->base_addr + LANCE_DATA, csr0 | (ST_INE|ST_TINT));
        }       /* end of Tx-done interrupt */

        if (csr0 & ST_INIT)
        {
            OUTWORD(lp->base_addr + LANCE_DATA, csr0 | (ST_INE|ST_INIT));
        }


        if (csr0 & ST_TERR)
        {
            /* **************************************************      */
            /* Log misc errors.                                        */
            if (csr0 & 0x4000) lp->stats.errors_out++; /* Tx babble. */
            if (csr0 & 0x1000) lp->stats.errors_in++; /* Missed a Rx frame. */
            if (csr0 & 0x0800) 
            {
    /*          DEBUG_ERROR("Bus master arbitration failure, status = ",      */
    /*                 DINT1, csr0, 0);                                       */
                /* Restart the chip.                                          */
                must_restart = 1;
            }
            /* PVO 2-6-01 - clear the error by writing error bits back to csr0   */
            OUTWORD(lp->base_addr + LANCE_DATA, csr0 | (ST_INE|ST_BAB|ST_CERR|ST_MISS|ST_MERR));
        }

        /* **************************************************     */
        if (must_restart) 
        {
            /* stop the chip to clear the error condition, then restart     */
            OUTWORD(lp->base_addr + LANCE_ADDR, 0x0000);
            OUTWORD(lp->base_addr + LANCE_DATA, 0x0004);
            lance_restart(lp, 0x0002, 0);
            /* Clear any other interrupt, and set interrupt enable.     */
            OUTWORD(lp->base_addr + LANCE_ADDR, 0x0000);
            OUTWORD(lp->base_addr + LANCE_DATA, 0x7940);
        }
    }      /* end of while interrupt pending loop */

    /* Clear any other interrupt, and set interrupt enable.     */
/*    OUTWORD(lp->base_addr + LANCE_ADDR, 0x0000);              */
/*    OUTWORD(lp->base_addr + LANCE_DATA, 0x7940);              */

#if (DEBUG_LANCE)
    DEBUG_ERROR("exiting interrupt, csr", DINT2, INWORD(lp->base_addr + LANCE_ADDR),
           INWORD(lp->base_addr + LANCE_DATA));
#endif
    lp->in_interrupt -= 1;
    if (old_ptr != 0)
        OUTWORD(lp->base_addr + LANCE_ADDR, old_ptr);
    return;
}
static void lance_rx(PLANCE_PRIV lp)
{
PIFACE pi;
int entry = lp->cur_rx & RX_RING_MOD_MASK;
struct lance_rx_head *ph;
#if (USE_LANCE_BUS_MASTER)
DCU new_msg;
#endif
DCU msg;
        
    pi = lp->iface;
    if (!pi)
        return;

    /* If we own the next entry, it's a new packet. Send it up.                  */
/*  while (((ph = lp->prx_ring+entry)->status_hadrf & 0x8000) == 0)              */
    while ((SWAPIF((ph = lp->prx_ring+entry)->status_hadrf) & 0x8000) == 0)             
                /* peter; moved ph = lp->prx_ring+entry inside while loop   */
    {
/*  byte status = (byte)(ph->status_hadrf >> 8);   */
    byte status = (byte)(SWAPIF(ph->status_hadrf) >> 8);

        if (status != 0x03)             /* There was an error. */
        {
            /* There is a tricky error noted by John Murphy,
               <murf@perftech.com> to Russ Nelson: Even with full-sized
               buffers it's possible for a jabber packet to use two
               buffers, with only the last correctly noting the error. */
            if (status & 0x01)  /* Only count a general error at the */
                lp->stats.rx_other_errors++; /* end of a packet.*/
            if (status & 0x20) lp->stats.rx_frame_errors++;
            if (status & 0x10) lp->stats.rx_overwrite_errors++;
            if (status & 0x08) lp->stats.rx_crc_errors++;
            if (status & 0x04) lp->stats.rx_fifo_errors++;
/*          ph->status_hadrf |= 0x8000;                             // peter new code; to give buffer back to lance chip   */
            {
            word w;
                w = SWAPIF(ph->status_hadrf);
                w |= 0x8000;                                /* peter new code; to give buffer back to lance chip */
                ph->status_hadrf = SWAPIF(w);
            }
            give_lance_rx_buf(lp, entry);
        } 
        else 
        {
/*          word pkt_len = (word)((ph->msg_length & 0xfff)-4);   */
/*          pkt_len = SWAPIF(pkt_len);                           */
            word pkt_len = (word)((SWAPIF(ph->msg_length) & 0xfff)-4);

            if (pkt_len > ETHERSIZE)        /* tbd - read pkt */
            {
                DEBUG_ERROR("RCV bad length", EBS_INT1, 0, pkt_len);
#if (USE_LANCE_BUS_MASTER)
                /* Free the packet. Then reload the ring entry   */
                os_free_packet(lp->rx_dcus[entry]);
                lp->rx_dcus[entry] = 0; /* PETER - zero it */
                alloc_lance_rx_buf(lp, entry);
#endif
#if (USE_LANCE_SHARED_MEM)
                /* In shared memory mode just give the packet back to the controller   */
                give_lance_rx_buf(lp, entry);
#endif
                lp->stats.rx_frame_errors++;
                return;
            }
#if (USE_LANCE_BUS_MASTER)
            /* Allocate a new msg buffer. If we succeed we will   */
            /* give msg to the stack and thread new_msg into the  */
            /* ring when we call alloc_input                      */
            /* Otherwise the ring entry will be reset by the      */
            /* alloc call.                                        */
            new_msg = os_alloc_packet_input((ETHERSIZE+4), DRIVER_ALLOC);
            if (new_msg)
            {
                msg = lp->rx_dcus[entry];
                lp->rx_dcus[entry] = new_msg; 
            }
            else
                msg = 0;
#endif
#if (USE_LANCE_SHARED_MEM)
            /* grab a packet and copy the data   */
            msg = os_alloc_packet_input((ETHERSIZE+4), DRIVER_ALLOC);
            if (msg)
                tc_movebytes(DCUTODATA(msg), lp->prx_shared_mem[entry], pkt_len);
            else
            {
                DEBUG_ERROR("lance_rx: out of DCUs", NOVAR, 0, 0);
                lp->stats.packets_lost++;
            }
            give_lance_rx_buf(lp, entry);
#endif
            if (msg)
            {
                DCUTOPACKET(msg)->length = pkt_len;
                lp->stats.packets_in++;
                lp->stats.bytes_in += pkt_len;  
#if (RTIP_VERSION > 24)
                ks_invoke_input(pi, msg);   
                /* send to input list; will get sent to IP exchange   */
#else
                /* send to input list; will get sent to IP exchange   */
                os_sndx_input_list(pi, msg);
                ks_invoke_input(pi);   
#endif
            }

#if (USE_LANCE_BUS_MASTER)
            /* alloc_lance is really resetting the ring entry here.
              the memory for it is either new_msg or msg depending on
              if the alloc failed or not */
            alloc_lance_rx_buf(lp, entry);

            if (!new_msg)
            {
                DEBUG_ERROR("Memory squeeze, deferring packet.", NOVAR, 0, 0);
                lp->stats.packets_lost++;
            }
#endif
        }
        entry = (++lp->cur_rx) & RX_RING_MOD_MASK;
    }

    /* We should check that at least two ring entries are free.  If not,
       we should free one and mark stats->rx_dropped++. */
}

/* ********************************************************************   */
/* STATISTICS routine.                                                    */
/* ********************************************************************   */
RTIP_BOOLEAN lance_statistics(PIFACE pi)                       /*__fn__*/
{
PLANCE_PRIV lp;
PETHER_STATS p;
int ioaddr;
int saved_addr;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */

    lp = iface_to_lp(pi);
    if (!lp)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    ioaddr = lp->base_addr;

    if (lance_chip_table[lp->chip_version].flags & LANCE_HAS_MISSED_FRAME) 
    {
        ks_disable();
        saved_addr = INWORD(ioaddr+LANCE_ADDR);
        OUTWORD(ioaddr+LANCE_ADDR, 112);
        lp->stats.rx_other_errors = INWORD(ioaddr+LANCE_DATA);  /* missed errors */
        OUTWORD(ioaddr+LANCE_ADDR, saved_addr);
        ks_enable();
    }

    p = (PETHER_STATS) &(lp->stats);
    UPDATE_SET_INFO(pi,interface_packets_in, p->packets_in)
    UPDATE_SET_INFO(pi,interface_packets_out, p->packets_out)
    UPDATE_SET_INFO(pi,interface_bytes_in, p->bytes_in)
    UPDATE_SET_INFO(pi,interface_bytes_out, p->bytes_out)
    UPDATE_SET_INFO(pi,interface_errors_in, p->errors_in)
    UPDATE_SET_INFO(pi,interface_errors_out, p->errors_out)
    UPDATE_SET_INFO(pi,interface_packets_lost, p->packets_lost)
    return(TRUE);
}

/* ********************************************************************   */
/* MULTICAST routines.                                                    */
/* ********************************************************************   */
/* Set or clear the multicast filter for this adaptor.
   num_addrs == -1      Promiscuous mode, receive all packets
   num_addrs == 0       Normal mode, clear multicast list
   num_addrs > 0        Multicast mode, receive normal and MC packets, and do
                        best-effort filtering.
 */
RTIP_BOOLEAN lance_setmcast(PIFACE pi)      /* __fn__ */
{
PLANCE_PRIV lp;

    lp = iface_to_lp(pi);
    if (!lp)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    set_multicast_list(lp, pi->mcast.lenmclist, pi->mcast.mclist);
    return(TRUE);
}

static void set_multicast_list(PLANCE_PRIV lp, int num_addrs, void *addrs)
{
int ioaddr;

    ARGSUSED_PVOID(addrs);

    ioaddr = lp->base_addr;

    OUTWORD(ioaddr+LANCE_ADDR, 0);
    OUTWORD(ioaddr+LANCE_DATA, 0x0004); /* Temporarily stop the lance.   */

    if (num_addrs >= 0) 
    {
        int multicast_table[4];
        int i;

        /* We don't use the multicast table, but rely on upper-layer filtering.   */
        tc_memset(multicast_table, (num_addrs == 0) ? 0 : -1, sizeof(multicast_table));
        for (i = 0; i < 4; i++) 
        {
            OUTWORD(ioaddr+LANCE_ADDR, 8 + i);
            OUTWORD(ioaddr+LANCE_DATA, multicast_table[i]);
        }
        OUTWORD(ioaddr+LANCE_ADDR, 15);
        OUTWORD(ioaddr+LANCE_DATA, 0x0000); /* Unset promiscuous mode */
    } 
    else 
    {
        /* Log any net taps.   */
        DEBUG_ERROR("Promiscuous mode enabled.", NOVAR, 0, 0);
        OUTWORD(ioaddr+LANCE_ADDR, 15);
        OUTWORD(ioaddr+LANCE_DATA, 0x8000); /* Set promiscuous mode */
    }

    lance_restart(lp, 0x0142, 0); /*  Resume normal operation */

}

#if (USE_LANCE_BUS_MASTER)

#define DMA_8MASK_REG   0x0A   /*       ; system 2nd dma cntler mask reg */
#define DMA_16MASK_REG  0xD4  /*        ; system 1st dma cntler mask reg */
#define DMA_8MODE_REG   0x0B  /*        ; system 2nd dma cntler mode reg */
#define DMA_16MODE_REG  0xD6  /*        ; system 1st dma cntler mode reg */
#define DMA_CHL_FIELD   0x3    /*       ; dma channel fields bit 1:0 */
#define SET_DMA_MASK    0x4    /*       ; mask reg,bit 2: 0,clear/1,set */
#define SINGLE_MODE     0x40  /*        ; mode reg,bit 7,6: 01 single mode */
#define CASCADE_MODE    0xC0  /*        ; mode reg,bit 7,6: 11 cascade mode */

void dma_init(int channel)
{
#if (RTPCI_CONFIG_PCI)
#else
byte b;
    b = (byte) channel;
    if (channel <= 4)
    {
        b |= CASCADE_MODE;          /* set single mode bit */
        OUTBYTE(DMA_8MODE_REG,b);   /* write to 8 bit dma mode reg. */
        b &= DMA_CHL_FIELD;         /* set proper dma channel bit */
        OUTBYTE(DMA_8MASK_REG,b);   /* write to 8 bit dma mask reg. */
    }
    else
    {
        b &= DMA_CHL_FIELD;         /* set proper dma channel bit */
        OUTBYTE(DMA_16MASK_REG,b);  /* write to 16 bit dma mask reg */
        b |= CASCADE_MODE;          /* set single mode bit */
        OUTBYTE(DMA_16MODE_REG,b);  /* write to 16 bit dma mode reg. */
    }
#endif
}

#endif  /* (BUS MASTER) */
    
#endif  /* INCLUDE_LANCE */







