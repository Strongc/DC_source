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
/* MODULE NAME      : vir_dia_slave.c                                       */
/*                                                                          */
/* FILE NAME        : vir_dia_slave.c                                       */
/*                                                                          */
/* FILE DESCRIPTION : Dialog file slave applications                        */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "geni_cnf.h"                                      // Access to user configurations
#include "common.h"                                        // Access to common definitions
#include "profiles.h"                                      // Access to genipro profiles
#include "geni_sys.h"                                      // Aceess to Geni system
#include "vir_common.h"                                    // Access to common routines
#include "vir_ctr_slave.h"                                 // Access to slave controller
#include "vir_dia_slave.h"                                 // Access to itself
#include "Geni_rtos_if.h"                                  // Access to rtos definitions

#if(USE_VIRTUAL_SLAVES == TRUE)
#include "vir_slave_tab.h"                                 // Access to vir. slave unit tables
#endif

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
// Default Data Acces for Class 0 to 7
#define ACCESS_CLASS_0_TO_7   (ACCESS_CLASS_7 << 14) + (ACCESS_CLASS_6 << 12) + (ACCESS_CLASS_5 << 10) + \
                              (ACCESS_CLASS_4 << 8)  + (ACCESS_CLASS_3 << 6)  + (ACCESS_CLASS_2 << 4) + \
                              (ACCESS_CLASS_1 << 2)  +  ACCESS_CLASS_0
// Default Data Acces for Class 8 to 15
#define ACCESS_CLASS_8_TO_15  (ACCESS_CLASS_15 << 14)  + (ACCESS_CLASS_14 << 12)  + \
                              (ACCESS_CLASS_13 << 10) + (ACCESS_CLASS_12 << 8)  + (ACCESS_CLASS_11 << 6)  + \
                              (ACCESS_CLASS_10 << 4)  + (ACCESS_CLASS_9 << 2)   +  ACCESS_CLASS_8

// Default Data Acces for Class 16
#define ACCESS_CLASS_16_TO_X   (ACCESS_CLASS_16)

#if((CTO_CLASS_9 == Enable) && (VIR_CLASS_9 == Enable))
  // Default Data Acces for Virtual Class 2 to 7
  #define ACCESS_VIR_CLASS_0_TO_7 (ACCESS_VIR_CLASS_7 << 14) + (ACCESS_VIR_CLASS_6 << 12) + (ACCESS_VIR_CLASS_5 << 10) + \
                                  (ACCESS_VIR_CLASS_4 << 8)  + (ACCESS_VIR_CLASS_3 << 6)  + (ACCESS_VIR_CLASS_2 << 4) + \
                                  (ACCESS_VIR_CLASS_1 << 2)  +  ACCESS_VIR_CLASS_0

  // Default Data Acces for Virtual Class 8 to 13  except class 9
  #define ACCESS_VIR_CLASS_8_TO_15  (ACCESS_VIR_CLASS_15 << 14)  + (ACCESS_VIR_CLASS_14 << 12)  + \
                                    (ACCESS_VIR_CLASS_13 << 10) + (ACCESS_VIR_CLASS_12 << 8)  + (ACCESS_VIR_CLASS_11 << 6)  + \
                                    (ACCESS_VIR_CLASS_10 << 4)  +  ACCESS_VIR_CLASS_8

  // Default Data Acces for Class 16
  #define ACCESS_VIR_CLASS_16_TO_X       (ACCESS_VIR_CLASS_16)

#endif
/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/

#if ( CTO_BUF_OPT == Enable )
 const UINT buf_opt_ctr = Buf_opt_ctr;                     // how to access application buffers
#endif

#if( CTO_CLASS_9 == Enable )
//   conversion from channel specifier in APDU class 9 to Genipro logic channel
//                                                         //   channel specifier:
const SINT channel_tab[] = {  IR_CH,               //    0
                              BUS_CH,              //    1
                              COM_CH,              //    2
                              PLM_CH,              //    3
                              RS232_CH,            //    4
                              0XFF };              //    5  Not used old Modem channel
#endif

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif
const ID_PTR *data_item_ptab;                              // pointer to class table
const PROTAB *pre_tab_ptr;                                 // pointer to pre_tab
const INFO_DATA  *common_info_tab_p;                       // pointer to info_tab
const INFO_DATA_PTR *common_ptr_tab_p;

#if (USE_VIRTUAL_SLAVES == TRUE)
  UCHAR cur_unit;                                          // current unit being processed
  SUBSLAVE_UNIT_RECORD vir_slave_list[MAX_VIR_SLAVE_COUNT];// unit list
#endif

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/
//     Temporary variables, 20 bytes
void  *tab_buf;                                            // pointer to info table or application receive buffer
UINT  operation_acc_ctr;                                   // -7-6 -5-4 -3-2 -1-0 Class  // SGSG SGSG SGSG SGSG operation
UINT  operation_acc_ctr_high;                              // 15-14-13-12-11-10-9-8 Class// SGSG SGSG SGSG SGSG operation
UINT  operation_acc_ctr_high_h;                            // 16 Class      // 0000 0000 0000 00SG operation
UCHAR ack;                                                 // new interpreter
UCHAR a_i;                                                 // acknowledge index
UCHAR r_ln;                                                // lenght of received APDU
UCHAR t_ln;                                                // lenght of transmit APDU
UCHAR rec_indx;                                            // index to receive_buf
UCHAR a_hd;                                                // APDU header     -
UCHAR id_code;                                             // received ID code
UCHAR max_id_code;                                         // max ID code for given class
UCHAR buf_len;                                             // size of application buffer
UCHAR transmit_cnt;                                        // numbers of bytes to transmit

#if ( CTO_BUF_OPT == Enable )
 UCHAR acc_code;
#endif

#if (USE_VIRTUAL_SLAVES == TRUE)
  static UCHAR unit_count;                                 // number of units in networklist
#endif

#if (CTO_CLASS_8 == Enable)
  static UCHAR *dump_addr;                                 // pointer to dump address
  static UCHAR dump_ln;                                    // Dump length
  static UCHAR dump_status;                                // Dump status
#endif

#if (CTO_VIR_CLASS_8 == Enable)
  static UCHAR *virdump_addr;                              // pointer to dump address
  static UCHAR virdump_ln;                                 // Dump length
  static UCHAR virdump_status;                             // Dump status
#endif


#if ( CTO_CLASS_9 == Enable )
  BIT vir_cl_9_flg;                                        // Set when using vir cl. 9
#endif

#if (CTO_CLASS_10 == Enable)
  static UCHAR object_status;                              // Object status
#endif
#if (CTO_VIR_CLASS_10 == Enable)
  static UCHAR virobject_status;                           // Object status
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

//     presentation table buffers
#if ( CTO_CLASS_10 == Enable )                             // buffer for
//   Format for object_buf:
//
//   0       length of object
//   1       objectdata
//   2       objectdata..
  UCHAR object_buf[OBJECT_DF_buf_len+1];                   // object PDU's
#endif

#if ( CONF_BUF_LEN != 0 )
  BUFFER conf_buf[CONF_BUF_LEN+2];                         // configuration buffer
#endif

#if ( CMD_BUF_LEN != 0 )
  BUFFER cmd_buf[CMD_BUF_LEN+2];                           // command buffer
#endif

#if ( REF_BUF_LEN != 0 )
  BUFFER ref_buf[REF_BUF_LEN+2];                           // reference buffer
#endif

#if ( ASCII_BUF_LEN != 0 )
  BUFFER ascii_buf[ASCII_BUF_LEN+2];                       // text buffer
#endif

#if ( CONF16_BUF_LEN != 0 )
  BUFFER16 conf16_buf[CONF16_BUF_LEN+2];                   // configuration buffer
#endif

#if ( REF16_BUF_LEN != 0 )
  BUFFER16 ref16_buf[REF16_BUF_LEN+2];                     // reference buffer
#endif

#if ( CONF32_BUF_LEN != 0 )
  BUFFER32 conf32_buf[CONF32_BUF_LEN+2];                   // configuration buffer
#endif

#if ( REF32_BUF_LEN != 0 )
  BUFFER32 ref32_buf[REF32_BUF_LEN+2];                     // reference buffer
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif



