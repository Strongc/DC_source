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
File    : USB_Setup.c
Purpose : Handling of enumeration and setup
          Do not modify to allow easy updates !

Literature
  [1] Universal Serial Bus Specification Revision 2.0

--------  END-OF-HEADER  ---------------------------------------------
*/


#include "USB_Private.h"
#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32
#include <stdlib.h>

#ifndef   USB_DESC_BUFFER_SIZE
  #define USB_DESC_BUFFER_SIZE  256
#endif

#ifndef   USB_MAX_POWER
  #define USB_MAX_POWER  50
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 _aDescBuffer[USB_DESC_BUFFER_SIZE];
static U8 _MaxPower = USB_MAX_POWER;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _strlen
*/
static unsigned _strlen(const char * s) {
  int r = 0;
  while (*s++) {
    r++;
  }
  return r;
}

/*********************************************************************
*
*       _CountBits
*/
static int _CountBits(unsigned Mask) {
  int NumBits;

  NumBits = 0;
  while (Mask) {
    if (Mask & 1) {
      NumBits++;
    }
    Mask >>= 1;
  }
  return NumBits;
}

/*********************************************************************
*
*       _CalcInterval
*
*  Function description
*    Computes the interval parameter for the end point descriptor
*    according to USB Spec, 9.6.6
*
*/
static U8 _CalcInterval(unsigned EPIndex) {
  U16 Interval;

  switch (USB_aEPStat[EPIndex].EPType) {
  case USB_TRANSFER_TYPE_INT:
    Interval = USB_aEPStat[EPIndex].Interval;
    
    if (USB__IsHighSpeedMode() == 0) { // low-full speed
      if (Interval > 255) {
        Interval = 255;
      } else if (Interval == 0) {
        Interval = 8;  // set Interval of 8ms on INT EP
      }
#if 0 // !!!! Low-speed interrupt endpoints are limited from 10ms to 255ms.
      if (Interval < 10) {
          Interval = 10;
      }
#endif
    } else { // high-speed
      if (Interval > 16) {
        Interval = 16;
      } else if (Interval == 0) {
        Interval = 4;  // set Interval of 1ms on INT EP
      }
    }
    break;
  case USB_TRANSFER_TYPE_ISO:
    Interval = USB_aEPStat[EPIndex].Interval;
    if (Interval > 16) {
      Interval = 16;
    } else if (Interval == 0) {
      Interval = 1;  // set Interval of 125us on ISO EP
    }
    break;
  default:
    Interval = 1;
    break;
  }
  return (U8)Interval;
}


/*********************************************************************
*
*       USB_IB_Init
*/
void USB_IB_Init(USB_INFO_BUFFER * pInfoBuffer, U8 * pBuffer, unsigned SizeofBuffer) {

  pInfoBuffer->Cnt = 0;
  pInfoBuffer->pBuffer  = pBuffer;
  pInfoBuffer->Sizeof   = SizeofBuffer;
  
}

/*********************************************************************
*
*       USB_IB_AddU8
*/
void USB_IB_AddU8(USB_INFO_BUFFER * pInfoBuffer, U8 Data) {
  if (pInfoBuffer->Cnt < pInfoBuffer->Sizeof) {
    *(pInfoBuffer->pBuffer + pInfoBuffer->Cnt) = Data;
    pInfoBuffer->Cnt++;
  }
#if USB_DEBUG_LEVEL
  else {
    USB_OS_Panic(USB_ERROR_IBUFFER_SIZE_TOO_SMALL);              // Buffer to small
  }
#endif
}

/*********************************************************************
*
*       USB_IB_AddU16
*/
void USB_IB_AddU16(USB_INFO_BUFFER * pInfoBuffer, U16 Data) {
  USB_IB_AddU8(pInfoBuffer, (U8)(Data & 255));
  USB_IB_AddU8(pInfoBuffer, (U8)(Data >> 8));
}

/*********************************************************************
*
*       USB_IB_AddU32
*/
void USB_IB_AddU32(USB_INFO_BUFFER * pInfoBuffer, U32 Data) {
  USB_IB_AddU8(pInfoBuffer, (U8)(Data &  255));
  USB_IB_AddU8(pInfoBuffer, (U8)(Data >>   8));
  USB_IB_AddU8(pInfoBuffer, (U8)(Data >>  16));
  USB_IB_AddU8(pInfoBuffer, (U8)(Data >>  24));
}


