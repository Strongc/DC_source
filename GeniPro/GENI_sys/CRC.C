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
/* MODULE NAME      :  Crc.c                                                */
/*                                                                          */
/* FILE NAME        :  Crc.c                                                */
/*                                                                          */
/* FILE DESCRIPTION :  Contains the checksum/sum routines used in GENI      */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "geni_cnf.h"              /* Access to GENIpro configuration        */
#include "common.h"                /* Access to common definitions */
#include "profiles.h"              /* Access to channel profiles */
#include "crc.h"

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
#ifdef GENI_CRC_16

#if (CRC_OPTIMIZE==Memory)

   const UCHAR crc_table_low[] =
       {
       0x00, 0x21, 0x42, 0x63,
       0x84, 0xA5, 0xC6, 0xE7,
       0x08, 0x29, 0x4A, 0x6B,
       0x8C, 0xAD, 0xCE, 0xEF
       };

   const UCHAR crc_table_high[] =
       {
       0x00, 0x10, 0x20, 0x30,
       0x40, 0x50, 0x60, 0x70,
       0x81, 0x91, 0xA1, 0xB1,
       0xC1, 0xD1, 0xE1, 0xF1
       };

#else

   const UINT crc_tab[256] =
       {
       0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
       0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
       0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
       0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
       0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
       0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
       0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
       0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
       0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
       0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
       0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
       0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
       0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
       0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
       0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
       0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
       0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
       0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
       0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
       0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
       0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
       0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
       0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
       0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
       0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
       0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
       0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
       0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
       0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
       0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
       0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
       0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
       };

#endif          // end if CRC_OPTIMIZE
#endif          // end ifdef GENI_CRC_16

#ifdef GENI_CRC_8
#if (CRC_OPTIMIZE == Memory)

   //
   // This table is simply the first 16 entries from the big table..
   //
   const UCHAR crc8_table[] =
   {
      0x00,0x2F,0x5E,0x71,
      0xBC,0x93,0xE2,0xCD,
      0x57,0x78,0x09,0x26,
      0xEB,0xC4,0xB5,0x9A
   };
  #if (SEGMENT_CHANGE_ALLOWED == TRUE)
    #pragma memory=dataseg(GENI_RAM)
  #endif

  static UCHAR idx8;                     // only used in memory optimize
  #if (SEGMENT_CHANGE_ALLOWED == TRUE)
    #pragma memory=default
  #endif

#else

   const UCHAR crc8_table[] = {
      0x00,0x2F,0x5E,0x71,0xBC,0x93,0xE2,0xCD,0x57,0x78,0x09,0x26,
      0xEB,0xC4,0xB5,0x9A,0xAE,0x81,0xF0,0xDF,0x12,0x3D,0x4C,0x63,
      0xF9,0xD6,0xA7,0x88,0x45,0x6A,0x1B,0x34,0x73,0x5C,0x2D,0x02,
      0xCF,0xE0,0x91,0xBE,0x24,0x0B,0x7A,0x55,0x98,0xB7,0xC6,0xE9,
      0xDD,0xF2,0x83,0xAC,0x61,0x4E,0x3F,0x10,0x8A,0xA5,0xD4,0xFB,
      0x36,0x19,0x68,0x47,0xE6,0xC9,0xB8,0x97,0x5A,0x75,0x04,0x2B,
      0xB1,0x9E,0xEF,0xC0,0x0D,0x22,0x53,0x7C,0x48,0x67,0x16,0x39,
      0xF4,0xDB,0xAA,0x85,0x1F,0x30,0x41,0x6E,0xA3,0x8C,0xFD,0xD2,
      0x95,0xBA,0xCB,0xE4,0x29,0x06,0x77,0x58,0xC2,0xED,0x9C,0xB3,
      0x7E,0x51,0x20,0x0F,0x3B,0x14,0x65,0x4A,0x87,0xA8,0xD9,0xF6,
      0x6C,0x43,0x32,0x1D,0xD0,0xFF,0x8E,0xA1,0xE3,0xCC,0xBD,0x92,
      0x5F,0x70,0x01,0x2E,0xB4,0x9B,0xEA,0xC5,0x08,0x27,0x56,0x79,
      0x4D,0x62,0x13,0x3C,0xF1,0xDE,0xAF,0x80,0x1A,0x35,0x44,0x6B,
      0xA6,0x89,0xF8,0xD7,0x90,0xBF,0xCE,0xE1,0x2C,0x03,0x72,0x5D,
      0xC7,0xE8,0x99,0xB6,0x7B,0x54,0x25,0x0A,0x3E,0x11,0x60,0x4F,
      0x82,0xAD,0xDC,0xF3,0x69,0x46,0x37,0x18,0xD5,0xFA,0x8B,0xA4,
      0x05,0x2A,0x5B,0x74,0xB9,0x96,0xE7,0xC8,0x52,0x7D,0x0C,0x23,
      0xEE,0xC1,0xB0,0x9F,0xAB,0x84,0xF5,0xDA,0x17,0x38,0x49,0x66,
      0xFC,0xD3,0xA2,0x8D,0x40,0x6F,0x1E,0x31,0x76,0x59,0x28,0x07,
      0xCA,0xE5,0x94,0xBB,0x21,0x0E,0x7F,0x50,0x9D,0xB2,0xC3,0xEC,
      0xD8,0xF7,0x86,0xA9,0x64,0x4B,0x3A,0x15,0x8F,0xA0,0xD1,0xFE,
      0x33,0x1C,0x6D,0x42
   };

