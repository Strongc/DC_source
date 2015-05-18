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
File    : USB_CDC.c
Purpose : Implementation of USB CDC function.
----------Literature--------------------------------------------------

--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "USB_Private.h"
#include "USB_Conf.h"
#include "USB_CDC_Private.h"
#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32

/*********************************************************************
*
*       defines
*
**********************************************************************
*/
#define CS_INTERFACE                    0x24

#define SET_USB_CDC_LINE_CODING         0x20
#define GET_USB_CDC_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE          0x22
#define SEND_BREAK                      0x23

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
// data structure for GET_USB_CDC_LINE_CODING / SET_USB_CDC_LINE_CODING class requests

/*********************************************************************
*
*       static const
*
**********************************************************************
*/


/*********************************************************************
*
*       static data
*
**********************************************************************
*/

static USB_CDC_INIT_DATA                  _InitData;
static USB_CDC_LINE_CODING                _LineCoding = {115200, 0, 0, 8};
static U8                                 _SetLineCodingPending;
static USB_CDC_ON_SET_LINE_CODING        *_pfOnSetLineCoding;
static USB_CDC_ON_SET_CONTROL_LINE_STATE *_pfOnSetControlLineState;
static U8 _aStatus[] = {
  0xA1, 0x20, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0x00,
  0x00, 0x00
};

/*********************************************************************
*
*       static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _OnRxEP0
*
*  Function description
*    Called when data is received on EP0.
*    This is typically a "SetLineCoding" Packet.
*/
static void _OnRxEP0(const U8 * pData, unsigned NumBytes) {
  if (_SetLineCodingPending) {
    if (NumBytes != 7) {
#if USB_DEBUG_LEVEL > 0
      USB_DEBUGOUT("CDC: Warning: Received an unrecognized packet on EP0.\n");
      USB_StallEP(0);
#endif
    } else {
      USB_MEMCPY(&_LineCoding, pData, NumBytes);
      CDC_DPRINT(("DTERate=%u, CharFormat=%u, ParityType=%u, DataBits=%u\n",
                    _LineCoding.DTERate,
                    _LineCoding.CharFormat,
                    _LineCoding.ParityType,
                    _LineCoding.DataBits));
      if (_pfOnSetLineCoding) {
        _pfOnSetLineCoding(&_LineCoding);
      }
    }
    _SetLineCodingPending = 0;
    USB__WriteEP0FromISR((void*)1, 0, 1);
  }
}

/*********************************************************************
*
*       _OnClassRequest
*
*  Function description
*/
static int _OnClassRequest(const USB_SETUP_PACKET * pSetupPacket) {
  switch (pSetupPacket->bRequest) {
  // set line coding
  case SET_USB_CDC_LINE_CODING:
    CDC_DPRINT(("SET_USB_CDC_LINE_CODING\n"));
    _SetLineCodingPending = 1;
    return 0;
  // get line coding
  case GET_USB_CDC_LINE_CODING:
    CDC_DPRINT("GET_USB_CDC_LINE_CODING\n");
    USB__WriteEP0FromISR((U8 *)&_LineCoding, 7, 0);
    return 0;
  // set control line state
  case SET_CONTROL_LINE_STATE:
    // bit0 = DTR, bit = RTS
    CDC_DPRINT(("SET_CONTROL_LINE_STATE %X\n", pSetupPacket->wValueLow));
    if (_pfOnSetControlLineState) {
      USB_CDC_CONTROL_LINE_STATE CLState;
      
      CLState.DTR = pSetupPacket->wValueLow & (1 << 0) ? 1 : 0;
      CLState.RTS = pSetupPacket->wValueLow & (1 << 1) ? 1 : 0;
      _pfOnSetControlLineState(&CLState);
    }
    USB__WriteEP0FromISR((void*)1, 0, 1);
    return 0;
  case SEND_BREAK:
    CDC_DPRINT(("SET_CONTROL_LINE_STATE %X\n", pSetupPacket->wValueHigh));
    USB__WriteEP0FromISR((void*)1, 0, 1);
    return 0;
  default:
    break;
  }
  return 1;
}

