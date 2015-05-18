/*****************************************************************************/
/*****************************************************************************/
/***************************                    ******************************/
/***************************      GENIpro       ******************************/
/***************************                    ******************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                               usb_drv_if.h                                */
/*                                                                           */
/*             Include file for GENI Driver Interface Functions              */
/*                                                                           */
/*                                                                         */
/*                                                                           */
/*                            ?????????????                                  */
/*                                                                           */
/*                                                                           */
/*              Grundfos Electronics (C), Bjerringbro, Denmark               */
/*                                                                           */
/*                            ?????????????                                  */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef _USB_DRV_IF_H
#define _USB_DRV_IF_H
#include "geni_if.h"                    // Access to Geni interface
//
//  Prototypes for the interface of low level and high level Geni drivers

//
//
// Low level functions
//
#ifdef TCS_USB_SER_PORT

EXTERN void TransmitUSBByte(GF_UINT8 tx_byte);
/****************************************************************************
*     Name      : TransmitUSBByte                                           *
*     Inputs    : Byte to send                                              *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Send byte to the channel                                  *
*---------------------------------------------------------------------------*/
EXTERN void ResetUSBDriver(void);
/****************************************************************************
*     Name      :  ResetUSBDriver                                           *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :  Direction pin and interrupt enabling                     *
*     Returns   :                                                           *
*   Description :  Reset Genichannel driver low level settings for receiving*
*               :  the next tgm.                                            *
*---------------------------------------------------------------------------*/
EXTERN void InitUSBDriver(GF_UINT8 setup_param);
/****************************************************************************
*     Name      :  InitUSBDriver                                            *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Sets up the driver for receiving                         *
*               :                                                           *
*---------------------------------------------------------------------------*/
EXTERN void DisableUSBDriver(void);
/****************************************************************************
*     Name      :  DisableUSBDriver                                         *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Disables all interrupts                                  *
*               :                                                           *
*---------------------------------------------------------------------------*/
EXTERN GF_UINT8 TransmitSetupUSB(void);
/****************************************************************************
*     Name      : TransmitSetupUSB                                          *
*     Inputs    :                                                           *
*     Outputs   : None                                                      *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Sets up the driver for transmitting                      *
*               :                                                           *
*---------------------------------------------------------------------------*/
//
// High level functions
//
EXTERN void SaveRxByte(GF_UINT8 ch_indx, GF_UINT8 rx_byte);
/****************************************************************************
*     Name      :   SaveRxByte                                              *
*     Inputs    :   channel index and received byte                         *
*     Outputs   :                                                           *
*     Updates   :   geni_ch_receive_buf, geni_buf_index                     *
*     Returns   :                                                           *
*     Description : Read data and save it in the receive buffer             *
*---------------------------------------------------------------------------*/

EXTERN void GeniChannelBusy(GF_UINT8 ch_indx, GF_UINT8 busy_flg);
/****************************************************************************
*     Name      :   GeniChannelBusy                                         *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :   geni_busy_fl                                            *
*     Returns   :                                                           *
*     Description : Write to geni_busy_fl flag                              *
*---------------------------------------------------------------------------*/

EXTERN void SendTxByte(GF_UINT8 ch_indx);
/****************************************************************************
*     Name      : SendTxByte                                                *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Sends the next byte or finishes the transmission          *
*---------------------------------------------------------------------------*/

EXTERN void UsbTxCompleted(void);

#endif
#endif /* _DRV_IF_H  */