#if USB_SUPPORT_HIGH_SPEED
/*********************************************************************
*
*       _BuildOtherSpeedConfigDesc
*
*  Notes
*    (1) The config descriptor (9.6.3 of the spec) includes interface and
*        endpoint descriptors.
*/
static const U8 * _BuildOtherSpeedConfigDesc(void) {
  USB_INFO_BUFFER   InfoBuffer;
  INTERFACE       * pInterface;
  int               i;
  int               iIF;

  USB_IB_Init(&InfoBuffer, &_aDescBuffer[0], sizeof(_aDescBuffer));
  //
  // Add the real config descriptor, which includes interface and endpoint descriptors.  (9 bytes)
  //
  USB_IB_AddU8 (&InfoBuffer, 9);                                  // Descriptor len of first part of config descriptor
  USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_SPEED_CONFIG);         // Descriptor type (2)
  USB_IB_AddU16(&InfoBuffer, 0);                                  // Data length - to be filled in later
  USB_IB_AddU8 (&InfoBuffer, USB_Global.NumIFs);                  // Number of Interfaces Supported
  USB_IB_AddU8 (&InfoBuffer, 1);                                  // Config value
  USB_IB_AddU8 (&InfoBuffer, 0);                                  // iConfig: index of config string descriptor
  USB_IB_AddU8 (&InfoBuffer, 0xc0);                               // Config Attributes (Self Powered)
  USB_IB_AddU8 (&InfoBuffer, USB_MAX_POWER);                      // Max Power ([2mA] ... e.g. 50 means 100 mA)
  for (iIF = 0; iIF < USB_Global.NumIFs; iIF++) {
    int NumEPs;
    pInterface = &USB_Global.aIF[iIF];
    NumEPs = _CountBits(pInterface->EPs >> 1);     // Do not count EP0
    //
    // Add interface descriptor.  (9 bytes)
    //
    USB_IB_AddU8 (&InfoBuffer, 9);                           // Descriptor len
    USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_INTERFACE);     // Descriptor type: 4
    USB_IB_AddU8 (&InfoBuffer, (U8)iIF);                           // Interface Number
    USB_IB_AddU8 (&InfoBuffer, 0);                        // Alternate settings
    USB_IB_AddU8 (&InfoBuffer, (U8)NumEPs);                 // No of used EPs
    USB_IB_AddU8 (&InfoBuffer, pInterface->IFClass);    // Interface Class
    USB_IB_AddU8 (&InfoBuffer, pInterface->IFSubClass); // Interface Subclass
    USB_IB_AddU8 (&InfoBuffer, pInterface->IFProtocol); // Interface Protocol
    USB_IB_AddU8 (&InfoBuffer, 0);                      // Interface Descriptor String
    //
    // Add Functional descriptor via optional callback
    //
    if (pInterface->pfAddFuncDesc) {
      pInterface->pfAddFuncDesc(&InfoBuffer);
    }
    //
    // Add Endpoint descriptors.  (9 bytes)
    //
    for (i = 1; i < USB_Global.NumEPs; i++) {
      if (pInterface->EPs & (1 << (i))) {
        U16 MaxPacketSize;
        U8  Interval;
        USB_IB_AddU8 (&InfoBuffer, 7);                             // Descriptor len
        USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_ENDPOINT);        // Descriptor type: 5
        USB_IB_AddU8 (&InfoBuffer, USB_aEPStat[i].EPAddr);         //
        USB_IB_AddU8 (&InfoBuffer, USB_aEPStat[i].EPType);         // Endpoint Attributes
        if (USB__IsHighSpeedMode() == 0) {
          MaxPacketSize = USB_Global.pDriver->pfGetMaxPacketSize((U8)i);
          MaxPacketSize = USB__CalcMaxPacketSize(MaxPacketSize, USB_aEPStat[i].EPType, 1);
        } else {
          MaxPacketSize = USB__CalcMaxPacketSize(USB_aEPStat[i].MaxPacketSize, USB_aEPStat[i].EPType, 0);
        }
        USB_IB_AddU16(&InfoBuffer, MaxPacketSize);  // Max Packet Size
        Interval = _CalcInterval(i);
        USB_IB_AddU8 (&InfoBuffer, Interval);                       // Polling Interval
      }
    }
  }
  //
  // Fill in length of config descriptor
  //
  _aDescBuffer[2] = InfoBuffer.Cnt;
  return _aDescBuffer;
}


/*********************************************************************
*
*       _BuildDeviceQualDesc
*
*  Notes
*    (1)
*/
static const U8 * _BuildDeviceQualDesc(void) {
  USB_INFO_BUFFER InfoBuffer;

  USB_IB_Init  (&InfoBuffer, &_aDescBuffer[0], sizeof(_aDescBuffer));
  USB_IB_AddU8 (&InfoBuffer, 10);                             // desc length
  USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_QUALIFIER);        // desc type
  USB_IB_AddU16(&InfoBuffer, 0x200);                          // USB Spec Rev (Low Order)
  USB_IB_AddU8 (&InfoBuffer, USB_Global.Class);               // Class
  USB_IB_AddU8 (&InfoBuffer, USB_Global.SubClass);            // Device Subclass
  USB_IB_AddU8 (&InfoBuffer, USB_Global.Protocol);            // Device Protocol
  USB_IB_AddU8 (&InfoBuffer, (U8)USB_aEPStat[0].MaxPacketSize);   // Packet Size         8/16/32/64 are valid
  USB_IB_AddU8 (&InfoBuffer, 0x01);                           // bNumConfigurations: Number of Configurations
  USB_IB_AddU8 (&InfoBuffer, 0x00);                           // Reserved
  return _aDescBuffer;
}
#endif

