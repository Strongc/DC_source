
/*
* SERV.C
*
* Copyright 1994 Etc Bin Systems. All rights reserved
*
* Author: Peter Van Oudenaren
*
***********************************************************************/

#include "sock.h"
#include "rtipapi.h"
#include "rtip.h"
#include "servers.h"
#include "terminal.h"
#include "serv.h"
#include "webpages.h"

#if (INCLUDE_WEB)
    #include "webapi.h"
#endif

#if (INCLUDE_FTP_SRV)
    #include "ftpapi.h"
#endif

#if (INCLUDE_TFTP_SRV)
    #include "tftpapi.h"
#endif

#if (INCLUDE_TELNET_SRV)
    #include "tnsapi.h"
#endif

#if (INCLUDE_DHCP_SRV)
    #include "dhcpsapi.h"
#endif

#if (INCLUDE_NFS_SRV)
    #include "nfs.h"
#endif

#define TEST_WEB_AUTHENTICATION 0   /* turn on to test basic authentication  */
                                    /* from RFC   */
#define TEST_WEB_SECURITY       0   /* turn on to test security based up  */
                                    /* browser's IP address   */
#define TEST_NFS_UNIX_AUTH      0   /* turn on to test UNIX auth  */
#define TEST_LARGE_SNMP_BUF     1



/* *******************************************************************   */
/* GLOBAL DATA                                                           */
/* ********************************************************************  */
RTIP_BOOLEAN KS_FAR ftp_spawned = FALSE;
RTIP_BOOLEAN KS_FAR tftp_spawned = FALSE;
RTIP_BOOLEAN KS_FAR telnet_spawned = FALSE;
RTIP_BOOLEAN KS_FAR web_spawned = FALSE;
RTIP_BOOLEAN KS_FAR https_spawned = FALSE;
RTIP_BOOLEAN KS_FAR nfss_spawned = FALSE;
RTIP_BOOLEAN KS_FAR dhcps_spawned = FALSE;

#if (TEST_LARGE_SNMP_BUF)
#if (INCLUDE_RUN_TIME_CONFIG)
#define SNMP_LARGE_BUF (6100)
#else
#define SNMP_LARGE_BUF (CFG_MAX_FRAG+100)
#endif
byte KS_FAR snmp_buf[SNMP_LARGE_BUF];
#endif

/* ********************************************************************   */
void ftp_server_main(void);
void tftp_server_main(void);
void telnet_server_main(void);
void web_server_main(void);
void https_server_main(void);
void nfss_server_main(void);
void dhcps_server_main(void);
#if (INCLUDE_WEB_BROWSER )
void web_kbd_main(void);
#endif
#if (INCLUDE_SNMP)
void do_snmp_init(PFBYTE ip_trap_address);
#endif

/* ********************************************************************   */
/* EXTERNAL DATA                                                          */

extern RTIP_BOOLEAN KS_FAR ftp_spawned;
extern RTIP_BOOLEAN KS_FAR telnet_spawned;
extern RTIP_BOOLEAN KS_FAR web_spawned;
extern RTIP_BOOLEAN KS_FAR https_spawned;

/* ********************************************************************   */
/* ROUTINES TO SPAWN SERVER TASKS                                         */
/* ********************************************************************   */

#if (INCLUDE_WEB_BROWSER )
/* ********************************************************************       */
/* start_web_kbd() - spawn web keyboard task                                  */
/*                                                                            */
/*    If the web keyboard task ot previouly been spawned, this function *     */
/*    spawns the task web_kbd_main().                                         */
/*                                                                            */
/*    Returns nothing.                                                        */
/*                                                                            */
void web_kbd_server_main(void);

void start_web_kbd(void)
{
        /* spawn an web_kbd server task     */
        if (!os_spawn_task(TASKCLASS_DEMO_APPTASK,web_kbd_server_main, 0,0,0,0))
        {
            tm_puts( "spawn of web_kbd task failed");
            return;
        }
}
#endif


#if (INCLUDE_FTP_SRV)
/* ********************************************************************    */
/* start_ftp_server() - spawn ftp server task                              */
/*                                                                         */
/*    If the ftp server task has not previouly been spawned, this function */
/*    spawns the task ftp_server_main().  Then it displays server window   */
/*    which the user exits by hitting any key.                             */
/*                                                                         */
/*    Returns nothing.                                                     */
/*                                                                         */

