
/* uartport.c - UART driver porting layer.                              */
/*                                                                      */
/* Copyright 1997, EBSnet Inc.                                          */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER


#include "sock.h"
#include "rtip.h"
#include "uart.h"

#if (INCLUDE_SLIP || INCLUDE_PPP || INCLUDE_CSLIP)
#if (!EBSSMCUART)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define LOG_OUTPUT_CHARS 0  /* set to 1 to save characters ouput to */
                            /* the uart; chars will be saved in obuf   */
#define LOG_INPUT_CHARS 0   /* set to 1 to save characters input to */
                            /* the uart; chars will be saved in obuf   */
#define DEBUG_XMIT      0

#define DONT_OPEN_UART 0  /* for testing only */

#if (defined(SEGMC16))
/* ********************************************************************   */
int LastInterrupt = UART_ERROR_INTERRUPT;
int NumRxChars=0;
U8  RxChar;
int InCnt;
U8  RxBuf[256];
#endif

/* ********************************************************************   */
#if (LOG_OUTPUT_CHARS)
#define OBUF_SIZE 4096
char KS_FAR obuf[OBUF_SIZE];
static char KS_FAR *obuf_ptr = obuf;
static char KS_FAR * const obuf_max = &obuf[OBUF_SIZE];

void Nputchr(byte cc);

void Nputchr(byte cc)
{
    if (obuf_ptr < obuf_max)
        *obuf_ptr++ = cc;
}                
#endif

