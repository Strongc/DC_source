//
// HTTPGETF.C - WEB CGI support functions
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//
// This file has the table and the functions used for CGI-BIN (server
// side includes)
//

#include "sock.h"

#if (INCLUDE_RTIP && BUILD_NEW_BINARY && defined(EXBIN))
#include "exbin.h"
#elif (INCLUDE_RTIP)
#include "rtip.h"
#endif

#include "rtipapi.h"
#include "webapi.h"
#include "netutil.h"
#include "httpgetf.h"
#include "application/webif/webif.h"

// *******************************************************************
// CGI functions - these include cgi's from within a file and requests
// for cgi files (fakefilename.cgi)
// NOTE: this table is initialized with entries which are needed by
//       the sample webpages.  You may delete all the entries along
//       with their associated routines and add your own entries
struct get_function_entry KS_FAR get_function_table[] =
{
  {"get.fn",                     get_fn},
  {"get.cgi",                    get_cgi},
  {"", END_OF_TABLE}
};

static void line_put(PIO_CONTEXT io_context, const char* szLine)
{
  int remaining = tc_strlen(szLine);
  int offset = 0;

  while (remaining > 0) {
    if (remaining > MAX_PACKETSIZE) {
      io_context->length_out = MAX_PACKETSIZE;
    } else {
      io_context->length_out = remaining;
    }

    tc_strncpy(io_context->buffer_out, &szLine[offset], io_context->length_out);

    offset += io_context->length_out;
    remaining -= io_context->length_out;

    xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE | PUT_SEND);
  }
}

void xn_line_put_get_callback(void* pIOContext, char* pBuffer, int length)
{
  PIO_CONTEXT io_context = pIOContext;
  if (length) {
    io_context->length_out = length;
    xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE | PUT_SEND);
  }
}

void get_fn(PIO_CONTEXT io_context, PFCHAR param)
{
  if (param)
    webifHttpGetFN_C(param, io_context, io_context->buffer_out, MAX_PACKETSIZE, xn_line_put_get_callback);
  else
    line_put(io_context, "&lt;ERROR: get.fn param missing&gt;");
}

void get_cgi(PIO_CONTEXT io_context, PFCHAR param)
{
  if (param)
    webifHttpGetCGI_C(param, io_context, io_context->buffer_out, MAX_PACKETSIZE, xn_line_put_get_callback);
  else
    line_put(io_context, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 47\r\n\r\nError: Parameter missing on get.cgi request URL");
}
