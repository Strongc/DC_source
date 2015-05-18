/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIpro                                         */
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
/* MODULE NAME      :    bus_drv_l.c                                        */
/*                                                                          */
/* FILE NAME        :    bus_drv_l.c                                        */
/*                                                                          */
/* FILE DESCRIPTION :    Channel driver for K0 micros                       */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "typedef.h"
#include "geni_cnf.h"             /* Access to GENIpro configuration        */
#include "MicroP.h"               /* Access to global interrupt             */
#include "common.h"               /* Access to common definitions           */
#include "geni_rtos_if.h"         /* Access to RTOS                         */
#include "bus_drv_if.h"           /* Access to Driver interface functions   */
#include "bus_drv_l.h"            /* Access to itself                       */

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
#define CH_NO BUS_CH>>6
#if(CTO_MOD_BUS == Enable)
#define SAVE_RX_BYTE(ch_indx, rx_byte)                                  \
{                                                                       \
   if(modbus_on)                                                        \
   {                                                                    \
     SaveModBusRxByte(ch_indx, rx_byte);                                \
   }                                                                    \
   else                                                                 \
   {                                                                    \
     SaveRxByte(ch_indx, rx_byte);                                      \
   }                                                                    \
};

#define CHANNEL_BUSY(ch_indx, busy_flg)                                 \
{                                                                       \
   if(modbus_on)                                                        \
   {                                                                    \
     GeniModBusChannelBusy(ch_indx, busy_flg);                          \
   }                                                                    \
   else                                                                 \
   {                                                                    \
     GeniChannelBusy(ch_indx, busy_flg);                                \
   }                                                                    \
};

#define SEND_TX_BYTE(ch_indx)                                           \
{                                                                       \
  if(modbus_on)                                                         \
  {                                                                     \
    SendModBusTxByte(ch_indx);                                          \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    SendTxByte(ch_indx);                                                \
  }                                                                     \
};

#else
#define SAVE_RX_BYTE(ch_indx, rx_byte){ SaveRxByte(ch_indx, rx_byte); };
#define CHANNEL_BUSY(ch_indx, busy_flg){ GeniChannelBusy(ch_indx, busy_flg); };
#define SEND_TX_BYTE(ch_indx){ SendTxByte(ch_indx); };

#endif
#define PLM_1ST_PREAMBLE                 0xAA
#define PLM_2ND_PREAMBLE                 0xFF


