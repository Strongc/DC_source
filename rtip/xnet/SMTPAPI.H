// SMTPAPI.H
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//

#ifndef __SMTPAPI__
#define __SMTPAPI__ 1

#include "rtipapi.h"

// *********************************************************************
// ******                CONFIGURATION                            ******
// *********************************************************************
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_smtp_data
{
	int smtp_timeout;
	int smtp_init_timeout;
} CFG_SMTP_DATA;

extern struct cfg_smtp_data cfg_smtp_data;

#define CFG_SMTP_TIMEOUT		cfg_smtp_data.smtp_timeout
#define CFG_SMTP_INIT_TIMEOUT	cfg_smtp_data.smtp_timeout
#else
#define CFG_SMTP_TIMEOUT		_CFG_SMTP_TIMEOUT
#define CFG_SMTP_INIT_TIMEOUT	_CFG_SMTP_INIT_TIMEOUT
#endif

// ********************************************************************
#define RCPT_LEN 80

// receipient of mail message
struct rcpt
{
	char name[RCPT_LEN];
};

// ********************************************************************
typedef struct _attach_file_info
{
	PFCHAR attach_file;					// filename of attachment
										// (set to 0 if no attachment)
	PFCHAR attach_type;					// attachment type 
										// (us-ascii, text/plain etc.)
										// (set to 0 if no attachment)
} attach_file_info;


// parameter to xn_smtp
typedef struct _smtp_info
{
	byte smtp_server_ip[IP_ALEN];		// IP address of SMTP server
	char reverse_path[200];				// domain name of sender
										// in format [domain name]
										// can be [] but then server
										// cannot send back error messages
#if (INCLUDE_ESMTP)
	char user_name[CFG_MAX_LOGIN_LEN];
	char password[CFG_MAX_LOGIN_LEN];
#endif
	char from[CFG_MAX_FROM_LEN]; 		// email address of author
	char subject[CFG_MAX_SUBJECT_LEN];	// subject of message
	struct rcpt rcpts[CFG_SMTP_MAX_RCPTS]; 
										// list of recipients where the first
										// num_rcpts the message is sent To
										// and the next num_cc the message
										// is CC  
	int num_rcpts;						// # of recipients for To
	int num_cc;							// # of recipients for cc
	PFCCHAR mime_fields;				// mime fields separated by \r\n;
										// last mime field must end with \r\n
	PFCHAR body;  						// body of message
										// (set to 0 if body not in a buffer)
    PFCHAR body_file;					// filename for body of message
										// (set to 0 if body not in a file)
	
	PFCHAR attach;  					// attachment buffer
										// (set to 0 if no attachment in a 
										// buffer)

	int attach_len;						// set to length of attach if it 
										// contains
										// binary data, otherwise left at 
										// zero, and attach will be treated 
										// as a null-terminated string.

	struct _attach_file_info *at;       // point to array of file infos
    int    num_attach_files;            // number of file attachments
} smtp_info;
typedef struct _smtp_info KS_FAR *PSMTP_INFO;

// ********************************************************************
// API
// ********************************************************************

#ifdef __cplusplus
extern "C" {
#endif

int xn_smtp(PSMTP_INFO info);

#ifdef __cplusplus
}
#endif

#endif		// __SMTPAPI__