/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
void VirResetInterpreter(void);
void VirInterprete(UCHAR receive_cnt, UCHAR i_pdu_body);
void VirApduError(UCHAR d_i,UCHAR err);
void VirInfoData(void);
void GENISetData(UCHAR geni_class);
void VirSetCmds(void);
void VirSetASCII(void);
//static UCHAR SaveToASCIIParam(UCHAR* param_p);
#if (CTO_BUF_OPT == Enable)
static UCHAR SaveToASCIIParam(UCHAR* param_p);
static void SaveToASCIIBuffer(UCHAR id_code);
static void SaveToASCIIAddress(UCHAR *param_p);
#endif
void VirSetData(void);
void VirSetData16(void);
void VirSetData32(void);
void GENIGetData(UCHAR geni_class);
void VirGetASCII(void);
void VirGetData(void);
void VirGetData16(void);
void VirGetData32(void);
/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
#if(USE_VIRTUAL_SLAVES == TRUE)
/****************************************************************************
*     Name      :   VirSlaveConnectTmOut                                    *
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
*   Description :  connection timeout for virtual slaves, when one of the   *
*               :  slave timers reaches 0 it has lost connection.           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirSlaveConnectTmOut(void)
{
  UCHAR i;

  for (i = 0; i<unit_count; i++ )                          // check all units
  {
    if (vir_slave_list[i].timer_enabled == TRUE)
    {
      vir_slave_list[i].connect_timer--;
      if(vir_slave_list[i].connect_timer == 0)             // if timer is running count it down
      {
        vir_slave_list[i].timer_enabled = FALSE;
        if( ConReply_ON )
          vir_slave_list[i].connect_addr  = CONNECTION;    // and open the door again
        else
          vir_slave_list[i].connect_addr = vir_slave_list[i].unit_addr;
      }
    }
  }
}
/****************************************************************************
*     Name      :  VirSlavePollUpdate                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  vir_slave_list[].polled                                  *
*     Returns   :  none                                                     *
*               :                                                           *
*   Description :  Updates the poll flags for all virtual slaves            *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void VirSlavePollUpdate(void)
{
  UCHAR i;

  for (i=0; i < unit_count; i++)                           // reset all poll flags
    vir_slave_list[i].polled = FALSE;
  if(cur_unit != NO_UNIT)                                  // tgm was to a virtual slave unit_addr
    vir_slave_list[cur_unit].polled = TRUE;                // Set the polled flag
}
/****************************************************************************
*     Name      :  FindNext2Connect                                         *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  none                                                     *
*     Returns   :  unit index of the one connected                          *
*               :                                                           *
*   Description :  Finds if any the next unit to connect                    *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR FindNext2Connect(void)
{
  UCHAR i, returnval = NO_UNIT;

  for (i=0; i < unit_count; i++)                           // reset all poll flags
  {
    // check if the current can reply to a connection request
    if(vir_slave_list[i].connect_addr == CONNECTION)
    {
      returnval = i;                                       // return the first found
      i = unit_count;                                      // stop looking anymore
    }
  }
  return returnval;
}
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
*               :  if unit is found index of unit in vir_slave_list is      *
*               :  returned else 0xFF is                                    *
*               :  returned (NO_UNIT)                                       *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
UCHAR FindSlave(UCHAR addr)
{
UCHAR i, index = NO_UNIT;

  for(i=0;i<unit_count;i++)                                // go though list
  {
    if(vir_slave_list[i].unit_addr == addr)                // address match
    {
      index = i;                                           // save index
      i = unit_count;                                      // stop search
    }
  }
return index;                                              // return result
}
/****************************************************************************
*     Name      :  InsertSlave   (API function)                             *
*               :                                                           *
*     Inputs    :  addr //address of new unit                               *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  vir_slave_list, unit_count                               *
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
UCHAR InsertSlave(UCHAR addr)
{
UCHAR ret_code = FALSE;
UCHAR unit_index;

  unit_index = FindSlave(addr);                            // is unit in list ?
  if( unit_index == NO_UNIT )                              // unit is not in list
  {
    if( unit_count < MAX_VIR_SLAVE_COUNT)                  // space available
    {
      ret_code = TRUE;                                     // succes
      vir_slave_list[unit_count].unit_addr = addr;         // insert in list
      vir_slave_list[unit_count].polled = FALSE;           // reset polled flag
      vir_slave_list[unit_count].connect_timer = 0;
      vir_slave_list[unit_count].timer_enabled = FALSE;
      if( ConReply_ON )
        vir_slave_list[unit_count].connect_addr  = CONNECTION;
      else
        vir_slave_list[unit_count].connect_addr = addr;

      unit_count++;                                        // inc number of units
      if ( unit_count == 1)                                // if first unit added to list start the timer
        SetGeniTimer( ch_const[VIR_SLAVE_CH_INDX].vir_connect_timer,ch_const[VIR_SLAVE_CH_INDX].vir_connect_time,(VIR_SLAVE_CHANNEL+eVIR_CONNECT_TIMEOUT),GENI_RELOAD_TIMER);
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
*     Updates   :  vir_slave_list, unit_count                               *
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
UCHAR RemoveSlave(UCHAR addr)
{
UCHAR ret_code = FALSE;
UCHAR unit_index;

  unit_index = FindSlave(addr);                            // is unit in list ?
  if( unit_index != NO_UNIT )                              // unit is not in list
  {
    ret_code = TRUE;                                       // succes
    unit_count--;                                          // dec number of units

    vir_slave_list[unit_index].unit_addr
            = vir_slave_list[unit_count].unit_addr;        // move last element in list
    vir_slave_list[unit_index].polled
            = vir_slave_list[unit_count].polled;           // reset polled flag
    vir_slave_list[unit_index].connect_timer
            = vir_slave_list[unit_count].connect_timer;
    vir_slave_list[unit_index].timer_enabled
            = vir_slave_list[unit_count].timer_enabled;
    if( ConReply_ON )
      vir_slave_list[unit_index].connect_addr  = CONNECTION;
    else
      vir_slave_list[unit_index].connect_addr = vir_slave_list[unit_count].unit_addr;

    if ( unit_count == 0)                                  // if last unit are removed from list stop the timer
      ClearGeniTimer(ch_const[VIR_SLAVE_CH_INDX].vir_connect_timer);
  }
return ret_code;
}
/****************************************************************************
*     Name      :  ClearSlaveList     (API function)                        *
*               :                                                           *
*     Inputs    :  none                                                     *
*               :                                                           *
*               :                                                           *
*     Outputs   :  none                                                     *
*     Updates   :  unit_count                                               *
*     Returns   :  nothing                                                  *
*               :                                                           *
*   Description :                                                           *
*               :  clears all units in vir_slave_list                       *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ClearSlaveList(void)
{
  unit_count = 0;
  ClearGeniTimer(ch_const[VIR_SLAVE_CH_INDX].vir_connect_timer);// stop timer
}
#endif
/****************************************************************************
*    Name       :  ClassAcc  // API                                         *
*               :                                                           *
*    Inputs     :  cll      (CMD_APDU, MEAS_APDU, CONF_APDU,                *
*               :            REF_APDU)                                      *
*               :  opr      (GET_ACC, SET_ACC)                              *
*               :  en_dis   (Enable, Disable )                              *
*               :                                                           *
*    Outputs    :                                                           *
*    Updates    :  opr_acc_ctr                                              *
*    Returns    :  None                                                     *
*               :                                                           *
*    Description:  Enable or disable of SET and GET operations              *
*               :  for measurement, command, configuration and              *
*               :  reference APDU classes. AA_OK_init()                     *
*               :  disabel operations for all classes.                      *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void ClassAcc(UCHAR cl, UCHAR opr, UCHAR en_dis)
{
  UINT i;
  i = opr << ( cl << 1);                                   // two flags for each class
  if( en_dis != Disable )
    operation_acc_ctr |= i;
  else
    operation_acc_ctr &= (0xffff-i);
}
#if ((CTO_CLASS_8 == Enable) || (CTO_VIR_CLASS_8 == Enable))
/****************************************************************************
*     Name      :  SetDumpStatus                                            *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : API function the application uses this function to        *
*               : report back status of dump dialog                         *
*               : value of                                                  *
*               : 3: dialog has completed succesfully                       *
*               : 4: dialog has failed                                      *
*               : 5: operation is illegal                                   *
*               : default: geni is ready                                    *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/

void SetDumpStatus(UCHAR status)
{
  if (status <= sDUMP_ILLEGAL)
  {
    dump_status = status;
  }
  else
  {
    dump_status = sDUMP_ILLEGAL;
  }
}

/****************************************************************************
*     Name      :  VirSetDumpStatus                                         *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : API function the application uses this function to        *
*               : report back status of dump dialog                         *
*               : value of                                                  *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/

void VirSetDumpStatus(UCHAR status)
{
  if (status <= sDUMP_ILLEGAL)
  {
    virdump_status = status;
  }
  else
  {
    virdump_status = sDUMP_ILLEGAL;
  }
}


/****************************************************************************
*     Name      :  VirGetDumpApdu                                           *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Read data items from memory and build an response APDU.   *
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

void VirGetDumpApdu(void)
{
  UCHAR i;
  UCHAR *d_status;
  UCHAR *d_ln;
  UCHAR *tmp;
  UCHAR **d_addr;


#if(VIR_CLASS_9 == Enable)
  if (vir_cl_9_flg == FALSE)
  {
    d_status = &dump_status;
    d_ln = &dump_ln;
    d_addr = &dump_addr;
  }
  else
  {
    d_status = &virdump_status;
    d_ln = &virdump_ln;
    d_addr = &virdump_addr;
  }
#else
  d_status = &dump_status;
  d_ln = &dump_ln;
  d_addr = &dump_addr;
#endif

  if (r_ln > 0)                                                    // receive new request?
  {
    if ( *d_status != sDUMP_BUSY )                               // not busy?
    {
      *d_status = sDUMP_LOADED;
      #if(VIR_CLASS_9 == Enable)
        if(vir_cl_9_flg == FALSE)
          GetDump_fct(&cur_receive_buf[rec_indx], d_addr);        // call app.
        else
          VirGetDump_fct(&cur_receive_buf[rec_indx], d_addr);    // call app.
      #else
        GetDump_fct(&cur_receive_buf[rec_indx], d_addr);         // call app.
      #endif
      rec_indx += 5;
      *d_ln = cur_receive_buf[rec_indx++];
      if( *d_ln > 62 )                                           // more than maximum length?
        *d_ln = 62;                                              // maximum APDU length -3
      switch(*d_status)
      {
        case sDUMP_PROCESSED:                                      // dialog is processed and data is ready
          cur_transmit_buf[transmit_cnt++] = sDUMP_READY;          // insert status
          t_ln++;
          tmp = *d_addr;
          for( i = 0; i < *d_ln;i++)                             // get data from dump memory area
          {
            cur_transmit_buf[transmit_cnt++] = *(tmp+i);
            t_ln++;
          }
          break;
        case sDUMP_LOADED:
          VirApduError(1,A_ID_ERR);    // User function has not modified *d_status
          break;
        default:
          cur_transmit_buf[transmit_cnt++] = *d_status;          // reply status to master
          t_ln = 1;                                              // set length
        break ;
      }
      if (*d_status != sDUMP_BUSY)
        *d_status = sDUMP_READY;
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = sDUMP_BUSY;               // reply busy to master
      t_ln = 1;                                                    // set length
      rec_indx += r_ln;                                            // inc index to next APDU
    }
  }
  else                                                             // receive request for reply (r_ln =0)?
  {
    if( *d_status == sDUMP_PROCESSED )                             // data is ready
    {
      cur_transmit_buf[transmit_cnt++] = sDUMP_READY;              // insert status
      t_ln++;
      tmp = *d_addr;
      for( i = 0; i < *d_ln;i++)                                 // get data from dump memory area
      {
        cur_transmit_buf[transmit_cnt++] = *(tmp+i);
        t_ln++;
      }

    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = *d_status;              // respond status to master
      t_ln = 1;                                                  // set apdu length
    }
  }
}
/****************************************************************************
*     Name      :  VirSetDumpApdu                                           *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Write data items to memory and                            *
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

void VirSetDumpApdu(void)
{
  UCHAR *d_status;

#if(VIR_CLASS_9 == Enable)
  if (vir_cl_9_flg == FALSE)
  {
    d_status = &dump_status;
  }
  else
  {
    d_status = &virdump_status;
  }
#else
  d_status = &dump_status;
#endif
  if (r_ln > 0)                                            // receive new request?
  {
    if (*d_status != sDUMP_BUSY)                          // not busy?
    {
      *d_status = sDUMP_LOADED;
      #if(VIR_CLASS_9 == Enable)
        if(vir_cl_9_flg == FALSE)
          SetDump_fct(&cur_receive_buf[rec_indx]);         // call app.
        else
          VirSetDump_fct(&cur_receive_buf[rec_indx]);      // call app.
      #else
        SetDump_fct(&cur_receive_buf[rec_indx]);           // call app.
      #endif
      rec_indx += 5;                                       // move to length specifier
      rec_indx += cur_receive_buf[rec_indx] + 1;           // move to end of this APDU

      switch(*d_status)
      {
        case sDUMP_PROCESSED:                              // dialog is processed and data is ready
          cur_transmit_buf[transmit_cnt++] = sDUMP_READY;  // insert status
          t_ln = 1;
          break;
        case sDUMP_LOADED:
          VirApduError(1,A_ID_ERR);    // User function has not modified *obj_status
          break;
        default:
          cur_transmit_buf[transmit_cnt++] = *d_status;    // reply status to master
          t_ln = 1;                                        // set length
          break;
      }
      if (*d_status != sDUMP_BUSY)
        *d_status = sDUMP_READY;
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = sDUMP_BUSY;       // reply busy to master
      t_ln = 1;                                            // set length
      rec_indx += r_ln;                                    // inc index to next APDU
    }
  }
  else                                                     // receive request for reply (r_ln =0)?
  {
    if( *d_status == sDUMP_PROCESSED )                     // data is ready
    {
      cur_transmit_buf[transmit_cnt++] = sDUMP_READY;      // insert status
      *d_status = sDUMP_READY;
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = *d_status;      // respond status to master
    }
    t_ln = 1;                                              // set apdu length
  }
}
#endif //CTO_CLASS_8 code end

#if( CTO_CLASS_9 == Enable )
/******************************************************************************
*    Name       : RerouteError                                                *
*               :                                                             *
*    Inputs     : none                                                        *
*               :                                                             *
*    Outputs    :                                                             *
*    Updates    :                                                             *
*    Returns    :                                                             *
*               :                                                             *
*    Description: Generate an Error APDU. Note the APDU - acknowled field     *
*               : is leaved as ACK_OK                                         *
*               :                                                             *
*/
void RerouteError(void)
{
  cur_transmit_buf[transmit_cnt++] = routing_status;       // return status on routing
  rec_indx += r_ln;                                        // inc to next APDU if more
  t_ln = 1;
}
/****************************************************************************
*     Name      :  VirProcessRerouteApdu                                    *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Process the reroute APDU, ( embedded PDU's )              *
*               : Handle to cases:                                          *
*               :                                                           *
*               : 1. New reroute APDU received                              *
*               : 2. Request on current reroute dialog and return of        *
*               :    response                                               *
*               :                                                           *
*               : 1: Length of class 9 APDU > new reroute tgm               *
*               :     - check if system is ready for new reroute session    *
*               :       if so place the class 9 APDU content in reroute-buf *
*               :          set routing status and generate class9 APDU resp *
*               :          update the interpreter parameters                *
*               :          generate event to system ( channel, event )      *
*               :       else                                                *
*               :          generate class9 APDU resp ( error )              *
*               :                                                           *
*               : 2: Check if reroute session is finished with success      *
*               :       if so place the response in class9 resp. APDU       *
*               :       else                                                *
*               :          generate class9 APDU resp ( error )              *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirProcessRerouteApdu(void)           //
{
  UCHAR i,c;
  UCHAR cnt;
#if(VIR_CLASS_9 == Enable)
  UCHAR temp_rec_indx;
  UCHAR temp_ack;
  UCHAR vir_unit_addr;
  UCHAR temp_a_i;
  UINT temp_acc_low, temp_acc_high, temp_acc_high_h;
  vir_cl_9_flg  = FALSE;
#endif

  if ( r_ln > 0 )                                          // receiving new tgm for reroute?
  {
    if( routing_status != sROUTING_BUSY )                  // ready to initiate new routing dialog?
    {
      routing_buf[0] = a_hd;                               // save
      routing_buf[1] = r_ln;                               // and length
      cnt = r_ln + 2;                                      // compensate
      for( i=2; i < cnt; i++ )                             // save APDU content
        routing_buf[i] = cur_receive_buf[rec_indx++];

      i = routing_buf[3] & 0x0F;                           // get channel-specifier

#if(VIR_CLASS_9 == Enable)
      if(i == VIR_CLASS_9_SPECIFIER)
      {
        vir_cl_9_pre_fct;                                  // call user pre function
        cur_receive_buf[1] = routing_buf[1];
        cur_receive_buf[2] = routing_buf[2];
        vir_unit_addr = routing_buf[2];                    // save virtual unit address

        for(i = 4;i<(routing_buf[1]+2);i++)
           cur_receive_buf[i] = routing_buf[i];
        *cur_buf_cnt -= 4;
        temp_rec_indx = rec_indx;                          // save old value
        temp_ack = ack;                                    // save old value
        temp_a_i = a_i;                                    // save old value
        temp_acc_low =  operation_acc_ctr;                 // save old value
        temp_acc_high =  operation_acc_ctr_high;           // save old value
        temp_acc_high_h = operation_acc_ctr_high_h;         // save old value
        operation_acc_ctr = ACCESS_VIR_CLASS_0_TO_7;       // -7-6 -5-4 -3-2 -1-0 Class
        operation_acc_ctr_high = ACCESS_VIR_CLASS_8_TO_15; // 15-14-13-12-11-10-9-8 Class
        operation_acc_ctr_high_h = ACCESS_VIR_CLASS_16_TO_X;    // 16 class
        pre_tab_ptr = vir_pre_tab;                         // shift to virtual pointer
        common_info_tab_p = vir_common_info_tab;           // shift to virtual pointer
        common_ptr_tab_p = vir_common_ptr_tab;             // shift to virtual pointer
        vir_cl_9_flg = TRUE;                               // shift to virtual function
        VirInterprete(routing_buf[1]+2, HEADER_LEN);       //interprete again
        vir_cl_9_flg = FALSE;                              // shift to normal function
        pre_tab_ptr = pre_tab;                             // shift to normal pointer
        common_info_tab_p = common_info_tab;               // shift to normal pointer
        common_ptr_tab_p = common_ptr_tab;                 // shift to normal pointer
        operation_acc_ctr = temp_acc_low;                  // restore old value
        operation_acc_ctr_high = temp_acc_high ;           // restore old value
        operation_acc_ctr_high_h = temp_acc_high_h ;       // restore old value
        a_i = temp_a_i;                                    // restore old value
        ack = temp_ack;                                    // restore old value
        rec_indx = temp_rec_indx;                          // restore old value
        for ( i = HEADER_LEN; i < transmit_cnt; i++ )
          routing_buf[i] = cur_transmit_buf[i];
        routing_buf[iLN] = transmit_cnt-2;                 // Store PDU len
        routing_buf[iRA] = vir_unit_addr;                  // Store routing address
        cnt = routing_buf[iLN] + 2;                        // find the size of the internal PDU
        transmit_cnt = PDU_BODY;
        for ( i = 0; i < cnt; i++ )
          cur_transmit_buf[transmit_cnt++] = routing_buf[i];// Copy from one to another
        t_ln = routing_buf[iLN];
        routing_status = sROUTING_REPLY_READY;
        vir_cl_9_post_fct;                                 // call user post function
      }
      else
#endif
      {                                                    // perform the routing
        routing_status   = sROUTING_LOADED;                // set status and respond it to master
        cur_transmit_buf[transmit_cnt++] = sROUTING_LOADED;
                                                           // Signal to the channel, which is going to
        c = channel_tab[i];                                // event = f(channel_select)
        PutExtEvent( c + eROUTING_REQUEST );               // signal to the channel for an request
        t_ln = 1;                                          // set length of reply APDU to 1
      }
    }
    else
      RerouteError();
  }
  else                                                     // receiving request for reply?
  {                                                        // check whether reply is waiting
    if( routing_status == sROUTING_REPLY_READY )
    {                                                      // reply was ready,
      t_ln = routing_buf[iLN] + APDU_BODY;
      for( i = APDU_BODY; i < t_ln; i++)
        cur_transmit_buf[transmit_cnt++] = routing_buf[i];
      t_ln -= APDU_BODY;
      routing_status = sROUTING_REPLY_READY;               // reset status, ready to next
    }
    else
      RerouteError();
  }
}
#endif //CTO_CLASS_9 code end

#if((CTO_CLASS_10 == Enable) || (CTO_VIR_CLASS_10 == Enable))
/****************************************************************************
*     Name      :  VirGetObjectApdu                                         *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Process the object APDU, ( object PDU's )                 *
*               : Handle to cases:                                          *
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
void VirGetObjectApdu(void)
{
  UCHAR i;
  UCHAR *object_ptr;
  UINT sub_id_code;
  UCHAR *obj_status;
  UINT source_address;


  object_ptr = (UCHAR *)(pre_tab_ptr[OBJECT_APDU].buf);

#if(VIR_CLASS_9 == Enable)
  if (vir_cl_9_flg == FALSE)
  {
    obj_status = &object_status;
  }
  else
  {
    obj_status = &virobject_status;
  }
#else
  obj_status = &object_status;
#endif

  if ( r_ln > 0 )                                          // receiving new tgm for object request ?
  {
    if(*obj_status != sOBJECT_BUSY)                    // ready to initiate new object dialog?
    {
      *obj_status = sOBJECT_LOADED;
      id_code = cur_receive_buf[rec_indx++];
      sub_id_code = cur_receive_buf[rec_indx++] << 8;      // Get Sub Id high byte
      sub_id_code |= cur_receive_buf[rec_indx++];          // Add Sub Id low byte
      source_address = cur_receive_buf[3];
      #if(VIR_CLASS_9 == Enable)
        if(vir_cl_9_flg == FALSE)
          GetObject_fct(id_code, sub_id_code, source_address);             // call application
        else
          VirGetObject_fct(id_code, sub_id_code);          // call virtual application
      #else
        GetObject_fct(id_code, sub_id_code, source_address);               // call application
      #endif
      switch(*obj_status)
      {
        case sOBJECT_PROCESSED:                            // data is ready right away
          cur_transmit_buf[transmit_cnt++] = sOBJECT_READY;// insert status
          for( i = 1; i <= *object_ptr; i++)               // move from buffer
          cur_transmit_buf[transmit_cnt++] = *(object_ptr + i);
          t_ln = *object_ptr + 1;                          // set apdu length
          break;
        case sOBJECT_LOADED:
          VirApduError(1,A_ID_ERR);    // User function has not modified *obj_status
          break;
        default:
          cur_transmit_buf[transmit_cnt++] = *obj_status;   // respond status to master
          t_ln = 1;                                        // set length
          break;
      }
      if (*obj_status != sOBJECT_BUSY)                  // data is ready
        *obj_status = sOBJECT_READY;                    /* Ready for new object */
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = sOBJECT_BUSY;     // respond busy to master
      t_ln = 1;                                            // set apdu length
      rec_indx += r_ln;                                    // inc to next APDU if more
    }
  }
  else                                                     // receiving request for reply?
  {                                                        // check whether reply is waiting
    if (*obj_status == sOBJECT_PROCESSED)                  // data is ready
    {
      cur_transmit_buf[transmit_cnt++] = sOBJECT_READY;    // insert status
      for( i = 1; i <= *object_ptr; i++)                   // move from buffer
        cur_transmit_buf[transmit_cnt++] = *(object_ptr + i);
      t_ln =  *object_ptr + 1;                             // set apdu length
      *obj_status = sOBJECT_READY; /* Ready for new object */
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = *obj_status;    // respond status to master
      t_ln = 1;                                            // set apdu length
    }
  }
}
/****************************************************************************
*     Name      :  VirSetObjectApdu                                         *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Process the object APDU, ( object PDU's )                 *
*               : Handle to cases:                                          *
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
void VirSetObjectApdu(void)
{
  UCHAR i;
  UCHAR *object_ptr;
  UINT sub_id_code;
  UCHAR *obj_status;
  UCHAR source_address;

  object_ptr = (UCHAR *)(pre_tab_ptr[OBJECT_APDU].buf);

#if(VIR_CLASS_9 == Enable)
  if (vir_cl_9_flg == FALSE)
  {
    obj_status = &object_status;
  }
  else
  {
    obj_status = &virobject_status;
  }
#else
  obj_status = &object_status;
#endif

  if ( r_ln > 0 )                                          // receiving new object
  {
    if( *obj_status != sOBJECT_BUSY )                    // ready to initiate new object dialog?
    {
      if( r_ln <= OBJECT_DF_buf_len)                       // is buffer large enough
      {
        *obj_status = sOBJECT_LOADED;                    // change state
        *object_ptr = r_ln - 3;                            // save length
        id_code = cur_receive_buf[rec_indx++];
        sub_id_code = cur_receive_buf[rec_indx++] << 8;    // Get Sub Id high byte
        sub_id_code |= cur_receive_buf[rec_indx++];        // Add Sub Id low byte
        source_address = cur_receive_buf[3];
        for( i = 1; i < (r_ln - 2) ; i++)                  // move to buffer
          *(object_ptr + i) = cur_receive_buf[rec_indx++];
        #if(VIR_CLASS_9 == Enable)
          if(vir_cl_9_flg == FALSE)
            SetObject_fct(id_code,sub_id_code, source_address);            // call application
          else
            VirSetObject_fct(id_code,sub_id_code);         // call virtual application
        #else
          SetObject_fct(id_code,sub_id_code, source_address);              // call application
        #endif

        switch(*obj_status)
        {
          case sOBJECT_PROCESSED:                          // data has been processed right away
            cur_transmit_buf[transmit_cnt++] = sOBJECT_READY; // insert status
            t_ln = 1;                                      // set apdu length
            break;
          case sOBJECT_LOADED:
            VirApduError(1,A_ID_ERR);    // User function has not modified *obj_status
            break;
          default:
            cur_transmit_buf[transmit_cnt++] = *obj_status;   // respond status to master
            t_ln = 1;                                        // set length
            break;
        }
        if (*obj_status != sOBJECT_BUSY)                  // data is ready
          *obj_status = sOBJECT_READY; /* Ready for new object */
      }
      else
      {
        VirApduError(1,A_OPR_ERR);                         // report buffer filled
      }
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = sOBJECT_BUSY;     // respond busy to master
      t_ln = 1;                                            // set apdu length
      rec_indx += r_ln;                                    // inc to next APDU if more
    }
  }
  else                                                     // received request for status
  {
    if( *obj_status == sOBJECT_PROCESSED )
    {
      cur_transmit_buf[transmit_cnt++] = sOBJECT_READY;    // respond ready to master
      *obj_status = sOBJECT_READY;
    }
    else
    {
      cur_transmit_buf[transmit_cnt++] = *obj_status;    // respond status to master
    }
    t_ln = 1;                                              // set apdu length
  }
}
/****************************************************************************
*     Name      :  SetObjectStatus                                          *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : API function the application uses this function to        *
*               : report back status of object dialog                       *
*               : value of                                                  *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void SetObjectStatus(UCHAR status)
{
  if (status <= sOBJECT_FAILED)
  {
    object_status = status;
  }
  else
  {
    object_status = sOBJECT_FAILED;
  }
}

/****************************************************************************
*     Name      :  VirSetObjectStatus                                       *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : API function the application uses this function to        *
*               : report back status of virtual object dialog               *
*               : value of                                                  *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
#if(VIR_CLASS_9 == Enable)
void VirSetObjectStatus(UCHAR status)
{
  if (status <= sOBJECT_FAILED)
  {
    virobject_status = status;
  }
  else
  {
    virobject_status = sOBJECT_FAILED;
  }
}
#endif

#endif // CTO_CLASS_10 code end
/****************************************************************************
*     Name      :  VirResetInterpreter                                      *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  reset the virtual interpreter                            *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirResetInterpreter(void)
{
  #if (USE_VIRTUAL_SLAVES == TRUE)
  UCHAR i;
  unit_count = 0;
  cur_unit = 0;
  #endif
  pre_tab_ptr = pre_tab;                                   // initialize pointers
  common_info_tab_p = common_info_tab;
  common_ptr_tab_p = common_ptr_tab;

  unit_bus_mode = geni_setup;                              // initialise the bus mode register
                                                           // default access
  operation_acc_ctr = ACCESS_CLASS_0_TO_7;                 // -7-6 -5-4 -3-2 -1-0 Class
                                                           // SGSG SGSG SGSG SGSG operation
  operation_acc_ctr_high = ACCESS_CLASS_8_TO_15;           // 15-14-13-12-11-10-9-8 Class

  operation_acc_ctr_high_h = ACCESS_CLASS_16_TO_X;              // 16 Class


#if ( CMD_BUF_LEN != 0 )
  cmd_buf[WR_INDX]  = BUF_START;                           // reset write and read index
  cmd_buf[RD_INDX]  = BUF_START;                           // for command buffer if defined
#endif

#if ( CONF_BUF_LEN != 0 )
  conf_buf[WR_INDX] = BUF_START;                           // reset write and read index for
  conf_buf[RD_INDX] = BUF_START;                           // configuration (class 4) buffer
#endif                                                     // if defined

#if ( REF_BUF_LEN != 0 )                                   // reset write and read index for
  ref_buf[WR_INDX]  = BUF_START;                           // reference buffer
  ref_buf[RD_INDX]  = BUF_START;                           // if defined
#endif

#if ( ASCII_BUF_LEN != 0 )                                   // reset write and read index for
  ascii_buf[WR_INDX]  = BUF_START;                           // ascii buffer
  ascii_buf[RD_INDX]  = BUF_START;                           // if defined
#endif


#if ( CONF16_BUF_LEN != 0 )                                // reset write and read index for
  conf16_buf[WR_INDX] = BUF_START;                         // 16 bit (class 12 ) configuration
  conf16_buf[RD_INDX] = BUF_START;                         // buffer
#endif

#if ( REF16_BUF_LEN != 0 )                                 // reset write and read index for
  ref16_buf[WR_INDX] = BUF_START;                          // 16 bit (class 13 )
  ref16_buf[RD_INDX]  = BUF_START;                         // reference buffer
#endif

#if ( CONF32_BUF_LEN != 0 )                                // reset write and read index for
  conf32_buf[WR_INDX] = BUF_START;                         // 32 bit (class 15 ) configuration
  conf32_buf[RD_INDX] = BUF_START;                         // buffer
#endif

#if ( REF32_BUF_LEN != 0 )                                 // reset write and read index for
  ref32_buf[WR_INDX] = BUF_START;                          // 32 bit (class 16 )
  ref32_buf[RD_INDX]  = BUF_START;                         // reference buffer
#endif


#if( CTO_CLASS_9 == Enable )
  routing_status = sROUTING_READY;                         // reset status, ready to next
  #if(VIR_CLASS_9 == Enable)
    #if ( VIR_CMD_BUF_LEN != 0 )
        vir_cmd_buf[WR_INDX]  = BUF_START;                 // reset write and read index
        vir_cmd_buf[RD_INDX]  = BUF_START;                 // for command buffer if defined
    #endif

    #if ( VIR_CONF_BUF_LEN != 0 )
        vir_conf_buf[WR_INDX] = BUF_START;                 // reset write and read index for
        vir_conf_buf[RD_INDX] = BUF_START;                 // configuration (class 4) buffer
    #endif                                                 // if defined

    #if ( VIR_REF_BUF_LEN != 0 )                           // reset write and read index for
        vir_ref_buf[WR_INDX]  = BUF_START;                 // reference buffer
        vir_ref_buf[RD_INDX]  = BUF_START;                 // if defined
    #endif

    #if ( VIR_CONF16_BUF_LEN != 0 )                        // reset write and read index for
        vir_conf16_buf[WR_INDX] = BUF_START;               // 16 bit (class 12 ) configuration
        vir_conf16_buf[RD_INDX] = BUF_START;               // buffer
    #endif

    #if ( VIR_REF16_BUF_LEN != 0 )                         // reset write and read index for
        vir_ref16_buf[WR_INDX] = BUF_START;                // 16 bit (class 13 )
        vir_ref16_buf[RD_INDX]  = BUF_START;               // reference buffer
    #endif

    #if ( VIR_CONF32_BUF_LEN != 0 )                        // reset write and read index for
        vir_conf32_buf[WR_INDX] = BUF_START;               // 32 bit (class 15 ) configuration
        vir_conf32_buf[RD_INDX] = BUF_START;               // buffer
    #endif

    #if ( VIR_REF32_BUF_LEN != 0 )                         // reset write and read index for
        vir_ref32_buf[WR_INDX] = BUF_START;                // 32 bit (class 16 )
        vir_ref32_buf[RD_INDX]  = BUF_START;               // reference buffer
    #endif

  #endif
#endif

#if( (CTO_CLASS_8 == Enable) || (CTO_VIR_CLASS_8 == Enable) )
  dump_status = sDUMP_READY;                               // reset status, ready to next
#endif

#if((CTO_CLASS_10 == Enable) || (CTO_VIR_CLASS_10 == Enable))
  object_status = sOBJECT_READY;                           // reset status, ready to next
#endif
#if (USE_VIRTUAL_SLAVES == TRUE)
  for(i=0; i<MAX_VIR_SLAVE_COUNT; i++)
  {
    #if ( VIR_SLAVE_CMD_BUF_LEN != 0 )
      slave_cmd_buf[i][WR_INDX]  = BUF_START;              // reset write and read index
      slave_cmd_buf[i][RD_INDX]  = BUF_START;              // for command buffer if defined
    #endif

    #if ( VIR_SLAVE_CONF_BUF_LEN != 0 )
      slave_conf_buf[i][WR_INDX] = BUF_START;              // reset write and read index for
      slave_conf_buf[i][RD_INDX] = BUF_START;              // configuration (class 4) buffer
    #endif

    #if ( VIR_SLAVE_REF_BUF_LEN != 0 )                     // reset write and read index for
      slave_ref_buf[i][WR_INDX]  = BUF_START;              // reference buffer
      slave_ref_buf[i][RD_INDX]  = BUF_START;              // reference buffer
    #endif

    #if ( VIR_SLAVE_ASCII_BUF_LEN != 0 )                   // reset write and read index for
      slave_ascii_buf[i][WR_INDX]  = BUF_START;            // ascii buffer
      slave_ascii_buf[i][RD_INDX]  = BUF_START;            // ascii buffer
    #endif

    #if ( VIR_SLAVE_CONF16_BUF_LEN != 0 )                  // reset write and read index for
      slave_conf16_buf[i][WR_INDX] = BUF_START;            // 16 bit (class 12 ) configuration
      slave_conf16_buf[i][RD_INDX] = BUF_START;            // buffer
    #endif

    #if ( VIR_SLAVE_REF16_BUF_LEN != 0 )                   // reset write and read index for
      slave_ref16_buf[i][WR_INDX] = BUF_START;             // 16 bit (class 13 )
      slave_ref16_buf[i][RD_INDX] = BUF_START;             // reference buffer
    #endif

    #if ( VIR_SLAVE_CONF32_BUF_LEN != 0 )                  // reset write and read index for
      slave_conf32_buf[i][WR_INDX] = BUF_START;            // 32 bit (class 15 ) configuration
      slave_conf32_buf[i][RD_INDX] = BUF_START;            // buffer
    #endif

    #if ( VIR_SLAVE_REF32_BUF_LEN != 0 )                   // reset write and read index for
      slave_ref32_buf[i][WR_INDX] = BUF_START;             // 32 bit (class 16 )
      slave_ref32_buf[i][RD_INDX] = BUF_START;             // reference buffer
    #endif

  }
#endif
}

/*****************************************************************************
*
*
*       Handle errors on data item level
*       Transfer the ID code from receivebuf to transmit buf
*       Set lenght of error APDU to 1
*       Set temp index of receivebuf to next APDU to handle
*
*/
void VirApduError(UCHAR d_i,UCHAR err)
{
  ack = err;
  transmit_cnt = a_i+1;
  cur_transmit_buf[transmit_cnt++] = id_code;
  t_ln = 1;
  rec_indx += r_ln - d_i;
}

