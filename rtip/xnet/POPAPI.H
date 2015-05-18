/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */

/* POPAPI.H - Post Office Protocol API                                                  */

#ifndef POPAPI_H
#define POPAPI_H

#include "rtipapi.h"

/* *********************************************************************   */
/* ******                CONFIGURATION                            ******   */
/* *********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_pop_data
{
    long pop_timeout;
} CFG_POP_DATA;

extern struct cfg_pop_data cfg_pop_data;

#define CFG_POP_TIMEOUT     cfg_pop_data.pop_timeout
#else
#define CFG_POP_TIMEOUT     _CFG_POP_TIMEOUT
#endif

/* *********************************************************************   */
typedef struct _pop_context
{
    int  pop_sock;

    #define POP_S_START             0x01
    #define POP_S_MAIL_START        0x02
    #define POP_S_ATTACH_START      0x03
    #define POP_S_MAIL_END          0x04
    #define POP_S_ATTACH_END        0x05
    #define POP_S_MAIL              0x06
    #define POP_S_ATTACH            0x07
    #define POP_S_MSG_DONE          0x08
    #define POP_S_MAIL_HDR          0x10
    #define POP_S_MIME_MAIL_HDR     0x11
    byte pop_state;                 /* state when retrieving a mail msg */
    byte next_pop_state;            /* next state for POP_S_MAIL_END and */
                                    /* POP_S_ATTACH_END   */

    #define MULTIPART_FLAG  0x01
    #define ATTACHMENT_FLAG 0x02
    #define BASE64_FLAG     0x04
    byte pop_flags; 

    char pop_save_char;             /* look ahead char for folding */
    char pop_boundary[CFG_POP_MAX_BOUNDARY+1];
    char pop_filename[CFG_MAX_FILE_LEN+1];
   
    PFCHAR gbuffer;
    PFVOID io_context;              /* PIO_CONTEXT */
} pop_context;
typedef struct _pop_context KS_FAR *PPOP_CONTEXT;

typedef struct _pop_msg_size
{
    int           num;
    long          size;
} pop_msg_size;
typedef struct _pop_msg_size KS_FAR *PPOP_MSG_SIZE;

typedef struct _pop_msg_info
{
    char          from[CFG_MAX_FROM_LEN];
    char          subject[CFG_MAX_SUBJECT_LEN];
} pop_msg_info;
typedef struct _pop_msg_info KS_FAR *PPOP_MSG_INFO;

typedef struct _pop_info
{
    int           num_messages;
    dword         mail_size;
} pop_info;
typedef struct _pop_info KS_FAR *PPOP_INFO;

/* mime_type   */
#define MIME_RETURN_PATH    0
#define MIME_RECEIVED       1
#define MIME_DATE           2   
#define MIME_FROM           3
#define MIME_MESSAGE_ID     4
#define MIME_SUBJECT        5
#define MIME_VERSION        6
#define MIME_CONTENT_TYPE   7
#define MIME_STATUS         8
#define MIME_CONTENT_ENCODE 9
#define MIME_CONTENT_DISP  10
#define MAIL               11
#define ATTACH             12
#define MAIL_START         13
#define ATTACH_START       14
#define MAIL_END           15
#define ATTACH_END         16
#define MSG_DONE           17
#define TOTAL_MIME_HEADERS 18

/* parameter to xn_pop_parse; i.e. an array of these is passed to   */
/* xn_pop_parse and xn_pop_parse fills in the array                 */
typedef struct _mime_parse_info
{
    int    mime_type;           /* mime type (MIME_RETURN_PATH etc) */
    PFCCHAR mime_type_str;      /* pointer to string representation of */
                                /* mime_type   */
    PFCHAR mime_field_ptr;      /* pointer to body in mime field */
    int    mime_field_len;      /* length of body the mime field */
} mime_parse_info;
typedef struct _mime_parse_info KS_FAR *PMIME_PARSE_INFO;

/* ********************************************************************   */
/* API functions                                                          */

#ifdef __cplusplus
extern "C" {
#endif

int xn_pop_checkmail(PPOP_CONTEXT context, PPOP_INFO info);
int xn_pop_mailsize(PPOP_CONTEXT context, PPOP_MSG_SIZE mail_sizes, int first_msg, int num_messages);
int xn_pop_mailinfo(PPOP_CONTEXT context, PPOP_MSG_INFO info, int msg_num);
int xn_pop_client(PPOP_CONTEXT context, PFBYTE pop_server_ip, PFCHAR username, PFCHAR pass);
int xn_pop_DELE(PPOP_CONTEXT context, int num);
int xn_pop_RETR_next(PPOP_CONTEXT context, int num, PMIME_PARSE_INFO parse_info);
int xn_pop_RETR_done(PPOP_CONTEXT context);
int xn_pop_QUIT(PPOP_CONTEXT context);

#ifdef __cplusplus
}
#endif

#endif      /* POPAPI_H */

