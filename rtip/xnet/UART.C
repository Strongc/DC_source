/* ************************************************************************   */
/* UART.C - Portable UART Driver                                              */
/* ************************************************************************   */
/*                                                                            */
/* Copyright 1998, EBSnet Inc.                                                */
/* All rights reserved.                                                       */
/* This code may not be redistributed in source or linkable object form       */
/* without the consent of its author.                                         */
/*                                                                            */
/* Modified: 9/97 Ansel Freniere - Added porting layer                        */
/* Rewritten: 4/97 Peter Van Oudenaren                                        */
/* Rewritten: Ralph Moore 6/7/94 for use with SLIP                            */
/* Authors: Frank Rybicki and Bill Schubert                                   */
/*                                                                            */
/*                                                                            */
/*  UART.C and UARTPORT.C comprise a portable uart driver. It provides        */
/*  queued input and output and interrupt driven sends and receives.          */
/*                                                                            */
/*  Data buffering is done through a circular buffer management macro         */
/*  package provided by CIRCBUFF.H.                                           */
/*                                                                            */
/*  UARTPORT.C may be used as a model for implementing RTIP compatible        */
/*  drivers for other uarts. Please study the code in UARTPORT.C.             */
/*                                                                            */
/*  The code in UART.C is optimized for packetized sends and receives,        */
/*  such as in PPP and SLIP.  However, there is a "raw mode" which allows     */
/*  an application to stream characters.                                      */
/*  See uart_raw_mode() for more information.                                 */
/*                                                                            */
/*  This driver can be used for an 8250-compatible UART in a PC as            */
/*  well as other uarts.                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"  
#include "rtip.h"  


#include "circbuff.h"
#include "uartport.h"
#include "uart.h"

#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_CIRC_BUF      0
#define DEBUG_UART_RECV_INT 0
#define DEBUG_UART_RECV     0

/* ********************************************************************   */
#define UART_GUARD_XMIT   ks_disable();
#define UART_RELEASE_XMIT ks_enable();

#if (EBSSMCUART)
RTIP_BOOLEAN smc_uart_open(PUART_INFO uinfo);
void    smc_uart_close(PIFACE pi);                                
#endif

/* ********************************************************************   */
/* See uartport.h for description of UART_INFO                            */
/* This structure stores info used by the uart porting layer as well as   */
/* by uart.c itself.  This structure is filled by uart_init() and         */
/* and uart_hw_init()                                                     */
UART_INFO KS_FAR uart_info_arry[CFG_NUM_RS232];

/* ********************************************************************   */
/* Import functions                                                       */
void rs232_error(int channel, int error); /* rs232.c */

/* ********************************************************************   */
/* Function Prototypes                                                    */
PUART_INFO get_uinfo_struct(int minor_number);

/* RCV_INTERVAL is a configuration constant used in uart_receive().   */
/* See uart_receive() for more information.                           */
#define RCV_INTERVAL (5 * ks_ticks_p_sec())

/* For new uart porting method, these functions are provided in a board support
   file (ie- cf5272c3.c for coldfire, cs89712.c for arm7(CDK238)  
    uart_init
    uart_send
    uart_receive
    uart_raw_mode
    uart_close
    */
/* ********************************************************************       */
/* uart_init() - Initialize the uart driver.                                  */
/*                                                                            */
/* This function is called when the application calls xn_attach().            */
/* This function calls uart_hw_init() for hardware-specific initialization.   */
/* See UARTPORT.C for porting information.                                    */
/*                                                                            */

