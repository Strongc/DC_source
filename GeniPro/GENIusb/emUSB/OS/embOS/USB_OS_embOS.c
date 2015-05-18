  /*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
*                                                                    *
*       USB device stack for embedded applications                   *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : USB_OS_embOS.c
Purpose : Kernel abstraction for embOS
          Do not modify to allow easy updates !
--------  END-OF-HEADER  ---------------------------------------------
*/


#include "USB_Private.h"
#include "RTOS.h"
#include "USB_HW_Driver.h"


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

#if OS_VERSION < 33200
static OS_TASK * _apTask[USB_NUM_EPS];
#else
static OS_EVENT _aEvent[USB_NUM_EPS];
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Depending on the version of embOS, either event objects or
*       task events. Event object were in version V3.32 introduced.
*
**********************************************************************
*/

#if OS_VERSION < 33200
/*********************************************************************
*
*       USB_OS_Init
*
*  Function description:
*    This function shall initialize all event objects that are necessary.
*
*/
void USB_OS_Init(void) {
}


/**********************************************************
*
*       USB_OS_Delay
*
* Function description
*   Delays for a given number of ms.
*
*/
void USB_OS_Delay(int ms) {
  OS_Delay(ms);
}

/**********************************************************
*
*       USB_OS_Signal
*
* Function description
*   Wake the task waiting for reception
*   This routine is typically called from within an interrupt
*   service routine
*
*/
void USB_OS_Signal(unsigned EPIndex) {
  if (_apTask[EPIndex] != NULL) {
    OS_SignalEvent(1 << EPIndex, _apTask[EPIndex]);
    _apTask[EPIndex] = NULL;
  }
}

/**********************************************************
*
*        USB_OS_Wait
*
* Function description
*   Block the task until USB_OS_SignalRx is called
*   This routine is called from a task.
*
*/
void USB_OS_Wait(unsigned EPIndex) {
  _apTask[EPIndex] = OS_pCurrentTask;
  OS_WaitEvent(1 << EPIndex);
}

/**********************************************************
*
*        USB_OS_WaitTimed
*
* Function description
*   Block the task until USB_OS_Signal is called
*   or a time out occurs
*   This routine is called from a task.
*
*/
int USB_OS_WaitTimed(unsigned EPIndex, unsigned ms) {
  int r;
  _apTask[EPIndex] = OS_pCurrentTask;
  r = (int)OS_WaitEventTimed(1 << EPIndex, ms + 1);
  return r;
}

#else
/*********************************************************************
*
*       USB_OS_Init
*
*  Function description:
*    This function shall initialize all event objects that are necessary.
*
*/
void USB_OS_Init(void) {
  unsigned i;

  for (i = 0; i < COUNTOF(_aEvent); i++) {
    OS_EVENT_Create(&_aEvent[i]);
  }
}

/**********************************************************
*
*       USB_OS_Signal
*
* Function description
*   Wake the task waiting for reception
*   This routine is typically called from within an interrupt
*   service routine
*
*/
void USB_OS_Signal(unsigned EPIndex) {
  OS_EVENT_Pulse(&_aEvent[EPIndex]);
}

/**********************************************************
*
*        USB_OS_Wait
*
* Function description
*   Block the task until USB_OS_SignalRx is called
*   This routine is called from a task.
*
*/
void USB_OS_Wait(unsigned EPIndex) {
  OS_EVENT_Wait(&_aEvent[EPIndex]);
}

/**********************************************************
*
*        USB_OS_WaitTimed
*
* Function description
*   Block the task until USB_OS_Signal is called 
*   or a time out occurs
*   This routine is called from a task.
*
*/
int USB_OS_WaitTimed(unsigned EPIndex, unsigned ms) {
  int r;
  r = (int)OS_EVENT_WaitTimed(&_aEvent[EPIndex], ms + 1);
  return r;
}

/**********************************************************
*
*       USB_OS_Delay
*
* Function description
*   Delays for a given number of ms.
*
*/
void USB_OS_Delay(int ms) {
  OS_Delay(ms);
}
#endif
/**********************************************************
*
*       USB_OS_DecRI
*
* Function description
*   Decrement interrupt disable count and enable interrupts
*   if counter reaches 0.
*
*/
void USB_OS_DecRI(void) {
  OS_DecRI();
}

/**********************************************************
*
*        USB_OS_IncDI
*
* Function description
*   Increment interrupt disable count and disable interrupts
*
*/
void   USB_OS_IncDI(void) {
  OS_IncDI();
}


