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
/* MODULE NAME      :   common.h                                            */
/*                                                                          */
/* FILE NAME        :   common.h                                            */
/*                                                                          */
/* FILE DESCRIPTION :  General Definitions Header File                      */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _COMMON_H_
  #define _COMMON_H_

#include "typedef.h"
#include "geni_cnf.h"
#include "debug.h"

/*****************************************************************************/
/*                       Geni version definitions                            */
/*****************************************************************************/
#define GENI_VERSION_STR  "GENIpro V06.03.00"
#define GENI_VERSION_VAL  0x060300

/*****************************************************************************/
/*     Compatibility defines for Geni_cnf.h to support v06.yy.xx             */
/*****************************************************************************/
#if ( GENI_VERSION_VAL < 0x070000)

#else
  #error 'Remove these compatibility defines to V06.00.00 for Geni_cnf.h'
#endif
/*****************************************************************************/
/*                       Common definitions                                  */
/*****************************************************************************/
#ifndef Enable
 #define Enable               1
#endif
#ifndef Disable
 #define Disable              0
#endif

#define HIGH_LEVEL            1
#define LOW_LEVEL             0

#define  GENI_EMB_OS          1
#define  GENI_GF_OS_TIMER0    2
#define  GENI_GF_OS_USER_IRQ  3
#define  GENI_GF_OS_USER_DEF  4

#define Slave                 1    // Master_unit_fl = 0
#define Master                2    // Master_unit_fl = 1

#define GENI_NOT_USED         255

#define NO_RESP_FNC           0           // reserved index used to send messages



/*****************************************************************************/
/*                       Structure for network list                          */
/*****************************************************************************/
typedef enum
{
  NO_DEVICES  = 0x0000,
  DEVICE1     = 0x0001,
  DEVICE2     = 0x0002,
  DEVICE3     = 0x0004,
  DEVICE4     = 0x0008,
  DEVICE5     = 0x0010,
  DEVICE6     = 0x0020,
  DEVICE7     = 0x0040,
  DEVICE8     = 0x0080,
  DEVICE9     = 0x0100,
  DEVICE10    = 0x0200,
  DEVICE11    = 0x0400,
  DEVICE12    = 0x0800,
  DEVICE13    = 0x1000,
  DEVICE14    = 0x2000,
  DEVICE15    = 0x4000,
  DEVICE16    = 0x8000,
  ALL_DEVICES = 0xFFFF
} GENI_DEVICE_TYPE;

typedef struct  {  UCHAR unit_addr;
                   UINT unit_err_rate;
                   UCHAR bus_errors;
                   GENI_DEVICE_TYPE device_type;
                }  UNIT_RECORD;


/*****************************************************************************/
/*                       Devices for networklist                             */
/*****************************************************************************/

/*****************************************************************************/
/*                          Event structure                                  */
/*****************************************************************************/
#define CH_INFO    0xC0    // bit 7,6 for channel information
#define EV_INFO    0x3F    // bit 5,4,3,2,1,0 for event information

/*****************************************************************************/
/* Framning size                                                             */
/*****************************************************************************/
#define HEADER_LEN                         4            // Length of the DLL header: SD, LEN, DA and SA
#define CRC16_FRAME_SIZE                   6
#define SUM8_FRAME_SIZE                    5

/*****************************************************************************/
/*  Definition of channel information in events, None are fixed              */
/*****************************************************************************/
// DO NOT modify the order of the defines !!
#if(CTO_IR_TYPE != Disable)
  #define  IR_CH   0                    // Use Ir
#else
  #define  IR_CH   -0x40
#endif

#if(CTO_BUS_TYPE != Disable)            // Use Bus
  #define BUS_CH  IR_CH + 0x40
#else
  #define BUS_CH  IR_CH
#endif

#if(CTO_PLM_TYPE != Disable)            // Use Powerline
  #define PLM_CH  BUS_CH + 0x40
#else
  #define PLM_CH  BUS_CH
#endif

#if(CTO_COM_TYPE != Disable)            // Use Com channel
  #define COM_CH  PLM_CH + 0x40
#else
  #define COM_CH  PLM_CH
#endif

#if(CTO_RS232_TYPE != Disable)          // Use RS232 channel
  #define RS232_CH  COM_CH + 0x40
#else
  #define RS232_CH  COM_CH
#endif

