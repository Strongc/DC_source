/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIPro                                         */
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
/* MODULE NAME      : vir_dia_slave.h                                       */
/*                                                                          */
/* FILE NAME        : vir_dia_slave.h                                       */
/*                                                                          */
/* FILE DESCRIPTION : interface file for vir_dia_slave.c                    */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_DIA_SLAVE_H_
  #define _VIR_DIA_SLAVE_H
#include "geni_if.h"                    // Access to Geni interface

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/
#if((CTO_CLASS_9 == Enable) && (VIR_CLASS_9 == Enable))
  #include "vir_cl_9_tab.h"                                // access to virtual class 9 buffers
#else
  #define CTO_VIR_CLASS_8        Disable                   // virtual class 8 can't be enabled
  #define CTO_VIR_CLASS_10       Disable                   // virtual class 10 can't be enabled
  #define CTO_VIR_CLASS_16_BIT   Disable                   // virtual 16 bit classes can't be enabled
  #define CTO_VIR_CLASS_32_BIT   Disable                   // virtual 32 bit classes can't be enabled
#endif

#if ( CTO_CLASS_8 == Enable )
  #define HIGH_MEMORY_ID  1                                // memory blocks PDU's
#else
  #define HIGH_MEMORY_ID  0
#endif

// Dummy defines if buffers not used
#if ( CTO_CLASS_9 == Enable )
  #define HIGH_ROUTE_ID  1
#else
  #define HIGH_ROUTE_ID  0
  #define routing_buf    0
#endif

#if ( CTO_CLASS_10 != Enable )
  #define object_buf    0
#endif

#if ( CONF_BUF_LEN == 0 )
  #define conf_buf      0
#endif

#if ( CMD_BUF_LEN == 0 )
  #define cmd_buf       0
#endif

#if ( REF_BUF_LEN == 0 )
  #define ref_buf       0
#endif

#if ( ASCII_BUF_LEN == 0 )
  #define ascii_buf       0
#endif

#if ( CONF16_BUF_LEN == 0 )
  #define conf16_buf      0
#endif

#if ( REF16_BUF_LEN == 0 )
  #define ref16_buf       0
#endif

#if ( CONF32_BUF_LEN == 0 )
  #define conf32_buf      0
#endif

#if ( REF32_BUF_LEN == 0 )
  #define ref32_buf       0
#endif


/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
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

EXTERN UINT  operation_acc_ctr;                                   // -7-6 -5-4 -3-2 -1-0 Class  // SGSG SGSG SGSG SGSG operation
EXTERN UINT  operation_acc_ctr_high;                              // 15-14-13-12-11-10-9-8 Class// SGSG SGSG SGSG SGSG operation
EXTERN UINT  operation_acc_ctr_high_h;                            // 16 Class      // 0000 0000 0000 00SG operation

#if (USE_VIRTUAL_SLAVES == TRUE)
  // Virtual slave structure
  typedef struct  {  UCHAR unit_addr;
                     UCHAR polled;
                     UCHAR connect_addr;
                     UCHAR connect_timer;
                     UCHAR timer_enabled;
                  }  SUBSLAVE_UNIT_RECORD;

  extern  UCHAR cur_unit;                                          // current unit being processed
  extern  SUBSLAVE_UNIT_RECORD vir_slave_list[MAX_VIR_SLAVE_COUNT];// unit list
#endif

extern const PROTAB   *pre_tab_ptr;

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
extern void VirInterprete(UCHAR receive_cnt, UCHAR i_pdu_body);
extern void VirResetInterpreter(void);

#if (USE_VIRTUAL_SLAVES == TRUE)
extern void VirSlaveConnectTmOut(void);
extern UCHAR FindNext2Connect(void);
extern void VirSlavePollUpdate(void);
#endif

#endif // #ifdef _VIR_DIA_SLAVE_H_
