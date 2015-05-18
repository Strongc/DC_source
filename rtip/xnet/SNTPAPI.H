//
// SNTPAPI.H  - SNTPAPI header file
//
//   EBS - RTIP
//
//   Copyright Peter Van Oudenaren , 1993
//   All rights reserved.
//   This code may not be redistributed in source or linkable object form
//   without the consent of its author.
//
//    Module description:
//        This file contains prototypes constants and structure
//        declarations required fpr SNTP,  Simple Network Time Protocol's
//        API
//
#ifndef __SNTPAPI__
#define __SNTPAPI__ 1

// *********************************************************************
typedef struct _ebs_sys_time
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int msec;
	char day_of_week[10];
} EBS_SYS_TIME;
typedef EBS_SYS_TIME KS_FAR *PEBS_SYS_TIME;

// *********************************************************************
#define SNTP_VERSION_3 		0x18		// IPV4
#define SNTP_VERSION_4 		0x20		// IPV4, IPV6, OSI

// ********************************************************************
// EXTERNAL DECLARATIONS
// ********************************************************************
#if (INCLUDE_RUN_TIME_CONFIG)
extern CFG_SNTP_DATA KS_FAR cfg_sntp_data;
#endif

// ********************************************************************
// API
// ********************************************************************
#ifdef __cplusplus
extern "C" {
#endif

int  SOCKAPI_ xn_sntp_get_time(PEBS_SYS_TIME tme, PFBYTE ip_sntp_server, int version);

#ifdef __cplusplus
}
#endif


#endif // __SNTPAPI.H__





