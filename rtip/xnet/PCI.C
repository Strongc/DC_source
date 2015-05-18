/*                                                                                             */
/*  pci.c                                                                                      */
/*                                                                                             */
/*  EBS - RTIP                                                                                 */
/*                                                                                             */
/*  Copyright EBSnet, Inc., 1998                                                               */
/*  All rights reserved.                                                                       */
/*  This code may not be redistributed in source or linkable object form without the           */
/*  consent of its author.                                                                     */
/*                                                                                             */
/*  Module Description:                                                                        */
/*    This file contains the routines that access the PCI BIOS functions.  It is used          */
/*    by the device driver for any PCI device.                                                 */
/*   NOTE:  The caller to PCI System BIOS Functions must:                                      */
/*          1.  provide a minimum of 1024 bytes of stack                                       */
/*          2.  ensure that microprocessor privilege levels are set so that PCI BIOS functions */
/*              can access I/O address space without generating a protection fault.            */
/*                                                                                             */
/*  Revision History:                                                                          */
/*    Fall 1998        V. Kell      Initial Coding                                             */
/*  Feb  1999       PVO     Added Protected Mode support                                       */
/*  Sept 1999       TVO     Added 16 bit protected mode support                                */
/*                                                                                             */
/*                                                                                             */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "debug.h"

#define DEBUG_PCI 1

#if (RTIP_VERSION < 30)
#define INCLUDE_PCI (CFG_AMD_PCI || INCLUDE_I82559 || INCLUDE_R8139 || INCLUDE_TC90X || INCLUDE_N83815 || INCLUDE_PRISM)
#endif


/* Do not include this file for target PPC603. PCI functions are defined in init603.c
   for this target. PCI.H is still used for defines and prototypes */
#if (!PPC603)
#if (INCLUDE_PCI)

/* #include <dos.h>   */
#include "pci.h"        /*  PCI definitions  */

/* 16 and 32 bit protected mode   */

#define PCIREGS union REGS
#define CALL_PCI(X, Y)  int86(RTPCI_INT_1A,X,Y)



/*                                                                                  */
/*  rtpci_bios_present                                                              */
/*                                                                                  */
/*  This routine is called to determine if the PCI BIOS functions are present.      */
/*                                                                                  */
/*  It returns 0x00 in register AH IFF EDX is set to "PCI".  In addition, the Carry */
/*  Flag (CF) will be 0 if BIOS is present AND EDX is set properly.                 */
/*                                                                                  */
int rtpci_bios_present(void)
{

#if (!RTKBCPP && (IS_BCC_PM || IS_MS_PM))
    if (find_bios32_entry())
        return(1);
    else
        return(0);
#else
    PCIREGS inregs, outregs;

/* Run in 16 bit protected mode. Before accessing the BIOS we need to map a        */
/* 64 k byte segment at 0xF0000. We will want to call the INT 1A interrupt handler */
/* directly which is at 0xFFE6E.                                                   */
    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_BIOS_PRESENT;

    CALL_PCI(&inregs, &outregs);

    if ((outregs.x.cflag == 0) && (outregs.h.ah == 0x00) &&
        ((outregs.h.dl == 0x50) &&                           /*  "P"   */
         (outregs.h.dh == 0x43)))                            /*  "C"   */
        return(1);
    else
        return(0);
#endif

}

/*                                                                    */
/*  rtpci_find_device                                                 */
/*                                                                    */
/*  This routine is called to find the location of PCI Devices having */
/*  a specific Device ID and Vendor ID.                               */
/*                                                                    */
/*                                                                    */
/*                                                                    */
unsigned char rtpci_find_device (unsigned short DeviceID, unsigned short VendorID,
                                 int Index, unsigned char *BusNum,
                                 unsigned char *DevFncNum)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_FIND_DEVICE;
    inregs.x.cx = DeviceID;
    inregs.x.dx = VendorID;
    inregs.x.si = Index;

    CALL_PCI(&inregs, &outregs);

    if (outregs.x.cflag == 0)
    {
        *BusNum = outregs.h.bh;
        *DevFncNum =outregs.h.bl;
    }
#if (DEBUG_PCI)
    DEBUG_ERROR("rtpci_find_device returns ", EBS_INT1, outregs.h.ah, 0);
    DEBUG_ERROR("rtpci_find_device: busNum, DevFncNum:", 
        EBS_INT2, *BusNum, *DevFncNum);
        
#endif
    return(outregs.h.ah);
}

/*                                                                    */
/*  The following routines allow reading/writing a byte or word from  */
/*  the PCI configuration space of a specific device.                 */
/*                                                                    */

