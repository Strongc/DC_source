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
File    : USB_Main.c
Purpose : General purpose routines
          Do not modify to allow easy updates !
--------  END-OF-HEADER  ---------------------------------------------
*/

#define USB_MAIN_C

#include "USB_Private.h"
#include "Global.h"         // Type definitions: U8, U16, U32, I8, I16, I32
#include "USB_HW_Driver.h"


extern U8 * cdc_tx_buff_p;
extern U32 Usb_Rx_Buff_Ovrfl_err_cnt;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static USB_HOOK * _pFirstSCHook;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/


static void _RemovePendingOperation(U8 EPIndex, U8 SignalTask) {
  EP_STAT * pEPStat;
  U8        IsInEP;
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  pEPStat = &USB_aEPStat[EPIndex];
  if (EPIndex) {
    IsInEP = pEPStat->EPAddr & 0x80;
    if (IsInEP) {
      pEPStat->Dir.TxInfo.TxIsPending = 0;
      pDriver->pfDisableTx(EPIndex);
    } else {
      pDriver->pfDisableRxInterruptEP(EPIndex);
    }
    pEPStat->pData = NULL;
    pEPStat->NumBytesRem  = 0;
    if (SignalTask) {
      USB_OS_Signal(EPIndex);
    }
  }
}


/*********************************************************************
*
*       _InvalidateAllEPs
*
*  Function description
*    Invalidate all end point information and signal all end points so the functions
*    waiting have a chance to return.
*/
static void _InvalidateAllEPs(void) {
  if (USB_Global.State & USB_STAT_CONFIGURED) {
    U8 i;
    for (i = 1; i < USB_Global.NumEPs; i++) {
      USB__InvalidateEP(i);
    }
  }
}

/*********************************************************************
*
*       _CalcMaxPacketSizeIso
*
*  Function description
*    Possible return values are:
*    1023, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1, 0.
*/
#if USB_SUPPORT_TRANSFER_ISO
static unsigned _CalcMaxPacketSizeIso(unsigned MaxPacketSize) {
  unsigned v;
  if (MaxPacketSize >= 1023) {   // Full speed limit is 1023. High speed limit is 1024 (not taken into account here)
    return 1023;
  }
  v = 512;
  do {
    if (MaxPacketSize & v) {
      break;
    }
    v >>= 1;
  } while (v);
  return v;
}
#endif

/*********************************************************************
*
*       _CalcMaxPacketSizeInt
*
*  Function description
*    Possible return values are:
*    64, 32, 16, 8, 4, 2, 1, 0.
*
*  Notes
*    (1) High speed
*        High speed permits up to 1024 bytes per packet. This is not taken into account here.
*/
#if USB_SUPPORT_TRANSFER_INT
static unsigned _CalcMaxPacketSizeInt(unsigned MaxPacketSize) {
  unsigned v;

  v = 64;
  do {
    if (MaxPacketSize >= v) {
      break;
    }
    v >>= 1;
  } while (v);
  return v;
}
#endif

/*********************************************************************
*
*       _CalcMaxPacketSizeBulk
*
*  Function description
*    Possible return values are:
*    512 (high speed mode only), 64, 32, 16, 8, 4, 2, 1, 0.
*/
static unsigned _CalcMaxPacketSizeBulk(unsigned MaxPacketSize, U8 IsHighSpeedMode) {
#if USB_SUPPORT_HIGH_SPEED
  if (IsHighSpeedMode) {
    if (MaxPacketSize >= 512) {
      return 512;
    } else {
      USB_OS_Panic(USB_ERROR_ILLEGAL_MAX_PACKET_SIZE);   // For high speed devices the only correct MaxPacketSize is 512.
    }
  }
#endif
  if (MaxPacketSize >= 64) {
    return 64;
  }
  if (MaxPacketSize >= 32) {
    return 32;
  }
  if (MaxPacketSize >= 16) {
    return 16;
  }
  if (MaxPacketSize >= 8) {
    return 8;
  }
  return 0;
}

/*********************************************************************
*
*       _OnRx2TargetBuffer
*
*/
static void  _OnRx2TargetBuffer(U8 EPIndex, unsigned NumBytes) {
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
  pEPStat->pData       += NumBytes;
  pEPStat->NumBytesRem -= NumBytes;
  //
  // Check if a task is waiting
  //
  if ((pEPStat->NumBytesRem == 0) || (pEPStat->AllowShortPacket)) {
    USB_OS_Signal(EPIndex);
    USB_Global.pDriver->pfDisableRxInterruptEP(EPIndex);
  }
}


