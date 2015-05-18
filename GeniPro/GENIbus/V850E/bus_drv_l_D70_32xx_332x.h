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
/* MODULE NAME      :   bus_drv_l_D70_32xx_332x.h                           */
/*                                                                          */
/* FILE NAME        :   bus_drv_l_D70_32xx_332x.h                           */
/*                                                                          */
/* FILE DESCRIPTION :   Defines for the c file                              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _BUS_DRV_L_H
#define _BUS_DRV_L_H

#include "hw_res.h"
#include "hw_cnf.h"

/*****************************************************************************/
/* Specify the channel                                                       */
/*****************************************************************************/
#define CH_SPEC BUS                                       // BUS, COM, RS232, PLM

/*****************************************************************************/
/* The following macroes will adapt the rest of the file to the specified    */
/* channel.                                                                  */
/* Example:                                                                  */
/*  #define CH_SPEC   BUS                                                    */
/*  => BAUDRATE will be replaced by BUS_BAUDRATE                             */
/*  and so on.                                                               */
/*****************************************************************************/
#define M_IDLE_COUNT_VAL(a)           IDLE_COUNT_VAL_##a
#define S_IDLE_COUNT_VAL(tmr, baud)   M_IDLE_COUNT_VAL(tmr##_##baud)
#define IDLE_COUNT_VAL(tmr,baud)      S_IDLE_COUNT_VAL(tmr,baud)

#define M_CHANNEL(a)                  a##_CHANNEL
#define S_CHANNEL(a)                  M_CHANNEL(a)
#define CHANNEL                       S_CHANNEL(CH_SPEC)

#define M_BAUDRATE(a)   a##_BAUDRATE
#define S_BAUDRATE(a)   M_BAUDRATE(a)
#define BAUDRATE        S_BAUDRATE(CH_SPEC)

#define M_UART_NUM(a)   a##_UART_NUM
#define S_UART_NUM(a)   M_UART_NUM(a)
#define UART_NUM        S_UART_NUM(CH_SPEC)

#define M_UART_PRIORITY(a)   a##_UART_PRIORITY
#define S_UART_PRIORITY(a)   M_UART_PRIORITY(a)
#define UART_PRIORITY        S_UART_PRIORITY(CH_SPEC)

#define M_DIR_CTR(a)    a##_DIR_CTR
#define S_DIR_CTR(a)    M_DIR_CTR(a)
#define DIR_CTR         S_DIR_CTR(CH_SPEC)

#define M_DIR_PIN(a)    a##_DIR_PIN
#define S_DIR_PIN(a)    M_DIR_PIN(a)
#define DIR_PIN         S_DIR_PIN(CH_SPEC)

#define M_IDLE_TYPE(a)  a##_IDLE_TYPE
#define S_IDLE_TYPE(a)  M_IDLE_TYPE(a)
#define IDLE_TYPE       S_IDLE_TYPE(CH_SPEC)

#define M_IDLE_PIN(a)   a##_IDLE_PIN
#define S_IDLE_PIN(a)   M_IDLE_PIN(a)
#define IDLE_PIN        S_IDLE_PIN(CH_SPEC)

#define M_IDLE_TIMER(a) a##_IDLE_TIMER
#define S_IDLE_TIMER(a) M_IDLE_TIMER(a)
#define IDLE_TIMER      S_IDLE_TIMER(CH_SPEC)

#define M_IDLE_COUNT(a) a##_IDLE_COUNT
#define S_IDLE_COUNT(a) M_IDLE_COUNT(a)
#define IDLE_COUNT      S_IDLE_COUNT(CH_SPEC)

#define M_soft_timer_started(a)   a##_soft_timer_started
#define S_soft_timer_started(a)   M_soft_timer_started(a)
#define soft_timer_started        S_soft_timer_started(CH_SPEC)

