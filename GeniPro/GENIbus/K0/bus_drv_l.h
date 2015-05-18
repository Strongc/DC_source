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
/* MODULE NAME      :   bus_drv_l.h                                         */
/*                                                                          */
/* FILE NAME        :   bus_drv_l.h                                         */
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
#include "common.h"

/*****************************************************************************/
/* Specify the channel                                                       */
/*****************************************************************************/
#define CH_SPEC BUS                                       // PLM, BUS, RS232, COM

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

#define M_BAUDRATE(a)                 a##_BAUDRATE
#define S_BAUDRATE(a)                 M_BAUDRATE(a)
#define BAUDRATE                      S_BAUDRATE(CH_SPEC)

#define M_UART_NUM(a)                 a##_UART_NUM
#define S_UART_NUM(a)                 M_UART_NUM(a)
#define UART_NUM                      S_UART_NUM(CH_SPEC)

#define M_UART_PRIORITY(a)            a##_UART_PRIORITY
#define S_UART_PRIORITY(a)            M_UART_PRIORITY(a)
#define UART_PRIORITY                 S_UART_PRIORITY(CH_SPEC)

#define M_DIR_CTR(a)                  a##_DIR_CTR
#define S_DIR_CTR(a)                  M_DIR_CTR(a)
#define DIR_CTR                       S_DIR_CTR(CH_SPEC)

#define M_DIR_PIN(a)                  a##_DIR_PIN
#define S_DIR_PIN(a)                  M_DIR_PIN(a)
#define DIR_PIN                       S_DIR_PIN(CH_SPEC)

#define M_IDLE_TYPE(a)                a##_IDLE_TYPE
#define S_IDLE_TYPE(a)                M_IDLE_TYPE(a)
#define IDLE_TYPE                     S_IDLE_TYPE(CH_SPEC)

#define M_IDLE_PIN(a)                 a##_IDLE_PIN
#define S_IDLE_PIN(a)                 M_IDLE_PIN(a)
#define IDLE_PIN                      S_IDLE_PIN(CH_SPEC)

#define M_IDLE_TIMER(a)               a##_IDLE_TIMER
#define S_IDLE_TIMER(a)               M_IDLE_TIMER(a)
#define IDLE_TIMER                    S_IDLE_TIMER(CH_SPEC)

#define M_IDLE_COUNT(a)               a##_IDLE_COUNT
#define S_IDLE_COUNT(a)               M_IDLE_COUNT(a)
#define IDLE_COUNT                    S_IDLE_COUNT(CH_SPEC)

#define M_soft_timer_started(a)       a##_soft_timer_started
#define S_soft_timer_started(a)       M_soft_timer_started(a)
#define soft_timer_started            S_soft_timer_started(CH_SPEC)

#define M_geni_irq_idle(a)            a##_geni_irq_idle
#define S_geni_irq_idle(a)            M_geni_irq_idle(a)
#define geni_irq_idle                 S_geni_irq_idle(CH_SPEC)

#define M_CONNECT_RXD6_TO_INTP0(a)    a##_CONNECT_RXD6_TO_INTP0
#define S_CONNECT_RXD6_TO_INTP0(a)    M_CONNECT_RXD6_TO_INTP0(a)
#define CONNECT_RXD6_TO_INTP0         S_CONNECT_RXD6_TO_INTP0(CH_SPEC)

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
/*  Genichannel hardware configuration:                                      */
/*****************************************************************************/
//  UART resources

#if ((defined D78098X))
  #define TX_LATCH_LEVEL 0
#else // D7801xx
  #define TX_LATCH_LEVEL 1
#endif

