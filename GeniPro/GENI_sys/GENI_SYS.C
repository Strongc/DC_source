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
/* MODULE NAME      : Geni_sys.c                                            */
/*                                                                          */
/* FILE NAME        : Geni_sys.c                                            */
/*                                                                          */
/* FILE DESCRIPTION : Geni system control                                   */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "geni_cnf.h"                   // Access to GENIpro configuration
#include "common.H"                     // Access to common definitions
#include "geni_rtos_if.h"               // Access to OS interface
#include "geni_if.h"                    // Access to GENIpro interface
#include "vir_drv.h"                    // Access to driver logic
#if(CTO_MOD_BUS == Enable)
#include "modbus_slave_ctr.h"           // Access to Modbus
#endif
#include "geni_sys.h"                   // Own publics


/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* EmbOs interface                                                          */
/****************************************************************************/
#if (GENI_RTOS_TYPE == GENI_EMB_OS)
  #define GENI_EVENT 1
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_TSK)
#endif

  OS_TASK GeniSysTaskHandle;                                  // task handle for geni
  OS_STACKPTR unsigned int GENIStack[GENI_TASK_STACK_SIZE];   // Stack for geni
  OS_TIMER GeniTimer;                                         // timer for Geni Sys irq
  OS_RSEMA geni_class_data;                                   // semaphor for class data
  OS_RSEMA geni_master;                                       // semaphor for active GENI task

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif
#endif
/****************************************************************************/
/* Geni channel definitions                                                 */
/****************************************************************************/
#ifdef TCS_USB_SER_PORT

	#if ((CTO_IR_TYPE  == Slave) || (CTO_COM_TYPE == Slave)  || (CTO_BUS_TYPE != Disable) || (CTO_PLM_TYPE != Disable) || (CTO_RS232_TYPE != Disable)  || (CTO_USB_TYPE != Disable))
	  #include "vir_ctr_slave.h"
	  #define INIT_SLAVE          InitSlave()
	  #define SLAVE_HANDLER(a)    SlaveControlHandler(a)
	#else
	  #define INIT_SLAVE
	  #define SLAVE_HANDLER(a)
	#endif
	#if ((CTO_IR_TYPE  == Master) || (CTO_BUS_TYPE  == Master) || (CTO_PLM_TYPE == Master)  || (CTO_RS232_TYPE == Master) || (CTO_COM_TYPE == Master) || (CTO_USB_TYPE != Disable))
	  #include "vir_ctr_master.h"
	  #define INIT_MASTER         InitMaster()
	  #define DISABLE_MASTER      DisableMaster()
	  #define RESET_MASTER        ResetMaster()
	  #define MASTER_HANDLER(a)   MasterControlHandler(a)
	#else
	  #define INIT_MASTER
	  #define DISABLE_MASTER
	  #define RESET_MASTER
	  #define MASTER_HANDLER(a)
	#endif
#else
#if (  (CTO_IR_TYPE  == Slave)   || (CTO_COM_TYPE == Slave)  || (CTO_BUS_TYPE != Disable) || (CTO_PLM_TYPE != Disable) || (CTO_RS232_TYPE != Disable))
  #include "vir_ctr_slave.h"
  #define INIT_SLAVE          InitSlave()
  #define SLAVE_HANDLER(a)    SlaveControlHandler(a)
#else
  #define INIT_SLAVE
  #define SLAVE_HANDLER(a)
#endif
#if ( (CTO_IR_TYPE  == Master) || (CTO_BUS_TYPE  == Master) || (CTO_PLM_TYPE == Master)  || (CTO_RS232_TYPE == Master) || (CTO_COM_TYPE == Master) )
  #include "vir_ctr_master.h"
  #define INIT_MASTER         InitMaster()
  #define DISABLE_MASTER      DisableMaster()
  #define RESET_MASTER        ResetMaster()
  #define MASTER_HANDLER(a)   MasterControlHandler(a)
#else
  #define INIT_MASTER
  #define DISABLE_MASTER
  #define RESET_MASTER
  #define MASTER_HANDLER(a)
	#endif
#endif

#if ((CTO_BUS_TYPE  != Disable) && (BUS_IDLE_TYPE == GENI_IRQ_IDLE))
  #define BUS_IDLE_FUNCTION     GeniBUSIdleTimerIrq()
#else
  #define BUS_IDLE_FUNCTION
#endif

#if ((CTO_RS232_TYPE  != Disable) && (RS232_IDLE_TYPE == GENI_IRQ_IDLE))
  #define RS232_IDLE_FUNCTION   GeniRS232IdleTimerIrq()
#else
  #define RS232_IDLE_FUNCTION
#endif

#if ((CTO_COM_TYPE  != Disable) && (COM_IDLE_TYPE == GENI_IRQ_IDLE))
  #define COM_IDLE_FUNCTION     GeniCOMIdleTimerIrq()
#else
  #define COM_IDLE_FUNCTION
#endif

#if ((CTO_PLM_TYPE  != Disable) && (PLM_IDLE_TYPE == GENI_IRQ_IDLE))
  #define PLM_IDLE_FUNCTION     GeniPLMIdleTimerIrq()
#else
  #define PLM_IDLE_FUNCTION
#endif

#if ((CTO_USB_TYPE  != Disable) && (USB_IDLE_TYPE == GENI_IRQ_IDLE))
  #define USB_IDLE_FUNCTION     GeniUSBIdleTimerIrq()
#else
  #define USB_IDLE_FUNCTION