/****************************************************************
*    Name       : VirInterprete                                 *
*               :                                               *
*    Inputs     : Number of received bytes                      *
*               : Index to start of PDU body                    *
*    Outputs    :                                               *
*    Updates    : cur_buf_cnt                                   *
*               : Application API variables and buffers         *
*    Returns    :                                               *
*                                                               *
*    Description: Interprete the received telegram, update      *
*    application data and build a complete                      *
*    response telegram if required.                             *
*                                                               *
*                                                               *
*****************************************************************/
void VirInterprete(UCHAR receive_cnt, UCHAR i_pdu_body)
{
  UCHAR finish = FALSE, job;
  UCHAR d;
  UCHAR a_opr;                                             // APDU operation interpreter

  rec_indx    = i_pdu_body;                                // set index to start of APDU's
  transmit_cnt = i_pdu_body;                               // set min number of data to xmit, leave space for header

  if( receive_cnt == i_pdu_body )                          // nothing to inteprete?
    job = SEND_REPLY;                                      // skip, and send reply if an request was
  else                                                     // received
    job = CHK_HEADER;                                      // else start on interprete job, first
                                                           // first byte must be an APDU header
  while (!finish)
  {
    switch(job)
    {
      /***********************  APDU level ********************************/
      case CHK_HEADER:
      t_ln = 0;                                            // reset lenght of transmit APDU
      a_hd = cur_receive_buf[rec_indx++];                  // get APDU HEADER
      if( a_hd & RFS_APDU)                                 // RFS option byte ?
      {
          job = SAVE_RFS;                                  // then handle RFS
        break;
      }
      else
      {                                                    //
        cur_transmit_buf[transmit_cnt++] = a_hd;           // assume generation of response
        a_i  = transmit_cnt++;                             // note where to store ack, lenght
        ack  = AA_OK;                                      // assume everything OK
        r_ln = cur_receive_buf[rec_indx] & (~OPR_ACK);     // free lenght
        a_opr= cur_receive_buf[rec_indx++] & OPR_ACK;      // free operation

        //  APDU known by application ?
        max_id_code = pre_tab_ptr[a_hd].max_id_code;
        if( max_id_code == 0 )
          ack = A_CLL_ERR;
        else
        {
          switch(a_opr)
          {
            case GET:
            case SET:
            data_item_ptab = pre_tab_ptr[a_hd].item_pointer_tab;
            #if ( CTO_BUF_OPT == Enable )
            acc_code = (UCHAR) (buf_opt_ctr >> ( a_hd << 1 )) & 0x03;
            #endif
            tab_buf = pre_tab_ptr[a_hd].buf;             // get table or buffer pointer
            buf_len = pre_tab_ptr[a_hd].buf_lenght;      // get max buffer lenght

            if( a_hd  < MEMORY_APDU )
            {
              d = (UCHAR) (operation_acc_ctr >> ( a_hd << 1 ) );
              if(a_opr != 0 )                            // operation allowed?
                d &= SET_ACC;
              else
                d &= GET_ACC;
              if(!d)
                ack = A_OPR_ERR;                         // not allowed, set acknowledge
            }
            else if (a_hd < REF32_APDU)
            {
              d = (UCHAR) (operation_acc_ctr_high >> ((a_hd - MEMORY_APDU) << 1));
              if(a_opr != 0 )                        // operation allowed?
                d &= SET_ACC;
              else
                d &= GET_ACC;
              if(!d)
                ack = A_OPR_ERR;                     // not allowed, set acknowledge
            }
            else
            {
              d = (UCHAR) (operation_acc_ctr_high_h >> ((a_hd - REF32_APDU) << 1));
              if(a_opr != 0 )                        // operation allowed?
                d &= SET_ACC;
              else
                d &= GET_ACC;
              if(!d)
                ack = A_OPR_ERR;                     // not allowed, set acknowledge
            }
            break;

            case INFO:
            if( (tab_buf = (void *) pre_tab_ptr[a_hd].info_tab) == 0)
              ack = A_OPR_ERR;                           // info operation, no info
            break;

            default:
            ack = A_OPR_ERR;
            break;
          }
        }
      }

      // check if OK to carry out APDU operation or any error is detected
      if(ack != AA_OK)
      {
        job = NEXT_APDU;
        rec_indx += r_ln;
      }
      else
        job = APDU_DATA;
      break;

      //
      case NEXT_APDU:                                      // always insert acknowledge and lenght
      cur_transmit_buf[a_i] = (ack | t_ln);
      if( rec_indx < receive_cnt )                         // more APDU'S to handle ?
      {
        job = CHK_HEADER;                                  // then send reply
      }
      else
        job = SEND_REPLY;
      break;

      case SEND_REPLY:
      finish = TRUE;                                       // Intepreting is now finished
      break;

      /************************** Data level ********************************/
      case APDU_DATA:
      if( a_opr == APDU_SET)                               // APDU SET operations
      {
        GENISetData(a_hd);
      }
      else
      {
        if( a_opr == APDU_GET)                             // APDU GET operations
        {
          GENIGetData(a_hd);
        }
        else
          VirInfoData();                                   // APDU INFO operations
      }
      job = NEXT_APDU;
      break;

      case SAVE_RFS:
      job = NEXT_APDU;
      break;

    } // end switch
  }   // end while

  *cur_buf_cnt = transmit_cnt;                             //  save current buffer counter
}


