/*****************************************************************************/
/*****************************************************************************/
/***************************                    ******************************/
/***************************      GENIpro       ******************************/
/***************************                    ******************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                               com_drv_if.h                                */
/*                                                                           */
/*             Include file for GENI Driver Interface Functions               */
/*                                                                           */
/*                                                                         */
/*                                                                           */
/*                            Jan Soendergaard                               */
/*                                                                           */
/*                                                                           */
/*              Grundfos Electronics (C), Bjerringbro, Denmark               */
/*                                                                           */
/*                            October  2003                                  */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef _COM_DRV_IF_H
#define _COM_DRV_IF_H
#include "geni_if.h"                    // Access to Geni interface
//
//  Prototypes for the interface of low level and high level Geni drivers

//
//
// Low level functions
//
EXTERN void TransmitCOMByte(UCHAR tx_byte);
/****************************************************************************
*     Name      : TransmitCOMByte                                           *
*     Inputs    : Byte to send                                              *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Send byte to the channel                                  *
*---------------------------------------------------------------------------*/
EXTERN void ResetCOMDriver(void);
/****************************************************************************
*     Name      :  ResetCOMDriver                                           *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :  Direction pin and interrupt enabling                     *
*     Returns   :                                                           *
*   Description :  Reset Genichannel driver low level settings for receiving*
*               :  the next tgm.                                            *
*---------------------------------------------------------------------------*/
EXTERN void InitCOMDriver(UCHAR setup_param);
/****************************************************************************
*     Name      :  InitCOMDriver                                            *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Sets up the driver for receiving                         *
*               :                                                           *
*---------------------------------------------------------------------------*/
EXTERN void DisableCOMDriver(void);
/****************************************************************************
*     Name      :  DisableCOMDriver                                         *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Disables all interrupts                                  *
*               :                                                           *
*---------------------------------------------------------------------------*/
EXTERN UCHAR TransmitSetupCOM(void);
/****************************************************************************
*     Name      : TransmitSetupCOM                                          *
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
EXTERN void SaveRxByte(UCHAR ch_indx, UCHAR rx_byte);
/****************************************************************************
*     Name      :   SaveRxByte                                              *
*     Inputs    :   channel index and received byte                         *
*     Outputs   :                                                           *
*     Updates   :   geni_ch_receive_buf, geni_buf_index                     *
*     Returns   :                                                           *
*     Description : Read data and save it in the receive buffer             *
*---------------------------------------------------------------------------*/

EXTERN void GeniChannelBusy(UCHAR ch_indx, UCHAR busy_flg);
/****************************************************************************
*     Name      :   GeniChannelBusy                                         *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :   geni_busy_fl                                            *
*     Returns   :                                                           *
*     Description : Write to geni_busy_fl flag                              *
*---------------------------------------------------------------------------*/

EXTERN void SendTxByte(UCHAR ch_indx);
/****************************************************************************
*     Name      : SendTxByte                                                *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Sends the next byte or finishes the transmission          *
*---------------------------------------------------------------------------*/
#if(CTO_MOD_BUS == Enable)
EXTERN void SaveModBusRxByte(UCHAR ch_indx, UCHAR rx_byte);
/****************************************************************************
*     Name      :   SaveRxByte                                              *
*     Inputs    :   channel index and received byte                         *
*     Outputs   :                                                           *
*     Updates   :   geni_ch_receive_buf, geni_buf_index                     *
*     Returns   :                                                           *
*     Description : Read data and save it in the receive buffer             *
*---------------------------------------------------------------------------*/

EXTERN void GeniModBusChannelBusy(UCHAR ch_indx, UCHAR busy_flg);
/****************************************************************************
*     Name      :   GeniChannelBusy                                         *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :   geni_busy_fl                                            *
*     Returns   :                                                           *
*     Description : Write to geni_busy_fl flag                              *
*---------------------------------------------------------------------------*/

EXTERN void SendModBusTxByte(UCHAR ch_indx);
/****************************************************************************
*     Name      : SendTxByte                                                *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Sends the next byte or finishes the transmission          *
*---------------------------------------------------------------------------*/

#endif
#endif /* _DRV_IF_H  */
