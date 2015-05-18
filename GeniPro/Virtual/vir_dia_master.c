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
/* MODULE NAME      : vir_dia_master.c                                      */
/*                                                                          */
/* FILE NAME        : vir_dia_master.c                                      */
/*                                                                          */
/* FILE DESCRIPTION : Master dialog layer                                   */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "common.h"                         // access to all common definitions
#include "profiles.h"                       // access to common profiles
#include "geni_rtos_if.h"                   // Access to OS interface
#include "geni_sys.h"                       // access to genipro system resources
#include "vir_common.h"                     // access to the common part of the virtual channel
#include "vir_tab_master.h"                 // access to master table
#include "vir_ctr_master.h"                 // access to control layer
#include "vir_drv.h"                        // access to driver layer
#include "vir_dia_master.h"                 // access to itself

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
/*                                                                          */
/****************************************************************************/

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

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

const ID_PTR *parm_pnt_tab;                      // reference to parameter pointer table
UCHAR group_active;                              // TRUE if request of group is going on else FALSE
UNIT_RECORD network_list[MAS_MAX_UNIT_COUNT];    // the list containing the units

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/

//  Multipoint master
static UCHAR unit_count;                         // number of units in networklist
static UCHAR cur_unit;                           // current unit being processed
static UCHAR cur_group;                          // index for the current selected group
static UCHAR cur_list_index;                     // index into current class tab

static UCHAR dynprio_cnt[MAS_MAX_NO_GROUPS];     // current priority counter for given group
static UCHAR wait_active_flg[MAS_MAX_NO_GROUPS]; // wait or active flag for given group
static UCHAR auto_req_sta;                       // internal error status code
static UCHAR prev_apdu_size;                     // number of bytes send in prev apdu

//  direct tgm handler
static UCHAR dir_buf_in;                         //  buf in index
static UCHAR dir_buf_out;                        //  buf out index
static UCHAR cur_resp_code;                      //  code for response action
static UCHAR cur_dir_addr;                       // current direct address

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

static UCHAR dir_req_buf[MAS_DIR_REQ_BUF_SIZE];  //  circ buf, size must be modular 2

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
void GotoNextDirRequest(void);
void ResetDirectTgmHandler(void);
void ResetDynPrio(void);
UCHAR CheckWaitingDirectTgm(void);
void StartNextTgm(void);