#ifdef TCS_USB_SER_PORT
	#if(CTO_USB_TYPE != Disable)          // Use RS232 channel
	  #define USB_CH  RS232_CH + 0x40
	#else
	  #define USB_CH  RS232_CH
	#endif
	#define MAX_CH_NO      USB_CH         // Highest channel number
#else
#define MAX_CH_NO      RS232_CH         // Highest channel number
#endif

// Define which channel are master
#if CTO_IR_TYPE == Master
  #define MAS_CH              IR_CH
  #define MAS_CH_INDX         IR_CH >> 6
  #define MAS_BUF_LEN         IR_DF_buf_len
  #define MAS_MAX_RETRIES     IR_MAX_RTY
  #define MAS_BAUDRATE        1707 // see the GENI application programmers manual - appendix D.2 - chapter D2.3 - equation D2.8
  #if (IR_USE_CRC16_CHECK == TRUE)
    #define MAS_FRAMING_SIZE  CRC16_FRAME_SIZE
  #else
    #define MAS_FRAMING_SIZE  SUM8_FRAME_SIZE
  #endif

#elif CTO_BUS_TYPE == Master
  #define MAS_CH              BUS_CH
  #define MAS_CH_INDX         BUS_CH >> 6
  #define MAS_BUF_LEN         BUS_DF_buf_len
  #define MAS_MAX_RETRIES     BUS_MAX_RTY
  #define MAS_BAUDRATE        BUS_BAUDRATE
  #if (BUS_USE_CRC16_CHECK == TRUE)
    #define MAS_FRAMING_SIZE  CRC16_FRAME_SIZE
  #else
    #define MAS_FRAMING_SIZE  SUM8_FRAME_SIZE
  #endif

#elif CTO_RS232_TYPE == Master
  #define MAS_CH              RS232_CH
  #define MAS_CH_INDX         RS232_CH >> 6
  #define MAS_BUF_LEN         RS232_DF_buf_len
  #define MAS_MAX_RETRIES     RS232_MAX_RTY
  #define MAS_BAUDRATE        RS232_BAUDRATE
  #if (RS232_USE_CRC16_CHECK == TRUE)
    #define MAS_FRAMING_SIZE  CRC16_FRAME_SIZE
  #else
    #define MAS_FRAMING_SIZE  SUM8_FRAME_SIZE
  #endif
#elif CTO_PLM_TYPE == Master
  #define MAS_CH              PLM_CH
  #define MAS_CH_INDX         PLM_CH >> 6
  #define MAS_BUF_LEN         PLM_DF_buf_len
  #define MAS_MAX_RETRIES     PLM_MAX_RTY
  #define MAS_BAUDRATE        PLM_BAUDRATE
  #if (PLM_USE_CRC16_CHECK == TRUE)
    #define MAS_FRAMING_SIZE  CRC16_FRAME_SIZE
  #else
    #define MAS_FRAMING_SIZE  SUM8_FRAME_SIZE
  #endif
#elif CTO_COM_TYPE == Master
  #define MAS_CH              COM_CH
  #define MAS_CH_INDX         COM_CH >> 6
  #define MAS_BUF_LEN         COM_DF_buf_len
  #define MAS_MAX_RETRIES     COM_MAX_RTY
  #define MAS_BAUDRATE        COM_BAUDRATE
  #if (COM_USE_CRC16_CHECK == TRUE)
    #define MAS_FRAMING_SIZE  CRC16_FRAME_SIZE
  #else
    #define MAS_FRAMING_SIZE  SUM8_FRAME_SIZE
  #endif
#elif CTO_USB_TYPE == Master
	#define MAS_CH				USB_CH
	#define MAS_CH_INDX 		USB_CH >> 6
	#define MAS_BUF_LEN 		USB_DF_buf_len
	#define MAS_MAX_RETRIES 	
	#define MAS_BAUDRATE		
	#if (USB_USE_CRC16_CHECK == TRUE)
	 #define MAS_FRAMING_SIZE	CRC16_FRAME_SIZE
	#else
	  #define MAS_FRAMING_SIZE	SUM8_FRAME_SIZE
	#endif
#else
  #define MAS_CH_INDX    0xFF
#endif