#define M_geni_irq_idle(a)            a##_geni_irq_idle
#define S_geni_irq_idle(a)            M_geni_irq_idle(a)
#define geni_irq_idle                 S_geni_irq_idle(CH_SPEC)

#define M_MDM_SYNC_CLK_PIN(a)         a##_MDM_SYNC_CLK_PIN
#define S_MDM_SYNC_CLK_PIN(a)         M_MDM_SYNC_CLK_PIN(a)
#define MDM_SYNC_CLK_PIN              S_MDM_SYNC_CLK_PIN(CH_SPEC)

#define M_MDM_REG_DATA_PIN(a)         a##_MDM_REG_DATA_PIN
#define S_MDM_REG_DATA_PIN(a)         M_MDM_REG_DATA_PIN(a)
#define MDM_REG_DATA_PIN              S_MDM_REG_DATA_PIN(CH_SPEC)


/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Setup values of baudrate control registers                               */
/*****************************************************************************/
#if ( BAUDRATE == 1200)
  #define BAUD_CLK_VAL     BAUD_CLK_VAL_1200
  #define BAUDRATE_VAL     BAUDRATE_VAL_1200
#elif ( BAUDRATE == 2400)
  #define BAUD_CLK_VAL     BAUD_CLK_VAL_2400
  #define BAUDRATE_VAL     BAUDRATE_VAL_2400
#elif ( BAUDRATE == 4800)
  #define BAUD_CLK_VAL     BAUD_CLK_VAL_4800
  #define BAUDRATE_VAL     BAUDRATE_VAL_4800
#elif ( BAUDRATE == 9600)
  #define BAUD_CLK_VAL     BAUD_CLK_VAL_9600
  #define BAUDRATE_VAL     BAUDRATE_VAL_9600
#elif ( BAUDRATE == 19200)
  #define BAUD_CLK_VAL     BAUD_CLK_VAL_19200
  #define BAUDRATE_VAL     BAUDRATE_VAL_19200
#elif ( BAUDRATE == 38400)
  #define BAUD_CLK_VAL     BAUD_CLK_VAL_38400
  #define BAUDRATE_VAL     BAUDRATE_VAL_38400
#else
  #error Unsupported baudrate value
#endif

