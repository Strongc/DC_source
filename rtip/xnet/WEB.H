//
// WEB.H - WEB header file
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//

#ifndef __WEB_H__
#define __WEB_H__ 1

#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif
#include "webconf.h"
#include "netutil.h"

#if (INCLUDE_HTTPS_SRV)
#define HTTPS_PORT  443     // well known HTTPS port
#endif

#define WWW_PORT    80      // well known WWW port

// number of months in a year
#define NUM_MONTHS 12

// number of error codes; needs to jive with enum http_codes
#define NUMBER_HTTP_RET_STRINGS 8

// error codes which are used as an offset into the array http_return_strings
// NOTE: if add/delete error messages, need to modify NUMBER_HTTP_RET_STRINGS
enum http_codes 
{
	OK_200,                  /* 0 */
	OK_PUT_200,              /* 1 */
	NOT_MODIFIED_304,        /* 2 */
    BAD_REQUEST_400,         /* 3 */
    UNAUTHORIZED_401,        /* 4 */
    FORBIDDEN_403,           /* 5 */
    NOT_FOUND_404,           /* 6 */
    NOT_IMPLEMENTED_501      /* 7 */
};

// macro to get return string in http_return_strings
#define HTTP_GET_RETURN_STRING(buf, error) \
	tc_strcpy(buf, \
    (PFCHAR)&http_return_strings[error][0]);

//****************************************************************
enum 
{
#if (INCLUDE_WEB_PUT)
   	PUT_CMD,
#endif
   	GET_CMD, 
	POST_CMD, 
	INVALID_CMD
};

struct command_info_tag
{
   long lnumber;		// number associated with request
   char *zcommand;		// HTTP request (GET or POST)
};

#define END_OF_COMMANDS {INVALID_COMMAND, NULL}
typedef struct command_info_tag COMMAND_INFO;

typedef COMMAND_INFO COMMAND_ARRAY[];

//****************************************************************
// WEB CONTEXT
//****************************************************************
typedef struct _web_context 
{
#if (INCLUDE_MALLOC_CONTEXT)
	OS_LIST    list1;			// list of web contexts - needed to
								// kill all sessions
#endif
	SOCKET     web_socket;		// keep socket info so can kill task
	IO_CONTEXT io_context; 
} web_context;
typedef struct _web_context KS_FAR * PWEB_CONTEXT;

//****************************************************************
// OUTPUT MIME STUFF (mime info used to format response)
//****************************************************************
#define MAX_MIME_FIELD_SIZE 85
#define MAX_MIME_HEADER_SIZE 1000

// information used to format mime header for the response
struct http_mime_header 
{
	char    return_hdr[MAX_MIME_FIELD_SIZE];	// Normally "HTTP/1.1 200 Document Follows" 
	char    cdate[MAX_MIME_FIELD_SIZE];			// Creation date
	char    content_type[MAX_MIME_FIELD_SIZE];	// Content-Type (defaults to text/html)
	char    fdate[MAX_MIME_FIELD_SIZE];			// Last-modified
	long    clength;						  	// Content-Length; -1 if unknown
#if (INCLUDE_WEB_AUTHENTICATION)
    PFCCHAR  auth_realm;						// pointer to realm for
												// Basic authentication
#endif
	RTIP_BOOLEAN persistent;						// specifies whether the connection
											// is persistent (i.e. not closed
											// between web pages); default
											// is browser and server are 
											// version 1.1 is TRUE
#if (INCLUDE_WEB_EXTRA_MIME)
	PFCCHAR  web_fields_all_pages; 			// extra mime fields supplied
	PFCCHAR  web_fields; 					//   by api
#endif
};

//****************************************************************
// INPUT MIME STUFF (mime information in request)
//****************************************************************
// max length of date strings including eos
#define MAX_DATE_LEN 35	  

// pointers to information in mime header of request message
typedef struct http_mime_ptrs
{
	long 	clength;					// length from CONTENT-LENGTH
	char    if_mod_date[MAX_DATE_LEN];	// date from IF-MODIFIED-DATE
    char    auth_basic_cookie[CFG_MAX_AUTH_COOKIE];		    
										// 64-bit encoded username:password
									    // for basic authentication
	int     auth_basic_cookie_len;	    // length of basic cookie

#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
	RTIP_BOOLEAN keep_alive;					// browser supports keep-alive
	RTIP_BOOLEAN persistent;					// specifies whether the connection
										// is persistent (i.e. not closed
										// between web pages); default
										// is browser and server are 
										// version 1.1 is TRUE
#endif

#if (INCLUDE_WEB_ETAG)
	PFCHAR	etag;
#endif
} http_mime_ptrs;

//****************************************************************
// stuff for web_file_type (array used to convert ext type to
// Content-Type in mime header)
// number of entries in web_file_types
#define WEB_NUM_FILE_TYPES 17

// max length of content_type (used for field in web_file_type
#define WEB_MAX_CONTENT_TYPE 35

typedef struct web_file_type 
{
	char ext[6];
	char content_type[WEB_MAX_CONTENT_TYPE];
} web_file_type;


//****************************************************************
// EXTERNAL DECLARATIONS
//****************************************************************
KS_EXTERN_GLOBAL_CONSTANT COMMAND_ARRAY KS_FAR web_cmds;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR http_return_strings[NUMBER_HTTP_RET_STRINGS][80];

//****************************************************************
// FUNCTION DECLARATIONS
//****************************************************************
// WEBAPI.C
void    http_server(SOCKET socket, PWEB_CONTEXT web_context);
void   	http_send_response(PIO_CONTEXT io_context, int error);
void    http_get_return_string(PFCHAR buf, int error);

#if (INCLUDE_WEB_AUTHENTICATION)
RTIP_BOOLEAN check_authenticate_request(PIO_CONTEXT io_context, char *filename,
                                   struct http_mime_ptrs *mime_ptrs);
#endif

// WEBPROC.C
int     process_get_command(PIO_CONTEXT io_context, struct http_mime_ptrs *mime_ptrs, PFCHAR args);
int     process_post_command(PIO_CONTEXT io_context, struct http_mime_ptrs *mime_ptrs, PFCHAR args);
long    get_command(PFCHAR cmdline, PFCHAR *arguments);
void    get_version(PFCHAR buffer, int bufsize, int *major_number, int *minor_number);
RTIP_BOOLEAN tc_isnum(PFCHAR s);
RTIP_BOOLEAN check_char(PFCHAR buffer);
void    fix_escaped_chars(PFCHAR buffer, char *ns_string, char replace_char);
void    fix_escaped_chars_all(PFCHAR buffer);
int     send_mime_header(PIO_CONTEXT web_put_info, struct http_mime_header KS_FAR *hdr);
void    init_mime_header(struct http_mime_header KS_FAR *hdr);
#if (0)
void    read_mime_header(PIO_CONTEXT io_context, long wait, long *len);
#endif

#if (INCLUDE_WEB_PUT)
int     process_put_command(PIO_CONTEXT io_context, PFCHAR args);
#endif // INCLUDE_WEB_PUT


#endif		// __WEB_H__

