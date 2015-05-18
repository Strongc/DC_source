/* ETHCONF.H  - configuration file for device drivers

   EBS - RTIP

   Copyright Peter Van Oudenaren , 1993
   All rights reserved.
   This code may not be redistributed in source or linkable object form
   without the consent of its author.

    Module description:
        This module contains constants that may be change to the modify
        the build characteristics of drivers for RTIP.

    NOTE: this module needs to be included after ostype.h

*/

#ifndef __ETHCONFH__
#define __ETHCONFH__ 1

#include "xnconf.h"
#include "ostype.h"

/* YOU MIGHT WANT TO CHANGE ALL OF THE FOLLOWING   */

/* ************************************************************************    */
/* *******************       DEVICES         ******************************    */
/* ************************************************************************    */

/* determines PCMCIA support is included in various drivers.  See the
   device table (chapter 4 of the RTIP Manual) to determine which device drivers
   can support PCMCIA */
/* NOTE: if !AT_MOTHERBOARD, INCLUDE_PCMCIA will be turned off in rtip.h   */
/* NOTE: ERTFS might already have turned INCLUDE_PCMCIA on                 */
#if (!AT_MOTHERBOARD)
#define INCLUDE_PCMCIA              0
#define INCLUDE_RTKERNEL_PCMCIA     0
#else
#define INCLUDE_PCMCIA      1
#endif

/* set up defines what what drivers are allowed for the various platforms;   */
/* the drivers may still be turned off individually to save space            */
#if (BUILD_NEW_BINARY)
#define PC_DRV      0 
#define PC_DRV_ON   0 
#define LAN_DRV     0
#define ETH_DRV     0
#define ETH8_DRV    0
#define MCF5272_DRV 0 

#else
#define PC_DRV      0
#define PC_DRV_ON   0 
#define LAN_DRV     0   /* lance.c */
#define ETH_DRV     0
#define ETH8_DRV    0
#define MCF5272_DRV 0 
#endif

#define PC_LNX_DRV  0

/* ethernet drivers     */
#define INCLUDE_NE2000      0      /* Standalone NE2000 (ne2000.c) */
#define INCLUDE_ED          0      /* INCLUDE_ED_NE2000, INCLUDE_SMC8XXX, or INCLUDE_3C503; (ifed.c) */
#define INCLUDE_ED_NE2000   0      /* INCLUDE_ED must also be set (ifed.c) */
#define INCLUDE_SMC8XXX     0      /* INCLUDE_ED must also be set */
#define INCLUDE_3C503       0      /* INCLUDE_ED must also be set */
#define INCLUDE_SMC91C9X    1 
#define INCLUDE_3C509       0
#define INCLUDE_3C589       0      /* 3COM 3C589 card */
#define INCLUDE_LANCE       0
#define INCLUDE_RTLANCE     0
#define INCLUDE_LANCE_ISA   0
#define INCLUDE_EEXP        0      /* Intel 82586 - not supported yet */
#define INCLUDE_EEPRO       0      /* Intel 82595 */
#define INCLUDE_R8139       0      /* Realtek 8139 (Accton Cheetah PCI 10/100 */
#define INCLUDE_I82559      0      /* Intel 82559 (ether express pro 100) */
#define INCLUDE_DE4X5       0      /* not supported yet */
#define INCLUDE_TC90X       0      /* 3COM 90X (3C905c etherlink) */
#define INCLUDE_N83815      0      /* National 83815 PCI */
#define INCLUDE_CS89X0      0      /* Cirrus Crystal 8900/8920 - cs89x0.c */
#define INCLUDE_LAN89X0     0      /* Cirrus Crystal 8900/8920 - lan8900.c */
#define INCLUDE_XIRCOM      0      /* Xircom RealPort 10/100 CE3   */
#define INCLUDE_DAVICOM     0      /* Davicom 9102a PCI */
#define INCLUDE_TCFE574     0      /* 3Com 3CFE574BT PCMCIA */
#define INCLUDE_SMC8041_PCMCIA 0
#define INCLUDE_ETH360      0      /* Motorola 68EN360 - (68360 only) */
#define INCLUDE_ETH860      0      /* Motorola mpc860 or mpc 821 */
#define INCLUDE_ETHFEC      0      /* Motorola fec for coldfire 5272 */
#define INCLUDE_SBC5307     0      /* Motorola 5307 ColdFire eval board (sbc5307.c) */
#define INCLUDE_PRISM       0      /* PRISM BASED PCI and PCMCIA 802.11 cards */
#define INCLUDE_MTOKEN      0      /* Madge PCI token ring adapter */

