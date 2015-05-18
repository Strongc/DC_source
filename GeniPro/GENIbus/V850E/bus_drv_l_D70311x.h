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
/* MODULE NAME      :   bus_drv_l_D70311.h                                  */
/*                                                                          */
/* FILE NAME        :   bus_drv_l_D70311.h                                  */
/*                                                                          */
/* FILE DESCRIPTION :   Defines for the c file                              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _BUS_DRV_L_D70311_H
#define _BUS_DRV_L_D70311_H

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
  #define CKSR0_VAL     CKSR0_VAL_1200
  #define BRGC0_VAL     BRGC0_VAL_1200
  #define PRSCM12_VAL   PRSCM12_VAL_1200
  #define PRSM12_VAL    PRSM12_VAL_1200
#elif ( BAUDRATE == 2400)
  #define CKSR0_VAL     CKSR0_VAL_2400
  #define BRGC0_VAL     BRGC0_VAL_2400
  #define PRSCM12_VAL   PRSCM12_VAL_2400
  #define PRSM12_VAL    PRSM12_VAL_2400
#elif ( BAUDRATE == 9600)
  #define CKSR0_VAL     CKSR0_VAL_9600
  #define BRGC0_VAL     BRGC0_VAL_9600
  #define PRSCM12_VAL   PRSCM12_VAL_9600
  #define PRSM12_VAL    PRSM12_VAL_9600
#elif ( BAUDRATE == 19200)
  #define CKSR0_VAL     CKSR0_VAL_19200
  #define BRGC0_VAL     BRGC0_VAL_19200
  #define PRSCM12_VAL   PRSCM12_VAL_19200
  #define PRSM12_VAL    PRSM12_VAL_19200
#elif ( BAUDRATE == 38400)
  #define CKSR0_VAL     CKSR0_VAL_38400
  #define BRGC0_VAL     BRGC0_VAL_38400
  #define PRSCM12_VAL   PRSCM12_VAL_38400
  #define PRSM12_VAL    PRSM12_VAL_38400
#else
  #error Unsupported baudrate value
#endif

/*****************************************************************************/
/*  Genichannel hardware configuration:                                      */
/*****************************************************************************/
#if ( UART_NUM == 0 )                                                                 // If UART0 is chosen for GENI
  #define Tx_Vector                 INTST0_vector
  #define Rx_Vector                 INTSR0_vector
  #define Rx_Err_Vector             INTSER0_vector
  #define GENI_ASYNC_MODE           ASYNC_MODE_CH0                                    //  GENI_ASYNC_MODE :
  #define GENI_TXD_MODE             TXD_MODE_CH0                                      //  select channel 0 transmit mode bit
  #define GENI_RXD_MODE             RXD_MODE_CH0                                      //  select channel 0 receive mode bit
  #define GENI_TX_LINE              TX_LINE_CH0
  #define GENI_RX_LINE              RX_LINE_CH0
  #define GENI_SETUP_RXD_AS_IO      SETUP_RXD_AS_IO_CH0
  #define GENI_SETUP_TXD_AS_IO      SETUP_TXD_AS_IO_CH0
  #define GENI_TXD                  UART_TXD_CH0
  #define GENI_RXD                  UART_RXD_CH0
  #if(CHANNEL == PLM_CHANNEL)
    #define GENI_TXD_START          GENI_TXD = PLM_1ST_PREAMBLE                       // send preamble
  #else
    #define GENI_TXD_START          {INT_PEN_TX_CH0 = 1; INT_MSK_TX_CH0 = 0;}         // set pending flag, set mask flag
  #endif

  #define GENI_TXD_ENABLE           {INT_PEN_TX_CH0 = 0; INT_MSK_TX_CH0 = 0;}         // clear pending flag, set mask flag
  #define GENI_TXD_DISABLE          {INT_PEN_TX_CH0 = 0; INT_MSK_TX_CH0 = 1;}         // clear pending flag, clear mask flag
  #define GENI_RXD_ENABLE           {INT_PEN_RX_CH0 = 0; INT_MSK_RX_CH0 = 0;}         // clear pending flag, set mask flag
  #define GENI_RXD_DISABLE          {INT_PEN_RX_CH0 = 0; INT_MSK_RX_CH0 = 1;}         // clear pending flag, set mask flag

  #define GENI_CLEAR_RX_ERR         {if (GENI_RXD);}

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    RXD_MODE_CH0 = AS_BIT_INPUT;                  /* setting the port mode      */              \
                                    TXD_MODE_CH0 = AS_BIT_OUTPUT;                 /* setting the port mode      */              \
                                    GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                    PMC3 |= UART0_MODE;                                                                         \
                                    GENI_BAUD_INIT_0(baud);                                                                     \
                                    INT_CTR_RX_CH0 = (INT_CTR_RX_CH0 & 0xF8) | UART_PRIORITY;  /* mask the 3 LSB */             \
                                    INT_CTR_TX_CH0 = (INT_CTR_TX_CH0 & 0xF8) | UART_PRIORITY;  /* mask the 3 LSB */             \
                                    FRAME_SETUP_CH0(stop_bits, parity, GENI_DATA_8_BIT);                                        \
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


  #define GENI_BAUD_INIT_0(baud)                                                \
  {                                                                             \
    switch (baud)                                                               \
    {                                                                           \
      case GENI_BAUD_1200  : CKSR0 = CKSR0_VAL_1200; BRGC0 = BRGC0_VAL_1200;    \
      break;                                                                    \
      case GENI_BAUD_2400  : CKSR0 = CKSR0_VAL_2400; BRGC0 = BRGC0_VAL_2400;    \
      break;                                                                    \
      case GENI_BAUD_4800  : CKSR0 = CKSR0_VAL_4800; BRGC0 = BRGC0_VAL_4800;    \
      break;                                                                    \
      case GENI_BAUD_9600  : CKSR0 = CKSR0_VAL_9600; BRGC0 = BRGC0_VAL_9600;    \
      break;                                                                    \
      case GENI_BAUD_19200  : CKSR0 = CKSR0_VAL_19200; BRGC0 = BRGC0_VAL_19200; \
      break;                                                                    \
      case GENI_BAUD_38400  : CKSR0 = CKSR0_VAL_38400; BRGC0 = BRGC0_VAL_38400; \
      break;                                                                    \
      default              :                                                    \
      break;                                                                    \
    }                                                                           \
  }

