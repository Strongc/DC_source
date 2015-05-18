/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:                                                  */
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
/* MODULE NAME      : Vir_common.c                                          */
/*                                                                          */
/* FILE NAME        : Vir_commmon.c                                         */
/*                                                                          */
/* FILE DESCRIPTION : Contains the common routines for slave and master     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include <typedef.h>    /*definements for the Grundfos common types */
#include "common.h"                  // access to all common definitions
#include "crc.h"                     // access to the checksum routines
#include "profiles.h"                // access to the geni profile
#include "geni_sys.h"                // access to genipro system resources
#include "vir_common.h"              // access to virtual channel
#if ((CTO_CLASS_9 == Enable) && (VIR_CLASS_9 == Enable))
#include "vir_cl_9_tab.h"            // access to virtual class 9 tables
#endif
#if (USE_VIRTUAL_SLAVES == TRUE)
#include "vir_dia_slave.h"           // access to slave dialog layer
#endif


/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/
// 0 - 19   Accessibility
//static const UCHAR max_baud_rate   = (BUS_MAX_BAUDRATE == 9600 ? GENI_BAUD_9600 : (BUS_MAX_BAUDRATE == 19200 ? GENI_BAUD_19200 : (BUS_MAX_BAUDRATE == 38400 ? GENI_BAUD_38400 : GENI_BAUD_NO_CONF)))
#if (BUS_MAX_BAUDRATE == 9600)
  static const UCHAR max_baud_rate = GENI_BAUD_9600;
#elif (BUS_MAX_BAUDRATE == 19200)
  static const UCHAR max_baud_rate = GENI_BAUD_19200;
#elif (BUS_MAX_BAUDRATE == 38400)
  static const UCHAR max_baud_rate = GENI_BAUD_38400;
#else
  static const UCHAR max_baud_rate = GENI_BAUD_NO_CONF;        // Configurable baudrate not supported (fixed to 9600)
#endif  

const UCHAR df_buf_len        = DF_buf_len;                    // lenght of receive/transmit buf
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif
extern UCHAR unit_bus_mode;

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

#if ((CTO_COM_TYPE == Master) && !((CTO_RS232_TYPE != Disable) || (CTO_BUS_TYPE != Disable) || (CTO_PLM_TYPE != Disable) || (CTO_IR_TYPE != Disable)))
static const UINT operation_acc_ctr = 0;
static const UINT operation_acc_ctr_high = 0;
#else
extern UINT operation_acc_ctr;
extern UINT operation_acc_ctr_high;
#endif

static const UCHAR cto_class_8       = CTO_CLASS_8;
static const UCHAR cto_class_9       = CTO_CLASS_9;
static const UCHAR cto_class_10      = CTO_CLASS_10;
static const UCHAR cto_class_16_bit  = CTO_CLASS_16_BIT;
static const UCHAR cto_class_32_bit  = CTO_CLASS_32_BIT;