void start_ftp_server(void)
{
    if (!ftp_spawned)
    {
        ftp_spawned = TRUE;

        /* spawn an ftp server task   */
        if (!os_spawn_task(TASKCLASS_FTP_DAEMON,ftp_server_main, 0,0,0,0))
        {
            tm_puts( "spawn of FTP task failed");
            return;
        }
    }
}
#endif

#if (INCLUDE_TFTP_SRV)
/* ********************************************************************     */
/* start_tftp_server() - spawn tftp server task                             */
/*                                                                          */
/*    If the tftp server task has not previouly been spawned, this function */
/*    spawns the task tftp_server_main().  Then it displays server window   */
/*    which the user exits by hitting any key.                              */
/*                                                                          */
/*    Returns nothing.                                                      */
/*                                                                          */

void start_tftp_server(void)
{
    if (!tftp_spawned)
    {
        tftp_spawned = TRUE;

        /* spawn an tftp server task   */
        if (!os_spawn_task(TASKCLASS_TFTP_DAEMON,tftp_server_main, 0,0,0,0))
        {
            tm_puts("spawn of TFTP task failed");
            return;
        }
    }
}
#endif      /* INCLUDE_TFTP_SRV */

#if (INCLUDE_TELNET_SRV)
/* ********************************************************************       */
/* start_telnet_server() - spawn telnet server task                           */
/*                                                                            */
/*    If the telnet server task has not previouly been spawned, this function */
/*    spawns the task telnet_server_main().  Then it displays server window   */
/*    which the user exits by hitting any key.                                */
/*                                                                            */
/*    Returns nothing.                                                        */
/*                                                                            */

void start_telnet_server(void)
{
    if (!telnet_spawned)
    {
        telnet_spawned = TRUE;

        /* spawn a telnet server task   */
        if (!os_spawn_task(TASKCLASS_TELNET_DAEMON,telnet_server_main, 0,0,0,0))
        {
            tm_puts("spawn of TELNET task failed");
            return;
        }
    }
}
#endif      /* INCLUDE_TELNET_SRV */

#if (INCLUDE_WEB)
/* ********************************************************************    */
/* start_web_server() - spawn web server task                              */
/*                                                                         */
/*    If the web server task has not previouly been spawned, this function */
/*    spawns the task web_server_main().  Then it displays server window   */
/*    which the user exits by hitting any key.                             */
/*                                                                         */
/*    Returns nothing.                                                     */
/*                                                                         */

#if (INCLUDE_WEB_AUTHENTICATION && TEST_WEB_AUTHENTICATION)
/* set up strings for username=stone and password=mud   */
/* NOTE: they MUST be separated by a colon              */
#define USRPWDSIZE 10
char usrpwd1[USRPWDSIZE] = "stone:mud";
char encoded_string1[BASE64_MAX_ENCODED_SIZE(USRPWDSIZE)];
char usrpwd2[USRPWDSIZE] = "mica:dirt";
char encoded_string2[BASE64_MAX_ENCODED_SIZE(USRPWDSIZE)];

struct web_auth_entry auth_info[] =
{
    /* file        realm       user name,password   space to encode usrpwd1   */
    /* ----        -----       ------------------   -----------------------   */
    {"survey.htm", "group2",   usrpwd1,             encoded_string1},
    {"index.html", "group1",   usrpwd2,             encoded_string2}
};
#endif

#if (INCLUDE_WEB_EXTRA_MIME)
/* extra mime fields to be sent in response to a GET command for    */
/* specified web pages;                                             */
/* NOTE: null terminated                                            */
struct mime_for_pages mime_page_info[] =
{
    /* all pages                        */
/*  {"", "Cache-Control: public"},      */
    /* index.html                       */
    {"index.html", "Cache-Control: no-cache"},
    /* index.html                                                       */
/*  {"index.html", "Cache-Control: no-cache\r\nCache-Control: public"}, */
    {"rtip.htm",   "ETag: xyz"},
    {"disk.htm",   "ETag: pdq"},
    {0, 0}                      /* END OF TABLE MARKER */
};
#endif