#if (!defined(SEGMC16))
/* ********************************************************************   */
/* This function takes the variables set up by uart_init and sets up any
information that will be needed by the porting layer later on... any variables
that are set up here should be conditional members of UART_INFO.

Things that generally have to be done here:
Set up a port I/O base
Initialize the control register(s)
Clear existing/pending interrupts
Turn on I/O
Enable the RX Interrupt and disable tx interrupt
Hook the interrupts
Set uinfo->xmit_fifo_size to the proper value (1 is always safe)

Pseudo-code for the 16550 version of this routine:
#if hardware handshaking is supported/turned on
  set holdoff mask, handshake mask, and resume mask,
   based on what kind of handshaking requested. These
   are used for quick manipulation of registers later on.
#endif

  set port base and irq number based on uinfo->comm_port
  manipulate registers to set the baud rate in the uart to uinfo->baud_rate

*/
/* ********************************************************************   */
RTIP_BOOLEAN uart_hw_init(PUART_INFO uinfo)
{
#if (UART_16550)
byte baud_rate_divisor;
word port_base;
byte comm_irq;
byte temp_iir;
#if (DONT_OPEN_UART)
    return(TRUE);
#endif

#if (UART_SUPPORTS_HANDSHAKING)
    if (uinfo->handshake_type == 'R')
    {
#if (DEBUG_HANDSHAKING)
        DEBUG_ERROR("DOING R HANDSHAKING", NOVAR, 0, 0);
#endif
         uinfo->handshake_mask = UART_CTS;
         uinfo->holdoff_mask = (byte)(~UART_RTS);
         uinfo->resume_mask = UART_RTS;
    }
    else if (uinfo->handshake_type == 'D')
    {
       uinfo->handshake_mask = UART_DSR;
       uinfo->holdoff_mask = (byte)(~UART_DTR);
       uinfo->resume_mask = UART_DTR;
    }
    else
    {
       uinfo->handshake_mask = 0;
       uinfo->holdoff_mask = (byte)(~UART_DTR);
       uinfo->resume_mask = UART_DTR;
    }
#endif /* UART_SUPPORTS_HANDSHAKING */

    switch (uinfo->comm_port)
    {
    case 1 :
        port_base = 0x3F8;   /* I/O address for COM1  */
        comm_irq  =     4;   /* IRQ for COM1  */
        break;
    case 2 :
        port_base = 0x2F8;   /* I/O address for COM2  */
        comm_irq  =     3;   /* IRQ for COM2  */
        break;
    case 3 :
        port_base = 0x3E8;   /* I/O address for COM3  */
        comm_irq  =     4;   /* IRQ for COM3  */
        break;
    case 4 :
        port_base = 0x2E8;   /* I/O address for COM4  */
        comm_irq  =     3;   /* IRQ for COM4  */
        break;
    default:
        DEBUG_ERROR("uart_hw_init: illegal comm port: ", EBS_INT1, uinfo->comm_port, 0);
        return(FALSE);
    }

    /* ******                                                       */
    /* convert baud rate to divisor and check if baud rate is valid */
    switch (uinfo->baud_rate)
    {
    case 1152:
        baud_rate_divisor = 1;
        break;
    case 560:
        baud_rate_divisor = 2;
        break;
    case 384:
        baud_rate_divisor = 3;
        break;
    case 192:
        baud_rate_divisor = 6;
        break;
    case 96:
        baud_rate_divisor = 12;
        break;
    case 24:
        baud_rate_divisor = 48;
        break;
    case 12:
        baud_rate_divisor = 96;
        break;
    default:
        DEBUG_ERROR("uart_hw_init - bad baud rate = ", EBS_INT1, uinfo->baud_rate, 0);
        return(FALSE); /*  set_errno(EBADBAUD); */
    }

    /* Initialize UART                                   */
    /* interrupts were disabled already by the caller    */

    /* Set communication rate and parameters   */
    OUTBYTE(port_base + LCR, 0x80);   /* divisor access */
    io_delay();
    OUTBYTE(port_base + BRDL, baud_rate_divisor);
    io_delay();
    OUTBYTE(port_base + BRDH, 0);
    io_delay();
    OUTBYTE(port_base + LCR, 0x03);   /* divisor off, no parity, 1 stop bit, 8 bits */
    io_delay();

    /* Clear existing interrupts   */
    INBYTE(port_base + LSR);
    io_delay();
    INBYTE(port_base + MSR);
    io_delay();
    INBYTE(port_base + RBR);
    io_delay();

    /* Enable FIFO with receive interrupt after 8 char   */
    OUTBYTE(port_base + FCR, 0x87);
    io_delay();

    uinfo->xmit_fifo_size = 1;
    temp_iir = (byte)(INBYTE(port_base + IIR) & 0xf0);
    if (temp_iir == 0xc0)
    {
        DEBUG_ERROR("UART: 16550A", NOVAR, 0, 0);
        uinfo->is_16550=TRUE;
        uinfo->xmit_fifo_size = 16;
    }

    /* Enable read buffer full and receiver line status interrupts   */
    /* but not TX ints. Also enable modem status change INTS.        */
    
    OUTBYTE(port_base + IER, THRE_OFF);
    io_delay();

    /* cause uart_interrupt to execute when interrupt occurs and   */
    /* uart_receive to be called by the interrupt task             */
    ks_hook_interrupt(comm_irq, (PFVOID) 0, 
                      (RTIPINTFN_POINTER)uart_interrupt, 
                      (RTIPINTFN_POINTER) 0, uinfo->minor_number);
    io_delay();

    /* Enable interrupt signal and DTR    */
    OUTBYTE(port_base + MCR, 0x0b);   
    io_delay();

    uinfo->port_base=port_base;
    uinfo->comm_irq=comm_irq;
#else
#error: Set up any hardware dependent driver info here, and put it in UART_INFO
#endif
    return(TRUE);
}

/* ********************************************************************   */
/* Completely turn off the uart.  No more interrupts should be generated
    or anything. Called by uart_close(). */
void uart_hw_disable(PUART_INFO uinfo)
{
    ks_disable();

#if (UART_16550)
    /* Mask off all uart interrupts   */
    OUTBYTE(uinfo->port_base + IER, 0);
#endif
    ks_enable();
}

