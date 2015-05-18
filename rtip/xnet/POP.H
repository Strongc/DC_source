/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */

/* POP.H - Post Office Protocol                                                 */

#ifndef POP_H
#define POP_H

#include "rtip.h"
#include "socket.h"
#include "popapi.h"

/* *********************************************************************   */
#define ASCII_SPACE         ' '
#define ASCII_CR            '\r'
#define ASCII_LF            '\n'
#define ASCII_EQUAL         '='
#define ASCII_QUOTE         '"'
#define ASCII_EOS           '\0'
#define ASCII_COLON         ':'
#define ASCII_DASH          '-'
#define ASCII_SEMI_COLON    ';'
#define ASCII_BACKSLASH     '\\'
#define ASCII_HTAB         '\t'

/* set up defines to point to constant strings in rtipcons.c   */
#define MIME_TERM_FIELD  mime_term_field
#define MIME_MSG         mime_msg
#define MULTIPART_STR    multipart_str

/* *********************************************************************   */
/* well-known port number for POP3                                         */
#define POP_PORT_NO         110

/* *********************************************************************   */
/* used for database of converting mime types to strings and vice versa    */
typedef struct _mime_header
{
    int  mime_type;
    char mime_header[30];
} mime_header;

KS_EXTERN_GLOBAL_CONSTANT struct _mime_header KS_FAR mime_headers[TOTAL_MIME_HEADERS];

/* ********************************************************************   */
/* types returned by pop_get_whole_line                                   */
#define TERM_HEADER -4
#define TERM_MSG    -5

#endif

