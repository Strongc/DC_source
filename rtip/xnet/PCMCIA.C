/*****************************************************************************
*Filename: PCMCIA.C - PCMCIA ATA management code.
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1996
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
*
*
* Description:
*
*   This code provides a logical access layer to the PCMCIA
*   card information structure. It is hardware independent.
*
*
*
****************************************************************************/


#include "rtip.h"

#if (INCLUDE_PCMCIA)
#include "pcmcia.h"

void pcmcia_dummy(void) /* force pcmcia to link in under some linkers */
{
}

/* BSS area used by the pcmcia system     */
extern int card_device_type[NSOCKET];



/* CIS tuples     */
#define CISTPL_NULL                 0x00
#define CISTPL_DEVICE               0x01
#define CISTPL_FUNCE_LAN_NODE_ID    0x04
#define CISTPL_LONG_LINK_A          0x11
#define CISTPL_LONG_LINK_C          0x12
#define CISTPL_LINKTARGET           0x13
#define CISTPL_NO_LINK              0x14
#define CISTPL_VERS_1               0x15
#define CISTPL_CONFIG               0x1A
#define CISTPL_MANFID               0x20
#define CISTPL_FUNCE                0x22
#define CISTPL_END                  0xff

typedef struct tuple_args {
    int  socket;            /* Logical socket number  */
    word  attributes;       /* If bit 0 is 1 return link information */
    byte    tuple_desired;      /* Which tupple is wanted 0xFF is wild */
    byte    tuple_offset;       /* For get_tuple_data */
    word  flags;                /* INTERNAL USE. Following values are defined */
#define TUFLG_NO_LNK    0x01    /* Found NOLINK tupple. long links not allowed */
#define TUFLG_L_LNK_C   0x02    /* link_offset is valid for common memory */
#define TUFLG_L_LNK_A   0x04    /* link_offset is valid for attribut memory */
#define TUFLG_COMMON    0x08    /* cis_offset points to common memory */
#define TUFLG_FIRST 0x10    /* This is a get first */
    dword   link_offset;        /* If there is a link this is it's address */
    dword   cis_offset;     /* Offset into the CIS for this tupple */
    byte    tuple_code;     /* Current tuple */
    byte    tuple_link;     /* Next tuple */
} TUPLE_ARGS;
typedef TUPLE_ARGS KS_FAR * PTUPLE_ARGS;

static RTIP_BOOLEAN tuple_get(PTUPLE_ARGS pt);
static RTIP_BOOLEAN tuple_next_tuple(int socket, PTUPLE_ARGS pt, byte *ptuple, byte *plink);
static RTIP_BOOLEAN tuple_map_link(int socket, PTUPLE_ARGS pt, dword KS_FAR *plink);
static RTIP_BOOLEAN tuple_next_chain(int socket, PTUPLE_ARGS pt, byte *ptuple, byte *plink);
static int tuple_copy_data(int socket, PTUPLE_ARGS pt, byte KS_FAR *pdata, word offset , int count);
static void tuple_map_tuple(int socket, PTUPLE_ARGS pt, byte *ptuple, byte *plink);
static int card_GetTupleData(PTUPLE_ARGS pt, byte KS_FAR *pdata, int count);
static RTIP_BOOLEAN card_GetFirstTuple(int socket, PTUPLE_ARGS pt);
static RTIP_BOOLEAN card_config_addr(int socket, int register_no, dword *offset);
static RTIP_BOOLEAN card_write_config_regs(int socket, int register_no, byte c);


static RTIP_BOOLEAN card_write_config_regs(int socket, int register_no, byte c)          /*__fn__*/
{
dword offset;

    if (card_config_addr(socket, register_no, &offset))
    {
        pcmctrl_put_cis_byte(socket, offset, c);
        return(TRUE);
    }
    else
        return(FALSE);
}







static RTIP_BOOLEAN card_config_addr(int socket, int register_no, dword *offset)  /*__fn__*/
{
TUPLE_ARGS t;
byte  buf[8];
int base_address_size;
dword base_address;
int mask_size;
int n;
word mask;
word offset_bit;

    t.tuple_offset = 0x00; t.tuple_desired = CISTPL_CONFIG;
    if (!card_GetFirstTuple(socket, &t))
        return(FALSE);

    /* Read the first 8 bytes from the CONIFG tuple. It is a variable length      */
    /* structure (this is insane). But it must be at least 4 bytes.               */
    if (card_GetTupleData((PTUPLE_ARGS)&t, (byte KS_FAR *) buf, 8) < 4)
        return(FALSE);
    base_address_size = (int) (buf[0] & 0x03) + 1;  /* Address size is bits 10 */
    mask_size = (int) (buf[0] & 0x3C);              /* Mask size is bits 5432 */

    /* mask_size >= 2;     */
    mask_size += 1;

    /* The base address is in  low byte to hibyte buf[2], buf[3] buf[4], buf[5].       */
    /* base address size determines which ones are significant                         */
    for (base_address = 0,n = base_address_size; n ; n--)
    { base_address <<= 8; base_address |= buf[n+1]; }

    mask = 0;       /* Only supporting two mask bytes for now */
    if (mask_size > 1) mask = (word) buf[2+base_address_size+1];
    mask |= (word) buf[2+base_address_size];

    /* Test if the register is correct       */
    offset_bit = 0;
    if (register_no < 16) offset_bit =1; offset_bit <<= register_no;
    if (!(offset_bit&mask))
        return(FALSE);


    /* divide the base address by two and add the offset.
        Do this because CIS memory is accessed on an every other
        byte basis so this value when passed to map_cis will work
    */
    base_address = base_address / 2;
    base_address = base_address + register_no;
    *offset = base_address;
    return(TRUE);

}



/* card_GetFirstTuple - Get the first tuple from the CIS which matches selection      */
/* criterria.                                                                         */
/*                                                                                    */
/* Inputs:                                                                            */
/*  PTUPLE_ARGS pt  Pointer to a tuple arg structure (see below)                      */
/*      These values must be supplied by the caller                                   */
/*  pt->attributes  - If bit 0 is 1 link information is returned                      */
/*  pt->tuple_desired  - Which tupple. 0 - 0xFF. If any tuple desired                 */
/*                          set to 0xFF. Note 0xff does not alter the meaning         */
/*                          of attributes bit.                                        */
/*                                                                                    */
/* Outputs:                                                                           */
/*  PTUPLE_ARGS pt  Pointer to a tuple arg structure (see below)                      */
/*      If TRUE is returned the following fields will be valid                        */
/*      pt->tuple_code  - Tuple code found                                            */
/*      pt->tuple_link  - Link to next tuple. Also ==s the size of the                */
/*                          data field for this tuple.                                */
/* Returns:                                                                           */
/*  TRUE        - The query was succesful, pt is filled in                            */
/*  FALSE           - The query failed                                                */
/*                                                                                    */
/*  Note:                                                                             */
/*  If Get first returns success the pt structure may be passed to                    */
/*  card_GetNextTuple to get the next tuple and or card_GetTupleData                  */
/*  to read the data field of the tuple.                                              */
/*                                                                                    */