#define INCLUDE_ED_SE7709A  0

#define INCLUDE_WINETHER    0

#if (!INCLUDE_TOKEN)
#undef INCLUDE_MTOKEN
#define INCLUDE_MTOKEN 0
#endif

/* This is a special testing-only ethernet device that is only used (at the
   moment) by the RIP testing code; if you plan to run the RIP regression
   test, but don't have a second device on the test machine, just open one
   of type NIL_ETHER_DEVICE; otherwise set to 0 to save code space */
#define INCLUDE_NIL         0           /* NIL Ethernet device */

/* Just a reminder. NE2000 and ED_NE2000 are incompatible     */
#if (INCLUDE_NE2000)
#undef INCLUDE_ED_NE2000
#define INCLUDE_ED_NE2000   0
#endif

#if (INCLUDE_TCFE574)
/* choose the type of 3Com 3CFE574 driver you plan to support   */
#define INCLUDE_TCFE574_PCMCIA  (1 && INCLUDE_PCMCIA)
#endif

#if (INCLUDE_PRISM)
/* choose the type of prism 802.11 wireless driver you plan to support   */
#define INCLUDE_PRISM_PCI       1
#define INCLUDE_PRISM_PCMCIA    (1 && INCLUDE_PCMCIA)
#endif

#define INCLUDE_MB86964     0

/* Can't use 3c589 without pcmcia     */
#if (!INCLUDE_PCMCIA)
#undef INCLUDE_3C589
#define INCLUDE_3C589       0
#endif

#define INCLUDE_PKT         0           /* packet driver - NOTE: not supported for
                                           protected mode or RTXC */
#define INCLUDE_LOOP        1           /* loop back */

/* ************************************************************************       */
/* *******************  DEVICE BEHAVIOUR     ******************************       */
/* ************************************************************************       */
/* Set to 1 to include support for device drivers to take advantage
   of device drivers ability to queue packets in the transmit queue
   NOTE: not all drivers have this feature
   WARNING: DO NOT TURN THIS ON - Not supported yet (May 20, 2000) */
#define INCLUDE_XMIT_QUE 0

/* ************************************************************************       */
/* ******************* DEVICE TIMERS         ******************************       */
/* ************************************************************************       */

/* number of seconds to wait for transmit of ethernet or loopback
   output packet to complete   */
#define CFG_ETHER_XMIT_TIMER 4

/* ************************************************************************       */
/* *******************  NUMBER DEVICES   **********************************       */
/* ************************************************************************       */

#if (BUILD_NEW_BINARY)
#define NUM_DEVICE 2
#else
#define NUM_DEVICE 1
#endif


/* number of NE2000 connections (standalone NE2000) supported     */
#define CFG_NUM_NE2000 NUM_DEVICE

/* number of ED connections (NE2000, SMC8XXX and 3C503) supported     */
#define CFG_NUM_ED NUM_DEVICE

/* number of SMC91C9X connections supported     */
#define CFG_NUM_SMCX NUM_DEVICE

/* number of 3C509 connections supported      */
#define CFG_NUM_3C NUM_DEVICE

/* number of 3C589 connections supported      */
#define CFG_NUM_3C89 NUM_DEVICE

/* number of LANCE connections supported      */
#define CFG_NUM_LANCE NUM_DEVICE

/* number of LANCE connections supported by lanceisa.c      */
#define CFG_NUM_LANCE_ISA NUM_DEVICE

/* number of EEXPRESS (Intel 82586) connections supported     */
#define CFG_NUM_EEXP NUM_DEVICE

