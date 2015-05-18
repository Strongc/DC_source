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
/* MODULE NAME      :   usb_drv_l.h                                         */
/*                                                                          */
/* FILE NAME        :   usb_drv_l.h                                         */
/*                                                                          */
/* FILE DESCRIPTION :   Redirection of h-file                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _USB_DRV_L_H
#define _USB_DRV_L_H

#include "hw_res.h"
#include "hw_cnf.h"
#include "common.h"

/*****************************************************************************/
/* Specify the channel                                                       */
/*****************************************************************************/
#define CH_SPEC USB                                // PLM, BUS, RS232,COM, USB

/*****************************************************************************/
/* The following macros will adapt the rest of the file to the specified    */
/* channel.                                                                  */
/* Example:                                                                  */
/*  #define CH_SPEC   USB                                                    */
/*  => BAUDRATE will be replaced by BUS_BAUDRATE                             */
/*  and so on.                                                               */
/*****************************************************************************/
#define M_IDLE_COUNT_VAL(a)         IDLE_COUNT_VAL_##a
#define S_IDLE_COUNT_VAL(tmr, baud) M_IDLE_COUNT_VAL(tmr##_##baud)
#define IDLE_COUNT_VAL(tmr,baud)    S_IDLE_COUNT_VAL(tmr,baud)

#define M_CHANNEL(a)                a##_CHANNEL
#define S_CHANNEL(a)                M_CHANNEL(a)
#define CHANNEL                     S_CHANNEL(CH_SPEC)

#define M_BAUDRATE(a)   			a##_BAUDRATE
#define S_BAUDRATE(a)   			M_BAUDRATE(a)
#define BAUDRATE        			S_BAUDRATE(CH_SPEC)

#define M_UART_NUM(a)   			a##_UART_NUM
#define S_UART_NUM(a)   			M_UART_NUM(a)
#define UART_NUM        			S_UART_NUM(CH_SPEC)

#define M_UART_PRIORITY(a)   		a##_UART_PRIORITY
#define S_UART_PRIORITY(a)   		M_UART_PRIORITY(a)
#define UART_PRIORITY        		S_UART_PRIORITY(CH_SPEC)

#define M_DIR_CTR(a)    			a##_DIR_CTR
#define S_DIR_CTR(a)    			M_DIR_CTR(a)
#define DIR_CTR         			S_DIR_CTR(CH_SPEC)

#define M_DIR_PIN(a)    			a##_DIR_PIN
#define S_DIR_PIN(a)    			M_DIR_PIN(a)
#define DIR_PIN         			S_DIR_PIN(CH_SPEC)

#define M_IDLE_TYPE(a)  			a##_IDLE_TYPE
#define S_IDLE_TYPE(a)  			M_IDLE_TYPE(a)
#define IDLE_TYPE       			S_IDLE_TYPE(CH_SPEC)

#define M_IDLE_PIN(a)   			a##_IDLE_PIN
#define S_IDLE_PIN(a)   			M_IDLE_PIN(a)
#define IDLE_PIN        			S_IDLE_PIN(CH_SPEC)

#define M_IDLE_TIMER(a) 			a##_IDLE_TIMER
#define S_IDLE_TIMER(a) 			M_IDLE_TIMER(a)
#define IDLE_TIMER      			S_IDLE_TIMER(CH_SPEC)

#define M_IDLE_COUNT(a) 			a##_IDLE_COUNT
#define S_IDLE_COUNT(a) 			M_IDLE_COUNT(a)
#define IDLE_COUNT      			S_IDLE_COUNT(CH_SPEC)

#define M_soft_timer_started(a)   	a##_soft_timer_started
#define S_soft_timer_started(a)   	M_soft_timer_started(a)
#define soft_timer_started        	S_soft_timer_started(CH_SPEC)

#define M_geni_irq_idle(a)          a##_geni_irq_idle
#define S_geni_irq_idle(a)          M_geni_irq_idle(a)
#define geni_irq_idle               S_geni_irq_idle(CH_SPEC)

#define M_MDM_SYNC_CLK_PIN(a)       a##_MDM_SYNC_CLK_PIN
#define S_MDM_SYNC_CLK_PIN(a)       M_MDM_SYNC_CLK_PIN(a)
#define MDM_SYNC_CLK_PIN            S_MDM_SYNC_CLK_PIN(CH_SPEC)

#define M_MDM_REG_DATA_PIN(a)       a##_MDM_REG_DATA_PIN
#define S_MDM_REG_DATA_PIN(a)       M_MDM_REG_DATA_PIN(a)
#define MDM_REG_DATA_PIN            S_MDM_REG_DATA_PIN(CH_SPEC)

/* Channel specific */
/*****************************************************************************/
/*  Genichannel virtual hardware(for USB) configuration:                     */
/*****************************************************************************/

/*****************************************************************************/
/* TIMER_IDLE settings                                                       */
/*****************************************************************************/
#if(IDLE_TYPE == TIMER_IDLE) 

	#if( IDLE_TIMER == SOFT )
		#define GENI_IDLE_COUNT_VAL          IDLE_COUNT

	    #define GENI_IDLE_TIMER_SETUP        {\
	                                          GENI_DISABLE_GLOBAL_INT();\
	                                          soft_timer_started = FALSE; \
	                                          GENI_MSYSINT0REG |= (1 << 2); \
	                                          GENI_RTCL1HREG = 0; \
	                                          GENI_RTCL1LREG = GENI_RTC_CNT_VALUE;\
	                                          GENI_ENABLE_GLOBAL_INT();\
	                                          }       // start RTC timer

	    #define GENI_IDLE_TIMER_INT_PRIO_LOW
	    #define GENI_IDLE_TIMER_IRQ_ENABLE
	    #define GENI_IDLE_TIMER_START        soft_timer_started = TRUE
	    #define GENI_IDLE_TIMER_STOP         soft_timer_started = FALSE
	  
	    #define GENI_REFRESH_IDLE            {geni_idle_counter=GENI_IDLE_COUNT_VAL; \
	                                         GENI_IDLE_TIMER_STOP;\
	                                         GENI_IDLE_TIMER_START;}    
	#else
		#error 'Invalid Geni idle timer value'
	#endif

	#if ( IDLE_PIN == GENI_NOT_USED)
		#define SET_IDLE_PIN_AS_INPUT
		#define SET_PORT_EDGE_TRIG
		#define GENI_INT_PRIO_LOW
		#define GENI_IDLE_IRQ_ENABLE
		#define GENI_IDLE_IRQ_DISABLE
		#define SOFT_IDLE_IRQ		   
	#else
		#error 'Invalid value for GENI idle pin'
	#endif
#endif	
  #define SET_ISC0_REG;
  #define GENI_REFRESH_TX_IDLE
  #define GENI_CLEAR_RX_ERR
#define GENI_INT_PRIO_LOW

#define USB_TX_IDLE_CNT    ((50 * GENI_RTC_FREQ) / USB_BAUDRATE)

const GF_UINT8 usb_tx_idle_cnt_reset_value = USB_TX_IDLE_CNT;
GF_UINT8 usb_tx_idle_cnt = USB_TX_IDLE_CNT;
GF_UINT32 usb_tx_idle_active = FALSE;

#endif