static RTIP_BOOLEAN card_GetFirstTuple(int socket, PTUPLE_ARGS pt) /* __fn__ */
{
    pt->socket = socket;
    pt->flags = TUFLG_FIRST;
    return((RTIP_BOOLEAN)tuple_get(pt));
}

/* card_GetNextTuple - Get the next tuple from the CIS which matches selection      */
/* criteria in pt->tuple_desired                                                    */
/*                                                                                  */
/* Inputs:                                                                          */
/*  PTUPLE_ARGS pt  - tuple arg structure returned from card_GetFirstTuple          */
/*      pt->tuple_offset - Set this to zero                                         */
/* Outputs:                                                                         */
/*  PTUPLE_ARGS pt  - tuple arg structure (see card_GetFirstTuple)                  */
/* Returns:                                                                         */
/*                                                                                  */
/*  TRUE        - The query was succesful, pt is filled in                          */
/*  FALSE           - The query failed                                              */
/*                                                                                  */
/*  Note:                                                                           */
/*  card_GetFirstTuple must have been called first. Call card_GetTupleData          */
/*  to read the data field of the tuple. To scan all tuples in the CIS, first       */
/*  call card_GetFirstTuple() with tuple_desired set to 0xff. If link fields        */
/*  are of interest set pt->attributes to 0x0001.                                   */

/* static RTIP_BOOLEAN card_GetNextTuple(PTUPLE_ARGS pt)
{
    return(tuple_get(pt));
}
*/


/* card_GetTupleData - Get the data from the current tuple                          */
/*                                                                                  */
/*                                                                                  */
/* Inputs:                                                                          */
/*  PTUPLE_ARGS pt  - tuple arg structure returned from card_GetFirstTuple          */
/*  or GetFirstTuple().                                                             */
/*      You may change the following value:                                         */
/*      pt->tuple_offset - Set this value to zero to get all data. Or set it        */
/*                      to the offset into the data field to cop first.             */
/*  byte KS_FAR *pdata      - Destination address for the data                      */
/*  int count           - Number of bytes to copy (0-255). Pass in                  */
/*                          pt->tuple_link to get all bytes.                        */
/* Outputs:                                                                         */
/*  The tuple's data is copied into the buffer at pdata.                            */
/* Returns:                                                                         */
/*  The number of bytes copied. This will equal the input 'count'                   */
/*  or pt->tuple_link-pt->tuple_offset whichever is smaller.                        */
/*                                                                                  */

static int card_GetTupleData(PTUPLE_ARGS pt, byte KS_FAR *pdata, int count) /* __fn__ */
{
    return(tuple_copy_data(pt->socket, pt, pdata, pt->tuple_offset, count));
}


static void tuple_map_tuple(int socket, PTUPLE_ARGS pt, byte *ptuple, byte *plink) /* __fn__ */
{
    *ptuple = pcmctrl_get_cis_byte(socket, (int)pt->cis_offset);
    *plink  = pcmctrl_get_cis_byte(socket, (int)(pt->cis_offset+1));
}


static int tuple_copy_data(int socket, PTUPLE_ARGS pt, byte KS_FAR *pdata, word offset , int count) /*__fn__*/
{
byte currtuple,currlink;
int i;
int cis_offset;

    cis_offset = (int)pt->cis_offset+2;     /* Offset of data past tupple and link */
    cis_offset += (int) offset;
    tuple_map_tuple(socket, pt, &currtuple, &currlink);
    if (count > (int) currlink)
        count = (int) currlink;
    for (i = 0; i < count; i++)
        *pdata++ = pcmctrl_get_cis_byte(socket, cis_offset++);
    return(count);
}

