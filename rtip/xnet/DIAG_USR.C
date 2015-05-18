/*                                                                              */
/* DIAG_USR.C - log/diagnostics/debug/error dispatch function: user replacable. */
/*                                                                              */
/*                                                                              */
/*   EBS - RTIP                                                                 */
/*                                                                              */
/*   Copyright Peter Van Oudenaren , 1993                                       */
/*   All rights reserved.                                                       */
/*   This code may not be redistributed in source or linkable object form       */
/*   without the consent of its author.                                         */
/*                                                                              */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DIAGNOSTICS

#include    "sock.h"

#if (CFG_USE_DIAG_MSG)

int diag_debug_printf_write(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen)
{
    if (in_irq > 0)
    {
        return directdebug_printf_write(io, msg, msglen);
    }
    /*    return directdebug_printf_write(io, msg, msglen);   */
    
    /*
    * This routine decides if the diagnostic lines get put through to syslog() or not.
    * Some sections inside the RTIP TCP/IP stack are off-limits for syslog() traffic:
    *
    * - UDP traffic debugging CANNOT generate additional UDP traffic to report status, etc.
    *
    * - INTERRUPT ROUTINES cannot create UDP/any messages
    *
    * - ...
    *
    * Return:
    *  0: everything OK
    *  1: no remote syslogging allowed
    *  2: no semaphores, remote syslogging, etc. allowed (INSIDE INTERRUPT HANDLER!)
    *  3: IGNORE --> discard message
    */
    
    /* UDP section? Get out!   */
    switch (io->propagate.ivalue) /* section */
    {
    case DIAG_SECTION_CRYPTTEST:
    case DIAG_SECTION_CRYPTOLIB:
    case DIAG_SECTION_OPENSSL:
    case DIAG_SECTION_USER:
    case DIAG_SECTION_SNMP:
    case DIAG_SECTION_MEMSTATDUMP:
#if (INCLUDE_SYSLOG)
        /*    io->channel.syslog.option_and_mask &= ~LOG_CONS; // don't log to console...   */
        return xn_write2syslog(io, msg, msglen);
#else
        return directdebug_printf_write(io, msg, msglen);
#endif

    /* NFS/VFS: NFS&VFS are used to access filesystems locally and/or remotely.   */
    /* make sure NFS/VFS-generated syslog traffic doesn't end up requiring these  */
    /* very same services it is trying to log about.                              */
    case DIAG_SECTION_NFS:                                   
    case DIAG_SECTION_VFS:
#if (INCLUDE_SYSLOG)
        io->channel.syslog.option_and_mask &= ~(LOG_LOCALFILE | LOG_REMOTE);
        /* don't log to filesystem or remote: might cause recursive calls to syslog...   */
        return xn_write2syslog(io, msg, msglen);
#else
        return directdebug_printf_write(io, msg, msglen);
#endif


    /* ERTFS/MFS: ERTFS/MFS are used to access filesystems locally.                */
    /* make sure ERTFS/MFS-generated syslog traffic doesn't end up requiring these */
    /* very same services it is trying to log about.                               */
    case DIAG_SECTION_ERTFS:
    case DIAG_SECTION_MFS:
#if (INCLUDE_SYSLOG)
        io->channel.syslog.option_and_mask &= ~LOG_LOCALFILE;
        /* don't log to filesystem: might cause recursive calls to syslog...   */
        return xn_write2syslog(io, msg, msglen);
#else
        return directdebug_printf_write(io, msg, msglen);
#endif
    
    case DIAG_SECTION_DIRECTDIAG:
        return directdebug_printf_write(io, msg, msglen);

    /* this might be logging from inside an interrupt handler.                     */
    /* Don't try to use regular mechanisms unless you know what you are doing.     */
    case DIAG_SECTION_DRIVER:
#if 01
        return msglen; 
#else
        return directdebug_printf_write(io, msg, msglen);
#endif
    
    case DIAG_SECTION_DIAGNOSTICS:
        return directdebug_printf_write(io, msg, msglen);
    
    case DIAG_SECTION_API:
    case DIAG_SECTION_UDP:
    case DIAG_SECTION_IP:
    case DIAG_SECTION_ARP:
    case DIAG_SECTION_ICMP:
    case DIAG_SECTION_OS:
    /* it is very dangerous to have UDP/IP/OS log through syslog.   */
    /*
     * A scenario that actually took place:
     *
     * UDP message is being sent and diagnostic logging through syslog results. Just before
     * waiting for the semaphore, the SNMP task diagnostic syslogging grabs the semaphore and
     * writes a UDP message, resulting in a 'sendto()' lockup, as the first 'tranmitting'
     * diagnotic mentioned above originated from within the sendto() UDP critical sections.
     *
     * This type of lockup can only be prevented by NOT using the xn_write2syslog() call
     * for anything within the syslog UDP path.
     *
     * NOTE: to be absolutely sure, ARP has been included in this group too, as the message
     * will produce an ARP lookup too. If the host is active but its syslog deamon is down,
     * this can be reported through ICMP messages under certain circumstances.
     */
#if (01 || !INCLUDE_SYSLOG)
        /* [i_a] my choice while debugging SSL; reset to directdebug when done.   */
        /*return directdebug_printf_write(io, msg, msglen);                       */
        return msglen; 
#else
        io->channel.syslog.option_and_mask &= ~LOG_REMOTE;
        return xn_write2syslog(io, msg, msglen);
#endif
    
    default:
    case DIAG_SECTION_UNKNOWN:
#if (INCLUDE_SYSLOG)
        /*    io->channel.syslog.option_and_mask &= ~LOG_REMOTE;   */
        return xn_write2syslog(io, msg, msglen);
#else
        return directdebug_printf_write(io, msg, msglen);
#endif    
    
    
    
    /*********************************************
     *                                           *
     *     customer specific additions...        *
     *                                           *
     *********************************************/

    /* [i_a] while debugging SSL & file system access I didn't want any elaborate   */
    /* network stack logging I wasn't interested in.                                */
    case DIAG_SECTION_PPP:
    case DIAG_SECTION_TCP:
    case DIAG_SECTION_RARP:
    case DIAG_SECTION_RIP:
    case DIAG_SECTION_DNS:
    case DIAG_SECTION_MAIL:
    case DIAG_SECTION_IGMP:
        return msglen; 
    }
}


#endif /* (CFG_USE_DIAG_MSG) */