void GENISetData(UCHAR geni_class)
/****************************************************************************
*
*       Sort by class and set the received data
*
*/
{
  switch(geni_class)
  {
    case CMD_APDU :
      VirSetCmds();
    break;

    case PRO_APDU :
    case BUS_APDU :
    case MEAS_APDU :
    case CONF_APDU :
    case REF_APDU :
    case TEST_APDU :
      VirSetData();                                  // write data to application, 8 bit data
    break;
    #if((ACCESS_CLASS_7 & SET_ACC) > 0)
    case ASCII_APDU :
      VirSetASCII();
    break;
    #endif

    case MEMORY_APDU :
      #if( (CTO_CLASS_8 == Enable) || (CTO_VIR_CLASS_8 == Enable) )
        VirSetDumpApdu();                          // class 8 enabled special processing
      #else
        ack = A_CLL_ERR;                           // no class 8, report error
      #endif
    break;

    case OBJECT_APDU :
      #if((CTO_CLASS_10 == Enable) || (CTO_VIR_CLASS_10 == Enable))
        VirSetObjectApdu();                      // class 10 enabled special processing
      #else
        ack = A_CLL_ERR;                         // no class 10, report error
      #endif
    break;

    case MEAS16_APDU :
    case CONF16_APDU :
    case REF16_APDU  :
      #if( (CTO_CLASS_16_BIT == Enable) || (CTO_VIR_CLASS_16_BIT == Enable) )
        VirSetData16();                          // 16 bit classes are enabled
      #else
        ack = A_CLL_ERR;                         // no 16 bit classes, report error
      #endif
    break;

    case MEAS32_APDU :
    case CONF32_APDU :
    case REF32_APDU  :
      #if( (CTO_CLASS_32_BIT == Enable) || (CTO_VIR_CLASS_32_BIT == Enable) )
        VirSetData32();                          // 32 bit classes are enabled
      #else
        ack = A_CLL_ERR;                         // no 32 bit classes, report error
      #endif
    break;

    default:
      ack = A_CLL_ERR;                         // no valid classes, report error
    break;
  }
}