static RTIP_BOOLEAN tuple_next_chain(int socket, PTUPLE_ARGS pt, byte *ptuple, byte *plink)  /*__fn__*/
{
byte currtuple,currlink;
byte buffer[3];

    if (!(pt->flags & (TUFLG_L_LNK_A|TUFLG_L_LNK_C)))
        return(FALSE);      /* No links in the previous chain */
    if (pt->flags & TUFLG_NO_LNK)
        return(FALSE);      /* No links allowed */

    pt->cis_offset = (int)pt->link_offset;  /* Copy the new offset */
    if (pt->flags & TUFLG_L_LNK_C)      /* Note if in common mem */
        pt->flags |= TUFLG_COMMON;
                                        /* Clear the link info     */
    pt->flags &= (word) ~(TUFLG_L_LNK_A|TUFLG_L_LNK_C);
    /* map in the new tupple. check if it is a valid link        */
    /* target. If so see if the user wants it. If any part       */
    /* fails we fall through.                                    */
    tuple_map_tuple(socket, pt, &currtuple, &currlink);
    if (currtuple == CISTPL_LINKTARGET)
    {
        if (tuple_copy_data(socket, pt, buffer, 0, 3) == 3)
        {
            if (buffer[0]=='C'&&buffer[1]=='I'&&buffer[2]=='S')
            {
                *ptuple = currtuple;
                *plink  = currlink;
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

static RTIP_BOOLEAN tuple_map_link(int socket, PTUPLE_ARGS pt, dword KS_FAR *plink) /* __fn__ */
{
dword ultemp;
byte buffer[4];
*plink = 0;

    if (tuple_copy_data(socket, pt, buffer, 0, 4) == 4)
    {
        ultemp = 0;
        ultemp |= buffer[3]; ultemp <<= 8;
        ultemp |= buffer[2]; ultemp <<= 8;
        ultemp |= buffer[1]; ultemp <<= 8;
        ultemp |= buffer[0];
        *plink = ultemp;
        return(TRUE);
    }
    return(FALSE);
}


/*
static int tuple_get_data(PTUPLE_ARGS pt, byte KS_FAR *pdata, int count)
{
    return(tuple_copy_data(pt->socket, pt, pdata, 0, count));
}
*/

static RTIP_BOOLEAN tuple_next_tuple(int socket, PTUPLE_ARGS pt, byte *ptuple, byte *plink) /*__fn__*/
{
byte currtuple,currlink;
    tuple_map_tuple(socket, pt, &currtuple, &currlink);
    /* If at the end of a chain see if there is a long link to another chain     */
    if ( (currtuple == CISTPL_END) || (currlink == 0xff) )
        return(tuple_next_chain(socket, pt, ptuple, plink));
    else
    {
        pt->cis_offset += 1;        /* Skip past the tuple */
        if (currtuple != CISTPL_NULL)
        {
            pt->cis_offset += 1;            /* Past link */
            pt->cis_offset += currlink; /* Past data */
        }
        tuple_map_tuple(socket, pt, ptuple, plink); /* Get the values */
        return(TRUE);
    }
}

static RTIP_BOOLEAN tuple_get(PTUPLE_ARGS pt) /* __fn__ */
{
byte tuple;byte link;

    /* Clear return values     */
    pt->tuple_code  = pt->tuple_link =  0;
    pt->tuple_offset =  0;
    /* If starting go back to square one     */
    if (pt->flags & TUFLG_FIRST)
    {
        pt->flags       = 0;    /* Clear all flags */
        pt->link_offset  = pt->cis_offset = 0L;

        tuple_map_tuple(pt->socket, pt, &tuple, &link);
        /* If just starting make sure we have a CIS     */
        if (tuple != 0x01)
        {  /* !!!!!!!!! This needs fixing for special cases */
            return(FALSE);
        }
    }
    else
    {
        if (!tuple_next_tuple(pt->socket, pt, &tuple, &link))
            return(FALSE);
    }

    while (1)
    {
        /* handle special cases     */
        switch(tuple) {
            case CISTPL_NO_LINK:
                pt->flags |= TUFLG_NO_LNK;
                break;
            case CISTPL_LONG_LINK_A:
                if (tuple_map_link(pt->socket, pt, &pt->link_offset)) /* read the link */
                    pt->flags |= TUFLG_L_LNK_A;
                break;
            case CISTPL_LONG_LINK_C:
                if (tuple_map_link(pt->socket, pt, &pt->link_offset))  /* read the link */
                    pt->flags |= TUFLG_L_LNK_C;
                break;
            default:
                break;
        }
        /* Now if we have a match, take it.      */
        if (pt->tuple_desired == 0xff || pt->tuple_desired == tuple)
        {
            switch (tuple) {
                case CISTPL_NO_LINK:    /* Only return link info if requested */
                case CISTPL_LONG_LINK_A:
                case CISTPL_LONG_LINK_C:
                case CISTPL_LINKTARGET:
                    if (!(pt->attributes & 0x01))
                        break;
                    /* Falls through on a hit.     */
                default:
                    pt->tuple_code = tuple;
                    pt->tuple_link = link;
                    return(TRUE);
            }
        }
        if (!tuple_next_tuple(pt->socket, pt, &tuple, &link))
            return(FALSE);
    }
}


/* RTIP_BOOLEAN pcmcia_card_is_sram(int socket) Return TRUE if there is an ATA card in the socket      */
/*  Inputs:                                                                                            */
/*      Logical socket number                                                                          */
/*                                                                                                     */
/*  Returns:                                                                                           */
/*  TRUE    If an ATA device is installed.                                                             */
/*  FALSE       If an ATA device is not installed.                                                     */
/*                                                                                                     */

RTIP_BOOLEAN pcmcia_card_is_sram(int socket)                     /*__fn__*/
{
    return(TRUE);
}


/* RTIP_BOOLEAN pcmcia_card_is_ata(int socket) Return TRUE if there is an ATA card in the socket      */
/*  Inputs:                                                                                           */
/*      Logical socket number                                                                         */
/*                                                                                                    */
/*  Returns:                                                                                          */
/*  TRUE    If an ATA device is installed.                                                            */
/*  FALSE       If an ATA device is not installed.                                                    */
/*                                                                                                    */

RTIP_BOOLEAN pcmcia_card_is_ata(int socket,
        IOADDRESS register_file_address,
        int interrupt_number,      /* note -1 is polled for IDE */
        byte pcmcia_cfg_opt_value)  /*__fn__*/
{
TUPLE_ARGS t;
byte c;
word w;
int device_type;
RTIP_BOOLEAN is_ata;

    /* Get Device Information     */
    is_ata = FALSE;

    t.tuple_desired = CISTPL_DEVICE;
    if (card_GetFirstTuple(socket, &t))
    {
        /* Get the Device type     */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (byte KS_FAR *) &c, 1)==1)
        {
            device_type = (int)(c>>4);
            if (device_type == 0x0D /* DTYPE_FUNCSPEC */)
            {
                /* Call CISTPL_FUNCE. First 2 bytes should be 0x0101.          */
                /* We look at as a word. Since its 0x0101 it is portable       */
                /* with respect to byte order                                  */
                t.tuple_offset = 0x00;
                t.tuple_desired = CISTPL_FUNCE;
                if( card_GetFirstTuple(socket, &t) &&
                (card_GetTupleData((PTUPLE_ARGS)&t,(byte KS_FAR *) &w, 2)==2)
                && (w == (word) 0x0101))
                {
                    /* Put the chip into IO mode, map in the ATA register bank and         */
                    /* enable the interrupt (if interrupt_no is -1 the interrupt is        */
                    /* not enabled but everything else is done)                            */

                    /* Write 0:0 to Socket copy register SOCK_COPY     */
                    if (card_write_config_regs(socket, 3 /*SOCK_COPY_REGISTER*/, 0))
                    {
                    /* Set the Configuration option register.     */
                        if (card_write_config_regs(socket, 0/* CONFIG_OPTION_REGISTER */,pcmcia_cfg_opt_value))
                        {
                            /* Set up io windows, enable interrupts, power to Vpp     */
                            pcmctrl_map_ata_regs(socket, register_file_address, interrupt_number);
                            is_ata = TRUE;
                        }
                    }
                }
            }
        }
    }
    return(is_ata);
}




#if (INCLUDE_3C589)

/* RTIP_BOOLEAN pcmcia_card_is_3com(int socket, int ioaddr, int irq)      */
/* Return TRUE if there is a linksys card                                 */
/*  Inputs:                                                               */
/*      Logical socket number                                             */
/*                                                                        */
/*  Returns:                                                              */
/*  TRUE    If an Linksys device is installed.                            */
/*  FALSE       If an Linksys device is not installed.                    */
/*                                                                        */
void pcmctrl_map_linksys_regs(int socket,int irq, int iobase);
static RTIP_BOOLEAN pcmcia_linksys_mode(int socket, int irq, int iobase);

word _3c589_read_eeprom(short ioaddr, int index);
void _3c589_setel3_window(short ioaddr, int window);

RTIP_BOOLEAN pcmcia_card_is_3com(int socket, int irq, int iobase, byte *phys_addr)/*__fn__*/
{
TUPLE_ARGS t;
byte buf[64];
int multi;
int i;

    /* Get manufactures ID info     */
    t.tuple_offset = 0x00; t.tuple_desired = CISTPL_MANFID;
    if (!card_GetFirstTuple(socket, &t))
        return(FALSE);
    if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 64) < 4)
        return(FALSE);
    if (buf[0] != 0x01 || buf[1] != 0x01)
        return(FALSE);
    if (buf[2] == 0x62 && buf[3] == 0x05)
    {
        DEBUG_ERROR("pcmcia_card_is_3com: Got 3c562", NOVAR, 0, 0);
        multi = 1;
    }
    else
        multi = 0;
    pcmctrl_map_3com(socket, irq, iobase);
    /* Apply Vpp power (does nothing since we use auto Vpp)     */
    pcmctrl_device_power_up(socket);
    /* Set the Configuration option register.     */
    if (!card_write_config_regs(socket, 0/* CONFIG_OPTION_REGISTER */, 0x41))
    {
        return(FALSE);
    }

    /*
#define EL3_CMD     0x0e
SelectWindow = 1<<11
#define EL3WINDOW(win_num) outw(SelectWindow + (win_num), ioaddr + EL3_CMD)

*/
/*     EL3WINDOW(0); --> out(iobase + 0x0e, (1 << 11) );      */
/*  outpw((iobase + 0x0e), (1 << 11) );                       */

    _3c589_setel3_window((short)iobase, 0);

    /* The 3c589 has an extra EEPROM for configuration info, including
       the hardware address.  The 3c562 puts the address in the CIS. */

    if (multi)
    {
        t.tuple_offset = 0x00; t.tuple_desired = 0x88;
        if (!card_GetFirstTuple(socket, &t))
            return(FALSE);
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 64) < 4)
            return(FALSE);
        for (i = 0; i < 6; i++)
            phys_addr[i] = buf[i];
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            word n;
            n = (word)_3c589_read_eeprom((short)iobase, i);
            phys_addr[(i*2)+1] = (byte)(n & 0xff);
            phys_addr[(i*2)] = (byte)((n >> 8) & 0xff);
        }
    }
    return(TRUE);
}

