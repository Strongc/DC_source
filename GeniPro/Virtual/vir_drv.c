/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:    GENIpro                                       */
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
/* MODULE NAME      : vir_drv.c                                             */
/*                                                                          */
/* FILE NAME        : vir_drv.c                                             */
/*                                                                          */
/* FILE DESCRIPTION : Common driver for all configurations                  */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "geni_cnf.h"                /* Access to GENIpro configuration      */
#include "common.h"                  /* Access to common definitions         */
#include "Geni_rtos_if.h"            /* Access to rtos definitions           */
#include "profiles.h"                /* Access to channel profiles           */
#include "geni_sys.h"                /* Access to system ressources          */
#include "vir_drv.h"                 /* Access to itself                     */
#if defined MAS_CH
  #include "vir_ctr_master.h"                 /* Access to controller configuration    */
#endif
#if ( CTO_BUS_TYPE != Disable )
#include "bus_drv_if.h"                     /* Access to channel configuration  */
#endif

#if ( CTO_IR_TYPE != Disable)
#include "ir_drv_if.h"                     /* Access to channel configuration  */
#endif

#if ( CTO_PLM_TYPE != Disable )
#include "plm_drv_if.h"                     /* Access to channel configuration  */
#endif

 #if ( CTO_COM_TYPE != Disable )
#include "com_drv_if.h"                     /* Access to channel configuration  */
#endif

#if ( CTO_RS232_TYPE != Disable )
#include "rs232_drv_if.h"                   /* Access to channel configuration  */
#endif

#if ( CTO_USB_TYPE != Disable )
#include "usb_drv_if.h"                   /* Access to channel configuration  */
#endif
/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

#if ( CTO_BUS_TYPE != Disable )
UCHAR BUS_tx_buf[BUS_DF_buf_len];
UCHAR BUS_rx_buf[BUS_DF_buf_len];
#endif

#if ( CTO_IR_TYPE != Disable)
UCHAR IR_tx_buf[IR_DF_buf_len];
UCHAR IR_rx_buf[IR_DF_buf_len];
#endif

#if ( CTO_PLM_TYPE != Disable )
UCHAR PLM_tx_buf[PLM_DF_buf_len];
UCHAR PLM_rx_buf[PLM_DF_buf_len];
#endif

 #if ( CTO_COM_TYPE != Disable )
UCHAR COM_tx_buf[COM_DF_buf_len];
UCHAR COM_rx_buf[COM_DF_buf_len];
#endif

#if ( CTO_RS232_TYPE != Disable )
UCHAR RS232_tx_buf[RS232_DF_buf_len];
UCHAR RS232_rx_buf[RS232_DF_buf_len];
#endif

#if (( CTO_USB_TYPE != Disable ) && ( TCS_USB_SER_PORT == 1 ))
UCHAR USB_tx_buf[USB_DF_buf_len];
UCHAR USB_rx_buf[USB_DF_buf_len];
#endif
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

#if (DEBUG == TRUE)

UINT geni_warning = 0;                                // count the number of geni runtime warnings
UINT crc_errors = 0;                                  // count the number of CRC errors
UINT data_errors = 0;                                 // count the number of data errors
UINT master_timeout_errors = 0;                       // count the number of timeout errors
UINT slave_timeout_errors = 0;                        // count the number of timeout errors
UINT rx_break_errors = 0;                             // count the number of receive break errors
ULONG tx_tgm_cnt = 0;
ULONG rx_tgm_cnt = 0;
UINT rx_cnt = 0;
DEBUG_BUF_TYPE rx_all_buf[BUF_SIZE];

#else
const ULONG tx_tgm_cnt = 0;
const ULONG rx_tgm_cnt = 0;
const UINT geni_warning = 0;
const UINT crc_errors = 0;
const UINT data_errors = 0;
const UINT master_timeout_errors = 0;
const UINT slave_timeout_errors = 0;
const UINT rx_break_errors = 0;

#endif
/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/
typedef struct  {
                UCHAR (*TransmitSetup) (void);
                void (*TransmitByte) (UCHAR tx_byte);
                void (*ResetDriver) (void);
                void (*InitDriver) (UCHAR p_setup_param);
                void (*DisableDriver) (void);
                }DRV_INTERFACE;

#if !(defined MAS_CH)
  #define sSLAVE_MODE            0
  static const UCHAR ma_state = sSLAVE_MODE;
