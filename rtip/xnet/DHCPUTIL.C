/*
   DHCPUTIL.C - DHCP utility functions

   EBS - RTIP

   Copyright Peter Van Oudenaren , 1996
   All rights reserved.
   This code may not be redistributed in source or linkable object form
   without the consent of its author.


    Module description:
    This module provides functions for debugging and testing a DHCP client
    application. The compilation of this code is strictly optional, and
    may be controlled through #defines in xnconf.h.
*/

#define DIAG_SECTION_KERNEL DIAG_SECTION_DHCP


#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif

#if (INCLUDE_DHCP_CLI)

#include "dhcp.h"

/************************************************************   */
/************************** GLOBALS *************************   */
/************************************************************   */

#if (DHCP_DUMP_PACKETS)
char dump_line[80];
char dump_num[40];

byte op_table[NUM_DHCP_OPS]=
{
    0,          /* n/a */
    DWORD_OP,   /*SUBNET_MASK*/
    DWORD_OP,   /*TIME_OFFSET*/
    DWL_OP,     /*ROUTER_OPTION*/
    DWL_OP,     /*TIME_SERVER_OP*/
    DWL_OP,     /*NAME_SERVER_OP*/
    DWL_OP,     /*DNS_OP*/
    DWL_OP,     /*LOG_SERVER_OP*/
    DWL_OP,     /*COOKIE_SERVER_OP*/
    DWL_OP,     /*LPR_SERVER_OPTION*/
    DWL_OP,     /*IMPRESS_SERVER_OP*/
    DWL_OP,     /*RLS_OP*/
    STRING_OP,  /*HOST_NAME_OP*/
    WORD_OP,    /*BFS_OP*/
    STRING_OP,  /*MERIT_DUMP*/
    STRING_OP,  /*DOMAIN_NAME*/
    DWORD_OP,   /*SWAP_SERVER*/
    STRING_OP,  /*ROOT_PATH*/
    STRING_OP,  /*EXTENSIONS_PATH*/
    BOOL_OP,    /*IP_FORWARDING*/
    BOOL_OP,    /*NLSR*/
    DWL_OP,     /*POLICY_FILTER*/
    WORD_OP,    /*MDRS*/
    BYTE_OP,    /*DEFAULT_IP_TTL*/
    DWORD_OP,   /*PATH_MTU_AT*/
    WL_OP,      /*PATH_MTU_PLATEAU*/
    WORD_OP,    /*INTERFACE_MTU*/
    BOOL_OP,    /*ALL_SUBNETS_LOCAL*/
    DWORD_OP,   /*BROADCAST_ADDRESS*/
    BOOL_OP,    /*MASK_DISCOVERY*/
    BOOL_OP,    /*MASK_SUPPLIER*/
    BOOL_OP,    /*ROUTER_DISCOVERY*/
    DWORD_OP,   /*RSA_OP*/
    DWL_OP,     /*STATIC_ROUTE_OP*/
    BOOL_OP,    /*TRAILER_ENCAP_OP*/
    DWORD_OP,   /*ARP_CT_OP*/
    BOOL_OP,    /*EE_OP*/
    BYTE_OP,    /*TCP_DEFAULT_TTL*/
    DWORD_OP,   /*TCP_KA_INTERVAL*/
    BOOL_OP,    /*TCP_KA_GARBAGE*/
    STRING_OP,  /*NISD*/
    DWL_OP,     /*NIS_OP*/
    DWL_OP,     /*NTPS_OP*/
    BL_OP,      /*VENDOR_SPECIFIC*/
    DWL_OP,     /*NET_OVER_TCP_NS*/
    DWL_OP,     /*NET_OVER_TCP_DDS*/
    BYTE_OP,    /*NET_OVER_TCP_NT*/
    BL_OP,      /*NET_OVER_TCP_SCOPE*/
    DWL_OP,     /*XWIN_SFS*/
    DWL_OP,     /*XWIN_SDM*/
    DWORD_OP,   /*REQ_IP*/
    DWORD_OP,   /*IP_LEASE*/
    BYTE_OP,    /*OP_OVERLOAD*/
    BYTE_OP,    /*DHCP_MSG_TYPE*/
    DWORD_OP,   /*SERVER_ID*/
    BL_OP,      /*PARAM_REQ_LST*/
    STRING_OP,  /*MESSAGE*/
    WORD_OP,    /*MAX_DHCP_MSG_SIZE*/
    DWORD_OP,   /*RENEWAL_TIME*/
    DWORD_OP,   /*REBINDING_TIME*/
    STRING_OP,  /*CLASS_ID*/
    BL_OP       /*CLIENT_ID*/
    };