RTIP_BOOLEAN card_is_3com(int socket, int irq, int iobase, byte *phys_addr) /*__fn__*/
{
    /* Make sure the pcmcia controller is alive     */
    if (!pcmctrl_init())
        return(FALSE);
    if (card_device_type[socket] == PCMCIA_3C589_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    if (pcmctrl_card_installed(socket) && pcmcia_card_is_3com(socket, irq, iobase, phys_addr))
    {
        card_device_type[socket] = PCMCIA_3C589_DEVICE;
        return(TRUE);
    }
    else
        return(FALSE);
}


#endif      /* 3C589 */



#if (INCLUDE_XIRCOM)

#include "xircom.h"

static RTIP_BOOLEAN check_if_xircom(PIFACE pi, int socket);
static int has_ce2_string(int socket);
static int xircom_set_card_type( PIFACE pi, const void *s, int socket );
static RTIP_BOOLEAN pcmcia_xircom_mode(int socket, int irq, int iobase);

RTIP_BOOLEAN pcmcia_card_is_xircom(PIFACE pi, int socket, int irq, int iobase)
{
TUPLE_ARGS t;
byte buf[64];

    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    /* Check if a card is installed and powered up     */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(FALSE);
    }
    if (check_if_xircom(pi,socket))
    {
        t.tuple_offset = 0x00;
        t.tuple_desired = CISTPL_FUNCE;

        /* get functional extension subcode 4 which contains the
        ethernet address.
        Format is:
        0x22 0x08 0x04 0x6 xx xx xx xx xx xx
        where xx xx is the NIC address
        */
        if( card_GetFirstTuple(socket, &t))
        {
            do
            {
                if (card_GetTupleData((PTUPLE_ARGS)&t,(PFBYTE) buf, 8)==8)
                {
                    if (buf[0] == CISTPL_FUNCE_LAN_NODE_ID)
                    {
                        pi->addr.my_hw_addr[0] = buf[2];
                        pi->addr.my_hw_addr[1] = buf[3];
                        pi->addr.my_hw_addr[2] = buf[4];
                        pi->addr.my_hw_addr[3] = buf[5];
                        pi->addr.my_hw_addr[4] = buf[6];
                        pi->addr.my_hw_addr[5] = buf[7];
                        return(pcmcia_xircom_mode(socket, irq, iobase));
                    }
                }
            } while (tuple_get((PTUPLE_ARGS)&t));
        }
    }

    return (FALSE);
}

static RTIP_BOOLEAN pcmcia_xircom_mode(int socket, int irq, int iobase)  /* __fn__ */
{
    /* Set up io windows, enable interrupts, power to Vpp     */
    pcmctrl_map_linksys_regs(socket, irq, iobase);

    /* Set the Configuration option register.     */
    if (!card_write_config_regs(socket, 0 /* CONFIG_OPTION_REGISTER */, 0x41))
    {
        return(FALSE);
    }

    return(TRUE);
}

static RTIP_BOOLEAN check_if_xircom(PIFACE pi, int socket)
{
TUPLE_ARGS t;
byte buf[64];

    t.tuple_offset = 0;
    t.tuple_desired = CISTPL_MANFID;

    if (!card_GetFirstTuple(socket, &t))
        return (FALSE);

    if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 64) < 4)
        return (FALSE);

    if (!xircom_set_card_type(pi, buf, socket))
        return (FALSE);

    return (TRUE);
}

static int has_ce2_string(int socket)
{
TUPLE_ARGS t;
byte buf[256];
byte *p;

    t.tuple_offset = 0;
    t.tuple_desired = CISTPL_VERS_1;

    if (!card_GetFirstTuple(socket, &t))
        return 0;

    if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 254) < 4)
        return 0;

    p = &buf[2];
    if (tc_strstr((PFCHAR)p,"CE2"))
        return 1;

    return 0;
}

static int xircom_set_card_type(PIFACE pi, const void *s, int socket)
{
PDEVTABLE dev       = pi->pdev;
PXIRCOM_SOFTC local = &xircomssoftc[dev->minor_number];
byte mediaid        = ((const byte *)s)[3];
byte prodid         = ((const byte *)s)[4];

    local->mohawk = 0;
    local->dingo = 0;
    local->modem = 0;
    local->card_type = XIR_UNKNOWN;

    if( !(prodid & 0x40) )
        return 0;

    if (!(mediaid & 0x01))
        return 0;

    if( mediaid & 0x10 )
    {
        local->modem = 1;
        switch( prodid & 15 ) {
            case 1:
                        local->card_type = XIR_CEM;
                        break;
            case 2:
                        local->card_type = XIR_CEM2;
                        break;
            case 3:
                        local->card_type = XIR_CEM3;
                        break;
            case 4:
                        local->card_type = XIR_CEM33;
                        break;
            case 5:
                        local->card_type = XIR_CEM56M;
                        local->mohawk = 1;
                        break;
            case 6:
            case 7: /* 7 is the RealPort 10/56 */
                        local->card_type = XIR_CEM56 ;
                        local->mohawk = 1;
                        local->dingo = 1;
                        break;
        }
    }
    else
    {
        switch( prodid & 15 )
        {
            case 1:
                        local->card_type = has_ce2_string(socket)? XIR_CE2 : XIR_CE;
                        break;
            case 2:
                        local->card_type = XIR_CE2;
                        break;
            case 3:
                        local->card_type = XIR_CE3;
                        local->mohawk = 1;
                        break;
        }
    }

    if( local->card_type == XIR_CE || local->card_type == XIR_CEM )
        return 0;

    return 1;
}