RTIP_BOOLEAN uart_init(
    int         minor_number,       /* minor number (index into uart_info_arry) */
    PIFACE      rs232_pi,           /* Pointer to the device driver. */
    char        handshake_type,     /* (N)one (D)tr/Dsr (R)ts/Cts */
    int         baud_rate,          /* Baud rate - see above */
    int         comm_port,          /* Comm Port (1-4) */
    byte        framing_char,       /* PPP Framing Character */

    /* specify circular input buffer (ISR puts chars in circular buffer and   */
    /* interrupt task takes out and processes                                 */
    PFBYTE      input_buffer,       /* Pre-allocated circular buffer */
                                    /* (optional)                      */
                                    /* MAX_PACKETSIZE if not specified */
    int         input_buffer_len,   /* Pre-allocated buffer size   */
                                    /* (only used if input_buffer is not 0)   */

    /* specify circular output buffer (chars from escaps buffer are put   */
    /* circular output buffer.                                            */
    /* characters from the circular output buffer are transmitted to      */
    /* the UART hardware                                                  */
    PFBYTE      output_buffer,      /* Pre-allocated buffer       */
                                    /* (optional)                      */
                                    /* MAX_PACKETSIZE if not specified */
    int         output_buffer_len   /* Pre-allocated buffer size */
                                    /* (only used if output_buffer is not 0)   */
    )
{
PUART_INFO uinfo;
PCIRC_BUFF pq;

    uinfo = get_uinfo_struct(minor_number);
    if (!uinfo)
    {
        DEBUG_ERROR("uart_init: minor number is ", EBS_INT1, minor_number, 0);
        return(FALSE);
    }
    
    tc_memset((PFBYTE)uinfo, 0, sizeof(*uinfo));    /* Start clean  */
    
    uinfo->minor_number     = minor_number;
    uinfo->rs232_pi         = rs232_pi;

#if (UART_SUPPORTS_HANDSHAKING)
    uinfo->handshake_type   = handshake_type;
    if (uinfo->handshake_type == 'N')
    {
        uinfo->handshaking_enabled = FALSE;
    }
    else
    {
        uinfo->handshaking_enabled = TRUE;
    }
#else
    ARGSUSED_INT(handshake_type);
#endif

    uinfo->comm_port        = comm_port;
    uinfo->baud_rate        = baud_rate;    
    uinfo->framing_char     = framing_char; 
    /* Now set up the input and output buffers             */
    /* Either use the static method or the dynamic method  */
    if (input_buffer)
    {
        pq = &uinfo->input_queue;
        cq_init(input_buffer, input_buffer_len, pq);
    }
    else
    {
        uinfo->input_dcu = os_alloc_packet(MAX_PACKETSIZE, RS232_BUF_ALLOC);
        if (!uinfo->input_dcu)
        {
            DEBUG_ERROR("uart_init - No buffers = ", NOVAR, 0, 0);
            return(FALSE);
        }
        pq = &uinfo->input_queue;
        cq_init(DCUTODATA(uinfo->input_dcu), MAX_PACKETSIZE, pq);
    }

    if (output_buffer)
    {
        pq = &uinfo->output_queue;
        cq_init(output_buffer, output_buffer_len, pq);
    }
    else
    {
        uinfo->output_dcu = os_alloc_packet(MAX_PACKETSIZE, RS232_BUF_ALLOC);
        if (!uinfo->output_dcu)
        {
            DEBUG_ERROR("uart_init - No buffers = ", NOVAR, 0, 0);
            if (uinfo->input_dcu)
            {
                os_free_packet(uinfo->input_dcu);
            }
            return(FALSE);
        }
        pq = &uinfo->output_queue;
        cq_init(DCUTODATA(uinfo->output_dcu), MAX_PACKETSIZE, pq);
    }
/*  ks_disable();   */
#if (EBSSMCUART)
    if (!smc_uart_open(uinfo))
    {
/*      ks_enable();   */
        return(FALSE);
    }
#else
    if (!uart_hw_init(uinfo))
    {
/*      ks_enable();   */
        return(FALSE);
    }
#endif
/*  ks_enable();   */

    /* Wake up the interrupt (receiver) task which is blocking in tasks.c 
       waiting for a chance to run */
    /* This is done by ks_hook_interrupt if TASK_ISRS are on   */
#if (!INCLUDE_TASK_ISRS)
    rs232_signal_event(minor_number, 1);
#endif

    return(TRUE);
}