void start_web_server(int interface_nr)
{
#if (INCLUDE_WEB_SECURITY && TEST_WEB_SECURITY)
browser browser_info[2];
byte ip_addr[4] = {192, 168, 0, 103};
byte ip_mask[4] = {0xff, 0xff, 0xff, 0x00};

    tc_mv4(browser_info[0].ip_addr, ip_addr, IP_ALEN);
    tc_mv4(browser_info[0].ip_mask, ip_mask, IP_ALEN);
    ip_addr[3] = 61;
    tc_mv4(browser_info[1].ip_addr, ip_addr, IP_ALEN);
    tc_mv4(browser_info[1].ip_mask, ip_mask, IP_ALEN);
    if (http_set_browser_list((PFBROWSER)&browser_info, 2) < 0)
    {
        DEBUG_ERROR("http_set_browser_list failed", NOVAR, 0, 0);
    }
#endif

///////////////////////////////////////////////////////////////////////////////

http_file_init();

///////////////////////////////////////////////////////////////////////////////

#if (INCLUDE_WEB_AUTHENTICATION && TEST_WEB_AUTHENTICATION)
    http_set_auth(auth_info, sizeof(auth_info)/sizeof(web_auth_entry));
#endif

#if (INCLUDE_WEB_EXTRA_MIME)
    http_mime_fields((PFMIME_FOR_PAGES)&mime_page_info);
#endif
    if (!web_spawned)
    {
        web_spawned = TRUE;

        /* spawn an web server task   */
        if (!os_spawn_task(TASKCLASS_WEB_DAEMON,web_server_main, 0,0,0,0))
        {
            tm_puts("spawn of WEB task failed");
            return;
        }
    }
}


#if (INCLUDE_HTTPS_SRV)
/* ********************************************************************           */
/* start_https_server() - spawn secure web server task                            */
/*                                                                                */
/*    If the secure web server task has not previouly been spawned, this function */
/*    spawns the task https_server_main().  Then it displays server window        */
/*    which the user exits by hitting any key.                                    */
/*                                                                                */
/*    Returns nothing.                                                            */
/*                                                                                */

void start_https_server(void)
{
#if (INCLUDE_WEB_SECURITY && TEST_WEB_SECURITY)
browser browser_info[2];
byte ip_addr[4] = {192, 168, 0, 1};
byte ip_mask[4] = {0xff, 0xff, 0xff, 0xff};

    tc_mv4(browser_info[0].ip_addr, ip_addr, IP_ALEN);
    tc_mv4(browser_info[0].ip_mask, ip_mask, IP_ALEN);
    ip_addr[3] = 61;
    tc_mv4(browser_info[1].ip_addr, ip_addr, IP_ALEN);
    tc_mv4(browser_info[1].ip_mask, ip_mask, IP_ALEN);
    if (http_set_browser_list((PFBROWSER)&browser_info, 2) < 0)
    {
        DEBUG_ERROR("http_set_browser_list failed", NOVAR, 0, 0);
    }
#endif

#if (INCLUDE_WEB_AUTHENTICATION && TEST_WEB_AUTHENTICATION)
    http_set_auth(auth_info, sizeof(auth_info)/sizeof(web_auth_entry));
#endif

#if (INCLUDE_WEB_EXTRA_MIME)
    http_mime_fields((PFMIME_FOR_PAGES)&mime_page_info);
#endif

    if (!https_spawned)
    {
        https_spawned = TRUE;

        /* spawn a https server task   */
        if (!os_spawn_task(TASKCLASS_HTTPS_DAEMON,https_server_main, 0,0,0,0))
        {
            tm_puts("spawn of HTTPS Server task failed");
            return;
        }
    }
}
#endif  /* INCLUDE_HTTPS_SRV */
#endif  /* INCLUDE_WEB */

#if (INCLUDE_NFS_SRV)
/* ********************************************************************    */
/* start_nfss_server() - spawn nfss server task                            */
/*                                                                         */
/*    If the nfs server task has not previouly been spawned, this function */
/*    spawns the task nfss_server_main().  Then it displays server window  */
/*    which the user exits by hitting any key.                             */
/*                                                                         */
/*    Returns nothing.                                                     */
/*                                                                         */