#endif


#if (INCLUDE_SMC8041_PCMCIA)

static RTIP_BOOLEAN pcmcia_smc8041_mode (int socket, int irq, int iobase);
RTIP_BOOLEAN        check_if_smc8041    (int socket);


/* ********************************************************************         */
/*                                                                              */
/* Type:        RTIP_BOOLEAN                                                    */
/* Routine:     check_if_smc8041                                                */
/* Description: Return true if it is a smc8041 card                             */
/* Input:       socket - the socket number the card is located                  */
/* Output:      True   - if the card is found                                   */
/*              False  - if the card is not found                               */
/*                                                                              */
/* ********************************************************************         */
RTIP_BOOLEAN check_if_smc8041(int socket)
{
TUPLE_ARGS t;
byte buf[30];
byte *p;

    /* Get Device Information     */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* Get the Device Information     */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 30))
        {
            p = &buf[2];
            if (tc_stricmp((PFCHAR)p, (PFCHAR)"SMC") == 0)
            {
                p = &buf[6];
                if (tc_strstr((PFCHAR)p, (PFCHAR)"8041TX"))
                {
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}



/* ********************************************************************         */
/*                                                                              */
/* Type:        RTIP_BOOLEAN                                                    */
/* Routine:     pcmcia_card_is_smc8041                                          */
/* Description: get the ethernet address and set up io windows, enable          */
/*              interrupts, power to Vpp, then set the Configuration            */
/*              option register.                                                */
/* Input:                                                                       */
/* Output:      True   - if the card is found and setup                         */
/*              False  - if the card is not found and setup                     */
/*                                                                              */
/* ********************************************************************         */
RTIP_BOOLEAN pcmcia_card_is_smc8041(int socket, int irq, int iobase)                                 /*__fn__*/
{

    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    /* Check if a card is installed and powered up     */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(FALSE);
    }

    if (check_if_smc8041(socket))
        return(pcmcia_smc8041_mode(socket, irq, iobase));
    else
        return(FALSE);
}


/* ********************************************************************         */
/*                                                                              */
/* Type:        static RTIP_BOOLEAN                                             */
/* Routine:     pcmcia_smc8041_mode                                             */
/* Description: Put the chip into I) mode, map in the SMC register bank         */
/*              and enable the interrupt.  Called by                            */
/*              pcmcia_card_is_smc8041().  May also be called from              */
/*              external code.  NOTE: THIS IS FOR THE SMC8041 DRIVER            */
/* Input:                                                                       */
/* Output:                                                                      */
/*                                                                              */
/* ********************************************************************         */
static RTIP_BOOLEAN pcmcia_smc8041_mode(int socket, int irq, int iobase)
{
    /* Set up io windows, enable interrupts, power to Vpp     */
    pcmctrl_map_linksys_regs(socket, irq, iobase);

    /* Set the Configuration option register.                                   */
    /* For some reason the smc requires this to be done after IO mode           */
    if (!card_write_config_regs(socket, 0/* CONFIG_OPTION_REGISTER */, 0x20))
    {
        return(FALSE);
    }
    return(TRUE);
}
#endif


#if (INCLUDE_NE2000)

/* local functions     */
static RTIP_BOOLEAN pcmcia_linksys_mode(int socket, int irq, int iobase);

RTIP_BOOLEAN check_if_linksys(int socket)                /*__fn__*/
{
TUPLE_ARGS t;
byte buf[30];
byte *p;

    /* Get Device Information     */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* Get the Device Information     */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 30))
        {
            p = &buf[2];
            *(p+7) = 0;
            if (tc_stricmp((PFCHAR)p, (PFCHAR)"LINKSYS") == 0)
            {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

RTIP_BOOLEAN pcmcia_card_is_linksys(int socket, int irq, int iobase)                                 /*__fn__*/
{

    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    /* Check if a card is installed and powered up     */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(FALSE);
    }

    if (check_if_linksys(socket))
        return(pcmcia_linksys_mode(socket, irq, iobase));
    else
        return(FALSE);
}


RTIP_BOOLEAN card_is_ne2000(int socket, int irq, int iobase) /*__fn__*/
{
    /* Make sure the pcmcia controller is alive     */
    if (!pcmctrl_init())
        return(FALSE);
    if (card_device_type[socket] == PCMCIA_LINKSYS_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
#if (INCLUDE_SMC8041_PCMCIA)
    if ( pcmctrl_card_installed(socket) &&
        (pcmcia_card_is_linksys(socket, irq, iobase) ||
         pcmcia_card_is_smc8041(socket, irq, iobase)) )
#else
    if ( pcmctrl_card_installed(socket) &&
         pcmcia_card_is_linksys(socket, irq, iobase) )
#endif
    {
        card_device_type[socket] = PCMCIA_LINKSYS_DEVICE;
        return(TRUE);
    }
    else
        return(FALSE);
}


#endif /* NE2000 */


#if (INCLUDE_NE2000 || INCLUDE_SMC91C9X)
/* Put the chip into IO mode, map in the SMC or NE2000 register bank and       */
/* enable the interrupt                                                        */
/*  Called by pcmcia_card_is_linksys(). May also be called from external       */
/*  code                                                                       */
/*  NOTE: this is for both linksys and SMC drivers                             */
static RTIP_BOOLEAN pcmcia_linksys_mode(int socket, int irq, int iobase)  /* __fn__ */
{
    /* Set up io windows, enable interrupts, power to Vpp     */
    pcmctrl_map_linksys_regs(socket, irq, iobase);

    /* Set the Configuration option register.                                   */
    /* For some reason the linksys requires this to be done after IO mode       */
    if (!card_write_config_regs(socket, 0/* CONFIG_OPTION_REGISTER */, 0x41))
    {
        return(FALSE);
    }
    return(TRUE);
}
#endif


#if (INCLUDE_PRISM_PCMCIA)
void         pcmctrl_map_prism_regs(int socket,int irq, int iobase);
RTIP_BOOLEAN pcmcia_card_is_prism(int socket, int irq, int iobase, PFBYTE phys_addr);


/* ********************************************************************         */
/*                                                                              */
/* Type: static RTIP_BOOLEAN                                                    */
/* Routine: pcmcia_prism_mode                                                   */
/* Description:                                                                 */
/* Input:                                                                       */
/* Output:                                                                      */
/*                                                                              */
/* ********************************************************************         */

static RTIP_BOOLEAN pcmcia_prism_mode(int socket, int irq, int iobase)  /* __fn__ */
{
    /* ------------------------       */
    /*  Set up io windows,            */
    /*  enable interrupts,            */
    /*  power to Vpp                  */
    /* ------------------------       */
    pcmctrl_map_prism_regs(socket, irq, iobase);

    /* ------------------------       */
    /*  Set the Configuration         */
    /*  option register.              */
    /* ------------------------       */
    if (!card_write_config_regs(socket, 0/* CONFIG_OPTION_REGISTER */, 0x41))
    {
        return(FALSE);
    }
    return(TRUE);
}


/* ********************************************************************         */
/*                                                                              */
/* Type: RTIP_BOOLEAN                                                           */
/* Routine: check_if_symbol                                                     */
/* Description: Return true if it is a symbol card                              */
/* Input:                                                                       */
/* Output:                                                                      */
/*                                                                              */
/* ********************************************************************         */

RTIP_BOOLEAN check_if_symbol(int socket)
{
TUPLE_ARGS t;
byte buf[50];
byte *p;

    /* ------------------------       */
    /*  Get Device Information        */
    /* ------------------------       */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* ------------------------       */
        /*   Get the Device               */
        /*   Information                  */
        /* ------------------------       */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 30))
        {
            /* ------------------------       */
            /*  Support has been added        */
            /*  for the Symbol Spectrum24     */
            /* ------------------------       */
            p = &buf[2];
            if (tc_strcmp((PFCHAR)p, (PFCHAR)"Symbol Technologies") == 0)
            {
                p = &buf[22];   /* ex. "LA4111 S" */
                if (tc_strstr((PFCHAR)p, (PFCHAR)"LA"))
                    return(TRUE);
            }
        }
    }
    return(FALSE);
}