/*********************************************************************
*
*       _GetString
*
*  Function description
*    (1)
*/
static const char * _GetString(unsigned StringIndex) {
  switch (StringIndex) { // Send String Descriptor
  case STRING_INDEX_MANUFACTURER:   return USB_GetVendorName();
  case STRING_INDEX_PRODUCT:        return USB_GetProductName();
  case STRING_INDEX_SN:             return USB_GetSerialNumber();
  case 238:                         return "";
  }
  return NULL;
}




/*********************************************************************
*
*       _BuildStringDesc
*
*  Notes
*    (1)
*/
static const U8 * _BuildStringDesc(unsigned StringIndex) {
  USB_INFO_BUFFER InfoBuffer;
  unsigned i;
  unsigned NumChars;
  const char * s;

  //
  // String Index == 0 means Language identifier.
  //
  if (StringIndex == 0) {
    USB_DEBUGOUT(":Lang");
    USB_IB_Init(&InfoBuffer, &_aDescBuffer[0], sizeof(_aDescBuffer));
    USB_IB_AddU8(&InfoBuffer, 4);
    USB_IB_AddU8(&InfoBuffer, USB_DESC_TYPE_STRING);
    USB_IB_AddU16(&InfoBuffer, 0x409);
  } else {
    //
    // Get string from application
    //
    s = _GetString(StringIndex);
    if (s == NULL) {
      return NULL;
    }
    //
    // Build the descriptor from string
    //
    NumChars = _strlen(s);
    USB_IB_Init(&InfoBuffer, &_aDescBuffer[0], sizeof(_aDescBuffer));
    USB_IB_AddU8(&InfoBuffer, (U8)(2 + 2 * NumChars));
    USB_IB_AddU8(&InfoBuffer, USB_DESC_TYPE_STRING);
    for (i = 0; i < NumChars; i++) {
      U16 Char16;
      Char16 = *(s + i);
      USB_IB_AddU16(&InfoBuffer, Char16);
    }
  }
  return _aDescBuffer;
}

/*********************************************************************
*
*       _WriteEP0
*/
static void _WriteEP0(const U8 * pData, unsigned NumBytes, char Send0PacketIfRequired) {
  USB__WriteEP0FromISR(pData, NumBytes, Send0PacketIfRequired);
}

/*********************************************************************
*
*       _WriteEP0_NULL
*/
static void _WriteEP0_NULL(void) {
  _WriteEP0((U8 *)1, 0, 1);
}

/*********************************************************************
*
*       _CmdSetAddress
*
* Function description
*    Takes address assigned by the HOST and enters it into the USBA
*    register. This request is different depending on whether the
*    device already has an assigned address or not. Device enters the
*    ADDRESSED state at this point.
*
*/
static void _CmdSetAddress(U8 Addr) {
/*** Surendra: Modified to call the Setaddress function ****/
//  if (USB_Global.SetAddressBehavior) {
    if (1) {

    USB_Global.pDriver->pfSetAddress(Addr);
    if (Addr) {
      USB__OnStatusChange((U8)(USB_Global.State | USB_STAT_ADDRESSED));
    } else {
      USB__OnStatusChange((U8)(USB_Global.State & ~USB_STAT_ADDRESSED));
    }
    _WriteEP0_NULL();
  } else {
    _WriteEP0_NULL();
    USB_Global.Addr = Addr | 0x80;
  }
}

/*********************************************************************
*
*       _CmdGetDescriptor
* Function description
*   Determines what descriptor string is to be returned to the HOST.
*   Calls SendEP0Data to transfer data to HOST.
*
*/
static void _CmdGetDescriptor(const USB_SETUP_PACKET * pSetupPacket) {
  unsigned  Len = 0;
  const U8* pData = 0;
  unsigned  MaxLen;
  char      Send0PacketIfRequired;

  MaxLen = ((pSetupPacket->wLengthHigh) << 8) | (pSetupPacket->wLengthLow);
  Send0PacketIfRequired = 1;

  switch (pSetupPacket->wValueHigh) {    // Mask to determine what descriptor is being asked
  case 1:                   // Send Device Descriptor
    USB_DEBUGOUT("-Device");
    pData = USB__BuildDeviceDesc();
    break;
  case 2:                   // Send Config Descriptor
    USB_DEBUGOUT("-Config");
    pData = USB__BuildConfigDesc();
    Len   = *(pData + 2); // Size of string
    break;
  case 3:
    USB_DEBUGOUT("-String");
    pData = _BuildStringDesc(pSetupPacket->wValueLow);
    break;
#if USB_SUPPORT_HIGH_SPEED
  case 0x06:
    USB_DEBUGOUT("-DeviceQualifier");
    if (USB__IsHighSpeedCapable()) {
      pData = _BuildDeviceQualDesc();
      Len   = *(pData + 2); // Size of string
    }
    break;
  case 0x07:
    USB_DEBUGOUT("-OtherSpeedSettings");
    if (USB__IsHighSpeedCapable()) {
      pData = _BuildOtherSpeedConfigDesc();
      Len   = *(pData + 2); // Size of string
    }
    break;
#endif
  }
  if (pData == NULL) {     // Unhandled descriptor type
    USB_Global.pDriver->pfStallEP0();
    return;
  }
  if (Len == 0) {
    Len = pData[0];    // Get size of string
  }
  //
  // Make sure we send no more data than what the host asks for
  //
  if (Len >= MaxLen) {
    Len = MaxLen;
  }
  //
  // Send the data
  //
  _WriteEP0(pData, Len, Send0PacketIfRequired);
}