#endif

#if ( UART_NUM == 1 )                                                                 // If UART1 is chosen for GENI
  #define Tx_Vector                 INTST1_vector
  #define Rx_Vector                 INTSR1_vector
  #define GENI_ASYNC_MODE           ASYNC_MODE_CH1                                    //  GENI_ASYNC_MODE :
  #define GENI_RXD_MODE             RXD_MODE_CH1                                      //  select channel 1 receive mode bit
  #define GENI_TXD_MODE             TXD_MODE_CH1                                      //  select channel 1 transmit mode bit
  #define GENI_TX_LINE              TX_LINE_CH1
  #define GENI_RX_LINE              RX_LINE_CH1
  #define GENI_SETUP_RXD_AS_IO      SETUP_RXD_AS_IO_CH1
  #define GENI_SETUP_TXD_AS_IO      SETUP_TXD_AS_IO_CH1
  #define GENI_TXD                  UART_TXD_CH1
  #define GENI_RXD                  UART_RXD_CH1
  #if(CHANNEL == PLM_CHANNEL)
    #define GENI_TXD_START          GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
    #define GENI_TXD_START          {INT_PEN_TX_CH1 = 1; INT_MSK_TX_CH1 = 0;}         // set pending flag, set mask flag
  #endif

  #define GENI_TXD_ENABLE           {INT_PEN_TX_CH1 = 0; INT_MSK_TX_CH1 = 0;}         // clear pending flag, set mask flag
  #define GENI_TXD_DISABLE          {INT_PEN_TX_CH1 = 0; INT_MSK_TX_CH1 = 1;}         // clear pending flag, clear mask flag
  #define GENI_RXD_ENABLE           {INT_PEN_RX_CH1 = 0; INT_MSK_RX_CH1 = 0;}         // clear pending flag, set mask flag
  #define GENI_RXD_DISABLE          {INT_PEN_RX_CH1 = 0; INT_MSK_RX_CH1 = 1;}         // clear pending flag, set mask flag

  #define GENI_CLEAR_RX_ERR         {if (GENI_RXD);}

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    RXD_MODE_CH1 = AS_BIT_INPUT;                  /* setting the port mode      */              \
                                    TXD_MODE_CH1 = AS_BIT_OUTPUT;                 /* setting the port mode      */              \
                                    GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                    PMC3 |= UART1_MODE;                                                                         \
                                    GENI_BAUD_INIT_1(baud);                                                                     \
                                    INT_CTR_RX_CH1 = (INT_CTR_RX_CH1 & 0xF8) | UART_PRIORITY;  /* mask the 3 LSB */             \
                                    INT_CTR_TX_CH1 = (INT_CTR_TX_CH1 & 0xF8) | UART_PRIORITY;  /* mask the 3 LSB */             \
                                    FRAME_SETUP_CH1(stop_bits, parity, GENI_DATA_8_BIT);                                        \
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


  #define GENI_BAUD_INIT_1(baud)                                                      \
  {                                                                                   \
    switch (baud)                                                                     \
    {                                                                                 \
      case GENI_BAUD_1200  : PRSCM1 = PRSCM12_VAL_1200; PRSM1 = PRSM12_VAL_1200;      \
      break;                                                                          \
      case GENI_BAUD_2400  : PRSCM1 = PRSCM12_VAL_2400; PRSM1 = PRSM12_VAL_2400;      \
      break;                                                                          \
      case GENI_BAUD_4800  : PRSCM1 = PRSCM12_VAL_4800; PRSM1 = PRSM12_VAL_4800;      \
      break;                                                                          \
      case GENI_BAUD_9600  : PRSCM1 = PRSCM12_VAL_9600; PRSM1 = PRSM12_VAL_9600;      \
      break;                                                                          \
      case GENI_BAUD_19200  : PRSCM1 = PRSCM12_VAL_19200; PRSM1 = PRSM12_VAL_19200;   \
      break;                                                                          \
      case GENI_BAUD_38400  : PRSCM1 = PRSCM12_VAL_38400; PRSM1 = PRSM12_VAL_38400;   \
      break;                                                                          \
      default              :                                                          \
      break;                                                                          \
    }                                                                                 \
  }

