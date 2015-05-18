/*                                                                      */
/* NETUTIL.C - API functions                                            */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*                                                                      */
/*  Module description:                                                 */
/*  This module provides utilities for sending and receiving data       */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IPLINE


#include "sock.h"
#include "netutil.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif
#if (INCLUDE_POP3)
#include "pop.h"
#endif

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_LINE_IN       0
#define DEBUG_LINE_IN_FULL  0
#define DEBUG_LINE_OUT      0

/* ********************************************************************   */
/* LOCAL FUNCTION DECLARATIONS                                            */
/* ********************************************************************   */
int     line_get_buf(PIO_CONTEXT io_context, PFCHAR *buffer, long wait);
RTIP_BOOLEAN line_setup_pb_out(PIO_CONTEXT io_context);
#if (INCLUDE_POP3)
PFCHAR check_for_crlfs(PIO_CONTEXT io_context);
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#if (INCLUDE_RTIP)
#define RESET_BUFFER(buffer, dcu)                               \
    io_context->buffer = (PFCHAR)DCUTODATA(io_context->dcu);
#else
#define RESET_BUFFER(buffer, dcu)                               \
    io_context->buffer = io_context->dcu;
#endif

/*****************************************************************   */
/* LINE NETWORK I/O routines (XN_PUT_LINE, XN_GET_LINE etc)          */
/*****************************************************************   */

/*****************************************************************              */
/* Summary:                                                                     */
/*   #include "rtip.h"                                                          */
/*                                                                              */
/*   int xn_line_put(PIO_CONTEXT io_context, long wait, int type)               */
/*                                                                              */
/* Description:                                                                 */
/*   Performs the following:                                                    */
/*   - Sends the data in the buffer over the connection.                        */
/*   - Two types of sends are done, if type=PUT_QUE, the data is                */
/*     queued and only sent when queue buffer will overflow; if                 */
/*     type=PUT_SEND, the data is sent immediately                              */
/*   - If wait parameter is non-zero, calls select to wait for                  */
/*     room in output window to queue data.  wait specified the                 */
/*     number of seconds to wait.                                               */
/*                                                                              */
/*   Usage:                                                                     */
/*   - Initially pb is set up with a buffer (PUT_ALLOC)                         */
/*   - To queue (PUT_QUE), call with buffer and length_out setup.  Will copy to */
/*     pb and send if necessary                                                 */
/*     NOTE: length_out must be <= MAX_PACKETSIZE                               */
/*   - To send (PUT_SEND), put data directly in pb and offset set to            */
/*     length                                                                   */
/*   - To send queued data (PUT_QUE) with length_out set to 0                   */
/*                                                                              */
/*  NOTE: if socket is in non-blocking mode, you should pass a timeout          */
/*        value in wait inorder to force the select to be called                */
/*                                                                              */
/*  Returns >=0 upon success; -1 upon failure                                   */
/*  Sets errno upon error                                                       */
/*                                                                              */