void start_nfss_server(void)
{
    if (!nfss_spawned)
    {
        /* spawn an nfss server task   */
        if (!os_spawn_task(TASKCLASS_NFS_SERVER,nfss_server_main, 0,0,0,0))
        {
            tm_puts("spawn of NFS task failed");
            return;
        }
    }
}

#endif

#if (INCLUDE_DHCP_SRV)
/* ********************************************************************     */
/* start_dhcps_server() - spawn dhcps server task                           */
/*                                                                          */
/*    If the dhcp server task has not previouly been spawned, this function */
/*    spawns the task dhcps_server_main().  Then it displays server window  */
/*    which the user exits by hitting any key.                              */
/*                                                                          */
/*    Returns nothing.                                                      */
/*                                                                          */

void start_dhcps_server(void)
{
    if (!dhcps_spawned)
    {
        /* spawn an dhcps server task   */
        if (!os_spawn_task(TASKCLASS_DHCP_SERVER,dhcps_server_main, 0,0,0,0))
        {
            tm_puts("spawn of DHCP task failed");
            return;
        }
    }
}

#endif

#if (INCLUDE_SNMP)
/* ********************************************************************   */
void snmp_main_(void);

/* function to spawn SNMP task   */
void start_snmp(PFBYTE ip_trap_address)
{
    do_snmp_init(ip_trap_address);

    if (!os_spawn_task(TASKCLASS_SNMP_AGENT,snmp_main_, 0,0,0,0))
    {
        DEBUG_ERROR("start_snmp - spawning of snmp task failed",
            NOVAR, 0, 0);
    }
}
#endif

/* ********************************************************************   */
/* SERVER TASKS WHICH ARE SPAWNED                                         */
/* ********************************************************************   */

#if (INCLUDE_WEB_BROWSER )
void peg_console_task(void);

/* ********************************************************************              */
/* web_kbd_server_main() - web_kbd server task                                       */
/*                                                                                   */
/*    The web_kbd server task is spawned via user request.  It calls                 */
/*    peg_console_task()                                                             */
/*                                                                                   */
/*    This task never returns.                                                       */
/*                                                                                   */

void web_kbd_server_main(void)
{
    tm_kbhit();                 /* Forces keyboard driver to initialize */
    /* call RTIP routine to release any entries in user table; also      */
    /* some kernels require you to kill yourself if you are a task       */
    /* which is taken care of by os_exit_task                            */
    peg_console_task();
    os_exit_task();
}
#endif      /* INCLUDE_WEB_BROWSER && !SMXNET */


#if (INCLUDE_FTP_SRV)
/* ********************************************************************   */
/* ftp_server_main() - ftp server task                                    */
/*                                                                        */
/*    The ftp server task is spawned via user request.  It processes      */
/*    ftp requests sent from a remote host.                               */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void ftp_server_main(void)
{
    while (ftp_spawned)     /* set to FALSE if server killed */
    {
        if (ftp_server_daemon() == -1)
        {
            DEBUG_ERROR("ftp server failed", NOVAR, 0, 0);
        }
        else
        {
            DEBUG_ERROR("ftp server finished", NOVAR, 0, 0);
        }
    }
    /* call RTIP routine to release any entries in user table; also   */
    /* some kernels require you to kill yourself if you are a task    */
    /* which is taken care of by os_exit_task                         */
    os_exit_task();
}
#endif      /* INCLUDE_FTP_SRV */

#if (INCLUDE_TFTP_SRV)
/* ********************************************************************   */
/* tftp_server_main() - tftp server task                                  */
/*                                                                        */
/*    The tftp server task is spawned via user request.  It processes     */
/*    tftp requests sent from a remote host.                              */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void tftp_server_main(void)
{
    while (tftp_spawned)        /* set to FALSE if server killed */
    {
        if (tftp_server_daemon() == -1)
        {
            DEBUG_ERROR("tftp server failed", NOVAR, 0, 0);
        }
        else
        {
            DEBUG_ERROR("tftp server finished", NOVAR, 0, 0);
        }
    }
}
#endif      /* INCLUDE_TFTP_SRV */