#if (UART_SUPPORTS_STATUS_INT)
/* ********************************************************************   */
void uart_get_status(PUART_INFO uinfo, PUART_STATUS uart_status)
{
#if (UART_16550)
byte msr;       /* modem status register */

    /* read the modem status register only once since reading it   */
    /* will clear it                                               */
    msr = INBYTE(uinfo->port_base + MSR);

#if (UART_SUPPORTS_HANDSHAKING)
    uart_status->tx_handshake = (RTIP_BOOLEAN)(msr & uinfo->handshake_mask);
#endif
#if (INCLUDE_MODEM)
    if (uinfo->flags & MODEM_CONNECTED)
    {
#    if (DEBUG_MODEM)
        DEBUG_ERROR("check for MODEM DROP", NOVAR, 0, 0);
#    endif
        uart_status->modem_drop   = (RTIP_BOOLEAN)(msr & UART_INV_DCD);
        if (uart_status->modem_drop)
            uinfo->flags &= ~MODEM_CONNECTED;
    }
    else
    {
        uart_status->modem_drop   = FALSE;
        uinfo->flags |= MODEM_CONNECTED;
    }
#endif
#else
#error: Implement checking if remote can recv or modem
#endif
}
#endif  /* UART_SUPPORTS_STATUS_INT */

#if (UART_SUPPORTS_HANDSHAKING)
/* uart_hw_poll_tx_handshake   */
/* Implement this only if you will have handshaking capability in your UART.
 If you won't, then turn off UART_SUPPORTS_HANDSHAKING for your target. 
 This function should return TRUE if the remote host is ready to receive
 data, FALSE otherwise. This function is called from the ISR. */
RTIP_BOOLEAN uart_hw_poll_tx_handshake(PUART_INFO uinfo)
{
dword msr_reg;
#if (UART_16550)
/*  return ((RTIP_BOOLEAN)(INBYTE(uinfo->port_base + MSR) & uinfo->handshake_mask));   */
    msr_reg = INBYTE(uinfo->port_base + MSR);
    if (!(msr_reg & uinfo->handshake_mask))
    {
        DEBUG_ERROR("uart_hw_poll_tx_handshake: msr, mask = ",
            DINT2, msr_reg, uinfo->handshake_mask);
    }
    return ((RTIP_BOOLEAN)(msr_reg & uinfo->handshake_mask));
#else
#error Implement checking if remote can recv
#endif
}

/* uart_hw_enable_sender()   */
/* Implement this only if you will have handshaking capability in your UART.
 If you won't, then turn off UART_SUPPORTS_HANDSHAKING for your target.
 This function is 
 */
void uart_hw_enable_sender(PUART_INFO uinfo)
{
#if (UART_16550)
DEBUG_ERROR("ENABLE SENDER", NOVAR, 0, 0);
    OUTBYTE(uinfo->port_base + MCR, INBYTE(uinfo->port_base + MCR) | RESUME_MASK); 
#else
#error Implement hw handshaking, rx enable, or turn off SUPPORTS_HANDSHAKING
#endif
}

/* ********************************************************************   */
#if (!UART_16550)
/* 16550 is done with a macro in uartport.h   */
void uart_hw_resume_sender(PUART_INFO uinfo)
{
#if (UART_16550)
    if (uinfo->handshake_mask)
    {
        OUTBYTE(uinfo->port_base + MCR, INBYTE(uinfo->port_base + MCR) | uinfo->resume_mask); 
        io_delay();
    }
#else
#error Implement hw handshaking, rx enable, or turn off SUPPORTS_HANDSHAKING
#endif
}

#endif /* (UART_16550) */




/* ********************************************************************   */
/* uart_hw_holdoff_sender()                                               */
/* Implement this only if you will have handshaking capability in your UART.
 If you won't, then turn off UART_SUPPORTS_HANDSHAKING for your target.
 This function is 
 */
# if (!UART_16550)
/* 16550 is done with a macro in uartport.h   */