#endif
/****************************************************************************/
/*                                                                          */
/* G L O B A L    C O N S T A N T S                                         */
/*                                                                          */
/****************************************************************************/
const CHANNEL_CONST ch_const[NO_OF_CHANNELS] = {
#if(CTO_IR_TYPE != Disable)   // Use IR
 //ch_avaible_msk,    max_retries,   connect_time,   connect_timer,       reply_time,       reply_timer,    inter_data_time,   inter_data_timer,       poll_time,       poll_timer,   vir_connect_time,   vir_connect_timer
{   IR_channel_fl,     IR_MAX_RTY,              0,        NO_TIMER,    IR_REPLY_TIME,    IR_REPLY_TIMER,    IR_INTER_D_TIME,   IR_INTER_D_TIMER,    IR_POLL_TIME,    IR_POLL_TIMER,                  0,            NO_TIMER }
  #if (MAX_CH_NO != IR_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_BUS_TYPE != Disable)  // Use Bus
{  BUS_channel_fl,    BUS_MAX_RTY,   BUS_CON_TIME,   BUS_CON_TIMER,   BUS_REPLY_TIME,   BUS_REPLY_TIMER,   BUS_INTER_D_TIME,   BUS_INTER_D_TIMER,   BUS_POLL_TIME,   BUS_POLL_TIMER,   BUS_VIR_CON_TIME,   BUS_VIR_CON_TIMER }
  #if (MAX_CH_NO != BUS_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_PLM_TYPE != Disable)  // Use Powerline
{  PLM_channel_fl,    PLM_MAX_RTY,   PLM_CON_TIME,   PLM_CON_TIMER,   PLM_REPLY_TIME,   PLM_REPLY_TIMER,   PLM_INTER_D_TIME,  PLM_INTER_D_TIMER,   PLM_POLL_TIME,   PLM_POLL_TIMER,   PLM_VIR_CON_TIME,   PLM_VIR_CON_TIMER }
  #if (MAX_CH_NO != PLM_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_COM_TYPE != Disable)   // Use Com channel
  #if (COM_USE_CON_REQ == TRUE)
  {  COM_channel_fl,    COM_MAX_RTY,   COM_CON_TIME,   COM_CON_TIMER,   COM_REPLY_TIME,   COM_REPLY_TIMER,   COM_INTER_D_TIME,  COM_INTER_D_TIMER,               0,         NO_TIMER,                  0,            NO_TIMER }
  #else
  {  COM_channel_fl,    COM_MAX_RTY,              0,        NO_TIMER,   COM_REPLY_TIME,   COM_REPLY_TIMER,   COM_INTER_D_TIME,  COM_INTER_D_TIMER,               0,         NO_TIMER,                  0,            NO_TIMER }
  #endif
  #if (MAX_CH_NO != COM_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_RS232_TYPE != Disable) // Use RS232 channel
{RS232_channel_fl,  RS232_MAX_RTY, RS232_CON_TIME, RS232_CON_TIMER, RS232_REPLY_TIME, RS232_REPLY_TIMER, RS232_INTER_D_TIME, RS232_INTER_D_TIMER, RS232_POLL_TIME, RS232_POLL_TIMER, RS232_VIR_CON_TIME, RS232_VIR_CON_TIMER }
  #if (MAX_CH_NO != RS232_CH)
     ,
  #else
     };
  #endif
#endif
#if((CTO_USB_TYPE != Disable) && (TCS_USB_SER_PORT == 1))// Use USB channel
  #if (USB_USE_CON_REQ == TRUE)
	{  USB_channel_fl,	  0,	 USB_CON_TIME,	 USB_CON_TIMER,   USB_REPLY_TIME,	USB_REPLY_TIMER,   USB_INTER_D_TIME,  USB_INTER_D_TIMER,			   0,		  NO_TIMER, 				 0, 		   NO_TIMER }
  #else
	{  USB_channel_fl,	  0,				0,		  NO_TIMER,   USB_REPLY_TIME,	USB_REPLY_TIMER,   USB_INTER_D_TIME,  USB_INTER_D_TIMER,			   0,		  NO_TIMER, 				 0, 		   NO_TIMER }
  #endif
  #if (MAX_CH_NO != USB_CH)
	   ,
  #else
	   };
  #endif
#endif

/****************************************************************************/
/*  slave callback structure                                                 */
/****************************************************************************/
  // setup the user functions for the choosen channel
const SLAVE_INTERFACE ch_interface[NO_OF_CHANNELS] = {
#if(CTO_IR_TYPE != Disable)   // Use IR
  { Ir_rec_user_fct,       Ir_pre_user_fct,    Ir_post_user_fct}
  #if (MAX_CH_NO != IR_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_BUS_TYPE != Disable)  // Use Bus
  { Bus_rec_user_fct,     Bus_pre_user_fct,   Bus_post_user_fct}
  #if (MAX_CH_NO != BUS_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_PLM_TYPE != Disable)  // Use Powerline
  { Plm_rec_user_fct,     Plm_pre_user_fct,   Plm_post_user_fct}
  #if (MAX_CH_NO != PLM_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_COM_TYPE != Disable)   // Use Com channel
  { Com_rec_user_fct,     Com_pre_user_fct,   Com_post_user_fct}
  #if (MAX_CH_NO != COM_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_RS232_TYPE != Disable) // Use RS232 channel
  { RS232_rec_user_fct, RS232_pre_user_fct, RS232_post_user_fct}
  #if (MAX_CH_NO != RS232_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_USB_TYPE != Disable) // Use USB channel
  { Usb_rec_user_fct, Usb_pre_user_fct, Usb_post_user_fct}
  #if (MAX_CH_NO != USB_CH)
     ,
  #else
     };
  #endif