/* *************************************************************************   */
/* uart_close() - Stop the uart driver.  No more interrupts or signals.        */
/*                                                                             */
/* This function is called when xn_interface_close() is called.                */
/* See UARTPORT.C for porting information.                                     */
/*                                                                             */
void uart_close(int minor_number)
{
PUART_INFO uinfo;
    
    uinfo = get_uinfo_struct(minor_number);

    if (uinfo)
    {

#if (EBSSMCUART)
        smc_uart_close(uinfo->rs232_pi);
#else
        uart_hw_disable(uinfo);
#endif

        if (uinfo->output_dcu)
        { 
            os_free_packet(uinfo->output_dcu);
        }
        if (uinfo->input_dcu) 
        {   
            os_free_packet(uinfo->input_dcu);
        }

        tc_memset((PFBYTE)uinfo, 0, sizeof(*uinfo));
    }
}

/* ***********************************************************************   */
/* uart_receive() - Receiver task/data pump for input.                       */
/*                                                                           */
/* This function runs as a task in a multitasking environment.               */
/* It checks the input buffer to see if there is any data queued up.  If     */
/* so, it passes on the data to rs232.c via rs232_give_string.  In raw mode, */
/* rs232_give_string copies the data to a raw mode buffer.                   */
/* When multitasking, this function never returns.  It loops forever,        */
/* waking up and passing on data when it gets a signal from the ISR,         */
/* or at intervals specified by RCV_INTERVAL.                                */
/*                                                                           */
/* This routine is called from the interrupt task (see tasks.c)              */
/* after ks_invoke_interrupt is called                                       */


void uart_receive(int minor_number)
{
word timeout = (word)RCV_INTERVAL;
int n, wrap;
int buff_count, org_buff_count;
PCIRC_BUFF pq;
PUART_INFO uinfo;

#if (DEBUG_UART_RECV)
    DEBUG_ERROR("uart_receive: minor_number ", EBS_INT1, minor_number, 0);
#endif

    uinfo = get_uinfo_struct(minor_number);
    if (!uinfo)
    {
        DEBUG_ERROR("uart_receive: minor number is ", EBS_INT1, minor_number, 0);
        return;
    }
    pq = &uinfo->input_queue;

    for (;;) 
    {
        /* How many bytes are left in the ring buffer   */
        ks_disable();
        org_buff_count = buff_count = cq_count(pq);
        ks_enable();

#if (DEBUG_UART_RECV)
        DEBUG_ERROR("uart_receive: num bytes in ring buffer", EBS_INT2,
            buff_count, pq->buff_count);
#endif

        if (buff_count)
        {
            while (buff_count)
            {
                /* Get the number of bytes to the end of the buffer.
                    Transfer either that many or the number left to 
                    send. Whichever is smaller */
                wrap = 0;
                n = (int)cq_outbufftoend(pq);
                if (n <= buff_count)
                    wrap = 1;
                else
                    n = buff_count;
#if (DEBUG_UART_RECV)
                DEBUG_ERROR("uart_receive: call rs232_give_string with # bytes ", 
                    EBS_INT1, n, 0);
#endif
                rs232_give_string(minor_number, cq_outpointer(pq), n);

                /* Move the output pointer. wrap to the beginning if
                    needed.
                */
                cq_moveoutptr(pq,wrap,n);
                buff_count -= n;
            }
            ks_disable();
            cq_reducecount(pq,org_buff_count);
            ks_enable();
        }

#if (UART_SUPPORTS_HANDSHAKING)
        if (uinfo->we_held_off && !(cq_highwater(pq)))
        {
            uinfo->we_held_off =  FALSE;
#if (DEBUG_HANDSHAKING)
            DEBUG_ERROR("uart_receive: resume sender", NOVAR, 0, 0);
#endif
            uart_hw_resume_sender(uinfo);
        }
#endif

        /* Wait for characters   */
        if (OS_TEST_INTERRUPT_SIGNAL(uinfo->rs232_pi, timeout))
        {
            ;       /* keep compiler happy by doing a test */
        }

        /* check if interface was closed   */
        if (!uinfo->rs232_pi)
            return;
    }       /* end of for loop */
}