/*********************************************************************
*
*       _CmdGetStatus
* Function description
*   Returns various status information to the HOST about the state of the device
*   only valid in the Default and Configured State. If a request for status is
*   made that is invalid, then the device STALLs the request per the USB spec.
*   Note that there is no valid information for INTERFACE status, but the
*   device just returns 0x00 0x00 per the spec. Valid endpoint status is
*   also returned with this command. Linked to SET/CLEAR FEATURE requests.
*   Note also that word fields are converted to bytes here locally for ease
*   of code readability.
*
*/
static void _CmdGetStatus(const USB_SETUP_PACKET * pSetupPacket) {
  U8 aData[2];
  U16 wValue, wLength;
  unsigned EPIndex;
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  wValue  = pSetupPacket->wValueLow  | pSetupPacket->wValueHigh << 8;
  wLength = pSetupPacket->wLengthLow | pSetupPacket->wLengthHigh << 8;

  if ((wValue != 0x00) || (wLength != 0x02)) {
    pDriver->pfStallEP0();
    return;       // send STALL and leave function
  }
  if ((USB_Global.State & USB_STAT_ADDRESSED) == 0) {   // Response not defined in USB spec in default state
    pDriver->pfStallEP0();
    return;
  }
  if ((USB_Global.State & USB_STAT_CONFIGURED) == 0) {    // If device is in Address state, use these responses
    switch (pSetupPacket->bmRequestType & 0x03) {
    case 0:                    // Device status
      aData[0] = 0x00;
      aData[1] = 0x00;         // Self powered and no remote wakeup support
      _WriteEP0(&aData[0], sizeof(aData), 0);
      break;                   // exit function
    case 1:                    // Interface status
      aData[0] = 0x00;
      aData[1] = 0x00;         // Return 2
      _WriteEP0(&aData[0], sizeof(aData), 0);
      break;
    case 2:                    // Endpoint status
      switch (pSetupPacket->wIndexLow) {     // Determine which endpoint
      case 0:                  // Only EP0 responds here
        aData[0] = 0x00;       // fill byte per spec
        aData[1] = 0x00;       // fill byte per spec
        _WriteEP0(&aData[0], sizeof(aData), 0);
        break;                  // exit function
      default:
        pDriver->pfStallEP0();
      }
    }
    return;
  }
  switch (pSetupPacket->bmRequestType & 0x03) {  // Configured responses
  case 0:           // Device status
    aData[0] = 0x01;
    break;          // exit function
  case 1:           // Interface status
    aData[0] = 0x00;
    break;            // exit function
  case 2:             // Endpoint status
    EPIndex = USB__EPAddr2Index(pSetupPacket->wIndexLow);
    if (EPIndex == 0xff) {
      pDriver->pfStallEP0();
      return;
    }
    aData[0] = USB_aEPStat[EPIndex].IsHalted;
  }
  aData[1] = 0x00;
  _WriteEP0(&aData[0], sizeof(aData), 0);
}

/*********************************************************************
*
*       _SetClrFeature
*
*  Notes
*    [1] 9.4.1 Clear Feature
*/
static void _SetClrFeature(const USB_SETUP_PACKET * pSetupPacket, int SetFeature) {
  U16             wValue, wLength;
  unsigned        EPIndex;
  U8              OnOff;
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  wValue  = pSetupPacket->wValueLow  | pSetupPacket->wValueHigh << 8;
  wLength = pSetupPacket->wLengthLow | pSetupPacket->wLengthHigh << 8;
  OnOff   = SetFeature;
  if ((wValue != 0x00) || (wLength != 0x00)) {
    pDriver->pfStallEP0();
    return;       // send STALL and leave function
  }
  if ((USB_Global.State & USB_STAT_ADDRESSED) == 0) {   // Response not defined in USB spec in default state
    pDriver->pfStallEP0();
    return;
  }
  //
  // Handle addressed state (not configured)
  //
  if ((USB_Global.State & USB_STAT_CONFIGURED) == 0) {    // If device is in Address state, use these responses
    switch (pSetupPacket->bmRequestType & 0x03) {
    case 0:                    // Set/Clear Device status
      _WriteEP0_NULL();
      break;                   // exit function
    case 1:                    // Set/Clear Interface
      pDriver->pfStallEP0();
      break;
    case 2:                    // Endpoint status
      EPIndex = USB__EPAddr2Index(pSetupPacket->wIndexLow);
      if (EPIndex == 0) {                 // In addressed state, only EP0 is permitted
        if (SetFeature) {
          USB_aEPStat[EPIndex].IsHalted = 1;
        } else {
          USB_aEPStat[EPIndex].IsHalted = 0;
        }
        _WriteEP0_NULL();
      } else {
        pDriver->pfStallEP0();
      }
    }
    return;
  }
  //
  // Handle configured state
  //
  switch (pSetupPacket->bmRequestType & 0x03) {  // Configured responses
  case 0:                    // Set/Clear Device status
    _WriteEP0_NULL();
    break;                   // exit function
  case 1:                    // Set/Clear Interface
    pDriver->pfStallEP0();
    break;
  case 2:                    // Endpoint status
    EPIndex = USB__EPAddr2Index(pSetupPacket->wIndexLow);
    if (EPIndex == 0xff) {
      pDriver->pfStallEP0();
      return;
    }
    USB_SetClrStallEP((U8)EPIndex, OnOff);
    if (OnOff == 0) {
      USB__ResetDataToggleEP((U8)EPIndex);
      USB__InvalidateEP((U8)EPIndex);
    }
    _WriteEP0_NULL();
    break;                  // exit function
  default:
    pDriver->pfStallEP0();
  }
}