// Define array index's for the channels
#define IR_CH_INDX          IR_CH >> 6
#define BUS_CH_INDX         BUS_CH >> 6
#define PLM_CH_INDX         PLM_CH >> 6
#define MDM_CH_INDX         MDM_CH >> 6
#define COM_CH_INDX         COM_CH >> 6
#define RS232_CH_INDX       RS232_CH >> 6
#define VIR_SLAVE_CH_INDX   VIR_SLAVE_CHANNEL >> 6
#ifdef TCS_USB_SER_PORT
#define USB_CH_INDX			USB_CH >> 6
// Define the number of channels
#define NO_OF_CHANNELS     (USB_CH_INDX) + 1
#else
// Define the number of channels
#define NO_OF_CHANNELS  (RS232_CH_INDX) + 1
#endif

#if (NO_OF_CHANNELS > 4)
  #error 'To many channels selected, max 4'
#endif

/*****************************************************************************/
/*  GENI UART Setup values                                                   */
/*  setup param byte definition :                                            */
/*  BIT7 - Modbus               0 : Modbus OFF; 1 :  Modbus ON               */
/*  BIT6 - Baudrate             -                                            */
/*  BIT5 - Baudrate             -                                            */
/*  BIT4 - Baudrate             0 : Configurable baudrate not supported (fixed to 9600) 1 : 1200 baud; 2 : 2400; 3 : 4800; 4 : 9600; 5 : 19200; 6 : 384000; 7-8 : not used yet */
/*  BIT3 - Parity               -                                            */
/*  BIT2 - Parity               0 : no parity; 1 : 0 parity;  2 : Odd parity; 3 : even parity; */
/*  BIT1 - Char len             0 : 7 bits;    1 : 8 bits                    */
/*  BIT0 - Stopbits             0 : 1 StopBit; 1 : 2 StopBits                */
/*****************************************************************************/
#define GENI_DATA_8_BIT              1
#define GENI_DATA_7_BIT              0
#define GENI_DATA_NO_PARITY          0
#define GENI_DATA_0_PARITY           1
#define GENI_DATA_ODD_PARITY         2
#define GENI_DATA_EVEN_PARITY        3
#define GENI_DATA_ONE_STOP_BIT       0
#define GENI_DATA_TWO_STOP_BIT       1
#define GENI_BAUD_NO_CONF            0    // Configurable baudrate not supported (fixed to 9600)
#define GENI_BAUD_1200               1
#define GENI_BAUD_2400               2
#define GENI_BAUD_4800               3
#define GENI_BAUD_9600               4
#define GENI_BAUD_19200              5
#define GENI_BAUD_38400              6
//not used yet #define GENI_BAUD_xxxxx              7
//not used yet #define GENI_BAUD_xxxxx              8

#define GENI_SETUP_PARAM(modbus_on, baud, parity, stop_bits) (((modbus_on << 7) & 0x80) | ((baud << 4) & 0x70) | ((parity << 2) & 0x0C) | (stop_bits & 0x01))
//#define GENI_SETUP_PARAM(modbus_on, baud, parity, data_len, stop_bits) (((modbus_on << 7) & 0x80) | ((baud << 4) & 0x70) | ((parity << 2) & 0x0C) | ((data_len << 1) & 0x02) | (stop_bits & 0x01))
//#define GENI_SETUP_PARAM(modbus_on, parity, data_len, stop_bits) (((modbus_on << 7) & 0x80) | ((parity << 2) & 0x0C) | ((data_len << 1) & 0x02) | (stop_bits & 0x01))

/*****************************************************************************/
/*  GENI idle type                                                           */
/*****************************************************************************/
#define HW_IDLE       0
#define TIMER_IDLE    1
#define GENI_IRQ_IDLE 2
#define NO_IDLE       3

/*****************************************************************************/
/*  GENI idle interrupt sources                                              */
/*****************************************************************************/
#define INTP0                     0
#define INTP1                     1
#define INTP2                     2
#define INTP3                     3
#define INTP4                     4
#define INTP5                     5
#define INTP6                     6
#define INTP7                     7
#define SOFT                      99

/*****************************************************************************/
/* Interrupt priority order for V850                                         */
/*****************************************************************************/
#define INT_PRIO_LEV0         0x00                 // level 0 (highest)
#define INT_PRIO_LEV1         0x01                 // level 1
#define INT_PRIO_LEV2         0x02                 // level 2
#define INT_PRIO_LEV3         0x03                 // level 3
#define INT_PRIO_LEV4         0x04                 // level 4
#define INT_PRIO_LEV5         0x05                 // level 5
#define INT_PRIO_LEV6         0x06                 // level 6
#define INT_PRIO_LEV7         0x07                 // level 7 (lowest)

/*****************************************************************************/
/* Interrupt priority order for K0                                           */
/*****************************************************************************/
#define HIGH_PRIO            0
#define LOW_PRIO             1