#endif

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
CHANNEL_PARAM *cur_ch;

// default values - some will be changed during run
CHANNEL_PARAM channels[NO_OF_CHANNELS] = {
#if(CTO_IR_TYPE != Disable)   // Use IR
//buf_index,           rx_ptr,           tx_ptr,     retry_cnt,        flags, connect_addr, buf_cnt,         drv_state
{         0,    &IR_rx_buf[0],    &IR_tx_buf[0],             0,    IR_DF_FLG,         0xFE,       0, GENI_RECEIVE_WAIT }
  #if (MAX_CH_NO != IR_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_BUS_TYPE != Disable)  // Use Bus
{         0,   &BUS_rx_buf[0],   &BUS_tx_buf[0],   BUS_MAX_RTY,   BUS_DF_FLG,         0xFE,       0, GENI_RECEIVE_WAIT }
  #if (MAX_CH_NO != BUS_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_PLM_TYPE != Disable)  // Use Powerline
{         0,   &PLM_rx_buf[0],   &PLM_tx_buf[0],   PLM_MAX_RTY,   PLM_DF_FLG,         0xFE,       0, GENI_RECEIVE_WAIT }
  #if (MAX_CH_NO != PLM_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_COM_TYPE != Disable)   // Use Com channel
{         0,   &COM_rx_buf[0],   &COM_tx_buf[0],   COM_MAX_RTY,   COM_DF_FLG,         0xFE,       0, GENI_RECEIVE_WAIT }
  #if (MAX_CH_NO != COM_CH)
     ,
  #else
     };
  #endif
#endif
#if(CTO_RS232_TYPE != Disable) // Use RS232 channel
{         0, &RS232_rx_buf[0], &RS232_tx_buf[0], RS232_MAX_RTY, RS232_DF_FLG,         0xFE,       0, GENI_RECEIVE_WAIT }
  #if (MAX_CH_NO != RS232_CH)
     ,
  #else
     };
  #endif
#endif
#if((CTO_USB_TYPE != Disable) && (TCS_USB_SER_PORT == 1)) // Use USB channel
{         0, &USB_rx_buf[0], &USB_tx_buf[0], USB_MAX_RTY, USB_DF_FLG,         0xFE,       0, GENI_RECEIVE_WAIT }
  #if (MAX_CH_NO != USB_CH)
     ,
  #else
     };
  #endif
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

UCHAR slave_unit_addr;                          // unit address for the slaves
UCHAR slave_group_addr;                         // An app. set group addr.
UCHAR geni_setup;                               // An app. setup of unit_bus_mode
UCHAR unit_bus_mode;                            // genipro mode register

#if (CTO_BUS_TYPE == Slave)
 UCHAR baud_rate_default;
 UCHAR set_baud_rate;
#endif

UCHAR min_reply_delay_default = GENI_REPLY_DELAY;
UCHAR set_min_reply_delay;

UCHAR cur_channel;                              // channel currently processed
UCHAR cur_indx;                                 // Array index of current channel
UCHAR * cur_receive_buf;                        // pointer to the current receive buffer
UCHAR * cur_transmit_buf;                       // pointer to the current transmit buffer
UCHAR * cur_buf_cnt;                            // pointer to the current buffer counter

#if(CTO_MOD_BUS == Enable)
UCHAR modbus_info;                       // MSB is the modbus on/off, the rest is the channel indx
#endif

#if (defined MAS_CH)
  UCHAR master_unit_addr;                         // unit address for the master
#else
  const UCHAR master_unit_addr;
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

static UINT geni_sys_timer;                            // main geni timer register,
static GENI_TIMER geni_timers[NO_GENI_TIMERS];         // timers avaiable for the system

static UCHAR event_queue[EVENT_QUEUE_LEN];    // event queue
static UCHAR ev_input;                        // event input index
static UCHAR ev_output;                       // event output index

#if ( EXT_EVENT_QUEUE_LEN > 0 )
  static UCHAR event_queue_ext[EXT_EVENT_QUEUE_LEN];   // event queue
  static UCHAR ev_input_ext;                  // event input index
  static UCHAR ev_output_ext;                 // event output index
#endif



#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
void GeniSysIrq(void);
void GeniSysTask(void);
void GeniSysTimer(void);
void GeniOpenChannel(UCHAR ch_indx, UCHAR p_modbus_on);
void GeniCloseChannel(UCHAR ch_indx);
void GeniResetChannel(UCHAR ch_indx);

void SetGeniTimer(UCHAR timer_no, UINT interval, UCHAR event, UCHAR type);
void ClearGeniTimer(UCHAR timer_no);            //
UINT GetGeniSysTimer(void);                     //
void ResetGeniTimers(void);                     //  Sub function

void ClearAllEvents(void);                      //  Sub function
UCHAR CheckGeniEvent(void);                     //  Sub function
UCHAR GetGeniEvent(void);                       //  Sub function
void PutGeniEvent( UCHAR event);                //  Sub function
void PutFirstEvent( UCHAR event);               //  Sub function
static UCHAR GetBaudrateIndx(ULONG baudrate);

