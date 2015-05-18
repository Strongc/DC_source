/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIPro                                         */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      :  mips_irq.c                                           */
/*                                                                          */
/* FILE NAME        :  mips_irq.c                                           */
/*                                                                          */
/* FILE DESCRIPTION :  This file handles the interrupts for Geni from       */
/*                     the UserIrq.c file                                   */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include <typedef.h>    /*definements for the Grundfos common types */
#include "hw_res.h"
#include "vir_drv.h"
#include "geni_if.h"

#define USB_TEST 1
#if (TCS_USB_SER_PORT && USB_TEST)
#include "USB_HW_Driver.h"
#endif
/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
#define BUS_MSK                            (1 << 0)
#define RS232_MSK                          (1 << 1)
#define COM_MSK                            (1 << 2)
#ifdef TCS_USB_SER_PORT
#define USB_MSK                            (1 << 3)
#endif

#define RTC1_INTERRUPT_MSK                  0x0002
#define RX_IRQ_MSK                          0x04
#define TX_IRQ_MSK                          0x02

#ifdef TCS_USB_SER_PORT
#define USB_IRQ_MSK							0xFF  //? need to change check USB status register
#endif
#define GENI_UART_TX_IRQ_PENDING(p)         ( p ==  TX_IRQ_MSK )
#define GENI_UART_RX_IRQ_PENDING(p)         ( p ==  RX_IRQ_MSK )
#ifdef TCS_USB_SER_PORT
#define GENI_USB_IRQ_PENDING(p)             ( p ==  USB_IRQ_MSK )
#endif

#if ((CTO_BUS_TYPE != Disable) && (BUS_UART_NUM != GENI_NOT_USED))
  #define BUS_TX_IDLE_CNT                   (25 * GENI_RTC_FREQ)/ BUS_BAUDRATE  // ((10 bit * 2.5 byte) * timerfreq)/ baudrate

  #define BUS_IRQ_HANDLER                   BUSIrqHandler()
  #define BUS_IDLE_HANDLER                  BUSIdleDetection()
  #if ( BUS_UART_NUM == 0)
    #define BUS_UART_IRQ_REG                ((*(UCHAR *)(SIU_IID_0    + SIU_BASE)))
    #define BUS_UART_STATUS_REG             (*(UCHAR *)(SIU_LS_0      + SIU_BASE))
  #elif ( BUS_UART_NUM == 1)
    #define BUS_UART_IRQ_REG                ((*(UCHAR *)(SIU_IID_1    + SIU_BASE)))
    #define BUS_UART_STATUS_REG             (*(UCHAR *)(SIU_LS_1      + SIU_BASE))
  #else
    #define BUS_UART_IRQ_REG                ((*(UCHAR *)(SIU_IID_2    + SIU_BASE)))
    #define BUS_UART_STATUS_REG             (*(UCHAR *)(SIU_LS_1      + SIU_BASE))
  #endif

  #if (BUS_IDLE_TYPE != NO_IDLE)
	//TCSL -  Mapping changes for supporting TFT display
	#ifndef TFT_16_BIT_LCD
	    #if (BUS_IDLE_PIN == GENI_GPIO46)
	      #define BUS_BUSY_IRQ_FLG              ((*(UINT *)GPINTSTAT2) & (1 << 14))
	      #define BUS_CLR_BUSY_IRQ_FLG          (*(UINT *)GPINTSTAT2) |= (1 << 14)
	    #elif (BUS_IDLE_PIN == GENI_GPIO47)
	      #define BUS_BUSY_IRQ_FLG              ((*(UINT *)GPINTSTAT2) & (1 << 15))
	      #define BUS_CLR_BUSY_IRQ_FLG          (*(UINT *)GPINTSTAT2) |= (1 << 15)
	    #else
	      #error 'Not a valid Bus Idle Pin'
	    #endif
	#else //TFT
		#if (BUS_IDLE_PIN == GENI_GPIO52)
		   #define BUS_BUSY_IRQ_FLG 			 ((*(UINT *)GPINTSTAT3) & (1 << 4))
		   #define BUS_CLR_BUSY_IRQ_FLG 		 (*(UINT *)GPINTSTAT3) |= (1 << 4)
		 #elif (BUS_IDLE_PIN == GENI_GPIO53)
		   #define BUS_BUSY_IRQ_FLG 			 ((*(UINT *)GPINTSTAT3) & (1 << 5))
		   #define BUS_CLR_BUSY_IRQ_FLG 		 (*(UINT *)GPINTSTAT3) |= (1 << 5)
		 #else
	      	   #error 'Not a valid Bus Idle Pin'
		#endif
	#endif
  #else
    #define BUS_BUSY_IRQ_FLG                FALSE
    #define BUS_CLR_BUSY_IRQ_FLG
  #endif