#endif

// driver interface to hardware layer
// built array of interfaces
static const DRV_INTERFACE driver_if[NO_OF_CHANNELS] = {
#if(CTO_IR_TYPE != Disable)                                // Use IR
  {   TransmitSetupIR,    TransmitIRByte,     ResetIRDriver,    InitIRDriver,     DisableIRDriver}
  #if (MAX_CH_NO != IR_CH)
     ,                                                     // not last channel in array
  #else
     };                                                    // last channel in array
  #endif
#endif
#if(CTO_BUS_TYPE != Disable)                               // Use Bus
  {   TransmitSetupBUS,  TransmitBUSByte,    ResetBUSDriver,   InitBUSDriver,   DisableBUSDriver }
  #if (MAX_CH_NO != BUS_CH)
     ,                                                     // not last channel in array
  #else
     };                                                    // last channel in array
  #endif
#endif
#if(CTO_PLM_TYPE != Disable)                               // Use Powerline
  {   TransmitSetupPLM,  TransmitPLMByte,    ResetPLMDriver,   InitPLMDriver,   DisablePLMDriver }
  #if (MAX_CH_NO != PLM_CH)
     ,                                                     // not last channel in array
  #else
     };                                                    // last channel in array
  #endif
#endif
#if(CTO_COM_TYPE != Disable)                               // Use Com channel
  {   TransmitSetupCOM,   TransmitCOMByte,   ResetCOMDriver,   InitCOMDriver,    DisableCOMDriver }
  #if (MAX_CH_NO != COM_CH)
     ,                                                     // not last channel in array
  #else
     };                                                    // last channel in array
  #endif
#endif
#if(CTO_RS232_TYPE != Disable)                             // Use RS232 channel
  { TransmitSetupRS232, TransmitRS232Byte, ResetRS232Driver, InitRS232Driver, DisableRS232Driver }
  #if (MAX_CH_NO != RS232_CH)
     ,                                                     // not last channel in array
  #else
     };                                                    // last channel in array
  #endif
#endif
#if(CTO_USB_TYPE != Disable)                             // Use RS232 channel
  { TransmitSetupUSB, TransmitUSBByte, ResetUSBDriver, InitUSBDriver, DisableUSBDriver }
  #if (MAX_CH_NO != USB_CH)
     ,                                                     // not last channel in array
  #else
     };                                                    // last channel in array
  #endif
#endif

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
UCHAR TransmitGeniTgm(UCHAR ch_indx);
void InitGeniDriver(UCHAR ch_indx, UCHAR setup_param);
void DisableGeniDriver(UCHAR ch_indx);
void ResetGeniDriver(UCHAR ch_indx);
void SendTxByte(UCHAR ch_indx);
void SaveRxByte(UCHAR ch_indx, UCHAR rx_byte);
/****************************************************************************
*     Name      : InitGenibusDriver                                         *
*     Inputs    : channeladdress                                            *
*               : modbus on/off flag
*     Outputs   : None                                                      *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Perform initialisation of the genibus hardware            *
*               : Select RS485 hardware driver as default                   *
*---------------------------------------------------------------------------*/
void InitGeniDriver(UCHAR ch_indx, UCHAR setup_param)
{
  channels[ch_indx].buf_index = 0;
  channels[ch_indx].buf_cnt   = 0;                       // clear the buffer counter
  driver_if[ch_indx].InitDriver(setup_param);            // Initialise hardware
}
/****************************************************************************
*     Name      : DisableGenibusDriver                                      *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   : None                                                      *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Performs disabling of the genibus hardware                 *
*               :                                                           *
*---------------------------------------------------------------------------*/
void DisableGeniDriver(UCHAR ch_indx)
{
    channels[ch_indx].buf_index = 0;
    channels[ch_indx].buf_cnt   = 0;                       // clear the buffer counter
    driver_if[ch_indx].DisableDriver();                    // disable driver
}
/****************************************************************************
*     Name      :  ResetGeniReceiver                                        *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :  driverstate                                              *
*     Returns   :                                                           *
*   Description :  Prepare the Genibus driver for receiving the next        *
*               :  tgm.                                                     *
*---------------------------------------------------------------------------*/
void ResetGeniDriver(UCHAR ch_indx)
{
  channels[ch_indx].buf_index = 0;
  channels[ch_indx].buf_cnt   = 0;
  channels[ch_indx].drv_state = GENI_RECEIVE_WAIT;         // set state
  channels[ch_indx].flags    &= ~BUSY_FLG_MSK;             // Clear busy flag
  driver_if[ch_indx].ResetDriver();                        // reset receiver
}

