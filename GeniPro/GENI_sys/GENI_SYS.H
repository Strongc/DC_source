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
/* MODULE NAME      :   geni_sys.h                                          */
/*                                                                          */
/* FILE NAME        :   geni_sys.h                                          */
/*                                                                          */
/* FILE DESCRIPTION :   Geni system control                                 */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _GENI_SYS_H_
  #define GENI_SYS_H_

#include "geni_rtos_if.h"               // Access to OS interface
#include "geni_if.h"                    // Access to Geni interface

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*  Channel structure                                                       */
/****************************************************************************/
typedef struct  { UCHAR buf_index;   // this member may not be moved, because of the use of the pointer
                  UCHAR *rx_ptr;
                  UCHAR *tx_ptr;
                  UCHAR retry_cnt;
                  UCHAR flags;
                  UCHAR connect_addr;
                  UCHAR buf_cnt;
                  UCHAR drv_state;
                }CHANNEL_PARAM;

/****************************************************************************/
/*  slave callback structure                                                 */
/****************************************************************************/
  typedef struct  {
                  void (*ReceiveUserFct) (void);
                  void (*PreUserFct) (void);
                  void (*PostUserFct) (void);
                  }SLAVE_INTERFACE;

/****************************************************************************/
/*  Defines for masking the flags variable int the channel struct           */
/****************************************************************************/
#define BUSY_FLG_MSK    1
#define USE_CRC16_MSK   2
#define DONT_REPLY_MSK  4
#define POLLED_FLG_MSK  8

#define BUSY_FLG_SHL    0
#define USE_CRC16_SHL   1
#define DONT_REPLY_SHL  2
#define POLLED_FLG_SHL  3

#define BUS_DF_FLG    (0<<POLLED_FLG_SHL) | (0<<DONT_REPLY_SHL) | (BUS_USE_CRC16_CHECK<<USE_CRC16_SHL)   | (0 << BUSY_FLG_SHL)
#define COM_DF_FLG    (0<<POLLED_FLG_SHL) | (0<<DONT_REPLY_SHL) | (COM_USE_CRC16_CHECK<<USE_CRC16_SHL)   | (0 << BUSY_FLG_SHL)
#define PLM_DF_FLG    (0<<POLLED_FLG_SHL) | (0<<DONT_REPLY_SHL) | (PLM_USE_CRC16_CHECK<<USE_CRC16_SHL)   | (0 << BUSY_FLG_SHL)
#define RS232_DF_FLG  (0<<POLLED_FLG_SHL) | (0<<DONT_REPLY_SHL) | (RS232_USE_CRC16_CHECK<<USE_CRC16_SHL) | (0 << BUSY_FLG_SHL)
#define IR_DF_FLG     (0<<POLLED_FLG_SHL) | (0<<DONT_REPLY_SHL) | (IR_USE_CRC16_CHECK<<USE_CRC16_SHL)    | (0 << BUSY_FLG_SHL)
#ifdef TCS_USB_SER_PORT
#define USB_DF_FLG    (0<<POLLED_FLG_SHL) | (0<<DONT_REPLY_SHL) | (USB_USE_CRC16_CHECK<<USE_CRC16_SHL)   | (0 << BUSY_FLG_SHL)
#endif

/****************************************************************************/
/*  channel constant structure                                              */
/****************************************************************************/
typedef struct { UCHAR ch_avaible_msk;
                 UCHAR max_retries;
                 UINT  connect_time;
                 UCHAR connect_timer;
                 UINT  reply_time;
                 UCHAR reply_timer;
                 UINT  inter_data_time;
                 UCHAR inter_data_timer;
                 UINT  poll_time;
                 UCHAR poll_timer;
                 UINT  vir_connect_time;
                 UCHAR vir_connect_timer;
               } CHANNEL_CONST;

/****************************************************************************/
/*  Geni timing structure                                                   */
/****************************************************************************/
typedef struct { UINT compare;
                 UINT interval;
                 UCHAR  event;
                 UCHAR  type;
               } GENI_TIMER;
/****************************************************************************/
/*  Geni timer types                                                        */
/****************************************************************************/
#define GENI_TIMEOUT_TIMER     0x00            //
#define GENI_RELOAD_TIMER      0x01
#define GENI_PUT_FIRST_TIMER   0x02

