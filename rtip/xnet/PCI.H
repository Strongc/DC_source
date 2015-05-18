/*                                                                                   */
/*  pci.h                                                                            */
/*                                                                                   */
/*  EBS - RTIP                                                                       */
/*                                                                                   */
/*  Copyright EBSnet, Inc., 1998                                                     */
/*  All rights reserved.                                                             */
/*  This code may not be redistributed in source or linkable object form without the */
/*  consent of its author.                                                           */
/*                                                                                   */
/*  Module Description:                                                              */
/*    This file contains definitions of constants and routines used to support the   */
/*    PCI BIOS functions as defined in the PCI BIOS Specification, Rev. 2.1,         */
/*    August 26, 1994.                                                               */
/*    It also includes Vendor IDs and Device IDs for the following                   */
/*    PCI devices:                                                                   */
/*       Am79C972  - supported added 10/98. VK                                       */
/*                                                                                   */
/*                                                                                   */
/*  Revision History:                                                                */
/*    Fall 1998        V. Kell        Initial Coding                                 */
/*                                                                                   */
/*                                                                                   */

#ifndef RTPCIDEF
#define RTPCIDEF

/*
**  Function Definitions input to PCI BIOS Routines
*/
#define RTPCI_K_FUNCTION_ID        0xB1
#define RTPCI_K_BIOS_PRESENT       0x01
#define RTPCI_K_FIND_DEVICE        0x02
#define RTPCI_K_FIND_CLASS_CODE    0x03
#define RTPCI_K_GEN_SPECIAL_CYCLE  0x06
#define RTPCI_K_READ_CONFIG_BYTE   0x08
#define RTPCI_K_READ_CONFIG_WORD   0x09
#define RTPCI_K_READ_CONFIG_DWORD  0x0A
#define RTPCI_K_WRITE_CONFIG_BYTE  0x0B
#define RTPCI_K_WRITE_CONFIG_WORD  0x0C
#define RTPCI_K_WRITE_CONFIG_DWORD 0x0D
#define RTPCI_K_GET_IRQ_RTE_OPT    0x0E
#define RTPCI_K_SET_IRQ            0x0F

/*
**  Return Codes for PCI BIOS Routines
*/
#define RTPCI_K_SUCCESSFUL         0x00
#define RTPCI_K_FNC_NOT_SUPPORTED  0x81
#define RTPCI_K_BAD_VENDOR_ID      0x83
#define RTPCI_K_DEVICE_NOT_FOUND   0x86
#define RTPCI_K_BAD_REG_NUM        0x87
#define RTPCI_K_SET_FAILED         0x88
#define RTPCI_K_BUFFER_TOO_SMALL   0x89

#define RTPCI_K_UNSUCCESSFUL       0x01  /* Not defined in PCI spec. Used in */
                                         /* rtpcifnc.c as a return code.       */

/*
**  PCI Configuration Register offsets in PCI Configuration Space
**  (for Am79C972)
*/
#define RTPCI_REG_CMD            0x04  /* Command Register -           word*/
#define RTPCI_REG_STAT           0x06  /* Status Register -            word*/
#define RTPCI_REG_LTNCY_TMR      0x0D  /* Latency Timer Register -     byte*/
#define RTPCI_REG_IOBASE         0x10  /* I/O Base Address Register - dword*/
#define RTPCI_REG_MEMMAPIO       0x14  /* Memory Mapped I/O Register- dword*/
#define RTPCI_REG_ROMBASE        0x30  /* Expansion ROM Base Addr Reg-dword*/
#define RTPCI_REG_INT_LINE       0x3C  /* Interrupt Line Register -    byte*/
#define RTPCI_REG_INT_PIN        0x3D  /* Interrupt Pin Register -     byte*/
#define RTPCI_REG_MIN_GRANT      0x3E  /* Min Grant Register -         byte*/
#define RTPCI_REG_MAX_LAT        0x3F  /* Max Latency Register -       byte*/

/*                                                                         */
/*  Register Masks for PCI registers.                                      */
/*  Encoded:  RTPCI_M_XXX_NNNN  where RTPCI is module indicator            */
/*                                    M     indicates this is a mask value */
/*                                    XXX   indicates register             */
/*                                    NNNN  is the bit position.           */
/*                                                                         */
#define RTPCI_M_CMD_IOEN        0x0001  /* I/O Space Access Enable      */
#define RTPCE_M_CMD_MEMEN       0x0002  /* Memory Space Access Enable   */
#define RTPCI_M_CMD_BMEM        0x0004  /* Bus Master Enable            */
#define RTPCI_M_IOBASE          0xFFFFFFE0  /* I/O BASE Reg. - all bits */
#define RTPCI_M_IOBASE_L        0xFFE0   /* I/O BASE Reg - low 16 bits  */
#define RTPCI_M_IOBASE_H        0xFFFF  /* I/O BASE Reg - high 16 bits  */
#define RTPCI_IOBASE_NOVAL      0x0001  /* low bit only set on read     */
#define RTPCI_INT_LINE_NOVAL    0x00    /* no interrupt line set        */