/*****************************************************************************/
/*  Genichannel hardware configuration:                                      */
/*****************************************************************************/
#if (UART_NUM  == 0)
  #define Rx_Vector              RXD_INT_CH0
  #define Tx_Vector              TXD_INT_CH0
  #define Rx_Err_Vector          ERR_INT_CH0

  #define GENI_RXD               RXB_CH0                                        // Uart read adr.
  #define GENI_TXD               TXB_CH0                                        // Uart transmit adr.

  #define GENI_TXD_ENABLE        ENABLE_TXD_IRQ_CH0
  #define GENI_TXD_DISABLE       DISABLE_TXD_IRQ_CH0
  #define GENI_RXD_ENABLE        ENABLE_RXD_IRQ_CH0
  #define GENI_RXD_DISABLE       DISABLE_RXD_IRQ_CH0

  #if(CHANNEL == PLM_CHANNEL)
    #define GENI_TXD_START       GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
    #define GENI_TXD_START       INT_PEN_TX_CH0 = 1
  #endif
  #define GENI_RXD_MODE          RXD_MODE_CH0                                   //  select channel 0 receive mode bit
  #define GENI_TXD_MODE          TXD_MODE_CH0                                   //  select channel 0 transmit mode bit
  #define GENI_RX_LINE           RXD_LINE_CH0
  #define GENI_TX_LINE           TXD_LINE_CH0
  #define GENI_SETUP_RXD_AS_IO   SETUP_RXD_AS_IO_CH0
  #define GENI_SETUP_TXD_AS_IO   SETUP_TXD_AS_IO_CH0
  #define GENI_CLEAR_RX_ERR      CLEAR_RX_ERR_CH0
  #define GENI_RX_ERR_FLG        RX_ERR_FLAG_CH0

  #define GENI_UART_STOP         DISABLE_UART0

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                   \
  {                                                                                                                           \
                                  SETUP_RXD_CH0;                  /* setting the port mode      */              \
                                  SETUP_TXD_CH0;                 /* setting the port mode      */              \
                                  GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                  GENI_BAUD_INIT_0(baud);                                                                     \
                                  INT_PRI_RX_CH0(UART_PRIORITY);            \
                                  INT_PRI_TX_CH0(UART_PRIORITY);            \
                                  FRAME_SETUP_CH0(stop_bits, parity, GENI_DATA_8_BIT);                                        \
  }

  #define GENI_SETUP_UART_FOR_RX                                                                                              \
  {                                                                                                                           \
                                  GENI_RXD_ENABLE;                              /* enable receive interrupt */                \
                                  GENI_IDLE_IRQ_ENABLE;                         /* Enable interrupt on port change */         \
  }


  #define GENI_SETUP_UART_FOR_TX    {GENI_IDLE_TIMER_STOP; GENI_TXD_ENABLE;}

  #define GENI_DISABLE_UART                                       \
  {                                                               \
                                    GENI_RXD_DISABLE;             \
                                    GENI_TXD_DISABLE;             \
                                    GENI_IDLE_IRQ_DISABLE;        \
  }


  #define GENI_BAUD_INIT_0(baud)                                                                            \
  {                                                                                                         \
    switch (baud)                                                                                           \
    {                                                                                                       \
      case GENI_BAUD_1200  : BAUD_CLK_REG_CH0 = BAUD_CLK_VAL_1200; BAUDRATE_REG_CH0 = BAUDRATE_VAL_1200;    \
      break;                                                                                                \
      case GENI_BAUD_2400  : BAUD_CLK_REG_CH0 = BAUD_CLK_VAL_2400; BAUDRATE_REG_CH0 = BAUDRATE_VAL_2400;    \
      break;                                                                                                \
      case GENI_BAUD_4800  : BAUD_CLK_REG_CH0 = BAUD_CLK_VAL_4800; BAUDRATE_REG_CH0 = BAUDRATE_VAL_4800;    \
      break;                                                                                                \
      case GENI_BAUD_9600  : BAUD_CLK_REG_CH0 = BAUD_CLK_VAL_9600; BAUDRATE_REG_CH0 = BAUDRATE_VAL_9600;    \
      break;                                                                                                \
      case GENI_BAUD_19200  : BAUD_CLK_REG_CH0 = BAUD_CLK_VAL_19200; BAUDRATE_REG_CH0 = BAUDRATE_VAL_19200; \
      break;                                                                                                \
      case GENI_BAUD_38400  : BAUD_CLK_REG_CH0 = BAUD_CLK_VAL_38400; BAUDRATE_REG_CH0 = BAUDRATE_VAL_38400; \
      break;                                                                                                \
      default              :                                                                                \
      break;                                                                                                \
    }                                                                                                       \
  }