/*********************************************************************
*
*       _CmdClearFeature
*
* Function description
*   The USB specification supports the clearing of certain 'features'
*   of a device. These can include remote wakeup support and endpoint
*   halting (STALL). Since this firmware does not support device
*   or interface features, those requests are stalled. Only endpoints
*   which exist are allowed to be unSTALLed with this request.
*/
static void _CmdClearFeature(const USB_SETUP_PACKET * pSetupPacket) {
  _SetClrFeature(pSetupPacket, 0);
}

/*********************************************************************
*
*       _CmdSetFeature
*
* Function description
*   The USB specification supports the setting of certain 'features'
*   of a device. These can include remote wakeup support and endpoint
*   halting (STALL). Since this firmware does not support device
*   or interface features, those requests are stalled. Only endpoints
*   which exist are allowed to be STALL with this request.
*/
static void _CmdSetFeature(const USB_SETUP_PACKET * pSetupPacket) {
  _SetClrFeature(pSetupPacket, 1);
}

/*********************************************************************
*
*       _CmdGetConfiguration
*
* Function description
*   Returns the current configuration value to the HOST. If the device
*   is unconfigured then this request should return 0x00. Associated
*   with the SET_CONFIGURATION request.
*
*/
static void _CmdGetConfiguration(void) {
  U8 IsConfigured;
  IsConfigured = USB_IsConfigured();
  _WriteEP0(&IsConfigured, 1, 0);
}

/*********************************************************************
*
*       _CmdGetInterface
*
* Function description
*   Returns the correct ALTERNATE_INTERFACE setting of the current
*   configuration. Since there is only one config in this example
*   there exists only 2 alt interfaces controlling the ISO endpoint.
*   If other configurations existed, another level would be needed
*   to check to see if that interface was a valid for the specified
*   configuration. Here the check is just done on the current interface
*   value, making the assumption (incorrectly) that the HOST is always
*   asking about this interface alone.
*
*/
static void _CmdGetInterface(const USB_SETUP_PACKET * pSetupPacket){
  U8 wValue  = pSetupPacket->wValueLow;
  U8 wLength = pSetupPacket->wLengthLow;
  U8 Data;
  if ((wValue != 0x00) || (wLength != 0x01)) {
    USB_Global.pDriver->pfStallEP0();
    return;               // Incorrect values so STALL here
  }
  if (((USB_Global.State & (USB_STAT_ADDRESSED | USB_STAT_CONFIGURED)) != (USB_STAT_ADDRESSED | USB_STAT_CONFIGURED))) {   // If in DEFAULT or ADDRESSED
    USB_Global.pDriver->pfStallEP0();
    return;
  }
  //
  // Send answer (1 byte)
  //
  Data = 0;
  _WriteEP0(&Data, 1, 0);
}

