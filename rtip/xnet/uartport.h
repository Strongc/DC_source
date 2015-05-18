/* uartport.h - UART driver porting layer.                              */
/*                                                                      */
/* Copyright 1997, EBSnet Inc.                                          */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __UARTPORT__
#define __UARTPORT__ 1

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)

/* ********************************************************************   */
/* all the uart ports are mutually exclusive                              */
#if (AT_MOTHERBOARD )
#define UART_16550      1
#else
#define UART_16550      0
#endif

/* ********************************************************************   */
/* DEFINES FOR UART PORTING LAYER                                         */
/* ********************************************************************   */
#if (UART_16550 )
#define UART_SUPPORTS_HANDSHAKING 1
#define UART_SUPPORTS_INT_POLLING 1
#define UART_SUPPORTS_STATUS_INT  1
#define UART_SUPPORTS_TX_INT      1

#endif

#if (UART_16550)
/* ********************************************************************   */
/* DEFINES FOR 16550 UART                                                 */
/* ********************************************************************   */
/* UART register offsets from port_base                                   */
#define THR    0
#define RBR    0
#define BRDL   0
#define IER    1
#define BRDH   1
#define IIR    2
#define FCR    2
#define LCR    3
#define MCR    4
#define LSR    5    /* line status register */
#define MSR    6

/* Misc   */
#define LSR_RXRDY     1
#define LSR_TBE       0x020     /* transmiter holding register empty */

/* Modem Status Register   */
#define UART_CTS       16       /* inverted clear to send */
#define UART_DSR       32       /* inverted data set ready */
#define UART_INV_DCD    8       /* inverted data carrier detect */
#define UART_RTS       2        /* data set ready */
#define UART_DTR       1        /* clear to send */
#define UART_DCD       8        /* data carrier detect */

/* Line status register definitions   */
#define LS_OVRRUN_ERR 0x2   /* overrun error (data not removed in time) */
#define LS_PARITY_ERR 0x4   /* parity error in read data */
#define LS_FRAME_ERR  0x8   /* framing error (no valid stop bit) */

/* This is the value to put into the IER to turn off transmit INTS      */
/* Note that this is 1101, which enables MODEM status INTS, ERROR INTS, */
/* and receiver INTS.                                                   */
#define THRE_OFF 0xd

/* This is the value to put into the IER to turn on transmit INTS       */
/* Note that this is 1111, which enables MODEM status INTS, ERROR INTS, */
/* TX INTS, and receiver INTS.                                          */
#define THRE_ON  0xf

/* ********************************************************************   */
/* HARDWARE HANDSHAKING                                                   */
/* ********************************************************************   */

/* This macro will un_assert the RTS or DTR line if handshaking is enabled    */
/* to hold off the sender.                                                    */
#define uart_hw_hold_off_sender(uinfo)  \
    if (uinfo->handshake_mask) \
    {                           \
        OUTBYTE(uinfo->port_base + MCR, INBYTE(uinfo->port_base + MCR) & uinfo->holdoff_mask);\
    }

/* ********************************************************************    */
/* This routine will assert the RTS or DTR line, if handshaking is enabled */
/* to signal the sender to resume sending.                                 */
/* Interrupts are disabled when called.                                    */
#define uart_hw_resume_sender(uinfo) if (uinfo->handshake_mask) {OUTBYTE(uinfo->port_base + MCR, INBYTE(uinfo->port_base + MCR) | uinfo->resume_mask);} 

#define HANDSHAKE_MASK 0xFF  /* tbd - fix these */
#define RESUME_MASK 0x00

#endif /* UART_16550 */

#if (defined(SEGMC16))
/* ********************************************************************   */
#define Chip_30600
#include <IOM16C.H>
#include "IPUARTConf.h"

/*
   Define configuration
*/
#if (UARTSEL == 0)
  #define UC0 U0C0
  #define UC1 U0C1
  #define URB U0RB
  #define UMR U0MR
  #define UTB U0TB
  #define UBRG U0BRG
  #define SRIC S0RIC
  #define STIC S0TIC
  #define  TXINT 68
  #define  RXINT 72
#elif (UARTSEL == 1)
  #define UC0 U1C0
  #define UC1 U1C1
  #define URB U1RB
  #define UMR U1MR
  #define UTB U1TB
  #define UBRG U1BRG
  #define SRIC S1RIC
  #define STIC S1TIC
  #define  TXINT 76
  #define  RXINT 80
#else  
  #error 'UARTSEL not defined or invalid'
#endif

#define BAUDSOURCE 1
#define DIV (BAUDSOURCE*16*BAUDRATE)
#define BAUDDIVIDE (((UPCLOCK+DIV/2)/DIV)-1)

/*   MACROs for SIO-Control   */

#define EnableTx()    (UC1 |=  (1<<0))
#define DisableTx()   (UC1 &= ~(1<<0))
#define EnableRxTx()  (UC1  =   0x05)
#define DisableRxTx() (UC1  =   0x00)

#define WAITFORTBE()   while (!(UC1 & (1<<1)));    /* Wait until TB empty */
#define WaitForTxEnd() while (!(UC0 & (1<<3)));    /* Wait until Tx finished */
#endif      /* SEGMC16 */

/* ********************************************************************   */
/* UART INFO                                                              */
/* ********************************************************************   */
typedef struct _uart_info 
{
    int      minor_number;          /* minor number (index into _8250_info_arry) */
    PIFACE   rs232_pi;              /* Pointer to the device driver. */
    int      baud_rate;             /* Baud rate - see above */
    byte     framing_char;
    RTIP_BOOLEAN     raw_mode;              /* TRUE if in raw mode */
    RTIP_BOOLEAN     xmit_on;               /* TRUE if xmitting */
    RTIP_BOOLEAN  handshaking_enabled;
    CIRC_BUFF input_queue;
    CIRC_BUFF output_queue;
    DCU input_dcu;
    DCU output_dcu;
    int comm_port;
    byte xmit_fifo_size;

#if (UART_SUPPORTS_HANDSHAKING)
    RTIP_BOOLEAN we_held_off;
    char    handshake_type;     /* (N)one (D)tr/Dsr (R)ts/Cts */
#endif

#if (UART_16550)
#if (UART_SUPPORTS_HANDSHAKING)
    byte handshake_mask;
    byte holdoff_mask;
    byte resume_mask;
#endif
    RTIP_BOOLEAN is_16550;
    word    port_base;
    int     comm_irq;
    word    int_id;
#if (INCLUDE_MODEM)
#define MODEM_CONNECTED 1
    int     flags;
#endif
#elif (defined(SEGMC16))
#elif (defined SOC_TARGET_CS89712)
#elif (EBSSMCUART)
/* SMC uart is set up in eth860   */
#else
#error Implement hardware-specific members of UART_INFO struct.
#endif
} UART_INFO;
typedef UART_INFO KS_FAR *PUART_INFO; 

#endif /* (C)SLIP, PPP */

#endif      /* __UARTPORT__ */