/*****************************************************************************
*
*
*       Get command ID and store in the application command buffer
*       Insert channel flag in command ID bit 7
*
*/
void VirSetCmds(void)
{
  UCHAR i;
  UCHAR *cmd_buf;
  cmd_buf = (UCHAR *) tab_buf;                             // set the adress on command buffer

// loop until end of APDU and no APDU errors detected
  for( i = 0 ;(i < r_ln) && ( ack == AA_OK) ; )
  {
    i++;                                                   // one command
    id_code = cur_receive_buf[rec_indx++];                 // get command ID code
    if(cmd_buf[WR_INDX] <= ROOM_FOR_ID)                    // buf input index <
    {
      if( id_code <= max_id_code )                         //
      {
        #if (CTO_BUF_INSERT_CH_INDX == Enable)
          cmd_buf[(*cmd_buf)++] = cur_indx;                // Insert channel index
        #endif
          if( (SINT) cur_channel == IR_CH )                // Insert channel flag in command
            cmd_buf[(*cmd_buf)++] = id_code;               // Command from IR channel
          else
            cmd_buf[(*cmd_buf)++] = (id_code | 0x80 );     // Command from other channel than IR
      }
      else
        VirApduError(i,A_ID_ERR);
    }
    else
      VirApduError(i,A_OPR_ERR);
  }
}