// 20 - 29  Receive and Transmit buffers
static const UCHAR bus_df_buf_len    = BUS_DF_buf_len;
static const UCHAR plm_df_buf_len    = PLM_DF_buf_len;
static const UCHAR rs232_df_buf_len  = RS232_DF_buf_len;
static const UCHAR com_df_buf_len    = COM_DF_buf_len;
static const UCHAR ir_df_buf_len     = IR_DF_buf_len;
// 30 - 49  Buffers
static const UCHAR router_df_buf_len = ROUTER_DF_buf_len;
static const UCHAR object_df_buf_len = OBJECT_DF_buf_len;
static const UCHAR cto_buf_opt       = CTO_BUF_OPT;
#if ( CTO_BUF_OPT == Enable )
extern const UINT buf_opt_ctr;
#else
static const UINT buf_opt_ctr        = FALSE;
#endif
static const UCHAR cmd_buf_len       = CMD_BUF_LEN;
static const UCHAR conf_buf_len      = CONF_BUF_LEN;
static const UCHAR ref_buf_len       = REF_BUF_LEN;
static const UCHAR cto_buf16_opt     = CTO_BUF16_OPT;
static const UCHAR conf16_buf_len    = CONF16_BUF_LEN;
static const UCHAR ref16_buf_len     = REF16_BUF_LEN;
static const UCHAR cto_buf32_opt     = CTO_BUF32_OPT;
static const UCHAR conf32_buf_len    = CONF32_BUF_LEN;
static const UCHAR ref32_buf_len     = REF32_BUF_LEN;
// 50 - 59 Geni Master
#if (defined MAS_CH)
extern UINT all_master_errors;
extern UCHAR ma_state;
#else
static const UCHAR all_master_errors = 0;
static const UCHAR ma_state = 0;
#endif
// 60 - 84 Geni Channel specific states
extern CHANNEL_PARAM channels[NO_OF_CHANNELS];
// 60 - 64 Bus channel
#if ( CTO_BUS_TYPE != Disable)
#define bus_ch_drv_state       channels[BUS_CH_INDX].drv_state
#define bus_ch_flags           channels[BUS_CH_INDX].flags
#else
static const UCHAR bus_ch_drv_state   = 0;
static const UCHAR bus_ch_flags       = 0;
#endif
// 65 - 69 Ir channel
#if ( CTO_IR_TYPE != Disable)
#define ir_ch_drv_state        channels[IR_CH_INDX].drv_state
#define ir_ch_flags            channels[IR_CH_INDX].flags
#else
static const UCHAR ir_ch_drv_state    = 0;
static const UCHAR ir_ch_flags        = 0;
#endif
// 70 - 74 COM channel
#if ( CTO_COM_TYPE != Disable)
#define com_ch_drv_state       channels[COM_CH_INDX].drv_state
#define com_ch_flags           channels[COM_CH_INDX].flags
#else
static const UCHAR com_ch_drv_state   = 0;
static const UCHAR com_ch_flags       = 0;
#endif
// 75 - 79 RS232 channel
#if ( CTO_RS232_TYPE != Disable)
#define rs232_ch_drv_state     channels[RS232_CH_INDX].drv_state
#define rs232_ch_flags         channels[RS232_CH_INDX].flags
#else
static const UCHAR rs232_ch_drv_state = 0;
static const UCHAR rs232_ch_flags     = 0;
#endif
// 80 - 84 PLM channel
#if ( CTO_PLM_TYPE != Disable)
#define plm_ch_drv_state       channels[PLM_CH_INDX].drv_state
#define plm_ch_flags           channels[PLM_CH_INDX].flags
#else
static const UCHAR plm_ch_drv_state   = 0;
static const UCHAR plm_ch_flags       = 0;
#endif
// 100 - 102 Geni Version
static const UCHAR major_ver         = (UCHAR)(GENI_VERSION_VAL >> 16);
static const UCHAR minor_ver         = (UCHAR)(GENI_VERSION_VAL >> 8);
static const UCHAR revision_ver      = (UCHAR) GENI_VERSION_VAL;