#if (INCLUDE_TELNET_SRV)
/* ********************************************************************   */
/* telnet_server_main() - telnet server task                              */
/*                                                                        */
/*    The telnet server task is spawned via user request.  It processes   */
/*    telnet requests sent from a remote host by echoing them back.  A    */
/*    command interpreter would need to be added for the application.     */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void telnet_server_main(void)
{
    while (telnet_spawned)      /* set to FALSE if server killed */
    {
        if (telnet_server_daemon() == -1)
        {
            DEBUG_ERROR("telnet server failed", NOVAR, 0, 0);
        }
        else
        {
            DEBUG_ERROR("telnet server finished", NOVAR, 0, 0);
        }
    }
}
#endif      /* INCLUDE_TELNET */

#if (INCLUDE_WEB)
/* ********************************************************************   */
/* web_server_main() - web server task                                    */
/*                                                                        */
/*    The web server task is spawned via user request.  It processes      */
/*    web requests sent from a remote host.                               */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void web_server_main(void)
{
    while (web_spawned)     /* set to FALSE if server killed */
    {
        if (http_server_daemon() == -1)
        {
            DEBUG_ERROR("HTTP server failed!", NOVAR, 0, 0);
            ks_sleep(ks_ticks_p_sec());
        }
        else
        {
            DEBUG_ERROR("HTTP server finished", NOVAR, 0, 0);
        }
    }
}


#if (INCLUDE_HTTPS_SRV)
/* ********************************************************************   */
/* https_server_main() - secure web server task                           */
/*                                                                        */
/*    The secure web server task is spawned via user request.  It         */
/*    processes web requests sent from a remote host.                     */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void https_server_main(void)
{
    while (https_spawned)       /* set to FALSE if server killed */
    {
        if (https_server_daemon() == -1)
        {
            DEBUG_ERROR("HTTPS server failed!", NOVAR, 0, 0);
            ks_sleep(ks_ticks_p_sec());
        }
        else
        {
            DEBUG_ERROR("HTTPS server finished", NOVAR, 0, 0);
        }
    }
}
#endif  /* INCLUDE_HTTPS_SRV */
#endif  /* INCLUDE_WEB */

#if (INCLUDE_NFS_SRV)
/* ********************************************************************   */
/* nfss_server_main() - nfs server task                                   */
/*                                                                        */
/*    The nfs server task is spawned via user request.  It processes      */
/*    nfs requests sent from a remote host.                               */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void nfss_server_main(void)
{
byte ip_addr[4] = {192, 168, 0, 10};
#if (INCLUDE_MFS)
VSTAT stat;
#endif

#if (INCLUDE_MFS)
    /* make dir for testing purposes, i.e. nfsdemo expects \users
       and we do not have a file system */
    DEBUG_ERROR("NFS SERVER: call mkdir users", NOVAR, 0, 0);

    if (!vf_mkdir("\\users"))
    {
        tm_puts("nfss_server_main: mkdir failed");
    }

    if (!VF_IS_DIR("\\users", &stat))
    {
        tm_puts("nfss_server_main: users is not a dir");
    }
#endif

    /* initialize NFS server   */
#if (TEST_NFS_UNIX_AUTH)
    /* force NFS client to use UNIX authentication     */
    ns_init(AUTH_UNIX);
    ns_add_unix_user(2ul, 4ul, "etcbin.com");
#else
    ns_init(AUTH_NONE);
#endif

    /* allow all dirs to be mounted   */
    ns_add_export(0, ip_nulladdr);
/*  ns_add_export("\\users", ip_addr);   */

    for(;;)
    {
        if (ns_daemon() == -1)
        {
            DEBUG_ERROR("NFS server failed!", NOVAR, 0, 0);
            ks_sleep(ks_ticks_p_sec());
        }
        else
        {
            DEBUG_ERROR("NFS server finished", NOVAR, 0, 0);
        }
    }
}

#endif


#if (INCLUDE_DHCP_SRV)
/* ********************************************************************   */
char  my_dom_name[14] = "ebsnetinc.com";
byte  dhcps_gw_ip[IP_ALEN]  = {192, 168, 0, 2};
byte  dhcps_mask[IP_ALEN]   = {255, 255, 0, 0};
dword dhcps_mdrs = 4000;
dword dhcps_ip_ttl = 60;