/*****************************************************************************/
/*                                                                           */
/*    Genipro telegram specifications:                                       */
/*                                                                           */
/*    index        description:                                              */
/*                                                                           */
/*     0           start delimiter         ( 0x27, 0x26 or 0x24 )            */
/*     1           length specifier        ( n - 4 )                         */
/*     2           destination address                                       */
/*     3           source address                                            */
/*     4           Apdu header,            ( PDU BODY )                      */
/*     5           Apdu opr/ack + length                                     */
/*     ..          apdu data                                                 */
/*     n-2         crc_0                                                     */
/*     n-1         crc_1                   ( last byte in tgm )              */
/*     n           free space                                                */
/*                                                                           */
/*****************************************************************************/
#define iSD       0   // start delimiter position
#define iLN       1   // length specifier position
#define iDA       2   // destination address position
#define iSA       3   // source address position
#define iRA       2   // route address in routing buf

/*****************************************************************************/
/* Addresses reserved for special purpose                                    */
/*****************************************************************************/
#define CONNECTION  0xFE               // Destination address in connection request telegrams
#define BROADCAST   0xFF               // Destination address in Broadcast telegrams

/*****************************************************************************/
/* Start Delimiter identifiers                                               */
/*****************************************************************************/
#define GENI_REPLY    0x24
#define GENI_MESSAGE  0x26
#define GENI_REQUEST  0x27

/*****************************************************************************/
/* Data not avaible definition                                               */
/*****************************************************************************/
extern const ULONG na;
#define NA (void*)&na

/*****************************************************************************/
/* Definements used in Data Item INFO tables                                 */
/*****************************************************************************/
#define Bits_254         0x81   /* 10000001   Bits, 255=data not available          */
#define Bits_255         0xA1   /* 10100001   Bits, 255=valid data                  */
#define Common_info_no   0x40+  /* 01000000+  Value with static scaleinfo.          */
#define Common_ptr_no    0xC0+  /* 11000000+  Value with dynamic scaleinfo.         */
#define Low_ord          0xB0   /* 10110000   Low order byte to 16 bit value        */
#define NI               0x80   /* 10000000   No Scale Information                  */
#define DimLess_254      0x80   /* 10000000   Dimensionless, 255=data not available */
#define DimLess_255      0xA0   /* 10100000   Dimensionless, 255=valid data         */
#define Executable       0xA0   /* 10100000   Command excist                        */
#define Unexecutable     0xAC   /* 10101100   Command does not excist               */

/*****************************************************************************/
/* Definements used in common_info_tab to specify INFO head                  */
/*****************************************************************************/
#define User_acc         0x82    // 10000010 Data Item User Access
#define Serv_acc         0x86    // 10000110 Data Item Service Access Protected
#define Fact_acc         0x8A    // 10001010 Data Item Factory Access Protected
#define Unchange         0x8E    // 10001110 Data Item Unchangable

/*****************************************************************************/
/* Application interface buffers                                             */
/*****************************************************************************/
#define WR_INDX       0       // location of buffer write index
#define RD_INDX       1       // location of buffer read index
#define BUF_START    2       // start of buffer data

/*****************************************************************************/
/* Specifications for an APDU                                                */
/*****************************************************************************/
#define APDU_HEAD     0       // location for the APDU class
#define APDU_LENGHT   1       // location for the APDU lenght/operation
#define APDU_BODY     2       // location for APDU data
#define PDU_BODY      4

/*****************************************************************************/
/* APDU header definitions                                                   */
/*****************************************************************************/
#define APDU_CLL        0x1F  // bit 4-0
#define CI              0x20  // bit 5 USED for PMS2000 according to HAM
                              // bit 6, not used
#define RFS_APDU        0x80  // bit 7 USED Multi master systems

/*****************************************************************************/
/* APDU classes                                                              */
/*****************************************************************************/
#define PRO_APDU      0
#define BUS_APDU      1
#define MEAS_APDU     2
#define CMD_APDU      3
#define CONF_APDU     4
#define REF_APDU      5
#define TEST_APDU     6
#define ASCII_APDU    7
#define MEMORY_APDU   8
#define REROUTE_APDU  9
#define OBJECT_APDU   10
#define MEAS16_APDU   11
#define CONF16_APDU   12
#define REF16_APDU    13
#define MEAS32_APDU   14
#define CONF32_APDU   15
#define REF32_APDU    16

