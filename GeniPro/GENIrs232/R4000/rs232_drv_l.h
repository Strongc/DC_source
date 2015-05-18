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
/* MODULE NAME      :   rs232_drv_l.h                                         */
/*                                                                          */
/* FILE NAME        :   rs232_drv_l.h                                         */
/*                                                                          */
/* FILE DESCRIPTION :   Defines for the c file                              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _RS232_DRV_L_H
#define _RS232_DRV_L_H

#include "hw_res.h"
#include "hw_cnf.h"
#include "geni_cnf.h"
#include "geni_rtos_if.h"         /* Access to RTOS                         */
#include "RTOS.H"

/*****************************************************************************/
/* Specify the channel                                                       */
/*****************************************************************************/
#define CH_SPEC RS232                                       // BUS, COM, RS232

/*****************************************************************************/
/* The following macroes will adapt the rest of the file to the specified    */
/* channel.                                                                  */
/* Example:                                                                  */
/*  #define CH_SPEC   BUS                                                    */
/*  => BAUDRATE will be replaced by BUS_BAUDRATE                             */
/*  and so on.                                                               */
/*****************************************************************************/

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
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Genichannel hardware configuration:                                      */
/*****************************************************************************/
#if ( UART_NUM == 0 )                                                                 // If UART0 is chosen for GENI
  #define GENI_SIU_IID              GENI_SIU_IID_0
  #define GENI_TXD_MODE             GENI_SIU_SC_0                                           // dummy reg
  #define GENI_RXD_MODE             GENI_SIU_SC_0                                           // dummy reg
  #define GENI_TX_LINE              GENI_SIU_SC_0                                           // dummy reg
  #define GENI_TXD                  GENI_SIU_TH_0                                           // tx reg
  #define GENI_RXD                  GENI_SIU_RB_0                                           // rx reg

  #define GENI_TXD_START
  #define GENI_TXD_ENABLE           { GENI_SIU_SC_0   = (GENI_SIU_IID_0);                     /* read pending irq's to clear tx irq */ \
                                     (GENI_SIU_IE_0) |= 0x02;}                                // set tx mask flag
  #define GENI_TXD_DISABLE          {(GENI_SIU_IE_0) &= ~0x02;}                               // clear tx mask flag
  #define GENI_RXD_ENABLE           {(GENI_SIU_IE_0) &= ~0x02;                                /* clear tx msk flag */        \
                                     (GENI_SIU_SC_0)  = GENI_SIU_RB_0;                        /* read rxd for clearing it */ \
                                     (GENI_SIU_IE_0) |= 0x01;}                                // set rx mask flag
  #define GENI_RXD_DISABLE          {(GENI_SIU_IE_0) &= ~0x01;}                               // clear rx mask flag

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    GENI_DISABLE_GLOBAL_INT();                                                             \
                                    (GENI_SIU_RESET_0)    = 0x01;                              /* Initially reset UART */ \
                                    (GENI_SIU_ACTMSK_0)   = 0xFF;                             /* Activity timer notification mask - disable all */ \
                                    (GENI_SIU_ACTTMR_0)   = 0x00;                             /* Activity timer  disabled */ \
                                    (GENI_SIU_RESET_0)    = 0x00;                             /* Release reset UART */ \
                                    (GENI_SIU_FC_0)       = 0x00;                             /* FIFO control register - disabled */ \
                                    (GENI_SIU_MC_0)       = 0x00;                             /* Modem interface control register - disabled */ \
                                    (GENI_SIU_IE_0)       = 0x00;                             /* disable all uart irqs */ \
                                    GENI_BAUD_INIT_0(baud);                                                                     \
                                    FRAME_SETUP_CH0(stop_bits, parity, GENI_DATA_8_BIT);      /* Uart mode - Communication setup register */ \
                                    (GENI_MSYSINT0REG)   |= 0x1000;                           /* Enable SIU0 interrupt */       \
                                    GENI_ENABLE_GLOBAL_INT();                                                             \
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


  #define GENI_BAUD_INIT_0(baud)                                                                                                \
  {                                                                                                                             \
    (GENI_SIU_LC_0)      |= 0x80;                             /* switch to setup mode */                                        \
    switch (baud)                                                                                                               \
    {                                                                                                                           \
      case GENI_BAUD_1200  : (GENI_SIU_DLL_0) = (UCHAR)DIVISOR_VAL_1200; (GENI_SIU_DLM_0) = (DIVISOR_VAL_1200 >> 8);            \
      break;                                                                                                                    \
      case GENI_BAUD_2400  : (GENI_SIU_DLL_0) = (UCHAR)DIVISOR_VAL_2400; (GENI_SIU_DLM_0) = (DIVISOR_VAL_2400 >> 8);            \
      break;                                                                                                                    \
      case GENI_BAUD_4800  : (GENI_SIU_DLL_0) = (UCHAR)DIVISOR_VAL_4800; (GENI_SIU_DLM_0) = (DIVISOR_VAL_4800 >> 8);            \
      break;                                                                    \
      case GENI_BAUD_9600  : (GENI_SIU_DLL_0) = (UCHAR)DIVISOR_VAL_9600; (GENI_SIU_DLM_0) = (DIVISOR_VAL_9600 >> 8);    \
      break;                                                                    \
      case GENI_BAUD_19200  : (GENI_SIU_DLL_0) = (UCHAR)DIVISOR_VAL_19200; (GENI_SIU_DLM_0) = (DIVISOR_VAL_19200 >> 8);    \
      break;                                                                    \
      case GENI_BAUD_38400  : (GENI_SIU_DLL_0) = (UCHAR)DIVISOR_VAL_38400; (GENI_SIU_DLM_0) = (DIVISOR_VAL_38400 >> 8);    \
      break;                                                                    \
      default              :                                                    \
      break;                                                                    \
    }                                                                           \
  }

