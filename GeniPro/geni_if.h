/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project: GENIpro                                          */
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
/* MODULE NAME      :   geni_if.h                                           */
/*                                                                          */
/* FILE NAME        :   geni_if.h                                           */
/*                                                                          */
/* FILE DESCRIPTION :   Interface file for GENI                             */
/*                                                                          */
/****************************************************************************/

#ifndef _GENI_IF_H
  #define _GENI_IF_H

#include "typedef.h"                                       //definements for the Grundfos common types
#include "common.h"
#include "geni_cnf.h"
#include "geni_rtos_if.h"
#if (USE_VIRTUAL_SLAVES == TRUE)                           // virtual slave enabled
#include "vir_slave_tab.h"
#endif
#if ((CTO_CLASS_9  == Enable) &&  (VIR_CLASS_9 == Enable)) // virtual class 9 enabled
#include "vir_cl_9_tab.h"
#endif

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L   C O N S T A N T S                                          */
/*                                                                          */
/****************************************************************************/

EXTERN const ID_PTR   pro_tab[HIGH_PRO_ID+1];              // Access to protocol descriptor table for Genibus
EXTERN const ID_PTR   ch_tab[HIGH_CH_ID+1];                // Access to Genibus descriptor table

#if ( HIGH_MEAS_ID != 0 )
  EXTERN const ID_PTR  meas_tab[HIGH_MEAS_ID+1];
  EXTERN const ID_INFO meas_info_tab[HIGH_MEAS_ID+1];
#endif

#if ( HIGH_CMD_ID != 0 )
  EXTERN const ID_INFO cmd_info_tab[HIGH_CMD_ID+1];
#endif

#if ( HIGH_CONF_ID != 0)
  EXTERN const ID_PTR  conf_tab[HIGH_CONF_ID+1];
  EXTERN const ID_INFO conf_info_tab[HIGH_CONF_ID+1];
#endif

#if ( HIGH_REF_ID  != 0)
  EXTERN const ID_PTR   ref_tab[HIGH_REF_ID+1];
  EXTERN const ID_INFO ref_info_tab[HIGH_REF_ID+1];
#endif

#if ( HIGH_TEST_ID != 0 )
  EXTERN const ID_PTR  test_tab[HIGH_TEST_ID+1];
#endif

#if ( HIGH_ASCII_ID != 0 )
  EXTERN const ID_PTR ascii_tab[HIGH_ASCII_ID+1];
#endif

#if (COM_INFO_LEN != 0)
  EXTERN const INFO_DATA  common_info_tab[COM_INFO_LEN];
#endif

#if (COM_PTR_LEN != 0)
  EXTERN const INFO_DATA_PTR common_ptr_tab[COM_PTR_LEN];
#endif

#if ( HIGH_MEAS16_ID != 0 )
  EXTERN const ID_PTR  meas16_tab[HIGH_MEAS16_ID+1];
  EXTERN const ID_INFO meas16_info_tab[HIGH_MEAS16_ID+1];
#endif

#if ( HIGH_CONF16_ID != 0)
  EXTERN const ID_PTR  conf16_tab[HIGH_CONF16_ID+1];
  EXTERN const ID_INFO conf16_info_tab[HIGH_CONF16_ID+1];
#endif

#if ( HIGH_REF16_ID  != 0)
  EXTERN const ID_PTR   ref16_tab[HIGH_REF16_ID+1];
  EXTERN const ID_INFO  ref16_info_tab[HIGH_REF16_ID+1];
#endif

#if ( HIGH_MEAS32_ID != 0 )
  EXTERN const ID_PTR  meas32_tab[HIGH_MEAS32_ID+1];
  EXTERN const ID_INFO meas32_info_tab[HIGH_MEAS32_ID+1];
#endif

#if ( HIGH_CONF32_ID != 0)
  EXTERN const ID_PTR  conf32_tab[HIGH_CONF32_ID+1];
  EXTERN const ID_INFO conf32_info_tab[HIGH_CONF32_ID+1];
#endif

#if ( HIGH_REF32_ID  != 0)
  EXTERN const ID_PTR   ref32_tab[HIGH_REF32_ID+1];
  EXTERN const ID_INFO  ref32_info_tab[HIGH_REF32_ID+1];
#endif

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

/****************************************************************************/
/* Common variables used in all applications                                */
/****************************************************************************/
 EXTERN UCHAR slave_unit_addr;
 EXTERN UCHAR slave_group_addr;
 EXTERN UCHAR geni_setup;
 EXTERN UCHAR unit_bus_mode;

#if (CTO_BUS_TYPE == Slave)
 EXTERN UCHAR baud_rate_default;
 EXTERN UCHAR set_baud_rate;