#if((ACCESS_CLASS_7 & SET_ACC) > 0)
/****************************************************************************
*
*       Transfer received data to application memory:
*       If the CTO_BUF_OPT facility not enabled then update the
*       application memory directly
*
*       If the CTO_BUF_OPT facility is enabled, then select the
*       access method required for the given class and transfer the
*       received data to application.
*
*/
void VirSetASCII(void)
{
  UCHAR *parm_p;                                               // application parameter pointer
  #if ( CTO_BUF_OPT == Disable)
  const UCHAR acc_code = PARM_ACC;                             // the only way when not using buffers
  #endif

  id_code = cur_receive_buf[rec_indx++];                       // get id code  - only one ID pr. PDU
  parm_p  = (UCHAR *) data_item_ptab[id_code];                 // where to GET or SET

  if((id_code <= max_id_code) && (parm_p != 0) && (parm_p != (UCHAR *)NA))   // id, parameter pnt Ok?
  {
    if(cur_receive_buf[(rec_indx-1) + r_ln - 1] == 0)              // is the character a \0 character
    {
      switch( acc_code )                                       // acc_options in use
      {
        /**************************************************************************************************************************/
        case PARM_ACC:                                         // direct app. access
          SaveToASCIIParam(parm_p);
        break;

        #if (CTO_BUF_OPT == Enable)                            // buffer application access
        /**************************************************************************************************************************/
        case BUF_ID:                                          // idcode and data to buffer
          SaveToASCIIBuffer(id_code);
        break;

        /**************************************************************************************************************************/
        case BUF_ADDR:                                          // Store parameter and insert addres in buffer
          SaveToASCIIAddress(parm_p);
        break;
        #endif
      }
    }
    else
      VirApduError(1,A_OPR_ERR);                               // No trailing \0
  }
  else
    VirApduError(1,A_ID_ERR);                                  // Illegal ID
}

