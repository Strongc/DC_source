/*                                                                        */
/* UART.H  - RTIP header file                                             */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains prototypes constants and structure           */
/*        declarations required to build uart driver for RTIP.            */
/*                                                                        */
#ifndef __UART__
#define __UART__ 1

#include "circbuff.h"
#include "uartport.h"

#define DEBUG_HANDSHAKING 1

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
/* used for passing info to uart_interrupt   */
#define UART_NO_INTERRUPT       0xf
#define UART_ERROR_INTERRUPT    0x6
#define UART_INPUT_INTERRUPT    0x4
#define UART_OUTPUT_INTERRUPT   0x2
#define UART_STATUS_INTERRUPT   0x0

/* status of UART when get status interrupt   */
typedef struct uart_status
{
    RTIP_BOOLEAN tx_handshake;  /* if the remote host is ready to receive data */
    RTIP_BOOLEAN modem_drop;        /* if modem dropped */
} UART_STATUS;
typedef struct uart_status KS_FAR * PUART_STATUS;

/* Note:  Some of these may be implemented as macros, inlines, or fns.   */
void uart_hw_set_baud(PUART_INFO uinfo, int speed);

#if (!defined(UART_VECTORED_INTS))
int uart_hw_get_interrupt_type(PUART_INFO uinfo);
#endif

/* Used in uart_hw_close().  Makes the uart no longer fire any interrupts.   */
void uart_hw_disable(PUART_INFO uinfo);

/* Get uart status after a status interrupt   */
void uart_get_status(PUART_INFO uinfo, PUART_STATUS uart_status);

#if (UART_SUPPORTS_STATUS_INT)  
/* Polls to see if a char can be sent.  This   */
/* is uart buffering, NOT RTS/CTS handshaking. */
RTIP_BOOLEAN uart_hw_poll_tx(PUART_INFO uinfo);
#endif

#if UART_SUPPORTS_HANDSHAKING
/* Polls to see if a char can be sent.  This                                */
/* is RTS/CTS handshaking, NOT uart buffering.                              */
/* TBD - check and see if differentiation between buffering and handshaking */
/* needs to be made.                                                        */
RTIP_BOOLEAN uart_hw_poll_tx_handshake(PUART_INFO uinfo);

/* Same as above, but for rx   */
RTIP_BOOLEAN uart_hw_poll_rx_handshake(PUART_INFO uinfo);

/* If supported, set the handshake low so that the sender will not send.   */
/* if not supported, do nothing.                                           */
#if (!UART_16550)
/* These are done with macros for the 550   */
void uart_hw_hold_off_sender(PUART_INFO uinfo);
void uart_hw_enable_sender(PUART_INFO uinfo);
#endif
#endif

#if UART_SUPPORTS_INT_POLLING
/* Checks to see if an interrupt needs to be serviced.                    */
/* Used in the ISR too see if another interrupt has fired during the ISR. */
RTIP_BOOLEAN uart_hw_poll_iir(PUART_INFO uinfo);
#endif

#if (!defined(ONE_BYTE_FIFO))    
/* Polls to see if a char is waiting.   */
RTIP_BOOLEAN uart_hw_poll_rx(PUART_INFO uinfo);
#endif

#if UART_SUPPORTS_TX_INT
void uart_hw_enable_tx_int(PUART_INFO uinfo);
void uart_hw_disable_tx_int(PUART_INFO uinfo);
#endif

/* Check to see if the last interrupt fired because of an error, and return   */
/* a code to see which one.                                                   */
int uart_hw_check_error(PUART_INFO uinfo);

/* gets and sends bytes over the rs232 cable.   */
byte uart_hw_rx(PUART_INFO uinfo);
void uart_hw_tx(PUART_INFO uinfo, byte data);

#if (!defined(UART_VECTORED_INTS))
void uart_interrupt(int n);
#else
void uart_interrupt(int n,int intr_type);
#endif

RTIP_BOOLEAN uart_hw_init(PUART_INFO uinfo);

void uart_hw_clear_interrupt(PUART_INFO uinfo);

RTIP_BOOLEAN uart_hw_needs_servicing(PUART_INFO uinfo);

#endif /* __UART__ */

#endif /* (C)SLIP, PPP */