#if ( UART_NUM == 0 )                                                               // If UART0 is chosen for GENI
  #define GENI_OUT_IRQ              UART_OUT_IRQ_CH0
  #define GENI_IN_IRQ               UART_IN_IRQ_CH0
  #define GENI_ERR_IRQ              void
  #define GENI_RX_LINE              RX_LINE_CH0
  #define GENI_TX_LINE              TX_LINE_CH0
  #define GENI_TXD                  UART_TXD_CH0
  #define GENI_RXD                  UART_RXD_CH0

  #define GENI_ASYNC_MODE           ASYNC_MODE_CH0
  #define GENI_RXD_MODE             RXD_MODE_CH0                                    //  select channel 0 receive mode bit
  #define GENI_TXD_MODE             TXD_MODE_CH0                                    //  select channel 0 transmit mode bit

  #if(CHANNEL == PLM_CHANNEL)
  #define GENI_TXD_START            GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
  #define GENI_TXD_START            {INT_PEN_TX_CH0 = 1; INT_MSK_TX_CH0 = 0;}       // set pending flag, set mask flag
  #endif

  #define GENI_TXD_ENABLE           {RX_ENABLE_REG0 = 0; INT_PEN_TX_CH0 = 0; INT_MSK_TX_CH0 = 0;}       // clear pending flag, set mask flag
  #define GENI_TXD_DISABLE          {INT_PEN_TX_CH0 = 0; INT_MSK_TX_CH0 = 1;}       // clear pending flag, clear mask flag
  #define GENI_RXD_ENABLE           {RX_ENABLE_REG0 = 1; INT_PEN_RX_CH0 = 0; INT_MSK_RX_CH0 = 0;}       // clear pending flag, set mask flag
  #define GENI_RXD_DISABLE          {INT_PEN_RX_CH0 = 0; INT_MSK_RX_CH0 = 1;}       // clear pending flag, set mask flag
  #define GENI_CLEAR_RX_ERR         //{ASYNC_ERR_CH0 = 0;}
  #define GENI_RX_ERR_FLG           ASYNC_ERR_CH0

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    RXD_MODE_CH0 = AS_BIT_INPUT;                  /* setting the port mode      */              \
                                    TXD_MODE_CH0 = AS_BIT_OUTPUT;                 /* setting the port mode      */              \
                                    GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                    GENI_BAUD_INIT_0(baud);                                                                     \
                                    RX_INT_PRI_CH0 = UART_PRIORITY;                                                             \
                                    TX_INT_PRI_CH0 = UART_PRIORITY;                                                             \
                                    FRAME_SETUP_CH0(stop_bits, parity, GENI_DATA_8_BIT);                                               \
  }

  #define GENI_SETUP_UART_FOR_RX                                                                                                \
  {                                                                                                                             \
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


  #define GENI_BAUD_INIT_0(baud)                                      \
  {                                                                   \
    switch (baud)                                                     \
    {                                                                 \
      case GENI_BAUD_1200  : BAUD_CONT_CH0 = BAUD_SETUP_VAL0_1200;    \
      break;                                                          \
      case GENI_BAUD_2400  : BAUD_CONT_CH0 = BAUD_SETUP_VAL0_2400;    \
      break;                                                          \
      case GENI_BAUD_4800  : BAUD_CONT_CH0 = BAUD_SETUP_VAL0_4800;    \
      break;                                                          \
      case GENI_BAUD_9600  : BAUD_CONT_CH0 = BAUD_SETUP_VAL0_9600;    \
      break;                                                          \
      case GENI_BAUD_19200 : BAUD_CONT_CH0 = BAUD_SETUP_VAL0_19200;   \
      break;                                                          \
      case GENI_BAUD_38400 : BAUD_CONT_CH0 = BAUD_SETUP_VAL0_38400;   \
      break;                                                          \
      default              :                                          \
      break;                                                          \
    }                                                                 \
  }

#elif ( UART_NUM == 1 )                                                             // If UART1 is chosen for GENIbus
  #define GENI_OUT_IRQ              UART_OUT_IRQ_CH1
  #define GENI_IN_IRQ               UART_IN_IRQ_CH1
  #define GENI_ERR_IRQ              void
  #define GENI_RX_LINE              RX_LINE_CH1
  #define GENI_TX_LINE              TX_LINE_CH1
  #define GENI_TXD                  UART_TXD_CH1
  #define GENI_RXD                  UART_RXD_CH1

  #define GENI_ASYNC_MODE           ASYNC_MODE_CH1
  #define GENI_RXD_MODE             RXD_MODE_CH1                                    //  select channel 1 receive mode bit
  #define GENI_TXD_MODE             TXD_MODE_CH1                                    //  select channel 1 transmit mode bit

  #if(CHANNEL == PLM_CHANNEL)
  #define GENI_TXD_START            GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
  #define GENI_TXD_START            {INT_PEN_TX_CH1 = 1; INT_MSK_TX_CH1 = 0;}       // set pending flag, set mask flag
  #endif

  #define GENI_TXD_ENABLE           {RX_ENABLE_REG1 = 0; INT_PEN_TX_CH1 = 0; INT_MSK_TX_CH1 = 0;}       // clear pending flag, set mask flag
  #define GENI_TXD_DISABLE          {INT_PEN_TX_CH1 = 0; INT_MSK_TX_CH1 = 1;}       // clear pending flag, clear mask flag
  #define GENI_RXD_ENABLE           {RX_ENABLE_REG1 = 1; INT_PEN_RX_CH1 = 0; INT_MSK_RX_CH1 = 0;}       // clear pending flag, set mask flag
  #define GENI_RXD_DISABLE          {INT_PEN_RX_CH1 = 0; INT_MSK_RX_CH1 = 1;}       // clear pending flag, set mask flag
  #define GENI_CLEAR_RX_ERR         //{ASYNC_ERR_CH1 = 0;}
  #define GENI_RX_ERR_FLG           ASYNC_ERR_CH1

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                \
  {                                                                                                                             \
                                    RXD_MODE_CH1 = AS_BIT_INPUT;                  /* setting the port mode      */              \
                                    TXD_MODE_CH1 = AS_BIT_OUTPUT;                 /* setting the port mode      */              \
                                    GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                    GENI_BAUD_INIT_1(baud);                                                                 \
                                    RX_INT_PRI_CH1 = UART_PRIORITY;                                                             \
                                    TX_INT_PRI_CH1 = UART_PRIORITY;                                                             \
                                    FRAME_SETUP_CH1(stop_bits, parity, GENI_DATA_8_BIT);                                               \
  }

  #define GENI_SETUP_UART_FOR_RX                                                                                                       \
  {                                                                                                                             \
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


  #define GENI_BAUD_INIT_1(baud)                                      \
  {                                                                   \
    switch (baud)                                                     \
    {                                                                 \
      case GENI_BAUD_1200  : BAUD_CONT_CH1 = BAUD_SETUP_VAL1_1200;    \
      break;                                                          \
      case GENI_BAUD_2400  : BAUD_CONT_CH1 = BAUD_SETUP_VAL1_2400;    \
      break;                                                          \
      case GENI_BAUD_4800  : BAUD_CONT_CH1 = BAUD_SETUP_VAL1_4800;    \
      break;                                                          \
      case GENI_BAUD_9600  : BAUD_CONT_CH1 = BAUD_SETUP_VAL1_9600;    \
      break;                                                          \
      case GENI_BAUD_19200 : BAUD_CONT_CH1 = BAUD_SETUP_VAL1_19200;   \
      break;                                                          \
      case GENI_BAUD_38400 : BAUD_CONT_CH1 = BAUD_SETUP_VAL1_38400;   \
      break;                                                          \
      default              :                                          \
      break;                                                          \
    }                                                                 \
  }

#elif ( UART_NUM == 6 )
  #define GENI_OUT_IRQ              UART_OUT_IRQ_CH6
  #define GENI_IN_IRQ               UART_IN_IRQ_CH6
  #define GENI_ERR_IRQ              void
  #define GENI_RX_LINE              RX_LINE_CH6
  #define GENI_TX_LINE              TX_LINE_CH6
  #define GENI_TXD                  UART_TXD_CH6
  #define GENI_RXD                  UART_RXD_CH6

  #define GENI_ASYNC_MODE           ASYNC_MODE_CH6
  #define GENI_RXD_MODE             RXD_MODE_CH6                                    //  select channel 0 receive mode bit
  #define GENI_TXD_MODE             TXD_MODE_CH6                                    //  select channel 0 transmit mode bit

  #if(CHANNEL == PLM_CHANNEL)
  #define GENI_TXD_START            GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
  #define GENI_TXD_START            {INT_PEN_TX_CH6 = 1; INT_MSK_TX_CH6 = 0;}       // set pending flag, set mask flag
  #endif

  #define GENI_TXD_ENABLE           {RX_ENABLE_REG6 = 0; INT_PEN_TX_CH6 = 0; INT_MSK_TX_CH6 = 0;}       // clear pending flag, set mask flag
  #define GENI_TXD_DISABLE          {INT_PEN_TX_CH6 = 0; INT_MSK_TX_CH6 = 1;}       // clear pending flag, clear mask flag
  #define GENI_RXD_ENABLE           {RX_ENABLE_REG6 = 1; INT_PEN_RX_CH6 = 0; INT_MSK_RX_CH6 = 0;}       // clear pending flag, set mask flag
  #define GENI_RXD_DISABLE          {INT_PEN_RX_CH6 = 0; INT_MSK_RX_CH6 = 1;}       // clear pending flag, set mask flag
  #define GENI_CLEAR_RX_ERR         //{ASYNC_ERR_CH6 = 0;}
  #define GENI_RX_ERR_FLG           ASYNC_ERR_CH6

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                \
  {                                                                                                                             \
                                    RXD_MODE_CH6 = AS_BIT_INPUT;                  /* setting the port mode      */              \
                                    TXD_MODE_CH6 = AS_BIT_OUTPUT;                 /* setting the port mode      */              \
                                    GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                    GENI_BAUD_INIT_6(baud);                                                                 \
                                    RX_INT_PRI_CH6 = UART_PRIORITY;                                                             \
                                    TX_INT_PRI_CH6 = UART_PRIORITY;                                                             \
                                    FRAME_SETUP_CH6(stop_bits, parity, GENI_DATA_8_BIT);                                               \
  }

  #define GENI_SETUP_UART_FOR_RX                                                                                                       \
  {                                                                                                                             \
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


  #define GENI_BAUD_INIT_6(baud)                                      \
  {                                                                   \
    switch (baud)                                                     \
    {                                                                 \
      case GENI_BAUD_1200  : BAUD_CONT_CH6 = BAUD_SETUP_VAL6_1200;    \
                             BAUD_CLK_SEL_CH6 = CLK_SELECT_SETUP6_1200;\
      break;                                                          \
      case GENI_BAUD_2400  : BAUD_CONT_CH6 = BAUD_SETUP_VAL6_2400;    \
                             BAUD_CLK_SEL_CH6 = CLK_SELECT_SETUP6_2400;\
      break;                                                          \
      case GENI_BAUD_4800  : BAUD_CONT_CH6 = BAUD_SETUP_VAL6_4800;    \
                             BAUD_CLK_SEL_CH6 = CLK_SELECT_SETUP6_4800;\
      break;                                                          \
      case GENI_BAUD_9600  : BAUD_CONT_CH6 = BAUD_SETUP_VAL6_9600;    \
                             BAUD_CLK_SEL_CH6 = CLK_SELECT_SETUP6_9600;\
      break;                                                          \
      case GENI_BAUD_19200 : BAUD_CONT_CH6 = BAUD_SETUP_VAL6_19200;   \
                             BAUD_CLK_SEL_CH6 = CLK_SELECT_SETUP6_19200;\
      break;                                                          \
      case GENI_BAUD_38400 : BAUD_CONT_CH6 = BAUD_SETUP_VAL6_38400;   \
                             BAUD_CLK_SEL_CH6 = CLK_SELECT_SETUP6_38400;\
      break;                                                          \
      default              :                                          \
      break;                                                          \
    }                                                                 \
  }
#endif

/*****************************************************************************/
/* RS485 direction pin                                                       */
/*****************************************************************************/
#if(CHANNEL == PLM_CHANNEL)                                                           // invert direction pin for PLM
  #define AS_DIR_INPUT           1                                                    // disable the output driver
  #define AS_DIR_OUTPUT          0                                                    // enable the output driver
#else
  #define AS_DIR_INPUT           0                                                    // disable the output driver
  #define AS_DIR_OUTPUT          1                                                    // enable the output driver
#endif

#if (DIR_CTR == Enable)                                                             // Direction control enabled?
  #define GENI_DATA_DIR              GENI_PIN(DIR_PIN)
  #define GENI_DATA_DIR_MODE         GENI_PIN_MODE(DIR_PIN)

  #define GENI_DIR_OUTPUT            {GENI_DATA_DIR = AS_DIR_OUTPUT; \
                                      GENI_TX_LINE = TX_LATCH_LEVEL;}               // set port latch for safety

  #define GENI_DIR_INPUT              GENI_DATA_DIR = AS_DIR_INPUT
  #define GENI_DIR_SETUP              GENI_DATA_DIR_MODE  = AS_BIT_OUTPUT

  #define GENI_DIR_INPUT_MODE        (GENI_DATA_DIR == (AS_DIR_INPUT))
  #define GENI_DIR_OUTPUT_MODE       (GENI_DATA_DIR == (AS_DIR_OUTPUT))

#else
  #define GENI_DIR_OUTPUT             GENI_TX_LINE = TX_LATCH_LEVEL                 // set port latch for safety
  #define GENI_DIR_INPUT
  #define GENI_DIR_SETUP
#endif
/*****************************************************************************/
/* HW_IDLE settings                                                          */
/*****************************************************************************/

#if( IDLE_TYPE == HW_IDLE )
  #if(CHANNEL == PLM_CHANNEL)
    #error "Not supported for powerline channel"
  #endif
  #define GENI_REFRESH_IDLE
  #define GENI_IDLE_TIMER_STOP

  #if ( IDLE_PIN == INTP0 )                                                         // if port_120 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P0
    #define GENI_IDLE_IRQ          PORT0_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P0_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P0 = 0; INT_MSK_P0 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P0 = 0; INT_MSK_P0 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P0_EDGE_LOW_HIGH = 1; P0_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP1 )                                                       // if port_30 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P1
    #define GENI_IDLE_IRQ          PORT1_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P1_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P1 = 0; INT_MSK_P1 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P1 = 0; INT_MSK_P1 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P1_EDGE_LOW_HIGH = 1; P1_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP2 )                                                       // if port_31 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P2
    #define GENI_IDLE_IRQ          PORT2_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P2_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P2 = 0; INT_MSK_P2 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P2 = 0; INT_MSK_P2 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P2_EDGE_LOW_HIGH = 1; P2_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP3 )                                                       // if port_32 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P3
    #define GENI_IDLE_IRQ          PORT3_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P3_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P3 = 0; INT_MSK_P3 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P3 = 0; INT_MSK_P3 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P3_EDGE_LOW_HIGH = 1; P3_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP4 )                                                       // if port_33 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P4
    #define GENI_IDLE_IRQ          PORT4_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P4_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P4 = 0; INT_MSK_P4 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P4 = 0; INT_MSK_P4 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P4_EDGE_LOW_HIGH = 1; P4_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP5 )                                                       // if port_16 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P5
    #define GENI_IDLE_IRQ          PORT5_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P5_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P5 = 0; INT_MSK_P5 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P5 = 0; INT_MSK_P5 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P5_EDGE_LOW_HIGH = 1; P5_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP6 )                                                       // if port_140 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P6
    #define GENI_IDLE_IRQ          PORT6_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P6_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P6 = 0; INT_MSK_P6 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P6 = 0; INT_MSK_P6 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P6_EDGE_LOW_HIGH = 1; P6_EDGE_HIGH_LOW = 0; }

  #elif ( IDLE_PIN == INTP7 )                                                       // if port_140 is selected as GENI_IDLE_PIN
    #define GENI_IDLE_PIN          INT_P7
    #define GENI_IDLE_IRQ          PORT7_IRQ
    #define SET_IDLE_PIN_AS_INPUT  INT_P7_MODE = AS_BIT_INPUT;
    #define GENI_IDLE_IRQ_ENABLE   { INT_PEN_P7 = 0; INT_MSK_P7 = 0; }
    #define GENI_IDLE_IRQ_DISABLE  { INT_PEN_P7 = 0; INT_MSK_P7 = 1; }
    #define SET_IDLE_EDGE_TRIG     { P7_EDGE_LOW_HIGH = 1; P7_EDGE_HIGH_LOW = 0; }
  #else
    #error 'Invalid value for GENI_idle_pin'
  #endif
#endif

/*****************************************************************************/
/* TIMER_IDLE settings                                                       */
/*****************************************************************************/
#if(IDLE_TYPE == TIMER_IDLE)
  #if (( BAUDRATE == 19200) || ( BAUDRATE == 38400))
    #error 'Timer idle not supported at this baudrate'
  #endif

  //#define GENI_IDLE_IRQ                void                                         // GENI_IDLE_IRQ    : PORT0-2_IRQ, PORT4-7_IRQ

  #if ( IDLE_TIMER == 50)
    #define GENI_IDLE_TIMER_SETUP        {TIMER50_COMP_REG = IDLE_TIMER_COMP_VAL_50;\
                                          TIMER50_CLK_REG = IDLE_TIMER_SELECT_50;\
                                          TIMER50_MODE_REG = TIMER50_MODE_VAL;}     // setup of idle timer
    #define GENI_IDLE_TIMER_STOP         {INT_MSK_TIMER50 = 1;                      /* set irq mask */     \
                                          TIMER50_MODE_REG.7 = 0;}                  // stop timer
    #define GENI_IDLE_TIMER_START        {INT_FLG_TIMER50 = 0;                      /* clear irq flg */   \
                                          INT_MSK_TIMER50 = 0;                      /* set irq mask */     \
                                          TIMER50_MODE_REG.7 = 1;}                  // start timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW INT_PRIO_TIMER50 = 1                       // set low priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE   {INT_FLG_TIMER50 = 0;                       /* clear irq flg */   \
                                          INT_MSK_TIMER50 = 0;}                      /* set irq mask */
    #define GENI_IDLE_TIMER_IRQ          TIMER50_IRQ                                // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL(50, BAUDRATE)               // idle count value
    #define GENI_IDLE_TIMER_MODE         TIMER50_MODE_REG                           // timer mode register

  #elif ( IDLE_TIMER == 51)
    #define GENI_IDLE_TIMER_SETUP        {TIMER51_COMP_REG = IDLE_TIMER_COMP_VAL_51;\
                                          TIMER51_CLK_REG = IDLE_TIMER_SELECT_51;\
                                          TIMER51_MODE_REG = TIMER51_MODE_VAL;}     // setup of idle timer
    #define GENI_IDLE_TIMER_STOP         {INT_MSK_TIMER51 = 1;                      /* set irq mask */     \
                                          TIMER51_MODE_REG.7 = 0;}                  // stop timer
    #define GENI_IDLE_TIMER_START        {INT_FLG_TIMER51 = 0;                      /* clear irq flg */   \
                                          INT_MSK_TIMER51 = 0;                      /* set irq mask */     \
                                          TIMER51_MODE_REG.7 = 1;}                  // start timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW INT_PRIO_TIMER51 = 1                       // set low priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE   {INT_FLG_TIMER51 = 0;                      /* clear irq flg */   \
                                          INT_MSK_TIMER51 = 0;}                      /* set irq mask */
    #define GENI_IDLE_TIMER_IRQ          TIMER51_IRQ                                // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL(51, BAUDRATE)               // idle count value
    #define GENI_IDLE_TIMER_MODE         TIMER51_MODE_REG                           // timer mode register

  #elif ( IDLE_TIMER == 52)
    #define GENI_IDLE_TIMER_SETUP        {TIMER52_COMP_REG = IDLE_TIMER_COMP_VAL_52;\
                                          TIMER52_CLK_REG = IDLE_TIMER_SELECT_52;\
                                          TIMER52_MODE_REG = TIMER52_MODE_VAL;}     // setup of idle timer
    #define GENI_IDLE_TIMER_STOP         {INT_MSK_TIMER52 = 1;                      /* set irq mask */     \
                                          TIMER52_MODE_REG.7 = 0;}                  // stop timer
    #define GENI_IDLE_TIMER_START        {INT_FLG_TIMER52 = 0;                      /* clear irq flg */   \
                                          INT_MSK_TIMER52 = 0;                      /* set irq mask */     \
                                          TIMER52_MODE_REG.7 = 1;}                  // start timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW INT_PRIO_TIMER52 = 1                       // set low priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE   {INT_FLG_TIMER52 = 0;                      /* clear irq flg */   \
                                          INT_MSK_TIMER52 = 0;}                     /* set irq mask */
    #define GENI_IDLE_TIMER_IRQ          TIMER52_IRQ                                // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL(52, BAUDRATE)               // idle count value
    #define GENI_IDLE_TIMER_MODE         TIMER52_MODE_REG                           // timer mode register

   #elif ( IDLE_TIMER == SOFT)
    //extern BIT soft_idle_timer_started;
    #define GENI_IDLE_TIMER_SETUP        soft_timer_started = FALSE                 // setup of idle timer
    #define GENI_IDLE_TIMER_STOP         soft_timer_started = FALSE                 // stop timer
    #define GENI_IDLE_TIMER_INT_PRIO_LOW                                            // set low priority
    #define GENI_IDLE_TIMER_IRQ_ENABLE                                              // enable interrupt for timer
    #define GENI_IDLE_TIMER_IRQ          void                                       // interrupt vector for selected timer
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT                                 // idlle count value
                                                                                    // idlle time = 2,7 ms = (1/(fx/2))*256*48
  #else
      #error 'Invalid value for GENI_IDLE_TIMER'
  #endif

  #if ( IDLE_TIMER == SOFT)
    #define GENI_REFRESH_IDLE            {geni_idle_counter=GENI_IDLE_COUNT_VAL; \
                                          soft_timer_started = TRUE; \
                                          }                                         // set counter and bit to let timer count
  #else
    #define GENI_REFRESH_IDLE            {geni_idle_counter=GENI_IDLE_COUNT_VAL; \
                                         GENI_IDLE_TIMER_STOP; \
                                         GENI_IDLE_TIMER_START;}                    // set counter and bit to let timer count
  #endif

  #if ( IDLE_PIN == INTP0 )                                                         // if INTP0 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P0_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P0_EDGE_LOW_HIGH=0; P0_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P0 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0 = 0; INT_MSK_P0 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0 = 0; INT_MSK_P0 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                PORT0_IRQ                                  // timer interrupt vector

  #elif ( IDLE_PIN == INTP1 )                                                       // if INTP1 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P1_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P1_EDGE_LOW_HIGH=0; P1_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P1 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P1 = 0; INT_MSK_P1 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P1 = 0; INT_MSK_P1 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT1_IRQ                                 // timer interrupt vector

  #elif ( IDLE_PIN == INTP2 )                                                       // if INTP2 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P2_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P2_EDGE_LOW_HIGH=0; P2_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P2 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P2 = 0; INT_MSK_P2 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P2 = 0; INT_MSK_P2 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT2_IRQ                                 // timer interrupt vector

  #elif ( IDLE_PIN == INTP3 )                                                       // if INTP3 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P3_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P3_EDGE_LOW_HIGH=0; P3_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P3 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P3 = 0; INT_MSK_P3 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P3 = 0; INT_MSK_P3 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT3_IRQ                                 // timer interrupt vector

  #elif ( IDLE_PIN == INTP4 )                                                       // if INTP4 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P4_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P4_EDGE_LOW_HIGH=0; P4_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P4 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P4 = 0; INT_MSK_P4 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P4 = 0; INT_MSK_P4 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT4_IRQ                                 // timer interrupt vector

  #elif ( IDLE_PIN == INTP5 )                                                       // if INTP5 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P5_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P5_EDGE_LOW_HIGH=0; P5_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P5 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P5 = 0; INT_MSK_P5 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P5 = 0; INT_MSK_P5 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT5_IRQ                                 // timer interrupt vector

  #elif ( IDLE_PIN == INTP6 )                                                       // if INTP6 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P6_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P6_EDGE_LOW_HIGH=0; P6_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P6 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P6 = 0; INT_MSK_P6 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P6 = 0; INT_MSK_P6 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT6_IRQ                                 // timer interrupt vector

  #elif ( IDLE_PIN == INTP7 )                                                       // if INTP7 is selected as GENI_IDLE_PIN
    #define SET_IDLE_PIN_AS_INPUT        INT_P7_MODE = AS_BIT_INPUT;
    #define SET_PORT_EDGE_TRIG           P7_EDGE_LOW_HIGH=0; P7_EDGE_HIGH_LOW=1;    // flag for falling and rising edge
    #define GENI_INT_PRIO_LOW            INT_PRIO_P7 = 1;                           // priority flag
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P7 = 0; INT_MSK_P7 = 0; }        // enable interrupt clear pending
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P7 = 0; INT_MSK_P7 = 1; }        // disable interrupt clear pending
    #define SOFT_IDLE_IRQ                 PORT7_IRQ                                 // timer interrupt vector

  #elif (IDLE_PIN == GENI_NOT_USED)
    #define SET_IDLE_PIN_AS_INPUT
    #define SET_PORT_EDGE_TRIG
    #define GENI_INT_PRIO_LOW
    #define GENI_IDLE_IRQ_ENABLE
    #define GENI_IDLE_IRQ_DISABLE
    #define SOFT_IDLE_IRQ                void

  #else
    #error 'Invalid value for GENI_idle_pin'

  #endif

  #if ((defined D78010X) || (defined D78012X) || (defined D78013X))
    #if(( IDLE_PIN == INTP0 ) && (CONNECT_RXD6_TO_INTP0 == TRUE))
      // If internal connection between RXD6 and
      #define SET_ISC0_REG              ISC0_REG_ON;                             // the INTP0 should be made set ISC0_REG = ON
    #else
      #define SET_ISC0_REG              {SET_IDLE_PIN_AS_INPUT;                    /* Set Idle pin to input mode*/ \
                                         ISC0_REG_OFF;}
    #endif
  #else
    #define SET_ISC0_REG                SET_IDLE_PIN_AS_INPUT;                      /* Set Idle pin to input mode*/
  #endif