#endif

#if ( UART_NUM == 2 )                                                                 // If UART1 is chosen for GENI
  #define Tx_Vector                 INTST2_vector
  #define Rx_Vector                 INTSR2_vector
  #define GENI_ASYNC_MODE           ASYNC_MODE_CH2                                    //  GENI_ASYNC_MODE :
  #define GENI_RXD_MODE             RXD_MODE_CH2                                      // select channel 2 receive mode bit
  #define GENI_TXD_MODE             TXD_MODE_CH2                                      // select channel 2 transmit mode bit
  #define GENI_TX_LINE              TX_LINE_CH2
  #define GENI_RX_LINE              RX_LINE_CH2
  #define GENI_SETUP_RXD_AS_IO      SETUP_RXD_AS_IO_CH2
  #define GENI_SETUP_TXD_AS_IO      SETUP_TXD_AS_IO_CH2
  #define GENI_TXD                  UART_TXD_CH2
  #define GENI_RXD                  UART_RXD_CH2

  #if(CHANNEL == PLM_CHANNEL)
    #define GENI_TXD_START          GENI_TXD = PLM_1ST_PREAMBLE                     // send preamble
  #else
    #define GENI_TXD_START          {INT_PEN_TX_CH2 = 1; INT_MSK_TX_CH2 = 0;}         // set pending flag, set mask flag
  #endif

  #define GENI_TXD_ENABLE           {INT_PEN_TX_CH2 = 0; INT_MSK_TX_CH2 = 0;}         // clear pending flag, set mask flag
  #define GENI_TXD_DISABLE          {INT_PEN_TX_CH2 = 0; INT_MSK_TX_CH2 = 1;}         // clear pending flag, clear mask flag
  #define GENI_RXD_ENABLE           {INT_PEN_RX_CH2 = 0; INT_MSK_RX_CH2 = 0;}         // clear pending flag, set mask flag
  #define GENI_RXD_DISABLE          {INT_PEN_RX_CH2 = 0; INT_MSK_RX_CH2 = 1;}         // clear pending flag, set mask flag

  #define GENI_CLEAR_RX_ERR         {if (GENI_RXD);}

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    RXD_MODE_CH2 = AS_BIT_INPUT;                  /* setting the port mode      */              \
                                    TXD_MODE_CH2 = AS_BIT_OUTPUT;                 /* setting the port mode      */              \
                                    GENI_TX_LINE = 1;                             /* avoid spikes when shifting */              \
                                    PMC3 |= UART2_MODE;                                                                         \
                                    GENI_BAUD_INIT_2(baud);                                                                     \
                                    INT_CTR_RX_CH2 = (INT_CTR_RX_CH2 & 0xF8) | UART_PRIORITY;  /* mask the 3 LSB */             \
                                    INT_CTR_TX_CH2 = (INT_CTR_TX_CH2 & 0xF8) | UART_PRIORITY;  /* mask the 3 LSB */             \
                                    FRAME_SETUP_CH2(stop_bits, parity, GENI_DATA_8_BIT);                                        \
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


  #define GENI_BAUD_INIT_2(baud)                                                      \
  {                                                                                   \
    switch (baud)                                                                     \
    {                                                                                 \
      case GENI_BAUD_1200  : PRSCM2 = PRSCM12_VAL_1200; PRSM2 = PRSM12_VAL_1200;      \
      break;                                                                          \
      case GENI_BAUD_2400  : PRSCM2 = PRSCM12_VAL_2400; PRSM2 = PRSM12_VAL_2400;      \
      break;                                                                          \
      case GENI_BAUD_4800  : PRSCM2 = PRSCM12_VAL_4800; PRSM2 = PRSM12_VAL_4800;      \
      break;                                                                          \
      case GENI_BAUD_9600  : PRSCM2 = PRSCM12_VAL_9600; PRSM2 = PRSM12_VAL_9600;      \
      break;                                                                          \
      case GENI_BAUD_19200  : PRSCM2 = PRSCM12_VAL_19200; PRSM2 = PRSM12_VAL_19200;   \
      break;                                                                          \
      case GENI_BAUD_38400  : PRSCM2 = PRSCM12_VAL_38400; PRSM2 = PRSM12_VAL_38400;   \
      break;                                                                          \
      default              :                                                          \
      break;                                                                          \
    }                                                                                 \
  }


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

