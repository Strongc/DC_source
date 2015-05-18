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
/* MODULE NAME      : vir_ctr_slave.c                                       */
/*                                                                          */
/* FILE NAME        : vir_ctr_slave.c                                       */
/*                                                                          */
/* FILE DESCRIPTION : Controller for slave applications                     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "typedef.h"            /*definements for the Grundfos common types */
#include "geni_cnf.h"                    /* Access to configuration file    */
#include "common.h"                      /* Access to common definitions    */
#include "geni_sys.h"                    /* Access to system ressources     */
#include "profiles.h"                    /* Access to channel profiles      */
#include "vir_common.h"                  /* Access to common routines       */
#include "vir_dia_slave.h"               /* Access to slave dialog layer    */
#include "vir_ctr_slave.h"               /* Access to it self               */
#include "vir_drv.h"                     /* Access to driver                */
#if(USE_VIRTUAL_SLAVES == TRUE)
#include "vir_slave_tab.h"               /* Access to vir. slave unit tables*/
#endif

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
/* This is the number of arbitration groups used to give each unit its own  */
/* delay before replying to a connection request. The delay is depending on */
/* the unit address, so that consecutive unit addresses will not get the    */
/* same delay. There will be NO_OF_ARB_GROUPS different delays from 5-10    */
/* msec to NO_OF_ARB_GROUPS * 5 msec.                                       */
/* NO_OF_ARB_GROUPS is used to find the delay. The delay is found by        */
/* finding the number unit_addr MODULO NO_OF_ARB_GROUPS. This gives a number*/
/* between 0 and NO_OF_ARB_GROUPS-1. 2 is then added to avoid the timer     */
/* from getting the values 0 and 1 (this gives a value between 2 and        */
/* NO_OF_ARB_GROUPS+1. Both 0 and 1 can lead to a real timer value of       */
/* 0 msec. This is the reason for the "-2" in the calculation of            */
/* NO_OF_ARB_GROUPS.                                                        */
/* The GENIpro system can handle a maximum of 32 bus units. This gives a    */
/* maximum of 6 units in each connection reply delay group.                 */
#define MAX_CONN_REPLY_DELAY   40        /* Maximum conn. reply delay in ms */
                                         /* + 5 to 10 ms in driver when     */
                                         /* telegram has arrived            */

#define NO_OF_ARB_GROUPS       ((MAX_CONN_REPLY_DELAY/5) - 2)

#define RETRY_TIME            2
/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/

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
/****************************************************************************
*     Name      :  InitSlave                                                *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Initiates the slave functionality                        *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void InitSlave(void)
{
  VirResetInterpreter();                    // initialize the virtual protocol interpreter
}
/****************************************************************************
*     Name      :  DisableSlave                                             *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Disables the slave functionality                         *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void DisableSlave(void)        // OBS!
{
}

/****************************************************************************
*     Name      :  ResetSlave                                               *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Resets the slave functionality                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ResetSlave(void)
{
  VirResetInterpreter();                    // initialize the virtual protocol interpreter
}

/****************************************************************************
*     Name      :  SlaveControlHandler                                      *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  The SlaveControlHandler is called when an new event is *
*               :  generated by the Genibus channel. The BusControlHandler  *
*               :  then process the event by calling the related action-    *
*               :  functions                                                *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SlaveControlHandler(UCHAR event)
{
  switch ( event )
  {
    case eNEW_TGM :                         // New tgm arrived,

    ProcesGeniTgm();                        // Then process it, but no
    break;                                  // reply transmision is started

    case eSEND_TGM :                        // Timeout from BUS_REPLY_TIMER
    SendSlaveReply();
    break;

    case eCONNECT_TIMEOUT :                 // Connection request timeout
    SlaveConnectTmOut();
    break;

    case eREPLY_TO:                        // To late to reply
    SlaveDontReply();
    break;

    #if(USE_VIRTUAL_SLAVES == TRUE)
    case eVIR_CONNECT_TIMEOUT :             // Connection request timeout for vir. slaves
    VirSlaveConnectTmOut();
    break;
    #endif
  }
}
/****************************************************************************
*     Name      :  ProcesGeniTgm                                            *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Handle new geni tgms from either RS485 or RS232          *
*               :  channel.                                                 *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ProcesGeniTgm(void)
{
  UCHAR reply = FALSE;
  UINT reply_timeout;
  UCHAR cur_addr;
  ch_interface[cur_indx].ReceiveUserFct();                  // indecate that we receieve something
  if (((cur_ch->flags & DONT_REPLY_MSK) == 0))              // did we get a reply timeout ?
  {
    if( CheckHeader(slave_unit_addr, &reply))               // Tgm to us?
    {
      if( CheckCRC())                                        // Check CRC
      {
        cur_addr = slave_unit_addr;                      // set the default reply address
        ch_interface[cur_indx].PreUserFct();                 // Call user specified function if any

        #if(USE_VIRTUAL_SLAVES == TRUE)
        if ((cur_unit != NO_UNIT))                           // Are the current unit a virtual slave
        {
          cur_addr = vir_slave_list[cur_unit].unit_addr;     // set the virtual slave address
          pre_tab_ptr = &vir_slave_pre_tab[cur_unit][0];     // change table pointer to current virtual slave
        }
        #endif

        GENI_USE_CLASS_DATA;                                 // semaphor protection
        VirInterprete(*cur_buf_cnt, HEADER_LEN);             // CRC ok, inteprete the tgm
        GENI_UNUSE_CLASS_DATA;

        #if(USE_VIRTUAL_SLAVES == TRUE)
        pre_tab_ptr = pre_tab;                               // change table pointer back to normal
        #endif
        if( reply )                                          // going to send reply?
        {
          cur_ch->retry_cnt = ch_const[cur_indx].max_retries;// reset retry counter
          AddHeader(cur_receive_buf[iSA], cur_addr, GENI_REPLY);  // build reply header
          AddCRC();                                          // calculate and insert CRC
          if( cur_receive_buf[iDA] == CONNECTION )           // connection request response?
            reply_timeout = (cur_addr % NO_OF_ARB_GROUPS) + ch_const[cur_indx].inter_data_time + GENI_REPLY_DELAY;
          else // Use the default inter data time we know that at least 5 ms - one tick time has gone already
            reply_timeout = ch_const[cur_indx].inter_data_time + GENI_REPLY_DELAY;

          if ( reply_timeout > (ch_const[cur_indx].reply_time - 3))  // reply time must not be too long
            reply_timeout = ch_const[cur_indx].reply_time - 3;
#if (CTO_BUS_TYPE == Slave)
          if ((cur_indx == BUS_CH_INDX) && (set_min_reply_delay))
          {
            ClearGeniTimer(ch_const[BUS_CH_INDX].reply_timer);             // clear the reply-timer
            reply_timeout += set_min_reply_delay << 1; /* set_min_reply_delay is in 10 ms unit */
          }