extern volatile unsigned char usbError;

/**********************************************************
*
*        USB_OS_Panic
*
* Function description
*   Called if fatal error is detected.
*
* Add. info
*   Error codes:
*     1   USB_ERROR_RX_OVERFLOW
*     2   USB_ERROR_ILLEGAL_MAX_PACKET_SIZE
*     3   USB_ERROR_ILLEGAL_EPADDR
*     4   USB_ERROR_IBUFFER_SIZE_TOO_SMALL
*/
void USB_OS_Panic(unsigned ErrCode) {
static unsigned long errcount = 0;
	//while (ErrCode);
/*	GENI_USBF_EP4_CONT &= ~EP4EN;
	usbError = ErrCode;  */
	errcount++;
}

/**********************************************************
*
*        USB_OS_GetTickCnt
*
* Function description
*   Returns the current system time in ticks.
*/
U32 USB_OS_GetTickCnt(void) {
  return OS_Time;
}

/*************************** End of file ****************************/


#if 0
/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
*                                                                    *
*       USB device stack for embedded applications                   *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : USB_OS_embOS.c
Purpose : Kernel abstraction for embOS
          Do not modify to allow easy updates !
--------  END-OF-HEADER  ---------------------------------------------
*/


#include "USB_Private.h"
#include "RTOS.H"
#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32

#define DISABLE_EMBOS 	1

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

#ifndef DISABLE_EMBOS
static OS_EVENT _aEvent[USB_NUM_EPS];
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       USB_OS_Init
*
*  Function description:
*    This function shall initialize all event objects that are necessary.
*
*/
void USB_OS_Init(void) {
  unsigned i;
#ifndef DISABLE_EMBOS
  for (i = 0; i < COUNTOF(_aEvent); i++) {
    OS_EVENT_Create(&_aEvent[i]);
  }
#endif  
}


/**********************************************************
*
*       USB_OS_Delay
*
* Function description
*   Delays for a given number of ms.
*
*/
void USB_OS_Delay(int ms) {
  OS_Delay(ms);
}

/**********************************************************
*
*       USB_OS_DecRI
*
* Function description
*   Decrement interrupt disable count and enable interrupts
*   if counter reaches 0.
*
*/
void USB_OS_DecRI(void) {
  OS_DecRI();
}

/**********************************************************
*
*        USB_OS_IncDI
*
* Function description
*   Increment interrupt disable count and disable interrupts
*
*/
void   USB_OS_IncDI(void) {
  OS_IncDI();
}

/**********************************************************
*
*       USB_OS_Signal
*
* Function description
*   Wake the task waiting for reception
*   This routine is typically called from within an interrupt
*   service routine
*
*/
void USB_OS_Signal(unsigned EPIndex) {
#ifndef DISABLE_EMBOS    
  OS_EVENT_Pulse(&_aEvent[EPIndex]);
#endif  
}

/**********************************************************
*
*        USB_OS_Wait
*
* Function description
*   Block the task until USB_OS_SignalRx is called
*   This routine is called from a task.
*
*/
void USB_OS_Wait(unsigned EPIndex) {
#ifndef DISABLE_EMBOS    
  OS_EVENT_Wait(&_aEvent[EPIndex]);
#endif  
}

/**********************************************************
*
*        USB_OS_WaitTimed
*
* Function description
*   Block the task until USB_OS_Signal is called 
*   or a time out occurs
*   This routine is called from a task.
*
*/
int USB_OS_WaitTimed(unsigned EPIndex, unsigned ms) {
  int r;
#ifndef DISABLE_EMBOS  
  r = (int)OS_EVENT_WaitTimed(&_aEvent[EPIndex], ms + 1);
#endif  
  return r;
}

/**********************************************************
*
*        USB_OS_Panic
*
* Function description
*   Called if fatal error is detected.
*
* Add. info
*   Error codes:
*     1   USB_ERROR_RX_OVERFLOW
*     2   USB_ERROR_ILLEGAL_MAX_PACKET_SIZE
*     3   USB_ERROR_ILLEGAL_EPADDR
*     4   USB_ERROR_IBUFFER_SIZE_TOO_SMALL
*/
void USB_OS_Panic(unsigned ErrCode) {
  while (ErrCode);
}

/**********************************************************
*
*        USB_OS_GetTickCnt
*
* Function description
*   Returns the current system time in ticks.
*/
U32 USB_OS_GetTickCnt(void) {
  return OS_Time;
}

/*************************** End of file ****************************/

#endif