/****************************************************************************/
/*                                                                          */
/* V A R I A B L E S                                                        */
/*                                                                          */
/****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif

  static BIT channel_busy;
  static volatile BIT modbus_on;                          // volatile to avoid warning

#if (BUS_IDLE_TYPE == TIMER_IDLE )
  #if(BUS_IDLE_TIMER == SOFT)
    BIT BUS_soft_timer_started;                            // flag used when timer idle is soft
  #endif
  static UCHAR geni_idle_counter;                             // counter used when timer idle is active
#elif (BUS_IDLE_TYPE == GENI_IRQ_IDLE )
  BIT BUS_soft_timer_started;                            // flag used when timer idle is soft
  BIT BUS_geni_irq_idle;
#endif

#if(CHANNEL == PLM_CHANNEL)
  static BIT preamble;
  #if ( PLM_MDM_ENHANCED_CTR == Enable)
  BIT mdm_busy;
  UCHAR enable_reg_access;
  static UCHAR bit_cnt;
  ULONG mdm_reg_val;
  #endif
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
#if(BUS_IDLE_TYPE == HW_IDLE)
  static void InitHwIdle(void);
#elif(BUS_IDLE_TYPE == TIMER_IDLE)
  static void InitIdleTimer(void);
#endif
/****************************************************************************
*     Name      : InitBUSDriver                                             *
*     Inputs    :                                                           *
*     Outputs   : None                                                      *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Perform initialisation of the genibus hardware            *
*               : Select RS485 hardware driver as default                   *
*---------------------------------------------------------------------------*/
void InitBUSDriver(UCHAR p_setup_param)
{
  UCHAR stop_bits = 0;
  UCHAR parity    = 0;
  UCHAR baudrate  = 0;

  GENI_DISABLE_UART;

  modbus_on = (p_setup_param >> 7) & 0x01;
  baudrate  = (p_setup_param >> 4) & 0x07;
  parity    = (p_setup_param >> 2) & 0x03;
  stop_bits = (p_setup_param >> 0) & 0x01;

  GENI_DIR_SETUP;                                           // Setup RS485 direction control
  GENI_DIR_INPUT;                                           // set the RS485 channel to input

  channel_busy = FALSE;                                     // indicate that the channel is NOT busy
  CHANNEL_BUSY(CH_NO, channel_busy);                        // signal that the channel is NOT busy

  GENI_INIT_UART(baudrate, stop_bits, parity, GENI_DATA_8_BIT);    // init uart
  GENI_SETUP_UART_FOR_RX;

  #if(BUS_IDLE_TYPE == HW_IDLE)
    InitHwIdle();
  #elif(BUS_IDLE_TYPE == TIMER_IDLE)
    InitIdleTimer();                                        // initialise port and timer for timer idle
  #endif
  #if ((CHANNEL == PLM_CHANNEL) && ( PLM_MDM_ENHANCED_CTR == Enable))
  REG_DATA_PIN_SETUP;
  mdm_busy = FALSE;
  #endif

}
/****************************************************************************
*     Name      : DisableBUSDriver                                          *
*               :                                                           *
*     Inputs    :                                                           *
*               :                                                           *
*     Outputs   : None                                                      *
*     Updates   :                                                           *
*     Returns   :                                                           *
*               :                                                           *
*   Description : Performs disablingof the genibus hardware                 *
*               :                                                           *
*---------------------------------------------------------------------------*
* Date:     Changes:
*
*
*/
void DisableBUSDriver(void)
{
  modbus_on = OFF;
  GENI_DISABLE_UART;
}
/****************************************************************************
*     Name      :  ResetBUSDriver                                           *
*     Inputs    :  none                                                     *
*     Outputs   :  none                                                     *
*     Updates   :  transmission state                                       *
*     Returns   :                                                           *
*   Description :  Reset Geni driver low level settings for receiving       *
*               :  the next tgm.                                            *
*---------------------------------------------------------------------------*/
void ResetBUSDriver(void)
{
  GENI_DISABLE_UART;

  GENI_DIR_INPUT;                                           // set direction to input
  channel_busy = FALSE;                                     // indicate that the channel is NOT busy
  CHANNEL_BUSY(CH_NO, channel_busy);                        // signal that the channel is NOT busy
  #if (( PLM_MDM_ENHANCED_CTR == Enable ) && (CHANNEL == PLM_CHANNEL) )
  mdm_busy = FALSE;
  #endif

  GENI_SETUP_UART_FOR_RX;
}
/****************************************************************************
*     Name      : TransmitSetupBUS                                          *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Set UART to transmit mode                                 *
*---------------------------------------------------------------------------*/
UCHAR TransmitSetupBUS(void)
{
  UCHAR ret_code = FALSE;
  #if(CHANNEL != PLM_CHANNEL)
  if (channel_busy == FALSE)
  #endif

  {
    GENI_DISABLE_UART;
    GENI_DIR_OUTPUT;                                        // set the RS485 channel to output
    #if(CHANNEL == PLM_CHANNEL)
    preamble = TRUE;
    #endif

    GENI_SETUP_UART_FOR_TX;
    GENI_TXD_START;
    ret_code = TRUE;
  }
  #if(BUS_IDLE_TYPE != NO_IDLE)
    channel_busy = TRUE;                                    // indicate that the channel is busy
    CHANNEL_BUSY(CH_NO, channel_busy);                      // signal that the channel is busy
  #endif

  return ret_code;
}
/****************************************************************************
*     Name      : TransmitBUSByte                                           *
*     Inputs    : Byte to send                                              *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Initiate transmission                                     *
*                 Send byte to the channel                                  *
*---------------------------------------------------------------------------*/
void TransmitBUSByte(UCHAR tx_byte)
{
  GENI_TXD = tx_byte;
}