/* number of EEXPRESS PRO (Intel 82595) connections supported     */
#define CFG_NUM_EEPRO NUM_DEVICE

/* number of Realtek 8139 based devices      */
#define CFG_NUM_R8139 NUM_DEVICE

/* number of Intel 82559 based devices    */
#define CFG_NUM_I82559 NUM_DEVICE

/* number of DE4X5 connections supported     */
#define CFG_NUM_DE4X5 1

/* number of 3com 90x based devices      */
#define CFG_NUM_TC90X  NUM_DEVICE

/* number of CS89X0 connections supported     */
#define CFG_NUM_CS89X0 NUM_DEVICE

/* number of LAN89X0 connections (lan8900.c) supported (this driver replaces cs89x0.c)   */
#define CFG_NUM_CS 2

/* number of XIRCOM connections supported     */
#define CFG_NUM_XIRCOM   NUM_DEVICE

/* number of National 83815 PCI connections supported     */
#define CFG_NUM_N83815   NUM_DEVICE

/* number of PRISM based devices            */
#define CFG_NUM_PRISM  NUM_DEVICE

/* number of Davicom connections supported   */
#define CFG_NUM_DAVICOM NUM_DEVICE

/* number of 3Com 3CFE574BT PCMCIA devices   */
#define CFG_NUM_TCFE574 NUM_DEVICE

/* SBC5307 definitions - TBD - change name  */
#define CFG_SBC5307_NUM      NUM_DEVICE

/* ************************************************************************       */
/* *******************  DEVICE PARAMETER     ******************************       */
/* ************************************************************************       */
/* the following configuration values are only used if no bios exists to          */
/* set up cfg space                                                               */
#define CFG_82559_PCI_IRQ       0x0a     /* Default IRQ if not in cfg space */
#define CFG_82559_PCI_IOBASE    0x1000   /* Default io base if not in cfg space */
#define CFG_82559_PCI_MEMBASE   0x04000000   /* Default base if not in cfg space */

/* ************************************************************************      */
/* configuration values for R8139                                                */
#define CFG_R8139_RX_BUFLEN_IDX 0   /* 0==8K, 1==16K, 2==32K, 3==64K */
/* TBD - should this be a configuration parameter   */
#define CFG_R8139_RX_BUFLEN (8192 << CFG_R8139_RX_BUFLEN_IDX)

/* the following configuration values are only used if no bios exists to   */
/* set up cfg space                                                        */
#define CFG_R8139_PCI_IRQ       0x0a     /* Default IRQ if not in cfg space */
#define CFG_R8139_PCI_IOBASE    0x1000   /* Default io base if not in cfg space */

/* ************************************************************************      */
/* the following configuration values are only used if no bios exists to         */
/* set up cfg space                                                              */
#define CFG_TC90X_PCI_IRQ       0x0a     /* Default IRQ if not in cfg space */
#define CFG_TC90X_PCI_IOBASE    0x1000   /* Default io base if not in cfg space */
#define CFG_TC90X_PCI_MEMBASE   0x04000000   /* Default base if not in cfg space */

/* I/O address base and irq for DP83902 on Hitachi SH SolutionEngine board   */
/* ************************************************************************      */
/* if LANCE is 32 bit, set to 1
   if LANCE is 16 bit, set to 0 */
/* NOTE: INCLUDE_RTLANCE must be set                                         */
/* NOTE: if protected mode, LANCE can be run in either 32 bit or 16 bit mode */
/*       if real mode, LANCE must be 16 bit                                  */
#define CFG_AMD_32BIT 1

/* if LANCE is PCI-based (Am79C972), CFG_AMD_PCI must be set   */
#define CFG_AMD_PCI 1

#define CFG_LANCE_BUS_MASTER 1
#define CFG_LANCE_SHARED_MEM 0
#if (CFG_LANCE_SHARED_MEM)
#   if (IS_MS_PM || IS_BCC_PM)  /* protected mode */
#       define CFG_LANCE_SH_MEM_ADDR  (PFBYTE) 0xD0000
#   elif (defined(__BORLANDC__) || defined(_MSC_VER) )
#       define CFG_LANCE_SH_MEM_ADDR (PFBYTE) 0xD0000000
#   else
#       error - Define an address for the shared memory area of the lance card
#   endif
#endif

