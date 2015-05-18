/*                                                                         */
/* NETUTIL.H  - RTIP header file                                           */
/*                                                                         */
/*   EBS - RTIP                                                            */
/*                                                                         */
/*   Copyright Peter Van Oudenaren , 1993                                  */
/*   All rights reserved.                                                  */
/*   This code may not be redistributed in source or linkable object form  */
/*   without the consent of its author.                                    */
/*                                                                         */
/*    Module description:                                                  */
/*        This file contains prototypes constants and structure            */
/*        declarations required to build RTIP libraries and applications.  */
/*        There should be no need to change values in this file            */
/*        configuration values are in xnconf.h                             */
/*                                                                         */
#ifndef __NETUTIL__
#define __NETUTIL__ 1

#include "xnconf.h"
#include "rtipapi.h"

/* used by WEB, FTP, INCLUDE_SMTP, POP3 and TFTP   */

/* parameter type of xn_line_put   */
#define PUT_QUE   1
#define PUT_SEND  2
#define PUT_ALLOC 4

/* parameter type of xn_line_get   */
#define GET_LINE  1     /* return a line of data (where a line is term */
                        /* by CR,LF   */
#define GET_BUF   2     /* return a buffer full of data */
#define GET_MAIL  3

/* parameter type of xn_line_init   */
#define LINE_INPUT       1
#define LINE_OUTPUT_SEND 2
#define LINE_OUTPUT_QUE  4

/*****************************************************************   */
/* IO CONTEXT                                                        */
/*****************************************************************   */
/* io context for reading and sending data on network and            */
/* reading data from a file                                          */
typedef struct io_context
{
    /* the following fields are set up by caller to xn_put_line   */
    int    sock;
    PFCHAR buffer_out;  /* data to be queued/sent */
    int    length_out;  /* amount of data to be queued */

    /* the following fields are used internally by http_put_line   */
#if (INCLUDE_RTIP)
    PFVOID msg_buf_out; /* DCU for buffer_out */
    PFVOID msg_pb_out;  /* DCU for pb_out */
#else
    PFBYTE msg_pb_out;  /* origional value of pb_out */
#endif
    PFCHAR pb_out;
    int    offset_out;  /* length of data queued in pb */
    int    n_left_out;  /* number of bytes left to be queued in pb */
    int    total_out;   /* number of bytes left to be queued in pb */

    /* the following fields are used internally by http_get_line   */
#if (INCLUDE_RTIP)
    PFVOID msg_in;      /* DCU  */
#endif
    PFBYTE pb_in;
    int    begin_offset_in;
    int    end_offset_in;

    /* file info   */
    int    virtual_fd;

#if (INCLUDE_WEB)   
    /* version's major and minor numbers from HTTP/x.x request; used   */
    /* to format output message                                        */
    int    major_number;        
    int    minor_number;        
    PFVOID mime_in_ptrs;
    int     auth_mime_header;   /*RER-2003-07-30: add this mime header in execute_get_command() */
    PFCCHAR auth_realm;         /*RER-2003-07-30: add this realm in execute_get_command() */
#endif

} IO_CONTEXT;
typedef struct io_context KS_FAR *PIO_CONTEXT;

/* ********************************************************************   */
/* MALLOC BUFFER                                                          */
/* ********************************************************************   */

#if (!INCLUDE_RTIP)
/* who parameter to ks_malloc   */
#define NETUTIL_MALLOC  1
#define VF_MALLOC       2
#endif

#if (INCLUDE_RTIP)
#define MALLOC_BUFFER(MSG, BUFFER, SIZE, WHO)                           \
        BUFFER = 0;                                                     \
        MSG = os_alloc_packet((int)SIZE, (int)WHO);                     \
        if (MSG)                                                        \
            BUFFER = (PFVOID)(DCUTODATA((DCU)MSG));
#define FREE_BUFFER(MSG, BUFFER)                                        \
        if (MSG)                                                        \
            os_free_packet((DCU)MSG);
#else
#define MALLOC_BUFFER(MSG, BUFFER, SIZE, WHO)                           \
        BUFFER = ks_malloc(1, SIZE, NETUTIL_MALLOC);
#define FREE_BUFFER(MSG, BUFFER)                                        \
        if (BUFFER)                                                     \
            ks_free(BUFFER, 1, 1);
#endif

/*****************************************************************   */
/* API                                                               */
/*****************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

int         SOCKAPI_ xn_line_get(PIO_CONTEXT io_context, PFCHAR *buffer, long wait, int type);
int         SOCKAPI_ xn_line_get(PIO_CONTEXT io_context, PFCHAR *buffer, long wait, int type);
int         SOCKAPI_ xn_line_put(PIO_CONTEXT io_context, long wait, int type);
RTIP_BOOLEAN     SOCKAPI_ xn_line_init(PIO_CONTEXT io_context, int type);
void        SOCKAPI_ xn_line_done(PIO_CONTEXT io_context);

#ifdef __cplusplus
}
#endif

#endif          /* NETUTIL */