#elif (UART_NUM  == 1)
  #define Rx_Vector              RXD_INT_CH1
  #define Tx_Vector              TXD_INT_CH1
  #define Rx_Err_Vector          ERR_INT_CH1

  #define GENI_RXD               RXB_CH1                                        // Uart read adr.
  #define GENI_TXD               TXB_CH1                                        // Uart transmit adr.

  #define GENI_TXD_ENABLE        ENABLE_TXD_IRQ_CH1
  #define GENI_TXD_DISABLE       DISABLE_TXD_IRQ_CH1
  #define GENI_RXD_ENABLE        ENABLE_RXD_IRQ_CH1
  #define GENI_RXD_DISABLE       DISABLE_RXD_IRQ_CH1
  #if(CHANNEL == PLM_CHANNEL)
    #define GENI_TXD_START       GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
    #define GENI_TXD_START       INT_PEN_TX_CH1 = 1
  #endif
  #define GENI_RXD_MODE          RXD_MODE_CH1                                   //  select channel 1 receive mode bit
  #define GENI_TXD_MODE          TXD_MODE_CH1                                   //  select channel 1 transmit mode bit
  #define GENI_RX_LINE           RXD_LINE_CH1
  #define GENI_TX_LINE           TXD_LINE_CH1
  #define GENI_SETUP_RXD_AS_IO   SETUP_RXD_AS_IO_CH1
  #define GENI_SETUP_TXD_AS_IO   SETUP_TXD_AS_IO_CH1
  #define GENI_CLEAR_RX_ERR      CLEAR_RX_ERR_CH1
  #define GENI_RX_ERR_FLG        RX_ERR_FLAG_CH1

  #define GENI_UART_STOP         DISABLE_UART1

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                   \
  {                                                                                                                           \
                                  SETUP_RXD_CH1;                  /* setting the port mode      */              \
                                  SETUP_TXD_CH1;                 /* setting the port mode      */              \
                                  GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                  GENI_BAUD_INIT_1(baud);                                                                     \
                                  INT_PRI_RX_CH1(UART_PRIORITY);            \
                                  INT_PRI_TX_CH1(UART_PRIORITY);            \
                                  FRAME_SETUP_CH1(stop_bits, parity, GENI_DATA_8_BIT);                                        \
  }

  #define GENI_SETUP_UART_FOR_RX                                                                                              \
  {                                                                                                                           \
                                  GENI_RXD_ENABLE;                              /* enable receive interrupt */                \
                                  GENI_IDLE_IRQ_ENABLE;                         /* Enable interrupt on port change */         \
  }


  #define GENI_SETUP_UART_FOR_TX    {GENI_IDLE_TIMER_STOP; GENI_TXD_ENABLE;}

  #define GENI_DISABLE_UART                                       \
  {                                                               \
                                    GENI_RXD_DISABLE;             \
                                    GENI_TXD_DISABLE;             \
                                    GENI_IDLE_IRQ_DISABLE;        \
  }


  #define GENI_BAUD_INIT_1(baud)                                                                            \
  {                                                                                                         \
    switch (baud)                                                                                           \
    {                                                                                                       \
      case GENI_BAUD_1200  : BAUD_CLK_REG_CH1 = BAUD_CLK_VAL_1200; BAUDRATE_REG_CH1 = BAUDRATE_VAL_1200;    \
      break;                                                                                                \
      case GENI_BAUD_2400  : BAUD_CLK_REG_CH1 = BAUD_CLK_VAL_2400; BAUDRATE_REG_CH1 = BAUDRATE_VAL_2400;    \
      break;                                                                                                \
      case GENI_BAUD_4800  : BAUD_CLK_REG_CH1 = BAUD_CLK_VAL_4800; BAUDRATE_REG_CH1 = BAUDRATE_VAL_4800;    \
      break;                                                                                                \
      case GENI_BAUD_9600  : BAUD_CLK_REG_CH1 = BAUD_CLK_VAL_9600; BAUDRATE_REG_CH1 = BAUDRATE_VAL_9600;    \
      break;                                                                                                \
      case GENI_BAUD_19200  : BAUD_CLK_REG_CH1 = BAUD_CLK_VAL_19200; BAUDRATE_REG_CH1 = BAUDRATE_VAL_19200; \
      break;                                                                                                \
      case GENI_BAUD_38400  : BAUD_CLK_REG_CH1 = BAUD_CLK_VAL_38400; BAUDRATE_REG_CH1 = BAUDRATE_VAL_38400; \
      break;                                                                                                \
      default              :                                                                                \
      break;                                                                                                \
    }                                                                                                       \
  }

#else
  #error "Invalid value of UART_NUM - the Uart channel"