/* Use these values for the AM79C972 if probes of the IOBASE and IRQ
   Registers show that the System BIOS is not assigning these resources
   to the device.
   Values set below are used instead of values from the device table
   or the global configuration values */
#define CFG_AMD_PCI_IOBASE  0x1000      /* 1st valid PCI address    */
#define CFG_AMD_PCI_IRQ     0x0A        /* IRQ available to system  */
#define CFG_AMD_PCI_MEMBASE 0x04000000  /* Default base if not in cfg space */

/* The rtlance driver allows for the "manual" setting of the PHY speed
     (as opposed to auto-negotiation) when the ethernet interface is an
     Am79C972.  One of the following must be written into rt_set_phy_speed
     (defined in rtipdata.c) prior to opening the device. */
#define PHY_10BT_HD     0
#define PHY_10BT_FD     1
#define PHY_100BT_HD    2
#define PHY_100BT_FD    3
#define PHY_AUTO        4
#define CFG_DEFAULT_PHY_SPEED PHY_AUTO

/* The rtlance driver allows the Am79C972 to be placed in loopback mode.
     NO_LOOP indicates that loopback mode is not enabled.
     EXTERNAL_LOOP indicates that loopback is in the PHY.
     INTERNAL_LOOP indicates that loopback is in the MII.
     GPSI loopback is not implemented.  */
#define NO_LOOP         0
#define EXTERNAL_LOOP   1
#define INTERNAL_LOOP   2
#define CFG_DEFAULT_LB_MODE NO_LOOP

/* ************************************************************************      */
#define CFG_N83815_PCI_IRQ      5
#define CFG_N83815_PCI_IOBASE   0x300

/* ************************************************************************      */
#define CFG_PRISM_PCI_IRQ       0x0a

/* ************************************************************************      */
#define CFG_DAVICOM_PCI_IRQ     5

/* ************************************************************************      */
/* Set to one to hardwire the NE2000 to Board address and IRQ rather
   then extracting from global parms or device table (Saves space on
   little systems) */
#define CFG_NE2000_HW    0

/* Use these values in hardwired mode (i.e. for the NE2000 driver
   (INCLUDE_NE2000 is 1 and CFG_NE2000_HW is 1), the configuration
   values set below are used instead of values from the device table
   or the global configuration values */
#define CFG_NE2000_IOBASE 0x300
#define CFG_NE2000_IRQ    0x05

/* If the following is set to 1 a probe for the device is done. Set to
   0 for a product with ne2000/83902 embedded */
#define CFG_NE2000_PROBE 0

/* Set this if we know it is an  NE1000     */
#define CFG_FORCE_NE1000 0

/* ************************************************************************      */
/* SBC5307 definitions                                                           */
#define CFG_SBC5307_HW       1
#define CFG_SBC5307_IOBASE   0xFE600300
#define CFG_SBC5307_IRQ      0x03

/* ************************************************************************      */
/* mode to run packet driver n(see INCLUDE_PKT) where:
      4 = accept packets to this board, multicast and broadcast
      3 = accept packets to this board and broadcast
   NOTE: 4 did not work for 3c509 packet driver (3 did work)
*/
#define CFG_PKT_MODE 4

/* ************************************************************************       */
/* *******************          PCI          ******************************       */
/* ************************************************************************       */
#if (defined(SMX) && SMX_VERSION >= 0x353)
/* SMX >= 3.5.3 has its own PCI functions. Macros in pci.h map rtpci_ to them.   */
#define INCLUDE_PCI 0
#elif (INCLUDE_RTLANCE && CFG_AMD_PCI)
#define INCLUDE_PCI 1
#elif (INCLUDE_I82559 || INCLUDE_R8139 || INCLUDE_TC90X || INCLUDE_PRISM_PCI || INCLUDE_DAVICOM)
#define INCLUDE_PCI 1
#else
#define INCLUDE_PCI 0
#endif