#endif

 EXTERN UCHAR min_reply_delay_default;
 EXTERN UCHAR set_min_reply_delay;

 // only used when using enhanced features of powerline modem
 EXTERN BIT mdm_busy;
 EXTERN ULONG mdm_reg_val;

 // only used if timer idle and soft timer chosen in geni_cnf.h
 EXTERN BIT BUS_soft_timer_started;
 EXTERN BIT COM_soft_timer_started;
 EXTERN BIT RS232_soft_timer_started;

 // only used when using embOS RTOS
 EXTERN SEMA_TYPE geni_class_data;                          // semaphor for class data

/****************************************************************************/
/* Variables used for Slave applications                                    */
/****************************************************************************/
// only avaible when modbus fuc is enabled
#if(CTO_MOD_BUS == Enable)
EXTERN UCHAR modbus_info;                       // MSB is the modbus on/off, the rest is the channel indx
#endif

/****************************************************************************/
/* Variables used in all Master applications                                */
/****************************************************************************/
#if (defined MAS_CH)
  EXTERN UCHAR master_unit_addr;                         // unit address for the master
#else
  EXTERN const UCHAR master_unit_addr;
#endif

/****************************************************************************/
/* Variables used for MP Master applications - se also slave variables      */
/****************************************************************************/
 EXTERN UNIT_RECORD network_list[MAS_MAX_UNIT_COUNT];       // list of units to be polled - read only

 /****************************************************************************/
/* Variables used for PP Master applications                                */
/****************************************************************************/

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* G L O B A L    B U F F E R   V A R I A B L E S                           */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

/****************************************************************************/
/*  rx and tx buffers                                                       */
/****************************************************************************/
EXTERN UCHAR BUS_tx_buf[BUS_DF_buf_len];
EXTERN UCHAR BUS_rx_buf[BUS_DF_buf_len];

EXTERN UCHAR IR_tx_buf[IR_DF_buf_len];
EXTERN UCHAR IR_rx_buf[IR_DF_buf_len];

EXTERN UCHAR PLM_tx_buf[PLM_DF_buf_len];
EXTERN UCHAR PLM_rx_buf[PLM_DF_buf_len];

EXTERN UCHAR COM_tx_buf[COM_DF_buf_len];
EXTERN UCHAR COM_rx_buf[COM_DF_buf_len];

EXTERN UCHAR RS232_tx_buf[RS232_DF_buf_len];
EXTERN UCHAR RS232_rx_buf[RS232_DF_buf_len];

#ifdef TCS_USB_SER_PORT
EXTERN UCHAR USB_tx_buf[USB_DF_buf_len];
EXTERN UCHAR USB_rx_buf[USB_DF_buf_len];
#endif
/****************************************************************************/
/*  rx and tx buffers ends                                                  */
/****************************************************************************/
/****************************************************************************/
/*  class buffers                                                           */
/****************************************************************************/
#if ( CTO_CLASS_10 == Enable )
  EXTERN UCHAR object_buf[OBJECT_DF_buf_len+1];             // object PDU's
#endif

#if ( CONF_BUF_LEN != 0 )
  EXTERN BUFFER conf_buf[CONF_BUF_LEN+2];                   // configuration buffer
#endif

#if ( CMD_BUF_LEN != 0 )
  EXTERN BUFFER cmd_buf[CMD_BUF_LEN+2];                     // command buffer
#endif

#if ( REF_BUF_LEN != 0 )
  EXTERN BUFFER ref_buf[REF_BUF_LEN+2];                     // reference buffer
#endif

#if ( ASCII_BUF_LEN != 0 )
  EXTERN BUFFER ascii_buf[ASCII_BUF_LEN+2];                 // text buffer
#endif

#if ( CONF16_BUF_LEN != 0 )
  EXTERN BUFFER16 conf16_buf[CONF16_BUF_LEN+2];             // configuration buffer 16 bit
#endif

#if ( REF16_BUF_LEN != 0 )
  EXTERN BUFFER16 ref16_buf[REF16_BUF_LEN+2];               // reference buffer 16 bit
#endif

#if ( CONF32_BUF_LEN != 0 )
  EXTERN BUFFER32 conf32_buf[CONF32_BUF_LEN+2];             // configuration buffer 32 bit
#endif

#if ( REF32_BUF_LEN != 0 )
  EXTERN BUFFER32 ref32_buf[REF32_BUF_LEN+2];               // reference buffer 32 bit
#endif