struct dhcps_option my_extra_options[] =
{
    {DOMAIN_NAME,    11,      (PFVOID)my_dom_name},
    {ROUTER_OPTION,  IP_ALEN, (PFVOID)dhcps_gw_ip},
    {MDRS,           2,       (PFVOID)&dhcps_mdrs},     /* max frag size */
    {DEFAULT_IP_TTL, 1,       (PFVOID)&dhcps_ip_ttl},
    {SUBNET_MASK,    IP_ALEN, (PFVOID)dhcps_mask},
};

#define NUM_EXTRA_OPTIONS \
    (sizeof(my_extra_options)/sizeof(struct dhcps_option))

struct dhcps_params dhcps_parms =
{
    {255,255,255,0},                    /* subnet mask */
    {192,168,0,1},                      /* default gateway */
    { {192,168,0,5}, {192,168,0,1} },   /* DNS servers */
    my_extra_options,NUM_EXTRA_OPTIONS
};

struct dhcps_config _dhcps_cfg =
{
    {   /* public address pool (start, end, lease time) */
        { {192,168,0,200},{192,168,0,205}, 10000 },
        { {192,168,0,220},{192,168,0,225}, 20000 },
        { {192,168,0,230},{192,168,0,230}, 30000 }
    },
    &dhcps_parms,
    (struct dhcps_client_profile *)0,
    0
};

/* ********************************************************************   */
/* dhcps_server_main() - DHCP server task                                 */
/*                                                                        */
/*    The DHCP server task is spawned via user request.  It processes     */
/*    DHCP requests sent from a remote host.                              */
/*                                                                        */
/*    This task never returns.                                            */
/*                                                                        */

void dhcps_server_main(void)
{
    for(;;)
    {
        if (xn_dhcp_server_daemon(&_dhcps_cfg) == -1)
        {
            DEBUG_ERROR("DHCP server failed - errno = ", EBS_INT1,
                xn_getlasterror(), 0);
            ks_sleep(ks_ticks_p_sec());
        }
        else
        {
            DEBUG_ERROR("DHCP server finished", NOVAR, 0, 0);
        }
    }
}
#endif

/* ********************************************************************   */
/* SNMP ROUTINES                                                          */
#if (INCLUDE_SNMP)

void do_snmp_init(PFBYTE ip_trap_address)
{
struct snmp_config_info snmp_info;

/* vender authoritative id within enterprise mib   */
struct oid enter = { {YOUR_ENTERPRISE_OID}, YOUR_ENTERPRISE_OID_SIZ};
    /* you should always make sure that your data is nulled when
     * not completely initialized otherwise before use
     */
    tc_memset(&snmp_info, 0, sizeof(snmp_info));

    tc_strcpy(snmp_info.snmp_version, "PC, DOS 6.3, RTIP 3.0");
    tc_strcpy(snmp_info.snmp_contact, "Joe Smith (508)448-9340");
    tc_strcpy(snmp_info.snmp_location, "broom closet, 3rd floor");
    tc_strcpy(snmp_info.snmp_sysname, "joe@etcbin.com");
    snmp_info.num_community_get = 1;
    tc_strcpy((PFCHAR)(&snmp_info.community_get[0][0]), (PFCHAR)"public");
    snmp_info.num_community_set = 1;
    tc_strcpy((PFCHAR)(&snmp_info.community_set[0][0]), (PFCHAR)"private");
    tc_strcpy(snmp_info.community_out_trap, "private-traps");

    /* system services - 1 bit is set as follows for each service provided:
     0x40 - applications (e.g., FTP etc)
     0x08 - end-to-end (e.g., IP hosts)
     0x04 - internet (e.g., IP gateways) - NI
     0x02 - datalink/subnetwork (e.g., bridges) - NI
     0x01 - physical (e.g., repeaters) - NI */
    snmp_info.snmp_services = 0x48;

    /* vender authoritative id within enterprise mib   */
    OIDCPY(snmp_info.snmp_sys_objid, enter);

    /* if TRAP address entered (if not all ffs)   */
    if (!tc_cmp4(ip_trap_address, ip_ffaddr, 4))
        snmp_info.num_trap_managers = 1;
    else
        snmp_info.num_trap_managers = 0;
    tc_mv4((PFBYTE)&(snmp_info.trap_manager[0][0]),
           ip_trap_address, IP_ALEN);
#if (INCLUDE_SNMPV2)
    tm_puts("Configuring SNMP V2");
#endif

    if (xn_snmp_config((PSNMPCONFIG)&snmp_info) < 0)
    {
        DEBUG_ERROR("do_snmp_init - xn_snmp_config failed", NOVAR, 0, 0);
    }
}