/****************************************************************************/
/*  Geni channel timing values                                              */
/****************************************************************************/
#define BUS_POLL_TIME         MAS_TICK_TIME   // Poll cycle time
#define BUS_CON_TIME          20000/5         // 20 sec connect timeout, resolution 5 msec
#define BUS_INTER_D_TIME      1               // * 5ms  inter data delay
#ifndef __PC__
#define BUS_REPLY_TIME        10              // * 5ms  Max reply time for slave
#else
#define BUS_REPLY_TIME        1000             // * 5ms  Max reply time for slave
#endif
#define BUS_VIR_TIME_SCALE    100             // Virtual slave connect timeout must be called this number of times.
#define BUS_VIR_CON_TIME      BUS_CON_TIME/BUS_VIR_TIME_SCALE //timeout value

#define IR_POLL_TIME          MAS_TICK_TIME   // Poll cycle time
#define IR_INTER_D_TIME       1               // * 5ms  inter data delay
#define IR_REPLY_TIME         10              // * 5ms  Max reply time for slave

#define PLM_POLL_TIME         MAS_TICK_TIME   // Poll cycle time
#define PLM_CON_TIME          20000/5         // 20 sec connect timeout, resolution 5 msec
#define PLM_INTER_D_TIME      4               // * 5ms  inter data delay
#define PLM_REPLY_TIME        10              // * 5ms  Max reply time for slave
#define PLM_VIR_TIME_SCALE    100             // Virtual slave connect timeout must be called this number of times.
#define PLM_VIR_CON_TIME      PLM_CON_TIME/PLM_VIR_TIME_SCALE //timeout value

#define RS232_POLL_TIME       MAS_TICK_TIME   // Poll cycle time
#define RS232_CON_TIME        20000/5         // 20 sec connect timeout, resolution 5 msec
#define RS232_INTER_D_TIME    1               // * 5ms  inter data delay
#define RS232_REPLY_TIME      10              // * 5ms  Max reply time for slave
#define RS232_VIR_TIME_SCALE  100             // Virtual slave connect timeout must be called this number of times.
#define RS232_VIR_CON_TIME    RS232_CON_TIME/RS232_VIR_TIME_SCALE //timeout value

#define COM_CON_TIME          20000/5         // 20 sec connect timeout, resolution 5 msec
#define COM_INTER_D_TIME      1               // * 5ms  inter data delay
#ifndef __PC__
#define COM_REPLY_TIME        10              // * 5ms  Max reply time for slave
#else
#define COM_REPLY_TIME        100              // * 5ms  Max reply time for slave
#endif
#define COM_VIR_TIME_SCALE    100             // Virtual slave connect timeout must be called this number of times.
#define COM_VIR_CON_TIME      COM_CON_TIME/COM_VIR_TIME_SCALE //timeout value
#ifdef TCS_USB_SER_PORT
#define USB_CON_TIME          20000/5         // 20 sec connect timeout, resolution 5 msec
#define USB_INTER_D_TIME      1               // * 5ms  inter data delay (delay between each 64 byte packets)
#define USB_REPLY_TIME        10              // * 5ms  Max reply time for slave
#define USB_VIR_TIME_SCALE    100             // TCS don't know:? Virtual slave connect timeout must be called this number of times.
#define USB_VIR_CON_TIME      USB_CON_TIME/USB_VIR_TIME_SCALE // TCS don't know:? timeout value
#endif

#define STATIC_APDU_SIZE      static_apdu_tgm[cur_group].size + 2 // ID's + CL + CLLEN
#define POLL_APDU_SIZE        MAS_MAX_REQ_APDU + 2     // ID's + CL + CLLEN
#define TGM_HEADER            6                        // SD + LN + DA + SA + CRCHI + CRCLO
#define TGM_LEN               (STATIC_APDU_SIZE + POLL_APDU_SIZE + TGM_HEADER)

// Define the time from the master starts sending tgm until received a whole reply tgm
//  ((tgm length * (byte + interbyte delay) * baudrate factor * (request + answer)) + 50 ms timeout) / 5 + 1
// To avoid truncation the BytePeriod is multiplied by 1000 and later the formula is divided by 1000

#define MAS_POLL_TIMEOUT      (((TGM_LEN * (1 + 1.2) * ((ULONG)10000*1000/MAS_BAUDRATE) * 2)/1000 + 50 ) / 5  + 1 + GENI_ADD_DELAY)