#endif
/*****************************************************************************/
/* Direction pin                                                             */
/*****************************************************************************/
#if(CHANNEL == PLM_CHANNEL)                                                           // invert direction pin for PLM
  #define AS_DIR_INPUT           1                                                    // disable the output driver
  #define AS_DIR_OUTPUT          0                                                    // enable the output driver
  #define GENI_DIR_OUTPUT_MODE   TEST_PIN_LOW(DIR_PIN)
#else
  #define AS_DIR_INPUT           0                                                    // disable the output driver
  #define AS_DIR_OUTPUT          1                                                    // enable the output driver
  #define GENI_DIR_OUTPUT_MODE   TEST_PIN_HIGH(DIR_PIN)
#endif

#if (DIR_CTR == Enable)                                                         // Direction control enabled?
  #define CLEAR_PIN_MODE1(pin)   SET_BIT_LOW(PIN_MODE1_DEF(pin),PIN_BIT_NUMBER(pin))
  #define CLEAR_PIN_MODE2(pin)   SET_BIT_LOW(PIN_MODE2_DEF(pin),PIN_BIT_NUMBER(pin))
  #define GENI_DIR_SETUP         {CLEAR_PIN_MODE2(DIR_PIN);CLEAR_PIN_MODE1(DIR_PIN);}
  #if(CHANNEL == PLM_CHANNEL)                                                           // invert direction pin for PLM
    #define GENI_DIR_OUTPUT        {CLEAR_PIN(DIR_PIN); \
                                    GENI_TX_LINE = 1;}                            // set port latch for safety
    #define GENI_DIR_INPUT         SET_PIN(DIR_PIN)
  #else
    #define GENI_DIR_OUTPUT        {SET_PIN(DIR_PIN); \
                                    GENI_TX_LINE = 1;}                            // set port latch for safety
    #define GENI_DIR_INPUT         CLEAR_PIN(DIR_PIN)
  #endif
#else
  #define GENI_DIR_OUTPUT        GENI_TX_LINE = 1                               // set port latch for safety
  #define GENI_DIR_INPUT
  #define GENI_DIR_SETUP
#endif

/*****************************************************************************/
/* HW_IDLE settings                                                          */
/*****************************************************************************/
#if( IDLE_TYPE == HW_IDLE )
  #if((CHANNEL == PLM_CHANNEL) || (defined D70332X))
    #error "Not supported"
  #endif
  #define GENI_REFRESH_IDLE
  #define GENI_IDLE_TIMER_STOP

  #if (IDLE_PIN == INTP0)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP0                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP0(ON, ON, UART_PRIORITY)  // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP0
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP0
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP0
  #elif (IDLE_PIN == INTP1)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP1                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP1(ON, ON, UART_PRIORITY)                 // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP1
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP1
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP1
  #elif (IDLE_PIN == INTP2)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP2                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP2(ON, ON, UART_PRIORITY)                 // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP2
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP2
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP2
  #elif (IDLE_PIN == INTP3)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP3                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP3(ON, ON, UART_PRIORITY)                 // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP3
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP3
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP3
  #elif (IDLE_PIN == INTP4)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP4                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP4(ON, ON, UART_PRIORITY)                 // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP4
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP4
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP4
  #elif (IDLE_PIN == INTP5)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP5                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP6(ON, ON, UART_PRIORITY)                 // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP5
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP5
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP5
  #elif (IDLE_PIN == INTP6)
    #define GENI_IDLE_PIN          IDLE_PIN_INTP6                               // Port bit used for powerline Idle detect
    #define SET_IDLE_EDGE_TRIG     SETUP_IDLE_DET_INTP6(ON, ON, UART_PRIORITY)                 // rising and falling edges
    #define GENI_IDLE_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP6
    #define GENI_IDLE_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP6
    #define GENI_IDLE_IRQ_Vector   IDLE_VECTOR_INTP6

  #else
     #error 'Invalid value of Idle_pin'
  #endif

#endif