#endif
          SetGeniTimer(ch_const[cur_indx].inter_data_timer,reply_timeout,(cur_channel+eSEND_TGM), GENI_TIMEOUT_TIMER);

        }
        ch_interface[cur_indx].PostUserFct();
      }
      else
      {
        DEBUG_INC_CRC_ERRORS
        reply = FALSE;
      }
    }
  }
  if(!reply )
  {
    ClearGeniTimer(ch_const[cur_indx].reply_timer);        // stop reply timeout timer, because no reply.
    ResetGeniDriver(cur_indx);
  }
  cur_ch->flags &= ~DONT_REPLY_MSK;                        // clear the flag
}
/****************************************************************************
*     Name      :   SendSlaveReply                                          *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  send slave reply when master is acting  as slave         *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SendSlaveReply(void)                                  //  action function
{
  // if dont_reply flag cleared and retry count > 0
  if (((cur_ch->flags & DONT_REPLY_MSK) == 0))
  {
    if (TransmitGeniTgm(cur_indx) == rOK)                  // transmit the tgm
      ClearGeniTimer(ch_const[cur_indx].reply_timer);      // we have send a reply - so stop reply timeout
    else
    {
      if ( cur_ch->retry_cnt-- != 0 )
        SetGeniTimer(ch_const[cur_indx].inter_data_timer,RETRY_TIME,(cur_channel+eSEND_TGM), GENI_TIMEOUT_TIMER); // make a retry
      else
      { // we gave up
        ClearGeniTimer(ch_const[cur_indx].reply_timer);    // be sure that reply timer is stopped
        ResetGeniDriver(cur_indx);
      }
    }
  }
  else
  { // reply timeout
    ResetGeniDriver(cur_indx);
  }

  cur_ch->flags &= ~DONT_REPLY_MSK;                        // we are allowed to answer next time -  clear the flag
  #if(USE_VIRTUAL_SLAVES == TRUE)
    if ((VIR_SLAVE_CHANNEL == cur_channel) && (cur_unit != NO_UNIT) && (vir_slave_list[cur_unit].polled))
    {
      vir_slave_list[cur_unit].polled = FALSE;             // clear the flag - the poll has been noticed
      vir_slave_list[cur_unit].connect_addr
                    = vir_slave_list[cur_unit].unit_addr;  // no more response to connect addr
      vir_slave_list[cur_unit].connect_timer
                    = VIR_SLAVE_COUNT_VAL;                 // reset timeout value
      vir_slave_list[cur_unit].timer_enabled = TRUE;       // Start timer
     }
  #endif
  if ( (cur_ch->flags & POLLED_FLG_MSK) != 0 )             // flag set ? - unit is being polled
  {
    cur_ch->flags &= ~POLLED_FLG_MSK;                      // clear the flag - the poll has been noticed
    cur_ch->connect_addr = slave_unit_addr;            // and no more response on connection address
    SetGeniTimer( ch_const[cur_indx].connect_timer,ch_const[cur_indx].connect_time,(cur_channel+eCONNECT_TIMEOUT),GENI_TIMEOUT_TIMER);
  }
}

/****************************************************************************
*     Name      :   SlaveConnectTmOut                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  connection timeout in slave mode                         *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SlaveConnectTmOut(void)                               //  action function
{
  if( ConReply_ON )
  {
    cur_ch->connect_addr  = CONNECTION;                    // and open the door again
#if (CTO_BUS_TYPE == Slave)
    if (cur_indx == BUS_CH_INDX)
    {
      GeniPresetActBusBaudrate();
    }
#endif
  }
  else
    cur_ch->connect_addr = slave_unit_addr;

}

/****************************************************************************
*     Name      :   SlaveDontReply                                          *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  We are are not allowed to reply anymore                  *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SlaveDontReply(void)
{
  DEBUG_INC_SLAVE_TIMEOUT_ERRORS;
  cur_ch->flags |= DONT_REPLY_MSK;                         // set the flag
}
/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/