static UCHAR SaveToASCIIParam(UCHAR* parm_p)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :  Returns TRUE if no errors occurred
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :  Saves the string to the parameter
*
****************************************************************************/
{
  UCHAR i = 0;
  UCHAR ok = FALSE;
  if ((r_ln-1) <= ASCII_SIZE)                        // PDU length - 1 <= Ascii parameter size
  {
    while(i < ASCII_SIZE)                            // write to all characters in parameter
    {
      if(i < (r_ln-1))                               // more left of the PDU
        *parm_p = cur_receive_buf[rec_indx++];       // transfer data
      else
        *parm_p = 0;                                 // pad with zeros in the rest of the parameter
      i++;
      parm_p++;
    }
    ok = TRUE;
  }
  else
    VirApduError(1,A_OPR_ERR);                       // PDU too big

  return ok;
}

#if (CTO_BUF_OPT == Enable)
static void SaveToASCIIBuffer(UCHAR id_code)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Saves the string to the buffer
*
****************************************************************************/
{
  UCHAR i = 0;
  UCHAR *tab_buf8;
  tab_buf8 = (UCHAR *) tab_buf;

  if(    (*tab_buf8 <= (ROOM_FOR_ID - (r_ln-1)))       // check for space for id and ascii char
      && ((r_ln-1) <= ASCII_SIZE))                     // and PDU length - 1 <= Ascii parameter size

  {
    #if (CTO_BUF_INSERT_CH_INDX == Enable)
      tab_buf8[(*tab_buf8)++] = cur_indx;               // Insert channel index
    #endif
    tab_buf8[(*tab_buf8)++] = id_code;                  // get ID code

    while(i < (r_ln-1))                               // Save all characters to buffer
    {
      tab_buf8[(*tab_buf8)++] = cur_receive_buf[rec_indx++];
      i++;                                            // increment counter
    }
  }
  else
    VirApduError(1,A_OPR_ERR);                        // error no room in buffer
}

static void SaveToASCIIAddress(UCHAR *parm_p)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Saves the string to the parameter and the address of
*                        the parameter to the buffer
*
****************************************************************************/
{
  UCHAR *tab_buf8;
  tab_buf8 = (UCHAR *) tab_buf;

  if( *tab_buf8 <= ROOM_FOR_ADDRESS)                   // check for space
  {
    if(SaveToASCIIParam(parm_p))
    {
      #if (CTO_BUF_INSERT_CH_INDX == Enable)
        tab_buf8[(*tab_buf8)++] = cur_indx;              // Insert channel index
      #endif
      tab_buf8[(*tab_buf8)++] = HI_val(parm_p);            // store app. mem. address
      tab_buf8[(*tab_buf8)++] = LO_val(parm_p);            // store app. mem. address
    }
  }
  else
    VirApduError(1,A_OPR_ERR);                         // No room in buffer
}

#endif
#endif // ((ACCESS_CLASS_7 & SET_ACC) > 0)
/****************************************************************************
*
*       Transfer received data to application memory:
*       If the CTO_BUF_OPT facility not enabled then update the
*       application memory directly
*
*       If the CTO_BUF_OPT facility is enabled, then select the
*       access method required for the given class and transfer the
*       received data to application.
*
*/
void VirSetData(void)
{
  UCHAR i;
  UCHAR *parm_p;                                           // application parameter pointer
#if ( CTO_BUF_OPT == Enable)
  UCHAR *tab_buf8;
  tab_buf8 = (UCHAR *) tab_buf;
#else
  const UCHAR acc_code = PARM_ACC;                         // the only way when not using buffers
#endif

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )
  {
    i++;
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to GET or SET

    switch( acc_code )                                     // acc_options in use
    {
      /**************************************************************************************************************************/
      case PARM_ACC:                                       // direct app. access
      if( (id_code <= max_id_code) && (parm_p != 0) && (parm_p !=(UCHAR *) NA) ) // id and parameter pnt OK?
      {
        *parm_p = cur_receive_buf[rec_indx++];             // transfer data
        i++;
      }
      else
        VirApduError(i,A_ID_ERR);

      break;

      #if ( CTO_BUF_OPT == Enable)                           // buffer application access
      /**************************************************************************************************************************/
      case BUF_ID:                                         // id code and data to buffer
      if( *tab_buf8 <= ROOM_FOR_ID_N_DATA)                 // check for space
      {
        #if (CTO_BUF_INSERT_CH_INDX == Enable)
          tab_buf8[(*tab_buf8)++] = cur_indx;              // Insert channel index
        #endif
        tab_buf8[(*tab_buf8)++] = id_code;                 // get ID code
        tab_buf8[(*tab_buf8)++] = cur_receive_buf[rec_indx++];
        i ++;                                              // increment counter
      }
      else
        VirApduError(i,A_OPR_ERR);
      break;

      /**************************************************************************************************************************/
      case BUF_ADDR:                                       // Store parameter and put address in buffer
      if( *tab_buf8 <= ROOM_FOR_ADDRESS)                   // check for space
      {
        #if (CTO_BUF_INSERT_CH_INDX == Enable)
          tab_buf8[(*tab_buf8)++] = cur_indx;              // Insert channel index
        #endif
        *parm_p = cur_receive_buf[rec_indx++];             // get ID code
        tab_buf8[(*tab_buf8)++] = HI_val(parm_p);          // store app. mem. address
        tab_buf8[(*tab_buf8)++] = LO_val(parm_p);          // store app. mem. address
        i++;                                               // increment item counter
      }
      else
        VirApduError(i,A_OPR_ERR);
      break;
      #endif
    }
  }
}