// Define the time from the master starts sending a routing tgm until received a whole reply tgm
// (Routing_buffer size * (byte + interbyte delay) * baudrate factor * (request + answer)) + 50 ms timeout) / 5 + 1 to round
// To avoid truncation the BytePeriod is multiplied by 1000 and later the formula is divided by 1000
#define ROUTING_POLL_TIMEOUT  ((( ROUTER_DF_buf_len * (1 + 1.2) *((ULONG)10000*1000/MAS_BAUDRATE) * 2)/1000  + 50 ) / 5  + 1 + GENI_ADD_DELAY)

// Define the time from the master starts sending a routing tgm until received a whole reply tgm
// (COM buffer size * (byte + interbyte delay) * baudrate factor * (request + answer)) + 50 ms timeout) / 5 + 1 to round
// To avoid truncation the BytePeriod is multiplied by 1000 and later the formula is divided by 1000
#define MAS_COM_TIMEOUT      (((COM_DF_buf_len * (1 + 1.2) * ((ULONG)10000*1000/MAS_BAUDRATE) * 2)/1000  + 50 ) / 5  + 1 + GENI_ADD_DELAY)

#define DIRECT_TIME           0               // used for put event

/****************************************************************************/
/*  Geni channel timers                                                     */
/****************************************************************************/
#define NO_TIMER              0xFF            // Don't start timer
#define DIRECT_TIMER          0xFE            // Don't start timer - PutEvent instead
#define DIRECT_FIRST_TIMER    0xFD            // Don't start timer - PutFirstEvent instead

/****************************************************************************/
/*  Define the Bus channel timers and number                                */
/****************************************************************************/
#if(CTO_BUS_TYPE==Disable)
  #define BUS_TIMERS 0

#elif(CTO_BUS_TYPE != Disable )
  // slave and master timers
  #define BUS_REPLY_TIMER            0
  #define BUS_INTER_D_TIMER          1
  #define BUS_CON_TIMER              2 // slave mode

  #if(CTO_BUS_TYPE==Master)
  // specific master timer
    #define BUS_POLL_TIMER           2 // Master mode
    #define BUS_VIR_CON_TIMER        NO_TIMER
    #define BUS_TIMERS               3

  #elif((USE_VIRTUAL_SLAVES == TRUE) && (VIR_SLAVE_CHANNEL == BUS_CH))
  // slave with virtual slaves
    #define BUS_POLL_TIMER           NO_TIMER
    #define BUS_VIR_CON_TIMER        3
    #define VIR_SLAVE_COUNT_VAL      BUS_VIR_TIME_SCALE
    #define BUS_TIMERS               4
  #else
  // normal slave
    #define BUS_POLL_TIMER           NO_TIMER
    #define BUS_VIR_CON_TIMER        NO_TIMER
    #define BUS_TIMERS               3
  #endif
#endif
/****************************************************************************/
/*  Define the Ir channel timers and number                                */
/****************************************************************************/
#if(CTO_IR_TYPE==Disable)
  #define IR_TIMERS 0

#elif(CTO_IR_TYPE != Disable )
  // slave and master timers

  #if(CTO_IR_TYPE==Master)
  // specific master timer
    #define IR_REPLY_TIMER          BUS_TIMERS + 0
    #define IR_INTER_D_TIMER        BUS_TIMERS + 1
    #define IR_POLL_TIMER           BUS_TIMERS + 2
    #define IR_TIMERS               3
  #elif((USE_VIRTUAL_SLAVES == TRUE) && (VIR_SLAVE_CHANNEL == IR_CH))
    #error "IR is not allowed to have virtual slaves"
  #else
  // normal slave
    #define IR_REPLY_TIMER          BUS_TIMERS + 0
    #define IR_INTER_D_TIMER        DIRECT_FIRST_TIMER
    #define IR_POLL_TIMER           NO_TIMER
    #define IR_TIMERS               1
  #endif
#endif

/****************************************************************************/
/*  Define the Powerline channel timers and number                          */
/****************************************************************************/
#if(CTO_PLM_TYPE==Disable)
  #define PLM_TIMERS                 0

