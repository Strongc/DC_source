/*
* SERV.H
*
*
* Copyright 1994 Etc Bin Systems. All rights reserved
*
*
* Author: Peter Van Oudenaren
*
***********************************************************************/

#ifndef __SERV__
#define __SERV__ 1

#include "XNCONF.H"
/* ********************************************************************   */
/* API for servers                                                        */
/* ********************************************************************   */

#ifdef __cplusplus
  extern "C" {
#endif

#if (INCLUDE_FTP_SRV)
void start_ftp_server(void);
void kill_ftp(void);
#endif

#if (INCLUDE_WEB)
void start_web_server(int interface_nr);
void kill_web(void);
#else
#define start_web_server(a)
#define kill_web(a)

#endif

#if (INCLUDE_HTTPS_SRV)
void start_https_server(void);
void kill_https(void);
#endif

#if (INCLUDE_TELNET_SRV)
void start_telnet_server(void);
void kill_telnet(void);
#endif

#if (INCLUDE_SNMP)
void kill_snmp(void);
#endif

#if (INCLUDE_TFTP_SRV)
void start_tftp_server(void);
#endif

#if (INCLUDE_SNMP)
void start_snmp(PFBYTE ip_trap_address);
#endif

#if (INCLUDE_DHCP_SRV)
void start_dhcps_server(void);
#endif

#if (INCLUDE_NFS_SRV)
void start_nfss_server(void);
#endif

#ifdef __cplusplus
  }
#endif



#endif  /*  __SERV__ */