void uart_hw_hold_off_sender(PUART_INFO uinfo)
{
#if (UART_16550)
    if (uinfo->handshake_mask) 
    {
DEBUG_ERROR("HOLD OFF SENDER", NOVAR, 0, 0);
        OUTBYTE(uinfo->port_base + MCR, INBYTE(uinfo->port_base + MCR) & uinfo->holdoff_mask ); 
        io_delay();
    }
#else
#error Implement hw handshaking, rx enable, or turn off SUPPORTS_HANDSHAKING
#endif
}
#endif /* (UART_16550) */

#endif /* SUPPORTS_HANDSHAKING */

/* ********************************************************************   */
/* uart_hw_enable_tx_int()                                                */
/* Enable the transmit interrupt.  Called when data is ready to be sent.
   This routine should be ISR safe. */
void uart_hw_enable_tx_int(PUART_INFO uinfo)
{
#if (UART_16550)
    if (!uinfo->is_16550 || !uinfo->xmit_on)
    {
#if (DEBUG_XMIT)
        DEBUG_ERROR("uart_hw_enable_tx_int: start xmit", NOVAR, 0, 0);
#endif

        /* Start tranmitting by toggling xmit reg empty in interrupt enable reg   */
        OUTBYTE(uinfo->port_base + IER, THRE_OFF);  /* THRE off */
        io_delay();
        OUTBYTE(uinfo->port_base + IER, THRE_ON);  /* THRE on */
        io_delay();
    }
#else
#error: Implement uart enable tx interrupts
#endif
}

#if (!defined(UART_VECTORED_INTS))  
/* ********************************************************************   */
/* uart_hw_get_interrupt_type()                                           */
/* We only have one ISR in our uart driver.  For systems where the type of
   interrupt (i.e. input, output, error) is determined by vector number, a
   global variable can be set that this routine can read and return.
   Because of the PC architecture, where all interrupt types for the UART
   come in on the same IRQ, we have one handler that calls this routine to
   determine which kind of interrupt it is dealing with.  This routine should
   return the type of the last interrupt that fired.  The types are:

    UART_ERROR_INTERRUPT on error
    UART_INPUT_INTERRUPT on input int
    UART_OUTPUT_INTERRUPT on output int
    UART_STATUS_INTERRUPT on status int
*/

int uart_hw_get_interrupt_type(PUART_INFO uinfo)
{
#if (UART_16550)
    /* Hack :) This works because of the way the constants are set up.   */
    return uinfo->int_id&0x06;
#else
#error: Implement handling of interrupt type
#endif
}
#endif  /* !defined(UART_VECTORED_INTS) */

/* ********************************************************************   */
#if (UART_SUPPORTS_INT_POLLING)
/* This should check and see whether the interrupt needs servicing.  The
   first time is is called after an interrupt it should always return true.
   Otherwise, no interrupt processing will be done.  It should also get and store
   an interrupt type in the uinfo struct so that uart_hw_get_interrupt_type
   can return what kind of interrupt needed servicing.  If the UART will never
   glob events together into a single interrupt, then turn off
   UART_SUPPORTS_INT_POLLING.
*/

RTIP_BOOLEAN uart_hw_needs_servicing(PUART_INFO uinfo)
{
#if (UART_16550)
    uinfo->int_id=(word)INBYTE(uinfo->port_base+IIR);
    return ((RTIP_BOOLEAN)((uinfo->int_id & 0x0001) == 0));
#else 
#error: Implement interrupt polling or turn off SUPPORTS_INT_POLLING
#endif
}
#endif /* UART_SUPPORTS_INT_POLLING */

/* ********************************************************************   */
/* uart_hw_check_error()                                                  */
/* Only called after uart_hw_get_interrupt_type returned 
   UART_ERROR_INTERRUPT.  This function should return which error it was,
   for reporting to the rs232.c layer.  The codes are:
   0   Overrun Error
   1   Parity Error
   2   Framing Error
*/

