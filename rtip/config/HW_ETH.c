/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*           (C) 2003    SEGGER Microcontroller Systeme GmbH          *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : HW_ETH.C
Purpose     : HW functions for Avionic MIPS board with SMSC LAN91C111
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "cu351_cpu_types.h"
#include "RTOS.H"     /* embOS header */
#include "vr4181a.h"  /* Hardware definitions */
#include "Ethernet_hwc.h"
#include "gpio.h"

#include <ipconfig/MPCNetworkDaemon_C.h>

/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
* HW_EthernetIRQ
*
* Function description
*   Checks if it is irq from ethernet and handles irq if so
*
* Notes
*   Called from OS_USER_irq(), thus OS_EnterInterrupt() is already
*   called by embOS, and OS_LeaveInterrupt() will be called by embOS
*
*/

extern void smc91c9x_interrupt(int n);  /* RTIP SMC91... driver ISR                          */

U8 HW_EthernetIRQ(void)
{
  auto U8 ret_value = FALSE;

  if (   (*(U16*)GPINTMSK2)  & (1 << 2)   //Only call interrupthandler if interrupt is enabled, JLA IO Technologies 28-06-05
      && (*(U16*)GPINTSTAT2) & (1 << 2))  /* is it an irq from ethernet (GPIO34)?               */
  {
    *(U16*)GPINTSTAT2 |= (1 << 2);     /* clear irq by writing 1 to interrupt status bit     */
    //smc91c9x_interrupt(0);             /* Call RTIP SMC91... driver ISR                      */
    if (MPCNetworkDaemonInterrupt_C())	//JLA(IO), call MPCNetworkDaemon to see if Network is enabled,
	{									//and to let MPCNetworkDaemon count interrupts
		smc91c9x_interrupt(0);
	}
    ret_value = TRUE;
  }
  return ret_value;
}


/*********************************************************************
*
*             HW_EnEthernetIRQ
*
* Function description
*   Enable rising edge interrupt from GPIO34 pin - ethernet irq  FKA!!!
*/
void HW_EnEthernetIRQ(void)
{
  *(U16*)GPINEN2     |=  (1 <<  2);  /* Enable input                                         */
  *(U16*)GPINTTYP4   |=  (1 <<  5);  /* Interrupt should be level-triggered - JLA IO Technologies 05-10-2005 */
  *(U16*)GPINTTYP4   |=  (1 <<  4);  /* High level required                                  */
  *(U16*)GPINTSTAT2  |=  (1 <<  2);  /* Clear irq by writing 1 to interrupt status bit       */
  *(U16*)GPINTMSK2   |=  (1 <<  2);  /* Enable interrupt from this particular pin (GIO unit) */
  *(U16*)MSYSINT0REG |=  (1 << 10);  /* Enable interrupt from this group of pins (ICU unit, MGIU2INTR bit) */
}


/*********************************************************************
*
*             HW_DisEthernetIRQ
*
* Function description
*   Disable interrupt from GPIO34 pin - ethernet irq  FKA !!!
*/
void HW_DisEthernetIRQ(void)
{
  *(U16*)GPINTMSK2  &= ~(1 <<  2);  /* Disable interrupt from this particular pin (GIO unit)          */
  *(U16*)GPINTSTAT2 &= ~(1 <<  2);  /* Clear irq (if there is already) by writing 1 to irq status bit */
}

void HW_EthernetInit(void)
{
  int i;

  C_SetAs(ETHERNET_RESET,OUTPUT,1);

  for (i=0; i < 500; i++);

  C_Set(ETHERNET_RESET,0);

 (*(unsigned short *)0xB000030E) = 0x0001;  // select bank 1
// (*(unsigned short *)0xB0000300) &= ~0x0400;
 (*(unsigned short *)0xB0000300) |= 0x0400;

}