/****************************************************************************
*     Name      : GeniBUSOutIrq                                             *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Output the next byte to bus until end of telegram         *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
  #pragma vector = Tx_Vector
  __interrupt void GeniBUSOutIrq(void)

#elif (defined K0)
  static GENI_OUT_IRQ GeniBUSOutIrq(void)

#elif ((defined MIPS) || (defined PC))
  void GeniBUSOutIrq(void)

#endif
{
  GENI_ENTER_NESTABLE_INT();                                // allow interrupts
  #if(CHANNEL == PLM_CHANNEL)
  if ( preamble )
  {
     preamble = FALSE;
     TransmitBUSByte(PLM_2ND_PREAMBLE);
  }
  else
  #endif
  {
  SEND_TX_BYTE(CH_NO);                                      // get next data
  }
  GENI_DISABLE_GLOBAL_INT();                                // protect variable update
  GENI_REFRESH_TX_IDLE;                                     // refresh idle detection
  GENI_ENABLE_GLOBAL_INT();                                 // procect variable update

  GENI_LEAVE_NESTABLE_INT();

}

/****************************************************************************
*     Name      : GeniBUSInIrq                                              *
*     Inputs    : none                                                      *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Save the received byte from the channel                   *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
  #pragma vector = Rx_Vector
  __interrupt void GeniBUSInIrq(void)

#elif (defined K0)
  static GENI_IN_IRQ GeniBUSInIrq(void)

#elif ((defined MIPS) || (defined PC))
  void GeniBUSInIrq(void)

#endif
{
  GENI_RXD_DISABLE;
  GENI_ENTER_NESTABLE_INT();                                // allow interrupts

  #if(BUS_IDLE_TYPE != NO_IDLE)
    channel_busy = TRUE;                                    // indicate that the channel is busy
    CHANNEL_BUSY(CH_NO, channel_busy);                      // signal that the channel is busy
  #endif

  SAVE_RX_BYTE(CH_NO, GENI_RXD);                              // get data from the uart

  GENI_DISABLE_GLOBAL_INT();                                // protect variable update
  GENI_REFRESH_IDLE;                                        // refresh idle detection
  GENI_ENABLE_GLOBAL_INT();                                 // procect variable update

  GENI_LEAVE_NESTABLE_INT();
  GENI_RXD_ENABLE;
}
/****************************************************************************
*     Name      : GeniBUSErrIrq                                             *
*     Inputs    : none                                                      *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description : Receive error - reads RXD and clears errors               *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
#if (defined Rx_Err_Vector)
#pragma vector = Rx_Err_Vector
__interrupt void GeniBUSErrIrq(void)
#else
void GeniBUSErrIrq(void)
#endif

#elif (defined K0)
  static GENI_ERR_IRQ GeniBUSErrIrq(void)

#elif ((defined MIPS) || (defined PC))
  void GeniBUSErrIrq(void)

#endif
{
  if ( GENI_RXD != 0 )
  {
      GENI_CLEAR_RX_ERR;
  }
  GENI_CLEAR_RX_ERR;
}

#if(IDLE_TYPE == HW_IDLE)
/****************************************************************************
*     Name      :  InitHwIdle                                               *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Sets the hw idle input pin up                            *
*               :                                                           *
*---------------------------------------------------------------------------*/
static void InitHwIdle(void)
{
  SET_IDLE_EDGE_TRIG;                                       // set edge triggering to rising edge trig
  GENI_IDLE_IRQ_ENABLE;                                     // enable idle interrupt
}
/****************************************************************************
*     Name      :  GeniBUSLineIdleIrq                                       *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Prepare receive of next telegram by calling              *
*               :  GeniChannelBusy()                                        *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
  #pragma vector = GENI_IDLE_IRQ_Vector
  __interrupt void GeniBUSLineIdleIrq(void)

#elif (defined K0)
  static GENI_IDLE_IRQ GeniBUSLineIdleIrq(void)

#elif ((defined MIPS) || (defined PC))
  void GeniBUSLineIdleIrq(void)

#endif
{
  GENI_ENTER_NESTABLE_INT();                                // allow interrupts
  channel_busy = FALSE;                                     // indicate that the channel is NOT busy
  CHANNEL_BUSY(CH_NO, channel_busy);                        // signal that the channel is NOT busy
  GENI_LEAVE_NESTABLE_INT();
}
#endif  // GENI_IDLE_TYPE == HW_IDLE

#if(IDLE_TYPE == TIMER_IDLE)
/****************************************************************************
*     Name      :  InitIdleTimer                                 *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :  none                                                     *
*     Returns   :                                                           *
*   Description :  this procedure initiates the ports and timers used       *
*               :  with TIMER_IDLE                                          *
*---------------------------------------------------------------------------*/
static void InitIdleTimer(void)
{
  SET_ISC0_REG;
  GENI_IDLE_TIMER_SETUP;
  GENI_IDLE_TIMER_INT_PRIO_LOW;                             // Low priority for timer interrupt
  GENI_IDLE_TIMER_IRQ_ENABLE;                               // Enable interrupt
  SET_PORT_EDGE_TRIG;                                       // Set falling edges for interrupt
  GENI_INT_PRIO_LOW;                                        // Low priority for port interrupt
  GENI_IDLE_IRQ_ENABLE;                                     // Enable interrupt on port change
}
/****************************************************************************
*     Name      :  GeniBUSIdleTimerIrq                                      *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :  none                                                     *
*     Returns   :                                                           *
*   Description :  this procedure is called every time  the idle timer      *
*               :  expires                                                  *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
  #if ( BUS_IDLE_TIMER == SOFT )
    void GeniBUSIdleTimerIrq(void)

  #else
    #pragma vector = GENI_IDLE_TIMER_IRQ
    __interrupt void GeniBUSIdleTimerIrq(void)

  #endif
#elif (defined K0)
  GENI_IDLE_TIMER_IRQ GeniBUSIdleTimerIrq(void)

#elif ((defined MIPS) || (defined PC))
  void GeniBUSIdleTimerIrq(void)

#endif
{
  GENI_ENTER_NESTABLE_INT();                                // allow interrupts

  GENI_DISABLE_GLOBAL_INT();                                // procect variable update
  if (geni_idle_counter == 0)                               // last pass or not?
  {
    geni_idle_counter--;                                    // dec idlecount.
    GENI_ENABLE_GLOBAL_INT();                               // protect variable update

    GENI_IDLE_TIMER_STOP;                                   // Assign the timer to stop after next FF to 00 change

    channel_busy = FALSE;                                   // indicate that the channel is NOT busy
    CHANNEL_BUSY(CH_NO, channel_busy);                      // signal that the channel is NOT busy
  }
  else
  {
    geni_idle_counter--;                                    // dec idlecount.
    GENI_ENABLE_GLOBAL_INT();                               // procect variable update
  }
  GENI_LEAVE_NESTABLE_INT();
}
#if (BUS_IDLE_PIN != GENI_NOT_USED)
/****************************************************************************
*     Name      :  GeniBUSLineBusyIrq                                       *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :  none                                                     *
*     Returns   :                                                           *
*   Description :  this procedure is called when channel activity is sensed *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
  #pragma vector = SOFT_IDLE_IRQ_Vector
  __interrupt void GeniBUSLineBusyIrq(void)

#elif (defined K0)
  static SOFT_IDLE_IRQ GeniBUSLineBusyIrq(void)

#elif ((defined MIPS) || (defined PC))
  void GeniBUSLineBusyIrq(void)

#endif
{
  GENI_ENTER_NESTABLE_INT();                                // allow interrupts

  GENI_IDLE_IRQ_DISABLE;                                    // disable further interrupts on port
  channel_busy = TRUE;                                      // indicate that the channel is busy
  CHANNEL_BUSY(CH_NO, channel_busy);                        // signal that the channel is busy

  GENI_DISABLE_GLOBAL_INT();                                // protect variable update
  GENI_REFRESH_IDLE;                                        // refresh idle detection
  GENI_ENABLE_GLOBAL_INT();                                 // procect variable update


  GENI_LEAVE_NESTABLE_INT();
}
#endif // BUS_IDLE_PIN != GENI_NOT_USED
#endif // GENI_IDLE_TYPE == TIMER_IDLE

#if(IDLE_TYPE == GENI_IRQ_IDLE)
/****************************************************************************
*     Name      :  GeniBUSIdleTimerIrq                                      *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :  BUS_soft_timer_started, channel_busy, BUS_geni_irq_idle  *
*     Returns   :                                                           *
*   Description :  this procedure is called every time  the GeniSysIrq is   *
*               :  called                                                   *
*---------------------------------------------------------------------------*/
void GeniBUSIdleTimerIrq(void)
{
  if (BUS_soft_timer_started == TRUE)
  {
    if (BUS_geni_irq_idle == TRUE)                              // we didn't receive any since last time
    {
      BUS_soft_timer_started = FALSE;                           // Stop the checking
      channel_busy = FALSE;                                     // indicate that the channel is busy
      CHANNEL_BUSY(CH_NO, channel_busy);                        // signal that the channel is busy
    }
    BUS_geni_irq_idle = TRUE;
  }
}
#endif

