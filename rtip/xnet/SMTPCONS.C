// SMTPCONS.C - SMTP CONSTANT GLOBAL DATA 
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//
//  Module description:
//      This module contains all the global data for SMTP
//      which is never modified.

#define DIAG_SECTION_KERNEL DIAG_SECTION_SNMP


#include "sock.h"

#if (INCLUDE_RTIP && INCLUDE_SMTP)

#include "base64.h"

// ********************************************************************
// SMTP
// ********************************************************************
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str1a1 = 
			"Date: %s\r\n"
			"Subject: %s\r\n"
            "MIME-version: 1.0\r\n"
			"From: %s\r\n"
			"To: %s, ";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str1a2 = 
			"Subject: %s\r\n"
            "MIME-version: 1.0\r\n"
			"From: %s\r\n"
			"To: %s, ";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str1b = 
			"Content-Type: multipart/mixed; boundary=\"%s\"\r\n"
			"This is a multi-part message in MIME format.\r\n\r\n"
            "%s\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str1c = 
			"Content-Type: text/plain; charset=us-ascii\r\n"
            "Content-Transfer-Encoding: 7BIT\r\n\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str2=	"\r\n\r\n%s\r\nContent-Type: %s"
			" name=\"%s\"\r\nContent-Transfer-Encoding"
			": %s\r\nContent-Disposition: inline; filename=\"%s\"\r\n\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str2a = "\r\n\r\n%s\r\nContent-Type: %s; "
			"charset=us-ascii; \r\nContent-Transfer-Encoding"
			": %s\r\nContent-Disposition: inline; \r\n\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str3     = "\r\n%s--";		// closing boundary
KS_GLOBAL_CONSTANT char KS_FAR *smtp_boundary = "--g4hd62ngJ2e!1~00B";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_helo     = "HELO ";
#if (INCLUDE_ESMTP)
KS_GLOBAL_CONSTANT char KS_FAR *smtp_ehlo     = "EHLO ";
#endif
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str6     = ">\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_FROM     = "MAIL FROM:<";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str8     = "RCPT TO:<";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str9     = "DATA\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str10    = "\r\n.\r\n";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_str11    = "QUIT\r\n";   // michele
KS_GLOBAL_CONSTANT char KS_FAR *smtp_7bit     = "7BIT";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_base64   = "base64";

#if (INCLUDE_ESMTP)
KS_GLOBAL_CONSTANT char KS_FAR *smtp_auth             = "250-AUTH=";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_resp_334         = "334";
KS_GLOBAL_CONSTANT char KS_FAR smtp_space_char        = ' ';
KS_GLOBAL_CONSTANT char KS_FAR *smtp_login_plain      = "LOGIN PLAIN";
KS_GLOBAL_CONSTANT char KS_FAR *smtp_auth_login_reply = "AUTH LOGIN\r\n";
#endif

#endif		// INCLUDE_RTIP && INCLUDE_SMTP