/*********************************************************************
*
*       _AddFuncDesc
*
*  Function description
*/
static void _AddFuncDesc(USB_INFO_BUFFER * pInfoBuffer) {
  //
  // Header functional desc
  //
  USB_IB_AddU8(pInfoBuffer,  0x05);
  USB_IB_AddU8(pInfoBuffer,  CS_INTERFACE);
  USB_IB_AddU8(pInfoBuffer,  0x00);
  USB_IB_AddU16(pInfoBuffer, 0x0110);
  //
  // Call management functional desc
  //
  USB_IB_AddU8(pInfoBuffer, 0x05);
  USB_IB_AddU8(pInfoBuffer, CS_INTERFACE);
  USB_IB_AddU8(pInfoBuffer, 0x01);
  USB_IB_AddU8(pInfoBuffer, 0x01);  // bmCapabilities = device handles call management
  USB_IB_AddU8(pInfoBuffer, 0x01);  // bDataInterface
  //
  // ACM functional desc
  //
  USB_IB_AddU8(pInfoBuffer, 0x04);
  USB_IB_AddU8(pInfoBuffer, CS_INTERFACE);
  USB_IB_AddU8(pInfoBuffer, 0x02);
  USB_IB_AddU8(pInfoBuffer, 0x06); // bmCapabilities: Supports Send_Break, Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, and the notification Serial_State
  //
  // Union functional desc
  USB_IB_AddU8(pInfoBuffer, 0x05);
  USB_IB_AddU8(pInfoBuffer, CS_INTERFACE);
  USB_IB_AddU8(pInfoBuffer, 0x06);
  USB_IB_AddU8(pInfoBuffer, 0x00); // bMasterInterface
  USB_IB_AddU8(pInfoBuffer, 0x01);// bSlaveInterface0
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       USB_CDC_Add
*
*  Function description:
*    
*
*  Parameters:
*    pInitData    - 
*  
*/
void USB_CDC_Add(const USB_CDC_INIT_DATA * pInitData) {
  unsigned InterfaceNum;
  unsigned FirstInterfaceNo;
  INTERFACE * pInterface;


  USB_SetOnRxEP0(_OnRxEP0);
  InterfaceNum     = USB__AllocIF();
  FirstInterfaceNo = InterfaceNum;
  USB_SetClassRequestHook(InterfaceNum, _OnClassRequest);
  pInterface                = &USB_Global.aIF[InterfaceNum];
  pInterface->pfAddFuncDesc = _AddFuncDesc;
  pInterface->IFClass       = 2;
  pInterface->IFSubClass    = 2;
  pInterface->IFProtocol    = 1;
  pInterface->EPs           = (1 << pInitData->EPInt);

  InterfaceNum              = USB__AllocIF();
  pInterface                = &USB_Global.aIF[InterfaceNum];
  pInterface->pfAddFuncDesc = NULL;
  pInterface->IFClass       = 0x0A;
  pInterface->IFSubClass    = 0;
  pInterface->IFProtocol    = 0;
  pInterface->EPs           = (1 << pInitData->EPIn) | (1 << pInitData->EPOut);

  if (USB_Global.pIadAPI) {
    USB_Global.pIadAPI->pfAdd((U8)FirstInterfaceNo, 2, 2, 0, 0);
  } else {
    USB_Global.Class    = 2;
    USB_Global.SubClass = 0;
    USB_Global.Protocol = 0;
  }
  _InitData = *pInitData;
}

/*********************************************************************
*
*       USB_CDC_SetOnLineCoding
*
*  Function description:
*    
*
*  Parameters:
*    pf    - 
*  
*/
void USB_CDC_SetOnLineCoding(USB_CDC_ON_SET_LINE_CODING * pf) {
  _pfOnSetLineCoding = pf;
}

/*********************************************************************
*
*       USB_CDC_SetOnLineCoding
*
*  Function description:
*    
*
*  Parameters:
*    pf    - 
*  
*/
void USB_CDC_SetOnControlLineState(USB_CDC_ON_SET_CONTROL_LINE_STATE * pf) {
  _pfOnSetControlLineState = pf;
}

/*********************************************************************
*
*       USB_CDC_Write
*
*  Function description:
*    
*
*  Parameters:
*    pData     - 
*    NumBytes  - 
*  
*/
void USB_CDC_Write(const void * pData, unsigned NumBytes) {
  USB_WriteEP(_InitData.EPIn, pData, NumBytes, 1);
}

/*********************************************************************
*
*       USB_CDC_Read
*
*  Function description:
*    
*
*  Parameters:
*    pData     - 
*    NumBytes  - 
*  
*/
int USB_CDC_Read(void * pData, unsigned NumBytes) {
  return USB_ReadEP(_InitData.EPOut, pData, NumBytes);
}


/*********************************************************************
*
*       USB_CDC_Receive
*
*  Function description:
*    
*
*  Parameters:
*    pData     - 
*    NumBytes  - 
*  
*/
int USB_CDC_Receive(void * pData, unsigned NumBytes) {
  return USB_ReceiveEP(_InitData.EPOut, pData, NumBytes);
}


/*********************************************************************
*
*       USB_CDC_ReceiveTimed
*
*  Function description:
*    
*
*  Parameters:
*    pData     - 
*    NumBytes  - 
*  
*/
int  USB_CDC_ReceiveTimed(void * pData, unsigned NumBytes, unsigned ms) {
  return USB_ReceiveEPTimed(_InitData.EPOut, pData, NumBytes, ms);
}

/*********************************************************************
*
*       USB_CDC_ReadOverlapped
*
*  Function description:
*    
*
*  Parameters:
*    pData    - 
*    NumBytes - 
*  
*/
int USB_CDC_ReadOverlapped(void * pData, unsigned NumBytes) {
  return USB_ReadEPOverlapped(_InitData.EPOut, pData, NumBytes);
}


/*********************************************************************
*
*       USB_CDC_WriteOverlapped
*
*  Function description:
*    
*
*  Parameters:
*    pData     - 
*    NumBytes  - 
*  
*/
int USB_CDC_WriteOverlapped(const void * pData, unsigned NumBytes) {
	USB_Global.OnTxBehavior = 1;

  return USB_WriteEPOverlapped(_InitData.EPIn, pData, NumBytes, 1);
}

/*********************************************************************
*
*       USB_CDC_WaitForRX
*
*  Function description:
*    
*
*/
void USB_CDC_WaitForRX(void) {
  USB_WaitForEndOfTransfer(_InitData.EPOut);
}

/*********************************************************************
*
*       USB_CDC_WaitForTX
*
*  Function description:
*    
*
*/
void USB_CDC_WaitForTX(void) {
  USB_WaitForEndOfTransfer(_InitData.EPIn);
}

/*********************************************************************
*
*       USB_CDC_WriteTimed
*/
int USB_CDC_WriteTimed(const void* pData, unsigned NumBytes, unsigned ms) {
  return USB_WriteEPTimed(_InitData.EPIn, pData, NumBytes, 1, ms);
}


/*********************************************************************
*
*       USB_CDC_ReadTimed
*/
int USB_CDC_ReadTimed(void* pData, unsigned NumBytes, unsigned ms) {
  return USB_ReadEPTimed(_InitData.EPOut, pData, NumBytes, ms);
}


/*********************************************************************
*
*       USB_CDC_CancelRead
*/
void USB_CDC_CancelRead() {
  USB_CancelIO(_InitData.EPOut);
}

/*********************************************************************
*
*       USB_CDC_CancelWrite
*/
void USB_CDC_CancelWrite() {
  USB_CancelIO(_InitData.EPIn);
}

/*********************************************************************
*
*       USB_CDC_WriteSerialState
*/
void USB_CDC_WriteSerialState(void) {
  USB_WriteEPTimed(_InitData.EPInt, _aStatus, sizeof(_aStatus), 0, 10);
}

/*********************************************************************
*
*       USB_CDC_UpdateSerialState
*/
void USB_CDC_UpdateSerialState(USB_CDC_SERIAL_STATE * pSerialState) {
  U8 State;
  
  State = 0;
  if (pSerialState->DCD & 1) {
    State |= (1 << 0);
  }
  if (pSerialState->DSR & 1) {
    State |= (1 << 1);
  }
  if (pSerialState->Break & 1) {
    State |= (1 << 2);
  }
  if (pSerialState->Ring & 1) {
    State |= (1 << 3);
  }
  if (pSerialState->FramingError & 1) {
    State |= (1 << 4);
  }
  if (pSerialState->ParityError & 1) {
    State |= (1 << 5);
  }
  if (pSerialState->OverRunError & 1) {
    State |= (1 << 6);
  }
  if (pSerialState->CTS & 1) {
    State |= (1 << 7);
  }
 _aStatus[8] = State;
}

/**************************** end of file ***************************/

