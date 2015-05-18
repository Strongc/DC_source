/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIpro                                         */
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
/* MODULE NAME      :    vir_ctr_master.c                                   */
/*                                                                          */
/* FILE NAME        :    vir_ctr_master.c                                   */
/*                                                                          */
/* FILE DESCRIPTION :    Control Module to Geni master channel              */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include <typedef.h>            /*definements for the Grundfos common types */
#include "geni_cnf.h"
#include "common.h"                     /* Access to common definitions     */
#include "profiles.h"                   /* Access to channel profiles       */
#include "geni_sys.h"                   /* Access to system ressources      */
#include "vir_common.h"                 /* Access to virtual channel        */
#if (CTO_COM_TYPE == Master)
  #include "com_dia_master.h"           /* Acces to COM master dialog       */
#elif ((CTO_IR_TYPE == Master) || (CTO_BUS_TYPE == Master) || (CTO_PLM_TYPE == Master) || (CTO_RS232_TYPE == Master))
  #include "vir_tab_master.h"           /* Access to vir. master tables     */
  #include "vir_dia_master.h"           /* Access to the vir. master dialog */
  #include "vir_ctr_slave.h"
#endif
#include "vir_drv.h"                    /* access to driver layer           */
#include "vir_ctr_master.h"             /* Access to itself                 */

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
// Definition of struction for InterComControlHandler decision table
typedef struct               {   UCHAR event;
                                 UCHAR cur_dialog_state;
                                 UCHAR next_dialog_state;
                                 void (*action_function) (void);
                             } MAS_DECISION_TABLE;

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

    UCHAR ma_state;                                   // intercom control state
    UINT all_master_errors;                           // total number of errors

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/
#if ( CTO_CLASS_9 == Enable )
  static UCHAR r_poll_count;                          // poll counter;
  static UCHAR retry_r;                               // retry counter for rounting
  static UCHAR routing_poll;                          // rounting poll flag
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif
/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
UCHAR SendRoutedTelegram(void);
void ResetDriver(void);
void ProcessRouteReply(void);
UCHAR SendRoutingPoll(void);
void NoReplyRouteRequest(void);
/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/
#if (PROCESSOR_TYPE == D780988)
  #pragma codeseg(GENICOD)
#endif
//   control table for Genipro bus mas communication
const MAS_DECISION_TABLE mas_control_tab[] = {
//
//         event   current state |  next state       action
//
#if ( CTO_CLASS_9 == Enable )
    {         eNEW_TGM,   sMROUTING,      sREADY,        ProcessRouteReply  },
    {        eREPLY_TO,   sMROUTING,      sREADY,        NoReplyRouteRequest},
#endif

#if (CTO_COM_TYPE == Master)
    {           eCLOCK,  sDONT_CARE,  sDONT_CARE,        CheckComPending    },
    {         eNEW_TGM, sWAIT_REPLY,      sREADY,        ProcessComTgm      },
    {        eSEND_TGM,  sBUILD_TGM, sWAIT_REPLY,        ComSendTgm         },
    {    eTRANS_FAILED, sWAIT_REPLY,      sREADY,        NoComReply         },
    {    eTRANS_FAILED,      sREADY,      sREADY,        NoComReply         },
    {        eREPLY_TO,  sDONT_CARE,  sDONT_CARE,        RetransmitComTgm   },
    {      eRETRANSMIT,  sDONT_CARE,  sDONT_CARE,        RetransmitComTgm   },
    {          eRX_BRK,  sDONT_CARE,  sDONT_CARE,        RetransmitComTgm   }};