#if ( EXT_EVENT_QUEUE_LEN > 0 )
void ClearAllExtEvents(void);                   //  Sub function
UCHAR CheckExtEvent(void);                      //  Sub function
UCHAR GetExtEvent(void);                        //  Sub function
void PutExtEvent( UCHAR event);                 //  Sub function
UCHAR ExtEventExists(UCHAR event);              //  Sub function
void PutFirstExtEvent( UCHAR event);            //  Sub function
#endif
/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
*     Name      : GeniSysInit                                               *
*               :                                                           *
*     Inputs    : none                                                      *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Perform all system initialisation. Initialise IR- and     *
*               : power line channel. Reset the protocol interpreter        *
*               : and the main genipro system                               *
*               :                                                           *
*---------------------------------------------------------------------------*/
void GeniSysInit(void)
{
  UCHAR i;
  UCHAR baud;
  #if (GENI_RTOS_TYPE == GENI_EMB_OS)       // if using embOS then create the geni task
    OS_CREATERSEMA(&geni_class_data);
    OS_CREATERSEMA(&geni_master);
    OS_CREATETASK(&GeniSysTaskHandle, "GENI Task", GeniSysTask,  GENI_TASK_PRIORITY, GENIStack);
  #endif

  ClearAllEvents();               // reset the event system
  #if ( EXT_EVENT_QUEUE_LEN > 0 )
  ClearAllExtEvents();            // reset the extend event system
  #endif
  ResetGeniTimers();              // initialize the Genipro system timers

  INIT_SLAVE;                      // init slave functionality if any

#if (CTO_BUS_TYPE == Slave)
  baud_rate_default = GetBaudrateIndx(BUS_BAUDRATE);
  set_baud_rate = baud_rate_default;
#endif

  set_min_reply_delay = min_reply_delay_default;

  i = 0;
  while (i < NO_OF_CHANNELS)      // open all channels
  {
    // get the specified default baudrate
    switch (i)
    {
      #if (CTO_BUS_TYPE == Slave)
      case BUS_CH_INDX :
        baud = set_baud_rate;
        GeniOpenChannel(i, GENI_SETUP_PARAM(OFF, baud, GENI_DATA_NO_PARITY, GENI_DATA_ONE_STOP_BIT ));
      break;
      #endif
      #if (CTO_BUS_TYPE == Master)
      case BUS_CH_INDX :
        baud = GetBaudrateIndx(BUS_BAUDRATE);
        GeniOpenChannel(i, GENI_SETUP_PARAM(OFF, baud, GENI_DATA_NO_PARITY, GENI_DATA_ONE_STOP_BIT ));
      break;
      #endif
      #if ((CTO_COM_TYPE == Slave) || (CTO_COM_TYPE == Master))
      case COM_CH_INDX :
        baud = GetBaudrateIndx(COM_BAUDRATE);
        GeniOpenChannel(i, GENI_SETUP_PARAM(OFF, baud, GENI_DATA_NO_PARITY, GENI_DATA_ONE_STOP_BIT ));
      break;
      #endif

      #if ((CTO_PLM_TYPE == Slave) || (CTO_PLM_TYPE == Master))
      case PLM_CH_INDX :
        baud = GetBaudrateIndx(PLM_BAUDRATE);
        GeniOpenChannel(i, GENI_SETUP_PARAM(OFF, baud, GENI_DATA_NO_PARITY, GENI_DATA_TWO_STOP_BIT ));
      break;
      #endif

      #if ((CTO_RS232_TYPE == Slave) || (CTO_RS232_TYPE == Master))
      case RS232_CH_INDX :
        baud = GetBaudrateIndx(RS232_BAUDRATE);
        GeniOpenChannel(i, GENI_SETUP_PARAM(OFF, baud, GENI_DATA_NO_PARITY, GENI_DATA_ONE_STOP_BIT ));
      break;
      #endif
	    
      #if (((CTO_USB_TYPE == Slave) || (CTO_USB_TYPE == Master)) && (TCS_USB_SER_PORT == 1))
      case USB_CH_INDX :
        baud = GetBaudrateIndx(USB_BAUDRATE);
        GeniOpenChannel(i, GENI_SETUP_PARAM(OFF, baud, GENI_DATA_NO_PARITY, GENI_DATA_ONE_STOP_BIT ));
    	break;
      #endif

      default:
      break;
    }
    i++;
  }

  GENI_SYS_TIMER_INIT;            // and init / enable
}

/****************************************************************************
*     Name      : GetBaudrateIndx                                           *
*               :                                                           *
*     Inputs    : baudrate as value                                         *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : The baudrate index                                        *
*               :                                                           *
*   Description : Opens and enables the specified channel.                  *
*               :                                                           *
*---------------------------------------------------------------------------*/
static UCHAR GetBaudrateIndx(ULONG baudrate)
{
  UCHAR baud_indx = GENI_BAUD_9600;

  switch(baudrate)
  {
    case 1200 :
      baud_indx = GENI_BAUD_1200;
    break;
    case 2400 :
      baud_indx = GENI_BAUD_2400;
    break;
    case 4800 :
      baud_indx = GENI_BAUD_4800;
    break;
    case 9600 :
      baud_indx = GENI_BAUD_9600;
    break;
    case 19200 :
      baud_indx = GENI_BAUD_19200;
    break;
    case 38400 :
      baud_indx = GENI_BAUD_38400;
    break;
    default:
      baud_indx = GENI_BAUD_9600;
    break;
  }
  return baud_indx;
}