/*********************************************************************
*
*       _CmdSetInterface
*
* Function description
*   The HOST uses this command to set the current interface to a different
*   (alternate) setting. This interface is a subset of it's configuration
*   and allows the HOST to adjust bandwidth or endpoints according to
*   bus loading or need. This is necessary for ISO endpoints so that they can
*   be shut off if bus loading is too great. Alternate interfaces for
*   simple devices (only a few EP's, bulk & interrupts in nature) might
*   only have a singular interface and would not have to include an
*   alt. interface. In that case, the EP descriptors would simpily follow
*   the interface descriptor and both bCurInt and bAltSet would = 0x00.
*
*   Alternate interfaces should be described in order of endpoint usage.
*   For example, Alt. Int. 0 should describe no endpoints (only EP0 active).
*   Alt. Int. 1 can describe the endpoints used and their sizes in increasing size,
*   and so on. Endpoint MAXP adjustments and enabling should be made
*   here, not with the SET_CONFIGURATION command, as it just specifies
*   a configuration to use. This is more advanced that this example, but
*   should be noted here for clarity.
*
*  Notes:
*   (1)  -
*
*/
static void _CmdSetInterface(const USB_SETUP_PACKET * pSetupPacket){
  U8 wValue  = pSetupPacket->wValueLow;
  U8 wIndex  = pSetupPacket->wIndexLow;
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  //
  // If device is neither addressed nor configured, EP0 is stalled, s. note 1
  //
  if (((USB_Global.State & (USB_STAT_ADDRESSED | USB_STAT_CONFIGURED)) != (USB_STAT_ADDRESSED | USB_STAT_CONFIGURED))) {   // If in DEFAULT or ADDRESSED
    pDriver->pfStallEP0();
    return;
  }
  if (wIndex > USB_Global.NumIFs) {
    pDriver->pfStallEP0();
    return;
  }
  if (wValue <= 1) {
    int i;
    _WriteEP0_NULL();
    for (i = 1; i < USB_Global.NumEPs; i++) {
      USB_SetClrStallEP((U8)i, 0);
      USB__ResetDataToggleEP((U8)i);
      USB__InvalidateEP((U8)i);
    }
    return;
  }
  pDriver->pfStallEP0();
}

/*********************************************************************
*
*       _CmdSetConfiguration
*
*  Function description
*    Handles "Set Configuration" requests acc. to [USB] 9.4.7
*
*  Notes
*    (1)  USB Spec
*         The lower byte of the wValue field specifies the desired configuration. This configuration value must be
*         zero or match a configuration value from a configuration descriptor. If the configuration value is zero, the
*         device is placed in its Address state. The upper byte of the wValue field is reserved.
*         If wIndex, wLength, or the upper byte of wValue is non-zero, then the behavior of this request is not
*         specified.
*         Default state: Device behavior when this request is received while the device is in the Default state
*                        is not specified.
*         Address state: If the specified configuration value is zero, then the device remains in the Address
*                        state. If the specified configuration value matches the configuration value from a
*                        configuration descriptor, then that configuration is selected and the device enters the
*                        Configured state. Otherwise, the device responds with a Request Error.
*         Configured state: If the specified configuration value is zero, then the device enters the Address state.
*                        If the specified configuration value matches the configuration value from a
*                        configuration descriptor, then that configuration is selected and the device remains in
*                        the Configured state. Otherwise, the device responds with a Request Error.
*/
static void _CmdSetConfiguration(const USB_SETUP_PACKET * pSetupPacket) {
  U8                    v;
  const USB_HW_DRIVER * pDriver;

  v       = pSetupPacket->wValueLow;
  pDriver = USB_Global.pDriver;
  if (v == 1) {
    U8 i;

    _WriteEP0_NULL();    // Send 0-byte packet as confirmation (status stage)
    USB__OnStatusChange((U8)(USB_Global.State | USB_STAT_CONFIGURED));
    for (i = 1; i < USB_Global.NumEPs; i++) {
      USB_SetClrStallEP(i, 0);
      USB__ResetDataToggleEP(i);
      USB__InvalidateEP(i);
    }
    if (pDriver->pfControl) {
      pDriver->pfControl(USB_DRIVER_CMD_SET_CONFIGURATION, &v);
    }
  } else if (v == 0) {
    _WriteEP0_NULL();    // Send 0-byte packet as confirmation (status stage)
    USB__OnStatusChange((U8)(USB_Global.State & ~USB_STAT_CONFIGURED));
    if (pDriver->pfControl) {
      pDriver->pfControl(USB_DRIVER_CMD_SET_CONFIGURATION, &v);
    }
  } else {     // Any non-permitted value stalls
    USB_Global.pDriver->pfStallEP0();
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       USB__BuildConfigDesc
*
*  Notes
*    (1) The config descriptor (9.6.3 of the spec) includes interface and
*        endpoint descriptors.
*/
const U8 * USB__BuildConfigDesc(void) {
  USB_INFO_BUFFER   InfoBuffer;
  INTERFACE       * pInterface;
  int               i;
  int               iIF;

  USB_IB_Init(&InfoBuffer, &_aDescBuffer[0], sizeof(_aDescBuffer));
  //
  // Add the real config descriptor, which includes interface and endpoint descriptors.  (9 bytes)
  //
  USB_IB_AddU8 (&InfoBuffer, 9);                       // Descriptor len of first part of config descriptor
  USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_CONFIG);    // Descriptor type (2)
  USB_IB_AddU16(&InfoBuffer, 0);                       // Data length - to be filled in later
  USB_IB_AddU8 (&InfoBuffer, USB_Global.NumIFs);       // Number of Interfaces Supported
  USB_IB_AddU8 (&InfoBuffer, 1);                       // Config value
  USB_IB_AddU8 (&InfoBuffer, 0);                       // iConfig: index of config string descriptor
  USB_IB_AddU8 (&InfoBuffer, 0xc0);                    // Config Attributes (Self Powered)
  USB_IB_AddU8 (&InfoBuffer, _MaxPower);               // Max Power ([2mA] ... e.g. 50 means 100 mA)
  for (iIF = 0; iIF < USB_Global.NumIFs; iIF++) {
    int NumEPs;

    if (USB_Global.pIadAPI) {
      USB_Global.pIadAPI->pfAddIadDesc(iIF, &InfoBuffer);
    }
    pInterface = &USB_Global.aIF[iIF];
    NumEPs = _CountBits(pInterface->EPs >> 1);     // Do not count EP0
    //
    // Add interface descriptor.  (9 bytes)
    //
    USB_IB_AddU8 (&InfoBuffer, 9);                           // Descriptor len
    USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_INTERFACE);     // Descriptor type: 4
    USB_IB_AddU8 (&InfoBuffer, (U8)iIF);                           // Interface Number
    USB_IB_AddU8 (&InfoBuffer, 0);                        // Alternate settings
    USB_IB_AddU8 (&InfoBuffer, (U8)NumEPs);                 // No of used EPs
    USB_IB_AddU8 (&InfoBuffer, pInterface->IFClass);    // Interface Class
    USB_IB_AddU8 (&InfoBuffer, pInterface->IFSubClass); // Interface Subclass
    USB_IB_AddU8 (&InfoBuffer, pInterface->IFProtocol); // Interface Protocol
    USB_IB_AddU8 (&InfoBuffer, 0);                      // Interface Descriptor String
    //
    // Add Functional descriptor via optional callback
    //
    if (pInterface->pfAddFuncDesc) {
      pInterface->pfAddFuncDesc(&InfoBuffer);
    }
    //
    // Add Endpoint descriptors.  (9 bytes)
    //
    for (i = 1; i < USB_Global.NumEPs; i++) {
      if (pInterface->EPs & (1 << (i))) {
        U8  Interval;
        USB_IB_AddU8 (&InfoBuffer, 7);                             // Descriptor len
        USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_ENDPOINT);        // Descriptor type: 5
        USB_IB_AddU8 (&InfoBuffer, USB_aEPStat[i].EPAddr);         //
        USB_IB_AddU8 (&InfoBuffer, USB_aEPStat[i].EPType);         // Endpoint Attributes
        if (USB__IsHighSpeedMode() == 0) {
          if (USB_aEPStat[i].MaxPacketSize > 64) {
            USB_aEPStat[i].MaxPacketSize = 64;
          }
        }

        USB_IB_AddU16(&InfoBuffer, USB_aEPStat[i].MaxPacketSize);  // Max Packet Size
        Interval = _CalcInterval(i);
        USB_IB_AddU8 (&InfoBuffer, Interval);                       // Polling Interval
      }
    }
  }
  //
  // Fill in length of config descriptor
  //
  _aDescBuffer[2] = InfoBuffer.Cnt;
  return _aDescBuffer;
}