int xn_line_put(PIO_CONTEXT io_context, long wait, int type)
{
int           ret_val;
#if (INCLUDE_RTIP || INCLUDE_BSDSOCK || SELECT_TCP_EMPTY)
unsigned long nleft_window;
#endif
#if (!SELECT_TCP_EMPTY && !USE_PKT_API)
int           nleft;
int           nsent;
#endif

/*    DEBUG_ERROR(io_context->buffer_out, NOVAR, 0, 0);   */

    ret_val = 0;

/*if (type & PUT_QUE)                                                                 */
/*{                                                                                   */
/*  DEBUG_ERROR("xn_line_put: PUT_QUE, len = ", EBS_INT1, io_context->length_out, 0); */
/*}                                                                                   */

/*  if ( (type & PUT_QUE) && (io_context->length_out > MAX_PACKETSIZE))              */
/*  {                                                                                */
/*      DEBUG_ERROR("WARNING: length_out is too big! You probably overwrote buffer", */
/*          NOVAR, 0, 0);                                                            */
/*  }                                                                                */

    /* **************************************************   */
    if (io_context->msg_pb_out)
    {
        /* if PUT_QUE and not enough room to que all the data OR   */
        /*    PUT_QUE and no data to queue OR                      */
        /*    PUT_SEND and not(queing data)                        */
        if ( ((type & PUT_QUE) && (io_context->n_left_out < io_context->length_out)) || 
             ((type & PUT_QUE) && !(io_context->length_out)) ||
             ((type & PUT_SEND) && 
              !((type & PUT_QUE) && io_context->length_out)) )
        {

            /* going to send; check if need to wait first   */
            if (wait)
            {
#if (SELECT_TCP_EMPTY)
                nleft_window = 0;
                if (ioctlsocket(io_context->sock, FIONWRITE, 
                                (unsigned long KS_FAR *)&nleft_window) != 0)  
                {
                    DEBUG_ERROR("xn_line_put: ioctlsocket (FIONWRITE) Failed", NOVAR, 0, 0);
                }

                /* if more data queued in pb then will fit in the window, wait   */
                if ((unsigned long)(io_context->offset_out) > nleft_window)
                {
                    /* if timesout, return failure                                    */
                    /* NOTE: for push_data could check state - tbd incase remote side */
                    /*       closes the connection                                    */
                    if (!do_write_select(io_context->sock, wait))
                    {
                        DEBUG_ERROR("xn_line_put - select timed out", NOVAR, 0, 0);
                        DEBUG_ERROR(io_context->buffer_out, NOVAR, 0, 0);
                        return(-1);
                    }
                }
#elif (USE_PKT_API)
                /* if timesout, return failure                                    */
                /* NOTE: for push_data could check state - tbd incase remote side */
                /*       closes the connection                                    */
                if (!do_write_select(io_context->sock, wait))
                {
                    DEBUG_ERROR("xn_line_put - select timed out", NOVAR, 0, 0);
                    DEBUG_ERROR(io_context->buffer_out, NOVAR, 0, 0);
                    return(-1);
                }
#endif
            }

#if (USE_PKT_API)
            /* wait calculation could overflow if number of seconds is   */
            /* to large but it is ok unless the configuration parameter  */
            /* is changed to a large number                              */
            ret_val = xn_pkt_send(io_context->sock, 
                                  io_context->msg_pb_out, 
                                  io_context->offset_out, RTIP_INF);

            /* if successful, the DCU will be freed when sent   */
            if (ret_val >= 0)
            {
                io_context->msg_pb_out = (DCU)0;
                xn_line_put(io_context, wait, PUT_ALLOC);
            }
            else
            {
                /* re-intialize pb out info   */
                if (!line_setup_pb_out(io_context))
                {
                    DEBUG_ERROR("xn_line_put: line_setup_pb_out failed",
                        NOVAR, 0, 0);
                    return(-1);
                }
            }
#else       /* USE_PKT_API */
#if (SELECT_TCP_EMPTY)
/*          ret_val = send(io_context->sock, io_context->pb_out,   */
/*                         io_context->offset_out, 0);             */
            ret_val = net_send(io_context->sock, io_context->pb_out, io_context->offset_out);
#else
            nleft = io_context->offset_out;
            nsent = 0;
            while (nleft > 0)
            {
                if (wait)
                {
#if (INCLUDE_RTIP || INCLUDE_BSDSOCK)
                    nleft_window = 0;
                    if (ioctlsocket(io_context->sock, FIONWRITE, 
                                    (unsigned long KS_FAR *)&nleft_window) != 0)  
                    {
                        DEBUG_ERROR("xn_line_put: ioctlsocket (FIONWRITE) Failed", NOVAR, 0, 0);
                    }

                    /* if no more room to queue data in output window   */
                    if (nleft_window == 0)
#endif
                    {
                        /* wait until some room opens up in output window   */
                        /* if timesout, return failure                      */
                        if (!do_write_select(io_context->sock, wait))
                        {
                            DEBUG_ERROR("xn_line_put - select timed out", EBS_INT1,
                                wait, 0);
                            return(-1);
                        }
                    }
                }
/*              ret_val = send(io_context->sock, io_context->pb_out+nsent,   */
/*                             nleft, 0);                                    */
                ret_val = net_send(io_context->sock, io_context->pb_out+nsent, nleft);
                if (ret_val < 0)
                    break;
                nleft -= ret_val;
                nsent += ret_val;
            }
#endif      /* SELECT_TCP_EMPTY */

            /* re-intialize pb out info   */
            if (!line_setup_pb_out(io_context))
            {
                DEBUG_ERROR("xn_line_put: line_setup_pb_out failed",
                    NOVAR, 0, 0);
                return(-1);
            }
#endif      /* USE_PKT_API */

            if (ret_val < 0)
            {
                DEBUG_ERROR("xn_line_put - send returned error, errno = ", 
                    EBS_INT1, GETLASTERROR(), 0);
            }
        }
    }

    /* **************************************************   */
    if ( !io_context->msg_pb_out && (io_context->length_out || 
                                     type & PUT_ALLOC) )
    {
        MALLOC_BUFFER(io_context->msg_pb_out, io_context->pb_out, 
                      MAX_PACKETSIZE, LINE_PUT_ALLOC)
        if (!io_context->pb_out)
        {
            return(set_errno(ENOPKTS));
        }

#if (!INCLUDE_RTIP)
        io_context->msg_pb_out = io_context->pb_out;
#endif

        /* intialize pb out info   */
        if (!line_setup_pb_out(io_context))
        {
            DEBUG_ERROR("xn_line_put: line_setup_pb_out failed",
                NOVAR, 0, 0);
            return(-1);
        }
    }

    /* **************************************************   */
    /* queue data in pb_out                                 */
    if ( io_context->length_out && (type & PUT_QUE) )
    {
#if (DEBUG_LINE_OUT)
        DEBUG_ERROR("PUT_QUE", STR1, io_context->buffer_out, 0);
#endif
        tc_movebytes(io_context->pb_out+io_context->offset_out, 
                     io_context->buffer_out, io_context->length_out);

        io_context->offset_out += io_context->length_out;
        io_context->n_left_out -= io_context->length_out;

        if (type & PUT_SEND)
        {
            /* NOTE: recursive call but will only recurse one level   */
            /*       since PUT_SEND and length is 0                   */
            io_context->length_out = 0;
            ret_val = xn_line_put(io_context, wait, PUT_SEND);
        }
    }
    return(ret_val);
}