#endif                          //  end else (CRC_OPTIMIZE == Memory)
#endif                          //  end ifdef GENI_CRC_8
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
/* P R O T O T Y P E S                                                      */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* prototypes for GENI_CRC_16                                               */
/****************************************************************************/
#ifdef GENI_CRC_16
    UCHAR CheckCRC16(UCHAR *buf_st );
    void  CalcCRC16(UCHAR *buf_st );
    static void crc_16_update(UCHAR d);
    #if (SEGMENT_CHANGE_ALLOWED == TRUE)
      #pragma memory=dataseg(GENI_RAM)
    #endif

    static UINT crc_16_value;
    #if (SEGMENT_CHANGE_ALLOWED == TRUE)
        #pragma memory=default
    #endif
#endif // end ifdef GENI_CRC_16

/****************************************************************************/
/* prototypes for GENI_CRC_8                                                */
/****************************************************************************/
#ifdef GENI_CRC_8
    UCHAR CheckCRC8(UCHAR *buf_st );
    void  CalcCRC8(UCHAR *buf_st );


    #if (SEGMENT_CHANGE_ALLOWED == TRUE)
      #pragma memory=dataseg(GENI_RAM)
    #endif
    static UCHAR crc_8_value;
    #if (SEGMENT_CHANGE_ALLOWED == TRUE)
      #pragma memory=default
    #endif
#endif   // end ifdef GENI_CRC_8

/****************************************************************************/
/* prototypes for GENI_SUM_8                                                */
/****************************************************************************/
#ifdef GENI_SUM_8
    extern UCHAR CheckSUM8( UCHAR *buf_st);
    extern void CalcSUM8( UCHAR *buf_st);
#endif // end ifdef GENI_SUM_8

/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
#ifdef GENI_CRC_16
/*****************************************************************************/
/*                    CRC_16 implementations                                 */
/*****************************************************************************/
#if (CRC_OPTIMIZE==Memory)
/****************************************************************************
*     Name      : calc_crc16_nibble                                         *
*               :                                                           *
*     Inputs    : nibble                                                    *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   : crc_16_value                                              *
*     Returns   : None                                                      *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void calc_crc16_nibble(UCHAR nibble)
{
  static UCHAR idx;

  idx = (HI_val(crc_16_value) >> 4) ^ nibble;
  crc_16_value <<= 4;
  LO_val(crc_16_value) ^= crc_table_low[idx];
  HI_val(crc_16_value) ^= crc_table_high[idx];
}
/****************************************************************************
*     Name      : crc_16_update                                             *
*               :                                                           *
*     Inputs    : d                                                         *
*               :                                                           *
*     Outputs   : none                                                      *
*     Updates   : none                                                      *
*     Returns   : None                                                      *
*               :                                                           *
*   Description :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void crc_16_update(UCHAR d)
{
  calc_crc16_nibble(d >> 4);
  calc_crc16_nibble(d & 0x0f);
}

#else                                       // end if (CRC_OPTIMIZE==Memory)
void crc_16_update(UCHAR d)
{
  crc_16_value = (crc_16_value<<8 ) ^ crc_tab[(UCHAR)((crc_16_value>>8) ^ d)];
}
#endif                                      // end else (CRC_OPTIMIZE==Memory)
/****************************************************************************
*     Name      : CheckCRC16                                                *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : Return TRUE if CRC is OK else FALSE                       *
*               :                                                           *
*   Description : Calculate the checksum for the entire PDU and compare     *
*               : with the checksum stored.                                 *
*               : Note the PDU header must be fully defined ( Length field) *
*---------------------------------------------------------------------------*/
UCHAR CheckCRC16(UCHAR *buf_st)
{
  UCHAR i,c;
  UCHAR *d_p;

  d_p = buf_st+1;                               // CRC is calculated from and include the length specifier
  c   = (*d_p)+1;                               // the length specifier, number of bytes in tgm from lenght exclude crc
  CRC_16_START();                               // initialise CRC word
  for( i = 0; i < c; i++)
    crc_16_update( *(d_p++));
  CRC_16_END();

  c  = ( *(d_p++) == HI_val(crc_16_value) );    // compare
  c &= ( *(d_p)   == LO_val(crc_16_value) );
  return c;                                     // return result
}
/****************************************************************************
*     Name      : CalcCRC16                                                 *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   : telegram with CRC16 word                                  *
*     Returns   : none                                                      *
*               :                                                           *
*   Description : Generate the checksum for the entire PDU and append it    *
*               :                                                           *
*               : Note the PDU header must be fully defined ( Length field) *
*---------------------------------------------------------------------------*/
void CalcCRC16(UCHAR *buf_st)
{
  UCHAR i,c;
  UCHAR *d_p;

  d_p = buf_st+1;                            // where to start
  c   = (*d_p)+1;                            // number of bytes in
                                             // calculation ( incl length )
  CRC_16_START();                            // set start
  for( i = 0; i < c; i++)
    crc_16_update( *(d_p++));
  CRC_16_END();                              // reverse

  *(d_p++) = HI_val(crc_16_value);           // save HI byte first
  *(d_p)   = LO_val(crc_16_value);           // and then low byte
}
#endif // end ifdef GENI_CRC_16

