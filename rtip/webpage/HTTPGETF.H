//
// HTTPGETF.H - CGI defines
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//

#ifndef __HTTPGETF_HPP__
#define __HTTPGETF_HPP__

// *******************************************************************
// CGI function prototypes
// *******************************************************************
void get_fn(PIO_CONTEXT io_context, PFCHAR param);
void get_cgi(PIO_CONTEXT io_context, PFCHAR param);

#endif