/*****************************************************************   */
RTIP_BOOLEAN line_setup_pb_out(PIO_CONTEXT io_context)
{
#if (USE_PKT_API)
PTCPPORT port;
#endif

#if (USE_PKT_API)
    io_context->pb_out = (PFCHAR)xn_pkt_data_pointer(io_context->sock, 
                                                     io_context->msg_pb_out, 
                                                     OUTPUT_PKT);
    if (!io_context->pb_out)
        return(FALSE);
    port = (PTCPPORT)sock_to_port(io_context->sock);
    io_context->n_left_out = io_context->total_out =
        xn_pkt_data_max(io_context->sock, port->out_ip_temp.ip_dest);
#else
    RESET_BUFFER(pb_out, msg_pb_out)
    io_context->n_left_out = io_context->total_out = MAX_PACKETSIZE;
#endif
    io_context->offset_out = 0;
    return(TRUE);
}

/*****************************************************************                */
/* xn_line_get() - Read a line from a socket                                      */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "rtip.h"                                                            */
/*                                                                                */
/*   int xn_line_get(PIO_CONTEXT io_context, PFCHAR *buffer, long wait, int type) */
/*                                                                                */
/* Description:                                                                   */
/*   Performs the following:                                                      */
/*     Reads data until LF or EOF (connection closed).                            */
/*     Sets buffer to the start of the data in io_context->pb_in.                 */
/*                                                                                */
/*   NOTE:  It is recommended that xn_line_get always be called with              */
/*          tmeout = TRUE.                                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   -1 on error, -2 if there was no data in the alloted time, -3 means           */
/*   the remote host is no longer connected, else returns number of               */
/*   bytes read.                                                                  */
/*   Sets errno upon error                                                        */
/*                                                                                */
/*                                                                                */