/****************************************************************************
*     Name      : TransmitGeniTgm                                           *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Transmit tgm prepared in geni_ch_transmit_buf             *
*---------------------------------------------------------------------------*/
UCHAR TransmitGeniTgm(UCHAR ch_indx)
{
  UCHAR reply = rBUSY;
  if (channels[ch_indx].buf_index < SMALLEST_TGM_SIZE)               // Did we receive less than a whole tgm while processing
  {
    channels[ch_indx].buf_index = 0;                                 // reset index
    if ((channels[ch_indx].flags & USE_CRC16_MSK) != 0)              // flag set ?
      channels[ch_indx].buf_cnt = channels[ch_indx].tx_ptr[iLN]+3;   // get tx length
    else // SUM8
      channels[ch_indx].buf_cnt = channels[ch_indx].tx_ptr[iLN]+2;   // get tx length

    if (driver_if[ch_indx].TransmitSetup())                          // Set UART to transmit mode
    {
      #ifndef __PC__  // recursing
      channels[ch_indx].drv_state = GENI_TRANSMITTING;               // set state
      #endif
      
      reply = rOK;
    }
    else
      reply = rBUSY;
  }
  else
    channels[ch_indx].buf_index = 0;                                 // reset index

  return reply;
}
/****************************************************************************
*     Name      : SendTxByte                                                *
*     Inputs    : Byte to send                                              *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Initiate transmission                                     *
*                 Send byte to the bus                                      *
*---------------------------------------------------------------------------*/
void SendTxByte(UCHAR ch_indx)
{
  if (channels[ch_indx].buf_index <= channels[ch_indx].buf_cnt && channels[ch_indx].buf_index < 255)
  { // transmit next byte
    driver_if[ch_indx].TransmitByte(channels[ch_indx].tx_ptr[channels[ch_indx].buf_index++]);
  }
  else
  {
    DEBUG_INC_TX_CNT;
#ifndef __PC__
    #if (!(defined MIPS) && !(defined D70332X))                  // don't reset yet if mips or V850 ES IK1
    #if (CTO_IR_TYPE != Disable)
      if (ch_indx != IR_CH_INDX) ResetGeniDriver(ch_indx);       // reset driver if not IR channel
    #else
      ResetGeniDriver(ch_indx);                                  // reset driver
    #endif
    #endif

	#ifdef TCS_USB_SER_PORT
	#if (CTO_USB_TYPE != Disable)
	  if(ch_indx == USB_CH_INDX)
    {
		  UsbTxCompleted();
    }
	#endif
	#endif
#else
    ResetGeniDriver(ch_indx);
#endif
  }
}
/****************************************************************************
*     Name      :   SaveRxByte                                              *
*     Inputs    :   received byte                                           *
*     Outputs   :                                                           *
*     Updates   :   geni_ch_receive_buf, geni_buf_index                     *
*     Returns   :                                                           *
*     Description : Read data and save it in the receive buffer             *
*---------------------------------------------------------------------------*/
void SaveRxByte(UCHAR ch_indx, UCHAR rx_byte)
{
  CHANNEL_PARAM *ch_ptr;
  ch_ptr = &channels[ch_indx];

  switch (ch_ptr->drv_state )
  {
    case GENI_RECEIVE_WAIT:                                          // We expect to receive first tgm byte
      if (((MAS_CH_INDX == ch_indx) && ( ma_state != sSLAVE_MODE)
          && (rx_byte == GENI_REPLY))                                     // Master and reply
          || (((MAS_CH_INDX != ch_indx) || ( ma_state == sSLAVE_MODE))
          && ((rx_byte == GENI_REQUEST) ||(rx_byte == GENI_MESSAGE))))         // Slave and (message or request)
      {
        ch_ptr->buf_index = 0;                                       // reset index
        ch_ptr->rx_ptr[ch_ptr->buf_index++] = rx_byte;               // save data
        ch_ptr->drv_state = GENI_RECEIVE_HEAD;                       // and receive rest of header
      }
      else
      {
        ch_ptr->drv_state = GENI_IDLE_WAIT;
      }
    break;

    case GENI_RECEIVE_HEAD :                                         // receive the full header
    ch_ptr->rx_ptr[ch_ptr->buf_index++] = rx_byte;                   // save next byte
    if( ch_ptr->buf_index > 3 )                                      // all header info received?
    {
      if(ch_ptr->rx_ptr[iLN] < (BUS_DF_buf_len-3) )
      {
        if ((channels[ch_indx].flags & USE_CRC16_MSK) != 0)          // flag set?
          ch_ptr->buf_cnt = ch_ptr->rx_ptr[iLN]+3;                   // get tx length
        else // SUM8
          ch_ptr->buf_cnt = ch_ptr->rx_ptr[iLN]+2;                   // get tx length

        ch_ptr->drv_state = GENI_RECEIVING;                          // code optimizing
      }
      else
        ch_ptr->drv_state = GENI_RECEIVE_WAIT;                       // wait on new data
    }
    break;

    case GENI_RECEIVING:

    ch_ptr->rx_ptr[ch_ptr->buf_index++] = rx_byte;                   // no, store data
    if( ch_ptr->buf_index > ch_ptr->buf_cnt )                        // the complete tgm received?
    {
      ch_ptr->buf_cnt--;                                             // adjust ( code optimizing )
      ch_ptr->drv_state = GENI_PROCESSING;                           // then process the contents
      ch_ptr->buf_index = 0;                                         // reset index
      DEBUG_INC_RX_CNT;
      if ((MAS_CH_INDX == ch_indx) && ( ma_state != sSLAVE_MODE))    // Master
      {
        if( master_unit_addr == ch_ptr->rx_ptr[iDA])                 // tgm for us?
        {
          ClearGeniTimer(ch_const[ch_indx].reply_timer);             // clear the reply-timer
          SetGeniTimer(ch_const[ch_indx].inter_data_timer,ch_const[ch_indx].inter_data_time+1, (ch_indx<<6) + eNEW_TGM, GENI_TIMEOUT_TIMER);  // wait at least 5ms
        }
      }
      else                                                           // Slave
      {
        PutGeniEvent((ch_indx<<6) + eNEW_TGM );                      // signal that new tgm is arrived
        SetGeniTimer(ch_const[ch_indx].reply_timer,ch_const[ch_indx].reply_time, (ch_indx<<6) + eREPLY_TO, GENI_PUT_FIRST_TIMER);  // make sure to reply in time
      }
    }
    break;

    case GENI_PROCESSING:
      if ((MAS_CH_INDX != ch_indx) || ( ma_state == sSLAVE_MODE))    // Slave
        ch_ptr->buf_index++;                                         // count how many received while processing
    break;
    case GENI_IDLE_WAIT:
    case GENI_TRANSMITTING:
    break;
    default:
    break;
  }
}
/****************************************************************************
*     Name      :   GeniBusBusy                                             *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :   geni_busy_fl                                            *
*     Returns   :                                                           *
*     Description : Write to geni_busy_fl flag                              *
*---------------------------------------------------------------------------*/
void GeniChannelBusy(UCHAR ch_indx, UCHAR busy_flg)
{
  UCHAR old_flag = channels[ch_indx].flags & BUSY_FLG_MSK;

  if ( busy_flg == TRUE )
    channels[ch_indx].flags |= BUSY_FLG_MSK;                        // set the flag
  else
    channels[ch_indx].flags &= ~BUSY_FLG_MSK;                       // clear the flag

  //  we're receiving and busy_flag set? and new state not set - We got an idle channel
  if( ((channels[ch_indx].drv_state > GENI_RECEIVE_WAIT) && (channels[ch_indx].drv_state < GENI_PROCESSING))
      && (old_flag != 0) && (busy_flg == FALSE) )
  {
    DEBUG_INC_RX_BREAK_ERRORS;
    if ((MAS_CH_INDX != ch_indx) || ( ma_state == sSLAVE_MODE))    // slave
      ResetGeniDriver(ch_indx);                                    // Set geni_drv_state to GENI_RECEIVE_WAIT
    else                                                           // master
      PutGeniEvent((ch_indx<<6) + eRX_BRK );                       // signal to controller,
  }

  if( ((channels[ch_indx].drv_state == GENI_TRANSMITTING) || (channels[ch_indx].drv_state == GENI_IDLE_WAIT))
      && (busy_flg == FALSE) )
      ResetGeniDriver(ch_indx);                                    // Finished transmitting - reset


}
/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
