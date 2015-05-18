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
/* MODULE NAME      :   debug.h                                             */
/*                                                                          */
/* FILE NAME        :   debug.h                                             */
/*                                                                          */
/* FILE DESCRIPTION :  Geni debug file                                      */
/*                                                                          */
/****************************************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H
#include "typedef.h"
#include "geni_cnf.h"

/*****************************************************************************/
/*                       Debug tools                                         */
/*****************************************************************************/
  #define DEBUG  FALSE

  #if (DEBUG == TRUE)
    #include "microp.h"
    #define BUF_SIZE                              1                        // size of debug buffer
    #define DEBUG_BUF_TYPE                        UCHAR
    #define DEBUG_GENI_WARNING(warning)           geni_warning |= warning;
    #define DEBUG_INC_CRC_ERRORS                  crc_errors++;
    #define DEBUG_INC_DATA_ERRORS                 data_errors++;
    #define DEBUG_INC_MASTER_TIMEOUT_ERRORS       {master_timeout_errors = all_master_errors - crc_errors - data_errors - rx_break_errors;}
    #define DEBUG_INC_SLAVE_TIMEOUT_ERRORS        slave_timeout_errors++;
    #define DEBUG_INC_RX_BREAK_ERRORS             rx_break_errors++;
    #define DEBUG_INC_TX_CNT                      {tx_tgm_cnt++;}
    #define DEBUG_INC_RX_CNT                      {rx_tgm_cnt++;}
    #define ADD_2_DEBUG_BUF(a)                    {\
                                                   GENI_DISABLE_GLOBAL_INT();         \
                                                   rx_all_buf[rx_cnt++] = a;          \
                                                   rx_cnt = (rx_cnt) % (BUF_SIZE-5);  \
                                                   GENI_ENABLE_GLOBAL_INT();          \
                                                  }

    EXTERN ULONG tx_tgm_cnt;
    EXTERN ULONG rx_tgm_cnt;
    EXTERN UINT rx_cnt;
    EXTERN UINT geni_warning;                                  // count the number of geni runtime warnings
    EXTERN UINT crc_errors;                                    // count the number of CRC errors
    EXTERN UINT data_errors;                                   // count the number of data errors
    EXTERN UINT master_timeout_errors;                         // count the number of timeout errors
    EXTERN UINT slave_timeout_errors;                          // count the number of timeout errors
    EXTERN UINT rx_break_errors;                               // count the number of receive break errors
    EXTERN DEBUG_BUF_TYPE rx_all_buf[BUF_SIZE];


  #else
    #define DEBUG_GENI_WARNING(warning)
    #define DEBUG_INC_CRC_ERRORS
    #define DEBUG_INC_DATA_ERRORS
    #define DEBUG_INC_MASTER_TIMEOUT_ERRORS
    #define DEBUG_INC_SLAVE_TIMEOUT_ERRORS
    #define DEBUG_INC_RX_BREAK_ERRORS
    #define DEBUG_INC_TX_CNT
    #define DEBUG_INC_RX_CNT
    #define BUF_SIZE
    #define ADD_2_DEBUG_BUF(a)

    EXTERN const ULONG tx_tgm_cnt;
    EXTERN const ULONG rx_tgm_cnt;
    EXTERN const UINT geni_warning;
    EXTERN const UINT crc_errors;
    EXTERN const UINT data_errors;
    EXTERN const UINT master_timeout_errors;
    EXTERN const UINT slave_timeout_errors;
    EXTERN const UINT rx_break_errors;

  #endif
/*****************************************************************************/
/*                       geni_warnings                                       */
/*****************************************************************************/
#define GENI_WARNING_EVENT_QUEUE_OVERRUN        1
#define GENI_WARNING_EXT_EVENT_QUEUE_OVERRUN    2
#define GENI_WARNING_WRITE_TO_CONST             4

#endif