int xn_line_get(PIO_CONTEXT io_context, PFCHAR *buffer, long wait, int type)
{
int n, len;
int n_left;
PFCHAR slash_r = 0;
PTCPPORT port;

    if (type == GET_BUF)
    {
        return(line_get_buf(io_context, buffer, wait));
    }

    /* type may be GET_LINE or GET_MAIL   */
    for (;;)
    {
        /* yield; if the remote host is blasting data across and we have   */
        /* a full input window we must yield to let the IP task run        */
        /* since this loop will not block for a period of time             */
        ks_sleep(0);        

        /* strings should end with \r\n                    */
        /* if no \r\n in what we have read, read more data */
        if (type == GET_LINE)
        {
            if (io_context->end_offset_in > io_context->begin_offset_in) 
                slash_r = tc_strstr((PFCHAR)(io_context->pb_in+io_context->begin_offset_in), 
                                    "\r\n");
        }
#if (INCLUDE_POP3)              
        else   /* type must be GET_MAIL  */
           slash_r = check_for_crlfs(io_context);
#endif           
                            
        if (!slash_r)
        {
            /* calculate how much room until end of buffer; leave room   */
            /* for \0                                                    */
            n_left = MAX_PACKETSIZE - io_context->end_offset_in - 1;

            if (n_left)     /* if more room to read */
            {
                /* if remote host is not responding, avoid recv() hanging   */
#if (DEBUG_LINE_IN)
                DEBUG_ERROR("xn_line_get: call do_read_select", NOVAR, 0, 0);
#endif
                if (!do_read_select(io_context->sock, wait))
                {
#if (DEBUG_LINE_IN)
                    DEBUG_ERROR("xn_line_get: select returned failure", NOVAR, 0, 0);
#endif
                    /* if there is still data in the buffer return what   */
                    /* is there                                           */
                    if (io_context->end_offset_in > 
                        io_context->begin_offset_in)
                    {
#if (DEBUG_LINE_IN)
                        DEBUG_ERROR("xn_line_get: return what we have", STR1, 
                            io_context->pb_in+io_context->begin_offset_in, 0);
#endif
                        /* return what we have   */
                        slash_r = (PFCHAR)(io_context->pb_in + 
                                           io_context->end_offset_in);
                        break;
                    }
#if (INCLUDE_RTIP)
                    port = (PTCPPORT)sock_to_port(io_context->sock);
                    if ( port && tc_is_read_state(port) &&
                         ((port->state != TCP_S_CWAIT) ||
                          (port->in.contain > 0)) )
                    {
                        return(-2);
                    }
                    else
#endif
                        return(-3);
                }

#if (DEBUG_LINE_IN)
                DEBUG_ERROR("xn_line_get: do recv", NOVAR, 0, 0);
#endif

                /* use the socket API to get another buffer of data         */
                /* NOTE: there is no option here to use the packet API      */
                /*       since there is not guarentee that a line           */
                /*       will not cross packets and be in two packets;      */
                /*       to handle this case would be messy and would       */
                /*       involve copying anyway and input done by this      */
                /*       routine is not the bulk of the data transmissions  */
                /*       going on anyway                                    */
/*                n = recv(io_context->sock,                                */
/*                         (PFCHAR)(io_context->pb_in +                     */
/*                                  io_context->end_offset_in),             */
/*                         n_left, 0);                                      */
                n = net_recv(io_context->sock,
                             (PFCHAR)(io_context->pb_in +
                                      io_context->end_offset_in),
                             n_left);
                if (n < 0)
                {
                    return(-1);
                }

                io_context->end_offset_in += n;
                io_context->pb_in[io_context->end_offset_in] = '\0';
    
                /* if EOF (no data and otherside closed)   */
                if (n == 0)         
                {
                    if (io_context->end_offset_in > 
                        io_context->begin_offset_in)
                    {
                        /* return what we have   */
                        slash_r = (PFCHAR)(io_context->pb_in + 
                                           io_context->end_offset_in);
                        break;
                    }
                    else
                        return(-3);
                }

                /* strings should end with \r\n    */
                if (type == GET_LINE)
                    slash_r = tc_strstr((PFCHAR)(io_context->pb_in+io_context->begin_offset_in), 
                           "\r\n");
#if (INCLUDE_POP3)              
                else   /* type must be GET_MAIL  */
                    slash_r = check_for_crlfs(io_context);
#endif           
            }
        }

        /* we have filled the buffer from current postion to end,    */
        /* check if we now have a \r                                 */
        if (slash_r)
        {
            *slash_r = '\0';
            break;        
        }
        else
        {
            /* if this is true then the buffer is empty so reset it   */
            if (io_context->end_offset_in <= io_context->begin_offset_in)
            {
                io_context->end_offset_in = io_context->begin_offset_in = 0;
                io_context->pb_in[0] = '\0';
            }
            else if (io_context->begin_offset_in == 0)
                break;      /* never found slashr and no more room */
            else
            {
                /* move unread data to beginning of buffer and go try again   */
                /* NOTE: +1 in length to ensure moving \0 at end of string    */
                tc_movebytes(io_context->pb_in, 
                             io_context->pb_in+io_context->begin_offset_in,
                             io_context->end_offset_in - 
                             io_context->begin_offset_in + 1);
                io_context->end_offset_in -= io_context->begin_offset_in;
                io_context->begin_offset_in = 0;
            }
        }
    }           /* end of while loop */

    *buffer = (PFCHAR)(io_context->pb_in + io_context->begin_offset_in);

    if (!slash_r)
    {
        /* never found slashr so return whole buffer   */
        len = (int)(io_context->end_offset_in-io_context->begin_offset_in);
        /* new beginning for next call is after \r\n   */
        io_context->begin_offset_in = io_context->end_offset_in = 0;
    }
    else
    {
        /* found slashr so return up to the /r/n   */
        len = (int)(slash_r - *buffer);
        /* new beginning for next call is after \r\n   */
        io_context->begin_offset_in += (len + 2);
    }

#if (DEBUG_LINE_IN_FULL)
    DEBUG_ERROR("xn_line_get: buffer: ", STR1, *buffer, 0);
    DEBUG_ERROR("xn_line_get: len:    ", EBS_INT1, len, 0);
#endif
    return(len);
}