#endif



/*****************************************************************************/
/* NO_IDLE settings                                                          */
/*****************************************************************************/

#if(IDLE_TYPE == NO_IDLE)
  #if(CHANNEL == PLM_CHANNEL)
    #error "Not supported for powerline channel"
  #endif
  #define GENI_IDLE_IRQ                 void
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

#define GENI_REFRESH_TX_IDLE


#if(CHANNEL == PLM_CHANNEL)
/*****************************************************************************/
/* Modem access settings                                                     */
/*****************************************************************************/
/*****************************************************************************/
/* Modem access settings - sync clock interrupt setup                        */
/*****************************************************************************/
  #if ( MDM_SYNC_CLK_PIN == INTP0 )                                                         // if int0 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P0_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P0 = 0; INT_MSK_P0 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P0 = 0; INT_MSK_P0 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P0_EDGE_LOW_HIGH = 1; P0_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P0 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT0_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP1 )
                                                                                            // if int1 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P1_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P1 = 0; INT_MSK_P1 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P1 = 0; INT_MSK_P1 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P1_EDGE_LOW_HIGH = 1; P1_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRI0_P1 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT1_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP2 )                                                       // if int2 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P2_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P2 = 0; INT_MSK_P2 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P2 = 0; INT_MSK_P2 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P2_EDGE_LOW_HIGH = 1; P2_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P2 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT2_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP3 )                                                       // if int3 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P3_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P3 = 0; INT_MSK_P3 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P3 = 0; INT_MSK_P3 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P3_EDGE_LOW_HIGH = 1; P3_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P3 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT3_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP4 )                                                       // if int4 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P4_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P4 = 0; INT_MSK_P4 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P4 = 0; INT_MSK_P4 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P4_EDGE_LOW_HIGH = 1; P4_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P4 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT4_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP5 )                                                       // if int5 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P5_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P5 = 0; INT_MSK_P5 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P5 = 0; INT_MSK_P5 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P5_EDGE_LOW_HIGH = 1; P5_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P5 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT5_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP6 )                                                       // if int6 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P6_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P6 = 0; INT_MSK_P6 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P6 = 0; INT_MSK_P6 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P6_EDGE_LOW_HIGH = 1; P6_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P6 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT6_IRQ

  #elif ( MDM_SYNC_CLK_PIN == INTP7 )                                                       // if int7 is selected as MDM_SYNC_CLK_PIN
    #define SET_MDM_CLK_PIN_AS_INPUT  INT_P7_MODE = AS_BIT_INPUT;
    #define GENI_MDM_CLK_IRQ_ENABLE   { INT_PEN_P7 = 0; INT_MSK_P7 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE  { INT_PEN_P7 = 0; INT_MSK_P7 = 1; }
    #define SET_MDM_CLK_EDGE_TRIG     { P7_EDGE_LOW_HIGH = 1; P7_EDGE_HIGH_LOW = 0; }
    #define GENI_MDM_PRIO_LOW         INT_PRIO_P7 = 1;                                     // priority flag
    #define GENI_SYNC_IRQ             PORT7_IRQ

  #else
    #error 'Invalid value for MDM_SYNC_CLK_PIN'
  #endif

  #define FIRST_BIT_NO              23
  #define ENABLE_REG_ACCESS        {GENI_PIN(MDM_REG_DATA_PIN) = 1;}

  #define DISABLE_REG_ACCESS       {GENI_PIN(MDM_REG_DATA_PIN) = 0;}

  #define REG_DATA_PIN_SETUP       {GENI_PIN(MDM_REG_DATA_PIN) = 0;         /* Set pin to low  */  \
                                    GENI_PIN_MODE(MDM_REG_DATA_PIN) = 0; }  /* set Register acces pin as output */

  #define SETUP_MDM_SYNC_PINS      {GENI_ASYNC_MODE = 0x00;                 /* Disable uart    */  \
                                    GENI_RXD_MODE = AS_BIT_INPUT;           /* set rx as input */  \
                                    GENI_TXD_MODE = AS_BIT_OUTPUT;           /* set tx as output*/  \
                                    SET_MDM_CLK_PIN_AS_INPUT;               /* set clk as input*/  \
                                    SET_MDM_CLK_EDGE_TRIG;                  /* Rising edge trig*/  \
                                    GENI_INT_PRIO_LOW;                      /*                 */  \
                                    GENI_DIR_SETUP;                         /* Setup dir pin   */  \
                                    GENI_PIN(MDM_REG_DATA_PIN) = 0;         /* Set pin to low  */  \
                                    GENI_PIN_MODE(MDM_REG_DATA_PIN) = 0; }  /* set Register acces pin as output */

  #define ENABLE_RX_MODE            GENI_DIR_INPUT
  #define ENABLE_TX_MODE            {GENI_DATA_DIR = AS_DIR_OUTPUT;}
  #define RX_LINE                   GENI_RX_LINE
  #define TX_LINE                   GENI_TX_LINE


#endif


#endif /*_DRV_L_H  */