/****************************************************************************
*     Name      : GeniOpenChannel                                           *
*               :                                                           *
*     Inputs    : ch_indx - the channel index                               *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Opens and enables the specified channel.                  *
*               :                                                           *
*---------------------------------------------------------------------------*/
void GeniOpenChannel(UCHAR ch_indx, UCHAR setup_param)
{
  if ( ch_indx < NO_OF_CHANNELS)                           // valid channel index ?
  {
    channels[ch_indx].flags &= ~DONT_REPLY_MSK;            // it is ok to transmit - clear the flag
    channels[ch_indx].flags &= ~POLLED_FLG_MSK;            // clear the flag
    unit_bus_mode |= ch_const[ch_indx].ch_avaible_msk;     // Set flag
    SET_MODBUS((setup_param >> 7) & 0x01, ch_indx);

    if ( ch_indx == MAS_CH_INDX)
    {
      if( ConReply_ON )
        channels[ch_indx].connect_addr  = CONNECTION;
      else
        channels[ch_indx].connect_addr = master_unit_addr;

      INIT_MASTER;                                        // init the master
    }
    else
    {
      if( ConReply_ON )
        channels[ch_indx].connect_addr  = CONNECTION;
      else
        channels[ch_indx].connect_addr = slave_unit_addr;
    }
    // slave cannot be reinitialized because it is shared between more channels
    InitGeniDriver(ch_indx, setup_param);

    GeniResetChannel(ch_indx);
  }
}

/****************************************************************************
*     Name      : GeniCloseChannel                                          *
*               :                                                           *
*     Inputs    : ch_indx - the channel index                               *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Closes and disables the specified channel                 *
*               :                                                           *
*---------------------------------------------------------------------------*/
void GeniCloseChannel(UCHAR ch_indx)
{
  if ( ch_indx < NO_OF_CHANNELS)                           // valid channel index ?
  {
    SET_MODBUS(OFF, ch_indx);
    unit_bus_mode &= ~ch_const[ch_indx].ch_avaible_msk;    // clear flag

    // clear all timers
    ClearGeniTimer(ch_const[ch_indx].reply_timer);
    ClearGeniTimer(ch_const[ch_indx].connect_timer);
    ClearGeniTimer(ch_const[ch_indx].inter_data_timer);
    ClearGeniTimer(ch_const[ch_indx].vir_connect_timer);
    ClearGeniTimer(ch_const[ch_indx].poll_timer);

    if ( ch_indx == MAS_CH_INDX)
    {
      DISABLE_MASTER;                                     // Disable the master
    }
    // slave cannot be closed because it is shared between more channels

    DisableGeniDriver(ch_indx);
  }

}
/****************************************************************************
*     Name      : GeniResetChannel                                          *
*               :                                                           *
*     Inputs    : ch_indx - the channel index                               *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Resets the specified channel.                             *
*               :                                                           *
*---------------------------------------------------------------------------*/
void GeniResetChannel(UCHAR ch_indx)
{
  if ( ch_indx < NO_OF_CHANNELS)                           // valid channel index ?
  {
    if ( ch_indx == MAS_CH_INDX)
    {
      RESET_MASTER;                                       // Reset the master
    }
    // slave cannot be reset because it is shared between more channels

    ResetGeniDriver(ch_indx);
  }
}