#else
  #define BUS_IDLE_HANDLER
  #define BUS_IRQ_HANDLER                   FALSE
  #define BUS_UART_IRQ_REG                  0
  #define BUS_BUSY_IRQ_FLG                  FALSE
  #define BUS_CLR_BUSY_IRQ_FLG

#endif

#if ((CTO_RS232_TYPE != Disable) && (RS232_UART_NUM != GENI_NOT_USED))
  #define RS232_TX_IDLE_CNT                   (25 * GENI_RTC_FREQ)/ RS232_BAUDRATE  // ((10 bit * 2.5 byte) * timerfreq)/ baudrate

  #define RS232_IRQ_HANDLER                 RS232IrqHandler()
  #define RS232_IDLE_HANDLER                RS232IdleDetection()
  #if ( RS232_UART_NUM == 0)
    #define RS232_UART_IRQ_REG              ((*(UCHAR *)(SIU_IID_0    + SIU_BASE)))
  #elif ( RS232_UART_NUM == 1)
    #define RS232_UART_IRQ_REG              ((*(UCHAR *)(SIU_IID_1    + SIU_BASE)))
  #else
    #define RS232_UART_IRQ_REG              ((*(UCHAR *)(SIU_IID_2    + SIU_BASE)))
  #endif

  #if (RS232_IDLE_TYPE != NO_IDLE)
//TCSL -  Mapping changes for supporting TFT display
#ifndef TFT_16_BIT_LCD
    #if (RS232_IDLE_PIN == GENI_GPIO46)
      #define RS232_BUSY_IRQ_FLG            ((*(UINT *)GPINTSTAT2) & (1 << 14))
      #define RS232_CLR_BUSY_IRQ_FLG        (*(UINT *)GPINTSTAT2) |= (1 << 14)
    #elif (RS232_IDLE_PIN == GENI_GPIO47)
      #define RS232_BUSY_IRQ_FLG            ((*(UINT *)GPINTSTAT2) & (1 << 15))
      #define RS232_CLR_BUSY_IRQ_FLG        (*(UINT *)GPINTSTAT2) |= (1 << 15)
    #else
      #error 'Not a valid Rs232 Idle Pin'
    #endif
	#else
	    #if (RS232_IDLE_PIN == GENI_GPIO52)
	      #define RS232_BUSY_IRQ_FLG            ((*(UINT *)GPINTSTAT3) & (1 << 4))
	      #define RS232_CLR_BUSY_IRQ_FLG        (*(UINT *)GPINTSTAT3) |= (1 << 4)
	    #elif (RS232_IDLE_PIN == GENI_GPIO53)
	      #define RS232_BUSY_IRQ_FLG            ((*(UINT *)GPINTSTAT3) & (1 << 5))
	      #define RS232_CLR_BUSY_IRQ_FLG        (*(UINT *)GPINTSTAT3) |= (1 << 5)
	    #else
	      #error 'Not a valid Rs232 Idle Pin'
	    #endif

    #endif
  #else
    #define RS232_BUSY_IRQ_FLG              FALSE
    #define RS232_CLR_BUSY_IRQ_FLG
  #endif
#else
  #define RS232_IDLE_HANDLER
  #define RS232_IRQ_HANDLER                 FALSE
  #define RS232_UART_IRQ_REG                0
  #define RS232_BUSY_IRQ_FLG                FALSE
  #define RS232_CLR_BUSY_IRQ_FLG

#endif