/* ********************************************************************         */
/*                                                                              */
/* Type: RTIP_BOOLEAN                                                           */
/* Routine: check_if_agere                                                      */
/* Description: Return true if it is an agere card                              */
/* Input:                                                                       */
/* Output:                                                                      */
/*                                                                              */
/* ********************************************************************         */

RTIP_BOOLEAN check_if_agere(int socket)
{
TUPLE_ARGS t;
byte buf[50];
byte *p;

    /* ------------------------       */
    /*  Get Device Information        */
    /* ------------------------       */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* ------------------------       */
        /*   Get the Device               */
        /*   Information                  */
        /* ------------------------       */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 30))
        {
            p = &buf[2];
            if (tc_strcmp((PFCHAR)p, (PFCHAR)"Lucent Technologies") == 0)
            {
                p = &buf[22];
                if (tc_strstr((PFCHAR)p, (PFCHAR)"WaveLAN"))
                    return(TRUE);
            }
        }
    }
    return(FALSE);
}

/* ********************************************************************         */
/*                                                                              */
/* Type: RTIP_BOOLEAN                                                           */
/* Routine: check_if_intersil                                                   */
/* Description: Return true if it is an intersil card                           */
/* Input:                                                                       */
/* Output:                                                                      */
/*                                                                              */
/* ********************************************************************         */

RTIP_BOOLEAN check_if_intersil(int socket)
{
TUPLE_ARGS t;
byte buf[50];
byte *p;

    /* ------------------------       */
    /*  Get Device Information        */
    /* ------------------------       */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* ------------------------       */
        /*   Get the Device               */
        /*   Information                  */
        /* ------------------------       */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 30))
        {
            /* ------------------------       */
            /*  Support has been added        */
            /*  for the ActionTec model       */
            /*  number:  HWC01170-01          */
            /*  and the Netgear MA401         */
            /* ------------------------       */
            p = &buf[2];
            if (tc_strstr((PFCHAR)p, (PFCHAR)"Intersil"))   /* __st__ TBD update */
            {
                return(TRUE);
            }
            else if (tc_strcmp((PFCHAR)p, (PFCHAR)"ACTIONTEC") == 0)
            {
                p = &buf[12];   /* ex. "PRISM Wireless LAN" */
                if (tc_strstr((PFCHAR)p, (PFCHAR)"PRISM"))
                    return(TRUE);
            }
            else if (tc_strstr((PFCHAR)p, (PFCHAR)"NETGEAR"))
            {                   /* ex. "NETGEAR MA401RA Wireless PC" */
                if (tc_strstr((PFCHAR)p, (PFCHAR)"MA"))
                    return(TRUE);
            }
        }
    }
    return(FALSE);
}


/* ********************************************************************         */
/*                                                                              */
/* Type: int                                                                    */
/* Routine: pcmcia_card_is_prism                                                */
/* Description: get the ethernet address and set up io windows, enable          */
/*              interrupts, power to Vpp, then set the Configuration            */
/*              option register.                                                */
/* Input:                                                                       */
/* Output: return PCMCIA_SYMBOL_DEVICE if symbol                                */
/*         return PCMCIA_INTERSIL_DEVICE if intersil                            */
/*         return PCMCIA_AGERE_DEVICE if agere                                  */
/*         return -1 if fail                                                    */
/*                                                                              */
/* ********************************************************************         */

int pcmcia_card_is_prism(int socket, int irq, int iobase, PFBYTE phys_addr)               /*__fn__*/
{
int card_type = -1;
TUPLE_ARGS t;
byte buf[30];


    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(card_type);

    /* ------------------------       */
    /*   Check if a card is           */
    /*   installed and powered        */
    /*   up                           */
    /* ------------------------       */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(card_type);
    }

    if (check_if_symbol(socket))
        card_type = PCMCIA_SYMBOL_DEVICE;
    else if (check_if_intersil(socket))
        card_type = PCMCIA_INTERSIL_DEVICE;
    else if (check_if_agere(socket))
        card_type = PCMCIA_AGERE_DEVICE;

    /* ------------------------       */
    /*  Get Device Information        */
    /* ------------------------       */
    if (card_type != -1)
    {
        t.tuple_offset = 0x00;
        t.tuple_desired = CISTPL_FUNCE;

        /* ------------------------       */
        /* get functional extension       */
        /* subcode 4 which contains       */
        /* the ethernet address.          */
        /* ------------------------       */

        /* -------------------------------------       */
        /* Format is:                                  */
        /* 0x22 0x08 0x04 0x6 xx xx xx xx xx xx        */
        /* where xx xx is the NIC address              */
        /* -------------------------------------       */
        if( card_GetFirstTuple(socket, &t))
        {
            do
            {
                if (card_GetTupleData((PTUPLE_ARGS)&t,(PFBYTE) buf, 8)==8)
                {
                    if (buf[0] == 4)
                    {
                        phys_addr[0] = buf[2];
                        phys_addr[1] = buf[3];
                        phys_addr[2] = buf[4];
                        phys_addr[3] = buf[5];
                        phys_addr[4] = buf[6];
                        phys_addr[5] = buf[7];

                        if (!pcmcia_prism_mode(socket, irq, iobase))
                            return(-1);
                        else
                            return(card_type);
                    }
                }
            } while (tuple_get((PTUPLE_ARGS)&t));
        }
    }
    return(card_type);
}


