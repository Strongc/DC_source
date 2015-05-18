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
/* MODULE NAME      :   crc.h                                               */
/*                                                                          */
/* FILE NAME        :   crc.h                                               */
/*                                                                          */
/* FILE DESCRIPTION :   Interface to CRC/SUM routines for GENI              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _CRC_H
#define _CRC_H

#ifndef _COMMON_H_
  #include "common.h"         // common.h and geni_cnf.h definitions must be specified
#endif

/*****************************************************************************/
/* From channel Genipro channel selection, include necessary check functions */
/*****************************************************************************/
#if ( ((CTO_BUS_TYPE != Disable)   && ( BUS_USE_CRC16_CHECK == TRUE)) \
   || ((CTO_PLM_TYPE != Disable)   && ( PLM_USE_CRC16_CHECK == TRUE)) \
   || ((CTO_COM_TYPE != Disable)   && ( COM_USE_CRC16_CHECK == TRUE)) \
   || ((CTO_IR_TYPE != Disable)    && ( IR_USE_CRC16_CHECK == TRUE))  \
   || ((CTO_RS232_TYPE != Disable) && ( RS232_USE_CRC16_CHECK == TRUE)))
  #define GENI_CRC_16
#endif

#if ( ((CTO_BUS_TYPE != Disable)   && ( BUS_USE_CRC16_CHECK == FALSE)) \
   || ((CTO_PLM_TYPE != Disable)   && ( PLM_USE_CRC16_CHECK == FALSE)) \
   || ((CTO_COM_TYPE != Disable)   && ( COM_USE_CRC16_CHECK == FALSE)) \
   || ((CTO_IR_TYPE != Disable)    && ( IR_USE_CRC16_CHECK == FALSE))  \
   || ((CTO_RS232_TYPE != Disable) && ( RS232_USE_CRC16_CHECK == FALSE)))
  #define GENI_SUM_8
#endif

//
//  #if( CTO_xxx == Enable )       No current channels need CRC_8
//    #define GENI_CRC_8
//  #endif
//

/*****************************************************************************/
/* Interface for CRC16                                                       */
/*****************************************************************************/
#ifdef GENI_CRC_16
    #define CRC_16_START()     crc_16_value = 0xffff;
    #define CRC_16_END()       crc_16_value = ~crc_16_value;
    extern UCHAR CheckCRC16(UCHAR *buf_st );
    extern void  CalcCRC16(UCHAR *buf_st );
#endif

/*****************************************************************************/
/* Interface for CRC8                                                        */
/*****************************************************************************/
#ifdef GENI_CRC_8
    #define CRC_8_START()     crc_8_value = 0xff;
    #define CRC_8_END()       crc_8_value = ~crc_8_value;
    extern UCHAR CheckCRC8(UCHAR *buf_st );
    extern void  CalcCRC8(UCHAR *buf_st );
#endif

/*****************************************************************************/
/* Interface for SUM8                                                        */
/*****************************************************************************/
#ifdef GENI_SUM_8
    extern UCHAR CheckSUM8( UCHAR *buf_st);
    extern void CalcSUM8( UCHAR *buf_st);
#endif

#endif