#if (( PLM_MDM_ENHANCED_CTR == Enable) && (CHANNEL == PLM_CHANNEL))
/****************************************************************************
*     Name      :  PLMSetMDMRegister                                        *
*     Inputs    :                                                           *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Starts the storing of the value of mdm_reg_val  in the   *
*               :  powerline modem.                                         *
*---------------------------------------------------------------------------*/
void PLMSetMDMRegister(void)
{
  mdm_busy = TRUE;
  bit_cnt = FIRST_BIT_NO;                                   // set first bit no. to send
  SETUP_MDM_SYNC_PINS;
  //ENABLE_TX_MODE;                                           // set direction pin to output
  enable_reg_access = 2;
  GENI_MDM_CLK_IRQ_ENABLE;
}
/****************************************************************************
*     Name      :  PLMGetMDMRegister                                        *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Gets the value of hte register in the powerline modem    *
*               :                                                           *
*---------------------------------------------------------------------------*/
void PLMGetMDMRegister(void)
{
  mdm_busy = TRUE;
  bit_cnt = FIRST_BIT_NO;                                   // set first bit no. to recieve
  mdm_reg_val = 0;                                          // reset value
  SETUP_MDM_SYNC_PINS;
  //ENABLE_RX_MODE;                                           // set direction pin to input
  enable_reg_access = 1;
  GENI_MDM_CLK_IRQ_ENABLE;
}
/****************************************************************************
*     Name      :  PLMSyncClockIrq                                          *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :                                                           *
*     Returns   :                                                           *
*   Description :  Sets the bitvalue to Tx line on falling edge of clock    *
*               :  or reads the bitvalue of the Rx line on rising edge      *
*---------------------------------------------------------------------------*/
#if ((defined V850E) || (defined V850ES))
#pragma vector = GENI_Sync_IRQ_Vector
__interrupt void PLMSyncClockIrq(void)