#elif ( UART_NUM == 1 )                                                               // If UART1 is chosen for GENI

  #define GENI_SIU_IID              GENI_SIU_IID_1
  #define GENI_ASYNC_MODE
  #define GENI_ASYNC_MODE_SETUP
  #define GENI_ASYNC_MODE_SHIFT
  #define GENI_TXD_MODE             GENI_SIU_SC_1                                           // dummy reg
  #define GENI_RXD_MODE             GENI_SIU_SC_1                                           // dummy reg
  #define GENI_TX_LINE              GENI_SIU_SC_1                                           // dummy reg
  #define GENI_TXD                  GENI_SIU_TH_1                                           // tx reg
  #define GENI_RXD                  GENI_SIU_RB_1                                           // rx reg

  #define GENI_TXD_START
  #define GENI_TXD_ENABLE           { GENI_SIU_SC_1   = (GENI_SIU_IID_1);                     /* read pending irq's to clear tx irq */ \
                                     (GENI_SIU_IE_1) |= 0x02;}                                // set tx mask flag
  #define GENI_TXD_DISABLE          {(GENI_SIU_IE_1) &= ~0x02;}                               // clear tx mask flag
  #define GENI_RXD_ENABLE           {(GENI_SIU_IE_1) &= ~0x02;                                /* clear tx msk flag */        \
                                     (GENI_SIU_SC_1)  = GENI_SIU_RB_1;                        /* read rxd for clearing it */ \
                                     (GENI_SIU_IE_1) |= 0x01;}                                // set rx mask flag
  #define GENI_RXD_DISABLE          {(GENI_SIU_IE_1) &= ~0x01;}                               // clear rx mask flag

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    (GENI_SIU_RESET_1)    = 0x01;                              /* Initially reset UART */ \
                                    GENI_DISABLE_GLOBAL_INT();                                                             \
                                    (GENI_GPMODE1)       &= ~0x1400;                          /* Disable GPIO for RxD1 and TxD1 pins */ \
                                    (GENI_PINMODED1)     |= 0x0018;                           /* Enable RxD1 and TxD1 */ \
                                    (GENI_SIU_ACTMSK_1)   = 0xFF;                             /* Activity timer notification mask - disable all */ \
                                    (GENI_SIU_ACTTMR_1)   = 0x00;                             /* Activity timer  disabled */ \
                                    (GENI_SIU_RESET_1)    = 0x00;                             /* Release reset UART */ \
                                    (GENI_SIU_FC_1)       = 0x00;                             /* FIFO control register - disabled */ \
                                    (GENI_SIU_MC_1)       = 0x00;                             /* Modem interface control register - disabled */ \
                                    (GENI_SIU_IE_1)       = 0x00;                             /* disable all uart irqs */ \
                                    GENI_BAUD_INIT_1(baud);                                                                     \
                                    FRAME_SETUP_CH1(stop_bits, parity, GENI_DATA_8_BIT);      /* Uart mode - Communication setup register */ \
                                    (GENI_MSYSINT0REG )  |= 0x2000;                           /* Enable SIU1 interrupt */  \
                                    GENI_ENABLE_GLOBAL_INT();                                                             \
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


  #define GENI_BAUD_INIT_1(baud)                                                                                                \
  {                                                                                                                             \
    (GENI_SIU_LC_1)      |= 0x80;                             /* switch to setup mode */                                        \
    switch (baud)                                                                                                               \
    {                                                                                                                           \
      case GENI_BAUD_1200  : (GENI_SIU_DLL_1) = (UCHAR)DIVISOR_VAL_1200; (GENI_SIU_DLM_1) = (DIVISOR_VAL_1200 >> 8);            \
      break;                                                                                                                    \
      case GENI_BAUD_2400  : (GENI_SIU_DLL_1) = (UCHAR)DIVISOR_VAL_2400; (GENI_SIU_DLM_1) = (DIVISOR_VAL_2400 >> 8);            \
      break;                                                                                                                    \
      case GENI_BAUD_4800  : (GENI_SIU_DLL_1) = (UCHAR)DIVISOR_VAL_4800; (GENI_SIU_DLM_1) = (DIVISOR_VAL_4800 >> 8);            \
      break;                                                                    \
      case GENI_BAUD_9600  : (GENI_SIU_DLL_1) = (UCHAR)DIVISOR_VAL_9600; (GENI_SIU_DLM_1) = (DIVISOR_VAL_9600 >> 8);    \
      break;                                                                    \
      case GENI_BAUD_19200  : (GENI_SIU_DLL_1) = (UCHAR)DIVISOR_VAL_19200; (GENI_SIU_DLM_1) = (DIVISOR_VAL_19200 >> 8);    \
      break;                                                                    \
      case GENI_BAUD_38400  : (GENI_SIU_DLL_1) = (UCHAR)DIVISOR_VAL_38400; (GENI_SIU_DLM_1) = (DIVISOR_VAL_38400 >> 8);    \
      break;                                                                    \
      default              :                                                    \
      break;                                                                    \
    }                                                                           \
  }


