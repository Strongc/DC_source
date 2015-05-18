//
// WEBCONS.C - WEB CONSTANT GLOBAL DATA 
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//
//  Module description:
//      This module contains all the global data for WEB
//      which is never modified.


#include "sock.h"

#if (INCLUDE_WEB)

#include "web.h"
//****************************************************************
// webproc.c
//****************************************************************

// strings returned to browser;
// the emun http_codes defines the offset in this array, 
// DO NOT CHANGE ORDER OR ADD ENTRIES WITHOUT MODIFYING http_codes ACCORDINGLY
// NOTE: all entries have a newline except 200 since this is the only non-error
//       case; this one is used in the mime header and all string added to
//       the header are proceeded by \r\n; all errors are sent as a complete
//       message
#if (INCLUDE_WEB_11)
KS_GLOBAL_CONSTANT char KS_FAR http_return_strings[NUMBER_HTTP_RET_STRINGS][80] = 
{
	"HTTP/1.1 200 Document follows",		
    "HTTP/1.1 200 File upload OK\r\n",
	"HTTP/1.1 304 Not Modified\r\n\r\n",
	"HTTP/1.1 400 Malformed Get request\n\n\n\n<H1>Malformed GET request</H1>",
	"HTTP/1.1 401 Unauthorized",
	"HTTP/1.1 403 Forbidden",
	"HTTP/1.1 404 The requested URL was not found\n\n\n\n<H1>404 Not Found</H1>",
	"HTTP/1.1 501 Command not implemented\n\n\n\n"
};
#else
KS_GLOBAL_CONSTANT char KS_FAR http_return_strings[NUMBER_HTTP_RET_STRINGS][80] = 
{
	"HTTP/1.0 200 Document follows",		
    "HTTP/1.0 200 File upload OK\r\n",
	"HTTP/1.0 304 Not Modified\r\n\r\n",
	"HTTP/1.0 400 Malformed Get request\n\n\n\n<H1>Malformed GET request</H1>",
	"HTTP/1.0 401 Unauthorized",
	"HTTP/1.0 403 Forbidden",
	"HTTP/1.0 404 The requested URL was not found\n\n\n\n<H1>404 Not Found</H1>",
	"HTTP/1.0 501 Command not implemented\n\n\n\n"
};
#endif

// array used to convert ext type to Content-Type in mime header
KS_GLOBAL_CONSTANT struct web_file_type KS_FAR web_file_types[WEB_NUM_FILE_TYPES] =
{
	// NOTE: do not change order of first two entries
	{"",      "text/plain"},					// default
//	{"",	  "application/octet-stream"},		// default
	// htm and html need to be offsets 1 and 2
	{"htm",   "text/html"},						
	{"html",  "text/html"},

	// NOTE: order of rest (or new entries can be added) from this point
	{"txt",   "text/plain"},
	{"jpg",   "image/jpeg"},
	{"gif",   "image/gif"},
	{"mpg",   "image/mpeg"},
	{"wav",   "audio/x-wave"},
	{"zip",   "application/x-zip-compressed"},
	{"class", "application/java-vm"},
	{"cla",   "application/java-vm"},
    {"css",   "text/css"},                      /* I2SE CSS */
    {"tar",   "application/x-tar"},             /* I2SE Tar Archiv*/
    {"z",     "application/x-compress"},        /* I2SE Z Archiv*/
//	{"jar",   "text/plain"}, 
    {"jar",   "application/java-archive"},      /* I2SE Java archive*/
    {"js",    "application/x-javascript"},      /* I2SE JavaScript*/
    {"pdf",   "application/pdf"}                /* I2SE Acrobat Reader*/
};

// task name for tasks spawned to process web requests
KS_GLOBAL_CONSTANT char KS_FAR *web_string = "WEB ";

#if (INCLUDE_HTTPS_SRV)
// task name for tasks spawned to process secure web requests
KS_GLOBAL_CONSTANT char KS_FAR *https_string = "HTTPS ";
#endif // INCLUDE_HTTPS_SRV

//****************************************************************
// webproc.c
//****************************************************************
// These are the various HTTP commands that are understood.
KS_GLOBAL_CONSTANT COMMAND_ARRAY KS_FAR web_cmds =
{
   {GET_CMD,  "GET"},
   {POST_CMD, "POST"},
#if (INCLUDE_WEB_PUT)
   {PUT_CMD,  "PUT"},
#endif 
   {INVALID_CMD, ""}
};

// list of months used for dates
KS_GLOBAL_CONSTANT char KS_FAR month_strings[12][4] = 
{
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};

#endif		// INCLUDE_WEB