#if (DIR_CTR == Enable)                                                               // Direction control enabled?
  #define CLEAR_PIN_MODE1(pin)        SET_BIT_LOW(PIN_MODE1_DEF(pin),PIN_BIT_NUMBER(pin))
  #define CLEAR_PIN_MODE2(pin)        SET_BIT_LOW(PIN_MODE2_DEF(pin),PIN_BIT_NUMBER(pin))
  #define GENI_DIR_SETUP              {CLEAR_PIN_MODE2(DIR_PIN);CLEAR_PIN_MODE1(DIR_PIN);}
  #if(CHANNEL == PLM_CHANNEL)                                                           // invert direction pin for PLM
    #define GENI_DIR_OUTPUT           CLEAR_PIN(DIR_PIN)
    #define GENI_DIR_INPUT            SET_PIN(DIR_PIN)
  #else
    #define GENI_DIR_OUTPUT           SET_PIN(DIR_PIN)
    #define GENI_DIR_INPUT            CLEAR_PIN(DIR_PIN)
  #endif

#else
  #define GENI_DIR_OUTPUT
  #define GENI_DIR_INPUT
  #define GENI_DIR_SETUP
#endif

/*****************************************************************************/
/* HW_IDLE settings                                                          */
/*****************************************************************************/
#if( IDLE_TYPE == HW_IDLE )
  #if((CHANNEL == PLM_CHANNEL))
    #error "Not supported"
  #endif
  #define GENI_REFRESH_IDLE
  #define GENI_IDLE_TIMER_STOP

  #if ( IDLE_PIN == INTP0 )
    #define GENI_IDLE_PIN                P0_bit.no1                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP0_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC0 = 0; INT_MSK_P0IC0 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC0 = 0; INT_MSK_P0IC0 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM1 &= ~0x03; INTM1 |=0x01;}              // Rising edge

  #elif ( IDLE_PIN == INTP1 )
    #define GENI_IDLE_PIN                P0_bit.no2                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP1_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC1 = 0; INT_MSK_P0IC1 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC1 = 0; INT_MSK_P0IC1 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM1 &= ~0x0C; INTM1 |=0x04;}              // Rising edge

  #elif ( IDLE_PIN == INTP2 )
    #define GENI_IDLE_PIN                P0_bit.no3                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP2_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC2 = 0; INT_MSK_P0IC2 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC2 = 0; INT_MSK_P0IC2 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM1 &= ~0x30; INTM1 |=0x10;}              // Rising edge

  #elif ( IDLE_PIN == INTP3 )
    #define GENI_IDLE_PIN                P0_bit.no4                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP3_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC3 = 0; INT_MSK_P0IC3 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC3 = 0; INT_MSK_P0IC3 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM1 &= ~0xC0; INTM1 |=0x40;}              // Rising edge

  #elif ( IDLE_PIN == INTP4 )
    #define GENI_IDLE_PIN                P0_bit.no5                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP4_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC4 = 0; INT_MSK_P0IC4 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC4 = 0; INT_MSK_P0IC4 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM2 &= ~0x03; INTM2 |=0x01;}              // Rising edge

  #elif ( IDLE_PIN == INTP5 )
    #define GENI_IDLE_PIN                P0_bit.no6                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP5_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC5 = 0; INT_MSK_P0IC5 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC5 = 0; INT_MSK_P0IC5 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM2 &= ~0x0C; INTM2 |=0x04;}              // Rising edge

  #elif ( IDLE_PIN == INTP6 )
    #define GENI_IDLE_PIN                P0_bit.no7                                   //  GENI_IDLE_PIN
    #define GENI_IDLE_IRQ_Vector         INTP6_vector                                 //  GENI_IDLE_IRQ
    #define GENI_IDLE_IRQ_ENABLE         { INT_PEN_P0IC6 = 0; INT_MSK_P0IC6 = 0; }
    #define GENI_IDLE_IRQ_DISABLE        { INT_PEN_P0IC6 = 0; INT_MSK_P0IC6 = 1; }
    #define SET_IDLE_EDGE_TRIG           {INTM2 &= ~0x30; INTM2 |=0x10;}              // Rising edge
  #else
     #error 'Invalid value of Idle_pin'
  #endif