#if ((CTO_COM_TYPE != Disable) && (COM_UART_NUM != GENI_NOT_USED))
  #define COM_TX_IDLE_CNT                   (25 * GENI_RTC_FREQ)/ COM_BAUDRATE  // ((10 bit * 2.5 byte) * timerfreq)/ baudrate

  #define COM_IRQ_HANDLER                   COMIrqHandler()
  #define COM_IDLE_HANDLER                  COMIdleDetection()

  #if ( COM_UART_NUM == 0 )
    #define COM_UART_IRQ_REG                ((*(UCHAR *)(SIU_IID_0    + SIU_BASE)))
  #elif ( COM_UART_NUM == 1 )
    #define COM_UART_IRQ_REG                ((*(UCHAR *)(SIU_IID_1    + SIU_BASE)))
  #else
    #define COM_UART_IRQ_REG                ((*(UCHAR *)(SIU_IID_2    + SIU_BASE)))
  #endif

  #if (COM_IDLE_TYPE != NO_IDLE)
//TCSL -  Mapping changes for supporting TFT display
	#ifndef TFT_16_BIT_LCD
	    #if (COM_IDLE_PIN == GENI_GPIO46)
	      #define COM_BUSY_IRQ_FLG              ((*(UINT *)GPINTSTAT2) & (1 << 14))
	      #define COM_CLR_BUSY_IRQ_FLG          (*(UINT *)GPINTSTAT2) |= (1 << 14)
	    #elif (COM_IDLE_PIN == GENI_GPIO47)
	      #define COM_BUSY_IRQ_FLG              ((*(UINT *)GPINTSTAT2) & (1 << 15))
	      #define COM_CLR_BUSY_IRQ_FLG          (*(UINT *)GPINTSTAT2) |= (1 << 15)
	    #else
	      #error 'Not a valid Com Idle Pin'
	    #endif
	#else
	    #if (COM_IDLE_PIN == GENI_GPIO52)
	      #define COM_BUSY_IRQ_FLG              ((*(UINT *)GPINTSTAT3) & (1 << 4))
	      #define COM_CLR_BUSY_IRQ_FLG          (*(UINT *)GPINTSTAT3) |= (1 << 4)
	    #elif (COM_IDLE_PIN == GENI_GPIO53)
	      #define COM_BUSY_IRQ_FLG              ((*(UINT *)GPINTSTAT3) & (1 << 5))
	      #define COM_CLR_BUSY_IRQ_FLG          (*(UINT *)GPINTSTAT3) |= (1 << 5)
	    #else
	      #error 'Not a valid Com Idle Pin'
	    #endif
    #endif
  #else
    #define COM_BUSY_IRQ_FLG                FALSE
    #define COM_CLR_BUSY_IRQ_FLG
  #endif
#else
    #define COM_IDLE_HANDLER
    #define COM_IRQ_HANDLER		    FALSE
    #define COM_UART_IRQ_REG                0
    #define COM_BUSY_IRQ_FLG                FALSE
    #define COM_CLR_BUSY_IRQ_FLG
#endif

#if ((CTO_USB_TYPE != Disable) && (TCS_USB_SER_PORT == 1))

  #define USB_IRQ_HANDLER                 USBIrqHandler()
  #define USB_IDLE_HANDLER                USBIdleDetection()
#endif

/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
#if ((CTO_BUS_TYPE != Disable) && (BUS_UART_NUM != GENI_NOT_USED))
UCHAR bus_tx_idle_cnt = BUS_TX_IDLE_CNT, bus_tx_idle_active = FALSE;
#endif
#if ((CTO_COM_TYPE != Disable) && (COM_UART_NUM != GENI_NOT_USED))
UCHAR com_tx_idle_cnt = COM_TX_IDLE_CNT, com_tx_idle_active = FALSE;
#endif
#if ((CTO_RS232_TYPE != Disable) && (RS232_UART_NUM != GENI_NOT_USED))
UCHAR rs232_tx_idle_cnt = RS232_TX_IDLE_CNT, rs232_tx_idle_active = FALSE;
#endif
#ifdef TCS_USB_SER_PORT
#if (CTO_USB_TYPE != Disable)
extern GF_UINT8 usb_tx_idle_cnt;
extern GF_UINT32 usb_tx_idle_active;
#endif
#endif
/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
#ifdef TCS_USB_SER_PORT
extern void GeniUSBIrq(ULONG,ULONG);
extern void _IsrHandler(ULONG,ULONG);
#endif