/*****************************************************************           */
/* line_get_buf() - Read a buffer of data from a socket                      */
/*                                                                           */
/* Description:                                                              */
/*   Performs the following:                                                 */
/*     Reads data and places it in io_context->pb_in.                        */
/*     Sets buffer to the start of the data in io_context->pb_in.            */
/*                                                                           */
/* Returns:                                                                  */
/*   -1 on error, -2 if there was no data, else return number of bytes read. */
/*   Sets errno upon error                                                   */
/*                                                                           */
/*                                                                           */

int line_get_buf(PIO_CONTEXT io_context, PFCHAR *buffer, long wait)
{
int n, len;
int n_left;

        /* yield; if the remote host is blasting data across and we have   */
        /* a full input window we must yield to let the IP task run        */
        /* since this loop will not block for a period of time             */
        ks_sleep(0);        

    /* if buffer is empty   */
    if (io_context->end_offset_in <= io_context->begin_offset_in)
    {
        /* if remote host is not responding, avoid recv() hanging   */
        if (wait)
        {
            if (!do_read_select(io_context->sock, wait))
            {
                return(-2);
            }
        }

        /* use the socket API to get another buffer of data         */
        /* NOTE: there is no option here to use the packet API      */
        /*       since there is not guarentee that a line           */
        /*       will not cross packets and be in two packets;      */
        /*       to handle this case would be messy and would       */
        /*       involve copying anyway and input done by this      */
        /*       routine is not the bulk of the data transmissions  */
        /*       going on anyway                                    */
        /* calculate how much room until end of buffer; leave room  */
        /* for \0                                                   */
        n_left = MAX_PACKETSIZE - io_context->end_offset_in - 1;
/*        n = recv(io_context->sock,                      */
/*                    (PFCHAR)(io_context->pb_in +        */
/*                            io_context->end_offset_in), */
/*                    n_left, 0);                         */
        n = net_recv(io_context->sock,
                     (PFCHAR)(io_context->pb_in +
                              io_context->end_offset_in),
                     n_left);
        if (n < 0)
        {
            return(-1);
        }

        io_context->end_offset_in += n;
        io_context->pb_in[io_context->end_offset_in] = '\0';

        /* if EOF (no data and otherside closed)   */
        if (n == 0)         
        {
            if (io_context->end_offset_in <=
                io_context->begin_offset_in)
                return(-2);
        }
    }

    *buffer = (PFCHAR)(io_context->pb_in + io_context->begin_offset_in);
    len = (int)(io_context->end_offset_in - io_context->begin_offset_in);
    io_context->begin_offset_in = io_context->end_offset_in = 0;

    return(len);
}