void snmp_main_(void)
{
/*  for(;;)   */
    {
#if (TEST_LARGE_SNMP_BUF)
        if (xn_snmp_daemon((PFBYTE)snmp_buf, SNMP_LARGE_BUF) == -1)
#else
        if (xn_snmp_daemon((PFBYTE)0, 0) == -1)
#endif
        {
            DEBUG_ERROR("SNMP server failed! - errno = ", STR1,
                xn_geterror_string(xn_getlasterror()), 0);
            DEBUG_ERROR("PORTS UDP", PORTS_UDP, 0, 0);
            ks_sleep(ks_ticks_p_sec());
        }
        else
            DEBUG_ERROR("SNMP server finished", NOVAR, 0, 0);
    }
    /* call RTIP routine to release any entries in user table; also   */
    /* some kernels require you to kill yourself if you are a task    */
    /* which is taken care of by os_exit_task                         */
    os_exit_task();
}
#endif


/* ********************************************************************   */
/* KILL SERVER TASKS                                                      */
/* ********************************************************************   */
#if (INCLUDE_FTP_SRV)
void kill_ftp(void)
{
    if (ftp_spawned)
    {
        DEBUG_ERROR("KILL FTP SERVER", NOVAR, 0, 0);
        ftp_spawned = FALSE;        /* let main ftp server task know */
                                    /* not to recall the server   */
        ftp_kill_server_daemon();

        /* some kernels require you to kill the task from a different   */
        /* task (see osport.h)                                          */
/*      KS_DELETE_TASK(demo_spawn_task[FTP_TASK], DEMO_SPAWN_PRIO)      */
    }
}
#endif

#if (INCLUDE_WEB)
void kill_web(void)
{
    if (web_spawned)
    {
        DEBUG_ERROR("KILL WEB SERVER", NOVAR, 0, 0);
        web_spawned = FALSE;        /* let main web server task know */
                                    /* not to recall the server   */
        http_kill_server_daemon();

        /* some kernels require you to kill the task from a different   */
        /* task (see osport.h)                                          */
/*      KS_DELETE_TASK(demo_spawn_task[WEB_TASK], DEMO_SPAWN_PRIO)      */
    }
}


#if (INCLUDE_HTTPS_SRV)
void kill_https(void)
{
    if (https_spawned)
    {
        DEBUG_ERROR("KILL HTTPS SERVER", NOVAR, 0, 0);
        https_spawned = FALSE;      /* let main web server task know */
                                    /* not to recall the server   */
        https_kill_server_daemon();

        /* some kernels require you to kill the task from a different   */
        /* task (see osport.h)                                          */
/*      KS_DELETE_TASK(demo_spawn_task[HTTPS_TASK], DEMO_SPAWN_PRIO)    */
    }
}
#endif  /* INCLUDE_HTTPS_SRV */
#endif  /* INCLUDE_WEB */

#if (INCLUDE_TELNET_SRV)
void kill_telnet(void)
{
    if (telnet_spawned)
    {
        DEBUG_ERROR("KILL TELNET SERVER", NOVAR, 0, 0);
        telnet_spawned = FALSE;     /* let main telnet server task know */
                                    /* not to recall the server   */

        telnet_kill_server_daemon();

        /* some kernels require you to kill the task from a different   */
        /* task (see osport.h)                                          */
/*      KS_DELETE_TASK(demo_spawn_task[TELNET_TASK], DEMO_SPAWN_PRIO)   */
    }
}
#endif

#if (INCLUDE_SNMP)
void kill_snmp(void)
{
    xn_snmp_kill_server_daemon();
}
#endif