#elif ( UART_NUM == 2 )                                                                       // If UART2 is chosen for GENI

  #define GENI_SIU_IID              GENI_SIU_IID_2
  #define GENI_ASYNC_MODE
  #define GENI_ASYNC_MODE_SETUP
  #define GENI_ASYNC_MODE_SHIFT
  #define GENI_TXD_MODE             GENI_SIU_SC_2                                           // dummy reg
  #define GENI_RXD_MODE             GENI_SIU_SC_2                                           // dummy reg
  #define GENI_TX_LINE              GENI_SIU_SC_2                                           // dummy reg
  #define GENI_TXD                  GENI_SIU_TH_2                                           // tx reg
  #define GENI_RXD                  GENI_SIU_RB_2                                           // rx reg

  #define GENI_TXD_START
  #define GENI_TXD_ENABLE           { GENI_SIU_SC_2   = (GENI_SIU_IID_2);                     /* read pending irq's to clear tx irq */ \
                                     (GENI_SIU_IE_2) |= 0x02;}                                // set tx mask flag
  #define GENI_TXD_DISABLE          {(GENI_SIU_IE_2) &= ~0x02;}                               // clear tx mask flag
  #define GENI_RXD_ENABLE           {(GENI_SIU_IE_2) &= ~0x02;                                /* clear tx msk flag */        \
                                     (GENI_SIU_SC_2)  = GENI_SIU_RB_2;                        /* read rxd for clearing it */ \
                                     (GENI_SIU_IE_2) |= 0x01;}                                // set rx mask flag
  #define GENI_RXD_DISABLE          {(GENI_SIU_IE_2) &= ~0x01;}                               // clear rx mask flag

  #define GENI_INIT_UART(baud, stop_bits, parity, data_len)                                                                     \
  {                                                                                                                             \
                                    (GENI_SIU_RESET_2)    = 0x01;                              /* Initially reset UART */ \
                                    GENI_DISABLE_GLOBAL_INT();                                                             \
                                    (GENI_PINMODED2)     |= 0x0005;                           /* Enable RxD2 and TxD2 */ \
                                    (GENI_SIU_ACTMSK_2)   = 0xFF;                             /* Activity timer notification mask - disable all */ \
                                    (GENI_SIU_ACTTMR_2)   = 0x00;                             /* Activity timer  disabled */ \
                                    (GENI_SIU_RESET_2)    = 0x00;                             /* Release reset UART */ \
                                    (GENI_SIU_FC_2)       = 0x00;                             /* FIFO control register - disabled */ \
                                    (GENI_SIU_MC_2)       = 0x00;                             /* Modem interface control register - disabled */ \
                                    (GENI_SIU_IE_2)       = 0x00;                             /* disable all uart irqs */ \
                                    GENI_BAUD_INIT_2(baud);                                                                     \
                                    FRAME_SETUP_CH2(stop_bits, parity, GENI_DATA_8_BIT);      /* Uart mode - Communication setup register */ \
                                    (GENI_MSYSINT0REG )  |= 0x4000;                           /* Enable SIU2 interrupt */ \
                                    GENI_ENABLE_GLOBAL_INT();                                                             \
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


  #define GENI_BAUD_INIT_2(baud)                                                                                                \
  {                                                                                                                             \
    (GENI_SIU_LC_2)      |= 0x80;                             /* switch to setup mode */                                        \
    switch (baud)                                                                                                               \
    {                                                                                                                           \
      case GENI_BAUD_1200  : (GENI_SIU_DLL_2) = (UCHAR)DIVISOR_VAL_1200; (GENI_SIU_DLM_2) = (DIVISOR_VAL_1200 >> 8);            \
      break;                                                                                                                    \
      case GENI_BAUD_2400  : (GENI_SIU_DLL_2) = (UCHAR)DIVISOR_VAL_2400; (GENI_SIU_DLM_2) = (DIVISOR_VAL_2400 >> 8);            \
      break;                                                                                                                    \
      case GENI_BAUD_4800  : (GENI_SIU_DLL_2) = (UCHAR)DIVISOR_VAL_4800; (GENI_SIU_DLM_2) = (DIVISOR_VAL_4800 >> 8);            \
      break;                                                                    \
      case GENI_BAUD_9600  : (GENI_SIU_DLL_2) = (UCHAR)DIVISOR_VAL_9600; (GENI_SIU_DLM_2) = (DIVISOR_VAL_9600 >> 8);    \
      break;                                                                    \
      case GENI_BAUD_19200  : (GENI_SIU_DLL_2) = (UCHAR)DIVISOR_VAL_19200; (GENI_SIU_DLM_2) = (DIVISOR_VAL_19200 >> 8);    \
      break;                                                                    \
      case GENI_BAUD_38400  : (GENI_SIU_DLL_2) = (UCHAR)DIVISOR_VAL_38400; (GENI_SIU_DLM_2) = (DIVISOR_VAL_38400 >> 8);    \
      break;                                                                    \
      default              :                                                    \
      break;                                                                    \
    }                                                                           \
  }


#else
  #error 'Illegal UART'

#endif

/*****************************************************************************/
/* Direction pin                                                             */
/*****************************************************************************/
#if (DIR_CTR == Enable)                                                               // Direction control enabled?
  #if ( DIR_PIN == GENI_GPIO15 )
    #define GENI_DIR_SETUP             {GENI_DISABLE_GLOBAL_INT();(GENI_GPMODE1) |=  0xC000;GENI_ENABLE_GLOBAL_INT();}                           // Enable GPIO 15 for output - Bit 11 and 15
    #define GENI_DIR_OUTPUT            {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA0) |=  0x8000;GENI_ENABLE_GLOBAL_INT();}                           // Set Pin - Bit 15
    #define GENI_DIR_INPUT             {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA0) &= ~0x8000;GENI_ENABLE_GLOBAL_INT();}                           // Clear Pin - Bit 15

  #elif ( DIR_PIN == GENI_GPIO16 )
    #define GENI_DIR_SETUP             {GENI_DISABLE_GLOBAL_INT();(GENI_GPMODE2) |=  0x0003;GENI_ENABLE_GLOBAL_INT();}                           // Enable GPIO 16 for output - Bit 0 and 1
    #define GENI_DIR_OUTPUT            {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) |=  0x0001;GENI_ENABLE_GLOBAL_INT();}                           // Set Pin - Bit 0
    #define GENI_DIR_INPUT             {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) &= ~0x0001;GENI_ENABLE_GLOBAL_INT();}                          // Clear Pin - Bit 0

  #elif ( DIR_PIN == GENI_GPIO17 )
    #define GENI_DIR_SETUP             {GENI_DISABLE_GLOBAL_INT();(GENI_GPMODE2) |=  0x000C;GENI_ENABLE_GLOBAL_INT();}                           // Enable GPIO 17 for output - Bit 2 and 3
    #define GENI_DIR_OUTPUT            {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) |=  0x0002;GENI_ENABLE_GLOBAL_INT();}                           // Set Pin - Bit 1
    #define GENI_DIR_INPUT             {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) &= ~0x0002;GENI_ENABLE_GLOBAL_INT();}                           // Clear Pin - Bit 1

  #elif ( DIR_PIN == GENI_GPIO20 )
    #define GENI_DIR_SETUP             {GENI_DISABLE_GLOBAL_INT();(GENI_GPMODE2) |=  0x0300;GENI_ENABLE_GLOBAL_INT();}                           // Enable GPIO 20 for output - Bit 8 and 9
    #define GENI_DIR_OUTPUT            {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) |=  0x0010;GENI_ENABLE_GLOBAL_INT();}                           // Set Pin - Bit   4
    #define GENI_DIR_INPUT             {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) &= ~0x0010;GENI_ENABLE_GLOBAL_INT();}                           // Clear Pin - Bit 4

  #elif ( DIR_PIN == GENI_GPIO22 )
    #define GENI_DIR_SETUP             {GENI_DISABLE_GLOBAL_INT();(GENI_GPMODE2) |=  0x3000;GENI_ENABLE_GLOBAL_INT();}                           // Enable GPIO 22 for output - Bit 12 and 13
    #define GENI_DIR_OUTPUT            {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) |=  0x0040;GENI_ENABLE_GLOBAL_INT();}                           // Set Pin - Bit 6
    #define GENI_DIR_INPUT             {GENI_DISABLE_GLOBAL_INT();(GENI_GPDATA1) &= ~0x0040;GENI_ENABLE_GLOBAL_INT();}                           // Clear Pin - Bit 6

  #else
    #error 'DIR_PIN value not supported'
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
  #error 'HW_IDLE not supported'
#endif

/*****************************************************************************/
/* TIMER_IDLE settings                                                       */
/*****************************************************************************/
#if(IDLE_TYPE == TIMER_IDLE)

  #if ( IDLE_TIMER == SOFT )
    #define GENI_IDLE_COUNT_VAL          IDLE_COUNT

    #define GENI_IDLE_TIMER_SETUP        {\
                                          GENI_DISABLE_GLOBAL_INT();                                                             \
                                          soft_timer_started = FALSE; \
                                          GENI_MSYSINT0REG |= (1 << 2); \
                                          GENI_RTCL1HREG = 0; \
                                          GENI_RTCL1LREG = GENI_RTC_CNT_VALUE;       \
                                          GENI_ENABLE_GLOBAL_INT();                                                             \
                                          }       // start RTC timer

    #define GENI_IDLE_TIMER_INT_PRIO_LOW
    #define GENI_IDLE_TIMER_IRQ_ENABLE
    #define GENI_IDLE_TIMER_START        soft_timer_started = TRUE
    #define GENI_IDLE_TIMER_STOP         soft_timer_started = FALSE
  #else
    #error 'Illegal IDLE_TIMER value'
  #endif

   #define GENI_REFRESH_IDLE            {geni_idle_counter=GENI_IDLE_COUNT_VAL; \
                                         GENI_IDLE_TIMER_STOP;\
                                         GENI_IDLE_TIMER_START;}                      // set counter and bit to let timer count
    #ifndef TFT_16_BIT_LCD

  #if ( IDLE_PIN == GENI_GPIO46 )
    #define GENI_INT_PRIO_LOW           { \
                                         GENI_DISABLE_GLOBAL_INT();                                                             \
                                         GENI_GPMODE5 |= (1 << 12);                   /* Enable Pin */  \
                                         GENI_GPMODE5 &= ~(1 << 13);                  /* Set as input */ \
                                         GENI_GPINTTYP5 &= ~((1 << 13) | (1 << 12));  /* Set edge trig and falling edge */ \
                                         GENI_MSYSINT0REG |= (1 << 10);\
                                         GENI_ENABLE_GLOBAL_INT();                                                             \
                                         }              /* enable GPIO 47 - 32 irq */
    #define SET_PORT_EDGE_TRIG
    #define GENI_IDLE_IRQ_ENABLE        { \
                                         GENI_DISABLE_GLOBAL_INT();                                                             \
                                         GENI_GPINTSTAT2 |= (1 << 14);  \
                                         GENI_GPINEN2 |= (1 << 14);\
                                         GENI_GPINTMSK2 |= (1 << 14); \
                                         GENI_ENABLE_GLOBAL_INT();                                                             \
                                         } // Clear status - Enable interrupt

    #define GENI_IDLE_IRQ_DISABLE       {\
                                         GENI_DISABLE_GLOBAL_INT();                                                             \
                                         GENI_GPINTMSK2 &= ~(1 << 14); \
                                         GENI_GPINEN2 &= ~(1 << 14); \
                                         GENI_ENABLE_GLOBAL_INT();                                                             \
                                         }              // Disable interrupt

  #elif ( IDLE_PIN == GENI_GPIO47 )
    #define GENI_INT_PRIO_LOW           {\
                                         GENI_DISABLE_GLOBAL_INT();                                                             \
                                         GENI_GPMODE5 |= (1 << 14);                   /* Enable Pin */  \
                                         GENI_GPMODE5 &= ~(1 << 15);                  /* Set as input */ \
                                         GENI_GPINTTYP5 &= ~((1 << 15) | (1 << 14));  /* Set edge trig and falling edge*/ \
                                         GENI_MSYSINT0REG |= (1 << 10); \
                                         GENI_ENABLE_GLOBAL_INT();                                                             \
                                         }              /* enable GPIO 47 - 32 irq */
    #define SET_PORT_EDGE_TRIG
    #define GENI_IDLE_IRQ_ENABLE        {\
                                         GENI_DISABLE_GLOBAL_INT();                                                             \
                                         GENI_GPINTSTAT2 |= (1 << 15); \
                                         GENI_GPINEN2 |= (1 << 15); \
                                         GENI_GPINTMSK2 |= (1 << 15);\
                                         GENI_ENABLE_GLOBAL_INT();                                                             \
                                         } // clear status and enable interrupt

    #define GENI_IDLE_IRQ_DISABLE       {\
                                         GENI_DISABLE_GLOBAL_INT();                                                             \
                                         GENI_GPINTMSK2 &= ~(1 << 15);\
                                         GENI_GPINEN2 &= ~(1 << 15);\
                                         GENI_ENABLE_GLOBAL_INT();                                                             \
                                           }              // Disable interrupt
  #else
    #error 'IDLE_PIN value not supported'
  #endif
    #else

        #if ( IDLE_PIN == GENI_GPIO52 )
          #define GENI_INT_PRIO_LOW           { \
                                                   GENI_DISABLE_GLOBAL_INT();                                                             \
                                                   GENI_GPMODE6 |= (1 << 8);                   /* Enable Pin */  \
                                                   GENI_GPMODE6 &= ~(1 << 9);                  /* Set as input */ \
                                                   GENI_GPINTTYP6 &= ~((1 << 9) | (1 << 8));  /* Set edge trig and falling edge */ \
                                                   GENI_MSYSINT0REG |= (1 << 11);\
                                                   GENI_ENABLE_GLOBAL_INT();                                                             \
                                                   }              /* enable GPIO 53 - 48 irq */
          #define SET_PORT_EDGE_TRIG
          #define GENI_IDLE_IRQ_ENABLE        { \
                                                   GENI_DISABLE_GLOBAL_INT();                                                             \
                                                   GENI_GPINTSTAT3 |= (1 << 4);  \
                                                   GENI_GPINEN3 |= (1 << 4);\
                                                   GENI_GPINTMSK3 |= (1 << 4); \
                                                   GENI_ENABLE_GLOBAL_INT();                                                             \
                                                   } // Clear status - Enable interrupt

          #define GENI_IDLE_IRQ_DISABLE       {\
                                                   GENI_DISABLE_GLOBAL_INT();                                                             \
                                                   GENI_GPINTMSK3 &= ~(1 << 4); \
                                                   GENI_GPINEN3 &= ~(1 << 4); \
                                                   GENI_ENABLE_GLOBAL_INT();                                                             \
                                                   }              // Disable interrupt

        #elif ( IDLE_PIN == GENI_GPIO53 )
          #define GENI_INT_PRIO_LOW           {\
                                                   GENI_DISABLE_GLOBAL_INT();                                                             \
                                                   GENI_GPMODE6 |= (1 << 10);                   /* Enable Pin */  \
                                                   GENI_GPMODE6 &= ~(1 << 11);                  /* Set as input */ \
                                                   GENI_GPINTTYP6 &= ~((1 << 11) | (1 << 10));  /* Set edge trig and falling edge*/ \
                                                   GENI_MSYSINT0REG |= (1 << 11); \
                                                   GENI_ENABLE_GLOBAL_INT();                                                             \
                                                   }              /* enable GPIO 53 - 48 irq */
          #define SET_PORT_EDGE_TRIG
          #define GENI_IDLE_IRQ_ENABLE        {\
                                                   GENI_DISABLE_GLOBAL_INT();                                                             \
                                                   GENI_GPINTSTAT3 |= (1 << 5); \
                                                   GENI_GPINEN3 |= (1 << 5); \
                                                   GENI_GPINTMSK3 |= (1 << 5);\
                                                   GENI_ENABLE_GLOBAL_INT();                                                             \
                                                   } // clear status and enable interrupt

          #define GENI_IDLE_IRQ_DISABLE       {\
                                                   GENI_DISABLE_GLOBAL_INT();                                                             \
                                                   GENI_GPINTMSK3 &= ~(1 << 5);\
                                                   GENI_GPINEN3 &= ~(1 << 5);\
                                                   GENI_ENABLE_GLOBAL_INT();                                                             \
                                                     }              // Disable interrupt
        #else
          #error 'IDLE_PIN value not supported'
        #endif

    #endif
#else
  #define GENI_REFRESH_IDLE

#endif

#define  SET_ISC0_REG;
#define GENI_REFRESH_TX_IDLE
#define GENI_CLEAR_RX_ERR

/*****************************************************************************/
/* NO_IDLE settings                                                          */
/*****************************************************************************/
#if(IDLE_TYPE == NO_IDLE)
  #define GENI_IDLE_IRQ_ENABLE
  #define GENI_IDLE_IRQ_DISABLE
#endif

/*****************************************************************************/
/* GENI_IRQ_IDLE settings                                                    */
/*****************************************************************************/
#if( IDLE_TYPE == GENI_IRQ_IDLE )
  #error 'GENI_IRQ_IDLE not supported'
#endif



#endif /* _DRV_L_H  */