/* *************************************************************************   */
/* uart_send() - UART Transmit Routine                                         */
/*                                                                             */
/* This routine sends data over the serial port by queueing it up on the       */
/* circular buffer and enabling transmit interrupts.                           */
/* It never blocks.  It returns the number of bytes that it was actually       */
/* able to queue up.  Returns -1 on error.                                     */
/*                                                                             */


int uart_send(int minor_number, PFBYTE pb, int tot_to_send)
{
PUART_INFO uinfo;
PCIRC_BUFF pq;
int room,n_to_queue,toend;
int n_to_send;
int n_sent;
    uinfo = get_uinfo_struct(minor_number);
    if (!uinfo)
    {
        DEBUG_ERROR("uart send: minor number incorrect", EBS_INT1, 
            minor_number, 0);
        return(-1);
    }
    
#if (EBSSMCUART)
    return(smc_uart_send(pb, tot_to_send));
#else

    pq = &uinfo->output_queue;

    UART_GUARD_XMIT
    room = cq_room(pq);
    UART_RELEASE_XMIT

    n_to_send = room;

    if (tot_to_send < n_to_send)
    {
        n_to_send = tot_to_send;
    }

    n_sent = n_to_send;

    while (n_to_send > 0)
    {
        toend = cq_inbufftoend(pq);

        /* figure out number of bytes in circular buffer   */
        n_to_queue = room<toend ? room:toend;

        /* if less room in circular buffer than in the escape buffer   */
        /* than only queue what there is room for                      */
        if (n_to_send < n_to_queue)
        {
            n_to_queue = n_to_send;
        }

        /* move chars from escape buffer to circular buffer   */
        tc_movebytes(cq_inpointer(pq), pb, n_to_queue);
        pb += n_to_queue;
        cq_moveinptr(pq,(n_to_queue==toend),n_to_queue);
        room -= n_to_queue;
        n_to_send -= n_to_queue;
    }   

    UART_GUARD_XMIT
    cq_incrementcount(pq, n_sent);
    if (n_sent < tot_to_send)
    {
#if (DEBUG_CIRC_BUF)
        DEBUG_ERROR("CIRC BUF FULL: have IRQ wake me up when room", NOVAR, 0, 0);
#endif
        /* CIRCULAR BUFFER IS FULL!!!!!                           */
        /* wake me up (via ks_invoke_output) when there is enough */
        /* room to send the rest of the data                      */
        cq_setouttriggerval(pq,tot_to_send-n_sent);
    }

#if (DEBUG_CIRC_BUF)
    DEBUG_ERROR("uart_send: start xmit: minor number", EBS_INT1,
        minor_number, 0);
#endif

    /* start the xmit by enabling transmit interrupt                */
    /* NOTE: interrupt will occur as soon as interrupts are enabled */
    /*       since xmitter holding register is empty                */
    uart_hw_enable_tx_int(uinfo);

    uinfo->xmit_on = TRUE;

    UART_RELEASE_XMIT

    return n_sent;
#endif /* (EBSSMCUART) */

}

/* ***********************************************************************   */
void uart_raw_mode(int minor_number, RTIP_BOOLEAN raw_on)
{
PUART_INFO uinfo;

    uinfo = get_uinfo_struct(minor_number);

    DEBUG_ASSERT(uinfo,"uart_raw_mode: invalid minor number: ",EBS_INT1,minor_number,0);

    if (uinfo)
    {
        /* Disable interrupts because uinfo->raw_mode is used by the ISR   */
        /* we don't want munging to happen.                                */
        ks_disable();
        uinfo->raw_mode = raw_on;
        ks_enable();
    }
}