int uart_hw_check_error(PUART_INFO uinfo)
{
#if (UART_16550)
byte err_flag = (byte)(INBYTE(uinfo->port_base + LSR) & 0x1eu);

    if (err_flag)
    {
        if (err_flag & LS_OVRRUN_ERR)   return 0;
        if (err_flag & LS_PARITY_ERR)   return 1;
        if (err_flag & LS_FRAME_ERR)    return 2;
    }
    io_delay();
    return -1;
#else
#error: Implement checking error status 
#endif
}

/* ********************************************************************   */
/* uart_hw_rx()                                                           */
/* Read one byte of data from the uart, and return it.
   Is only called after a transmit interrupt fires (i.e. 
   uart_hw_get_interrupt_type() returned UART_INPUT_INTERRUPT.) */
byte uart_hw_rx(PUART_INFO uinfo)
{
#if (UART_16550)
    return (byte) INBYTE(uinfo->port_base + RBR);

#else
#error: Implement retrieval of char from UART
#endif
}

#if (!defined(ONE_BYTE_FIFO)) 
/* ********************************************************************   */
/* uart_hw_poll_rx()                                                      */
/* Checks to see if the data buffer in the UART has any data in it.  
   This is useful for UARTs with FIFOs, where one interrupt may 
   fire because the FIFO is full.  For UARTs with a one-byte FIFO,
   it suffices to just check and see if that's full.  If your
   UART does not have any method for checking its data buffer, then 
   something must be done.  We do not support this condition yet. */
RTIP_BOOLEAN uart_hw_poll_rx(PUART_INFO uinfo)
{ 
#if (UART_16550)
    return ((RTIP_BOOLEAN)(INBYTE(uinfo->port_base + LSR) & LSR_RXRDY));

#else
#error: Implement polling of the uart buffer
#endif
}
#endif  /* !defined(ONE_BYTE_FIFO) */

/* ********************************************************************   */
/* uart_hw_disable_tx_int()                                               */
/* Temporarily disable the transmit interrupt.  Called when there is not
any more data queued up to send.*/
void uart_hw_disable_tx_int(PUART_INFO uinfo)
{
#if (UART_16550)
    OUTBYTE(uinfo->port_base + IER, THRE_OFF); io_delay();

#else
#error: Implement disabling uart tx interrupts
#endif
}

/* ********************************************************************   */
/* Queue up some data to send.  This is only called when uart_hw_poll_tx
returns true, which means that there is some space in the UART to hold some
data.  Therefore, this routine does not need to check the uart to see if data
can be written, because uart_hw_poll_tx has been called. */
void uart_hw_tx(PUART_INFO uinfo, byte data)
{
#if (UART_16550)
    OUTBYTE(uinfo->port_base + THR, data); io_delay();

#else
#error: Implement sending data over uart
#endif
}

#if (UART_SUPPORTS_STATUS_INT)
/* ********************************************************************   */
/* Return TRUE if there is space in the UARTs registers to hold transmit
data. Only called after a transmit or status interrupt happens. 
Return TRUE if there is space, FALSE if not.  This is useful for when
the UART has a FIFO that can be filled up multiple times on each tx 
interrupt. */
RTIP_BOOLEAN uart_hw_poll_tx(PUART_INFO uinfo)
{           
#if (UART_16550)
    return ((RTIP_BOOLEAN)(INBYTE(uinfo->port_base + LSR) & LSR_TBE));

#else
#error: Implement buffer checking for UART chip
#endif
}
#endif  /* UART_SUPPORTS_STATUS_INT */

/* ********************************************************************   */
/* This routine is pretty much specific to the 16550/8250 UARTs.  When
a status interrupt comes in on these uarts, it is necessary to ack
the interrupt by reading some registers.  Thus, this function exists. */
/* TBD - move this to get_interrupt_type?   */
void uart_hw_clear_interrupt(PUART_INFO uinfo)
{
#if (UART_16550)
    INBYTE(uinfo->port_base + LSR); /* clear status interrupt */
    INBYTE(uinfo->port_base + MSR);
#endif
}
    
#endif  /* !defined(SEGMC16)) */