#if ((CTO_BUS_TYPE != Disable) && (BUS_UART_NUM != GENI_NOT_USED))
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :  Returns TRUE if an interrupt was handled
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Bus channel interrupt handler
*
****************************************************************************/
static BIT BUSIrqHandler(void)
{
  UCHAR pending;
  // UART interrupts
  pending = (BUS_UART_IRQ_REG & 0x0F);   // Examine current interrupt state

  if ( GENI_UART_RX_IRQ_PENDING(pending) )                                   // Receive interrupt
    GeniBUSInIrq();

  else if ( GENI_UART_TX_IRQ_PENDING(pending) )                              // Transmit interrupt
  {
    GeniBUSOutIrq();
    bus_tx_idle_cnt = BUS_TX_IDLE_CNT;                                       // restart countdown
    bus_tx_idle_active = TRUE;                                               // tx idle should be active now
  }

  // Busy detection interrupts
  else if (BUS_BUSY_IRQ_FLG)
  {
    BUS_CLR_BUSY_IRQ_FLG;
    GeniBUSLineBusyIrq();
  }
  // no interrupts pending
  else
    return FALSE;

  return TRUE;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Checks if tx idle occured or calls the channels rx
*                      :  idle detection routine if active.
*
****************************************************************************/
void BUSIdleDetection(void)
{
  if ((bus_tx_idle_active) && ( bus_tx_idle_cnt-- == 0))      // tx active ? and count down value and check if we reached the bottom
  {
    bus_tx_idle_active = FALSE;                               // stop tx idle check
    ResetGeniDriver(BUS_CH_INDX);                             // reset the driver
  }

  #if ((BUS_IDLE_TYPE != NO_IDLE))
  else if (BUS_soft_timer_started)                            // Check for idle on the channel
    GeniBUSIdleTimerIrq();
  #endif
}
#endif

#if ((CTO_RS232_TYPE != Disable) && (RS232_UART_NUM != GENI_NOT_USED))
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :  Returns TRUE if an interrupt was handled
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Rs232 channel interrupt handler
*
****************************************************************************/
static BIT RS232IrqHandler(void)
{
  UCHAR pending;

  // UART interrupts
  pending = (RS232_UART_IRQ_REG & 0x0F);   // Examine current interrupt state

  if ( GENI_UART_RX_IRQ_PENDING(pending) )                                   // Receive interrupt
    GeniRS232InIrq();

  else if ( GENI_UART_TX_IRQ_PENDING(pending) )                              // Transmit interrupt
  {
    GeniRS232OutIrq();
    rs232_tx_idle_cnt = RS232_TX_IDLE_CNT;                                   // restart countdown
    rs232_tx_idle_active = TRUE;                                             // tx idle should be active now
  }

  // Busy detection interrupts
  else if (RS232_BUSY_IRQ_FLG)
  {
    RS232_CLR_BUSY_IRQ_FLG;
    GeniRS232LineBusyIrq();
  }
  // no interrupts pending
  else
    return FALSE;

  return TRUE;

}
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Checks if tx idle occured or calls the channels rx
*                      :  idle detection routine if active.
*
****************************************************************************/
void RS232IdleDetection(void)
{
  if ((rs232_tx_idle_active) && ( rs232_tx_idle_cnt-- == 0))  // tx active ? and count down value and check if we reached the bottom
  {
    rs232_tx_idle_active = FALSE;                             // stop tx idle check
    ResetGeniDriver(RS232_CH_INDX);                           // reset the driver
  }

  #if ((RS232_IDLE_TYPE != NO_IDLE))
  else if (RS232_soft_timer_started)                          // Check for idle on the channel
    GeniRS232IdleTimerIrq();
  #endif
}
#endif

#if ((CTO_COM_TYPE != Disable) && (COM_UART_NUM != GENI_NOT_USED))
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :  Returns TRUE if an interrupt was handled
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Com channel interrupt handler
*
****************************************************************************/
static BIT COMIrqHandler(void)
{
  UCHAR pending;

  // UART interrupts
  pending = (COM_UART_IRQ_REG & 0x0F);   // Examine current interrupt state

  if ( GENI_UART_RX_IRQ_PENDING(pending) )                                   // Receive interrupt
  {
    GeniCOMInIrq();
  }
  else if ( GENI_UART_TX_IRQ_PENDING(pending) )                              // Transmit interrupt
  {
    GeniCOMOutIrq();
    com_tx_idle_cnt = COM_TX_IDLE_CNT;                                       // restart countdown
    com_tx_idle_active = TRUE;                                               // tx idle should be active now
  }

  // Busy detection interrupts
  else if (COM_BUSY_IRQ_FLG)
  {
    COM_CLR_BUSY_IRQ_FLG;
    GeniCOMLineBusyIrq();
  }
  // no interrupts pending
  else
    return FALSE;

  return TRUE;
}
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Checks if tx idle occured or calls the channels rx
*                      :  idle detection routine if active.
*
****************************************************************************/
void COMIdleDetection(void)
{
  if ((com_tx_idle_active) && ( com_tx_idle_cnt-- == 0))      // tx active ? and count down value and check if we reached the bottom
  {
    com_tx_idle_active = FALSE;                               // stop tx idle check
    ResetGeniDriver(COM_CH_INDX);                             // reset the driver
  }

  #if ((COM_IDLE_TYPE != NO_IDLE))
  else if (COM_soft_timer_started)                            // Check for idle on the channel
    GeniCOMIdleTimerIrq();
  #endif
}
#endif

#if ((CTO_USB_TYPE != Disable) && (TCS_USB_SER_PORT == 1))
volatile GF_UINT32 gsr_int1;
volatile GF_UINT32 gsr_int2;
/****************************************************************************
* INPUT(S)			 :
* OUTPUT(S)			 :	Returns TRUE if an interrupt was handled
* DESIGN DOC. 		 :
* FUNCTION DESCRIPTION :	USB channel interrupt handler
*
****************************************************************************/
static BIT USBIrqHandler(void)
{
	GF_UINT32 pending1;
	GF_UINT32 pending2;

	gsr_int1 = GENI_USBF_GSTAT1;
	gsr_int2 = GENI_USBF_GSTAT2;

	pending1 = gsr_int1 & (TX_FINISH | RX_FINISH |GSR1_ERROR);
	pending2 = gsr_int2;

	// Check USB interrupt Status register here
	if(pending1 || pending2)
	{
		UsbIrqHandler(pending1,pending2);
	}
	else
	  return FALSE;

	return TRUE;
}
  /****************************************************************************
  * INPUT(S)			 :
  * OUTPUT(S)			 :
  * DESIGN DOC. 		 :
  * FUNCTION DESCRIPTION :	Checks if tx idle occured or calls the channels rx
  * 					 :	idle detection routine if active.
  *
  ****************************************************************************/
  void USBIdleDetection(void)
  {
    if ((usb_tx_idle_active) && (--usb_tx_idle_cnt == 0)) // tx active ? and count down value and check if we reached the bottom
    {
      usb_tx_idle_active = FALSE;     // stop tx idle check
      ResetGeniDriver(USB_CH_INDX);   // reset the driver
    }
  }
#endif

/****************************************************************************
* INPUT(S)             :  channels_disabled - bit 0 = 1 : bus disabled
*                      :                      bit 1 = 1 : rs232 disabled
*                      :                      bit 2 = 1 : com disabled
* OUTPUT(S)            :  Returns TRUE if an interrupt was handled
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Common handler for mips interrupts for Geni channels
*
****************************************************************************/
BIT GENIIrqHandler(UCHAR channels_disabled)
{
  BIT irq_handled = FALSE;

  if ( !irq_handled && ((channels_disabled & COM_MSK) == 0))
    irq_handled = COM_IRQ_HANDLER;

  if ( !irq_handled && ((channels_disabled & RS232_MSK) == 0))
    irq_handled = RS232_IRQ_HANDLER;

  if ( !irq_handled && ((channels_disabled & BUS_MSK) == 0))
    irq_handled = BUS_IRQ_HANDLER;

  #ifdef TCS_USB_SER_PORT
  if ( !irq_handled && ((channels_disabled & USB_MSK) == 0))
	  irq_handled = USB_IRQ_HANDLER;
  #endif
  return irq_handled;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :  Returns TRUE if an interrupt was handled
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Call all channels idle detection routine
*
****************************************************************************/
BIT GENIIdleDetection(void)
{
  if ((GENI_RTCINTREG & RTC1_INTERRUPT_MSK) != 0)
  {
    COM_IDLE_HANDLER;
    RS232_IDLE_HANDLER;
    BUS_IDLE_HANDLER;
	#ifdef TCS_USB_SER_PORT
	USB_IDLE_HANDLER;
	#endif

    GENI_RTCINTREG |= RTC1_INTERRUPT_MSK;                         // Set interrupt bit to clear it

    return TRUE;                                                  // The interrupt was set so we handled it
  }
  else
    return FALSE;
}



/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/