#endif

/*****************************************************************************/
/* TIMER_IDLE settings                                                       */
/*****************************************************************************/
#if(IDLE_TYPE == TIMER_IDLE)
  #if ( IDLE_TIMER == 3)
    #define GENI_IDLE_TIMER_INT_MSK      INT_MSK_TIMER50                              // interrupt mask for selected timer
    #define GENI_IDLE_TIMER_INT_PRIO     INT_PRIO_TM3                                 // interrupt priority flag for selected timer
    #define GENI_IDLE_TIMER_IRQ          INTCC30_vector

    #define GENI_IDLE_TIMER_CLK_REG      PRM03                                        // base clock for TM3
    #define GENI_IDLE_TIMER_CTR_REG0     TMC30                                        // control register 0
    #define GENI_IDLE_TIMER_CTR_REG1     TMC31                                        // control register 1
    #define GENI_IDLE_TIMER_COM_REG      CC30                                         // compare register
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT_VAL_TM3

    #define GENI_IDLE_TIMER_SETUP        { GENI_IDLE_TIMER_CLK_REG = GENI_IDLE_TIMER_CLK_TM3;\
                                           IDLE_TIMER_BASE_CLK_TM3 = ON;\
                                           GENI_IDLE_TIMER_CTR_REG0 |= GENI_IDLE_TIMER_CTR0_TM3;\
                                           GENI_IDLE_TIMER_CTR_REG1 = GENI_IDLE_TIMER_CTR1_TM3;\
                                           GENI_IDLE_TIMER_COM_REG  = GENI_IDLE_TIMER_COM_TM3;}

    #define GENI_IDLE_TIMER_INT_PRIO_LOW { INT_CTR_REG_INTCC30 &= 0xF8; \
                                           INT_CTR_REG_INTCC30 |= INT_PRIO_LEV7;}

    #define GENI_IDLE_TIMER_IRQ_ENABLE   { INT_PEN_CC3IC0 = 0; INT_MSK_CC3IC0 = 0; }
    #define GENI_IDLE_TIMER_START        TMC30_bit.no1 = 1
    #define GENI_IDLE_TIMER_STOP         TMC30_bit.no1 = 0

  #elif ( IDLE_TIMER == SOFT )
    #define GENI_IDLE_TIMER_INT_MSK                                                   // interrupt mask for selected timer
    #define GENI_IDLE_TIMER_INT_PRIO                                                  // interrupt priority flag for selected timer
    #define GENI_IDLE_TIMER_CLK_REG                                                   // base clock for TM3
    #define GENI_IDLE_TIMER_CTR_REG0                                                  // control register 0
    #define GENI_IDLE_TIMER_CTR_REG1                                                  // control register 1
    #define GENI_IDLE_TIMER_COM_REG                                                   // compare register
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT

    #define GENI_IDLE_TIMER_SETUP        soft_timer_started = FALSE

    #define GENI_IDLE_TIMER_INT_PRIO_LOW
    #define GENI_IDLE_TIMER_IRQ_ENABLE
    #define GENI_IDLE_TIMER_START        soft_timer_started = TRUE
    #define GENI_IDLE_TIMER_STOP         soft_timer_started = FALSE
  #else
    #error 'Illegal GENI_IDLE_TIMER value'
  #endif

   #define GENI_REFRESH_IDLE            { geni_idle_counter=GENI_IDLE_COUNT_VAL; \
                                        GENI_IDLE_TIMER_STOP;\
                                        GENI_IDLE_TIMER_START;}                       // set counter and bit to let timer count

  #if ( IDLE_PIN == INTP0 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP0 &= 0xF8; \
                                          INT_CTR_REG_INTP0 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM1 &= ~0x03;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC0 = 0; INT_MSK_P0IC0 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC0 = 0; INT_MSK_P0IC0 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP0_vector

  #elif ( IDLE_PIN == INTP1 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP1 &= 0xF8; \
                                          INT_CTR_REG_INTP1 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM1 &= ~0x0C;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC1 = 0; INT_MSK_P0IC1 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC1 = 0; INT_MSK_P0IC1 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP1_vector

  #elif ( IDLE_PIN == INTP2 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP2 &= 0xF8; \
                                          INT_CTR_REG_INTP2 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM1 &= ~0x30;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC2 = 0; INT_MSK_P0IC2 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC2 = 0; INT_MSK_P0IC2 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP2_vector

  #elif ( IDLE_PIN == INTP3 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP3 &= 0xF8; \
                                          INT_CTR_REG_INTP3 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM1 &= ~0xC0;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC3 = 0; INT_MSK_P0IC3 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC3 = 0; INT_MSK_P0IC3 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP3_vector

  #elif ( IDLE_PIN == INTP4 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP4 &= 0xF8; \
                                          INT_CTR_REG_INTP4 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM2 &= ~0x03;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC4 = 0; INT_MSK_P0IC4 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC4 = 0; INT_MSK_P0IC4 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP4_vector

  #elif ( IDLE_PIN == INTP5 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP5 &= 0xF8; \
                                          INT_CTR_REG_INTP5 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM2 &= ~0x0C;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC5 = 0; INT_MSK_P0IC5 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC5 = 0; INT_MSK_P0IC5 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP5_vector

  #elif ( IDLE_PIN == INTP6 )
    #define GENI_INT_PRIO_LOW           { INT_CTR_REG_INTP6 &= 0xF8; \
                                          INT_CTR_REG_INTP6 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_PORT_EDGE_TRIG          INTM2 &= ~0x30;                               // Falling edge
    #define GENI_IDLE_IRQ_ENABLE        { INT_PEN_P0IC6 = 0; INT_MSK_P0IC6 = 0; }
    #define GENI_IDLE_IRQ_DISABLE       { INT_PEN_P0IC6 = 0; INT_MSK_P0IC6 = 1; }
    #define SOFT_IDLE_IRQ_Vector        INTP6_vector

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