/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* Class 0 table                                                            */
/****************************************************************************/
//  Data Item Pointer tables for GENIpro parameters  - must be placed here because it's using local vars
const ID_PTR   pro_tab[HIGH_PRO_ID + 1]  = {
  0,                                                       /* 0  */
  (UCHAR *) &max_baud_rate,                                /* 1  */
  (UCHAR *) &df_buf_len,                                   /* 2  */
  &unit_bus_mode,                                          /* 3  */
  (((UCHAR *)&operation_acc_ctr) + 1),                     /* 4  */
  ( UCHAR *)&operation_acc_ctr,                            /* 5  */
  0,0,0,0,                                                 /* 6,7,8,9 */
  0,                                                       /* 10 */
  (UCHAR *)&cto_class_8,                                   /* 11 */
  (UCHAR *)&cto_class_9,                                   /* 12 */
  (UCHAR *)&cto_class_10,                                  /* 13 */
  (UCHAR *)&cto_class_16_bit,                              /* 14 */
  (UCHAR *)&cto_class_32_bit,                              /* 15 */
  0,0,0,0,                                                 /* 16,17,18,19 */
  (UCHAR *)&bus_df_buf_len,                                /* 20 */
  (UCHAR *)&ir_df_buf_len,                                 /* 21 */
  (UCHAR *)&plm_df_buf_len,                                /* 22 */
  (UCHAR *)&rs232_df_buf_len,                              /* 23 */
  (UCHAR *)&com_df_buf_len,                                /* 24 */
  0,0,0,0,0,                                               /* 25,26,27,28,29 */
  (UCHAR *)&router_df_buf_len,                             /* 30 */
  (UCHAR *)&object_df_buf_len,                             /* 31 */
  (UCHAR *)&cto_buf_opt,                                   /* 32 */
  (UCHAR *)&buf_opt_ctr,                                   /* 33 */
  (UCHAR *)&cmd_buf_len,                                   /* 34 */
  (UCHAR *)&conf_buf_len,                                  /* 35 */
  (UCHAR *)&ref_buf_len,                                   /* 36 */
  (UCHAR *)&cto_buf16_opt,                                 /* 37 */
  (UCHAR *)&cto_buf32_opt,                                 /* 38 */
  (UCHAR *)&conf16_buf_len,                                /* 39 */
  (UCHAR *)&ref16_buf_len,                                 /* 40 */
  (UCHAR *)&conf32_buf_len,                                /* 41 */
  (UCHAR *)&ref32_buf_len,                                 /* 42 */
  0,0,0,0,0,0,0,                                           /* 43,44,45,46,47,48,49 */
  (((UCHAR *)&all_master_errors) + 1),                     /* 50 */
  ((UCHAR *)&all_master_errors),                           /* 51 */
  ((UCHAR *)&ma_state),                                    /* 52 */
  0,0,0,0,0,0,0,                                           /* 53,54,55,56,57,58,59 */
  (UCHAR *)&bus_ch_drv_state,                              /* 60 */
  (UCHAR *)&bus_ch_flags,                                  /* 61 */
  0,0,0,                                                   /* 62,63,64 */
  (UCHAR *)&ir_ch_drv_state,                               /* 65 */
  (UCHAR *)&ir_ch_flags,                                   /* 66 */
  0,0,0,                                                   /* 67,68,69 */
  (UCHAR *)&com_ch_drv_state,                              /* 70 */
  (UCHAR *)&com_ch_flags,                                  /* 71 */
  0,0,0,                                                   /* 72,73,74 */
  (UCHAR *)&rs232_ch_drv_state,                            /* 75 */
  (UCHAR *)&rs232_ch_flags,                                /* 76 */
  0,0,0,                                                   /* 77,78,79 */
  (UCHAR *)&plm_ch_drv_state,                              /* 80 */
  (UCHAR *)&plm_ch_flags,                                  /* 81 */
  0,0,0,                                                   /* 82,83,84 */
  0,0,0,0,0,                                               /* 85 - 89 */
  0,0,0,0,0,0,0,0,0,0,                                     /* 90 - 99 */
  (UCHAR *)&major_ver,                                     /* 100 */
  (UCHAR *)&minor_ver,                                     /* 101 */
  (UCHAR *)&revision_ver,                                  /* 102 */
  0,0,0,0,0,0,0,                                           /* 103 - 109 */
  (((UCHAR *)&tx_tgm_cnt) + 3),                            /* 110 */
  (((UCHAR *)&tx_tgm_cnt) + 2),                            /* 111 */
  (((UCHAR *)&tx_tgm_cnt) + 1),                            /* 112 */
  (((UCHAR *)&tx_tgm_cnt) + 0),                            /* 113 */
  (((UCHAR *)&rx_tgm_cnt) + 3),                            /* 114 */
  (((UCHAR *)&rx_tgm_cnt) + 2),                            /* 115 */
  (((UCHAR *)&rx_tgm_cnt) + 1),                            /* 116 */
  (((UCHAR *)&rx_tgm_cnt) + 0),                            /* 117 */
  (((UCHAR *)&crc_errors) + 1),                            /* 118 */
  (((UCHAR *)&crc_errors) + 0),                            /* 119 */
  (((UCHAR *)&data_errors) + 1),                           /* 120 */
  (((UCHAR *)&data_errors) + 0),                           /* 121 */
  (((UCHAR *)&master_timeout_errors) + 1),                 /* 122 */
  (((UCHAR *)&master_timeout_errors) + 0),                 /* 123 */
  (((UCHAR *)&slave_timeout_errors) + 1),                  /* 124 */
  (((UCHAR *)&slave_timeout_errors) + 0),                  /* 125 */
  (((UCHAR *)&rx_break_errors) + 1),                       /* 126 */
  (((UCHAR *)&rx_break_errors) + 0),                       /* 127 */
  (((UCHAR *)&geni_warning) + 1),                          /* 128 */
  (((UCHAR *)&geni_warning) + 0)                           /* 129 */
  };


/****************************************************************************/
/* Class 1 table                                                            */
/****************************************************************************/