#elif(CTO_PLM_TYPE != Disable )
  // slave and master timers
  #define PLM_REPLY_TIMER            BUS_TIMERS + IR_TIMERS + 0
  #define PLM_INTER_D_TIMER          BUS_TIMERS + IR_TIMERS + 1
  #define PLM_CON_TIMER              BUS_TIMERS + IR_TIMERS + 2 // slave mode

  #if(CTO_PLM_TYPE==Master)
  // specific master timer
    #define PLM_POLL_TIMER           BUS_TIMERS + IR_TIMERS + 2 // Master mode
    #define PLM_VIR_CON_TIMER        NO_TIMER
    #define PLM_TIMERS               3

  #elif((USE_VIRTUAL_SLAVES == TRUE) && (VIR_SLAVE_CHANNEL == PLM_CH))
  // slave with virtual slaves
    #define PLM_POLL_TIMER           NO_TIMER
    #define PLM_VIR_CON_TIMER        BUS_TIMERS + IR_TIMERS + 3
    #define VIR_SLAVE_COUNT_VAL      PLM_VIR_TIME_SCALE
    #define PLM_TIMERS               4
  #else
  // normal slave
    #define PLM_POLL_TIMER           NO_TIMER
    #define PLM_VIR_CON_TIMER        NO_TIMER
    #define PLM_TIMERS               3
  #endif
#endif
/****************************************************************************/
/*  Define the COM channel timers and number                                */
/****************************************************************************/
#if(CTO_COM_TYPE == Disable)
  #define COM_TIMERS                 0

#else
  #define COM_REPLY_TIMER            BUS_TIMERS + IR_TIMERS + PLM_TIMERS + 0
  #define COM_INTER_D_TIMER          BUS_TIMERS + IR_TIMERS + PLM_TIMERS + 1

  #if ((COM_USE_CON_REQ == TRUE))
    #if (CTO_COM_TYPE == Slave)
      #define COM_CON_TIMER            BUS_TIMERS + IR_TIMERS + PLM_TIMERS + 2
      #define COM_TIMERS               3
    #else
      #error 'COM_USE_CON_REQ is not allowed to be true when using a com master'
    #endif
  #else
    #define COM_TIMERS               2

  #endif
#endif

#ifdef TCS_USB_SER_PORT
/****************************************************************************/
#if(CTO_USB_TYPE == Disable)
  #define USB_TIMERS                 0

#else
  #define USB_REPLY_TIMER            BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 0
  #define USB_INTER_D_TIMER          BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 1//DIRECT_FIRST_TIMER//BUS_TIMERS + IR_TIMERS + PLM_TIMERS + 1

  #if ((USB_USE_CON_REQ == TRUE))
    #if (CTO_USB_TYPE == Slave)
      #define USB_CON_TIMER            BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 2
      #define USB_TIMERS               3
    #else
      #error 'USB_USE_CON_REQ is not allowed to be true when using a USB master'
    #endif
  #else
    #define USB_TIMERS               2

  #endif
#endif

#endif

/****************************************************************************/
/*  Define the RS232 channel timers and number                              */
/****************************************************************************/
#ifdef TCS_USB_SER_PORT
#if(CTO_RS232_TYPE==Disable)
  #define RS232_TIMERS               0

#elif(CTO_RS232_TYPE != Disable )
  // slave and master timers
  #define RS232_REPLY_TIMER 		  BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + USB_TIMERS + 0
  #define RS232_INTER_D_TIMER		  BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + USB_TIMERS + 1
  #define RS232_CON_TIMER			  BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + USB_TIMERS + 2 // slave mode

  #if(CTO_RS232_TYPE==Master)
  // specific master timer
	#define RS232_POLL_TIMER		  BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + USB_TIMERS + 2 // Master mode
    #define RS232_VIR_CON_TIMER       NO_TIMER
    #define RS232_TIMERS              3

  #elif((USE_VIRTUAL_SLAVES == TRUE) && (VIR_SLAVE_CHANNEL == RS232_CH))
  // slave with virtual slaves
    #define RS232_POLL_TIMER          NO_TIMER
	#define RS232_VIR_CON_TIMER 	  BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + USB_TIMERS + 3
    #define VIR_SLAVE_COUNT_VAL       RS232_VIR_TIME_SCALE
    #define RS232_TIMERS              4
  #else
  // normal slave
    #define RS232_POLL_TIMER          NO_TIMER
    #define RS232_VIR_CON_TIMER       NO_TIMER
    #define RS232_TIMERS              3
  #endif
#endif