/* ************************************************************************       */
/* *******************  PACKET ADJUSTMENT    ******************************       */
/* ************************************************************************       */
/* Ethernet data frame size - size of data area in a DCU must have
   extra space for certain drivers/platforms to have room for CRC
   etc etc */
#if (POWERPC )
#define CFG_PACKET_ADJ 22       /* Must be set for the PPC860T */
#elif (INCLUDE_R8139 || INCLUDE_ETHFEC)
#define CFG_PACKET_ADJ 22       /* For Motorola FEC controller */
#elif (INCLUDE_I82559)
#define CFG_PACKET_ADJ 16
#elif (INCLUDE_TC90X || INCLUDE_RTLANCE || INCLUDE_LANCE || INCLUDE_LANCE_ISA || INCLUDE_DE4X5 )
#define CFG_PACKET_ADJ 4
#else
#define CFG_PACKET_ADJ 0
#endif

#if (INCLUDE_SNMP)
/* ************************************************************************       */
/* **********************      SNMP      **********************************       */
/* ************************************************************************       */
/* speed of the various devices (used by devices table)
   1000000=10BT 10000000=100BT
   Units: bits per second */
#define CFG_SPEED_NE2000    1000000
#define CFG_SPEED_SMC8X     1000000
#define CFG_SPEED_EL3       1000000
#define CFG_SPEED_PKT       1000000
#define CFG_SPEED_SMC9X     10000000
#define CFG_SPEED_DE4X5     1000000
#define CFG_SPEED_EL9       1000000
#define CFG_SPEED_EL89      1000000
#define CFG_SPEED_LANCE     10000000
#define CFG_SPEED_XIRCOM    1000000
#define CFG_SPEED_CS89X0    1000000
#define CFG_SPEED_EEXP      1000000
#define CFG_SPEED_EEPRO     1000000
#define CFG_SPEED_I82559    1000000
#define CFG_SPEED_ETH360    1000000
#define CFG_SPEED_N83815    1000000
#define CFG_SPEED_R8139     10000000
#define CFG_SPEED_TC90X     10000000
#define CFG_SPEED_DAVICOM   1000000
#define CFG_SPEED_ETH860    10000000
#define CFG_SPEED_ETHFEC    1000000
#define CFG_SPEED_SBC5307   1000000
#define CFG_SPEED_PRISM     1000000
#define CFG_SPEED_TCFE574   1000000
#define CFG_SPEED_MTOK      1000000
#define CFG_SPEED_LOOP      10000000
#define CFG_SPEED_WINETHER  1000000

/* OID of the various devices (used by devices table)     */
#define CFG_OID_NE2000      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_SMC8X       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_EL3         {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_PKT         {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_SMC9X       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_DE4X5       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_EL9         {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_EL89        {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_LANCE       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_XIRCOM      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_CS89X0      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_EEXP        {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_EEPRO       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_I82559      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_N83815      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_R8139       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_TC90X       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_DAVICOM     {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_ETH360      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_ETH860      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_ETHFEC      {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_SBC5307     {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_PRISM       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_TCFE574 {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_MTOK        {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_LOOP        {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_WINETHER    {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_SLIP        {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_CSLIP       {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#define CFG_OID_PPP         {{MIB_OID, 0, 0}, MIB_OID_SIZ + 2}
#endif  /* INCLUDE_SNMP */

/* ************************************************************************    */
/* *********************       TOKEN RING           ***********************    */
/* ************************************************************************    */

/* number of MADGE token ring adapters supported   */
#define CFG_NUM_MTOKEN   1

/* Use these values for the Madge Token ring adapter if probes of the IOBASE
   and IRQ Registers show that the System BIOS is not assigning these
   resources to the device.
   Values set below are used instead of values from the device table
   or the global configuration values */
/* The presto2000 device requires 0x80 bytes of I/O space   */
#define CFG_MTOK_PCI_IOBASE 0x1000  /* 1st valid PCI address    */
#define CFG_MTOK_PCI_IRQ    0x05        /* IRQ available to system  */

/* maximum RIF size for output   */
#define CFG_MAX_RIF 18              /* 2 for header and 16 for 8 routes */

#endif      /* ETHCONFH */


