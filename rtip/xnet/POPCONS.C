/* POPCONS.C - POP CONSTANT GLOBAL DATA                                 */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for POP                */
/*      which is never modified.                                        */

#define DIAG_SECTION_KERNEL DIAG_SECTION_MAIL


#include "sock.h"

#if (INCLUDE_POP3)
#include "popapi.h"
#include "pop.h"
#include "base64.h"

/* ********************************************************************   */
/* POP                                                                    */
/* ********************************************************************   */
KS_GLOBAL_CONSTANT char KS_FAR *mimesub_boundary_str    = "boundary";
KS_GLOBAL_CONSTANT char KS_FAR *mimesub_filename_str    = "filename";
KS_GLOBAL_CONSTANT char KS_FAR *mime_msg        = "This is a multi-part message in MIME format.";
KS_GLOBAL_CONSTANT char KS_FAR *multipart_str   = "multipart";

KS_GLOBAL_CONSTANT char KS_FAR *pop_str1 = "RETR ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str2 = "DELE ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str3 = "QUIT\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str4 = "R: ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str5 = "+OK";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str6 = "-ER";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str7 = "S: ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str8 = "LIST\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str9 = "USER ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str10 = "PASS ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str11 = "STAT\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str12 = "TOP ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str13 = " 0\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str14 = "From: ";
KS_GLOBAL_CONSTANT char KS_FAR *pop_str15 = "Subject: ";

KS_GLOBAL_CONSTANT struct _mime_header KS_FAR mime_headers[TOTAL_MIME_HEADERS] = 
{
    {MIME_RETURN_PATH,      "Return-Path"},
    {MIME_RECEIVED,         "Received"},
    {MIME_DATE,             "Date"},
    {MIME_FROM,             "From"},
    {MIME_MESSAGE_ID,       "Message-Id"},
    {MIME_SUBJECT,          "Subject"},
    {MIME_VERSION,          "MIME-version"},
    {MIME_CONTENT_TYPE,     "Content-Type"},
    {MIME_STATUS,           "Status"},
    {MIME_CONTENT_ENCODE,   "Content-Transfer-Encoding"},
    {MIME_CONTENT_DISP,     "Content-Disposition"},
    {MAIL,                  "Message"},
    {ATTACH,                "Attachment"},
    {MAIL_START,            "Mail start"},
    {ATTACH_START,          "Attachment start"},
    {MAIL_END,              "Mail end"},
    {ATTACH_END,            "Attachment end"},
    {MSG_DONE,              "Message done"},
};

KS_GLOBAL_CONSTANT char KS_FAR *base64_content_type = "base64";

#endif      /* INCLUDE_POP3 */