/* ***********************************************************************   */
/* uart_interrupt() - UART interrupt service routine                         */
/* ***********************************************************************   */
/*                                                                           */
/* Note: this routine heavily calls the UARTPORT module.  If it is not       */
/* generic enough, it may be necessary to move more of this into the         */
/* porting layer.                                                            */
/*                                                                           */
/* uart_interrupt()                                                          */
/*  while interrupt needs servicing:                                         */
/*   switch(interrupt type)                                                  */
/*    Error Interrupt:                                                       */
/*     Get the error code from the UARTPORT module, pass it on to RS232      */
/*                                                                           */
/*    Input Interrupt:                                                       */
/*     while there is data waiting                                           */
/*      read it from the hardware into the input queue                       */
/*      if we got a framing character or are in raw mode                     */
/*       signal that we got input                                            */
/*                                                                           */
/*    Output Interrupt:                                                      */
/*     while we can write to hardware/handshaking lets us write              */
/*      if we have data                                                      */
/*       write a character                                                   */
/*      else                                                                 */
/*       disable transmits                                                   */
/*      if output triggered                                                  */
/*       signal output done                                                  */
/*                                                                           */
/*    Status Interrupt:                                                      */
/*     if handshaking change                                                 */
/*      turn on transmitting                                                 */
/*                                                                           */
/* HARDWARE HANDSHAKING:                                                     */
/*       Here is a brief explanation of how hardware handshaking works.      */
/*       There are two forms of hardware handshaking for rs232, and          */
/*       they are know as RTS/CTS and DTR/DSR.                               */
/*                                                                           */
/*       CTS and DSR are the bits you check before sending data to the       */
/*       other end of the connection. Here are the definitions:              */
/*       Inputs     State       Meaning                                      */
/*       CTS          0          Do not send any more data to the other end. */
/*       CTS          1          It is OK to send data to the other end.     */
/*       DSR          0          Do not send any more data to the other end. */
/*       DSR          1          It is OK to send data to the other end.     */
/*                                                                           */
/*       RTS and DTR are the bits that you use on this end to tell the       */
/*       other end whether or not to send us data. Here are the definitions: */
/*       RTS          0          We cannot receive data now, we are full!    */
/*       RTS          1          We can receive data now.                    */
/*       DTR          0          We cannot receive data now, we are full!    */
/*       DTR          1          We can receive data now.                    */
/*                                                                           */
#if (!EBSSMCUART)   /* MPC860 with SMC uart device has its own isr. See initppc.c, eth860.c */
#if (!defined(UART_VECTORED_INTS))
void uart_interrupt(int n)
#else
void uart_interrupt(int n, int intr_type)
#endif
{
byte chr;
int  fifo_size;
PUART_INFO uinfo;
PCIRC_BUFF pq;
#if (UART_SUPPORTS_STATUS_INT)
UART_STATUS uart_status;
#endif

    uinfo = get_uinfo_struct(n);
    if (!uinfo)
    {
        DEBUG_ERROR("uart_interrupt: INVALID INTERRUPT", NOVAR, 0, 0);
        return;             /* Kablami time */
    }


#if (UART_SUPPORTS_INT_POLLING && !defined(UART_VECTORED_INTS))
    while (uart_hw_needs_servicing(uinfo))
#endif
    {
#if (!defined(UART_VECTORED_INTS))
        switch (uart_hw_get_interrupt_type(uinfo))
#else
        switch (intr_type)
#endif
        {
        case UART_ERROR_INTERRUPT:  /* Problems */
            rs232_error(n,uart_hw_check_error(uinfo));
#if (UART_16550)
            /* For a 16550 uart if the high bit of LSR is set it means that there
               is a byte with an error in the fifo. We must fall through 
               and remove it from the fifo */
            if ((byte)(INBYTE(uinfo->port_base + LSR) & 0x80))
                ;
            else
#endif
                break;

        case UART_INPUT_INTERRUPT:      /* INPUT - receiver data available */
            /* To take advantage of the UARTs with FIFOs, we need to        */
            /* keep getting data while data is available.  This makes       */
            /* the transfer much more efficient because there will only     */
            /* be 1 INT per 8 characters received.  This also means that    */
            /* if we get characters too fast, we stay in ISR forever... but */
            /* not very likely with RS232 I/O.                              */
            pq = &uinfo->input_queue;
#if (!defined(ONE_BYTE_FIFO))   
            while (uart_hw_poll_rx(uinfo))
#endif
            {
                chr=uart_hw_rx(uinfo);

                /* if buffer is not full, queue it   */
                if (!cq_full(pq))
                {
#if (DEBUG_UART_RECV_INT)
{
static long cq_cntr = 0;
    cq_cntr++;
    DEBUG_ERROR("enque: ctr, chr", EBS_INT2, cq_cntr, chr);
    DEBUG_ERROR("uart_interrupt: minor_number ", EBS_INT1, n, 0);
    DEBUG_ERROR("enque: num bytes in ring buffer before enque, minor_number", 
        EBS_INT2, pq->buff_count, n);
}
#endif
                    /* Signal if in raw mode or if it is a framing   */
                    /* character                                     */
                    cq_enque(pq,chr);       /* Que the character */
                    if (uinfo->raw_mode || (chr == uinfo->framing_char))
                    {
#if (DEBUG_UART_RECV_INT)
                        DEBUG_ERROR("raw mode or framing char", NOVAR, 0, 0);
#endif
                        /* wakeup uart_receive    */
                        rs232_signal_event(uinfo->minor_number, 1);
                    }
                }
                else
                {
                    /* Tell rs232 package that we lost a character   */
                    rs232_error(n, 3);
                }
#if (UART_SUPPORTS_HANDSHAKING)
                /* Hold off the sender if we need to   */
                if (cq_highwater(pq) && !uinfo->we_held_off)
                {
#if (DEBUG_HANDSHAKING)
                    DEBUG_ERROR("uart_interrupt: hold off sender", NOVAR, 0, 0);
#endif
                    uart_hw_hold_off_sender(uinfo);
                    uinfo->we_held_off = TRUE;

                    /* If we are running out of space in the input queue
                       signal the receive process to be sure we unload */
                    /* i.e. wakeup uart_receive    */
                    rs232_signal_event(uinfo->minor_number, 1);
                }
#endif
            };
            break;

        case UART_OUTPUT_INTERRUPT:     /* OUTPUT - transmitter holding  */
                                        /* register empty; output next     */
                                        /*          character              */
            /* Here is where we need to check                              */
            /* the CTS (or DSR) handshaking line and see if the remote     */
            /* end is able to take another character.  If the CTS (or DSR) */
            /* bit is 0, it means DO NOT SEND.                             */
            /* If the remote end cannot take a character,                  */
            /* then do not send the character. Note that we                */
            /* will have to turn OFF TX INTS in this case, and then        */
            /* make sure that MODEM STATUS INTS are turned on so that      */
            /* when the modem status (CTS or DSR) changes, we can check    */
            /* to see if it is OK to transmit.                             */
            /* Handshake type (N)one (D)tr/Dsr (R)ts/Cts                   */
           pq = &uinfo->output_queue;

#if (UART_SUPPORTS_HANDSHAKING)
            if (!uinfo->handshaking_enabled  || 
                uart_hw_poll_tx_handshake(uinfo))
#endif
            {
                /* If we got here, then the remote end can accept data.   */
                /* This while loop will make the 16550 system more        */
                /* efficient, since it will fill the transmit fifo        */
                /* until no more data can be crammed into it.             */
#if (DEBUG_CIRC_BUF)
                DEBUG_ERROR("sending from buffer of bytes=", EBS_INT1,
                    cq_count(pq), 0);
#endif
                fifo_size = uinfo->xmit_fifo_size;
                while (fifo_size--)
                {
                    if (cq_empty(pq))
                    {
                        /* No characters available, so turn OFF TX ints...   */
                        uinfo->xmit_on = FALSE;
                        uart_hw_disable_tx_int(uinfo);
                        break;
                    }
                    /* There is a character available.. send it.   */
                    chr = cq_dequeue(pq);
                    uart_hw_tx(uinfo,chr);
                }
            }
#if (UART_SUPPORTS_HANDSHAKING)
            else 
            {
#if (DEBUG_HANDSHAKING)
                DEBUG_ERROR("uart_interrupt: Can't send cuz handshaking",NOVAR,0,0);
#endif

                /* Since it is not ok to send data right now, we must disable    */
                /* TX INTS;                                                      */
                /* It will be re-enabled when the MODEM_STATUS INT happens       */
                /* and the CTS bit is 1.                                         */
                uinfo->xmit_on = FALSE;
                uart_hw_disable_tx_int(uinfo);
            }
#endif

            /* if circular buffer is empty enough to trigger IP task   */
            /* to continue processing (i.e. to try to queue more data  */
            /* in the circular buffer)                                 */
            if (cq_checkouttrigger(pq))
            {
#if (DEBUG_CIRC_BUF)
                DEBUG_ERROR("CIRC BUF HAS ROOM: wakeup uart_send", NOVAR, 0, 0);
#endif
                cq_clearouttrigger(pq);
                rs232_signal_event(uinfo->minor_number, 2);
            }
            break;

#if (UART_SUPPORTS_STATUS_INT)
        case UART_STATUS_INTERRUPT:  /* modem status -- ignore if no handshaking */
           uart_get_status(uinfo, &uart_status);

           /* See if the CTS (or DSR) line is now 1. If it is, see if any   */
           /* data needs to be sent. If it does, send a byte of data and    */
           /* then re-enable the TX interrupts.                             */
#if (UART_SUPPORTS_HANDSHAKING)
           if (uart_status.tx_handshake)
#endif
           {    
                pq = &uinfo->output_queue;
                if ( uart_hw_poll_tx(uinfo) ) 
                {
                    if (!cq_empty(pq))
                    {
                        DEBUG_ASSERT(uinfo->xmit_on, 
                            "uart_interrupt: handshaking: start send again", NOVAR, 0, 0);

                        chr = cq_dequeue(pq);

                        /* There is a character available.. send it.   */
                        uart_hw_tx(uinfo, chr);

                        /* And re-enable TX INTS so that when this character   */
                        /* is sent we will get another one TX INT.             */
                        uart_hw_enable_tx_int(uinfo);
                        uinfo->xmit_on = TRUE;
                    }
                }
                if (cq_checkouttrigger(pq))
                {
                    cq_clearouttrigger(pq);
                    rs232_signal_event(uinfo->minor_number, 2);
                }
           }
           if (uart_status.modem_drop)
           {
                rs232_signal_event(uinfo->minor_number, 3);
           }
           uart_hw_clear_interrupt(uinfo);
           break;
#endif /* UART_SUPPORTS_STATUS_INT */
      }
  }
}
#endif /* (!EBSSMCUART) */

/* ***********************************************************************   */
PUART_INFO get_uinfo_struct(int minor_number)
{
    if (minor_number >= CFG_NUM_RS232)
    {
        DEBUG_ERROR("uart: minor number too big (Increase CFG_NUM_RS232?) minor =",
            EBS_INT1, minor_number, 0);
        return 0;
    }
    return ((PUART_INFO) &uart_info_arry[minor_number]);
}

/* ***********************************************************************   */
/* setup to trigger call to ks_invoke_output when circular buffer            */
/* gets down to trigger characters in it                                     */
void uart_setouttrigger(PRS232_IF_INFO pif_info, int trigger)
{
PUART_INFO uinfo;
PCIRC_BUFF pq;

    uinfo = get_uinfo_struct(pif_info->index);
    if (!uinfo)
    {
        DEBUG_ERROR("uart_setouttrigger: uinfo failed: ", EBS_INT1,
            pif_info->index, 0);
        return;
    }
    pq = &uinfo->output_queue;
    cq_setouttriggerval(pq, trigger);
}

#endif  /* !SH2 && !SH3 */