/*****************************************************************************/
/* APDU operations                                                           */
/*****************************************************************************/
#define GET           0x00     // APDU operation GET
#define SET           0x80     // APDU operation SET
#define INFO          0xc0     // APDU operation INFO

/*****************************************************************************/
/* APDU lenght, operation and acknowled                                      */
/*****************************************************************************/
#define APDU_LEN        0x3f   // bit 5-0, lenght field
#define OPR_ACK         0xc0   // bit 7-6, operation and ack. field

/*****************************************************************************/
/* APDU acknowledge codes                                                    */
/*****************************************************************************/
#define AA_OK           0x00   // APDU acknowledge codes
#define A_CLL_ERR       0x40   // Class related error
#define A_ID_ERR        0x80   // ID code related error
#define A_OPR_ERR       0xc0   // Operation error

/*****************************************************************************/
/* RFS structure definitions                                                 */
/*****************************************************************************/
#define RFS_CMD       0x70     // bit 6-4, RFS command field
#define RFS_TIME      0x0f     // bit 3-0, RFS time field
#define BUS_CTRL      0x00     // command for request of bus control

/*****************************************************************************/
/* Info table definitions                                                    */
/*****************************************************************************/
#define INFO_DSC_M      0xC0   // Info descriptor mask
                               // 00xx.xxxx undefined ID-code
#define INFO_HEAD       0x80   // 10xx.xxxx Info head
#define INFO_TAB_INDX   0x40   // 01xx.xxxx Info table index
#define INFO_PTR_INDX   0xC0   // 11xx.xxxx Info pointer table index

#define COM_TAB_INDX    0x3f  // bit 5-0 = index to common_xxx_tab tables
/*****************************************************************************/
/* Application access operations, used in ClassAcc                           */
/*****************************************************************************/
#define GET_ACC       0x01    // access flag for GET
#define SET_ACC       0x02    // access flag for SET

/*****************************************************************************/
/* APDU data size                                                            */
/*****************************************************************************/
#if ( (DF_buf_len - 6) > 63 )
  #define MAX_APDU_DATA   63
#else
  #define MAX_APDU_DATA  (DF_buf_len+10-6)
#endif

/*****************************************************************************/
/* Type definitions                                                          */
/*****************************************************************************/
typedef UCHAR * GENI_ID_PTR_ADDRESSING ID_PTR;

typedef struct  { UCHAR max_id_code;
                  const UCHAR *info_tab;
                  const ID_PTR *item_pointer_tab;
                  void *buf;
                  UCHAR buf_lenght;
                }PROTAB;

typedef struct  { UCHAR info_head;
                  UCHAR unit;
                  UCHAR zero;
                  UCHAR range;
                }INFO_DATA;

typedef struct  { UCHAR option;
                  UCHAR address;
                }RFS;

typedef UCHAR ID_INFO;
typedef UCHAR BUFFER;
typedef UINT BUFFER16;
typedef ULONG BUFFER32;
typedef INFO_DATA * INFO_DATA_PTR;

typedef struct  { const INFO_DATA *info_data;
                  const INFO_DATA_PTR *info_data_ptr;
                }COMMON_INFO_TAB;

/*****************************************************************************/
/* Size and limitations                                                      */
/*****************************************************************************/
#define Min_DF_buf_len      70
#define Max_bus_units       32

/*****************************************************************************/
/* Unit configuration:   Location and intepretation of flags                 */
/*****************************************************************************/
#define Master_unit_fl      1    // bit 0  Master if set
#define ConReply_ON_fl      2    // bit 1  connection reply on if set
#define IR_channel_fl       4    // bit 2  Ir channel enabled if set
#define BUS_channel_fl      8    // bit 3  Bus channel enabled if set
#define RS232_channel_fl    16   // bit 4  Rs232 channel enabled if set
#define PLM_channel_fl      32   // bit 5  Plm channel enabled if set
#define COM_channel_fl      64   // bit 6  Com channel enabled if set
#ifdef TCS_USB_SER_PORT
#define USB_channel_fl      128  // bit 7  USB channel enabled if set
#endif