RTIP_BOOLEAN card_is_agere(int socket, int irq, int iobase, PFBYTE phys_addr)
{
    if (card_device_type[socket] == PCMCIA_AGERE_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    if (pcmctrl_card_installed(socket) &&
        (pcmcia_card_is_prism(socket, irq, iobase, phys_addr) == PCMCIA_AGERE_DEVICE))
    {
        card_device_type[socket] = PCMCIA_AGERE_DEVICE;
        return(TRUE);
    }
    return(FALSE);
}


RTIP_BOOLEAN card_is_intersil(int socket, int irq, int iobase, PFBYTE phys_addr)
{
    if (card_device_type[socket] == PCMCIA_INTERSIL_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    if (pcmctrl_card_installed(socket) &&
        (pcmcia_card_is_prism(socket, irq, iobase, phys_addr) == PCMCIA_INTERSIL_DEVICE))
    {
        card_device_type[socket] = PCMCIA_INTERSIL_DEVICE;
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


RTIP_BOOLEAN card_is_symbol(int socket, int irq, int iobase, PFBYTE phys_addr)
{
    if (card_device_type[socket] == PCMCIA_SYMBOL_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    if (pcmctrl_card_installed(socket) &&
        (pcmcia_card_is_prism(socket, irq, iobase, phys_addr) == PCMCIA_SYMBOL_DEVICE))
    {
        card_device_type[socket] = PCMCIA_SYMBOL_DEVICE;
        return(TRUE);
    }
    return(FALSE);
}
#endif /* INCLUDE_PRISM_PCMCIA */


#if (INCLUDE_SMC91C9X)

/* Return true if it is an smc card     */
RTIP_BOOLEAN check_if_smc(int socket)               /*__fn__*/
{
TUPLE_ARGS t;
byte buf[50];
byte *p;

    /* Get Device Information       */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* Get the Device Information       */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 50))
        {
            p = &buf[2];
            if (tc_strcmp((PFCHAR)p, (PFCHAR)"SMC") == 0)
            {
                p += 4;
                *(p+7) = 0;
                if (tc_strcmp((PFCHAR)p, (PFCHAR)"EtherEZ") == 0)
                {
                    return(TRUE);
                }
            }
            else if (tc_strcmp((PFCHAR)p, (PFCHAR)"Ositech") == 0)
            {
                p += 8;
                *(p+25) = 0;
                if (tc_strcmp((PFCHAR)p, (PFCHAR)"Trumpcard:Four of Diamond") == 0)
                {
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

RTIP_BOOLEAN check_if_smc(int socket);

RTIP_BOOLEAN pcmcia_card_is_smc(int socket, int irq, int iobase, PFBYTE phys_addr)               /*__fn__*/
{
TUPLE_ARGS t;
byte buf[30];

    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    /* Check if a card is installed and powered up     */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(FALSE);
    }

    /* Get Device Information     */
    if (check_if_smc(socket))
    {
        t.tuple_offset = 0x00;
        t.tuple_desired = CISTPL_FUNCE;

        /* get functional extension subcode 4 which contains the
        ethernet address.
        Format is:
        0x22 0x08 0x04 0x6 xx xx xx xx xx xx
        where xx xx is the NIC address
        */
        if( card_GetFirstTuple(socket, &t))
        {
            do
            {
                if (card_GetTupleData((PTUPLE_ARGS)&t,(PFBYTE) buf, 8)==8)
                {
                    if (buf[0] == 4)
                    {
                        phys_addr[0] = buf[2];
                        phys_addr[1] = buf[3];
                        phys_addr[2] = buf[4];
                        phys_addr[3] = buf[5];
                        phys_addr[4] = buf[6];
                        phys_addr[5] = buf[7];
                        /* The pcmcia setup is the same for smc and linksys so use the linksys version     */
                        return(pcmcia_linksys_mode(socket, irq, iobase));
                    }
                }
            } while (tuple_get((PTUPLE_ARGS)&t));
        }
    }
    return(FALSE);
}

RTIP_BOOLEAN pcmcia_card_is_smc(int socket, int irq, int iobase, PFBYTE phys_addr);

RTIP_BOOLEAN card_is_smc(int socket, int irq, int iobase, PFBYTE phys_addr) /*__fn__*/
{
    /* Make sure the pcmcia controller is alive     */
    if (!pcmctrl_init())
        return(FALSE);
    if (card_device_type[socket] == PCMCIA_SMC_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    if (pcmctrl_card_installed(socket) && pcmcia_card_is_smc(socket, irq, iobase, phys_addr))
    {
        card_device_type[socket] = PCMCIA_SMC_DEVICE;
        return(TRUE);
    }
    else
        return(FALSE);
}


#endif /* (INCLUDE_SMC91C9X) */


#if (INCLUDE_TCFE574_PCMCIA)
RTIP_BOOLEAN        pcmcia_card_is_tcfe574 (int socket, int irq, int iobase, byte *phys_addr);
static RTIP_BOOLEAN pcmcia_tcfe574_mode    (int socket, int irq, int iobase);
RTIP_BOOLEAN        check_if_tcfe574       (int socket);


RTIP_BOOLEAN card_is_tcfe574(int socket, int irq, int iobase, byte *phys_addr)
{
    /* Make sure the pcmcia controller is alive     */
    if (!pcmctrl_init())
        return(FALSE);
    if (card_device_type[socket] == PCMCIA_TCFE574_DEVICE)
        return(TRUE);
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    if (pcmctrl_card_installed(socket) && pcmcia_card_is_tcfe574(socket, irq, iobase, phys_addr))
    {
        card_device_type[socket] = PCMCIA_TCFE574_DEVICE;
        return(TRUE);
    }
    else
        return(FALSE);
}


/* ********************************************************************         */
/*                                                                              */
/* Type:        RTIP_BOOLEAN                                                    */
/* Routine:     check_if_tcfe574                                                */
/* Description: Return true if it is a 3Com 3cfe574 card                        */
/* Input:       socket - the socket number the card is located                  */
/* Output:      True   - if the card is found                                   */
/*              False  - if the card is not found                               */
/*                                                                              */
/* ********************************************************************         */
RTIP_BOOLEAN check_if_tcfe574(int socket)
{
TUPLE_ARGS t;
byte buf[30];
byte *p;
int i;
dword offset;

    /* Get Device Information     */
    t.tuple_desired = CISTPL_VERS_1;
    if (card_GetFirstTuple(socket, &t))
    {
        /* Get the Device Information     */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 30))
        {
            p = &buf[2];
            if (tc_stricmp((PFCHAR)p, (PFCHAR)"3Com") == 0)
            {
                p = &buf[7];
                if (tc_strstr((PFCHAR)p, (PFCHAR)"Megahertz 574"))
                {
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}


/* ********************************************************************         */
/*                                                                              */
/* Type:        RTIP_BOOLEAN                                                    */
/* Routine:     pcmcia_card_is_tcfe574                                          */
/* Description: get the ethernet address and set up io windows, enable          */
/*              interrupts, power to Vpp, then set the Configuration            */
/*              option register.                                                */
/* Input:                                                                       */
/* Output:      True   - if the card is found and setup                         */
/*              False  - if the card is not found and setup                     */
/*                                                                              */
/* ********************************************************************         */
RTIP_BOOLEAN pcmcia_card_is_tcfe574(int socket, int irq, int iobase, byte *phys_addr)                                 /*__fn__*/
{
TUPLE_ARGS t;
byte buf[64];
int i, j, temp=0;

    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
        return(FALSE);
    /* Check if a card is installed and powered up     */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(FALSE);
    }
    if (check_if_tcfe574(socket))
    {
                return(pcmcia_tcfe574_mode(socket, irq, iobase));
    }
}


/* ********************************************************************         */
/*                                                                              */
/* Type:        static RTIP_BOOLEAN                                             */
/* Routine:     pcmcia_tcfe574_mode                                             */
/* Description: Put the chip into I) mode, map in the SMC register bank         */
/*              and enable the interrupt.  Called by                            */
/*              pcmcia_card_is_tcfe574().  May also be called from              */
/*              external code.  NOTE: THIS IS FOR THE 3COM 3CFE574 DRIVER       */
/* Input:                                                                       */
/* Output:                                                                      */
/*                                                                              */
/* ********************************************************************         */
static RTIP_BOOLEAN pcmcia_tcfe574_mode(int socket, int irq, int iobase)
{
    /* Set up io windows, enable interrupts, power to Vpp     */
    pcmctrl_map_linksys_regs(socket, irq, iobase);

    /* Set the Configuration option register.                                   */
    /* For some reason the smc requires this to be done after IO mode           */
    if (!card_write_config_regs(socket, 0/* CONFIG_OPTION_REGISTER */, 0x41))
    {
        return(FALSE);
    }
    return(TRUE);
}
#endif  /* INCLUDE_TCFE574_PCMCIA */

/* Return true if it is an ATA card     */
RTIP_BOOLEAN check_if_ata(int socket)                              /*__fn__*/
{
TUPLE_ARGS t;
byte c;
word w;
int device_type;
RTIP_BOOLEAN is_ata;

    /* Get Device Information       */
    is_ata = FALSE;
    t.tuple_desired = CISTPL_DEVICE;
    if (card_GetFirstTuple(socket, &t))
    {
        /* Get the Device type       */
        if (card_GetTupleData((PTUPLE_ARGS)&t, (byte KS_FAR *) &c, 1)==1)
        {
            device_type = (int)(c>>4);
            if (device_type == 0x0D /* DTYPE_FUNCSPEC */)
            {
                /* Call CISTPL_FUNCE. First 2 bytes should be 0x0101.             */
                /* We look at as a word. Since its 0x0101 it is portable          */
                /* with respect to byte order                                     */
                t.tuple_offset = 0x00;
                t.tuple_desired = CISTPL_FUNCE;
                if( card_GetFirstTuple(socket, &t) &&
                (card_GetTupleData((PTUPLE_ARGS)&t,(byte KS_FAR *) &w, 2)==2)
                && (w == (word) 0x0101))
                {
                    is_ata = TRUE;
                }
            }
        }
    }
    return(is_ata);
}
/* SHANE - REMOVED OPEN COMMENT   */
#if (INCLUDE_3C589)

RTIP_BOOLEAN check_if_3com(int socket)
{
TUPLE_ARGS t;
byte buf[64];

    /* Get manufactures ID info     */
    t.tuple_offset = 0x00; 
    t.tuple_desired = CISTPL_MANFID;
    if (!card_GetFirstTuple(socket, &t))
        return(FALSE);
    if (card_GetTupleData((PTUPLE_ARGS)&t, (PFBYTE) buf, 64) < 4)
        return(FALSE);
    if (buf[0] != 0x01 || buf[1] != 0x01)
        return(FALSE);

    return(TRUE);
}
#endif


/*
* int  pcmcia_card_type(int slot)
*
* Check the slot (0 or 1) and return the type ofcard installed.
*
*
* Return values are:
*
* 0 - No card
* 1 - Unkown card
* 2 - ATA Card with normal CIS
* 3 - SMC Ethernet Card with normal CIS
* 4 - Linksys Ethernet Card
* 5 - 3COM Ethernet Card
* 9 - 3COM 3CFE574 Card
*/


int  pcmcia_card_type(int socket)                                  /*__fn__*/
{
int card_type = 1;  /* No Card */

    /* If the card is already in use. query the device type table.
       we can't touch the CIS now */
    if (card_device_type[socket] != PCMCIA_NO_DEVICE)
    {
        if (card_device_type[socket] == PCMCIA_ATA_DEVICE)
            return(2);
        else if (card_device_type[socket] == PCMCIA_SMC_DEVICE)
            return(3);
        else
            return(1);  /* Unknown */
    }
    /* Initalize the pcmcia controller if it has not been done already
       return -1 on a controller failure (unlikely) */
    if (!pcmctrl_init())
        return(-1);
    /* Check if a card is installed and powered up       */
    if (!pcmctrl_card_installed(socket))
    {
        pcmctrl_card_down(socket);
        return(0); /* No Card */
    }

    if (check_if_ata(socket))
        card_type = 2; /* Ata card */

#if (INCLUDE_SMC91C9X)
    if (card_type == 1)
        if (check_if_smc(socket))
        card_type = 3; /* SMC card */

#endif
#if (INCLUDE_SMC8041_PCMCIA)
    if (card_type == 1)
        if (check_if_smc8041(socket))
        card_type = 4; /* Linksys card */
#endif
#if (INCLUDE_NE2000)
    if (card_type == 1)
        if (check_if_linksys(socket))
        card_type = 4; /* Linksys card */
#endif
#if (INCLUDE_3C589)
    if (card_type == 1)
        if (check_if_3com(socket))
        card_type = 5; /* 3COM card */
#endif
#if (INCLUDE_TCFE574_PCMCIA)
    if (card_type == 1)
        if (check_if_tcfe574(socket))
            card_type = 9; /* 3COM 3CFE574 card */
#endif

/* printf("Not shutting down \n");                                 */
/*    pcmctrl_card_down(socket);  Leave the card as we left it     */
    return(card_type);
}

#endif /* INCLUDE_PCMCIA */
