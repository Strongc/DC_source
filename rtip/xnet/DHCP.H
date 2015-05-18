/*
   DHCP.H  - DHCP Client header file

   EBS - RTIP

   Copyright Peter Van Oudenaren , 1996
   All rights reserved.
   This code may not be redistributed in source or linkable object form
   without the consent of its author.

   Module description:
    This file contains constants, structures, and function prototypes
    for use in DHCP client and server applications.
*/

#ifndef __DHCP_H__
#define __DHCP_H__

#include "rtip.h"
#include "socket.h"
#if (INCLUDE_DHCP_CLI)
#include "dhcpcapi.h"
#endif

/************************************************************   */
/************************* CONSTANTS ************************   */
/************************************************************   */

/* infinate lease time   */
#define DHCP_INF_LEASE 0xffffffffl

/* set to 1 to send extend lease to server's IP address       */
/* NOTE: renew will send to server's IP and rebind will send  */
/*       to broadcast                                         */
/* NOTE: setting it to 1 is RFC compliant but not all servers */
/*       will respond to and extend address sent directly to  */
/*       it's IP address                                      */
#define DHCP_RENEW_RFC_NETADDR 1

#define ONE_MINUTE 60       /* number of seconds in a minute */

/********** DHCP RUNTIME BEHAVIOR CONTROL CONSTANTS *********  */
#define DHCP_DUMP_PACKETS               0
#define DHCP_TRACE                      0

/********** CONSTANTS FOR 'htype' FIELD IN DHCP_conf *********  */
#define ETHERNET_10MB 1     /* 10 Megabyte Ethernet */
#define ETHERNET_3MB  2     /* 3 MB Experimental Ethernet */
#define AMATEUR_RADIO 3     /* Amateur Radio AX.25 */
#define TOKEN_RING    4     /* Proteon ProNET Token Ring */
#define CHAOS         5     /* Chaos */
#define IEEE_802      6     /* IEEE 802 Networks */
#define ARCNET        7     /* ARCNET */
#define HYPERCHANNEL  8     /* Hyperchannel */
#define LANSTAR       9     /* Lanstar */
#define AUTONET      10     /* Autonet Short Address */
#define LOCALTALK    11     /* LocalTalk */
#define LOCALNET     12     /* LocalNet (IBM PCNet or SYTEK LocalNET) */

/********** CONSTANTS FOR 'prio' FIELD IN DHCP_param *********  */
/* normal values are 0-99                                       */
#define DHCPP_MUST_HAVE  100

/********** DHCP ARRAY SIZE CONSTANTS *********  */
#define NUM_DHCP_OPS           62
#define DHCP_STD_PARAMS         7

/********** PORT CONSTANTS *********  */
#define DHCP_SERVER_PORT 67     /* messages from client to server */
#define DHCP_CLIENT_PORT 68     /* messages from server to client */

/********** DHCP MESSAGE TYPE CONSTANTS *********  */
#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7

/* Maskable constants for message type   */
#define DHCPDISCOVER_MC (0x01)
#define DHCPOFFER_MC    (0x02)
#define DHCPREQUEST_MC  (0x04)
#define DHCPDECLINE_MC  (0x08)
#define DHCPACK_MC      (0x10)
#define DHCPNAK_MC      (0x20)
#define DHCPRELEASE_MC  (0x40)

/* constant used in creation of dhcp_msg_masks table   */
#define DHCP_MSG_TYPES 7

/********** CONSTANTS FOR 'OP' FIELD IN DHCP MESSAGE *********  */
#define BOOTREQUEST 1
#define BOOTREPLY   2

/********** CONSTANTS FOR 'overload_type' IN struct DHCP_message ********  */
#define NO_OVERLOAD             0
#define INTO_FILE               1
#define INTO_SNAME              2
#define INTO_BOTH               3

/* default style if not specified by api call   */
#define DHCP_DEF_PKT_STYLE DHCP_MICROSOFT
/* number of styles possible (DHCP_RFC and DHCP_MICROSOFT)   */
#define DHCP_REQ_STYLES 2