#define Slave_unit         ( (unit_bus_mode & Master_unit_fl)   == 0)
#define Master_unit        ( (unit_bus_mode & Master_unit_fl)   == 1)
#define ConReply_ON        ( (unit_bus_mode & ConReply_ON_fl)   == ConReply_ON_fl)
#define IR_ch_available    ( (unit_bus_mode & Ir_channel_fl)    == Ir_channel_fl)
#define BUS_ch_available   ( (unit_bus_mode & Bus_channel_fl)   == Bus_channel_fl)
#define RS232_ch_available ( (unit_bus_mode & RS232_channel_fl) == RS232_channel_fl)
#define PLM_ch_available   ( (unit_bus_mode & PLM_channel_fl)   == PLM_channel_fl)
#define COM_ch_available   ( (unit_bus_mode & COM_channel_fl)   == COM_channel_fl)
#ifdef TCS_USB_SER_PORT
#define USB_ch_available   ( (unit_bus_mode & USB_channel_fl)   == USB_channel_fl)
#endif

#define Memory         0       // Used for CRC_OPTIMIZE
#define Speed          1

#define TIME           0       // Used to configure the Master poll interval
#define TICK           1

#define NO_UNIT  0xFF


/*****************************************************************************/
/* Definements for little or big endian                                      */
/*****************************************************************************/
#define LO_HI          0       // Used for indication of little or
#define HI_LO          1       // big endian

#if (INT_STORAGE==LO_HI)
    #define HI_val(x)          ( *((UCHAR *)&x+1) )
    #define LO_val(x)          ( *((UCHAR *)&x)   )
#else
    #define HI_val(x)          ( *((UCHAR *)&x)   )
    #define LO_val(x)          ( *((UCHAR *)&x+1) )
#endif

#if (INT_STORAGE==LO_HI)                    // reference adress for int
    #define HI_pnt(x)          *( x+1 )     // is low order
    #define LO_pnt(x)          *( x )
#else                                       // reference adress for int
    #define HI_pnt(x)          *( x )       // is high order
    #define LO_pnt(x)          *( x+1 )     //
#endif
#if (INT_STORAGE==LO_HI)
    #define HI_val_msb(x)      ( *((UCHAR *)&x+3) )
    #define HI_val_lsb(x)      ( *((UCHAR *)&x+2) )
    #define LO_val_msb(x)      ( *((UCHAR *)&x+1) )
    #define LO_val_lsb(x)      ( *((UCHAR *)&x)   )

#else
    #define HI_val_msb(x)      ( *((UCHAR *)&x)   )
    #define HI_val_lsb(x)      ( *((UCHAR *)&x+1) )
    #define LO_val_msb(x)      ( *((UCHAR *)&x+2) )
    #define LO_val_lsb(x)      ( *((UCHAR *)&x+3) )
#endif

#if (INT_STORAGE==LO_HI)                    // reference adress for int
    #define HI_pnt_msb(x)      *( x+3 )     // is low order
    #define HI_pnt_lsb(x)      *( x+2 )     //
    #define LO_pnt_msb(x)      *( x+1 )     //
    #define LO_pnt_lsb(x)      *( x )
#else                                       // reference adress for int
    #define HI_pnt_msb(x)      *( x )       // is high order
    #define HI_pnt_lsb(x)      *( x+1 )     //
    #define LO_pnt_msb(x)      *( x+2 )     //
    #define LO_pnt_lsb(x)      *( x+3 )     //
#endif

/*****************************************************************************/
/* Definements for securing space in class buffers                           */
/*****************************************************************************/

#if (CTO_BUF_INSERT_CH_INDX == Enable)
  #define ROOM_FOR_1_BYTE     (buf_len+2-1)
  #define ROOM_FOR_ID         (buf_len+2-2)
  #define ROOM_FOR_ID_N_DATA  (buf_len+2-3)
  #define ROOM_FOR_ADDRESS    (buf_len+2-(sizeof(parm_p)+1))

#else
  #define ROOM_FOR_1_BYTE     (buf_len+2-1)
  #define ROOM_FOR_ID         (buf_len+2-1)
  #define ROOM_FOR_ID_N_DATA  (buf_len+2-2)
  #define ROOM_FOR_ADDRESS    (buf_len+2-sizeof(parm_p))
#endif

/*****************************************************************************/
/* Definements for Buf_opt_ctr                                               */
/*****************************************************************************/
#define PARM_ACC            0               // direct parameter write access
#define BUF_ID              1               // ID + data to buffer
#define BUF_ADDR            2               // parameter write + address to buffer