#endif
#define  SET_ISC0_REG;

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
  #if ( MDM_SYNC_CLK_PIN == INTP0 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP0 &= 0xF8; \
                                          INT_CTR_REG_INTP0 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM1 = ~0x03;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC0 = 0; INT_MSK_P0IC0 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC0 = 0; INT_MSK_P0IC0 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP0_vector

  #elif ( MDM_SYNC_CLK_PIN == INTP1 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP1 &= 0xF8; \
                                          INT_CTR_REG_INTP1 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM1 = ~0x0C;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC1 = 0; INT_MSK_P0IC1 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC1 = 0; INT_MSK_P0IC1 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP1_vector

  #elif ( MDM_SYNC_CLK_PIN == INTP2 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP2 &= 0xF8; \
                                          INT_CTR_REG_INTP2 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM1 = ~0x30;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC2 = 0; INT_MSK_P0IC2 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC2 = 0; INT_MSK_P0IC2 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP2_vector

  #elif ( MDM_SYNC_CLK_PIN == INTP3 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP3 &= 0xF8; \
                                          INT_CTR_REG_INTP3 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM1 = ~0xC0;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC3 = 0; INT_MSK_P0IC3 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC3 = 0; INT_MSK_P0IC3 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP3_vector

  #elif ( MDM_SYNC_CLK_PIN == INTP4 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP4 &= 0xF8; \
                                          INT_CTR_REG_INTP4 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM2 = ~0x03;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC4 = 0; INT_MSK_P0IC4 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC4 = 0; INT_MSK_P0IC4 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP4_vector

  #elif ( MDM_SYNC_CLK_PIN == INTP5 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP5 &= 0xF8; \
                                          INT_CTR_REG_INTP5 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM2 = ~0x0C;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC5 = 0; INT_MSK_P0IC5 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC5 = 0; INT_MSK_P0IC5 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP5_vector

  #elif ( MDM_SYNC_CLK_PIN == INTP6 )
    #define SET_MDM_CLK_PIN_AS_INPUT
    #define GENI_MDM_PRIO_LOW           { INT_CTR_REG_INTP6 &= 0xF8; \
                                          INT_CTR_REG_INTP6 |= INT_PRIO_LEV7;}        // set priority flag to lowest
    #define SET_MDM_CLK_EDGE_TRIG       INTM2 = ~0x30;                                  // Falling edge
    #define GENI_MDM_CLK_IRQ_ENABLE        { INT_PEN_P0IC6 = 0; INT_MSK_P0IC6 = 0; }
    #define GENI_MDM_CLK_IRQ_DISABLE       { INT_PEN_P0IC6 = 0; INT_MSK_P0IC6 = 1; }
    #define GENI_Sync_IRQ_Vector        INTP6_vector

  #else
    #error 'Invalid value for MDM_SYNC_CLK_PIN'

  #endif

  #define FIRST_BIT_NO              23
  #define ENABLE_REG_ACCESS        {SET_PIN(MDM_REG_DATA_PIN);}

  #define DISABLE_REG_ACCESS       {CLEAR_PIN(MDM_REG_DATA_PIN);}

  #define REG_DATA_PIN_SETUP       {PIN_SETUP(MDM_REG_DATA_PIN, OUTPUT_MODE, PULL_UP_NOT_USED, PIN_LOW);}

  #define SETUP_MDM_SYNC_PINS      {GENI_ASYNC_MODE = 0x00;                 /* Disable uart    */  \
                                    GENI_RXD_MODE = AS_BIT_INPUT;           /* set rx as input */  \
                                    GENI_TXD_MODE = AS_BIT_OUTPUT;           /* set tx as output*/ \
                                    GENI_SETUP_RXD_AS_IO;                                          \
                                    GENI_SETUP_TXD_AS_IO;                                          \
                                    SET_MDM_CLK_PIN_AS_INPUT;               /* set clk as input*/  \
                                    SET_MDM_CLK_EDGE_TRIG;                  /* Rising edge trig*/  \
                                    GENI_DIR_SETUP;                         /* Setup dir pin   */  \
                                    PIN_SETUP(MDM_REG_DATA_PIN, OUTPUT_MODE, PULL_UP_NOT_USED, PIN_LOW);}

  #define ENABLE_RX_MODE            SET_PIN(DIR_PIN)
  #define ENABLE_TX_MODE            CLEAR_PIN(DIR_PIN)
  #define RX_LINE                   GENI_RX_LINE
  #define TX_LINE                   GENI_TX_LINE

#endif




#endif /* _DRV_L_H  */