/*****************************************************************         */
/* xn_line_init() - Setup for line I/O routines                            */
/*                                                                         */
/* Summary:                                                                */
/*   #include "rtip.h"                                                     */
/*                                                                         */
/*   int xn_line_init(PIO_CONTEXT io_context)                              */
/*                                                                         */
/* Description:                                                            */
/*   Sets up for calls to xn_line_put and xn_line_get by allocating        */
/*   necessary DCUs for buffering.  The parameter type controls where      */
/*   setup is done for xn_line_put (LINE_OUTPUT_QUE and LINE_OUTPUT_SEND)  */
/*   and xn_line_get (LINE_INPUT).                                         */
/*                                                                         */
/* Returns:                                                                */
/*   TRUE upon success, FALSE on error                                     */
/*   Sets errno upon error                                                 */
/*                                                                         */
RTIP_BOOLEAN xn_line_init(PIO_CONTEXT io_context, int type)
{
#if (INCLUDE_RTIP)
    io_context->msg_in      = (DCU)0;
    io_context->msg_buf_out = (DCU)0;
    io_context->msg_pb_out =  (DCU)0;
#else
    io_context->msg_pb_out =  (PFBYTE)0;
#endif

    io_context->pb_in = 0;
    io_context->buffer_out = 0;
    io_context->pb_out = 0;

    if (type & LINE_INPUT)
    {
        MALLOC_BUFFER(io_context->msg_in, io_context->pb_in, 
                      MAX_PACKETSIZE, LINE_INIT1_ALLOC)
        if (!io_context->pb_in)
        {
            set_errno(ENOPKTS);
            return(FALSE);
        }

        /* set up info for xn_line_get   */
        io_context->begin_offset_in = 0;
        io_context->end_offset_in = 0;
        io_context->pb_in[0] = '\0';
    }

    if (type & LINE_OUTPUT_SEND)
    {
        /* allocate a DCU for the pb buffer   */
        if (xn_line_put(io_context, 0, PUT_ALLOC) < 0)
        {
            xn_line_done(io_context);
            return(FALSE);
        }
    }

    if (type & LINE_OUTPUT_QUE)
    {
        /* now allocate memory to use output buffer   */
        MALLOC_BUFFER(io_context->msg_buf_out, io_context->buffer_out, 
                      MAX_PACKETSIZE, LINE_INIT2_ALLOC)
        if (!io_context->buffer_out)
        {
            xn_line_done(io_context);
            set_errno(ENOPKTS);
            return(FALSE);
        }
    }

    return(TRUE);
}

