/*                                                                      */
/* base64.h - base64 encode and decode for SMTP, POP and WEB            */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __BASE64_H__
#define __BASE64_H__

/* NOTE: see rtipapi.h for additional BASE 64 definitions   */
#define NONTEXT_BUFFER_SIZE 200

#if (INCLUDE_SMTP || INCLUDE_WEB)
/* *********************************************************************   */
/* BASE 64 ENCODING/DECODING STUFF                                         */
/* *********************************************************************   */
typedef struct _base64_enconde_context
{
    int  current_line_length;
    byte triplet[3];
    byte bytes_in_triplet;
    byte line_len;              /* length of line before putting in \r\n; */
                                /* 0 means unlimited length   */
} base64_encode_context;                     
typedef base64_encode_context KS_FAR *PBASE64_ENCODE_CONTEXT;

#endif  /* INCLUDE_SMTP || INCLUDE_WEB */

/* *********************************************************************   */
#if (INCLUDE_POP3 || INCLUDE_SMTP || INCLUDE_WEB || BUILD_BINARY)
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mime_term_field;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *base64_alphabet;
#endif

/* *********************************************************************   */
#if (INCLUDE_SMTP || INCLUDE_WEB)
void        base64_init_encode_context(PBASE64_ENCODE_CONTEXT pContext, byte line_len);
int         base64_encode_finish(PBASE64_ENCODE_CONTEXT pContext, PFBYTE dest);
int         base64_encode(PBASE64_ENCODE_CONTEXT pContext, PFBYTE source, PFBYTE dest, int len);
#endif
#if (INCLUDE_POP3 || INCLUDE_ESMTP)
int         xn_decode_base64(PBASE64_DECODE_CONTEXT pContext,PFBYTE buffer, PFCHAR input, int input_length);
void        xn_base64_decode_init(PBASE64_DECODE_CONTEXT pContext);
#endif
#if (INCLUDE_POP3 || INCLUDE_WEB)
int         decode_base64_msg(PFCHAR input, int input_len);
#endif
#if (INCLUDE_POP3)
void    decode_msg(PFCHAR buffer,
                   int *offset, 
                   PBASE64_DECODE_CONTEXT p_base64_context,
                   int mime_len, RTIP_BOOLEAN not_first_flag);
#endif


#endif      /* __BASE64_H__ */