/*****************************************************************************/
/* TIMER_IDLE settings                                                       */
/*****************************************************************************/
#if(IDLE_TYPE == TIMER_IDLE)
  #define GENI_IDLE_IRQ                void                                     // GENI_IDLE_IRQ

  #if ( IDLE_TIMER == 50)
    #define GENI_IDLE_TIMER_SETUP        {TIMER50_COMP_REG = IDLE_TIMER_COMP_VAL_50;\
                                          TIMER50_CLK_REG = IDLE_TIMER_SELECT_50;\
                                          TIMER50_MODE_REG = TIMER50_MODE_VAL;} // setup of idle timer
    #define GENI_IDLE_TIMER_START        TIMER50_ENABLE_REG = 1                 // start timer
    #define GENI_IDLE_TIMER_STOP         TIMER50_ENABLE_REG = 0                 // stop timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW INT_PRIO_TIMER50 |= INT_PRIO_LEV7      // set lowest priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE   INT_MSK_TIMER50 = 0;                   // enable interrupt for timer
    #define GENI_IDLE_TIMER_IRQ          TIMER50_IRQ                            // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL(50, BAUDRATE)           // idle count value
    #define GENI_IDLE_TIMER_MODE         TIMER50_MODE_REG                       // timer mode register

  #elif ( IDLE_TIMER == 51)
    #define GENI_IDLE_TIMER_SETUP       {TIMER51_COMP_REG = IDLE_TIMER_COMP_VAL_51;\
                                         TIMER51_CLK_REG = IDLE_TIMER_SELECT_51;\
                                         TIMER51_MODE_REG = TIMER51_MODE_VAL;}  // setup of idle timer
    #define GENI_IDLE_TIMER_START        TIMER51_ENABLE_REG = 1                 // start timer
    #define GENI_IDLE_TIMER_STOP         TIMER51_ENABLE_REG = 0                 // stop timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW INT_PRIO_TIMER51 |= INT_PRIO_LEV7      // set lowest priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE   INT_MSK_TIMER51 = 0;                   // enable interrupt for timer
    #define GENI_IDLE_TIMER_IRQ          TIMER51_IRQ                            // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL(51, BAUDRATE)           // idle count value
    #define GENI_IDLE_TIMER_MODE         TIMER51_MODE_REG                       // timer mode register

  #elif ( IDLE_TIMER == TMP3)
    #define GENI_IDLE_TIMER_SETUP        {TMP3_COMP_REG = IDLE_TIMER_COMP_VAL_TMP3;\
                                          TMP3_CLK_REG = IDLE_TIMER_SELECT_TMP3;\
                                          TMP3_MODE_REG = TMP3_MODE_VAL;} // setup of idle timer
    #define GENI_IDLE_TIMER_START        TMP3_ENABLE_REG = 1                 // start timer
    #define GENI_IDLE_TIMER_STOP         TMP3_ENABLE_REG = 0                 // stop timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW INT_PRIO_TMP3 |= INT_PRIO_LEV7      // set lowest priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE   INT_MSK_TMP3 = 0;                   // enable interrupt for timer
    #define GENI_IDLE_TIMER_IRQ          TMP3_IRQ                            // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL_TMP3                 // idle count value
    #define GENI_IDLE_TIMER_MODE         TMP3_MODE_REG                       // timer mode register

   #elif ( IDLE_TIMER == SOFT)
    //extern BIT soft_idle_timer_started;
    #define GENI_IDLE_TIMER_SETUP        soft_timer_started = FALSE             // setup of idle timer
    #define GENI_IDLE_TIMER_STOP         soft_timer_started = FALSE             // stop timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW                                        // set low priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE                                          // enable interrupt for timer
    #define GENI_IDLE_TIMER_IRQ          void                                   // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT                             // idlle count value
                                                                                // idlle time = 2,7 ms = (1/(fx/2))*256*48
  #else
      #error 'Invalid value for GENI_IDLE_TIMER'
  #endif

  #if ( IDLE_TIMER == SOFT)
    #define GENI_REFRESH_IDLE            {geni_idle_counter=GENI_IDLE_COUNT_VAL; \
                                          soft_timer_started = TRUE; \
                                          }                                     // set counter and bit to let timer count
  #else
    #define GENI_REFRESH_IDLE            {geni_idle_counter=GENI_IDLE_COUNT_VAL; \
                                         GENI_IDLE_TIMER_STOP; \
                                         GENI_IDLE_TIMER_START;}                // set counter and bit to let timer count
  #endif

  #if ( IDLE_PIN == INTP0 )                                                     // if INTP0 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP0(ON,ON , UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP0                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP0                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP0                      // timer interrupt vector

  #elif ( IDLE_PIN == INTP1 )                                                   // if INTP1 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP1(ON, ON, UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP1                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP1                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP1                      // timer interrupt vector

  #elif ( IDLE_PIN == INTP2 )                                                   // if INTP2 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP2(ON, ON, UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP2                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP2                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP2                      // timer interrupt vector

  #elif ( IDLE_PIN == INTP3 )                                                   // if INTP3 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP3(ON, ON, UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP3                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP3                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP3                      // timer interrupt vector

  #elif ( IDLE_PIN == INTP4 )                                                   // if INTP4 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP4(ON, ON, UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP4                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP4                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP4                      // timer interrupt vector

  #elif ( IDLE_PIN == INTP5 )                                                   // if INTP5 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP5(ON, ON, UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP5                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP5                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP5                      // timer interrupt vector

  #elif ( IDLE_PIN == INTP6 )                                                   // if INTP6 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG           SETUP_IDLE_DET_INTP6(ON, ON, UART_PRIORITY)          // flag for falling edge
    #define GENI_INT_PRIO_LOW                                                   // priority flag
    #define GENI_IDLE_IRQ_ENABLE         ENABLE_IDLE_IRQ_INTP6                  // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        DISABLE_IDLE_IRQ_INTP6                 // disable interrupt clear pending
    #define SOFT_IDLE_IRQ_Vector         IDLE_VECTOR_INTP6                      // timer interrupt vector

  #elif (IDLE_PIN == GENI_NOT_USED)
    #define GENI_IDLE_IRQ_DISABLE
    #define SET_PORT_EDGE_TRIG
    #define GENI_INT_PRIO_LOW
    #define GENI_IDLE_IRQ_ENABLE
    #define GENI_IDLE_IRQ_DISABLE
    #define SOFT_IDLE_IRQ_Vector
  #else
    #error 'Invalid value for GENI_idle_pin'

  #endif

 #define SET_ISC0_REG