#if (defined(SEGMC16))
/* ********************************************************************   */
/* SEGGER MC16 UART DRIVER                                                */
/* ********************************************************************   */
void interrupt [RXINT] uart_rx_interrupt(void)  
{
  /*  Get new character   */
  int SioInput = URB;
  OS_EnterInterrupt();
  if (SioInput & 0xf000) {
#    ifdef DEBUG
    if (SioInput & 0x1000)
      HandleRxErrOverrun();
    if (SioInput & 0x2000)
      HandleRxErrFraming();
    if (SioInput & 0x4000)
      HandleRxErrParity();
#    endif
    UC1 &= ~(1<<2);      /* disable Rx */
    UC1 |=  (1<<2);      /* enable Rx */
/*    HandleRxErr();   */
  } else {
/*    HandleRxChar(SioInput&255);   */
    LastInterrupt = UART_INPUT_INTERRUPT;
    NumRxChars=1;
    RxChar = SioInput;
    if (InCnt <sizeof(RxBuf))
      RxBuf[InCnt] = SioInput;
    if (InCnt++)
    uart_interrupt(0);
  }
  OS_LeaveInterrupt();
}

void interrupt [TXINT] uart_tx_interrupt(void)  
{
  LastInterrupt = UART_OUTPUT_INTERRUPT;
  uart_interrupt(0);
}


/* ********************************************************************     */
/* This function takes the variables set up by uart_init and sets up any
information that will be needed by the porting layer later on... any variables
that are set up here should be conditional members of UART_INFO.

Things that generally have to be done here:
Set up a port I/O base
Initialize the control register(s)
Clear existing/pending interrupts
Turn on I/O
Enable the RX Interrupt and disable tx interrupt
Hook the interrupts
Set uinfo->xmit_fifo_size to the proper value (1 is always safe)

Pseudo-code for the 16550 version of this routine:
#if hardware handshaking is supported/turned on
  set holdoff mask, handshake mask, and resume mask,
   based on what kind of handshaking requested. These
   are used for quick manipulation of registers later on.
#endif

  set port base and irq number based on uinfo->comm_port
  manipulate registers to set the baud rate in the uart to uinfo->baud_rate

*/

RTIP_BOOLEAN uart_hw_init(PUART_INFO uinfo) {
  uinfo->xmit_fifo_size = 1;
  UMR   = 0x00;        /*  lock Sio, error reset */
  UC0   = 0x10;           /*  RTS/CTS disabled, clock divisor 1 */
  UBRG  = BAUDDIVIDE;  /*  Calculated Baudrate */
  DisableRxTx( );       /*  Lock Rx and Tx */
  UMR   = 0x05           /*  8 Data */
          +(0<<4)       /*  0: one stop bit */
          +(0<<5)       /*  X: parity selection */
          +(0<<6)       /*  0: parity disable */
          +(0<<7);      /*  0: no sleep */
  UCON  = 0x00;        /*  transmit-interrupt on buffer empty */
  EnableRxTx( );           /*  enable reception and transmition */
  SRIC = 1;            /* enable SIO interrupts */
  STIC = 1;            /* enable SIO interrupts */
  uinfo = uinfo;
  return( TRUE );
}

/* ********************************************************************     */
/* Completely turn off the uart.  No more interrupts should be generated
    or anything. Called by uart_close(). */
void uart_hw_disable(PUART_INFO uinfo) {
  KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
  sp = ks_splx( );
  ks_spl( sp );
  uinfo = uinfo;
  while ( 1 );
}


/* ********************************************************************      */
/* uart_hw_enable_tx_int()                                                   */
/* Enable the transmit interrupt.  Called when data is ready to be sent.
   This routine should be ISR safe. */
void uart_hw_enable_tx_int(PUART_INFO uinfo) 
{
  uinfo = uinfo;
  EnableTx();
  LastInterrupt = UART_OUTPUT_INTERRUPT;
  uart_interrupt(0);
}