/*********************************************************************
*
*       USB__BuildDeviceDesc
*
*  Notes
*    (1)
*/
const U8 * USB__BuildDeviceDesc(void) {
  USB_INFO_BUFFER InfoBuffer;

  USB_IB_Init  (&InfoBuffer, &_aDescBuffer[0], sizeof(_aDescBuffer));
  USB_IB_AddU8 (&InfoBuffer, 18);                             // desc length
  USB_IB_AddU8 (&InfoBuffer, USB_DESC_TYPE_DEVICE);           // desc type
  //USB_IB_AddU16(&InfoBuffer, 0x200);                          // USB Spec Rev (Low Order)
  USB_IB_AddU16(&InfoBuffer, 0x110);                          // VR 4181 supports only 1.1
  USB_IB_AddU8 (&InfoBuffer, USB_Global.Class);               // Class
  USB_IB_AddU8 (&InfoBuffer, USB_Global.SubClass);            // Device Subclass
  USB_IB_AddU8 (&InfoBuffer, USB_Global.Protocol);            // Device Protocol
  USB_IB_AddU8 (&InfoBuffer, (U8)USB_aEPStat[0].MaxPacketSize);   // Packet Size         8/16/32/64 are valid
  USB_IB_AddU16(&InfoBuffer, USB_GetVendorId());                  // idVendor:           Vendor ID.
  USB_IB_AddU16(&InfoBuffer, USB_GetProductId());                 // idProdcut:          Product ID.
  USB_IB_AddU16(&InfoBuffer, 0x100);                          // bcdDevice:          Device Release Number (BCD)
  USB_IB_AddU8 (&InfoBuffer, STRING_INDEX_MANUFACTURER);      // iManufacturer:      Index of String Desc (Manuf)
  USB_IB_AddU8 (&InfoBuffer, STRING_INDEX_PRODUCT);           // iProduct:           Index of String Desc (Product)
  USB_IB_AddU8 (&InfoBuffer, STRING_INDEX_SN);                // iSerialNumber:      Index of String Desc (Serial #)
  USB_IB_AddU8 (&InfoBuffer, 0x01);                           // bNumConfigurations: Number of Configurations
  return _aDescBuffer;
}