#endif
/*****************************************************************************/
/* NO_IDLE settings                                                          */
/*****************************************************************************/

#if(IDLE_TYPE == NO_IDLE)
  #if((CHANNEL == PLM_CHANNEL) || (defined D70332X))
    #error "Not supported"
  #endif
  #define GENI_IDLE_IRQ          void
  #define GENI_IDLE_IRQ_ENABLE
  #define GENI_IDLE_IRQ_DISABLE
  #define GENI_REFRESH_IDLE
  #define GENI_IDLE_TIMER_STOP
#endif

/*****************************************************************************/
/* GENI_IRQ_IDLE settings                                                    */
/*****************************************************************************/
#if(IDLE_TYPE == GENI_IRQ_IDLE)
  #define GENI_IDLE_IRQ                 void
  #define GENI_IDLE_IRQ_ENABLE         {geni_irq_idle = FALSE;}
  #define GENI_IDLE_IRQ_DISABLE        {soft_timer_started = FALSE;}
  #define GENI_REFRESH_IDLE            {geni_irq_idle = FALSE; \
                                        soft_timer_started = TRUE; \
                                        }                                         // set counter and bit to let timer count
  #define GENI_IDLE_TIMER_STOP          GENI_IDLE_IRQ_DISABLE
#endif

#if (defined D70332X)
  #define GENI_REFRESH_TX_IDLE           GENI_REFRESH_IDLE