/*****************************************************************   */
/* xn_line_done() - Frees packets                                    */
/*                                                                   */
/* Summary:                                                          */
/*   #include "rtip.h"                                               */
/*                                                                   */
/*   int xn_line_done(PIO_CONTEXT io_context)                        */
/*                                                                   */
/* Description:                                                      */
/*   Frees packets originally allocated by xn_line_init or           */
/*   xn_line_put                                                     */
/*                                                                   */
void xn_line_done(PIO_CONTEXT io_context)
{
    /* allocated by xn_line_init   */
#if (INCLUDE_RTIP)
    if (io_context->msg_in)
        os_free_packet(io_context->msg_in);
    if (io_context->msg_buf_out)
        os_free_packet(io_context->msg_buf_out);

    /* alloced by xn_line_put   */
    if (io_context->msg_pb_out)
        os_free_packet(io_context->msg_pb_out);

    io_context->msg_in      = (DCU)0;
    io_context->msg_buf_out = (DCU)0;
    io_context->msg_pb_out  = (DCU)0;
#else
    if (io_context->pb_in)
        free(io_context->pb_in);
    if (io_context->buffer_out)
        free(io_context->buffer_out);
    if (io_context->pb_out)
        free(io_context->pb_out);
#endif

}


#if (INCLUDE_POP3)

/*****************************************************************   */
/* returns number of bytes queued on a socket                        */
dword pop_get_data_avail(int sock)
{
unsigned long arg;

    if (ioctlsocket(sock, FIONREAD, (unsigned long KS_FAR *)&arg) != 0)  /* all options off */
    {
        DEBUG_ERROR("ioctlsocket (FIONREAD) Failed", NOVAR, 0, 0);
        return(0);
    }
    return(arg);
}

/*****************************************************************   */
/* check_for_crlfs() - Search a line from an incoming mime header    */
/*                                                                   */
/* Summary:                                                          */
/*                                                                   */
/*   PFCHAR check_for_crlfs(PIO_CONTEXT io_context)                  */
/*                                                                   */
/* Description:                                                      */
/*   Performs the following:                                         */
/*     Reads data until CRLF or EOF (connection closed).             */
/*  Returns a pointer to the CR.                                     */
/*     Also unwraps any lines containing CRLF followed by at least   */
/*     one space or horizontal tab per RFC 822.                      */
/*                                                                   */
/* Returns:                                                          */
/*   0 if no CRLFs are found.                                        */
/*                                                                   */
/*                                                                   */
PFCHAR check_for_crlfs(PIO_CONTEXT io_context)
{
PFCHAR slashrn;
int len;

    for (;;)
    {
        slashrn = tc_strstr((PFCHAR)(io_context->pb_in+io_context->begin_offset_in), 
                            "\r\n");   /* look for CRLF combination */
        if (!slashrn)                            
            return (0);  /* no CRLFs in this buffer */
        
        len = (int) (slashrn - ( (PFCHAR)(io_context->pb_in + io_context->begin_offset_in) ));
        if (io_context->end_offset_in > (io_context->begin_offset_in +len + 2)) /*check if there is any more data in buffer */
        {
            if ( ((*(slashrn + 2)) == ASCII_SPACE) ||  /* must unfold line per RFC 822 */
                 ((*(slashrn + 2)) == ASCII_HTAB))     /* if a space or htab is found */
            {
                *(slashrn) = ASCII_SPACE;      /* strip off CR */
                *(slashrn + 1) = ASCII_SPACE;  /*strip off LF        */
                continue;
            }
            else
                return(slashrn); /* non space or htab after CRLF so accept as \r\n */
        }
        else     /* there is not any data left in this buffer */
        {
            if (!pop_get_data_avail(io_context->sock))  
                return(slashrn);             /* and there isn't any available, return what we have */
            else    
                return(0);  /* more data avail, must get it into the buffer */
        }        
    }
}    

#endif