/*                                                          */
/*  rtpci_read_byte                                         */
/*                                                          */
/*  This routine reads a byte from the configuration space. */
/*                                                          */
unsigned char rtpci_read_byte(unsigned char BusNum, unsigned char DevFncNum,
                              int RegNum, unsigned char *ByteRead)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_READ_CONFIG_BYTE;
    inregs.h.bh = BusNum;
    inregs.h.bl = DevFncNum;
    inregs.x.di = RegNum;

    CALL_PCI(&inregs, & outregs);

    if (outregs.x.cflag == 0)
    {
        *ByteRead = outregs.h.cl;
        return(outregs.h.ah);
    }
    else
        return(RTPCI_K_UNSUCCESSFUL);
}

/*                                                          */
/*  rtpci_read_word                                         */
/*                                                          */
/*  This routine reads a word from the configuration space. */
/*                                                          */
unsigned char rtpci_read_word(unsigned char BusNum, unsigned char DevFncNum,
                              int RegNum, unsigned short *WordRead)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_READ_CONFIG_WORD;
    inregs.h.bh = BusNum;
    inregs.h.bl = DevFncNum;

    inregs.x.di = RegNum;     /* do I want to ensure that this is a multiple of 2? */
                              /* PCI book states that PCI BIOS will do this-pg695     */

    CALL_PCI(&inregs, &outregs);
    
    if (outregs.x.cflag == 0)
    {
        *WordRead = (unsigned short)(outregs.x.cx);
        return(outregs.h.ah);
    }
    else
        return(RTPCI_K_UNSUCCESSFUL);
}

/*                                                                           */
/*  rtpci_read_dword                                                         */
/*                                                                           */
/*  This routine reads a double word (4 bytes) from the configuration space. */
/*                                                                           */
unsigned char rtpci_read_dword(unsigned char BusNum, unsigned char DevFncNum,
                               int RegNum, unsigned long *DWordRead)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_READ_CONFIG_DWORD;
    inregs.h.bh = BusNum;
    inregs.h.bl = DevFncNum;

    inregs.x.di = RegNum;     /* do I want to ensure that this is a multiple of 2? */
                              /* PCI book states that PCI BIOS will do this-pg695    */

    CALL_PCI(&inregs, &outregs);
    
    if (outregs.x.cflag == 0)
        *DWordRead = outregs.x.cx;  /* outregs.e.ecx     */
    return(outregs.h.ah);
}


/*                                                         */
/*  rtpci_write_byte                                       */
/*                                                         */
/*  This routine writes a byte to the configuration space. */
/*                                                         */
unsigned char rtpci_write_byte(unsigned char BusNum, unsigned char DevFncNum,
                               int RegNum, unsigned char ByteWrite)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_WRITE_CONFIG_BYTE;
    inregs.h.bh = BusNum;
    inregs.h.bl = DevFncNum;
    inregs.x.di = RegNum;
    inregs.h.cl = ByteWrite;

    CALL_PCI(&inregs, & outregs);

    if (outregs.x.cflag == 0)
        return(outregs.h.ah);
    else
        return(RTPCI_K_UNSUCCESSFUL);
}

/*                                                         */
/*  rtpci_write_word                                       */
/*                                                         */
/*  This routine writes a word to the configuration space. */
/*                                                         */
unsigned char rtpci_write_word(unsigned char BusNum, unsigned char DevFncNum,
                               int RegNum, unsigned short WordWrite)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_WRITE_CONFIG_WORD;
    inregs.h.bh = BusNum;
    inregs.h.bl = DevFncNum;
    inregs.x.di = RegNum;
    inregs.x.cx = WordWrite;

    CALL_PCI(&inregs, &outregs);

    return(outregs.h.ah);
}
/*                                                                         */
/*  rtpci_write_dword                                                      */
/*                                                                         */
/*  This routine writes a double word(4 bytes) to the configuration space. */
/*                                                                         */
unsigned char rtpci_write_dword(unsigned char BusNum, unsigned char DevFncNum,
                                int RegNum, unsigned long DWordWrite)
{
    PCIREGS inregs, outregs;

    inregs.h.ah = RTPCI_K_FUNCTION_ID;
    inregs.h.al = RTPCI_K_WRITE_CONFIG_DWORD;
    inregs.h.bh = BusNum;
    inregs.h.bl = DevFncNum;
    inregs.x.di = RegNum;
    inregs.x.cx = (unsigned short)DWordWrite;       /* inregs.e.ecx  */

    CALL_PCI(&inregs, & outregs);

    return(outregs.h.ah);
}

#endif /* INCLUDE_PCI */
#endif /* #if (!PPC603) */
