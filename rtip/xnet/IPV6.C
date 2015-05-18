/*                                                                        */
/* IP.C - IP, RARP and ARP interpreter                                    */
/*                                                                        */
/* EBS - RTIP                                                             */
/*                                                                        */
/* Copyright Peter Van Oudenaren , 1993                                   */
/* All rights reserved.                                                   */
/* This code may not be redistributed in source or linkable object form   */
/* without the consent of its author.                                     */
/*                                                                        */
/*  Module description:                                                   */
/*     This module contains code to handle the IP layer.  It includes     */
/*     code to interpret and dispatch incoming IP packets, to             */
/*     handle IP fragmentation/packet reassembly and to perform           */
/*     simple router capablilities.                                       */
/*     NOTE: fragmentation and router code inclusion is controlled by     */
/*           INCLUDE_FRAG and INCLUDE_ROUTER                              */
/*                                                                        */
/* Routines in this module                                                */
/*                                                                        */
/* tc_ip_interpret()     - Take incoming ip packets and dispatch them to  */
/*                         the appropriate handlers. (IP, UDP, ICMP)      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"


