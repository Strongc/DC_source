/*                                                                      */
/* TFTPCONS.C - TFTP CONSTANT GLOBAL DATA                               */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains all the global data for TFTP               */
/*      which is never modified.                                        */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TFTP


#include "sock.h"

#if (INCLUDE_TFTP_CLI || INCLUDE_TFTP_SRV)
    #include "tftp.h"

/* ********************************************************************   */
/* TFTP CLIENT and SERVER                                                 */
/* ********************************************************************   */

KS_GLOBAL_CONSTANT struct errmsg KS_FAR errmsgs[9] = 
{
    { EIOERROR, "IO Error on file" },
    { ENOTFOUND,"File not found" },
    { EACCESS,  "Access violation" },
    { ENOSPACE, "Disk full or allocation exceeded" },
    { EBADOP,   "Illegal TFTP operation" },
    { EBADID,   "Unknown transfer ID" },
    { EEXISTS,  "File already exists" },
    { ENOUSER,  "No such user" },
    { -1,       "Undefined error code" }
};

#if (INCLUDE_TFTP_CLI)
KS_GLOBAL_CONSTANT char KS_FAR file_system_error[18] = "FILE SYSTEM ERROR";
KS_GLOBAL_CONSTANT char KS_FAR KS_FAR *netascii = "netascii";
#endif

#if (INCLUDE_TFTP_SRV)
int validate_access(char *filename, int mode);
int sendfile(KS_CONSTANT struct formats KS_FAR *pf);  
int recvfile(KS_CONSTANT struct formats KS_FAR *pf);  

KS_GLOBAL_CONSTANT struct formats KS_FAR tftpd_formats[] = 
{
    { "netascii", validate_access, sendfile, recvfile, 1 },
    { "octet",    validate_access, sendfile, recvfile, 0 },
    { 0 }
};
#endif      /* INCLUDE_TFTP_SRV */
#endif      /* INCLUDE_TFTP_CLI or INCLUDE_TFTP_SRV */

