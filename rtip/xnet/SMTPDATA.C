// SMTPDATA.C - SMTP GLOBAL DATA 
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
//      which is not constant.

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "rtipapi.h"
#include "smtpapi.h"

// *********************************************************************
// ******                CONFIGURATION                            ******
// *********************************************************************
#if (INCLUDE_RUN_TIME_CONFIG)
struct cfg_smtp_data cfg_smtp_data =
{
	_CFG_SMTP_TIMEOUT
};
#endif