const ID_PTR ch_tab[HIGH_CH_ID+1]  = { 0 };


/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

#if( CTO_CLASS_9 == Enable )
  UCHAR routing_status;
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

#if( CTO_CLASS_9 == Enable )
  UCHAR routing_buf[ROUTER_DF_buf_len];        // reroute of PDU's
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
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

/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
#if( CTO_CLASS_9 == Enable )
/****************************************************************************
*     Name      : GetRoutingPDU                                             *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : TRUE if transmit_buf is big enough else FALSE             *
*               :                                                           *
*   Description : Copies a PDU from routing_buf to transmit_buf.            *
*               : If transmit_buf is not big enough to hold the PDU         *
*               : and the framing no copying takes place                    *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR GetRoutingPDU(UCHAR buffer_len, UCHAR framing_size)
{
   UCHAR i, len, ret_code = TRUE;

   len = routing_buf[1] - 2;                   // find the size of the internal PDU

   if ( len <= buffer_len - framing_size)
   {                                           // transmit_buf long enough
      len += PDU_BODY;
      for ( i = PDU_BODY; i < len; i++ )
      {                                        // Copy from one to another
         cur_transmit_buf[i] = routing_buf[i];
      }
      *cur_buf_cnt = len;                      // save len of telegram
      routing_status = sROUTING_BUSY;
   }
   else
   {                                           // Buffer length exceeded
      routing_status = sROUTING_ERROR;
      ret_code = FALSE;
   }
   return ret_code;
}
/****************************************************************************
*     Name      : SetRoutingPDU                                             *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Copies a PDU from com_ch_receive_buf to routing_buf       *
*               : stored in receive_buf                                     *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void SetRoutingPDU(void)
{
   UCHAR i;
   for ( i = PDU_BODY; i < (cur_receive_buf[iLN] + 2); i++ )
     routing_buf[i] = cur_receive_buf[i];

   routing_buf[iLN] = cur_receive_buf[iLN];    // Store PDU len
   routing_status   = sROUTING_REPLY_READY;

}
#endif //   CTO_CLASS_9 code end

/****************************************************************************
*     Name      : CheckHeader                                               *
*               :                                                           *
*     Inputs    : own_addr -  the channels own_address                      *
*               : Pointer to reply flag.                                    *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :  reply - Is set to TRUE if request telegram else FALSE    *
*     Updates   :                                                           *
*     Returns   :  TRUE if tgm to us                                        *
*               :                                                           *
*   Description : Examine the header to see if the telegram is for us       *
*               : When connected the connect_addr is assigned with the      *
*               : same value as the own_addr                                *
*---------------------------------------------------------------------------*/
UCHAR CheckHeader(UCHAR own_addr, UCHAR *reply)
{
  UCHAR status = FALSE;
  *reply = FALSE;

  if( cur_receive_buf[iSD] == GENI_REPLY)
  { // Tgm is a reply
    if (cur_receive_buf[iDA] == own_addr)
      status = TRUE;
  }
  else if( (cur_receive_buf[iSD] == GENI_REQUEST) || ( cur_receive_buf[iSD] == GENI_MESSAGE) )
  { // Tgm is a message or request
    if( (cur_receive_buf[iDA] == BROADCAST ) ||
        (cur_receive_buf[iDA] == slave_group_addr ) ||
        (cur_receive_buf[iDA] == cur_ch->connect_addr ) ||
        (cur_receive_buf[iDA] == own_addr))
    { // Tgm is for us
      if ( cur_receive_buf[iDA] == own_addr )
        cur_ch->flags |= POLLED_FLG_MSK;              // tgm was to unit_addr - set the flag

      if ( cur_receive_buf[iSD] == 0x27 )
        *reply = TRUE;                                // Tgm is a request and we must reply

      status = TRUE;

    #if (USE_VIRTUAL_SLAVES == FALSE)
    }
    #else // This is a subslave implementation.
      cur_unit = NO_UNIT;
    }
    else  // Is the tgm sent to a virtual slave
    {
      if (VIR_SLAVE_CHANNEL == cur_channel)
      {
        if (cur_receive_buf[iDA] == CONNECTION)         // handle a connection request
          cur_unit = FindNext2Connect();                // find if any can reply to a connect request.
        else
          cur_unit = FindSlave(cur_receive_buf[iDA]);   // find if any address match with a virtual slave

        if (cur_unit != NO_UNIT)
        {
          status = TRUE;                                // check for any matches
          if ( cur_receive_buf[iDA] == vir_slave_list[cur_unit].unit_addr)
            vir_slave_list[cur_unit].polled = TRUE;
          else
            vir_slave_list[cur_unit].polled = FALSE;


          if ( cur_receive_buf[iSD] == 0x27 )
            *reply = TRUE;                              // Tgm is a request and we must reply
        }
      }
      else
        cur_unit = NO_UNIT;
    }
    #endif
}

  return status;
}
/****************************************************************************
*     Name      :  CheckCRC                                                 *
*               :                                                           *
*     Inputs    :  cur_receive_buf, cur_ch.flags                            *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :  Returns TRUE if CRC/SUM is ok                            *
*               :                                                           *
*   Description :  Checks which check rotine to use and calls the right     *
*               :  routine                                                  *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR CheckCRC(void)
{
  UCHAR ret_val = 0;
  #ifdef GENI_CRC_16
  if ( (cur_ch->flags & USE_CRC16_MSK) != 0 )         // flag set ?
    ret_val = CheckCRC16( cur_receive_buf );
  #endif
  #ifdef GENI_SUM_8
  if ( (cur_ch->flags & USE_CRC16_MSK) == 0 )         // flag not set?
    ret_val = CheckSUM8( cur_receive_buf );
  #endif
  return ret_val;
}
/****************************************************************************
*     Name      :  AddCRC                                                   *
*               :                                                           *
*     Inputs    :  cur_transmit_buf, cur_ch.flags                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :  cur_transmit_buf                                         *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Adds the checksum to tx_buf used for the current channel *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void AddCRC(void)
{
  #ifdef GENI_CRC_16
  if ( (cur_ch->flags & USE_CRC16_MSK) !=  0 )        // flag set ?
    CalcCRC16(cur_transmit_buf);                      //  call the CRC calculation function
  #endif
  #ifdef GENI_SUM_8
  if ( (cur_ch->flags & USE_CRC16_MSK) == 0 )         // flag not set ?
    CalcSUM8(cur_transmit_buf);                       //  call the SUM calculation function
  #endif
}
/****************************************************************************
*     Name      : AddHeader                                                 *
*               :                                                           *
*     Inputs    : sd_type           // start delimiter                      *
*               : dest_addr         // destination address                  *
*               : own_addr          // own addr                             *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   : cur_transmit_buf                                          *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Add Genipro header to the data contents of tgm            *
*               :                                                           *
*               :      0    Startdelimiter                                  *
*               :      1    Length ( number of bytes in tgm - 4 )           *
*               :      2    Destination address                             *
*               :      3    Source address                                  *
*               :      4    data ...                                        *
*---------------------------------------------------------------------------*/
void AddHeader(UCHAR dest_addr, UCHAR own_addr, UCHAR sd_type)
{
  cur_transmit_buf[iSD] = sd_type;                    //  save start delimiter
  cur_transmit_buf[iLN] = *cur_buf_cnt-2;             //  save lenght
  cur_transmit_buf[iDA] = dest_addr;                  //  save destination address
  cur_transmit_buf[iSA] = own_addr;                   //  save the channel source address
  *cur_buf_cnt += 2;
}
/****************************************************************************
*     Name      : CheckTgm                                                  *
*               :                                                           *
*     Inputs    : cur_receive_buf                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : 0 or error code for the class that failed                 *
*               :                                                           *
*   Description : Check received tgm for class errors                       *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR CheckTgm(void)
{
   UCHAR ret_code = 0;
   UCHAR *rx_pdu_ptr, rx_pdu_len;

   rx_pdu_ptr = cur_receive_buf + HEADER_LEN + 1;     // Point to ACK
   rx_pdu_len = cur_receive_buf[iLN] - 2;             // length of APDU's - no addresses

   while ( rx_pdu_len && !ret_code)                   // Process all APDUs - ACK Ok?
   {
     ret_code = (*rx_pdu_ptr & 0xC0);                 // mask ACK
     // Move to next APDU
     rx_pdu_len -= ((*rx_pdu_ptr & 0x3F) + 2);        // Bytes left to process
     rx_pdu_ptr += ((*rx_pdu_ptr & 0x3F) + 2);        // Point to next ACK

   }
   return !ret_code;
}
/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/