#elif ((CTO_IR_TYPE == Master) || (CTO_BUS_TYPE == Master) || (CTO_PLM_TYPE == Master) || (CTO_RS232_TYPE == Master))
    {           eCLOCK,  sDONT_CARE,  sDONT_CARE,        CheckPending       },
    {         eNEW_TGM, sSLAVE_MODE, sSLAVE_MODE,        ProcesGeniTgm      },
    {        eSEND_TGM, sSLAVE_MODE, sSLAVE_MODE,        SendSlaveReply     },
    { eCONNECT_TIMEOUT, sSLAVE_MODE, sSLAVE_MODE,        SlaveConnectTmOut  },
    {        eREPLY_TO, sSLAVE_MODE, sSLAVE_MODE,        SlaveDontReply     },
    {         eNEW_TGM,    sDIR_REQ,      sREADY,        ProcessDirRequest  },
    {        eREPLY_TO,    sDIR_REQ,      sREADY,        NoReplyDirRequest  },
    {          eRX_BRK,    sDIR_REQ,      sREADY,        NoReplyDirRequest  },
    {        eSEND_DIR,      sREADY,    sDIR_REQ,        SendDirRequest     },
    {         eNEW_TGM,   sAUTO_REQ,      sREADY,        ProcessAutoRequest },
    {        eREPLY_TO,   sAUTO_REQ,      sREADY,        NoReplyAutoRequest },
    {          eRX_BRK,   sAUTO_REQ,      sREADY,        NoReplyAutoRequest },
    {        eAUTO_REQ,      sREADY,   sAUTO_REQ,        SendAutoRequest    },
    {       eGROUP_END,      sREADY,      sREADY,      FindNextWaitingGroup },
    {         eUPD_GRP,      sREADY,   sAUTO_REQ,      FirstRequestInGroup  },
    {       eNEXT_UNIT,   sAUTO_REQ,      sREADY,      GotoNextAutoRequest  },
    {       eNEXT_UNIT,      sREADY,      sREADY,      GotoNextAutoRequest  }};
#endif
const UCHAR MAS_CONTROL_LEN = ( sizeof(mas_control_tab) / sizeof( MAS_DECISION_TABLE) );

/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
*     Name      :  ResetMaster                                              *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :  all_master_errors, retry_r                               *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Resets the entire system                                 *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ResetMaster(void)
{
#if ( CTO_CLASS_9 == Enable )
  retry_r = 0;
#endif
  all_master_errors = 0;                                    // reset error count.
  ClearGeniTimer(ch_const[MAS_CH_INDX].connect_timer);      // make sure timer is stopped
  ClearGeniTimer(ch_const[MAS_CH_INDX].reply_timer);        // make sure timer is stopped
  ClearGeniTimer(ch_const[MAS_CH_INDX].inter_data_timer);   // make sure timer is stopped
  ClearGeniTimer(ch_const[MAS_CH_INDX].poll_timer);         // make sure timer is stopped
  ClearGeniTimer(ch_const[MAS_CH_INDX].vir_connect_timer);  // make sure timer is stopped

  #if(CTO_COM_TYPE == Master)
    ResetPointToPointDia();                                 // reset PP master
    ma_state = sREADY;                                      // ready state
  #else
    ma_state = sSLAVE_MODE;                                 // Start in slave mode slave
  #endif
}

