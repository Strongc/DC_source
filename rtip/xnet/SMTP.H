/* SMTP.H                                                               */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __SMTP__
#define __SMTP__ 1

#include "socket.h"
#include "smtpapi.h"

/* attachment types   */
#define NO_TYPE         0
#define TEXT_TYPE       1
#define BINARY_TYPE     2

/* well-known port number for SMTP   */
#define SMTP_PORT_NO 25

/* MIME strings for predefined content types   */
extern char KS_FAR *text_plain;
extern char KS_FAR *multipart_mixed;
extern char KS_FAR *multipart_alternative;
extern char KS_FAR *multipart_digest;
extern char KS_FAR *multipart_parallel;
extern char KS_FAR *message_rfc822;
extern char KS_FAR *message_partial;
extern char KS_FAR *message_external;
extern char KS_FAR *application_octetstream;
extern char KS_FAR *application_postscript;

#endif