/********** CONSTANTS for return value of is_dhcp_response_valid() *********  */
#define DHCP_INVALID    (-1)
#define DHCP_VALID      (0)
#define DHCP_ACCEPTABLE (1)

/********** CONSTANTS FOR OPTION TABLE (used in debugging) *********  */

#if(DHCP_DUMP_PACKETS==1)
    #define BOOL_OP         1
    #define BYTE_OP         2
    #define WORD_OP         3
    #define DWORD_OP        4
    #define STRING_OP       5
    #define BL_OP           6   /* byte list (for CLIENT_ID etc) */
    #define WL_OP           7
    #define DWL_OP          8
#endif

#if (INCLUDE_DHCP_CLI)
/************************************************************   */
/******************** FUNCTION PROTOTYPES *******************   */
/************************************************************   */

/********** DHCP CLIENT INTERNAL FUNCTIONS (DHCPPROC.C) *********  */
int  extend_dhcp_lease_start(PFDHCP_session session, dword lease_time);
void process_extend_results(PFDHCP_session session, int dhcpc_sock);
void apply_dhcp_std_ops(PFDHCP_session session, int iface_no);
void get_dhcp_params(PFDHCP_packet ack,PFDHCP_std_params params);
int  open_dhcp_socket(int iface_no);
void init_dhcp_packet(PFDHCP_packet packet);
int  format_dhcp_discinf(PFDHCP_packet packet,int iface_no,PFDHCP_conf conf,
                         byte msg_type);
int  format_dhcp_discover(PFDHCP_packet packet,int iface_no,PFDHCP_conf conf);
int  format_dhcp_inform(PFDHCP_packet packet,int iface_no,PFDHCP_conf conf);
void format_dhcp_request(PFDHCP_packet discover,PFDHCP_packet offer,
                         int style);
void format_dhcp_extend(PFDHCP_packet prev_req, dword ciaddr,dword lease_time);
void format_dhcp_decrel(PFDHCP_packet packet,PFBYTE server_ip,
                        PFBYTE client_ip,byte msg_type);

int  do_dhcp_packet(PFDHCP_packet outmsg,PFDHCP_packet replies,int dhcpc_sock,
                    PFCBYTE ip_addr,int max_replies,int timeout,int max_retries,
                    byte ok_msg_types);

int  is_dhcp_response_valid(PFDHCP_packet client_pkt, PFDHCP_packet serv_resp);

int  dhcp_choose_offer(PFDHCP_packet offers,int num_offers,PFDHCP_conf conf);

PFBYTE map_dhcp_op(PFDHCP_packet packet,int index);

RTIP_BOOLEAN is_dhcp_op_there(PFDHCP_packet packet, byte op_id,
                         PFINT pindex,PFINT plen);

int  set_dhcp_op(PFDHCP_packet packet,byte op_id,byte len,PFVOID vpdata);
int  get_dhcp_op(PFDHCP_packet packet,byte op_id,byte bufsize,PFVOID vpdest);
int  remove_dhcp_op(PFDHCP_packet packet,byte op_id);

int  recv_dhcp_packet(PFDHCP_packet recv_pkt,int dhcpc_sock,
                      long timeout_in_ticks);
int  send_dhcp_packet(PFDHCP_packet send_pkt,int dhcpc_sock,PFCBYTE ip_addr);

/********** DHCP CLIENT DEBUG UTILITY FUNCTIONS (DHCPUTIL.C) *********  */

#if (DHCP_DUMP_PACKETS==1)
void dhcp_dump_packet(PFDHCP_packet packet);
void dhcp_dump_packet_hdr(PFDHCP_packet packet);
void dhcp_dump_packet_ops(PFDHCP_packet packet);
void db_out_ip_addr(char *line_start,dword num);
#endif
#endif /* INCLUDE_DHCP_CLI */

#endif