/****************************************************************************
*     Name      :  InitMaster                                               *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Enable and initialise the Geni master channel            *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void InitMaster(void)
{
  #if(CTO_COM_TYPE == Master)
    InitPointToPointDia();                            // Init PP master
  #else
    InitMultiPointDia();                              // Init MP master
  #endif
}
/****************************************************************************
*     Name      :  DisableMaster                                            *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Disable the Geni master channel                          *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void DisableMaster(void)
{
  #if(CTO_COM_TYPE == Master)
    DisablePointToPointDia();                         // Disable PP master
  #else
    DisableMultiPointDia();                           // Disable MP master
  #endif
  ma_state = sREADY;                                  // ready state
}

/****************************************************************************
*     Name      :  MasterControlHandler                                     *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  The MasterControlHandler is called when an new event is  *
*               :  generated by the Geni Master channel. The MasterControl- *
*               :  Handler then process the event by calling the related    *
*               :  actionfunctions                                          *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/

void MasterControlHandler(UCHAR event)
{
  UCHAR i;
  UCHAR c;
  ADD_2_DEBUG_BUF(event);

  c = FALSE;
#if ( CTO_CLASS_9 == Enable )
  if (event == eROUTING_REQUEST)
  {
    if ( ma_state == sREADY )
    {                                           // Channel idle
      if( SendRoutedTelegram() )                // Telegram fit in buffer
        ma_state = sMROUTING;                   // Enter router state
    }
    else                                        // Channel busy
    {
      PutExtEvent(MAS_CH + eROUTING_REQUEST);   // Save event for later use
    }
  }
  else if (event == eROUTING_POLL)
  {
    if ( (ma_state == sREADY) && (SendRoutingPoll() == rOK))
    {                                           // Channel idle
       ma_state = sMROUTING;                    // Enter router state
    }
    else                                        // Channel busy
    {
      PutExtEvent(MAS_CH + eROUTING_POLL);      // Save event for later use
    }
  }
  else
#endif
       #if (CTO_COM_TYPE != Master)

    if (SendWaitingDirectTgm())                // any direct telegrams
      ma_state = sDIR_REQ;
    else
   #endif

    {
      for( i = 0; (i < MAS_CONTROL_LEN) && !c; i++ )
      {
        if( event == mas_control_tab[i].event )
        {
          if(( mas_control_tab[i].cur_dialog_state == ma_state ) ||
            (mas_control_tab[i].cur_dialog_state == sDONT_CARE)  )

          { // don't change the ma_state if it was a DONT_CARE
            if (mas_control_tab[i].cur_dialog_state != sDONT_CARE)
              ma_state = mas_control_tab[i].next_dialog_state;
            mas_control_tab[i].action_function();
            c = TRUE;
          }
        }
      }
    }
}
#if ( CTO_CLASS_9 == Enable )
/****************************************************************************
*     Name      :  ProcessRouteReply                                        *
*               :                                                           *
*     Inputs    :  none                                                     *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :                                                           *
*     Returns   :  none                                                     *
*               :                                                           *
*   Description :                                                           *
*               :  This function processes a route reply from the slave     *
*               :  if routing is initiated or busy the master will poll for *
*               :  reply, else the master will return FAILED or READY       *
*               :  to genipro system                                        *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ProcessRouteReply(void)
{
UCHAR reply = FALSE;

  ClearGeniTimer(ch_const[MAS_CH_INDX].reply_timer);  // stop reply timer
  routing_poll = FALSE;                               // clear routing_poll flag

  if( CheckCRC()  )                                   // just check CRC
  {
    retry_r = 0;                                      // clear retry counter
    if(cur_receive_buf[PDU_BODY]==REROUTE_APDU)       // we are rounting more than one time !
    {
      if(cur_receive_buf[PDU_BODY+1]==1)              // received rounting status
      {
        switch(cur_receive_buf[PDU_BODY+2])
        {
          case 1:                                     // rounting initiated
          case 2:                                     // rounting busy
            r_poll_count = 2;                         // wait at least one eCLOCK
          break;
          default:
          case 4:                                     // rounting failed
            routing_status = sROUTING_ERROR;          // Update routing status
          break;
        }
      }
      else reply = TRUE;                              // received rounting reply
    }
    else reply = TRUE;                                // normal reply is ready

    if(reply==TRUE)                                   // reply with rounting telegram
    {
      SetRoutingPDU();                                // copy to routing buf
    }
  }
  else
  {
    NoReplyRouteRequest();                            // data received, but no OK
  }
#if(CTO_COM_TYPE != Master)
  if( group_active)                                   //  going to continue bus requesting?
    PutExtEvent(MAS_CH+eAUTO_REQ);                    //  yes, then signal it
#if(MAS_SHEDULE_METOD == TICK)                        //  if no timer used
  else
    PutExtEvent(MAS_CH+eCLOCK);                       //  generate tick
#endif
#endif
}
/****************************************************************************
*     Name      : NoReplyRouteRequest                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : If we have not tried to retransmit MAX_BUS_RETRIES times  *
*               : then send the rounting tgm again else report error to     *
*               : genipro system.                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void NoReplyRouteRequest(void)
{
  #if ((CTO_IR_TYPE == Master) || (CTO_BUS_TYPE == Master) || (CTO_PLM_TYPE == Master) || (CTO_RS232_TYPE == Master))
     // multipoint masters
UCHAR unit_index;

    unit_index = FindUnit( routing_buf[iRA] );        // find unit index
    if( unit_index != NO_UNIT )                       // unit found ?
    {
      if( network_list[unit_index].bus_errors < 254 )
        network_list[unit_index].bus_errors++;        // inc error rate for dialog
      network_list[unit_index].unit_err_rate++;       // inc error rate for unit
      if(network_list[unit_index].bus_errors == MAS_UNIT_ERR_HYS)
        {ErrNotifyApp(unit_index, 1);}                // Call user function if any
    }
  #endif
  ++all_master_errors;
  if( ++retry_r <= MAS_MAX_RETRIES )
  {
    if(routing_poll)
    {
      PutExtEvent(MAS_CH + eROUTING_POLL);            // try again
    }
    else
    {
      PutExtEvent(MAS_CH + eROUTING_REQUEST);         // try again
    }
  }
  else
  {
    routing_status = sROUTING_ERROR;                  // Update routing status
    retry_r = 0;                                      // clear retry counter
    ResetGeniDriver(MAS_CH_INDX);
#if(CTO_COM_TYPE != Master)
    if( group_active)                                 //  going to continue bus requesting?
      PutExtEvent(MAS_CH+eAUTO_REQ);                  //  yes, then signal it
#if(MAS_SHEDULE_METOD == TICK)                        //  if no timer used
    else
      PutExtEvent(MAS_CH+eCLOCK);                     //  generate tick
#endif
#endif

  }
}
/****************************************************************************
*     Name      : CheckWaitingRoutePoll                                     *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Check if routing poll is needed, if so put event on event *
*               : queue                                                     *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void CheckWaitingRoutePoll(void)
{                                                                 // rounting poll is needed and system ready
  if (r_poll_count)
    if(!--r_poll_count) PutExtEvent(MAS_CH+eROUTING_POLL);        //  send event for routing poll
}

/****************************************************************************
*     Name      :  SendRoutedTelegram                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :                                                           *
*               :  is OK then check the request APDU. If the operation is   *
*               :  Get then return 1. If the operation is INFO then return  *
*               :  2 else return 0                                          *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR SendRoutedTelegram(void)
{
UCHAR ret_code = FALSE;

  if ( GetRoutingPDU(MAS_BUF_LEN, MAS_FRAMING_SIZE) )             // Copy PDU from routing_buf to geni_ch_transmit_buf
  {                                                               // Copy OK - proceed
    AddHeader(routing_buf[iRA], master_unit_addr, GENI_REQUEST);       // insert tgm header
    AddCRC();                                                     // insert CRC
    ret_code = TRUE;
    cur_receive_buf[PDU_BODY] = 0;                              // clear class
    cur_receive_buf[PDU_BODY+1] = 0;                            // and length of last received APDU

    if (TransmitGeniTgm(MAS_CH_INDX) == rOK)                      // and send the tgm
      SetGeniTimer(ch_const[MAS_CH_INDX].reply_timer,ROUTING_POLL_TIMEOUT,MAS_CH+eREPLY_TO,GENI_TIMEOUT_TIMER);
    else
      PutGeniEvent(MAS_CH + eREPLY_TO);                            // report timeout
  }
return ret_code;
}


/****************************************************************************
*     Name      : SendRoutingPoll                                           *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Add header, address and CRC for routing tgm, and send it *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR SendRoutingPoll(void)
{
  UCHAR ret_code = rBUSY;

  cur_transmit_buf[PDU_BODY] = REROUTE_APDU;
  cur_transmit_buf[PDU_BODY+1] = 0;
  *cur_buf_cnt = 6;
  AddHeader(routing_buf[iRA], master_unit_addr, GENI_REQUEST);   // insert tgm header
  AddCRC();                                                       // insert CRC
  ret_code = rOK;
  cur_receive_buf[PDU_BODY] = 0;                                // clear class
  cur_receive_buf[PDU_BODY+1] = 0;                              // and length of last received APDU
  routing_poll = TRUE;                                          // set routing_poll flag to TRUE
  if (TransmitGeniTgm(MAS_CH_INDX) == rOK)                        // and send the tgm
    SetGeniTimer(ch_const[MAS_CH_INDX].reply_timer,ROUTING_POLL_TIMEOUT,MAS_CH+eREPLY_TO,GENI_TIMEOUT_TIMER);
  else
    PutGeniEvent(MAS_CH + eREPLY_TO);                            // report timeout

  return ret_code;
}
#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/