/*********************************************************************
*
*       USB__OnTx0Done
*
* Function description
*  Called when a Tx transfer on EP0 is completed
*/
void USB__OnTx0Done(void) {
  if (USB_Global.Addr & 0x80) {
    U8 Addr;
    USB_Global.Addr &= 0x7f;
    Addr = USB_Global.Addr;
    USB_Global.pDriver->pfSetAddress(Addr);
    if (Addr) {
      USB__OnStatusChange((U8)(USB_Global.State | USB_STAT_ADDRESSED));
    } else {
      USB__OnStatusChange((U8)(USB_Global.State & ~USB_STAT_ADDRESSED));
    }
  }
}


/*********************************************************************
*
*       USB__HandleSetup
*
* Function description
*  Determines what request type, (standard, class, or vendor) is being
*  asked for.
*
*/
void USB__HandleSetup(const USB_SETUP_PACKET * pSetupPacket) {
  const USB_HW_DRIVER * pDriver;
  unsigned NumInterfaces;
  unsigned i;

  NumInterfaces = USB_Global.NumIFs;
  for (i = 0; i < NumInterfaces; i++) {
    INTERFACE * pInterface;

    pInterface = &USB_Global.aIF[i];
    if (pInterface->pfOnSetup) {
      if (pInterface->pfOnSetup(pSetupPacket) == 0) {
        return;
      }
    }
  }
  pDriver = USB_Global.pDriver;
  switch( pSetupPacket->bmRequestType & 0x60 ) {  // Mask off unnecessary bits
  case 0x00:
    switch (pSetupPacket->bRequest) {
    case 0:
      USB_DEBUGOUT(" GetStat");
      _CmdGetStatus(pSetupPacket);
      break;
    case 1:
      USB_DEBUGOUT(" ClrFeat");
      _CmdClearFeature(pSetupPacket);
      break;
    case 3:
      USB_DEBUGOUT(" SetFeat");
      _CmdSetFeature(pSetupPacket);
      break;
    case 5:
      USB_DEBUGOUT(" SetAddr");
      _CmdSetAddress(pSetupPacket->wValueLow);
      break;
    case 6:
      USB_DEBUGOUT(" GetDesc");
      _CmdGetDescriptor(pSetupPacket);
      break;
    case 8:
      USB_DEBUGOUT(" GetConf");
      _CmdGetConfiguration();
      break;
    case 9:
      USB_DEBUGOUT(" SetConf");
      _CmdSetConfiguration(pSetupPacket);
      break;
    case 10:
      USB_DEBUGOUT(" GetIF");
      _CmdGetInterface(pSetupPacket);
      break;
    case 11:
      USB_DEBUGOUT(" SetIF");
      _CmdSetInterface(pSetupPacket);
      break;
    default:
      pDriver->pfStallEP0();
    }
    break;
  case 0x20:              // Class specific requests
    {
      INTERFACE * pInterface;
      unsigned    InterfaceIndex;

      InterfaceIndex = pSetupPacket->wIndexLow;
      pInterface     = &USB_Global.aIF[InterfaceIndex];
      if (pInterface->pfOnClassRequest) {
        if (pInterface->pfOnClassRequest(pSetupPacket) == 0) {
          return;
        }
      }
    }
    pDriver->pfStallEP0();
    break;
  case 0x40:              // USB vendor request class
    {
      INTERFACE * pInterface;
      unsigned    InterfaceIndex;

      InterfaceIndex = pSetupPacket->wIndexLow;
      pInterface     = &USB_Global.aIF[InterfaceIndex];
      if (pInterface->pfOnVendorRequest) {
        if (pInterface->pfOnVendorRequest(pSetupPacket) == 0) {
          return;
        }
      }
    }
    pDriver->pfStallEP0();
    break;
  default:
    pDriver->pfStallEP0();
    break;
  }
}

/*********************************************************************
*
*       USB__OnSetupCancel
*
*  Function description
*    Called if a setup transaction is ended prematurely by the host.
*    This happens frequently, especially if the device has a small
*    MaxPacketSize on EP0, typically on the first request
*    GetDescriptor (Device).
*/
void USB__OnSetupCancel(void) {
  USB_aEPStat[0].pData = NULL;
}

/*********************************************************************
*
*       USB_SetMaxPower
*
*  Function description
*    Sets the max power the USB device will consume from USB Host.
*    This value will be asked during enumeration.
*
*  Parameter:
*    MaxPower     - Current consumption of the device given in mA.
*
*
*/
void USB_SetMaxPower(unsigned char MaxPower) {
  //
  // The current consumption is given to the host
  // in multiples of 2 mA.
  // So we make sure we store the correct value.
  // We round up the value, in case we have an odd value.
  //
  _MaxPower = (MaxPower + 1) >> 1;
}

/*********************************************************************
*
*       USB__ResetDataToggleEP
*
*  Function description
*    Resets data toggle of specified EP to DATA0.
*/
void USB__ResetDataToggleEP(U8 EPIndex) {
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  if (pDriver->pfResetEP) {
    pDriver->pfResetEP(EPIndex);
  }
}

/*************************** End of file ****************************/