#if( (CTO_CLASS_16_BIT == Enable) || (CTO_VIR_CLASS_16_BIT == Enable) )         // 16 bit classes included
/****************************************************************************
*     Name      : VirSetData16(void)                                        *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirSetData16(void)
{
  UCHAR i;

#if ( CTO_BUF16_OPT == Enable )                            // access during 16 bit buffers
                                                           // and only one method which
  UINT *buf16_buf;                                         // is ID , 16 bit value
  UCHAR wr_index;
  buf16_buf = (UINT *) tab_buf;                            // get the adress on buffer

  if( buf_len > 0)
  {
    wr_index = buf16_buf[WR_INDX];                         // get write index
    for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )          // save all data in the APDU
    {                                                      // if possible
      i++;                                                 // inc one for ID-code
      id_code = cur_receive_buf[rec_indx++];               // get id code
      if( wr_index <= ROOM_FOR_ID_N_DATA )                 // check for space
      {                                                    // write value to buffer
        #if (CTO_BUF_INSERT_CH_INDX == Enable)
          buf16_buf[wr_index++] = (UINT)cur_indx;          // Insert channel index
        #endif
        buf16_buf[wr_index++]  = (UINT) id_code;           // write
        HI_pnt((UCHAR *) &buf16_buf[wr_index]) = cur_receive_buf[rec_indx++];
        LO_pnt((UCHAR *) &buf16_buf[wr_index]) = cur_receive_buf[rec_indx++];
        wr_index++;
        i += 2;                                            // increment counter
      }
      else
      {
        rec_indx++;                                        // be sure to move through buffer even when error occurs
        VirApduError(i,A_OPR_ERR);
      }
    }
    buf16_buf[WR_INDX] = wr_index;                         // save index again always
  }

#else

  UCHAR *parm_p;                                           // application parameter pointer

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )            //
  {
    i++;                                                   // inc one for ID-code
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to GET or SET
    if( (id_code <= max_id_code) && (parm_p != 0) && (parm_p != (UCHAR *)NA) )   // id and parameter pnt OK?
    {
        HI_pnt(parm_p) = cur_receive_buf[rec_indx++];      // transfer highorder
        LO_pnt(parm_p) = cur_receive_buf[rec_indx++];      // transfer loworder
        i +=2;                                             // 2 bytes of data
    }
    else
    {
      rec_indx++;                                          // be sure to move through buffer even when error occurs
      VirApduError(i,A_ID_ERR);
    }
  }

#endif

}
#endif           //             16 classes included

#if( (CTO_CLASS_32_BIT == Enable) || (CTO_VIR_CLASS_32_BIT == Enable) )         // 32 bit classes included
/****************************************************************************
*     Name      : VirSetData32(void)                                        *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirSetData32(void)
{
  UCHAR i;

#if ( CTO_BUF32_OPT == Enable )                            // access during 32 bit buffers
                                                           // and only one method which
  ULONG *buf32_buf;                                         // is ID , 32 bit value
  UCHAR wr_index;
  buf32_buf = (ULONG *) tab_buf;                           // get the adress on buffer

  if( buf_len > 0)
  {
    wr_index = buf32_buf[WR_INDX];                         // get write index
    for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )          // save all data in the APDU
    {                                                      // if possible
      i++;                                                 // inc one for ID-code
      id_code = cur_receive_buf[rec_indx++];               // get id code
      if( wr_index <= ROOM_FOR_ID_N_DATA )                 // check for space
      {                                                    // write value to buffer
        #if (CTO_BUF_INSERT_CH_INDX == Enable)
          buf32_buf[wr_index++] = (UINT)cur_indx;          // Insert channel index
        #endif
        buf32_buf[wr_index++]  = (UINT) id_code;           // write
        HI_pnt_msb((UCHAR *) &buf32_buf[wr_index]) = cur_receive_buf[rec_indx++];
        HI_pnt_lsb((UCHAR *) &buf32_buf[wr_index]) = cur_receive_buf[rec_indx++];
        LO_pnt_msb((UCHAR *) &buf32_buf[wr_index]) = cur_receive_buf[rec_indx++];
        LO_pnt_lsb((UCHAR *) &buf32_buf[wr_index]) = cur_receive_buf[rec_indx++];
        wr_index++;
        i += 4;                                            // increment counter
      }
      else
      {
        rec_indx += 3;                                     // be sure to move through buffer even when error occurs
        VirApduError(i,A_OPR_ERR);
      }
    }
    buf32_buf[WR_INDX] = wr_index;                         // save index again always
  }

#else

  UCHAR *parm_p;                                           // application parameter pointer

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )            //
  {
    i++;                                                   // inc one for ID-code
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to GET or SET
    if( (id_code <= max_id_code) && (parm_p != 0) && (parm_p != (UCHAR *)NA) )   // id and parameter pnt OK?
    {
        HI_pnt_msb(parm_p) = cur_receive_buf[rec_indx++];      // transfer higest highorder
        HI_pnt_lsb(parm_p) = cur_receive_buf[rec_indx++];      // transfer highorder
        LO_pnt_msb(parm_p) = cur_receive_buf[rec_indx++];      // transfer loworder
        LO_pnt_lsb(parm_p) = cur_receive_buf[rec_indx++];      // transfer lowest loworder
        i +=4;                                                 // 4 bytes of data
    }
    else
    {
      rec_indx += 3;                                          // be sure to move through buffer even when error occurs
      VirApduError(i,A_ID_ERR);
    }
  }

#endif

}
#endif           //             32 classes included

void GENIGetData(UCHAR geni_class)
/****************************************************************************
*
*       Sort by class and get the requested data
*
*/
{
  switch(geni_class)                                     // class
  {
    case PRO_APDU :
    case BUS_APDU :
    case MEAS_APDU :
    case CONF_APDU :
    case REF_APDU :
    case TEST_APDU :
      VirGetData();                                // get 8 bit data from application
    break;

    case ASCII_APDU :
      VirGetASCII();                                // get ASCII data from application
    break;

    case MEMORY_APDU :
      #if( (CTO_CLASS_8 == Enable) || (CTO_VIR_CLASS_8 == Enable) )
        VirGetDumpApdu();                        // yes, then special processing
      #else
        ack = A_CLL_ERR;
      #endif
    break;

    case REROUTE_APDU :
      #if( CTO_CLASS_9 == Enable )
        VirProcessRerouteApdu();               // yes, then special processing
      #else
        ack = A_CLL_ERR;
      #endif
    break;

    case OBJECT_APDU :
      #if((CTO_CLASS_10 == Enable) || (CTO_VIR_CLASS_10 == Enable))
         VirGetObjectApdu();                 // yes, then special processing
      #else
         ack = A_CLL_ERR;
      #endif
    break;

    case MEAS16_APDU :
    case CONF16_APDU :
    case REF16_APDU  :
      #if( (CTO_CLASS_16_BIT == Enable) || (CTO_VIR_CLASS_16_BIT == Enable) )
      VirGetData16();                                // get 16 bit data from application
      #else
      ack = A_CLL_ERR;                               // no 16 bit classes, report error
      #endif
    break;

    case MEAS32_APDU :
    case CONF32_APDU :
    case REF32_APDU  :
      #if( (CTO_CLASS_32_BIT == Enable) || (CTO_VIR_CLASS_32_BIT == Enable) )
      VirGetData32();                                // get 32 bit data from application
      #else
      ack = A_CLL_ERR;                               // no 32 bit classes, report error
      #endif
    break;

    default:
      ack = A_CLL_ERR;                               // not a valid class, report error
    break;
  }
}


/****************************************************************************
*
*       Read all required data items requested from application memory
*       and build an response APDU.
*
*/
void VirGetASCII(void)
{
  UCHAR i;
  UCHAR *parm_p;                                           // application parameter pointer

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )
  {
    i++;
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to set or store
    if( (id_code <= max_id_code) && (parm_p != 0) )        // id and parameter pnt OK?
    {
      // transfer string to transmit buffer
      for(  ;(t_ln < MAX_APDU_DATA ) && (parm_p[t_ln] != 0); )
        cur_transmit_buf[transmit_cnt++] = parm_p[t_ln++];
      // if end of string is detected
      if( t_ln < MAX_APDU_DATA )
      {
        cur_transmit_buf[transmit_cnt++] = 0;
        t_ln++;
      }
      i = r_ln;
    }
    else
      VirApduError(i,A_ID_ERR);
  }
}

/****************************************************************************
*
*       Read all required data items requested from application memory
*       and build an response APDU.
*
*/
void VirGetData(void)
{
  UCHAR i;
  UCHAR *parm_p;                                           // application parameter pointer

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )
  {
    i++;
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to set or store
    if( (id_code <= max_id_code) && (parm_p != 0) )        // id and parameter pnt OK?
    {
      cur_transmit_buf[transmit_cnt++] = *parm_p;        // else just get data
      t_ln++;
    }
    else
      VirApduError(i,A_ID_ERR);
  }
}
#if( (CTO_CLASS_16_BIT == Enable) || (CTO_VIR_CLASS_16_BIT == Enable) )         // 16 bit classes included
/****************************************************************************
*     Name      : VirGetData16(void)                                        *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirGetData16(void)
{
  UCHAR i;
  UCHAR *parm_p;

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )
  {
    i ++;                                                  // inc one for ID code
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to set or store
    if( (id_code <= max_id_code) && (parm_p != 0) )        // id and parameter pnt OK?
    {
      cur_transmit_buf[transmit_cnt++] =   HI_pnt(parm_p); // save highorder first
      cur_transmit_buf[transmit_cnt++] =   LO_pnt(parm_p); // and then low order
      t_ln +=2;
    }
    else
      VirApduError(i,A_ID_ERR);
  }
}
#endif                         //             16 classes included

#if( (CTO_CLASS_32_BIT == Enable) || (CTO_VIR_CLASS_32_BIT == Enable) )         // 32 bit classes included
/****************************************************************************
*     Name      : VirGetData32(void)                                        *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void VirGetData32(void)
{
  UCHAR i;
  UCHAR *parm_p;

  for( i = 0; (i < r_ln) && (ack == AA_OK) ;  )
  {
    i ++;                                                  // inc one for ID code
    id_code = cur_receive_buf[rec_indx++];                 // get id code
    parm_p  = (UCHAR *) data_item_ptab[id_code];           // where to set or store
    if( (id_code <= max_id_code) && (parm_p != 0) )        // id and parameter pnt OK?
    {
      cur_transmit_buf[transmit_cnt++] =   HI_pnt_msb(parm_p); // save highorder first
      cur_transmit_buf[transmit_cnt++] =   HI_pnt_lsb(parm_p); // save highorder first
      cur_transmit_buf[transmit_cnt++] =   LO_pnt_msb(parm_p); // and then low order
      cur_transmit_buf[transmit_cnt++] =   LO_pnt_lsb(parm_p); // and then low order
      t_ln +=4;
    }
    else
      VirApduError(i,A_ID_ERR);
  }
}
#endif                         //             32 classes included

/****************************************************************************
*
*       Find Info data for all requested data items, and build
*       up and response APDU
*
*/
void VirInfoData(void)
{
  UCHAR i,j;
  UCHAR d;
  UCHAR *parm_p;                                           // application parameter pointer
  UCHAR *tab_buf8;

  tab_buf8 = (UCHAR *) tab_buf;

  for( i = 0; (i < r_ln) && ( ack == AA_OK) ; )
  {
    i++;
    id_code = cur_receive_buf[rec_indx++];                 // get ID code
    d = tab_buf8[id_code] & INFO_DSC_M;                    // get info descriptor

    if( (id_code <= max_id_code) && ( d != 0) )            // check ID code, descriptor
    {

      j = 4;                                               // assume full info
      switch(d)
      {
        case INFO_HEAD:
        parm_p = &tab_buf8[id_code];                       // get address on entry
        j = 1;
        break;

        case INFO_TAB_INDX:
        d = tab_buf8[id_code] & COM_TAB_INDX;              // index to common_info_tab
        parm_p = (UCHAR *) &(common_info_tab_p[d].info_head);
        break;

        case INFO_PTR_INDX:
        d = tab_buf8[id_code] & COM_TAB_INDX;              // get index
        parm_p = (UCHAR *) common_ptr_tab_p[d];            // get ram pointer
        break;
      }

      // get info data
      for( d = 0; d < j; d++)
      {
        cur_transmit_buf[transmit_cnt++] = *(parm_p++);
        t_ln++;
      }
    }
    else
      VirApduError(i,A_ID_ERR);
  }
}

/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