//class 0
#define C0_da_mem           (PARM_ACC*1)    // direct parameter write access for class 0
#define C0_id_da_buf        (BUF_ID*1)      // ID + data to buffer for class 0
#define C0_da_mem_ad_buf    (BUF_ADDR*1)    // parameter write + address to buffer for class 0
//class 1
#define C1_da_mem           (PARM_ACC*4)
#define C1_id_da_buf        (BUF_ID*4)
#define C1_da_mem_ad_buf    (BUF_ADDR*4)
//class 2
#define C2_da_mem           (PARM_ACC*16)
#define C2_id_da_buf        (BUF_ID*16)
#define C2_da_mem_ad_buf    (BUF_ADDR*16)
//class 3
#define C3_id_buf           (PARM_ACC*64)
#define C3_id_da_buf        (BUF_ID*64)
#define C3_da_mem_ad_buf    (BUF_ADDR*64)
//class 4
#define C4_da_mem           (PARM_ACC*256)
#define C4_id_da_buf        (BUF_ID*256)
#define C4_da_mem_ad_buf    (BUF_ADDR*256)
//class 5
#define C5_da_mem           (PARM_ACC*1024)
#define C5_id_da_buf        (BUF_ID*1024)
#define C5_da_mem_ad_buf    (BUF_ADDR*1024)
//class 6
#define C6_da_mem           (PARM_ACC*4096)
#define C6_id_da_buf        (BUF_ID*4096)
#define C6_da_mem_ad_buf    (BUF_ADDR*4096)
//class 7
#define C7_da_mem           (PARM_ACC*16384)
#define C7_id_da_buf        (BUF_ID*16384)
#define C7_da_mem_ad_buf    (BUF_ADDR*16384)

/*****************************************************************************/
/* Definitions for the CONNECTION REQUEST RESPONSE                           */
/*****************************************************************************/
#define         HIGH_PRO_ID       129
#define         HIGH_CH_ID        0
/*****************************************************************************/
/* Interprete internal states                                                */
/*****************************************************************************/
#define APDU_GET         0x00    // operation specifier
#define APDU_SET         0x80
#define APDU_INFO        0xc0
#define CHK_HEADER       0xc1    // InterpreteAPDU internal states
#define FIND_CLASS       0xc2
#define SAVE_RFS         0xc4
#define SEND_REPLY       0xc3
#define APDU_DATA        0xc7
#define NEXT_APDU        0xc5


/*****************************************************************************/
/* Class 4 ID codes related to GENIpro                                       */
/*****************************************************************************/
#define UNIT_ADDR_ID         46
#define UNIT_GROUP_ID        47

/*****************************************************************************/
/* Class 0 ID codes used by GENIpro                                          */
/*****************************************************************************/
#define BUF_LENGHT_MAX_ID     2
#define UNIT_BUS_MODE_ID      3


/*****************************************************************************/
/* Class 9 channel specifiers                                                */
/*****************************************************************************/
#define IR_CHANNEL             0
#define BUS_CHANNEL            1
#define COM_CHANNEL            2
#define PLM_CHANNEL            3
#define RS232_CHANNEL          4
#define MDM_CHANNEL            5


/****************************************************************************/
/* Modbus interface                                                         */
/****************************************************************************/
typedef enum
{
  BIT8_VALUE = 1,
  BIT8_SCALED_VALUE,
  BIT16_VALUE,
  BIT32_VALUE_LOW16,
  BIT32_VALUE_HIGH16
} GENI_DATA_TYPE;

typedef struct
{
  GENI_DATA_TYPE data_type;
  UCHAR          cl;
  UCHAR          id;
} MODBUS_PARAM;

#if(CTO_MOD_BUS == Enable)
  #define MODBUS_SLAVE_HANDLER(ev)  ModBusSlaveCtlHandler(ev)
  #define MODBUS_STATE_MSK          0x80
  #define MODBUS_CHANNEL_MSK        0x7F
  #define MODBUS_STATE              ((modbus_info & MODBUS_STATE_MSK) > 0)
  #define MODBUS_CHANNEL            (modbus_info & MODBUS_CHANNEL_MSK)
  #define MODBUS_ON(ch_indx)        ((MODBUS_CHANNEL == ch_indx) && (MODBUS_STATE))

  #define SET_MODBUS(modbus_on, ch_indx)                                    \
  {                                                                         \
    if(modbus_on)                                                           \
    {                                                                       \
      modbus_info &= ~MODBUS_STATE_MSK;       /*disable modbus*/            \
      modbus_info &= ~MODBUS_CHANNEL_MSK;     /*clear channel*/             \
      modbus_info |= ch_indx;                 /*set channel*/               \
      modbus_info |= MODBUS_STATE_MSK;        /*enable modbus*/             \
    }                                                                       \
    else                                                                    \
      modbus_info &= ~MODBUS_STATE_MSK;       /* disable modbus*/           \
  }

  #define DISABLE_MODBUS   modbus_info = 0;