/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
*     Name      :  FindUnit   (API function)                                *
*               :                                                           *
*     Inputs    :  addr //address of unit to be found                       *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  none                                                     *
*     Returns   :  index of unit / NO_UNIT (not found)                      *
*               :                                                           *
*   Description :                                                           *
*               :  if unit is found index of unit in network_list or        *
*               :  is returned else NO_UNIT is                              *
*               :  returned (unit not found)                                *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR FindUnit(UCHAR addr)
{
UCHAR i, index = NO_UNIT;

  for(i=0;i<unit_count;i++)                      // go though list
  {
    if(network_list[i].unit_addr == addr)        // address match
    {
      index = i;                                 // save index
      i = unit_count;                            // stop search
    }
  }
return index;                                    // return result
}
/****************************************************************************
*     Name      :  InsertUnit   (API function)                              *
*               :                                                           *
*     Inputs    :  addr //address of new unit                               *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  network_list , unit_count                                *
*     Returns   :  TRUE / FALSE                                             *
*               :                                                           *
*   Description :                                                           *
*               :  if insert was succesful function returns TRUE else FALSE *
*               :  is returned                                              *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR InsertUnit(UCHAR addr, GENI_DEVICE_TYPE device)
{
UCHAR ret_code = FALSE;
UCHAR unit_index;

  unit_index = FindUnit(addr);                   // is unit in list ?
  if( unit_index == NO_UNIT )                    // unit is not in list
  {
    if( unit_count < MAS_MAX_UNIT_COUNT)         // space available
    {
      ret_code = TRUE;                           // succes
      network_list[unit_count].unit_addr = addr; // insert in list
      network_list[unit_count].device_type = device; // insert device_type
      network_list[unit_count].unit_err_rate = 0;// reset unit errors
      network_list[unit_count].bus_errors = 0;   // reset dialog errors
      unit_count++;                              // inc number of units
    }
  }
return ret_code;
}
/****************************************************************************
*     Name      :  RemoveUnit   (API function)                              *
*               :                                                           *
*     Inputs    :  addr //address of unit to remove                         *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  network_list, unit_count                                 *
*     Returns   :  TRUE / FALSE                                             *
*               :                                                           *
*   Description :                                                           *
*               :  if remove was succesful function returns TRUE else FALSE *
*               :  is returned                                              *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR RemoveUnit(UCHAR addr)
{
  UCHAR ret_code = FALSE;
  UCHAR unit_index;

  unit_index = FindUnit(addr);                   // is unit in list ?
  if( unit_index != NO_UNIT )                    // unit is not in list
  {
    ret_code = TRUE;                             // succes
    unit_count--;                                // dec number of units

    // move last element in list
    network_list[unit_index].unit_addr = network_list[unit_count].unit_addr;
    network_list[unit_index].device_type = network_list[unit_count].device_type;
    network_list[unit_index].unit_err_rate = network_list[unit_count].unit_err_rate;
    network_list[unit_index].bus_errors = network_list[unit_count].bus_errors;

    // reset to make debugging a lot easier
    network_list[unit_count].unit_addr = 0;
    network_list[unit_count].device_type = NO_DEVICES;
    network_list[unit_count].unit_err_rate = 0;
    network_list[unit_count].bus_errors = 0;
  }

  return ret_code;
}
/****************************************************************************
*     Name      :  ClearNetworkList   (API function)                        *
*               :                                                           *
*     Inputs    :  none                                                     *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  unit_count                                               *
*     Returns   :  nothing                                                  *
*               :                                                           *
*   Description :                                                           *
*               :  clears all units in networklist                          *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ClearNetworkList(void)
{
  unit_count = 0;
}

/****************************************************************************
*     Name      : UpdateGroup       // API                                  *
*               :                                                           *
*     Inputs    : group_no                                                  *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Force updating of the group indicated by the group_no     *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void UpdateGroup( UCHAR group_no)
{
  PutExtEvent(MAS_CH+eUPD_GRP);                  // signal that we want to change group
  ResetDynPrio();                                // Force all possible waiting to inactive
  wait_active_flg[group_no] = TRUE;              // set active for specific group
}

/****************************************************************************
*     Name      :   EnableMasterMode                                        *
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
*   Description :  Enables the master functionality                         *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void EnableMasterMode(void)
{
  GENI_USE_MASTER
  GeniResetChannel(MAS_CH_INDX);
  GENI_UNUSE_MASTER
  ResetMultiPointDia();                          // start the master mode
  ma_state = sREADY;                             // ready state

}

/****************************************************************************
*     Name      :   EnableSlaveMode                                         *
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
*   Description :  Disables the master functionality                        *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void EnableSlaveMode(void)
{
  GENI_USE_MASTER
  GeniResetChannel(MAS_CH_INDX);
  GENI_UNUSE_MASTER

}
/****************************************************************************
*     Name      :  SendDirAPDU               API                            *
*               :                                                           *
*     Inputs    :  pointer to command APDU                                  *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : TRUE or FALSE                                             *
*               :                                                           *
*   Description : Store an command APDU in the dir_req_buf. If the          *
*               : buffer is empty then initiate the transmission            *
*               : by sending an event for direct commonication              *
*               : The format referenced to by ap should be                  *
*               : ln, AH, AL, dd, dd, dd, dd, dd,                           *
*               : where ln specifies the total number of bytes to store     *
*               : The bus controller will check if any tgms is waiting      *
*               : and start the transmission                                *
*---------------------------------------------------------------------------*
* Date:     Changes:
*/
UCHAR SendDirAPDU(UCHAR resp_code, UCHAR *ap, UCHAR da)
{
  UCHAR free_bytes;
  UCHAR c;
  UCHAR i;
  UCHAR status = FALSE;
  if( resp_code <= MAS_LAST_RESP_FNC )                   // a valid response code?
  {
    if( dir_buf_in != dir_buf_out)
      free_bytes = ((dir_buf_out - dir_buf_in) & (MAS_DIR_REQ_BUF_SIZE-1));
    else
      free_bytes = (MAS_DIR_REQ_BUF_SIZE);

    c = (*ap) + 2;                                 // get the total number of APDU bytes,
                                                   // add one for resp code, and one for da
    if( c < free_bytes )
    {
      status = TRUE;
      dir_req_buf[dir_buf_in++] = resp_code;       // save resp code
      dir_buf_in &= (MAS_DIR_REQ_BUF_SIZE-1);      // circ buf
      dir_req_buf[dir_buf_in++] = da;              // save da
      dir_buf_in &= (MAS_DIR_REQ_BUF_SIZE-1);      // circ buf
      //ap++;                                      // inc to first element
      for( i = 1; i < c ; i++)
      {                                            // store APDU in buffer
        dir_req_buf[dir_buf_in++] = *(ap++);       // save
        dir_buf_in &= (MAS_DIR_REQ_BUF_SIZE-1);    // circ buf
      }
      if(!ExtEventExists(eSEND_DIR))               // if the event isn't in the queue - put one
        PutFirstExtEvent(MAS_CH+eSEND_DIR);        // generate tick
    }
  }
return status;
}