/*********************************************************************
*
*       _ReadEP
*
* Function description
*   Read data from USB EP
*
* Returns
*   Number of bytes read
* Context:
*   Called from a task
*/
static int _ReadEP(U8 EPIndex, void* pData, unsigned NumBytesReq, char AllowBlocking, unsigned ms) {
  EP_STAT * pEPStat;
  int NumBytesTransfered;

  NumBytesTransfered = -1;
  pEPStat = &USB_aEPStat[EPIndex];

  if (pEPStat->IsHalted == 1) {
    return -1;
  }

  USB_OS_IncDI();
  if ((USB_Global.State & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) {
    //
    // Read from buffer as far as possible
    //
    if (NumBytesReq) {
      NumBytesTransfered = BUFFER_Read(&pEPStat->Buffer, (U8 *)pData, NumBytesReq);
      NumBytesReq -= NumBytesTransfered;
      pData        = (U8*)pData + NumBytesTransfered;
      if (NumBytesReq == 0) {
        goto Done;
      }
    } else {
      if (pEPStat->Buffer.NumBytesIn) {
        goto Done;
      }
    }
    //
    // Wait for new data
    //
    pEPStat->pData       = (U8*)pData;
    pEPStat->NumBytesRem = NumBytesReq;
    USB_Global.pDriver->pfEnableRxInterruptEP(EPIndex);
    if (AllowBlocking) {
      if (ms) {
        if (USB_OS_WaitTimed(EPIndex, ms)) {
          NumBytesReq = 0;
          _RemovePendingOperation(EPIndex, 0);
        }
      } else {
        USB_OS_Wait(EPIndex);
      }
      if (pEPStat->AllowShortPacket) {
        NumBytesTransfered += NumBytesReq - pEPStat->NumBytesRem;
        goto Done;
      }
      if (pEPStat->NumBytesRem) {       // Did we wake up due to error condition such as USB Reset or time out ?
        NumBytesTransfered = -1;
        goto Done;
      }
    }
    NumBytesTransfered += NumBytesReq;
  }

Done:
  USB_OS_DecRI();
  return NumBytesTransfered;
}



/*********************************************************************
*
*       _ReceiveEP
*
* Function description
*   Setup to receive <= NumBytes that are requested.
*
* Context:
*   Called from a task
*/
static int _ReceiveEP(U8 EPOut, void* pData, unsigned NumBytesReq, unsigned ms) {
  int r;

  r = -1;
  if ((USB_Global.State & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) {
    EP_STAT * pEPStat;

    pEPStat = &USB_aEPStat[EPOut];
    pEPStat->AllowShortPacket = 1;
    r  = _ReadEP(EPOut, pData, NumBytesReq, 1, ms);
    pEPStat->AllowShortPacket = 0;
  }
  return r;
}

/*********************************************************************
*
*       _WaitForEndOfTransfer
*
* Function description
*   Wait until the current transfer on a particular EP has completed
*
* Context:
*   Called from a task
*/
static int _WaitForEndOfTransfer(U8 EPIndex, unsigned NumBytes, unsigned Timeout) {
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
  USB_OS_IncDI();
  if (USB_Global.OnTxBehavior == 0) {
    if (pEPStat->Dir.TxInfo.TxIsPending) {
      goto Wait;
    }
  }
  if (pEPStat->pData) {
Wait:
    if (Timeout) {
      if (USB_OS_WaitTimed(EPIndex, Timeout)) {
        _RemovePendingOperation(EPIndex, 0);
      }
    } else {
      USB_OS_Wait(EPIndex);
    }
  }
  USB_OS_DecRI();
  return NumBytes - pEPStat->NumBytesRem;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


/*********************************************************************
*
*       USB_AddEP
*/
unsigned USB_AddEP(U8 InDir, U8 TransferType, U16 Interval, U8 * pBuffer, unsigned BufferSize) {
  unsigned        r;
  U8              EPDevice;
  U8              EPIndex;
  EP_STAT       * pEPStat;
  unsigned        MaxPacketSize;
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  r = 0;
  if (USB_Global.NumEPs < USB_NUM_EPS) {
    EPIndex  = USB_Global.NumEPs;
    pEPStat  = &USB_aEPStat[USB_Global.NumEPs];
    EPDevice = pDriver->pfAllocEP(InDir, TransferType);
    if ((EPDevice != 0) && (EPDevice != 0xff)) {
      USB_MEMSET(pEPStat, 0, sizeof(EP_STAT));
      pEPStat->EPAddr           = (EPDevice) | (InDir << 7);
      pEPStat->EPType           = TransferType;
      //
      // Compute MaxPacketSize based on capabilities of USB controller, BufferSize and EPType
      //
      MaxPacketSize = pDriver->pfGetMaxPacketSize(EPIndex);
      if (InDir == 0) {
        if (MaxPacketSize > BufferSize) {
          MaxPacketSize = BufferSize;      // Buffer must be big enough to hold at least one complete packet for OUT EPs
        }
      }
      //
      // Make sure the packet size is permitted by the spec for this transfer type (See USB Spec, Chapter 5)
      //
      MaxPacketSize = USB__CalcMaxPacketSize(MaxPacketSize, TransferType, (U8)USB__IsHighSpeedCapable());
#if USB_DEBUG_LEVEL > 0
      if (MaxPacketSize == 0) {
        USB_OS_Panic(USB_ERROR_ILLEGAL_MAX_PACKET_SIZE);
      }
#endif
      pEPStat->MaxPacketSize    = MaxPacketSize;
      pEPStat->Buffer.pData     = pBuffer;
      pEPStat->Buffer.Size      = BufferSize;
      pEPStat->Interval         = Interval;
      r = USB_Global.NumEPs++;
    }
  }
  return r;
}


/*********************************************************************
*
*       USB__EPAddr2Index
*/
U8 USB__EPAddr2Index(unsigned EPAddr) {
  int i;
  for (i = 0; i < USB_Global.NumEPs; i++) {
    if (USB_aEPStat[i].EPAddr == EPAddr) {
      return i;
    }
  }
#if USB_DEBUG_LEVEL
  USB_OS_Panic(USB_ERROR_ILLEGAL_EPADDR);
#endif
  return 0xff;
}

/*********************************************************************
*
*       USB__EPIndex2Addr
*/
U8 USB__EPIndex2Addr(U8 EPIndex) {
  return USB_aEPStat[EPIndex].EPAddr;
}


/*********************************************************************
*
*       USB_IsConfigured
*/
char USB_IsConfigured(void) {
  return   (USB_Global.State & USB_STAT_CONFIGURED) ? 1 : 0;
}

/*********************************************************************
*
*       USB__UpdateEPHW
*/
void USB__UpdateEPHW(void) {
  EP_STAT * pEPStat;
  int       i;
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  for (i = 1; i < USB_Global.NumEPs; i++) {
    pEPStat = &USB_aEPStat[i];

    pDriver->pfUpdateEP(pEPStat);
    //
    // If there is space in OUT-Buffer (Rx) for a packet of max. size, enable Rx-interrupt
    //
#if 0
    if ((pEPStat->EPAddr & (1 << 7)) == 0) {
      BufferSpace = pEPStat->Buffer.Size - pEPStat->Buffer.NumBytesIn;
      if (BufferSpace >= pEPStat->MaxPacketSize) {
        pDriver->pfEnableRxInterruptEP(i);
      }
    }
#endif
    pDriver->pfEnable();
  }
}


/*********************************************************************
*
*       USB_Init
*/
void USB_Init(void) {
  //
  // Setup global data
  //
  USB_MEMSET(&USB_Global, 0, sizeof(USB_Global));
  USB_Global.NumEPs = 1;
  USB_MEMSET((void*)USB_aEPStat, 0, sizeof(USB_aEPStat));
  USB_X_AddDriver();
  USB_aEPStat[0].MaxPacketSize = USB_Global.pDriver->pfGetMaxPacketSize(0);
  //
  // Initialize OS Layer module
  USB_OS_Init();
}

/*********************************************************************
*
*       USB_Start
*/
void USB_Start(void) {
  const USB_HW_DRIVER * pDriver;

  pDriver = USB_Global.pDriver;
  //
  // Fire up the hardware
  //
  pDriver ->pfInit();
  USB__UpdateEPHW();
  if (pDriver->pfControl) {
     pDriver->pfControl(USB_DRIVER_CMD_GET_TX_BEHAVIOR, &USB_Global.OnTxBehavior);
     pDriver->pfControl(USB_DRIVER_CMD_GET_SETADDRESS_BEHAVIOR, &USB_Global.SetAddressBehavior);
  }
  USB_Global.State = USB_STAT_ATTACHED;
  pDriver->pfAttach();
}


/*********************************************************************
*
*       BUFFER_Read
*
* Function description
*   Read data from USB EP
*
* Returns
*   Number of bytes read
*/
unsigned BUFFER_Read(BUFFER * pBuffer, U8 * pData, unsigned NumBytesReq) {
  unsigned EndPos;
  unsigned NumBytesAtOnce;
  unsigned NumBytesTransfered;

#if (defined (USB_TRIAL))
  if (USB_OS_GetTickCnt() > (15 * 60 * 1000)) {
    return 0;
  }
#endif
  NumBytesTransfered = 0;
  while (NumBytesReq && pBuffer->NumBytesIn) {
    EndPos =  pBuffer->RdPos + pBuffer->NumBytesIn;
    if (EndPos > pBuffer->Size) {
      EndPos = pBuffer->Size;
    }
    NumBytesAtOnce = EndPos - pBuffer->RdPos;
    NumBytesAtOnce = MIN(NumBytesAtOnce, NumBytesReq);
    USB_MEMCPY(pData, pBuffer->pData + pBuffer->RdPos, NumBytesAtOnce);
    NumBytesReq         -= NumBytesAtOnce;
    pBuffer->NumBytesIn -= NumBytesAtOnce;
    NumBytesTransfered  += NumBytesAtOnce;
    pData               += NumBytesAtOnce;
    pBuffer->RdPos      += NumBytesAtOnce;
    if (pBuffer->RdPos == pBuffer->Size) {
      pBuffer->RdPos = 0;
    }
  }
  //
  // Optimization for speed: If buffer is empty, read position is reset.
  //
  if (pBuffer->NumBytesIn == 0) {
    pBuffer->RdPos = 0;
  }
  return NumBytesTransfered;
}

/*********************************************************************
*
*       BUFFER_Write
*
* Function description
*   Write data into ring buffer
*/
void BUFFER_Write(BUFFER * pBuffer, const U8 * pData, unsigned NumBytes) {
  unsigned WrPos;
  unsigned EndPos;
  unsigned NumBytesFree;
  unsigned NumBytesAtOnce;

#if (defined (USB_TRIAL))
  if (USB_OS_GetTickCnt() > (15 * 60 * 1000)) {
    return;
  }
#endif

  while (1) {
    //
    // Check if there is still something to do
    //
    NumBytesFree = pBuffer->Size - pBuffer->NumBytesIn;
    if (NumBytes == 0) {
      break;                                        // We are done !
    }
    if (NumBytesFree == 0) {
     Usb_Rx_Buff_Ovrfl_err_cnt++;
      break;                                        // Error ...
    }
    //
    // Compute number of bytes to copy at once
    //
    WrPos = pBuffer->RdPos + pBuffer->NumBytesIn;
    if (WrPos >= pBuffer->Size) {
      WrPos -= pBuffer->Size;
    }
    EndPos =  WrPos + NumBytes;
    if (EndPos > pBuffer->Size) {
      EndPos = pBuffer->Size;
    }
    NumBytesAtOnce = EndPos - WrPos;
    NumBytesAtOnce = MIN(NumBytesAtOnce, NumBytes);
    //
    // Copy
    //
    USB_MEMCPY(pBuffer->pData + WrPos, pData, NumBytesAtOnce);
    //
    // Update variables
    //
    NumBytes            -= NumBytesAtOnce;
    pBuffer->NumBytesIn += NumBytesAtOnce;
    pData               += NumBytesAtOnce;
  }
}

/*********************************************************************
*
*       USB_ReadEP
*
* Function description
*   Read data from USB EP
*
* Returns
*   Number of bytes read
* Context:
*   Called from a task
*/
int USB_ReadEP(U8 EPOut, void* pData, unsigned NumBytesReq) {
  int r;

  r = 0;
  if ((USB_Global.State & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) {
    r  = _ReadEP(EPOut, pData, NumBytesReq, 1, 0);
  }
  return r;
}


/*********************************************************************
*
*       USB_ReadEPTimed
*
* Function description
*   Read data from USB EP with time out
*
* Returns
*   Number of bytes read
* Context:
*   Called from a task
*/
int USB_ReadEPTimed(U8 EPOut, void * pData, unsigned NumBytesReq, unsigned ms) {
  int r;

  r = 0;
  if ((USB_Global.State & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) {
    r  = _ReadEP(EPOut, pData, NumBytesReq, 1, ms);
  }
  return r;
}


/*********************************************************************
*
*       USB_ReceiveEP
*
* Function description
*   Read data from USB EP
*
* Returns
*   Number of bytes read
* Context:
*   Called from a task
*/
int USB_ReceiveEP(U8 EPOut, void* pData, unsigned NumBytesReq) {
  return _ReceiveEP(EPOut, pData, NumBytesReq, 0);
}

/*********************************************************************
*
*       USB_ReceiveEPTimed
*
* Function description
*   Read data from USB EP
*
* Returns
*   Number of bytes read
* Context:
*   Called from a task
*/
int USB_ReceiveEPTimed(U8 EPOut, void* pData, unsigned NumBytesReq, unsigned ms) {
  return _ReceiveEP(EPOut, pData, NumBytesReq, ms);
}


/*********************************************************************
*
*       USB_ReadEPOverlapped
*
* Function description
*   Read data from USB EP
*
* Returns
*   Number of bytes read
* Context:
*   Called from a task
*/
int USB_ReadEPOverlapped(U8 EPOut, void * pData, unsigned NumBytesReq) {
  return _ReadEP(EPOut, pData, NumBytesReq, 0, 0);
}

/*********************************************************************
*
*       USB__WriteEP0FromISR
*
* Function description
*   Write data to USB EP
*
* Context:
*   Called from ISR
*/
void USB__WriteEP0FromISR(const void* pData, unsigned NumBytes, char Send0PacketIfRequired) {
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[0];
  //
  // Enqueue it
  //

  if(pData != cdc_tx_buff_p)
  	USB_MEMCPY(cdc_tx_buff_p,(U8*)pData,NumBytes);
  
  //pEPStat->pData       = (U8*)pData;
  pEPStat->pData       = (U8*)cdc_tx_buff_p;
  pEPStat->NumBytesRem = NumBytes;
  pEPStat->Dir.TxInfo.Send0PacketIfRequired = Send0PacketIfRequired;
  //
  // Start transfer
  //
  USB_Global.pDriver->pfStartTx(0);
}



/*********************************************************************
*
*       USB_WriteEPOverlapped
*
* Function description
*   Write data to USB EP
*
* Context:
*   Called from a task
*
* Return value:
*
*/
int USB_WriteEPOverlapped(U8 EPIndex, const void* pData, unsigned NumBytes, char Send0PacketIfRequired) {
  EP_STAT * pEPStat;
  int       r;

  if (EPIndex) {
    if ((USB_Global.State & USB_STAT_CONFIGURED) == 0) {
      return 0;
    }
  }
  pEPStat = &USB_aEPStat[EPIndex];
  if (pEPStat->IsHalted) {
    return 0;
  }
  if (pEPStat->EPType == USB_TRANSFER_TYPE_BULK) {
    if ((NumBytes & (pEPStat->MaxPacketSize - 1)) || ((NumBytes & 0x7FF) == 0)) {
      Send0PacketIfRequired = 0;
    }
  }
  //
  // In order to send a NULL packet, pData shall be set to valid pointer
  // 
  if ((pData == NULL) && (NumBytes == 0) && (Send0PacketIfRequired == 1)) {
    pData = (const void *)1;
  }
  USB_OS_IncDI();
  //
  // Check if a transfer is in progress. In this case,
  // we either extend the running transfer (preferred) or wait until transfer is completed
  //
  if (pEPStat->pData) {
    if (pEPStat->pData + pEPStat->NumBytesRem == (const U8*)pData) {
      //
      // Extend current transfer
      //
      pEPStat->NumBytesRem += NumBytes;
      pEPStat->Dir.TxInfo.Send0PacketIfRequired = Send0PacketIfRequired;
      goto Done;
    } else {
      //
      // Wait for transfer to end
      //
      //XFK: Changed to use timed wait. Part of workaround for communication halt issue
      if (USB_OS_WaitTimed(EPIndex, 100) == 0) 
      {
        return 0;
      }
    }
  }
  //
  // Enqueue it
  //
  pEPStat->pData       = (U8*)pData;
  pEPStat->NumBytesRem = NumBytes;
  pEPStat->Dir.TxInfo.Send0PacketIfRequired = Send0PacketIfRequired;
  //
  // Start transfer
  //
  USB_Global.pDriver->pfStartTx(EPIndex);
Done:
  USB_OS_DecRI();
  r = NumBytes - pEPStat->NumBytesRem;
  if (r == 0 && pEPStat->Dir.TxInfo.Send0PacketIfRequired) {
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       USB_WaitForEndOfTransfer
*
* Function description
*   Wait until the current transfer on a particular EP has completed
*
* Context:
*   Called from a task
*/
void USB_WaitForEndOfTransfer(U8 EPIndex) {
  _WaitForEndOfTransfer(EPIndex, 0, 100); //XFK: 0 => 100 [ms]. Part of workaround for communication halt issue
}

/*********************************************************************
*
*       USB_WriteEP
*
* Function description
*   Write data to USB EP
*
* Context:
*   Called from a task
*/
int USB_WriteEP(U8 EPIndex, const void* pData, unsigned NumBytes, char Send0PacketIfRequired) {
  int r;
  int NumBytesRem;
  r = 0;
  if ((USB_Global.State & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) {
    r = USB_WriteEPOverlapped(EPIndex, pData, NumBytes, Send0PacketIfRequired);
    NumBytesRem = NumBytes - r;
    if (USB_Global.OnTxBehavior == 0) {
      r += _WaitForEndOfTransfer(EPIndex, NumBytesRem, 100); //XFK: 0 => 100 [ms]. Part of workaround for communication halt issue
    } else {
      if (NumBytesRem > 0) {
        r += _WaitForEndOfTransfer(EPIndex, NumBytesRem, 100); //XFK: 0 => 100 [ms]. Part of workaround for communication halt issue
      }
    }
  }
  return r;
}

/*********************************************************************
*
*       USB_WriteEP
*
* Function description
*   Write data to USB EP
*
* Context:
*   Called from a task
*/
int USB_WriteEPTimed(U8 EPIndex, const void* pData, unsigned NumBytes, char Send0PacketIfRequired, unsigned ms) {
  int r;
  int NumBytesRem;
  r = 0;
  if ((USB_Global.State & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) {
    r = USB_WriteEPOverlapped(EPIndex, pData, NumBytes, Send0PacketIfRequired);
    NumBytesRem = NumBytes - r;
    if (USB_Global.OnTxBehavior == 0) {
      r += _WaitForEndOfTransfer(EPIndex, NumBytesRem, ms);
    } else {
      if (NumBytesRem > 0) {
        r += _WaitForEndOfTransfer(EPIndex, NumBytesRem, ms);
      }
    }
  }
  return r;
}


/*********************************************************************
*
*       USB_SetClassRequestHook
*
* Context:
*   Task
*/
void USB_SetClassRequestHook(unsigned InterfaceNum, USB_ON_CLASS_REQUEST * pfOnClassRequest) {
  USB_Global.aIF[InterfaceNum].pfOnClassRequest = pfOnClassRequest;
}

/*********************************************************************
*
*       USB_SetVendorRequestHook
*
* Context:
*   Task
*/
void USB_SetVendorRequestHook(unsigned InterfaceNum, USB_ON_CLASS_REQUEST * pfOnVendorRequest) {
  USB_Global.aIF[InterfaceNum].pfOnVendorRequest = pfOnVendorRequest;
}

/*********************************************************************
*
*       USB_SetOnSetupHook
*
* Context:
*   Task
*/
void USB_SetOnSetupHook(unsigned InterfaceNum, USB_ON_SETUP * pfOnSetup) {
  USB_Global.aIF[InterfaceNum].pfOnSetup = pfOnSetup;
}

/*********************************************************************
*
*       USB_SetOnRxEP0
*
*  Function description
*/
void USB_SetOnRxEP0(USB_ON_RX_FUNC * pfOnRx) {
  USB_Global.pfOnRxEP0 = pfOnRx;
}


/*********************************************************************
*
*       USB__GetpDest
*
*/
void * USB__GetpDest(U8 EPIndex, unsigned NumBytes) {
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
  //
  // Check if a task is waiting for >= NumBytes
  //
  if (pEPStat->NumBytesRem >= NumBytes) {
    return pEPStat->pData;
  }
  //
  // No destination pointer for Zero copy available
  //
  return NULL;
}

/*********************************************************************
*
*       USB__OnRxZeroCopy
*
*/
void USB__OnRxZeroCopy(U8 EPIndex, unsigned NumBytes) {
  _OnRx2TargetBuffer(EPIndex, NumBytes);
}


/*********************************************************************
*
*       USB__OnRx
*
* Function description
*   Store the received data
*
* Context:
*   ISR,  USB-IRQ Level
*/
void USB__OnRx(U8 EPIndex, const U8 * pData, unsigned NumBytes) {
  unsigned NumBytesAtOnce;
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
  //
  // Handle EP0 data separately: Only callback for data is allowed
  //
  if (EPIndex == 0) {
    if (USB_Global.pfOnRxEP0) {
      USB_Global.pfOnRxEP0(pData, NumBytes);
    }
    return;
  } else {
    if (pEPStat->Dir.pfOnRx) {
      pEPStat->Dir.pfOnRx(pData, NumBytes);
    }
  }
  //
  // Handle data requested
  //
  if (pEPStat->pData) {
    NumBytesAtOnce = MIN(NumBytes, pEPStat->NumBytesRem);
    if (NumBytesAtOnce) {
      USB_MEMCPY(pEPStat->pData, pData, NumBytesAtOnce);
      NumBytes             -= NumBytesAtOnce;
      pData                += NumBytesAtOnce;
      _OnRx2TargetBuffer(EPIndex, NumBytesAtOnce);
    }
  }
  if (NumBytes) {
    //
    // Put remainder of the data in buffer
    //
    BUFFER_Write(&pEPStat->Buffer, pData, NumBytes);
    USB_Global.pDriver->pfDisableRxInterruptEP(EPIndex);
  }
}


/*********************************************************************
*
*       USB__Send
*/
void USB__Send(U8 EPIndex) {
  unsigned NumBytes;
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
#if USB_DEBUG_LEVEL
  if (EPIndex) {
    if ((pEPStat->EPAddr & 0x80) == 0) {
      USB_OS_Panic(0xFF);              // Should only be called for IN (=Tx) EPs
    }
    //
    // If ...->pData == NULL, then we should not really be here and there is no need to send a packet.
    //
  }
#endif
  if (EPIndex == 0) {
    if (pEPStat->pData == NULL) {
      return;
    }
  }
  if (EPIndex) {
    if (USB_Global.OnTxBehavior == 0) {
      if (pEPStat->Dir.TxInfo.TxIsPending) {
        return;
      }
      pEPStat->Dir.TxInfo.TxIsPending = 1;
    }
    if (pEPStat->pData == NULL) {
      return;
    }
  }
  NumBytes = MIN(pEPStat->NumBytesRem, pEPStat->MaxPacketSize);
  USB_Global.pDriver->pfSendEP(EPIndex, pEPStat->pData, NumBytes);
  pEPStat->NumBytesRem -= NumBytes;
  pEPStat->pData       += NumBytes;
  //
  // Do we need to send an other packet ?
  //
  if (pEPStat->NumBytesRem) {    // More data ?
    return;                      // Send an other packet with data on next interrupt
  }
  if ((NumBytes == pEPStat->MaxPacketSize) && pEPStat->Dir.TxInfo.Send0PacketIfRequired) {
    pEPStat->Dir.TxInfo.Send0PacketIfRequired = 0;    // Make sure we send no more than a single 0-packe
    return;                      // Send an other packet without data on next interrupt
  }
  //
  // Disable Tx interrupts
  //
  pEPStat->pData = NULL;          // Indicate that all transfers are completed
}

/*********************************************************************
*
*       USB__OnTx
*/
void USB__OnTx(U8 EPIndex) {
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
  if (EPIndex) {
    if (USB_Global.OnTxBehavior == 0) {
      pEPStat->Dir.TxInfo.TxIsPending = 0;
    }
  }
  if (pEPStat->pData) {
    USB__Send(EPIndex);
    USB_OS_Signal(EPIndex); //tcsl
    if (USB_Global.OnTxBehavior == 0) {
      return;
    }
  }
  if ((USB_Global.OnTxBehavior == 0) && (pEPStat->NumBytesRem == 0)) {
    pEPStat->Dir.TxInfo.Send0PacketIfRequired = 0;
  }
  //
  // Disable Tx interrupts
  //
  if (pEPStat->pData == NULL) {
    USB_Global.pDriver->pfDisableTx(EPIndex);
  }
  //
  // Wake task / Notify setup
  //
  if (EPIndex) {
    if ((pEPStat->pData == NULL) && (pEPStat->Dir.TxInfo.Send0PacketIfRequired == 0)) {
      USB_OS_Signal(EPIndex);
    }
  } else {
    USB__OnTx0Done();
  }
}

/*********************************************************************
*
*       USB_SetClrStallEP
*
*/
void  USB_SetClrStallEP(U8 EPIndex, int OnOff) {
  USB_aEPStat[EPIndex].IsHalted = OnOff;
  USB_Global.pDriver->pfSetClrStallEP(EPIndex, OnOff);
}

/*********************************************************************
*
*       USB_StallEP
*
*/
void  USB_StallEP(U8 EPIndex) {
  USB_SetClrStallEP(EPIndex, 1);
}

/*********************************************************************
*
*       USB_AddDriver
*
*/
void USB_AddDriver(const USB_HW_DRIVER * pDriver) {
  USB_Global.pDriver = pDriver;
}

/*********************************************************************
*
*       USB_RegisterSCHook
*
* Function description
*   Adds hook function to the hook list for state change
*
* Returns
*   0    - OK
*   1    - Error, specified hook already exists.
*
*/
int USB_RegisterSCHook(USB_HOOK * pHook, void (*cb) (void * pContext), void * pContext, unsigned SizeOfContext) {
  USB_HOOK *  p;

  USB_MEMSET(pHook, 0, sizeof(USB_HOOK));
  pHook->cb            = cb;
  pHook->pContext      = pContext;
  pHook->SizeOfContext = SizeOfContext;
  //
  // Check if this hook is already in list. If so, return error.
  //
  p      = _pFirstSCHook;
  while (p) {
    if (p == pHook) {
      return 1;     // Error, hook already in list
    }
    p       = p->pNext;
  }
  //
  // Make new hook first in list.
  //
  pHook->pNext = _pFirstSCHook;
  _pFirstSCHook = pHook;
  return 0;

}


/*********************************************************************
*
*       USB_UnregisterSCHook
*
* Function description
*   Removes hook function from the hook list for state change
*
* Returns
*   0    - OK, removed
*   1    - Error, not in list
*
*/
int USB_UnregisterSCHook(USB_HOOK * pHook) {
  // TBD
  USB_USE_PARA(pHook);
  return 1;
}

/*********************************************************************
*
*       USB__OnBusReset
*
* Function description
*   Flushes the input buffer and set the "_IsInReset" flag which inhibits further
*   transfers until cleared
*
* Context:
*   ISR,  USB-IRQ Level
*/
void USB__OnBusReset(void) {
  int i;

  if ((USB_Global.State & USB_STAT_ATTACHED) == 0) {
    USB_DEBUGOUT("Bus RESET should not occur in unattached state!");
  }
  USB_Global.Addr         = 0;
  USB_Global.pDriver->pfSetAddress(USB_Global.Addr);
  
  USB__UpdateEPHW();     // Some USB controllers (OKI, NXP) reset all configuration info because of the BUS Reset, so we write it again
  for (i = 1; i < USB_Global.NumEPs; i++) {
    EP_STAT * pEPStat;

    pEPStat = &USB_aEPStat[i];
    pEPStat->IsHalted    = 0;
  }
  USB__OnStatusChange((U8)((USB_Global.State & USB_STAT_ATTACHED) | USB_STAT_READY));
}

/*********************************************************************
*
*       USB_GetState
*
* Function description
*   Returns the USB state as defined in [1], 9.1.1 Visible USB states and table 9-1.
*
* Return value
*   A bitwise combination of the USB state flags:
*     USB_STAT_ATTACHED
*     USB_STAT_READY
*     USB_STAT_ADDRESSED
*     USB_STAT_CONFIGURED
*     USB_STAT_SUSPENDED
*/
int USB_GetState(void) {
  return USB_Global.State;
}

/*********************************************************************
*
*       USB__OnSuspend
*
* Function description
*   Called by the driver when a suspend condition is recognized.
*/
void USB__OnSuspend(void) {
  USB__OnStatusChange((U8)(USB_Global.State | USB_STAT_SUSPENDED));
}

/*********************************************************************
*
*       USB__OnResume
*
* Function description
*   Called by the driver when a resume condition is recognized.
*/
void USB__OnResume(void) {
  USB__OnStatusChange((U8)(USB_Global.State & ~USB_STAT_SUSPENDED));
}

/*********************************************************************
*
*       USB__OnStatusChange
*
*  Function description
*    Called on every potential status change.
*    Signals all EPs and calls all registered hook functions on a real change.
*/
void USB__OnStatusChange(U8 State) {
  USB_HOOK * pHook;

  if (USB_Global.State != State) {
    //
    // Send notification s by calling all hook functions
    //
    pHook = _pFirstSCHook;
    while(pHook) {
      if (pHook->pContext) {
        USB_MEMCPY(pHook->pContext, &State, MIN(sizeof(State), pHook->SizeOfContext));
      }
      if (pHook->cb) {
        (pHook->cb)(pHook->pContext);
      }
      pHook = pHook->pNext;
    }
    USB_Global.State = State;
    _InvalidateAllEPs();
  }
}


/*********************************************************************
*
*       USB_GetNumBytesInBuffer
*
*  Function description
*    Returns the number of bytes that are available in the stack's
*    internal buffer memory of an EP.
*
*/
unsigned USB_GetNumBytesInBuffer(U8 EPIndex) {
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];
  return pEPStat->Buffer.NumBytesIn;
}

/*********************************************************************
*
*       USB__CalcMaxPacketSize
*/
unsigned USB__CalcMaxPacketSize(unsigned MaxPacketSize, U8 TransferType, U8 IsHighSpeedMode) {
#if USB_SUPPORT_TRANSFER_ISO
  if (TransferType == USB_TRANSFER_TYPE_ISO) {      // USB Spec, 5.6.3
    return _CalcMaxPacketSizeIso(MaxPacketSize);
  }
#endif
#if USB_SUPPORT_TRANSFER_INT
  if (TransferType == USB_TRANSFER_TYPE_INT) {      // USB Spec, 5.7.3
    return _CalcMaxPacketSizeInt(MaxPacketSize);
  }
#endif
  return _CalcMaxPacketSizeBulk(MaxPacketSize, IsHighSpeedMode);     // USB Spec, 5.8.3
}


/*********************************************************************
*
*       USB__StoreU32BE
*/
void USB__StoreU32BE(U8 * p, U32 v) {
  *p       = (U8)((v >> 24) & 255);
  *(p + 1) = (U8)((v >> 16) & 255);
  *(p + 2) = (U8)((v >> 8) & 255);
  *(p + 3) = (U8)( v       & 255);
}

/*********************************************************************
*
*       USB__StoreU32LE
*/
void USB__StoreU32LE(U8 * p, U32 v) {
  *p       = (U8)((v      ) & 255);
  *(p + 1) = (U8)((v >>  8) & 255);
  *(p + 2) = (U8)((v >> 16) & 255);
  *(p + 3) = (U8)((v >> 24) & 255);
}

/*********************************************************************
*
*       USB__StoreU16BE
*/
void USB__StoreU16BE(U8 * p, unsigned v) {
  *(p)     = (U8)((v >> 8) & 255);
  *(p + 1) = (U8)( v       & 255);
}

/*********************************************************************
*
*       USB__StoreU16LE
*/
void USB__StoreU16LE(U8 * p, unsigned v) {
  *p       = (U8)((v      ) & 255);
  *(p + 1) = (U8)((v >>  8) & 255);
}

/*********************************************************************
*
*       USB__GetU32BE
*/
U32 USB__GetU32BE(U8 * p) {
  U32 v;
  v  = (U32)(*(p + 0)) << 24;
  v |= (U32)(*(p + 1)) << 16;
  v |= *(p + 2) << 8;
  v |= *(p + 3);
  return v;
}

/*********************************************************************
*
*       USB__GetU32LE
*/
U32 USB__GetU32LE(U8 * p) {
  U32 v;
  v  = *p;
  v |= *(p + 1) << 8;
  v |= (U32)(*(p + 2)) << 16;
  v |= (U32)(*(p + 3)) << 24;
  return v;
}

/*********************************************************************
*
*       USB__GetU16BE
*/
U16 USB__GetU16BE(U8 * p) {
  U16 v;
  v  = *p << 8;
  v |= *(p + 1);
  return v;
}

/*********************************************************************
*
*       USB__GetU32LE
*/
U16 USB__GetU16LE(U8 * p) {
  U16 v;
  v  = *p;
  v |= *(p + 1) << 8;
  return v;
}

/*********************************************************************
*
*       USB__SwapU32
*/
U32 USB__SwapU32(U32 v) {
  v = (v << 24) | ((v & 0xFF00) << 8) | ((v >> 8) & 0xFF00) | (v >> 24);
  return v;
}


/*********************************************************************
*
*       USB__IsHighSpeedCapable
*
*/
int USB__IsHighSpeedCapable(void) {
  return ((USB_Global.pDriver->pfIsInHighSpeedMode)() & (1 << 1));
}

/*********************************************************************
*
*       USB__IsHighSpeedMode
*
*/
int USB__IsHighSpeedMode(void) {
  return ((USB_Global.pDriver->pfIsInHighSpeedMode)() & (1 << 0));
}

/*********************************************************************
*
*       USB__AllocIF
*
*/
U8 USB__AllocIF(void) {
  U8 InterFaceNo;

  InterFaceNo = USB_Global.NumIFs++;
  if (USB_Global.NumIFs > USB_MAX_NUM_IF) {
    USB_OS_Panic(USB_ERROR_INVALID_INTERFACE_NO);
  }
  return InterFaceNo;
}


/*********************************************************************
*
*       USB__InvalidateEP
*
*  Function description:
*    This function makes sure that all relevant information regarding
*    an end point will be invalidated.
*    It makes also sure that the task is signaled which may wait that
*    the read or write operation will be finished.
*
*  Parameters:
*    EPIndex    - End point index to invalidate
*
*/
void USB__InvalidateEP(U8 EPIndex) {
  EP_STAT * pEpStat;

  pEpStat = &USB_aEPStat[EPIndex];
  pEpStat->IsHalted          = 0;
  pEpStat->pData             = NULL;
  pEpStat->Buffer.NumBytesIn = 0;
  pEpStat->Buffer.RdPos      = 0;
  //
  // In case we have a EP in, we need to invalidate
  // the pending flag
  //
  if (pEpStat->EPAddr & 0x80) {
    pEpStat->Dir.TxInfo.TxIsPending       = 0;
  }
  //
  // Signal task
  //
  USB_OS_Signal(EPIndex);
}

/*********************************************************************
*
*       USB__InvalidateEP
*
*  Function description:
*    Set the stall flag on the control endpoint (EP0).
*    The flag will be reset after the next setup packet is received.
*
*/
void USB__StallEP0(void) {
  USB_Global.pDriver->pfStallEP0();
}


/*********************************************************************
*
*       USB_SetOnRXEPHook
*
*  Function description:
*
*
*  Parameters:
*    EPIndex    -  Any valid endpoint > 0.
*
*
*/
void USB_SetOnRXHookEP(U8 EPIndex, USB_ON_RX_FUNC * pfOnRx) {
  EP_STAT * pEpStat;

  pEpStat = &USB_aEPStat[EPIndex];
  pEpStat->Dir.pfOnRx = pfOnRx;
}


/*********************************************************************
*
*       USB_CancelIO
*
*  Function description:
*     Cancel any read endpoint operation.
*
*  Parameters:
*    EPIndex    -  Any valid endpoint > 0.
*
*
*/
void USB_CancelIO(U8 EPIndex) {
  USB_OS_IncDI();
  _RemovePendingOperation(EPIndex, 1);
  USB_OS_DecRI();
}



/*************************** End of file ****************************/