#else
  #define MODBUS_SLAVE_HANDLER(ev)  {}
  #define MODBUS_ON(ch_indx)     FALSE
  #define SET_MODBUS(modbus_on, ch_indx)
  #define DISABLE_MODBUS
#endif


/*****************************************************************************/
/* Event codes                                                               */
/*****************************************************************************/
enum    {   eNO_EVENT,             // 0
            eNEW_TGM,              // 1
            ePROCESS_TGM,          // 2
            eSEND_TGM,             // 3
            eREPLY_TO,             // 4
            eTRANS_FAILED,         // 5
            eBUS_SLAVE,            // 6
            eBUS_MASTER,           // 7
            eSEND_DIR,             // 8
            eRX_BRK,               // 9
            eRETRANSMIT,           // 10
            eCONNECT_TIMEOUT,      // 11
            eROUTING_REQUEST,      // 12
            eAUTO_REQ,             // 13
            eGROUP_END,            // 14
            eUPD_GRP,              // 15
            eCLOCK,                // 16
            eROUTING_POLL,         // 17
            eVIR_CONNECT_TIMEOUT,  // 18
            eNEXT_UNIT             // 19
        };

/*****************************************************************************/
/* Return codes                                                              */
/*****************************************************************************/
enum {
    rOK,                              // 0
    rBUSY,                            // 1
    rBUF_FULL,                        // 2
    rCS_ERROR,                        // 3
    rADDR_ERROR,                      // 4
    rSD_ERROR,                        // 5
    rCOM_BUS_BUSY,                    // 6
    rMAC_ERROR,                       // 7
    rREPLY_TO,                        // 8
    rRX_BRK                           // 9
    };
/*****************************************************************************/
/*    Format for routing_buf:                                                */
/*                                                                           */
/*   0       class_header                                                    */
/*   1       length                                                          */
/*   2       route_adress                                                    */
/*   3       route_channel                                                   */
/*   4       PDU..                                                           */
/*   5       PDU..                                                           */
/*****************************************************************************/
/*  Routing state codes, also used as status codes which is returned to      */
/*  the master                                                               */
/*****************************************************************************/
enum { sROUTING_READY,                //  0 System is ready for next route dialog
       sROUTING_LOADED,               //  1 Class9 APDU received, and route dialog initiated
       sROUTING_BUSY,                 //  2 Routing dialog in progress
       sROUTING_REPLY_READY,          //  3 Routing reply is ready
       sROUTING_ERROR                 //  4 Routing error occur
     };

/*****************************************************************************/
/*  Object state codes, also used as status codes which is returned to       */
/*  the master                                                               */
/*****************************************************************************/
enum { sOBJECT_READY,                 //  0 System is ready for next object dialog
       sOBJECT_LOADED,                //  1 object dialog in progress set command
       sOBJECT_BUSY,                  //  2 object dialog in progress set command
       sOBJECT_PROCESSED,             //  3 object dialog has been processed
       sOBJECT_FAILED                 //  4 object dialog has failed
     };

/*****************************************************************************/
/*  Memory specifier for class 8                                             */
/*****************************************************************************/
enum { mSPEC_NONE,                    //  0 no specifier
       mSPEC_RAM,                     //  1 RAM
       mSPEC_ROM,                     //  2 ROM
       mSPEC_EEPROM,                  //  3 EEPROM
       mSPEC_FLASH,                   //  4 Flash memory
       mSPEC_PERIPHERAL_1             //  5 Peripheral Device 1
     };

/*****************************************************************************/
/*  Dump state codes, also used as status codes which is returned to         */
/*  the master                                                               */
/*****************************************************************************/
enum { sDUMP_READY,                   //  0 System is ready for next dump dialog (This time is over)
       sDUMP_LOADED,                  //  1 Class8 APDU received, and dump dialog is initiated
       sDUMP_BUSY,                    //  2 dump dialog is in progress
       sDUMP_PROCESSED,               //  3 dump dialog has been processed
       sDUMP_FAILED,                  //  4 dump dialog has failed
       sDUMP_ILLEGAL                  //  5 dump dialog is illegal
     };

#endif                                //  End of common.h definitions
