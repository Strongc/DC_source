/*****************************************************************************/
/*****************************************************************************/
/***************************                    ******************************/
/***************************      GENIpro       ******************************/
/***************************                    ******************************/
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                               rs232_drv_if.h                                */
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
#ifndef _RS232_DRV_IF_H
#define _RS232_DRV_IF_H
#include "geni_if.h"                    // Access to Geni interface
//
//  Prototypes for the interface of low level and high level Geni drivers

//
//
// Low level functions
//
EXTERN void TransmitRS232Byte(UCHAR tx_byte);
/****************************************************************************
*     Name      : TransmitRS232Byte                                           *
*     Inputs    : Byte to send                                              *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Send byte to the channel                                  *
*---------------------------------------------------------------------------*/
EXTERN void ResetRS232Driver(void);
/****************************************************************************
*     Name      :  ResetRS232Driver                                           *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :  Direction pin and interrupt enabling                     *
*     Returns   :                                                           *
*   Description :  Reset Genichannel driver low level settings for receiving*
*               :  the next tgm.                                            *
*---------------------------------------------------------------------------*/
EXTERN void InitRS232Driver(UCHAR setup_param);
/****************************************************************************
*     Name      :  InitRS232Driver                                            *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Sets up the driver for receiving                         *
*               :                                                           *
*---------------------------------------------------------------------------*/
EXTERN void DisableRS232Driver(void);
/****************************************************************************
*     Name      :  DisableRS232Driver                                         *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Disables all interrupts                                  *
*               :                                                           *
*---------------------------------------------------------------------------*/
EXTERN UCHAR TransmitSetupRS232(void);
/****************************************************************************
*     Name      : TransmitSetupRS232                                          *
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