/****************************************************************************
*     Name      :  ReadyDirAPDU              API                            *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :  True if ready for next else False                        *
*               :                                                           *
*   Description :  Check if space for at least one class 3 APDU             *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*/
UCHAR ReadyDirAPDU(void)
{
  UCHAR free_bytes;

  if( dir_buf_in != dir_buf_out)
    free_bytes = ((dir_buf_out - dir_buf_in) & (MAS_DIR_REQ_BUF_SIZE-1));
  else
    free_bytes = (MAS_DIR_REQ_BUF_SIZE);
  if ( free_bytes > 5 )                          // APDU requires 3 or 4 bytes + responscode value
                                                 // + at least one free
   return TRUE;                                  // one byte response code, 3 APDU bytes
  else
   return FALSE;
}
/****************************************************************************
*     Name      : InitMultiPointDia                                         *
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
*   Description : Initialize multi point master                             *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void InitMultiPointDia(void)
{

}
/****************************************************************************
*     Name      : DisableMultiPointDia                                      *
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
*   Description : Disable multipoint master                                 *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void DisableMultiPointDia(void)
{

}
/****************************************************************************
*     Name      : ResetMultiPointDia                                        *
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
*   Description : Resets the dialog layer and the scheduling system.        *
*               : Clears the network list                                   *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ResetMultiPointDia(void)
{
                                                 // then the first group in group_table
  cur_group  = MAS_MAX_NO_GROUPS - 1;            // will be selected as the active
  cur_list_index = 0;                            // first ID code in given group list
  group_active   = FALSE;                        // No group is active
  ResetDynPrio();                                // Reset the dynamic priorities
  unit_count = 0;                                // Reset number of units in networklist
#if(MAS_SHEDULE_METOD == TIME)
  SetGeniTimer(ch_const[MAS_CH_INDX].poll_timer,ch_const[MAS_CH_INDX].poll_time,MAS_CH+eCLOCK,GENI_RELOAD_TIMER);
#endif
#if(MAS_SHEDULE_METOD == TICK)
  PutExtEvent(MAS_CH+eCLOCK);
#endif
  ResetDirectTgmHandler();                       // Reset the direct tgm handler
}
/****************************************************************************
*     Name      :  ResetDirectTgmHandler                                    *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   : dir_buf_in, dir_buf_out, cur_resp_code                    *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Resets the direct telegram functionality                  *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ResetDirectTgmHandler(void)
{
  dir_buf_in    = 0;
  dir_buf_out   = 0;
  cur_resp_code = 255;                           // stop sending messages
}
/****************************************************************************
*     Name      : CalcWaitingGroups                                         *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   : dynprio_cnt[] and wait_active_flg[]                       *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Opdate the dynprio_cnt and check for timeout              *
*               : If timeout then set the wait_active flag                  *
*               : and signal that new group is waiting                      *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void CalcWaitingGroups(void)
{
  UCHAR i;
  UCHAR n_g;

  for( i = 0, n_g = FALSE; i < MAS_MAX_NO_GROUPS; i++)     // go through all groups
  {
    if( dynprio_cnt[i] > 0 )                               // activated?
      if( dynprio_cnt[i] == 1 )                            // yes, count down and check for timeout
      {
        dynprio_cnt[i] = group_table[i].priority;          // reload priority again
        wait_active_flg[i] = TRUE;                         // set flag
        n_g = TRUE;
      }
      else
        dynprio_cnt[i]--;
  }
  if( n_g == TRUE )
    PutGeniEvent(MAS_CH+eGROUP_END);
}
/****************************************************************************
*     Name      :  FindNextWaitingGroup                                     *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Find the next active group. The first checked should be  *
*               :  cur_group + 1                                            *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void FindNextWaitingGroup(void)
{
  UCHAR i;
  UCHAR t_i;
#if(MAS_SHEDULE_METOD == TICK)
  UCHAR finished_cycle = TRUE;
#endif
  t_i = cur_group;
  for( i = 0 ; i < MAS_MAX_NO_GROUPS ; i++ )
  {
    if ( ++t_i >= MAS_MAX_NO_GROUPS )            //  go to next group
      t_i = 0;                                   //  if last select first
    if( wait_active_flg[t_i] == TRUE )           //  group active?
    {
#if(MAS_SHEDULE_METOD == TICK)
      finished_cycle = FALSE;
#endif
      PutExtEvent(MAS_CH+eAUTO_REQ);             //  then start request
      cur_group = t_i;                           //  for the group
      group_active = TRUE;                       //  indicate that group is active
      wait_active_flg[t_i] = FALSE;              //  clear the flag again
      i = MAS_MAX_NO_GROUPS;                     //  break;
      cur_list_index = 0;                        //  reset the list index pointer
      cur_unit = 0;                              //  restart the networklist
    }
  }
#if(MAS_SHEDULE_METOD == TICK)
  if(finished_cycle == TRUE) PutExtEvent(MAS_CH+eCLOCK);
#endif
}
/****************************************************************************
*     Name      :  FirstRequestInGroup                                      *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  This function finds the first waiting group and send     *
*               :  an request tgm for the first ID codes in this group      *
*               :  Scan of the waiting group is started with group 0,       *
*               :  and only one group should be active                      *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void FirstRequestInGroup(void)                             //  Action function
{
  UCHAR i;

  for( i = 0 ; i < MAS_MAX_NO_GROUPS ; i++ )
  {
    if( wait_active_flg[i] == TRUE )                       //  group active?
    {
      cur_group = i;                                       //  for the group
      group_active = TRUE;                                 //  request is going on
      wait_active_flg[i] = FALSE;                          //  clear the flag again
      cur_list_index = 0;                                  //  reset the list index pointer
      break;
    }
  }
  SendAutoRequest();                                       // send the request
}
/****************************************************************************
*     Name      :   ResetDynPrio                                            *
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
*   Description :  Reset the dynamic priorities and clear all               *
*               :  active flags                                             *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ResetDynPrio(void)                                    //  Sub function
{
  UCHAR i;

  for ( i = 0; i < MAS_MAX_NO_GROUPS; i++ )                // go through all the groups
  {
    dynprio_cnt[i] = group_table[i].priority;
    wait_active_flg[i] = FALSE;
  }
}

/****************************************************************************
*     Name      :   HandleApdu                                              *
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
*   Description :  Find the parameter reference table for given class       *
*               :  If the request response was OK and any values returned   *
*               :  then save all values                                     *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR HandleApdu(UCHAR *rx_apdu, UCHAR *tx_apdu)
{
  UCHAR c, id, i;
  UCHAR cl, clsize, j, *ptr;

  auto_req_sta = ( *tx_apdu == *rx_apdu );
  cl = *rx_apdu & APDU_CLL;
  parm_pnt_tab = mas_tab[cl];                              // get parameter reference table
  tx_apdu += 2;                                            // reference to transmit_buf ID codes
  c = *(++rx_apdu);                                        // get length and acknowledge
  rx_apdu++;                                               // reference to receive_buf data
  auto_req_sta &= ((c & OPR_ACK) == AA_OK );               // OKAY ?

  if ((auto_req_sta) && (parm_pnt_tab != 0))               // all OK?
  {                                                        // free lenght
    clsize = nof_bytes_class[cl];
    c &= APDU_LEN;                                         // go through all the receive values
    c /= clsize;
    GENI_USE_CLASS_DATA;                                   // semaphor protection
    for( i = 0 ; i < c  ; i++)                  // if anything received
    {
      id = *tx_apdu++;
      if(parm_pnt_tab[id] != (UCHAR *)NA)                  // is there variable to save data in?
      {
        ptr = parm_pnt_tab[id] + cur_unit;
        for (j = clsize; j > 0; j--)
        {
          *(ptr+j-1) = *rx_apdu++;
        }
      }
      else
        DEBUG_GENI_WARNING(GENI_WARNING_WRITE_TO_CONST);   // no variable to write data to
    }
    GENI_UNUSE_CLASS_DATA;                                 // semaphor protection ends
    c=TRUE;
  }
  else
  {
    c=FALSE;                                               // if no values received we just skip the loop
  }
return c;
}
/****************************************************************************
*     Name      :   ProcessAutoRequest                                      *
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
*   Description :  Find the parameter reference table for given class       *
*               :  If the request response was OK and any values returned   *
*               :  then save all values                                     *
*               :  This function can only handle normal GET/SET operations  *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ProcessAutoRequest(void)
{
  UCHAR c;
  UCHAR first_tx_apdu_size;
  UCHAR first_rx_apdu_size;

  c=TRUE;
  if( CheckCRC())
  {
    if( CheckTgm() )                                         // data contents of tgm OK?
    {                                                        // then process the result

      c = HandleApdu(&cur_receive_buf[PDU_BODY], &cur_transmit_buf[PDU_BODY]);

      if (c)
      {
        AutoPollReplyUserFct(cur_unit, cur_group);             // Auto request reply received notification
      }

      if( static_apdu_tgm[cur_group].size > 0 )               // static apdu is enabled 2 apdu's in tgm
      {
        first_tx_apdu_size = cur_transmit_buf[PDU_BODY+1] & APDU_LEN;
        first_rx_apdu_size = cur_receive_buf[PDU_BODY+1] & APDU_LEN;

        c = HandleApdu(&cur_receive_buf[PDU_BODY+first_rx_apdu_size+2],
                       &cur_transmit_buf[PDU_BODY+first_tx_apdu_size+2]);

      }
      cur_transmit_buf[PDU_BODY] = 0;                        // set remark for response processed
    }
    else
    {
      DEBUG_INC_DATA_ERRORS;
      c= FALSE;
    }
  }
  else
  {
    DEBUG_INC_CRC_ERRORS;
    c= FALSE;
  }
  if(c)                                                    // no errors
  {
    if(network_list[cur_unit].bus_errors >= MAS_UNIT_ERR_HYS)
      {ErrNotifyApp(cur_unit, 0);}                         // Call user function if any
    network_list[cur_unit].bus_errors = 0;                 // now, reset error counter
    GotoNextAutoRequest();                                 // just continue with next
  }
  else                                                     //
    NoReplyAutoRequest();                                  // else call error handler
}
/****************************************************************************
*     Name      :   ProcessDirRequest                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  If the tgm is OK the call the specified response         *
*               :  handler function.                                        *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ProcessDirRequest(void)
{
UCHAR unit_index;

  if( CheckCRC() )
  {
    if( CheckTgm())                            // just check CRC
    {
      // Be sure of valid resp_code
      if ( (cur_resp_code <= MAS_LAST_RESP_FNC) && (cur_resp_code > NO_RESP_FNC))
        dir_resp_table[cur_resp_code]();                     // call the response handler

      unit_index = FindUnit( cur_dir_addr );                 // find unit index
      if ( unit_index != 0xFF )                              // unit found
      {
        if(network_list[unit_index].bus_errors >= MAS_UNIT_ERR_HYS)
          {ErrNotifyApp(unit_index, 0);}                     // Call user function if any
        network_list[unit_index].bus_errors = 0;             // now, reset error counter
      }
      GotoNextDirRequest();                                  // continue to next direct tgm
      StartNextTgm();
    }
    else
    {
      DEBUG_INC_DATA_ERRORS;
      NoReplyDirRequest();                                   // data received, but no OK
    }
  }
  else
  {
    DEBUG_INC_CRC_ERRORS;
    NoReplyDirRequest();                                   // data received, but no OK
  }
}

/****************************************************************************
*     Name      : SendWaitingDirectTgm                                      *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Check the direct tgm buffer. If waiting APDU's then       *
*               : send them immediately.                                    *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR SendWaitingDirectTgm(void)
{                                                          // still APDU's left and system ready
UCHAR status = FALSE;
  if (dir_buf_in != dir_buf_out)
  {
    if (ma_state == sREADY)
    {
      SendDirRequest();                                   // send direct request
      ma_state = sDIR_REQ;
      status = TRUE;
    }
  }
return status;
}

/****************************************************************************
*     Name      : CheckWaitingDirectTgm                                     *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Check the direct tgm buffer. If waiting APDU's then       *
*               : signal it to the system                                   *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR CheckWaitingDirectTgm(void)
{                                                          // still APDU's left and system ready
UCHAR status = FALSE;
  if (dir_buf_in != dir_buf_out)
  {
    if (ma_state == sREADY)
    {
      PutExtEvent(MAS_CH+eSEND_DIR);                      // send direct request
      status = TRUE;
    }
  }
return status;
}

/****************************************************************************
*     Name      :  AddApdu                                                  *
*               :                                                           *
*     Inputs    :  cl        // class for apdu                              *
*               :  opr       // operation for the apdu                      *
*               :  size      // length of apdu                              *
*               :  apdu_ptr  // pointer to apdu data                        *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  This function adds one apdu to the transmitbuffer        *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void AddApdu(UCHAR cl, UCHAR opr, UCHAR size, UCHAR *apdu_ptr)
{
UCHAR i;

  cur_transmit_buf[(*cur_buf_cnt)++] = cl;
  if( opr == GET )                                         // if get operation
  {                                                        // save operation and length
    prev_apdu_size = size;
    cur_transmit_buf[(*cur_buf_cnt)++] = size | opr;
    // copy the ID codes for the current selected group to transmit buffer
    for( i = 0; i < size ; i++, apdu_ptr++)
      cur_transmit_buf[(*cur_buf_cnt)++] = *apdu_ptr;
  }
  else                                                     // else assume SET operation
  {
    prev_apdu_size = size*2;
    cur_transmit_buf[(*cur_buf_cnt)++] = (size*2) | opr;   // both ID and data
    parm_pnt_tab = mas_tab[cl];
    GENI_USE_CLASS_DATA;                                   // semaphor protection
    for( i = 0; i < size; i++, apdu_ptr++ )
    {
      cur_transmit_buf[(*cur_buf_cnt)++] = *apdu_ptr;      // get ID code
      cur_transmit_buf[(*cur_buf_cnt)++]
                 = *(parm_pnt_tab[*apdu_ptr] + cur_unit);  // get data
    }
    GENI_UNUSE_CLASS_DATA;                                 // semaphor protection ends
  }
}
/****************************************************************************
*     Name      :  SendAutoRequest                                          *
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
*   Description :  Transmit next auto-request for the current selected      *
*               :  group. if static_apdu_tgm is defined two apdu's will     *
*               :  be sent, if not only an single APDU is transmitted.      *
*               :  For an given group GET and SET operations can be handled.*
*               :  If SET is defined the data parameters must be defined in *
*               :  is defined the data parameters must be defined in        *
*               :  parameter pointer table, no check is made for undefined. *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SendAutoRequest(void)
{
  UCHAR cnt;
  UCHAR *id_list_p;

  if( unit_count > 0 )                                     // any units to poll
  {
    if (( network_list[cur_unit].device_type & group_table[cur_group].device_types) == NO_DEVICES)
    {
      PutGeniEvent(MAS_CH+eNEXT_UNIT);
    }
    else
    {
      *cur_buf_cnt = PDU_BODY;                             // where to store the very first byte
      if( static_apdu_tgm[cur_group].size > 0 )                       // static apdu is enabled
        AddApdu(static_apdu_tgm[cur_group].cl, static_apdu_tgm[cur_group].operation,
                static_apdu_tgm[cur_group].size, static_apdu_tgm[cur_group].grp_list_pnt);

      // set pointer to where we are in the current list
      id_list_p = group_table[cur_group].grp_list_pnt + cur_list_index;
      cnt = group_table[cur_group].size - cur_list_index;
      if ( cnt > MAS_MAX_REQ_APDU )
        cnt = MAS_MAX_REQ_APDU;                            // set cnt to max if more than MAX

      AddApdu(group_table[cur_group].cl, group_table[cur_group].operation, cnt, id_list_p);

      AddHeader(network_list[cur_unit].unit_addr, master_unit_addr, GENI_REQUEST);
      AddCRC();
      if (TransmitGeniTgm(cur_indx) == rOK)                // and send the tgm
      {
        cur_receive_buf[PDU_BODY] = 0;                     // clear class
        cur_receive_buf[PDU_BODY+1] = 0;                   // and length of last received APDU
        SetGeniTimer(ch_const[MAS_CH_INDX].reply_timer, MAS_POLL_TIMEOUT, MAS_CH+eREPLY_TO,GENI_TIMEOUT_TIMER);
      }
      else
        PutGeniEvent( MAS_CH + eREPLY_TO );
    }
  }
  else
    PutExtEvent(MAS_CH+eREPLY_TO);
}
/****************************************************************************
*     Name      :  GetAPDUSize                                              *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Get the size of the next APDU in the buffer. The          *
*               : returned size is the total number of byte, nessary to     *
*               : store the APDU                                            *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR GetAPDUSize(void)
{
  UCHAR tmp_out;
  if( dir_buf_out != dir_buf_in )                          // if anything in buffer?
  {                                                        //
    tmp_out = dir_buf_out+2;                               // skip resp_code and da
    tmp_out &= (MAS_DIR_REQ_BUF_SIZE-1);                   // wrap
    return dir_req_buf[tmp_out];                           // Total length of all APDUs
  }
  else
    return 0;
}


/****************************************************************************
*     Name      :  LoadDirApdu                                              *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Get the next APDU in dir_req_buf and store to bp          *
*               : return the total number of data stored                    *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR LoadDirApdu(UCHAR *bp)
{
  UCHAR cnt;
  UCHAR i;
  UCHAR tmp_buf_out;

  tmp_buf_out = dir_buf_out;                               // don't change dir_buf_out before going to next tgm
  cur_resp_code = dir_req_buf[tmp_buf_out++];              // get the response code for current
  tmp_buf_out &= (MAS_DIR_REQ_BUF_SIZE-1);                 // circ buf

  if( cur_resp_code > MAS_LAST_RESP_FNC )                  // no valid response code?
  {
    cnt = 0;                                               // nothing is loaded
    ResetDirectTgmHandler();                               // reset direct tgm handler system
  }                                                        // to get out of that mess!!
  else
  {
    cur_dir_addr = dir_req_buf[tmp_buf_out++];             // get destination addr
    tmp_buf_out &= (MAS_DIR_REQ_BUF_SIZE-1);               // circ buf

    cnt = GetAPDUSize();                                   // get length of all APDU's
    tmp_buf_out++;                                         // move to first ADPU
    tmp_buf_out &= (MAS_DIR_REQ_BUF_SIZE-1);               // circ buf
    for( i = 0; i < cnt; i++)                              // copy the complete
    {                                                      // APDU's
      *bp++ = dir_req_buf[tmp_buf_out++];                  //
      tmp_buf_out &= (MAS_DIR_REQ_BUF_SIZE-1);             // circ buf
    }
  }
  return cnt;                                              // return the total number of bytes stored
}
/****************************************************************************
*     Name      :   SendDirRequest                                          *
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
*   Description :   Copy one APDU pointed to by dir_apdu_pnt to             *
*               :   the transmit buffer and send the tgm                    *
*               :   If special request for changing pump address then       *
*               :   send to global address instead of specific              *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SendDirRequest(void)
{
UCHAR message_delay;

  *cur_buf_cnt = PDU_BODY;
  *cur_buf_cnt += LoadDirApdu( &cur_transmit_buf[PDU_BODY] );
  if( (cur_resp_code <= MAS_LAST_RESP_FNC)                        // a valid response code?
      && (*cur_buf_cnt != PDU_BODY))                              // and any APDU
  {
    if(cur_resp_code != NO_RESP_FNC )                             // if request
    {
      AddHeader(cur_dir_addr, master_unit_addr,GENI_REQUEST );    // add DA / SA address and SD
      //message_delay = (((cur_transmit_buf[iLN] + 4)*(1+(UINT)1.2) * ((ULONG)((10000*1000)/MAS_BAUDRATE))*2)/1000  + 50)/5 + 1;
      message_delay = MAS_DIR_REPLY_TIMEOUT;
    }
    else                                                          // if message
    {
      AddHeader(cur_dir_addr, master_unit_addr,GENI_MESSAGE );    // add DA / SA address and SD
      message_delay = (cur_transmit_buf[iLN] + 4);                        // len + header size
      message_delay = message_delay *(1+(UINT)1.2);                       // byte + interbytedelay
      message_delay = message_delay * ((UINT)(10000*6)/MAS_BAUDRATE)/6;   // baudrate factor
      message_delay = (message_delay + 3)/5 + 1;                          // ( + 3 ms turnaround)/5 + 1 to round up
    }

    AddCRC();                                                // insert CRC
    if (TransmitGeniTgm(MAS_CH_INDX) == rOK)                 // transmit the tgm
      SetGeniTimer(ch_const[MAS_CH_INDX].reply_timer,message_delay,MAS_CH+eREPLY_TO,GENI_TIMEOUT_TIMER);
    else
      PutGeniEvent( MAS_CH + eREPLY_TO );                    // make timeout at once
  }
  else
    PutGeniEvent( MAS_CH + eREPLY_TO );                      // make timeout at once
}
/****************************************************************************
*     Name      :  GotoNextAutoRequest                                      *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/

void  GotoNextAutoRequest(void)
{
UCHAR dir_req;

  cur_unit++;
  if(cur_unit < unit_count)
  {
    dir_req = CheckWaitingDirectTgm();                     //  any direct telegrams ?
    if(dir_req == FALSE) PutExtEvent(MAS_CH+eAUTO_REQ);    //  else just continue until no more units
  }
  else
  {
    // do always update the cur_list_index, will ensure that we do not stop on a single error
    cur_list_index += prev_apdu_size;
    if ((cur_list_index == 0) || (cur_list_index >= group_table[cur_group].size))
    {
      PutGeniEvent(MAS_CH+eGROUP_END);                     // finished with this group?
      group_active = FALSE;                                //  and set flag
    }
    else                                                   //
    {
      PutExtEvent(MAS_CH+eAUTO_REQ);                       //  else just continue until the hole list is
                                                           //  updated
      cur_unit = 0;                                        //  for all units
    }
  }
}
/****************************************************************************
*     Name      :  GotoNextDirRequest                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Update the dir_buf_out index and reset the retry counter  *
*               : Called when ok reply is received or request is            *
*               : retried enough                                            *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void  GotoNextDirRequest(void)
{
  UCHAR c;
  cur_ch->retry_cnt = ch_const[cur_indx].max_retries;      // reset the retry counter
  c = GetAPDUSize();                                       // get size of all APDU's
  if( c > 0 )                                              //
  {
    dir_buf_out += c;                                      // Total size of APDU's
    dir_buf_out += 3;                                      // resp_code, da addr, total APDU Len
  }
  dir_buf_out  &= (MAS_DIR_REQ_BUF_SIZE-1);                // wrap around
}                                                          // else just do nothing
/****************************************************************************
*     Name      : IncErrorCnt                                               *
*               :                                                           *
*     Inputs    : unit_addr                                                 *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Increase error count for the unit                         *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void IncErrorCnt(UCHAR unit_addr)
{
  UCHAR unit_index;

  unit_index = FindUnit( unit_addr );                    // find unit index
  if( unit_index != 0xFF )                               // unit found ?
  {
    if( network_list[unit_index].bus_errors < 254 )
      network_list[unit_index].bus_errors++;             // inc error rate for dialog
    network_list[unit_index].unit_err_rate++;            // inc error rate for unit
    if(network_list[unit_index].bus_errors == MAS_UNIT_ERR_HYS)
      {ErrNotifyApp(unit_index, 1);}                     // Call user function if any
  }
  ++all_master_errors;                                   // inc all bus errors
  DEBUG_INC_MASTER_TIMEOUT_ERRORS;
}
/****************************************************************************
*     Name      : StartNextTgm                                              *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Initiates the next tgm either direct or autopoll          *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void StartNextTgm(void)
{
  if ( !CheckWaitingDirectTgm() )                      // if no direct tgm
  {
    if( group_active )                                 // group active
      PutExtEvent(MAS_CH+eAUTO_REQ);                   // continue group
    #if(MAS_SHEDULE_METOD == TICK)
    else
      PutGeniEvent(MAS_CH+eGROUP_END);
    #endif
  }
}

/****************************************************************************
*     Name      : NoReplyDirRequest                                         *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : If we have not tried to retransmit X time then send       *
*               : the direct tgm again else just skip the control APDU      *
*               : and check if we did interrupt the bus status requesting.  *
*               : If so continue else check if more control APDU's to send  *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void NoReplyDirRequest(void)
{
  ResetGeniDriver(MAS_CH_INDX);                            // reset driver

  if ( (cur_resp_code <= MAS_LAST_RESP_FNC) && (cur_resp_code > NO_RESP_FNC))
  {
    IncErrorCnt(cur_dir_addr);                             // increase error count
    if( cur_ch->retry_cnt-- == 0 )                         // retry ?
    {
      cur_receive_buf[iLN] = 0;                            // clear length to mark failed
      dir_resp_table[cur_resp_code]();                     // call the response handler
      GotoNextDirRequest();                                // skip this dir req
      StartNextTgm();
    }
    else
      PutExtEvent(MAS_CH+eSEND_DIR);                       //  resend direct request
  }
  else                                                     // if message
  {
    cur_resp_code = 255;                                   // stop sending messages
    GotoNextDirRequest();                                  // goto next dir req
    StartNextTgm();
  }
}
/****************************************************************************
*     Name      : NoReplyAutoRequest                                        *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Called when no response or error in response. Update the  *
*               : bus_errors and just continue with next bus request if     *
*               : we are not finished                                       *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void NoReplyAutoRequest(void)
{
  ClearGeniTimer(ch_const[MAS_CH_INDX].reply_timer);       // clear the reply-timer
  if( unit_count > 0 )                                     // any units in list
    IncErrorCnt(network_list[cur_unit].unit_addr);         // increase error count on unit
  ResetGeniDriver(MAS_CH_INDX);
  GotoNextAutoRequest();
}
/****************************************************************************
*     Name      : CheckPending                                              *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Checks for pending telegrams to send                      *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void CheckPending(void)
{
  CalcWaitingGroups();
#if ( CTO_CLASS_9 == Enable )
  CheckWaitingRoutePoll();
#endif
}
/****************************************************************************
*     Name      :  CheckApduResp                                            *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Just check the response for cmd or set APDU operations   *
*               :  is OK then check the request APDU. If the operation is   *
*               :  Get then return 1. If the operation is INFO then return  *
*               :  2 else return 0                                          *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/

//void CheckApduResp(void)
//{
//  if( (cur_receive_buf[PDU_BODY+1] & OPR_ACK ) != 0 )    // just set the
//    all_master_errors++;                                        // internal error
//}                                                            // counter