/****************************************************************************
*     Name      : GeniSysIrq                                                *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  This function is responsible for all genipro timings     *
*               :  and initiates processing of incoming tgm's               *
*               :                                                           *
*               :  - Update the system timers                               *
*               :  - Check the event queue for events. If any events        *
*               :  - is waiting then run the geni_sys_task                  *
*---------------------------------------------------------------------------*/
void GeniSysIrq(void)
{                                     // operation system dependent
  GENI_SYS_TIMER_RELOAD;              // reload the timer for next irq
  GeniSysTimer();                     // perform system timer job

  BUS_IDLE_FUNCTION;                  // call idle function if any
  COM_IDLE_FUNCTION;                  // call idle function if any
  PLM_IDLE_FUNCTION;                  // call idle function if any
  RS232_IDLE_FUNCTION;                // call idle function if any

  #ifdef TCS_USB_SER_PORT
   USB_IDLE_FUNCTION;				  // call idle function if any
  #endif
  if( CheckGeniEvent() )              // any event from timers or system
    GENI_SYS_TASK_RUN;                // then run geni_task to handle it

}
/****************************************************************************
*     Name      :  GeniSysTask                                              *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  First all internal events is processed until the buffer  *
*               :  is empty. The internal events should all be a kind of    *
*               :  internal or feedback events and not events which trig an *
*               :  new dialog. The external events normally trig an new     *
*               :  dialog and may cause generation of feedback events, eg.  *
*               :  when the response of the telegram is received.           *
*               :  Only one external event is processed at each time.       *
*---------------------------------------------------------------------------*/
void GeniSysTask(void)
{
  UCHAR ev;
  GENI_SYS_TASK_WAIT;                          // wait to next system clock tick
  #if (GENI_RTOS_TYPE == GENI_EMB_OS)
  while(1)
  #endif
  {
    //  process all the events in the internal que
    while( ( (ev = GetGeniEvent()) & EV_INFO ) != eNO_EVENT )
    {
      cur_channel      = (ev & CH_INFO);       //  free channel number information
      // speed optimization
      cur_indx         = cur_channel >> 6;     //  get the channel array index
      cur_ch           = &channels[cur_indx];  //  get the current channel struct
      cur_buf_cnt      = &cur_ch->buf_cnt;     //  get current buf count
      cur_transmit_buf = cur_ch->tx_ptr;       //  get current tx buf
      cur_receive_buf  = cur_ch->rx_ptr;       //  get current rx buf
      // speed optimization  end
      ev &= EV_INFO;                           //  free event information

      if ( cur_indx == MAS_CH_INDX )           // if master channel
      {
        GENI_USE_MASTER
        MASTER_HANDLER(ev);                    // call master controller
        GENI_UNUSE_MASTER
      }
      else
      {
        if(MODBUS_ON(cur_indx))
        {
          MODBUS_SLAVE_HANDLER(ev);
        }
        else
        {
          SLAVE_HANDLER(ev);                     // call slavecontroller
        }
      }
    }

  #if ( EXT_EVENT_QUEUE_LEN > 0 )              // if any external queue
    // process one event from the external que
    if( ( (ev = GetExtEvent() ) & EV_INFO ) != eNO_EVENT )
    {                                          //
      cur_channel      = (ev & CH_INFO);       //  free channel number information
      // speed optimization
      cur_indx         = cur_channel >> 6;     //  get the channel array index
      cur_ch           = &channels[cur_indx];  //  get the current channel struct
      cur_buf_cnt      = &cur_ch->buf_cnt;     //  get current buf count
      cur_transmit_buf = cur_ch->tx_ptr;       //  get current tx buffer
      cur_receive_buf  = cur_ch->rx_ptr;       //  get current rx buffer
      // speed optimization end
      ev &= EV_INFO;                           //  free event information

      if ( cur_indx == MAS_CH_INDX )           // if master channel
      {
        GENI_USE_MASTER
        MASTER_HANDLER(ev);                    // call master controller
        GENI_UNUSE_MASTER
      }
      else
      {
        if(MODBUS_ON(cur_indx))
        {
          MODBUS_SLAVE_HANDLER(ev);
        }
        else
        {
          SLAVE_HANDLER(ev);                      // call slave controller
        }
      }
    }
  #endif

  #if (GENI_RTOS_TYPE == GENI_EMB_OS)
  OS_WaitEvent(GENI_EVENT);                    // Done! - wait for next signal
  #endif
  }
}
/****************************************************************************
*     Name      :  PutGeniEvent                                             *
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
*   Description :  PutGeniEvent stores an event signal in the circ          *
*               :  event queue buffer                                       *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void PutGeniEvent( UCHAR event)
{
  GENI_DISABLE_GLOBAL_INT();                   // No interrupts allowed when
  event_queue[ev_input++] = event;             // queue and pointers are updated
  ev_input &= (EVENT_QUEUE_LEN-1);             //
  if(ev_input == ev_output)                    // close to overrun in queue - one place left
  {
    DEBUG_GENI_WARNING(GENI_WARNING_EVENT_QUEUE_OVERRUN);
  }
  GENI_ENABLE_GLOBAL_INT();                    // Interrupts are allowed again
}

/****************************************************************************
*     Name      :  PutFirstEvent                                            *
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
*   Description :  PutGeniEvent stores an event signal in the circ          *
*               :  event queue buffer as the first event to be handled      *
*               :  This function should be used for high priority signals   *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void PutFirstEvent( UCHAR event)               //  Sub function
{
  GENI_DISABLE_GLOBAL_INT();                   // No interrupts allowed when queue
  ev_output--;                                 // and pointers are updated
  ev_output &= (EVENT_QUEUE_LEN-1);            // adjust for circ buf
  event_queue[ev_output] = event;              // insert event as first
  if(ev_input == ev_output)                    // close to overrun in queue - one place left
  {
    DEBUG_GENI_WARNING(GENI_WARNING_EVENT_QUEUE_OVERRUN);
  }
  GENI_ENABLE_GLOBAL_INT();                    // Interrupts are allowed again
}

/****************************************************************************
*     Name      :  GetGeniEvent                                             *
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
*   Description :  GetGeniEvent get the next event from the event queue     *
*               :  buffer                                                   *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR GetGeniEvent(void)
{
 UCHAR ev;
  if( ev_input != ev_output)
  {
    GENI_DISABLE_GLOBAL_INT();                 // no interrupts allowed
    ev = event_queue[ev_output++];             // PutFirstEvent can update the
    ev_output &= (EVENT_QUEUE_LEN-1);          // ev_output index from interrupt
    GENI_ENABLE_GLOBAL_INT();                  // Interrupt are allowed again
  }
  else
    ev = eNO_EVENT;
  return ev;
}
/****************************************************************************
*     Name      :  ClearAllEvents                                           *
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
*   Description :  Empty the event queue                                    *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void ClearAllEvents(void)
{
  GENI_DISABLE_GLOBAL_INT();
  ev_input  = 0;
  ev_output = 0;
  GENI_ENABLE_GLOBAL_INT();
}

/****************************************************************************
*     Name      :  CheckGeniEvent                                           *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Check if any events in queue and return TRUE else        *
*               :  return FALSE                                             *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR CheckGeniEvent(void)
{
  UCHAR c;
  c = (ev_input != ev_output);
  #if ( EXT_EVENT_QUEUE_LEN > 0 )
  c |= ( ev_input_ext != ev_output_ext );
  #endif
  return c;
}

#if ( EXT_EVENT_QUEUE_LEN > 0 )                // Prepared for two geni_sys event queues
/***************************************************************************/
/*                      External event handling                            */
/****************************************************************************
*     Name      :  PutExtEvent                                              *
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
*   Description :  PutExtEvent stores an event signal in the circ ext       *
*               :  event queue buffer.                                      *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void PutExtEvent( UCHAR event)
{
  GENI_DISABLE_GLOBAL_INT();
  event_queue_ext[ev_input_ext++] = event;
  ev_input_ext &= (EXT_EVENT_QUEUE_LEN-1);
  if(ev_input_ext == ev_output_ext)                    // close to overrun in queue - one place left
  {
    DEBUG_GENI_WARNING(GENI_WARNING_EXT_EVENT_QUEUE_OVERRUN);
  }

  GENI_ENABLE_GLOBAL_INT();
}

/****************************************************************************
*     Name      :  PutFirstExtEvent                                         *
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
*   Description :  PutGeniExtEvent stores an event signal in the circ       *
*               :  event queue buffer as the first event to be handled      *
*               :  This function should be used for high priority signals   *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void PutFirstExtEvent(UCHAR event)            //  Sub function
{
  GENI_DISABLE_GLOBAL_INT();                  // No interrupts allowed when queue
  ev_output_ext--;                            // and pointers are updated
  ev_output_ext &= (EXT_EVENT_QUEUE_LEN-1);   // adjust for circ buf
  event_queue_ext[ev_output_ext] = event;     // insert event as first
  if(ev_input_ext == ev_output_ext)           // close to overrun in queue - one place left
  {
    DEBUG_GENI_WARNING(GENI_WARNING_EXT_EVENT_QUEUE_OVERRUN);
  }

  GENI_ENABLE_GLOBAL_INT();                   // Interrupts are allowed again
}

/****************************************************************************
*     Name      :  GetExtEvent                                              *
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
*   Description :  GetExtEvent get the next event from the event queue      *
*               :  buffer                                                   *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR GetExtEvent(void)
{
 UCHAR ev;

  if( ev_input_ext != ev_output_ext)
  {
    GENI_DISABLE_GLOBAL_INT();                 // no interrupts allowed
    ev = event_queue_ext[ev_output_ext++];     // PutFirstEvent can update the
    ev_output_ext &= (EXT_EVENT_QUEUE_LEN-1);  // ev_output index from interrupt
    GENI_ENABLE_GLOBAL_INT();
  }
  else
    ev = eNO_EVENT;
  return ev;
}
/****************************************************************************
*     Name      :  ClearAllExtEvents                                        *
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
*   Description :  Empty the event queue                                    *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void ClearAllExtEvents(void)
{
  GENI_DISABLE_GLOBAL_INT();                   // no interrupts allowed
  ev_input_ext  = 0;
  ev_output_ext = 0;
  GENI_ENABLE_GLOBAL_INT();
}

/****************************************************************************
*     Name      :  CheckExtEvent                                            *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Check if any events in queue and return TRUE else        *
*               :  return FALSE                                             *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR CheckExtEvent(void)
{
  return (ev_input_ext != ev_output_ext);
}

/****************************************************************************
*     Name      :  ExtEventExists                                           *
*               :                                                           *
*     Inputs    :  event - event to search                                  *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Check if any of the specified event exists in queue      *
*               :  and return TRUE else  return FALSE                       *
*               :                                                           *
*---------------------------------------------------------------------------*/
UCHAR ExtEventExists(UCHAR event)
{
  UCHAR i;
  UCHAR exists = FALSE;

  i = ev_output_ext;
  while((!exists) & (i != ev_input_ext))
  {
    if ((event_queue_ext[i] & EV_INFO) == event)
      exists = TRUE;
    i++;
    i &= (EXT_EVENT_QUEUE_LEN-1);
  }

  return exists;
}