#else
  #define GENI_REFRESH_TX_IDLE
#endif

#if(CHANNEL == PLM_CHANNEL)
/*****************************************************************************/
/* Modem access settings                                                     */
/*****************************************************************************/
/*****************************************************************************/
/* Modem access settings - sync clock interrupt setup                        */
/*****************************************************************************/
  #if ( MDM_SYNC_CLK_PIN == INTP0 )                                                         // if int0 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP0
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP0
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP0(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP0

  #elif ( MDM_SYNC_CLK_PIN == INTP1 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP1
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP1
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP1(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP1

  #elif ( MDM_SYNC_CLK_PIN == INTP2 )                                                       // if int2 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP2
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP2
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP2(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP2

  #elif ( MDM_SYNC_CLK_PIN == INTP3 )                                                       // if int3 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP3
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP3
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP3(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP3

  #elif ( MDM_SYNC_CLK_PIN == INTP4 )                                                       // if int4 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP4
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP4
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP4(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP4

  #elif ( MDM_SYNC_CLK_PIN == INTP5 )                                                       // if int5 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP5
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP5
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP5(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP5

  #elif ( MDM_SYNC_CLK_PIN == INTP6 )                                                       // if int6 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_CLK_IRQ_ENABLE   ENABLE_IDLE_IRQ_INTP6
    #define GENI_MDM_CLK_IRQ_DISABLE  DISABLE_IDLE_IRQ_INTP6
    #define SET_MDM_CLK_EDGE_TRIG     SETUP_IDLE_DET_INTP6(OFF, ON, INT_PRIO_LEV0)          // flag for rising edge
    #define GENI_MDM_PRIO_LOW                                                               // priority flag
    #define GENI_Sync_IRQ_Vector      IDLE_VECTOR_INTP6

  #else
    #error 'Invalid value for MDM_SYNC_CLK_PIN'
  #endif

  #define FIRST_BIT_NO              23
  #define ENABLE_REG_ACCESS        {SET_PIN(MDM_REG_DATA_PIN);}

  #define DISABLE_REG_ACCESS       {CLEAR_PIN(MDM_REG_DATA_PIN);}

  #define REG_DATA_PIN_SETUP       {PIN_SETUP(MDM_REG_DATA_PIN, OUTPUT_MODE, PULL_UP_NOT_USED, PIN_LOW);}

  #define SETUP_MDM_SYNC_PINS      {GENI_UART_STOP;                         /* Disable uart    */  \
                                    GENI_RXD_MODE = AS_BIT_INPUT;           /* set rx as input */  \
                                    GENI_TXD_MODE = AS_BIT_OUTPUT;           /* set tx as output*/ \
                                    GENI_SETUP_RXD_AS_IO;                                          \
                                    GENI_SETUP_TXD_AS_IO;                                          \
                                    SET_MDM_CLK_PIN_AS_INPUT;               /* set clk as input*/  \
                                    SET_MDM_CLK_EDGE_TRIG;                  /* Rising edge trig*/  \
                                    GENI_INT_PRIO_LOW;                      /*                 */  \
                                    GENI_DIR_SETUP;                         /* Setup dir pin   */  \
                                    PIN_SETUP(MDM_REG_DATA_PIN, OUTPUT_MODE, PULL_UP_NOT_USED, PIN_LOW);}

  #define ENABLE_RX_MODE            SET_PIN(DIR_PIN)
  #define ENABLE_TX_MODE            CLEAR_PIN(DIR_PIN)
  #define RX_LINE                   GENI_RX_LINE
  #define TX_LINE                   GENI_TX_LINE


#endif


#endif /*_DRV_L_H  */
