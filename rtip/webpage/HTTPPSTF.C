//
// HTTPPSTF.C - WEB POST support functions
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//
// All the user supplied POST functions are defined here

#include "sock.h"

#if (INCLUDE_RTIP && BUILD_NEW_BINARY && defined(EXBIN))
#include "exbin.h"
#elif (INCLUDE_RTIP)
#include "rtip.h"
#endif

#include "httppstf.h"
#include "application/webif/webif.h"

// ********************************************************************
#define POST_TIMEOUT 1	// number of seconds to wait for more input
                        // NOTE: if this is too large, display from POST
                        // command will be slow since many browsers
                        // wait for FIN before displaying the data
// ********************************************************************
// GLOBAL DATA

// POST FUNCTION TABLE
// NOTE: this table is initialized with entries which are needed by
//       the sample webpages.  You may delete all the entries along
//       with their associated routines and add your own entries
struct post_function_entry KS_FAR post_function_table[] =
{
  {"post.cgi", post_cgi},
  {"", END_OF_TABLE}
};

void xn_line_put_post_callback(void* pIOContext, char* pBuffer, int length)
{
  PIO_CONTEXT io_context = pIOContext;
  if (length) {
    io_context->length_out = length;
    xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE | PUT_SEND);
  }
}

void post_cgi(PIO_CONTEXT io_context, long len)
{
	PFCHAR params;

	if (!len)
	{
		len = xn_line_get(io_context, &params, POST_TIMEOUT, GET_LINE);
	}

	if (len)
	{
		// bypass any line feeds
		while (xn_line_get(io_context, &params, POST_TIMEOUT, GET_LINE) == 0);
  }
  
  webifHttpPostCGI_C(params, io_context, io_context->buffer_out, MAX_PACKETSIZE, xn_line_put_post_callback);
}


