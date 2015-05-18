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
File    : USB_HID.c
Purpose : Implementation of human interface device class.
----------Literature--------------------------------------------------
--------  END-OF-HEADER  ---------------------------------------------
*/

#include <stdio.h>
#include "USB_Private.h"
#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U8 ClassNo;
  U8 SubClassNo;
  U8 ProtocolNo;
  U8 NumInterfaces;
  U8 FirstInterface;
} IAD_DESC;
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
static IAD_DESC  _aIADDesc[USB_MAX_NUM_IAD];
static U8        _NumIADs;

/*********************************************************************
*
*       static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _SetClass
*
*  Description:
*    When using IAD, every information about the USB class
*    in the device descriptor will be in the IAD.
*    
*
*/
static void _Add(U8 FirstInterFaceNo, U8 NumInterfaces, U8 ClassNo, U8 SubClassNo, U8 ProtocolNo) {
  IAD_DESC * pIad;
  if (++_NumIADs > USB_MAX_NUM_IAD) {
    USB_OS_Panic(USB_ERROR_IAD_DESCRIPTORS_EXCEED);
  }
  pIad = &_aIADDesc[_NumIADs - 1];
  pIad->FirstInterface = FirstInterFaceNo;
  pIad->NumInterfaces  = NumInterfaces;
  pIad->ClassNo        = ClassNo;
  pIad->ProtocolNo     = ProtocolNo;
  pIad->SubClassNo     = SubClassNo;
}

/*********************************************************************
*
*       _AddIadDesc
*
*  Description:
*    
*
*/
static void _AddIadDesc(int InterFaceNo, USB_INFO_BUFFER * pInfoBuffer) {
  int i;

  for (i = 0; i < USB_MAX_NUM_IAD; i++) {
    IAD_DESC * pIad;
    pIad = &_aIADDesc[i];
    if (pIad->FirstInterface == InterFaceNo) {
      USB_IB_AddU8(pInfoBuffer, 0x08);                  // Length of IAD
      USB_IB_AddU8(pInfoBuffer, USB_DESC_TYPE_IAD);     // Descriptor type
      USB_IB_AddU8(pInfoBuffer, pIad->FirstInterface);  // First Interface
      USB_IB_AddU8(pInfoBuffer, pIad->NumInterfaces);   // Interface count used for this class
      USB_IB_AddU8(pInfoBuffer, pIad->ClassNo);         // Class of the USB function
      USB_IB_AddU8(pInfoBuffer, pIad->SubClassNo);      // Subclass of the USB function
      USB_IB_AddU8(pInfoBuffer, pIad->ProtocolNo);      // Protocol no. of the USB function
      USB_IB_AddU8(pInfoBuffer, 0);                     // String descriptor index - 0 -> not available
      break;
    }
  }
}

/*********************************************************************
*
*       Public code
* 
**********************************************************************
*/
static USB_IAD_API _IadAPI = {
  _Add,
  _AddIadDesc
};

/*********************************************************************
*
*       USB_EnableIAD
*
*  Description:
*    Allows to create a USB function composite device with 
*    devices classes that use multiple interfaces to describe 
*    the class such as CDC.
*
*/
void USB_EnableIAD(void) {
  USB_MEMSET(_aIADDesc, 0xff, sizeof(_aIADDesc));
  _NumIADs  = 0;
  USB_Global.pIadAPI = &_IadAPI;
  USB_Global.Class    = 0xEF;    // Interface Class
  USB_Global.SubClass = 0x02;    // Interface Subclass
  USB_Global.Protocol = 0x01;    // Interface Protocol

}

/**************************** end of file ***************************/