/****************************************************************************/
/*  Class buffers ends                                                      */
/****************************************************************************/
/****************************************************************************/
/*  Virtual Slave buffers                                                   */
/****************************************************************************/
#if (USE_VIRTUAL_SLAVES == TRUE)

  #if ( VIR_SLAVE_CMD_BUF_LEN != 0 )
    EXTERN BUFFER slave_cmd_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CMD_BUF_LEN+2];            // command buffer
  #endif

  #if ( VIR_SLAVE_CONF_BUF_LEN != 0 )
    EXTERN BUFFER slave_conf_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CONF_BUF_LEN+2];          // configuration buffer
  #endif

  #if ( VIR_SLAVE_REF_BUF_LEN != 0 )
    EXTERN BUFFER slave_ref_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_REF_BUF_LEN+2];            // reference buffer
  #endif

  #if ( VIR_SLAVE_REF_BUF_LEN != 0 )
    EXTERN BUFFER slave_ascii_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_ASCII_BUF_LEN+2];        // ascii buffer
  #endif

  #if ( VIR_SLAVE_CONF16_BUF_LEN != 0 )
    EXTERN BUFFER16 slave_conf16_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CONF16_BUF_LEN+2];    // configuration buffer 16 bit
  #endif

  #if ( VIR_SLAVE_REF16_BUF_LEN != 0 )
    EXTERN BUFFER16 slave_ref16_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_REF16_BUF_LEN+2];      // reference buffer 16 bit
  #endif
  #if ( VIR_SLAVE_CONF32_BUF_LEN != 0 )
    EXTERN BUFFER32 slave_conf32_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CONF32_BUF_LEN+2];    // configuration buffer 32 bit
  #endif

  #if ( VIR_SLAVE_REF32_BUF_LEN != 0 )
    EXTERN BUFFER32 slave_ref32_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_REF32_BUF_LEN+2];      // reference buffer 32 bit
  #endif

#endif
/****************************************************************************/
/*  Virtual Slave ends                                                      */
/****************************************************************************/
/****************************************************************************/
/*  Virtual class 9 routing buffers                                         */
/****************************************************************************/
#if ((CTO_CLASS_9  == Enable) &&  (VIR_CLASS_9 == Enable))

  #if ( VIR_CMD_BUF_LEN != 0 )
    EXTERN BUFFER vir_cmd_buf[VIR_CMD_BUF_LEN+2];           // command buffer
  #endif

  #if ( VIR_CONF_BUF_LEN != 0 )
    EXTERN BUFFER vir_conf_buf[VIR_CONF_BUF_LEN+2];         // conf buffer
  #endif

  #if ( VIR_REF_BUF_LEN != 0 )
    EXTERN BUFFER vir_ref_buf[VIR_REF_BUF_LEN+2];           // ref buffer
  #endif

  #if ( VIR_ASCII_BUF_LEN != 0 )
    EXTERN BUFFER vir_ascii_buf[VIR_ASCII_BUF_LEN+2];       // ascii buffer
  #endif

  #if ( CTO_VIR_CLASS_10 == Enable )
    EXTERN UCHAR vir_object_buf[VIR_OBJECT_DF_buf_len+1];   // object PDU's
  #endif

  #if ( VIR_CONF16_BUF_LEN != 0 )
    EXTERN BUFFER16 vir_conf16_buf[VIR_CONF16_BUF_LEN +2];  // conf16 buffer
  #endif

  #if ( VIR_REF16_BUF_LEN != 0 )
    EXTERN BUFFER16 vir_ref16_buf[VIR_REF16_BUF_LEN +2];    // ref16 buffer
  #endif

  #if ( VIR_CONF32_BUF_LEN != 0 )
    EXTERN BUFFER32 vir_conf32_buf[VIR_CONF32_BUF_LEN +2];  // conf32 buffer
  #endif

  #if ( VIR_REF32_BUF_LEN != 0 )
    EXTERN BUFFER32 vir_ref32_buf[VIR_REF32_BUF_LEN +2];    // ref32 buffer
  #endif

#endif
/****************************************************************************/
/*  Virtual class 9 routing ends                                            */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* Common functions used in all applications                                */
/****************************************************************************/
EXTERN void  GeniSysIrq(void);
EXTERN void  GeniSysInit(void);
EXTERN void  GeniSysTask(void);
EXTERN void  GeniOpenChannel(UCHAR ch_indx, UCHAR setup_param);
EXTERN void  GeniCloseChannel(UCHAR ch_indx);
EXTERN void  GeniResetChannel(UCHAR ch_indx);
EXTERN void  GeniSetActBusBaudrate(void);
EXTERN void  GeniPresetActBusBaudrate(void);