#endif

/****************************************************************************
*     Name      :  ResetGeniTimers                                          *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description :  Disable all GeniTimers by setting the interval to 0      *
*               :                                                           *
*---------------------------------------------------------------------------*/
void ResetGeniTimers(void)
{
  UCHAR i;

  for( i = 0; i < NO_GENI_TIMERS; i++)         // disable all
    geni_timers[i].interval = 0;               // GeniSys timers
}

/****************************************************************************
*     Name      : GeniSysTimer                                              *
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
*   Description : The main function for Geni system timers. This function   *
*               : update the geni timer and check for any timer events to   *
*               : generate                                                  *
*               :                                                           *
*               :                                                           *
*---------------------------------------------------------------------------*/
void GeniSysTimer(void)
{
  UCHAR i;
  GENI_DISABLE_GLOBAL_INT();
  geni_sys_timer++;                            // update system timer
  GENI_ENABLE_GLOBAL_INT();

  // check all timers for trig
  for( i = 0; i < NO_GENI_TIMERS; i++)         //
  {
    if( geni_timers[i].interval > 0 )          // timer enabled?
    {                                          //
      if( geni_timers[i].compare == geni_sys_timer )  // timeout?
      {                                        // send event
        if ( geni_timers[i].type == GENI_PUT_FIRST_TIMER )
          PutFirstEvent( geni_timers[i].event );
        else
          PutGeniEvent( geni_timers[i].event );

        GENI_DISABLE_GLOBAL_INT();
        if( geni_timers[i].type == GENI_RELOAD_TIMER )
          geni_timers[i].compare = geni_sys_timer + geni_timers[i].interval;
        else                                   // reload timer, calculate new compare
          geni_timers[i].interval = 0;         // else , deactivate timer
        GENI_ENABLE_GLOBAL_INT();
      }
    }                                          //
  }
}