#ifdef GENI_CRC_8
/*****************************************************************************/
/*                    CRC_8 implementations                                  */
/*****************************************************************************/
#if (CRC_OPTIMIZE==Memory)

   #define calc_crc8_nibble(nibble) \
                  { idx8 = (crc_8_value >> 4) ^ nibble; \
                    crc_8_value <<= 4; \
                    crc_8_value ^= crc8_table[idx8]; }

   #define crc8_update(data) \
                  { calc_crc8_nibble(data >> 4); \
                    calc_crc8_nibble(data & 0x0f); }

#else

   #define crc8_update(data) crc_8_value = crc8_table[(UCHAR)crc_8_value^data]

#endif
/****************************************************************************
*     Name      : CheckCRC8                                                 *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : Return TRUE if CRC is OK else FALSE                       *
*               :                                                           *
*   Description : Calculate the checksum for the entire PDU and compare     *
*               : with the checksum stored.                                 *
*               : Note the PDU header must be fully defined ( Length field) *
*---------------------------------------------------------------------------*/
UCHAR CheckCRC8(UCHAR *buf_st)
{

  UCHAR i,len;
  UCHAR *index_ptr;

  index_ptr = buf_st+1;                   // CRC is calculated from and include the length specifier
  len   = (*index_ptr)+1;                 // number of bytes in tgm exclude SD and CRC
  CRC_8_START();                          // initialise CRC byte
  for( i = 0; i < len; i++)
    crc8_update( *(index_ptr++) );
  CRC_8_END();                            // Reverse CRC byte

  return ( *(index_ptr) == crc_8_value ); // Compare and return result
}
/****************************************************************************
*     Name      : CalcCRC8                                                  *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   : telegram with CRC8 byte                                   *
*     Returns   : none                                                      *
*               :                                                           *
*   Description : Generate the checksum for the entire PDU and append it    *
*               :                                                           *
*               : Note the PDU header must be fully defined ( Length field) *
*---------------------------------------------------------------------------*/
void CalcCRC8(UCHAR *buf_st)
{
  UCHAR i,len;
  UCHAR *index_ptr;

  index_ptr = buf_st+1;                      // where to start, exclude SD
  len = (*index_ptr)+1;                      // number of bytes in calculation ( incl length )

  CRC_8_START();                             // Initialise CRC byte
  for( i = 0; i < len; i++)
    crc8_update( *(index_ptr++));
  CRC_8_END();                               // Reverse CRC byte

  *index_ptr = crc_8_value;                  // Store CRC8 in tgm
}
#endif                                       // end ifdef GENI_CRC_8

#ifdef GENI_SUM_8
/*****************************************************************************/
/*                    SUM_8 implementations                                  */
/*****************************************************************************/
/****************************************************************************
*     Name      : CheckSUM8                                                 *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : TRUE if check sum is OK else FALSE                        *
*               :                                                           *
*   Description : Calculate check sum and compare the result with the       *
*               : check sum in the telegram                                 *
*               : The check sum is a simple 8 bit added sum of the telegram *
*               : excluding the Start Delimiter                             *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR CheckSUM8( UCHAR *buf_st)
{
   UCHAR len, i, cs = 0;

   len = buf_st[iLN] + 1;               // Length including the length field

   for ( i=1; i<=len; i++)              // Exclude SD
   {
      cs += buf_st[i];
   }

  return ( buf_st[i] == cs );           // Compare and return result
}
/****************************************************************************
*     Name      : CalcSUM8                                                  *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Calculate checksum and add it to the end the telegram     *
*               : stored in com_ch_transmit_buf                             *
*               : The check sum is a simple 8 bit added sum of the telegram *
*               : excluding the Start Delimiter                             *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void CalcSUM8( UCHAR *buf_st)
{
   UCHAR len, i, cs = 0;

   len = buf_st[iLN] + 1;               // Length including the length field

   for ( i=1; i<=len; i++)              // Exclude SD
   {
      cs += buf_st[i];
   }

   buf_st[i] = cs;                      // store the calculated sum
}

#endif                                  //  end ifdef GENI_SUM_8
/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
