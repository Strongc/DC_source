//
// WEBAPI.H - WEB header file
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//

#ifndef __WEBAPI_H__
#define __WEBAPI_H__ 1

#include "webconf.h"
#include "netutil.h" 

// *********************************************************************
// ******                CONFIGURATION                            ******
// *********************************************************************
#if (INCLUDE_RUN_TIME_CONFIG)
typedef struct cfg_web_data
{
	int web_keepalive_tmo;
	int web_keepalive_max;
	int web_listen_backlog;
	int web_timeout;
#if (CONFIG_PARAM_DONE)
	int max_auth_cookie;
	int max_browsers;
#endif
#if INCLUDE_WEB_CWD
	char * WebRootDir;
#endif
	int  mod_date;
} CFG_WEB_DATA;

extern CFG_WEB_DATA KS_FAR cfg_web_data;

#define CFG_WEB_KEEPALIVE_TMO   cfg_web_data.web_keepalive_tmo
#define CFG_WEB_KEEPALIVE_MAX   cfg_web_data.web_keepalive_max
#define CFG_WEB_LISTEN_BACKLOG  cfg_web_data.web_listen_backlog
#define CFG_WEB_TIMEOUT 		cfg_web_data.web_timeout
#if (CONFIG_PARAM_DONE)
#define CFG_MAX_AUTH_COOKIE     cfg_web_data.max_auth_cookie
#define CFG_MAX_BROWSERS 		cfg_web_data.max_browsers
#else
#define CFG_MAX_AUTH_COOKIE		_CFG_MAX_AUTH_COOKIE
#define CFG_MAX_BROWSERS		_CFG_MAX_BROWSERS
#endif
#define CFG_WEB_ROOT_DIR		cfg_web_data.WebRootDir
#define CFG_MOD_DATE			cfg_web_data.mod_date

#else
#define CFG_WEB_KEEPALIVE_TMO	_CFG_WEB_KEEPALIVE_TMO
#define CFG_WEB_KEEPALIVE_MAX	_CFG_WEB_KEEPALIVE_MAX
#define CFG_WEB_LISTEN_BACKLOG	_CFG_WEB_LISTEN_BACKLOG
#define CFG_MAX_AUTH_COOKIE		_CFG_MAX_AUTH_COOKIE
#define CFG_WEB_TIMEOUT			_CFG_WEB_TIMEOUT
#define CFG_MAX_BROWSERS		_CFG_MAX_BROWSERS
#define CFG_WEB_ROOT_DIR		_CFG_WEB_ROOT_DIR
#define CFG_MOD_DATE			_CFG_MOD_DATE			
#endif

// *********************************************************************
// max length of date strings including eos
#define MAX_DATE_LEN 35	  

// *******************************************************************
// GET FUNCTION TABLE
// *******************************************************************
// Structure for server includes
struct get_function_entry 
{
  char *function_name;							// HTML name
  void (*function_ptr)(PIO_CONTEXT io_context, PFCHAR param);	
												// function to call
};

#define END_OF_TABLE 0

//****************************************************************
// POST FUNCTION TABLE
//****************************************************************
// Structure for post functions
// NOTE: also uses END_OF_TABLE for last entry
struct post_function_entry 
{
	char *function_name;						// HTML name
	void (*function_ptr)(PIO_CONTEXT io_context, long len);
};

//****************************************************************
// BROWSER SECURITY STUFF
//****************************************************************
typedef struct browser
{
	byte ip_addr[IP_ALEN];		// acceptable server IP addresses
	byte ip_mask[IP_ALEN];		// mask for server IP addresses
} browser;

typedef browser KS_FAR * PFBROWSER;
	
//#if (INCLUDE_WEB_AUTHENTICATION)
//****************************************************************
// BASIC AUTHENTICATION
//****************************************************************

// format of structure with basic authentication information;
// parameter to http_set_basic_auth
typedef struct web_auth_entry
{
	char * filename;		  	// file name
    char * realm;				// realm value to use in mime header
								// Basic realm="xxx"
								// realm is case sensitive
    char * user_pwd;		  	// username and password 
								// NOTE: they must be separated by a colon
								//       i.e. stone:mud for 
								//       user name = stone and password = mud
	// INTERNAL USE			  	
	char * encoded_user_pwd;	// space to encode user_pwd
	int    encoded_len;			// length of encoded user_pwd

	// NOTE INTERNAL USE
    char * err401_file;   		// this is the file to return on 
								// authentication failure.
} web_auth_entry;

//#endif		

//#if (INCLUDE_WEB_EXTRA_MIME)
//****************************************************************
// OUTPUT MIME INFORMATION SUPPLIED BY API (used to format response)
//****************************************************************
// extra mime fields to sent in mime header in response to a GET
// command for specified web pages
typedef struct mime_for_pages
{
	PFCCHAR web_page;		// web page (i.e. file name); and empty string
							// means apply to all web pages
							// NOTE: only the first empty web page name
							//       will be used
	PFCCHAR mime_fields;	// mime fields separated by \r\n
} MIME_FOR_PAGES;

typedef struct mime_for_pages KS_FAR * PFMIME_FOR_PAGES;
//#endif

//****************************************************************
// FUNCTION DECLARATIONS
//****************************************************************

#ifdef __cplusplus
extern "C" {
#endif

// supplied by application
extern struct get_function_entry  KS_FAR get_function_table[];
extern struct post_function_entry KS_FAR post_function_table[];

// WEBAPI.C
void    init_web_srv(void);
int    	http_server_daemon(void);
int    	http_server_daemon_interface(int iface_no);
int     http_kill_server_daemon(void);
int     http_server_init(void);
int 	http_server_process_one(void);
//#if (INCLUDE_HTTPS_SRV)
int    	https_server_daemon(void);
int     https_server_daemon_interface(int iface_no);
int     https_kill_server_daemon(void);
int     https_server_init(void);
int 	https_server_process_one(void);
//#endif

//#if (INCLUDE_WEB_PUSH)
int    	http_begin_push(PIO_CONTEXT web_put_info);
int     http_push_data(PIO_CONTEXT web_put_info, PFCHAR data, int length, char *content_type);
int     http_end_push(PIO_CONTEXT web_put_info);
//#endif

int 	http_set_browser_list(PFBROWSER browser_info, int num_elements);
PFCHAR 	http_find_string_value(PFCHAR buffer, PFCHAR match, int got);
//#if (INCLUDE_WEB_AUTHENTICATION)
int     http_set_auth(struct web_auth_entry *web_auth, int num_entries);
//#endif
//#if (INCLUDE_WEB_EXTRA_MIME)
void    http_mime_fields(PFMIME_FOR_PAGES web_fields);
//#endif

#ifdef __cplusplus
}
#endif

#endif		// __WEBAPI_H__