// only used when using enhanced features of powerline modem
EXTERN void PLMSetMDMRegister(void);
EXTERN void PLMGetMDMRegister(void);

 // only used with the IR channel
 EXTERN UCHAR IRTestNClearCollisionBit(void);

// only  used whith the Software Idle Timer detection
#if (((BUS_IDLE_TYPE   == TIMER_IDLE ) && (BUS_IDLE_TIMER   == SOFT))  || (BUS_IDLE_TYPE   == GENI_IRQ_IDLE))
  EXTERN void GeniBUSIdleTimerIrq(void);                      // for BUS channel
#endif
#if (((PLM_IDLE_TYPE   == TIMER_IDLE ) && (PLM_IDLE_TIMER   == SOFT))  || (PLM_IDLE_TYPE   == GENI_IRQ_IDLE))
  EXTERN void GeniPLMIdleTimerIrq(void);                      // for BUS channel
#endif
#if (((RS232_IDLE_TYPE == TIMER_IDLE ) && (RS232_IDLE_TIMER == SOFT))  || (RS232_IDLE_TYPE == GENI_IRQ_IDLE))
  EXTERN void GeniRS232IdleTimerIrq(void);                    // for RS232 channel
#endif
#if (((COM_IDLE_TYPE   == TIMER_IDLE ) && (COM_IDLE_TIMER   == SOFT))  || (COM_IDLE_TYPE   == GENI_IRQ_IDLE))
  EXTERN void GeniCOMIdleTimerIrq(void);                      // for COM channel
#endif
#if (((USB_IDLE_TYPE   == TIMER_IDLE ) && (USB_IDLE_TIMER   == SOFT))  || (USB_IDLE_TYPE   == GENI_IRQ_IDLE))
  EXTERN void GeniUSBIdleTimerIrq(void);                      // for USB channel
#endif

// only used for MIPS processor and PC applications
#if ((defined MIPS) || (defined __PC__))
// Common interrupt handler
EXTERN BIT GENIIrqHandler(UCHAR channels_disabled);
// Common idle detection routine
EXTERN BIT GENIIdleDetection(void);
// for BUS channel
EXTERN void  GeniBUSOutIrq(void);
EXTERN void  GeniBUSInIrq(void);
EXTERN void  GeniBUSLineIdleIrq(void);
EXTERN void  GeniBUSLineBusyIrq(void);
// for COM channel
EXTERN void  GeniCOMOutIrq(void);
EXTERN void  GeniCOMInIrq(void);
EXTERN void  GeniCOMLineIdleIrq(void);
EXTERN void  GeniCOMLineBusyIrq(void);
// for RS232 channel
EXTERN void  GeniRS232OutIrq(void);
EXTERN void  GeniRS232InIrq(void);
EXTERN void  GeniRS232LineIdleIrq(void);
EXTERN void  GeniRS232LineBusyIrq(void);
#endif
/****************************************************************************/
/* functions used for Slave applications                                    */
/****************************************************************************/
EXTERN void  ClassAcc(UCHAR cl, UCHAR opr, UCHAR en_dis);

// only used when class 8 enabled
EXTERN void  SetDumpStatus(UCHAR status);
EXTERN void  VirSetDumpStatus(UCHAR status);

// only used when class 10 enabled
EXTERN void  SetObjectStatus(UCHAR status);
EXTERN void  VirSetObjectStatus(UCHAR status);

// only used when virtual slave is enabled
EXTERN void  ClearSlaveList(void);
EXTERN UCHAR RemoveSlave(UCHAR addr);
EXTERN UCHAR InsertSlave(UCHAR addr);
EXTERN UCHAR FindSlave(UCHAR addr);

/****************************************************************************/
/* functions used for MP Master applications - se also slave functions      */
/****************************************************************************/
EXTERN void  EnableMasterMode(void);
EXTERN void  EnableSlaveMode(void);
EXTERN UCHAR ReadyDirAPDU(void);
EXTERN UCHAR SendDirAPDU(UCHAR resp_code, UCHAR *ap, UCHAR da);
EXTERN UCHAR InsertUnit(UCHAR addr, GENI_DEVICE_TYPE device);
EXTERN UCHAR FindUnit(UCHAR addr);
EXTERN UCHAR RemoveUnit(UCHAR addr);
EXTERN void  ClearNetworkList(void);

/****************************************************************************/
/* functions used for PP Master applications                                */
/****************************************************************************/
EXTERN void  SetDestAddr(UCHAR);
EXTERN UCHAR SetDataItem(UCHAR, UCHAR, void *);
EXTERN UCHAR SetDataBlock(UCHAR, UCHAR, void *);
EXTERN void  SetDataEnd(void);
EXTERN UCHAR IsChannelReady(void);

#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