#endif

/************************************************************   */
/********************* FUNCTION BODIES **********************   */
/************************************************************   */


#if (DHCP_DUMP_PACKETS==1)
void dhcp_dump_packet(PFDHCP_packet packet)
{
    DEBUG_LOG("*****",LEVEL_2,NOVAR,0,0);
    dhcp_dump_packet_hdr(packet);
    dhcp_dump_packet_ops(packet);
    DEBUG_LOG("*****",LEVEL_2,NOVAR,0,0);
}

void dhcp_dump_packet_hdr(PFDHCP_packet packet)
{
    DEBUG_LOG("op = ",LEVEL_2,DINT1,(dword)(packet->op),0);
    DEBUG_LOG("htype = ",LEVEL_2,DINT1,(dword)(packet->htype),0);
    DEBUG_LOG("hlen = ",LEVEL_2,DINT1,(dword)(packet->hlen),0);
    DEBUG_LOG("hops = ",LEVEL_2,DINT1,(dword)(packet->hops),0);
    DEBUG_LOG("xid = ",LEVEL_2,DINT1,(dword)(packet->xid),0);
    DEBUG_LOG("secs = ",LEVEL_2,DINT1,(dword)(packet->secs),0);
    DEBUG_LOG("flags = ",LEVEL_2,DINT1,(dword)(packet->flags),0);

    db_out_ip_addr("ciaddr = ",(packet->ciaddr));
    db_out_ip_addr("yiaddr = ",(packet->yiaddr));
    db_out_ip_addr("siaddr = ",(packet->siaddr));
    db_out_ip_addr("giaddr = ",(packet->giaddr));
}

void dhcp_dump_packet_ops(PFDHCP_packet packet)
{
word n;
word len;
word start_op,end_op;
word op_type;
char op_string[DHCP_PKT_OP_SIZE*5];
char op_out_str[DHCP_PKT_OP_SIZE*5+30];
char op_id[4];
char op_len[4];
char cat_string[5];

    n=4;

    do
    {
        tc_strcpy(op_string,"");
        tc_strcpy(op_id,"");
        tc_strcpy(op_len,"");
        tc_strcpy(op_out_str,"");

        tc_ultoa((dword)packet->options[n],op_id,10);
        if (packet->options[n]==END)
        {
            break;
        }
        op_type=op_table[ packet->options[n] ];
        n++;
        len=packet->options[n];

        tc_ultoa((dword)packet->options[n],op_len,10);

        start_op=n+1;
        end_op=n+len;

        for (n=start_op;n<=end_op;n++)
        {
            if (op_type==STRING_OP)
            {
                cat_string[0]=packet->options[n];
                cat_string[1]='\0';
                tc_strcat(op_string,cat_string);
            }
            else
            {
                tc_ultoa((dword)packet->options[n],cat_string,10);
                tc_strcat(op_string,cat_string);
                tc_strcat(op_string,"|");
            }
        }
        /* n now is equal to end_op+1   */
        tc_strcpy(op_out_str,"op_id=");
        tc_strcat(op_out_str,op_id);
        tc_strcat(op_out_str," op_len=");
        tc_strcat(op_out_str,op_len);
        tc_strcat(op_out_str," op=");
        tc_strcat(op_out_str,op_string);

        DEBUG_LOG(op_out_str,LEVEL_2,NOVAR,0,0);
    } while(n<DHCP_PKT_OP_SIZE);
}

void db_out_ip_addr(char *line_start,dword num)
{
    /* assumes that num is in network byte order.   */
    tc_strcpy(dump_line,line_start);
    tc_itoa((word)(byte)(num),dump_num,10);
    tc_strcat(dump_line,dump_num);
    tc_strcat(dump_line,".");
    tc_itoa((word)(byte)(num>>8),dump_num,10);
    tc_strcat(dump_line,dump_num);
    tc_strcat(dump_line,".");
    tc_itoa((word)(byte)(num>>16),dump_num,10);
    tc_strcat(dump_line,dump_num);
    tc_strcat(dump_line,".");
    tc_itoa((word)(byte)(num>>24),dump_num,10);
    tc_strcat(dump_line,dump_num);

    DEBUG_LOG(dump_line,LEVEL_2,NOVAR,0,0);
    dump_line[0]='\0';
    dump_num[0]='\0';
}


#endif
#endif