/*
**  Software Interrupt through which 16-bit users access PCI BIOS functions.
**  Note that access for 16-bit protected mode and 32-bit protected mode is done
**  differently.  This will be added later.  Right now the concentration is on
**  real-mode.
*/
#define RTPCI_INT_1A              0x1A

/*
**  Device IDs and Vendor IDs for PCI Devices 
*/
#define RTPCI_V_ID_AMD                0x1022        /* Vendor ID for AM            */
#define RTPCI_D_ID_AM79972            0x2000        /* Device ID for               */
                                                /*  Am79C972 - PCnet-FAST+        */
                                                /*  Am70C970A- PCnet-PCI II and   */
                                                /*  Am79C971 - PCnet-FAST         */

#define RTPCI_V_ID_INTEL              0x8086    /* Vendor ID for INTEL         */
#define RTPCI_D_ID_82559              0x1229    /* Device ID for 82559         */
#define RTPCI_D_ID_82559ER            0x1209    /* Device ID for 82559ER       */
#define RTPCI_D_ID_82559IB            0x1030    /* Device ID for 82559 In Business       */

#define RTPCI_V_ID_RTK                0x10EC    /* Vendor ID for REALTEK? */
#define RTPCI_D_ID_8139               0x8139    /* Device ID for RTL8139  */

#define RTPCI_V_ID_SMC                0x1113    /* Vendor ID for SMC or ACCTON? */
#define RTPCI_V_ID_ACC                0x1113    /* Vendor ID for SMC or ACCTON? */
#define RTPCI_D_ID_8139A              0x1211    /* Device ID for 8139           */

#define RTPCI_V_ID_SIS                0x1039    /* Vendor ID for SIS */
#define RTPCI_D_ID_8139B              0x0900    /* Device ID for 8139 */
#define RTPCI_D_ID_8139C              0x7016    /* Device ID for 8139 */



/*
**  Error conditions returned from PCI configuration routines in 
**  device drivers coded by EBS/VK.
*/
#define RTPCI_ERR_INIT              0x05

/*
**  Functions defined in pci.c.
**  For PPC603 target these functions are defined in init603.c
*/

int rtpci_bios_present(void);
unsigned char rtpci_find_device(unsigned short DId, unsigned short VId,
                                int Idx, unsigned char *bus,
                                unsigned char *dev);
unsigned char rtpci_read_byte(unsigned char bus, unsigned char dev,
                              int reg, unsigned char *val);
unsigned char rtpci_read_word(unsigned char bus, unsigned char dev,
                              int reg, unsigned short *val);
unsigned char rtpci_read_dword(unsigned char bus, unsigned char dev,
                              int reg, unsigned long *val);
unsigned char rtpci_write_byte(unsigned char bus, unsigned char dev, 
                               int reg, unsigned char val);
unsigned char rtpci_write_word(unsigned char bus, unsigned char dev, 
                               int reg, unsigned short val);
unsigned char rtpci_write_dword(unsigned char bus, unsigned char dev, 
                                int reg, unsigned long val);

#if (defined(SMX) && SMX_VERSION >= 0x353)

/* Use SMX PCI API.   */

/* These are the same as RTIP, so don't bother listing parameters.   */
#define rtpci_bios_present  pciBiosPresent
#define rtpci_read_byte     pciReadConfigByte
#define rtpci_read_word     pciReadConfigWord
#define rtpci_read_dword    pciReadConfigDword
#define rtpci_write_byte    pciWriteConfigByte
#define rtpci_write_word    pciWriteConfigWord
#define rtpci_write_dword   pciWriteConfigDword

/* This one switches the first 2 parameters.   */
#define rtpci_find_device(a, b, c, d, e)   pciFindDevice(b, a, c, d, e)

#else

int rtpci_bios_present(void);
unsigned char rtpci_find_device(unsigned short DId, unsigned short VId,
                                int Idx, unsigned char *bus,
                                unsigned char *dev);
unsigned char rtpci_read_byte(unsigned char bus, unsigned char dev,
                              int reg, unsigned char *val);
unsigned char rtpci_read_word(unsigned char bus, unsigned char dev,
                              int reg, unsigned short *val);
unsigned char rtpci_read_dword(unsigned char bus, unsigned char dev,
                              int reg, unsigned long *val);
unsigned char rtpci_write_byte(unsigned char bus, unsigned char dev, 
                               int reg, unsigned char val);
unsigned char rtpci_write_word(unsigned char bus, unsigned char dev, 
                               int reg, unsigned short val);
unsigned char rtpci_write_dword(unsigned char bus, unsigned char dev, 
                                int reg, unsigned long val);
#endif  /* SMX_VERSION >= 0x353 */

#endif   /* ndef RTPCIDEF */

