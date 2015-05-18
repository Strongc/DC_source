/*********************************************************************
*
*        Etrhernet hardware routines
*
**********************************************************************
*/

#ifndef _ETHERNET_HWC_H_
#define _ETHERNET_HWC_H_

#include "mips.h"
#include "vr4181a.h"
#include <cu351_cpu_types.h>

/*********************************************************************
*
*        Ethernet
*
**********************************************************************
*/

/*********************************************************************
*        HW_SET_ETH_MEM_MAPPING MACRO
*
*        Loads PCS0 register with desired value of
*        base address for external devices
**********************************************************************
*/

/* Hardware definitions from vr4181a.h are used                     */

/* Start of window for external devices                             */
#define PCS0_BASE           0x10000000

/* Other values to prepare PCS0 register value                      */
#define HBU_SDRAM_WIDTH_16  0x00000040
#define	HBU_SDRAM_VISPCI    0x00000020
#define	HBU_SDRAM_MASK_32M  0x0000000b

/* ATTENTION! It's assumed that HBU_PCS0 is offset,
   i.e. doesn't include HBU_BASE
*/
#define HW_SET_ETH_MEM_MAPPING()                                     \
  *(U32*)(HBU_BASE + HBU_PCS0) = (PCS0_BASE |                        \
    HBU_SDRAM_VISPCI | HBU_SDRAM_WIDTH_16 | HBU_SDRAM_MASK_32M);



/*********************************************************************
*        ETHERNET_BASE calculation
**********************************************************************
*/
/* Address offset for ethernet chip                                 */
#define SMSC111_ADDR_OFFSET    0x0300

/* Now define ETHERNET_BASE                                         */
#define ETHERNET_BASE  (KSEG1_BASE | PCS0_BASE | SMSC111_ADDR_OFFSET)
/* this equals, for example, 0xB0000300 */

/* Example: Write to SMSC91C111 Bank Select Register (offset 0x0E):
   *(U16*)(ETHERNET_BASE + 0x0E) = 2;
*/


/*********************************************************************
*
**********************************************************************
*/
#ifdef __cplusplus
  extern "C" {
#endif

U8 HW_EthernetIRQ(void);
void HW_EnEthernetIRQ(void);
void HW_DisEthernetIRQ(void);
void HW_EthernetInit(void);
void HW_EthernetPhyInit(void);

#ifdef __cplusplus
  }
#endif

/*********************************************************************
*
**********************************************************************
*/

#endif /* _ETHERNET_HWC_H_ */