#else
	#if(CTO_RS232_TYPE==Disable)
	  #define RS232_TIMERS               0

	#elif(CTO_RS232_TYPE != Disable )
	  // slave and master timers
	  #define RS232_REPLY_TIMER           BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 0
	  #define RS232_INTER_D_TIMER         BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 1
	  #define RS232_CON_TIMER             BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 2 // slave mode

	  #if(CTO_RS232_TYPE==Master)
	  // specific master timer
	    #define RS232_POLL_TIMER          BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 2 // Master mode
	    #define RS232_VIR_CON_TIMER       NO_TIMER
	    #define RS232_TIMERS              3

	  #elif((USE_VIRTUAL_SLAVES == TRUE) && (VIR_SLAVE_CHANNEL == RS232_CH))
	  // slave with virtual slaves
	    #define RS232_POLL_TIMER          NO_TIMER
	    #define RS232_VIR_CON_TIMER       BUS_TIMERS + IR_TIMERS + PLM_TIMERS + COM_TIMERS + 3
	    #define VIR_SLAVE_COUNT_VAL       RS232_VIR_TIME_SCALE
	    #define RS232_TIMERS              4
	  #else
	  // normal slave
	    #define RS232_POLL_TIMER          NO_TIMER
	    #define RS232_VIR_CON_TIMER       NO_TIMER
	    #define RS232_TIMERS              3
	  #endif
	#endif
#endif
/****************************************************************************/
/*  Define the total number of timers                                       */
/****************************************************************************/
#ifndef TCS_USB_SER_PORT
	#if ((BUS_TIMERS + PLM_TIMERS + IR_TIMERS + COM_TIMERS + RS232_TIMERS) == 0) // if no timers wanted, make a dummy to avoid error
	  #define NO_GENI_TIMERS  1
	#else
	  #define NO_GENI_TIMERS  BUS_TIMERS + PLM_TIMERS + IR_TIMERS + COM_TIMERS + RS232_TIMERS
	#endif
#else
#if ((BUS_TIMERS + PLM_TIMERS + IR_TIMERS + COM_TIMERS + RS232_TIMERS + USB_TIMERS) == 0) // if no timers wanted, make a dummy to avoid error
  #define NO_GENI_TIMERS  1
#else
  #define NO_GENI_TIMERS  BUS_TIMERS + PLM_TIMERS + IR_TIMERS + COM_TIMERS + RS232_TIMERS + USB_TIMERS
#endif
#endif

#if (NO_TIMER <= DIRECT_FIRST_TIMER)
  #error 'To many timers used'
#endif
/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
/*                                                                          */
/****************************************************************************/
extern const CHANNEL_CONST ch_const[NO_OF_CHANNELS];
extern const SLAVE_INTERFACE ch_interface[NO_OF_CHANNELS];
/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/

extern CHANNEL_PARAM *cur_ch;                      // pointer to current channel parameter
extern CHANNEL_PARAM channels[NO_OF_CHANNELS];     // array of channel parameters

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif
    extern UCHAR * cur_receive_buf;                // pointer to the current receive buffer
    extern UCHAR * cur_transmit_buf;               // pointer to the current transmit buffer
    extern UCHAR * cur_buf_cnt;                    // pointer to the current buffer counter
    extern UCHAR cur_channel;                      // channel currently processed
    extern UCHAR cur_indx;                         // current array index
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* P R O T O T Y P E S   F O R   E X T E R N A L   F U N C T I O N S        */
/*                                                                          */
/****************************************************************************/
// system timers
extern void  SetGeniTimer(UCHAR timer_no, UINT interval, UCHAR event, UCHAR type);
extern void  ClearGeniTimer(UCHAR timer_no);
extern UINT GetGeniSysTimer(void);
// internal event system
extern void ClearAllEvents(void);
extern UCHAR CheckGeniEvent(void);
extern UCHAR GetGeniEvent(void);
extern void PutGeniEvent( UCHAR event);
extern void PutFirstEvent( UCHAR event);
// dummy func
extern void EmptyFunc(void);

// external event system
#if ( EXT_EVENT_QUEUE_LEN > 0 )
    extern void ClearAllExtEvents(void);
    extern UCHAR CheckExtEvent(void);
    extern UCHAR GetExtEvent(void);
    extern void PutExtEvent( UCHAR event);
    extern UCHAR ExtEventExists(UCHAR event);              //  Sub function
    extern void PutFirstExtEvent( UCHAR event);            //  Sub function

#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
#endif              // finish of include geni_sys.h include