/* ********************************************************************      */
/* uart_hw_get_interrupt_type()                                              */
/* We only have one ISR in our uart driver.  For systems where the type of
   interrupt (i.e. input, output, error) is determined by vector number, a
   global variable can be set that this routine can read and return.
   Because of the PC architecture, where all interrupt types for the UART
   come in on the same IRQ, we have one handler that calls this routine to
   determine which kind of interrupt it is dealing with.  This routine should
   return the type of the last interrupt that fired.  The types are:

    UART_ERROR_INTERRUPT on error
    UART_INPUT_INTERRUPT on input int
    UART_OUTPUT_INTERRUPT on output int
    UART_STATUS_INTERRUPT on status int
*/

int uart_hw_get_interrupt_type(PUART_INFO uinfo) {
  uinfo = uinfo;    /* we dont care !!! */
  return LastInterrupt;
}


/* ********************************************************************      */
/* uart_hw_check_error()                                                     */
/* Only called after uart_hw_get_interrupt_type returned 
   UART_ERROR_INTERRUPT.  This function should return which error it was,
   for reporting to the rs232.c layer.  The codes are:
   0   Overrun Error
   1   Parity Error
   2   Framing Error
*/

int uart_hw_check_error(PUART_INFO uinfo) {
  uinfo = uinfo;
  return -1;
}

/* ********************************************************************      */
/* uart_hw_rx()                                                              */
/* Read one byte of data from the uart, and return it.
   Is only called after a transmit interrupt fires (i.e. 
   uart_hw_get_interrupt_type() returned UART_INPUT_INTERRUPT.) */
byte uart_hw_rx(PUART_INFO uinfo) {
  uinfo = uinfo;
  NumRxChars--;
  return RxChar;
}

/* ********************************************************************      */
/* uart_hw_poll_rx()                                                         */
/* Checks to see if the data buffer in the UART has any data in it.  
   This is useful for UARTs with FIFOs, where one interrupt may 
   fire because the FIFO is full.  For UARTs with a one-byte FIFO,
   it suffices to just check and see if that's full.  If your
   UART does not have any method for checking its data buffer, then 
   something must be done.  We do not support this condition yet. */
RTIP_BOOLEAN uart_hw_poll_rx(PUART_INFO uinfo) 
{ 
  uinfo = uinfo;
  return NumRxChars;
}

/* ********************************************************************      */
/* uart_hw_disable_tx_int()                                                  */
/* Temporarily disable the transmit interrupt.  Called when there is not
any more data queued up to send.*/
void uart_hw_disable_tx_int(PUART_INFO uinfo) 
{
  uinfo = uinfo;
}

/* ********************************************************************     */
/* Queue up some data to send.  This is only called when uart_hw_poll_tx
returns true, which means that there is some space in the UART to hold some
data.  Therefore, this routine does not need to check the uart to see if data
can be written, because uart_hw_poll_tx has been called. */
void uart_hw_tx(PUART_INFO uinfo, byte data) 
{
  uinfo= uinfo;
  UTB = data;
}

/* ********************************************************************     */
/* Return TRUE if there is space in the UARTs registers to hold transmit
data. Only called after a transmit or status interrupt happens. 
Return TRUE if there is space, FALSE if not.  This is useful for when
the UART has a FIFO that can be filled up multiple times on each tx 
interrupt. */
RTIP_BOOLEAN uart_hw_poll_tx(PUART_INFO uinfo) {           
  uinfo = uinfo;
  return 0;
}

/* This routine is pretty much specific to the 16550/8250 UARTs.  When
a status interrupt comes in on these uarts, it is necessary to ack
the interrupt by reading some registers.  Thus, this function exists. */
/* TBD - move this to get_interrupt_type?     */
void uart_hw_clear_interrupt(PUART_INFO uinfo) {
  uinfo = uinfo;
}
#endif /* defined(SEGMC16) */
#endif /* (!EBSSMCUART) */

#endif  /* INCLUDE_SLIP, PPP, CSLIP */