/****************************************************************************
*     Name      : SetTimer                                                  *
*               :                                                           *
*     Inputs    : timer_no             // number for the timer              *
*               : interval             // the duration for the timeout- or  *
*               :                      // timer interval                    *
*               : type_event           // timeout or interval timer type    *
*               :                      // and the event to be signaled      *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Start an given timer type, either an timeout- or interval *
*               : timer. When the interval periode is elapsed the event     *
*               : signal is transmitted                                     *
*               : Be aware that setting an interval = 1 ( 5 msec ) gives    *
*               : an delay from 0 - 5 msec.                                 *
*---------------------------------------------------------------------------*/
void SetGeniTimer(UCHAR timer_no, UINT interval, UCHAR event, UCHAR type)
{
#if (DISABLE_TIMER_RETURNS_STATE == TRUE)
  UCHAR sys_timer_irq_status;
  sys_timer_irq_status = GENI_SYS_TIMER_DISABLE();                   // Disable the used system timer interrupt
#else
  GENI_SYS_TIMER_DISABLE();                                          // Disable the used system timer interrupt
#endif
  if ( timer_no < NO_GENI_TIMERS )
  {
    if( interval > 0 )                                                 // valid interval?
    {
      geni_timers[timer_no].compare    = interval + geni_sys_timer;    // calc next compare value
      geni_timers[timer_no].event      = event;                        // save event
      geni_timers[timer_no].type       = type;                         // and timer type
      geni_timers[timer_no].interval   = interval;                     // enable of timer, always at end
    }
  }
  else if (timer_no == DIRECT_TIMER)                                   // just PutEvent
  {
    if ( type == GENI_PUT_FIRST_TIMER )
      PutFirstEvent( event );
    else
      PutGeniEvent( event );
  }
  else if (timer_no == DIRECT_FIRST_TIMER)                             // just put first event
      PutFirstEvent( event );

#if (DISABLE_TIMER_RETURNS_STATE == TRUE)
  GENI_SYS_TIMER_ENABLE(sys_timer_irq_status);                         /* Enable system timer interrupt again */
                                                                       /* if it was enabled before            */
#else
  GENI_SYS_TIMER_ENABLE(1);                                            /* Enable system timer interrupt again , dummy param */
#endif
}

/****************************************************************************
*     Name      : ClearTimer                                                *
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
*   Description : Disable an given timer                                    *
*---------------------------------------------------------------------------*/
void ClearGeniTimer(UCHAR timer_no)
{
  if ( timer_no < NO_GENI_TIMERS)
  {
    geni_timers[timer_no].interval = 0;        //  This will disable the timer
  }
}
/****************************************************************************
*     Name      : GetGeniSysTimer                                           *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*               :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   : The value of the Genipro system timer                     *
*               :                                                           *
*   Description : Disable an given timer                                    *
*---------------------------------------------------------------------------*/
UINT GetGeniSysTimer(void)
{
  return (geni_sys_timer);
}

/****************************************************************************
*     Name      : EmptyFunc                                                 *
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
*   Description : Dummy function for use in structures                      *
*---------------------------------------------------------------------------*/
void EmptyFunc(void)
{}

#if (CTO_BUS_TYPE == Slave)
#define SET_BAUD_RATE_ID 39
#define C5_BUF_OPT_MASK (3*1024)
/****************************************************************************
*     Name      : GeniSetActBusBaudrate                                     *
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
*   Description : Function to call in GENI when Bus Baudrate should be set  *
*---------------------------------------------------------------------------*/
void GeniSetActBusBaudrate(void)
{
  if ((set_baud_rate >= GENI_BAUD_9600) && (set_baud_rate <= GENI_BAUD_38400))
  {
    GeniOpenChannel(BUS_CH_INDX, GENI_SETUP_PARAM(OFF, set_baud_rate, GENI_DATA_NO_PARITY, GENI_DATA_ONE_STOP_BIT ));
  }
}

/****************************************************************************
*     Name      : GeniPresetActBusBaudrate                                  *
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
*   Description : Function to call in GENI when Bus Baudrate should be set  *
*---------------------------------------------------------------------------*/
void GeniPresetActBusBaudrate(void)
{
  if ((baud_rate_default >= GENI_BAUD_9600) && (baud_rate_default <= GENI_BAUD_38400))
  {
#if ((Buf_opt_ctr & C5_BUF_OPT_MASK) == C5_da_mem)
    set_baud_rate = baud_rate_default;
    GeniSetActBusBaudrate();
#elif ((Buf_opt_ctr & C5_BUF_OPT_MASK) == C5_id_da_buf)
#if (REF_BUF_LEN < 2)
  #error REF_BUF_LEN NEEDS TO BE MINIMUM TWO
#endif
#if (CTO_BUF_INSERT_CH_INDX == Enable)
    ref_buf[ref_buf[WR_INDX]++] = BUS_CH_INDX;
#endif
    ref_buf[ref_buf[WR_INDX]++] = SET_BAUD_RATE_ID;
    ref_buf[ref_buf[WR_INDX]++] = baud_rate_default;
    ch_interface[BUS_CH].PostUserFct(); // To make the application read the ref_buf
#elif ((Buf_opt_ctr & C5_BUF_OPT_MASK) == C5_da_mem_ad_buf) // C5_da_mem_ad_buf
#if (REF_BUF_LEN < 2)
  #error REF_BUF_LEN NEEDS TO BE MINIMUM TWO
#endif
    ref_buf[ref_buf[WR_INDX]++] = HI_val(set_baud_rate);
    ref_buf[ref_buf[WR_INDX]++] = LO_val(set_baud_rate);
#else
  #error "BAD SETTING"
#endif
  }
}

#endif


/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