#elif (defined K0)
GENI_SYNC_IRQ PLMSyncClockIrq(void)

#endif
{
  if ( enable_reg_access != 0)
  {
    ENABLE_REG_ACCESS;                                        // enable IRQ and set REG_DATA flg
    if ( enable_reg_access == 1)
    {
      ENABLE_RX_MODE;
    }
    else
    {
      if ( (mdm_reg_val & ((ULONG)1 << bit_cnt)) > 0)
        TX_LINE = 1;
      else
        TX_LINE = 0;
      bit_cnt--;                                                // decrement bit no. to send
      ENABLE_TX_MODE;
    }

    enable_reg_access = 0;
  }
  else
  {
    if ( (bit_cnt) == 255)                                      // last bit?
    {
      DISABLE_REG_ACCESS;                                       // clear reg access flag and disable interrupt
      GENI_MDM_CLK_IRQ_DISABLE;
      mdm_busy = FALSE;
      ENABLE_RX_MODE;
    }
    else
    {
      if (GENI_DIR_OUTPUT_MODE)                                 // transmit or receive
      {
        if ( (mdm_reg_val & ((ULONG)1 << bit_cnt)) > 0 )        // shift the bit to send down to LSB and set it to the pin
          TX_LINE = 1;
        else
          TX_LINE = 0;
      }
      else
      {
        if ( RX_LINE > 0)
          mdm_reg_val |= ((ULONG)1 << bit_cnt);                 // shift the pin value up to the current bit and 'or' it
      }
    }
    bit_cnt--;                                                  // decrement the bit pointer
  }
}
#endif

/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
