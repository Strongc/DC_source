/*                                                                      */
/* OS.C - functions which interface to the porting layer                */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module consists of all the interface routines to the       */
/*      operating systems porting layer                                 */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
#include "os.h"
#if (INCLUDE_BGET)
#include "bget.h"
#endif
#if (INCLUDE_MALLOC_DCU_INIT )
#include <malloc.h>
#endif
#include <limits.h>     /* LONG_MAX definition */

// XHM: Added 15/11 2006 to power down on mem corruption
#include <PowerDown.h>
#include <ErrorLog.h>

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
/* The following 3 are related to INCLUDE_TRK_PKT                         */
/* NOTE: warning if TRACK_WAIT or LOW_WAIT are on, when more that a       */
/*       screen of information is displayed, terminal input is required   */
/*       to continue                                                      */
#define TRACK_WAIT 0    /* wait in track packet for return to be  */
                        /* entered; uses tm_getch (see terminal.c)   */
#define LOW_WAIT   0    /* wait in display_packet_lowwater for return  */
                        /* to be entered;                  */
                        /* useds tm_getch (see terminal.c) */
#define DEBUG_TRACK 1   /* call routine to track packets if cannot */
                        /* allocate a packet; this will display a report   */
                        /* (via DEBUG_ERROR) showing what exchanges, lists */
                        /* etc the DCUs are on                             */
                        /* NOTE: INCLUDE_TRK_PKTS in xnconf.h must also    */
                        /*       be on                                     */

#define DEBUG_DCU  0    /* tracks calls to alloc and free DCUs */
#define DEBUG_FREE 1    /* turns on code to check if freeing an already */
                        /* free packet; turn on and set breakpoint at   */
                        /* break_it()                                   */
#define DEBUG_FREE_TCP_WIN 0 /* check if freeing DCU on TCP window - NI */
                        /* DEBUG_FREE must also be 1   */
#define DISPLAY_DCU 0   /* turn on to display addresses of DCUs alloced */
                        /* and freed   */
#define DEBUG_IFACE_SEM 0 /* display claim and release of IFACE_SEM */
                        /* INCLUDE_TRK_PKTS must be on   */
#define DEBUG_LIST 1    /* calls to list offset routines are checked before */
                        /* being performed (i.e. makes sure when adding   */
                        /* an entry to a list it is not already on the    */
                        /* list etc, etc); when this is turned on set     */
                        /* a breakpoint at break_list() to stop at any    */
                        /* errors                                         */
#define DEBUG_IP_LIST 0
#define DEBUG_SELECT  0

#define DEBUG_MEMSTATS  0   /* track and dumps memory allocation requests */

#if (INCLUDE_TRK_PKTS && DEBUG_IP_LIST)
#error: INCLUDE_TRK_PKTS must be on for DEBUG_IP_LIST
#endif

/* ********************************************************************   */
/* DEBUG FUNCTIONS and VARIABLES                                          */
/* ********************************************************************   */
#if (DEBUG_FREE && !INCLUDE_MALLOC_DCU_AS_NEEDED)
RTIP_BOOLEAN check_free(DCU free_msg);
#endif
#if (DEBUG_FREE_TCP_WIN && !INCLUDE_MALLOC_DCU_AS_NEEDED)
RTIP_BOOLEAN check_tcp_window(DCU free_msg);
#endif
#if (DEBUG_FREE || DEBUG_FREE_TCP_WIN)
void break_it(void);
#endif

#if (DEBUG_DCU)
long n_alloc_calls = 0;
long n_free_calls = 0;
#endif

/* ********************************************************************   */
#define KS_MALLOC_PKT(A, B, C)  \
    ks_malloc((A)+CFG_PACKET_ADJ+PKT_GUARD_SIZE, B, C)
#define KS_FREE_PKT(A, B, C)    \
    ks_free((A)+CFG_PACKET_ADJ+PKT_GUARD_SIZE, B, C)

/* ********************************************************************   */
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
void os_init_packet_pools(void);
void os_init_freelist_entry(int index, PFBYTE pcore, int num_packets, int packet_size);
#endif

/* ********************************************************************   */
/* FUNCTION DECLARATIONS                                                  */

#if (DEBUG_LIST)
void break_list(void);
RTIP_BOOLEAN os_check_list_off(POS_LIST head, POS_LIST entry, int offset);
#endif

/* *********************************************************************   */
/* LIST MANAGEMENT (with offset)                                           */
/*                                                                         */
/* List management code. ADD FRONT, ADD REAR, REMOVE                       */
/*   - lists are doubly linked                                             */
/*   - the lists are circular, i.e. the first entry's prev pointer points  */
/*     to the last entry and the last entry's next pointer points to the   */
/*     head of the list                                                    */
/* ********************************************************************    */

/* ********************************************************************   */
/* os_list_add_front_off() - add an entry to the front of list            */
/*                                                                        */
/*   This function adds the entry (entry) to the front of the doubly list */
/*   list (head).                                                         */
/*                                                                        */
/*   Returns the new head of the list.                                    */
/*                                                                        */

POS_LIST os_list_add_front_off(POS_LIST head, POS_LIST entry, int offset)    /*__fn__*/
{
POS_LIST entry_info;
POS_LIST head_info;

#if (DEBUG_LIST)
    if (os_check_list_off(head, entry, offset))
    {
        break_list();
        return(head);
    }
#endif

    entry_info = POS_ENTRY_OFF(entry, offset);
    head_info  = POS_ENTRY_OFF(head, offset);
    if (head)
    { 
        entry_info->pnext = head;           /* point to the prev head  */
                                            /* of list   */
        entry_info->pprev = head_info->pprev;   /* point to the last entry  */
                                            /* in list   */
        POS_ENTRY_OFF(head_info->pprev, offset)->pnext = entry;         
                                            /* the last entry in the    */
                                            /* list needs to point      */
                                            /* to the new head          */
        head_info->pprev = entry;           /* the old head needs to  */
                                            /* point to new head   */
    }
    else        /* list was empty */
    { 
        entry_info->pnext = entry; 
        entry_info->pprev = entry; 
    }
    return(entry);    
}

/* ********************************************************************   */
/* os_list_add_rear_off() - add an entry to the end of list               */
/*                                                                        */
/*   This function adds the entry (entry) to the end of the doubly list   */
/*   list (head).                                                         */
/*                                                                        */
/*   Returns the head of the list.                                        */
/*                                                                        */

POS_LIST os_list_add_rear_off(POS_LIST head, POS_LIST entry, int offset) /*__fn__*/
{
POS_LIST entry_info;
POS_LIST head_info;

#if (DEBUG_LIST)
    if (os_check_list_off(head, entry, offset))
    {
        break_list();
        return(head);
    }
#endif

    entry_info = POS_ENTRY_OFF(entry, offset);
    head_info  = POS_ENTRY_OFF(head, offset);

    if (!head)      /* if empty list */
        return(os_list_add_front_off(head, entry, offset));
    else
    { 
        entry_info->pprev = head_info->pprev;   /* point to the old end of  */
                                            /* the list   */
        entry_info->pnext = head;           /* the end needs to point to  */
                                            /* the beginning   */
        POS_ENTRY_OFF(head_info->pprev, offset)->pnext = entry;         
                                            /* the old end needs to point    */
                                            /* to new end                    */
        head_info->pprev = entry;           /* the first needs to point  */
                                            /* to new end   */
        return(head);
    }
}

/* ********************************************************************   */
/* os_list_add_middle_off() - add an entry after an specified entry       */
/*                                                                        */
/*   This function adds the entry (entry) after entry_prev in list where  */
/*   list head is head.                                                   */
/*                                                                        */
/*   Returns the new head of the list.                                    */
/*                                                                        */

POS_LIST os_list_add_middle_off(POS_LIST head, POS_LIST entry, 
                                POS_LIST entry_prev, int offset)    /*__fn__*/
{
POS_LIST entry_info;
POS_LIST entry_prev_info;

#if (DEBUG_LIST)
    if (os_check_list_off(head, entry, offset))
    {
        break_list();
        return(head);
    }
#endif

    entry_info = POS_ENTRY_OFF(entry, offset);
    entry_prev_info = (POS_LIST)0;  /* keep compiler happy */

    /* if not adding to end of list   */
    if (entry_prev)
        entry_prev_info = POS_ENTRY_OFF(entry_prev, offset);

    /* add to the middle where list is not empty and not adding to   */
    /* end of list, i.e. after entry_prev                            */
   if (head && entry_prev)
   { 
        /* set up new backward pointers   */
        entry_info->pprev = entry_prev;
        POS_ENTRY_OFF(entry_prev_info->pnext, offset)->pprev = entry;

        /* set up new forward pointers   */
        entry_info->pnext = entry_prev_info->pnext;
        entry_prev_info->pnext = entry;

        return(head);
    }

    /* if list is not empty   */
    else if (head)
    {
        return(os_list_add_front_off(head, entry, offset));
    }
   else     /* list was empty */
   { 
        entry_info->pnext = entry; 
        entry_info->pprev = entry; 
    }
    return(entry);    
}

/* ********************************************************************   */
/* os_list_remove_off() - remove an entry from a list                     */
/*                                                                        */
/*   This function removes the entry (entry) from the doubly list         */
/*   list (head).                                                         */
/*                                                                        */
/*   Returns the new head of the list.                                    */
/*                                                                        */

POS_LIST os_list_remove_off(POS_LIST head, POS_LIST entry, int offset) /*__fn__*/
{
POS_LIST ret_val;
POS_LIST entry_info;

#if (DEBUG_LIST)
    if (!os_check_list_off(head, entry, offset))
    {
        break_list();
        return(head);
    }
#endif
    entry_info = POS_ENTRY_OFF(entry, offset);

    /* if no entry is to be removed, just return the head of the list   */
    if (!entry)
    {
        return(head);
    }

    /* if the entry is the only one on the list, the list is now empty   */
    if (entry_info->pprev == entry)  
    {
        ret_val = 0;
    }

    /* if there is more than 1 entry on list   */
    else
    { 
        /* remove the entry from the list   */
        POS_ENTRY_OFF(entry_info->pprev, offset)->pnext = entry_info->pnext;
        POS_ENTRY_OFF(entry_info->pnext, offset)->pprev = entry_info->pprev;

        /* get the new head of the list   */
        if (entry == head)
        {
            ret_val = entry_info->pnext;
        }
        else
        {
            ret_val = head;
        }
    }
    entry_info->pnext = entry_info->pprev = 0;
    return(ret_val);
}

/* ********************************************************************   */
/* os_list_next_entry_off() - returns next entry in a list                */
/*                                                                        */
/*   This function returns the next entry in a list or 0 if               */
/*   there are no more entries.                                           */
/*                                                                        */

POS_LIST os_list_next_entry_off(POS_LIST head, POS_LIST entry, int offset)
{
POS_LIST next;
POS_LIST entry_info;

    entry_info = POS_ENTRY_OFF(entry, offset);
    next = entry_info->pnext;
    if (next == head)
        return((POS_LIST)0);
    return(next);
}

/* ********************************************************************   */
/* os_list_last_off() - return last entry in list                         */
/*                                                                        */
/*   This function returns the last entry in a list.                      */
/*                                                                        */
/*   Returns the last entry in the list.                                  */
/*                                                                        */

POS_LIST os_list_last_off(POS_LIST head, int offset) /*__fn__*/
{
POS_LIST head_info;

    head_info  = POS_ENTRY_OFF(head, offset);

    if (!head)      /* if empty list */
        return((POS_LIST)0);
    return(head_info->pprev);
}

/* ********************************************************************   */
/* os_check_list_off() - checks if an entry is on a list                  */
/*                                                                        */
/*   This function checks if the entry (entry) is on the doubly list      */
/*   list (head).                                                         */
/*                                                                        */
/*   Returns TRUE if entry is on the list or FALSE if it is not.          */
/*                                                                        */

RTIP_BOOLEAN os_check_list_off(POS_LIST head, POS_LIST entry, int offset) /*__fn__*/
{
POS_LIST cur_entry;
POS_LIST cur_entry_info;

    if (!head)
        return(FALSE);

    cur_entry = head;
    do 
    {
        cur_entry_info = POS_ENTRY_OFF(cur_entry, offset);
        if (cur_entry == entry)
            return(TRUE);
        cur_entry = cur_entry_info->pnext;
    } while (cur_entry != head);
    return(FALSE);
}

#if (DEBUG_LIST)
void break_list(void)
{
    DEBUG_ERROR("break_list: doing an illegal list operation", NOVAR, 0, 0);
}

#endif

#if (DEBUG_LIST || DEBUG_IP_LIST)
/* os_cnt_list_off() - counts number of entries is on a list     */
/*                                                               */
/*   This function counts the entries on the doubly list         */
/*   list (head).                                                */
/*                                                               */
/*   Returns TRUE if entry is on the list or FALSE if it is not. */
/*                                                               */

int os_cnt_list_off(POS_LIST head, int offset) /*__fn__*/
{
POS_LIST cur_entry;
POS_LIST cur_entry_info;
int cnt;

    cnt = 0;

    if (!head)
        return(cnt);

    cur_entry = head;
    do 
    {
        cur_entry_info = POS_ENTRY_OFF(cur_entry, offset);
        cnt++;
        cur_entry = cur_entry_info->pnext;
    } while (cur_entry != head);
    return(cnt);
}
#endif /* DEBUG_LIST || DEBUG_IP_LIST */


#if (DEBUG_IP_LIST)
void break_ip_list(int who)
{
    DEBUG_ERROR("IP LIST CORRUPTED: who: ", EBS_INT1, who, 0);
}

void print_ip_values(PIFACE pi, POS_LIST list_before, POS_LIST list_after, DCU msg)
{
int iface_off;

    DEBUG_ERROR("IP LIST BEFORE AND AFTER: ", DINT2, 
        list_before, list_after);
    DEBUG_ERROR("PI, MSG = ", DINT2, pi, msg);

    for (iface_off = 0; iface_off < CFG_NIFACES; iface_off++)
    {
        /* get interface, don't care if open since valid flags will only      */
        /* be set if open and don't want tc_ino2_iface to report error        */
        PI_FROM_OFF(pi, iface_off)
        if (pi)
        {
            DEBUG_ERROR("EXISTING PI ADDR: ", DINT1, pi, 0);
            if (pi->open_count)
            {
                DEBUG_ERROR("OPEN COUNT: ", EBS_INT1, pi->open_count, 0);
                DEBUG_ERROR("DEVICE NAME: ", STR1, pi->pdev->device_name, 0);
            }
        }
    }
}

int cnt_ip_exch(void)
{
int iface_off;
DCU msg;
DCU root_msg;
word ex1;
PIFACE pi;
int ip_cnt;

    ip_cnt = 0;
    ex1 = OS_HNDL_TO_EXCH(IF_EX_IP); 
    LOOP_THRU_IFACES(iface_off)
    {
        PI_FROM_OFF(pi, iface_off)
        if (pi)
        {
            root_msg = msg = (DCU) pi->ctrl.exch_list[ex1];
            ip_cnt += os_cnt_list_off((POS_LIST)root_msg, ZERO_OFFSET);
        }
    }
    return(ip_cnt);
}

static int diff_ip_cnt = 0;

/* This shall be called with interrupts disabled   */
RTIP_BOOLEAN _check_ip_list(int who)
{
int num_in_ip_list;
int num_ip_cnt;
int i;
int d_ip_c;

    /* first count the number of DCUs in the IP list   */
    num_in_ip_list = 0;
    for (i = 0; i < ndcus_alloced; i++)
    {
        if (dcu_pool_ptr[i].ctrl.list_id == IN_IP_LIST)
        {
            num_in_ip_list++;
        }
    }
    num_ip_cnt = cnt_ip_exch();

    d_ip_c = num_ip_cnt - num_in_ip_list;
    if (d_ip_c < 0)
        d_ip_c = -d_ip_c;
    if (d_ip_c > diff_ip_cnt)
    {
        DEBUG_ERROR("num_ip_cnt != num_in_ip_list: ", EBS_INT2,
            num_ip_cnt, num_in_ip_list);
        break_ip_list(who);
        diff_ip_cnt = d_ip_c;
            return(FALSE);
    }
    return(TRUE);
}

RTIP_BOOLEAN check_ip_list(int who)
{
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
RTIP_BOOLEAN ret_val;

    KS_TL_SPLX(sp)  /* Disable and  push interrupts */;
    ret_val = _check_ip_list(who);
    KS_TL_SPL(sp)
    return(ret_val);
}

#endif      /* DEBUG_IP_LIST */

/* ********************************************************************   */
/* MEMORY INITIALIZATION                                                  */
/* ********************************************************************   */

/* ********************************************************************   */
/* os_memory_init() - performs memory initialization                      */
/*                                                                        */
/*   Performs all memory initialization, i.e. links pools (arrays) of     */
/*   data structures togther including ports (sockets), packets,          */
/*   windows.  Initializes routing table, interfaces etc.                 */
/*                                                                        */
/*   Returns TRUE if successful, FALSE if failure                         */
/*                                                                        */

RTIP_BOOLEAN os_memory_init(RTIP_BOOLEAN restart)                     /*__fn__*/
{
int      i;
PIFACE   pi;
#if (!INCLUDE_MALLOC_PORTS)
PANYPORT port;
#endif
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
DCU      msg;
#endif

#if (DEBUG_MEMSTATS)
    os_track_allocation_stats_init();
#endif  
  
    /* ********************************************************************   */
    /* UDP PORTS                                                              */
    /* Manually add all entries to their lists. We can't call standard        */
    /* calls because they use kernel services which aren't yet set up         */
#if (!INCLUDE_MALLOC_PORTS)
#    if (INCLUDE_UDP || INCLUDE_RAW)
    for (i = 0; i < CFG_NUDPPORTS; i++)
    {
        port = (PANYPORT) &tc_udpp_pool[i];
        tc_memset((PFBYTE)port, 0, sizeof(UDPPORT));
        port->ctrl.index = i;
        alloced_ports[port->ctrl.index] = (PANYPORT)0;
        root_udp_port = os_list_add_front_off(root_udp_port, 
                                              (POS_LIST)port, ZERO_OFFSET);
    }
#    endif

    /* ********************************************************************   */
    /* TCP PORTS                                                              */
#    if (INCLUDE_TCP)
    for (i = 0; i < CFG_NTCPPORTS; i++)
    {
        port = (PANYPORT) &tc_tcpp_pool[i];
        tc_memset((PFBYTE)port, 0, sizeof(TCPPORT));

        /* Note: Add NUDPPORTS so PORT signals & semaphores are contiguos   */
        port->ctrl.index = i+(word)CFG_NUDPPORTS;
        ((PTCPPORT)port)->state = TCP_S_FREE;
        alloced_ports[port->ctrl.index] = (PANYPORT)0;
        root_tcp_port = os_list_add_front_off(root_tcp_port, 
                                              (POS_LIST)port, ZERO_OFFSET);
    }
#    endif
#endif      /* INCLUDE_MALLOC_PORTS */


    /* ********************************************************************   */
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
    ndcus_alloced = CFG_NDCUS;
    /* SET UP POINTERS TO DCUs (POSSIBLY MALLOC DCUs)   */
#if (INCLUDE_MALLOC_DCU_INIT)
    if (!restart)
    {
        dcu_pool_ptr = (EPACKET KS_FAR *)ks_malloc(sizeof(EPACKET), CFG_NDCUS,
                                                   DCU_MALLOC);
        if (!dcu_pool_ptr)
            return(FALSE);
    }
#else
    ARGSUSED_INT(restart)
    dcu_pool_ptr = (EPACKET KS_FAR *)tc_dcu_pool;
#endif

    /* LINK THE DCUs                                          */
    /* We make a list of DCU's here. This list is used        */
    /* by init_packet_pool to assign DCU's to memory regions. */
    /* After that root_dcu is no longer used.                 */
    for (i = 0; i < ndcus_alloced; i++)
    {
        msg = (DCU) &dcu_pool_ptr[i];
        tc_memset((PFBYTE)msg, 0, sizeof(EPACKET));

#        if (INCLUDE_TRK_PKTS)
            msg->ctrl.index = i;
            msg->ctrl.requestor = 0;
#        endif

        root_dcu = os_list_add_front_off(root_dcu, (POS_LIST) msg, 
                                         ZERO_OFFSET);
    }

    lowest_free_packets = current_free_packets = ndcus_alloced;

    /* Now chop up the packet pool data and assign each packet pool a
       DCU and thread the DCU's together in the freelists at 
       root_dcu_array[]. Each entry in the array is the list head of
       a free list of dcu's pointing to data of a size corresponding to
       dcu_size_array[] */
    os_init_packet_pools();

    /* Zero root DCU. We don't need it any more   */
    root_dcu = 0;
#endif /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

    /* ********************************************************************   */
    /* IFACE's                                                                */
    LOOP_THRU_IFACES(i)
    {
        PI_FROM_OFF(pi, i)
        if (pi)
        {
            /* set the interface number and the device access   */
            pi->ctrl.index = i;
        }
    }

    /* ********************************************************************   */
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
#if (INCLUDE_TRK_PKTS)
    /* RS232 info array - needs to clear for tracking packet code so       */
    /* will not pick up bogas info; normally these info arrays are not     */
    /* looked at until attach interface has been called to alloc the dcu's */
    for (i=0; i < CFG_NUM_RS232; i++)
    {
        rs232_if_info_arry[i].msg_in = (DCU)0;
    }
#endif
#endif

    /* ********************************************************************   */
#if (INCLUDE_SYSLOG && INCLUDE_BGET)
    /* set to a suitable 'number of bytes' to reserve for snmpv2/v3 related run-time   */
    /* storage allocation. Don't worry about this one if INCLUDE_BGET is NOT set.      */
    bpool(&syslog_mempool[0], sizeof(syslog_mempool));
#endif
#if (INCLUDE_SSL && INCLUDE_BGET)
    /* tbd                                                                    */
    /* TBD [i_a] because this will need to move once we start creating binary */
    /* distributions.                                                         */
    bpool(&ssl_mempool[0], sizeof(ssl_mempool));
#endif

    return(TRUE);
}

#if (INCLUDE_MALLOC_DCU_INIT || INCLUDE_MALLOC_PORTS || BUILD_NEW_BINARY)
/* ********************************************************************          */
/* os_memory_free() - free any dynamic memory allocated by memory initialization */
/*                                                                               */
/*   Frees all dynamic memory allocated during initialization.                   */
/*   Call when RTIP is exited.                                                   */
/*                                                                               */
/*   Returns nothing                                                             */
/*                                                                               */

void os_memory_free(void)
{
#if (INCLUDE_MALLOC_DCU_INIT)
    /* FREE THE PACKETS   */
    if (CFG_NUM_PACKETS0)
        KS_FREE_PKT((PFBYTE)packet_pool0, CFG_PACKET_SIZE0 * CFG_NUM_PACKETS0, 1);
    if (CFG_NUM_PACKETS1)
        KS_FREE_PKT((PFBYTE)packet_pool1, CFG_PACKET_SIZE1 * CFG_NUM_PACKETS1, 1);
    if (CFG_NUM_PACKETS2)
        KS_FREE_PKT((PFBYTE)packet_pool2, CFG_PACKET_SIZE2 * CFG_NUM_PACKETS2, 1);
    if (CFG_NUM_PACKETS3)
        KS_FREE_PKT((PFBYTE)packet_pool3, CFG_PACKET_SIZE3 * CFG_NUM_PACKETS3, 1);
    if (CFG_NUM_PACKETS4)
        KS_FREE_PKT((PFBYTE)packet_pool4, CFG_PACKET_SIZE4 * CFG_NUM_PACKETS4, 1);
    if (CFG_NUM_PACKETS5)
        KS_FREE_PKT((PFBYTE)packet_pool5, CFG_PACKET_SIZE5 * CFG_NUM_PACKETS5, 1);

    /* FREE THE DCUs   */
    ks_free((PFBYTE)dcu_pool_ptr, sizeof(EPACKET), CFG_NDCUS);

#endif  /* INCLUDE_MALLOC_DCU_INIT */

#    if (defined (RTKBCPP))
        ks_dpmi_release_all();
#    endif
}
#endif      /* INCLUDE_MALLOC_DCU_INIT || BUILD_NEW_BINARY */

/* ********************************************************************   */
/* PACKET ALLOCATION                                                      */
/* ********************************************************************   */

/* count number of packets available of size greater or equal to nbytes   */
int cnt_num_avail_pkts(int nbytes)
{
int num_pkts;
int index;

    num_pkts = 0;
    for (index = 0; index < CFG_NUM_FREELISTS; index++)
    {
        if (nbytes <= dcu_size_array[index])
            num_pkts += current_free_packets_array[index];
    }
    return(num_pkts);
}

/* count number of packets available of size greater or equal to nbytes;   */
/* if there are greater than limit packets return TRUE else return         */
/* FALSE                                                                   */
RTIP_BOOLEAN cnt_num_avail_pkts_limit(int nbytes, int limit)
{
int num_pkts;
int index;

    num_pkts = 0;
    for (index = 0; index < CFG_NUM_FREELISTS; index++)
    {
        if (nbytes <= dcu_size_array[index])
            num_pkts += current_free_packets_array[index];
        if (num_pkts > limit)
            return(TRUE);
    }
    return(FALSE);
}

#if (INCLUDE_NO_DCU_BLOCK)
/* ********************************************************************   */
/* os_alloc_packet_wait() - allocate a DCU (packet) from the free list    */
/*                                                                        */
/*   Blocks if packet is not available then                               */
/*   allocates a DCU (packet) from the free list of DCUs.                 */
/*                                                                        */
/*   Returns the DCU taken off the free list or 0 if empty.               */
/*                                                                        */

DCU os_alloc_packet_wait(int nbytes, PANYPORT port, int who)           /*__fn__*/
{
int num_pkts;

    /* **************************************************          */
    /* count number of packets available which satisfy the request */
    num_pkts = cnt_num_avail_pkts(nbytes);

    /* block if below threshold   */
    if (port && port->ctrl.block_ticks && 
        (num_pkts < CFG_DCU_BLOCK))
    {
        /* add to list of blocking ports - tbd how protected   */
        ports_blocked_for_dcu = 
            os_list_add_front_off(ports_blocked_for_dcu,
                                  (POS_LIST)port,
                                  PORT_DCU_OFFSET);

#if (DEBUG_NO_DCU_BLOCK)
        DEBUG_ERROR("BLOCK ON DCU SIGNAL: num, threshold", EBS_INT2, 
            num_pkts, CFG_DCU_BLOCK);
#endif

        /* BLOCK WAITING FOR PACKET TO BE AVAILABLE   */
        port->block_on_nbytes = nbytes;
        OS_BIND_DCU_SIGNAL(port);
        OS_TEST_DCU_SIGNAL(port, port->ctrl.block_ticks);
    }
    return(os_alloc_packet(nbytes, who));
}
#endif

/* ********************************************************************       */
/* os_alloc_packet() - allocate a DCU (packet) from the free list             */
/*                                                                            */
/*   Allocates a DCU (packet) from the free list of DCUs.                     */
/*   Keeps track of the current free count as well as low water.              */
/*                                                                            */
/*   Modification of the free list is protected from reentrancy by disabling  */
/*   interrupts.                                                              */
/*                                                                            */
/*   Returns the DCU taken off the free list or 0 if empty.                   */
/*                                                                            */

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
DCU os_alloc_packet(int nbytes, int who)           /*__fn__*/
{
DCU    msg;
POS_LIST entry_info;

    ARGSUSED_INT(who);

    OS_CLAIM_SYSLOG(ARP_SOCK_ARP_TABLE_CLAIM)
    msg = (DCU)ks_malloc(sizeof(EPACKET), 1, EPACKET_MALLOC);
    DCUTODATA(msg) = ks_malloc(nbytes+PKT_GUARD_SIZE, 1, DCU_DATA_MALLOC);
    if (!DCUTODATA(msg))
    {
        ks_free((PFBYTE)msg, nbytes+PKT_GUARD_SIZE, 1);
        msg = 0;
    }
    OS_RELEASE_SYSLOG()

    /* save for free   */
    DCUTOPACKET(msg)->nbytes = nbytes;

    /* set up flags for sending packet in case this is an output packet;   */
    /* default is don't keep, don't signal                                 */
    DCUTOCONTROL(msg).dcu_flags = NO_DCU_FLAGS;    
#if (INCLUDE_FRAG)
    DCUTOPACKET(msg)->frag_next = (DCU)0;
#endif
    entry_info = POS_ENTRY_OFF(msg, OUTPUT_LIST_OFFSET);
    tc_memset(entry_info, 0, sizeof(OS_LIST));

    /* track highwater etc   */
    current_alloc_packets += 1;
    if (current_alloc_packets > highest_alloc_packets)
        highest_alloc_packets = current_alloc_packets;

#if (INCLUDE_TRK_PKTS)
    DCUTOCONTROL(msg).requestor = who;
#endif

#if (DISPLAY_DCU)
    DEBUG_ERROR("DCU: alloc DCU addr, who = ", DINT2, msg, who);
#endif
    return(msg);
}

#else
DCU os_alloc_packet(int nbytes, int who)           /*__fn__*/
{
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
DCU msg;
int index;
POS_LIST entry_info;

    /* **************************************************   */
#if (!INCLUDE_TRK_PKTS)
    ARGSUSED_INT(who);
#endif

#if (DEBUG_DCU)
    n_alloc_calls++;
    DEBUG_LOG("AL:currfree lowest ",LEVEL_2, DINT2,
              (long)current_free_packets,(long)lowest_free_packets );
    DEBUG_LOG("AL:allocs, frees == ", LEVEL_3, DINT2,
              n_alloc_calls, n_free_calls);
    DEBUG_LOG("who == ", LEVEL_4, EBS_INT1, who, 0);
#endif

#if (DEBUG_MEMSTATS)
    /* [i_a] so I can monitor mem-alloc size distribution...   */
    os_track_allocation_stats(nbytes);
#endif

#if (INCLUDE_TCP && INCLUDE_TCP_OUT_OF_ORDER)
    if (current_free_packets < CFG_PKT_LOWWATER_OOO)
    {
        free_ooo_lists = TRUE;
    }
#endif

    /* **************************************************   */
    /* allocate the DCU                                     */
    /* Search all of the free lists until we find one that 
       is big enough to hold our request and has a free packet */
    msg = (DCU)0;
    for (index = 0; index < CFG_NUM_FREELISTS; index++)
    {
        if (nbytes <= dcu_size_array[index])
        {
            KS_TL_SPLX(sp)
            msg = (DCU)(root_dcu_array[index]);
            if (msg)
            {
                root_dcu_array[index] = 
                    os_list_remove_off(root_dcu_array[index],
                                       root_dcu_array[index], ZERO_OFFSET);
                /* track lowwater etc   */
                current_free_packets -= 1;
                if (current_free_packets < lowest_free_packets)
                    lowest_free_packets = current_free_packets;
                current_free_packets_array[index]--;
                if (current_free_packets_array[index] < lowest_free_packets_array[index])
                    lowest_free_packets_array[index] = 
                        current_free_packets_array[index];

                KS_TL_SPL(sp)
                break;
            }
            KS_TL_SPL(sp)
        }
    }

    /* **************************************************   */
    /* display errors if allocation failed                  */
    if (!msg)
    {
        DEBUG_ERROR("ALLOC PKT FAILED: size = ", EBS_INT1,
            nbytes, 0);
        DEBUG_ERROR("                : current, low = ", EBS_INT2,
            current_free_packets, lowest_free_packets);
#if (INCLUDE_TRK_PKTS && DEBUG_TRACK)
{
struct track_dcu track_info;
        xn_stats_gather_packet_info(&track_info, TRUE);
}
        display_packet_lowwater();
        display_sem_info();
        display_xmit_info();
#endif
    }
    /* **************************************************     */
    /* initialize DCU, statistics etc if allocation succeeded */
    else
    {
        /* set up flags for sending packet in case this is an output packet;   */
        /* default is don't keep, don't signal                                 */
        DCUTOCONTROL(msg).dcu_flags = NO_DCU_FLAGS;    
#if (INCLUDE_FRAG)
        DCUTOPACKET(msg)->frag_next = (DCU)0;
#endif
        entry_info = POS_ENTRY_OFF(msg, OUTPUT_LIST_OFFSET);
        tc_memset(entry_info, 0, sizeof(OS_LIST));

#if (INCLUDE_TRK_PKTS)
        DCUTOCONTROL(msg).requestor = who;
#        if (DEBUG_TRACK)
            if (current_free_packets < 5)
            {
            struct track_dcu track_info;
                xn_stats_gather_packet_info(&track_info, TRUE);
                display_packet_lowwater();
                display_sem_info();
                display_xmit_info();
            }
#        endif
#endif
    }

    /* **************************************************   */
#if (DISPLAY_DCU)
    DEBUG_ERROR("DCU: alloc DCU addr, who = ", DINT2, msg, who);
    DEBUG_ERROR("     lowest_free_packets, current_free_packets", EBS_INT2,
        lowest_free_packets, current_free_packets);
#endif

    /* **************************************************   */
    return(msg);
}
#endif /* INCLUDE_MALLOC_DCU_AS_NEEDED */

/* ********************************************************************       */
/* os_alloc_packet_input() - allocate a DCU (packet) from the free list       */
/*                                                                            */
/*   Allocates a DCU (packet) from the free list of DCUs if there             */
/*   is at least CFG_PKT_LOWWATER packets left.  Also keeps                   */
/*   track of the current free count as well as low water.                    */
/*                                                                            */
/*   This routine is called by the drivers.  By limiting allocating packets   */
/*   to a threshold on the input side, output packets should still be able    */
/*   to be sent out.                                                          */
/*                                                                            */
/*   Modification of the free list is protected from reentrancy by disabling  */
/*   interrupts.                                                              */
/*                                                                            */
/*   Returns the DCU taken off the free list or 0 if cannot allocate          */
/*   a DCU.                                                                   */
/*                                                                            */

DCU os_alloc_packet_input(int nbytes, int who)                     /*__fn__*/
{
#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    return(os_alloc_packet(nbytes, who));
#else
    if (cnt_num_avail_pkts_limit(nbytes, CFG_PKT_LOWWATER))
    {
        return(os_alloc_packet(nbytes, who));
    }

    DEBUG_ERROR("ALLOC INPUT PKT FAILED: current, low = ", EBS_INT2,
            current_free_packets, lowest_free_packets);
#    if (INCLUDE_TRK_PKTS && DEBUG_TRACK)
        {
        struct track_dcu track_info;
            xn_stats_gather_packet_info(&track_info, TRUE);
        }
        display_packet_lowwater();
        display_sem_info();
        display_xmit_info();
#    endif
    return((DCU)0);
#endif  /* INCLUDE_MALLOC_DCU_AS_NEEDED */
}


/* ********************************************************************       */
/* os_free_packet() - free a DCU (packet)                                     */
/*                                                                            */
/*   Puts a DCU (packet) on the free list of DCUs.  The DCU is put            */
/*   on the front of the free list.  Also keeps track of the current          */
/*   free count.                                                              */
/*                                                                            */
/*   Modification of the free list is protected from reentrancy by disabling  */
/*   interrupts.                                                              */
/*                                                                            */
/*   Returns nothing.                                                         */
/*                                                                            */

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
void os_free_packet(DCU msg)                                 /*__fn__*/
{
#if (DISPLAY_DCU)
    DEBUG_ERROR("DCU: free DCU addr = ", DINT1, msg, 0);
#endif
    OS_CLAIM_SYSLOG(ARP_SOCK_ARP_TABLE_CLAIM)
    ks_free((PFBYTE)(DCUTODATA(msg)), 
            DCUTOPACKET(msg)->nbytes+PKT_GUARD_SIZE, 1);
    ks_free((PFBYTE)msg, sizeof(EPACKET), 1);

    /* track highwater etc   */
    current_alloc_packets -= 1;
    OS_RELEASE_SYSLOG()
}
#else
void os_free_packet(DCU msg)                                 /*__fn__*/
{
#if (INCLUDE_FRAG)
DCU next_msg;
#endif
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
#if (INCLUDE_NO_DCU_BLOCK)
PANYPORT port;
int      num_pkts;
#endif

#if (DEBUG_DCU)
    n_free_calls += 1;
    DEBUG_LOG("FR:currfree lowest ", LEVEL_2, DINT2, (long)current_free_packets,(long)lowest_free_packets);
    DEBUG_LOG("FR:allocs, frees == ", LEVEL_3, DINT2, n_alloc_calls, n_free_calls);
#endif
#if (DISPLAY_DCU)
    DEBUG_ERROR("DCU: free entered", DINT1, msg, 0);
#endif

    /* **************************************************   */
#if (INCLUDE_FRAG)
    while (msg)
#else
    if (msg)
#endif
    {
#        if (DISPLAY_DCU)
            DEBUG_ERROR("DCU: free ", DINT1, msg, 0);
#        endif
#        if (DEBUG_FREE)
            if (msg->root_dcu_index < 0 || msg->root_dcu_index >= CFG_NUM_FREELISTS)
            {
                DEBUG_ERROR("FREEING a CORRUPTED DCU - addr = ", DINT1, (dword) msg, 0);
                break_it();
                return;
            }

            KS_TL_SPLX(sp)  /* Disable and  push interrupts */
            if (check_free(msg))
            {
#                if (INCLUDE_TRK_PKTS)
                    DEBUG_ERROR("FREED AN ALREADY FREED PACKET - who = ",
                        EBS_INT1, msg->ctrl.requestor, 0);
#                else
                    DEBUG_ERROR("FREED AN ALREADY FREED PACKET", NOVAR, 0, 0);
#                endif
                KS_TL_SPL(sp)
                return;
            }
#if (INCLUDE_TCP && DEBUG_FREE_TCP_WIN)
            if (check_tcp_window(msg))
            {
                KS_TL_SPL(sp)
                break_it();
                DEBUG_ERROR("FREED A PACKET ON TCP WINDOW", NOVAR, 0, 0);
                return;
            }
#endif
            KS_TL_SPL(sp)
#        endif      /* DEBUG_FREE */

#if (INCLUDE_FRAG)
        next_msg = DCUTOPACKET(msg)->frag_next;
        DCUTOPACKET(msg)->frag_next = (DCU)0;
#endif

        /* if xmit of the packet is in progress do not free it now but   */
        /* set flag so IP layer will know to free it when xmit           */
        /* completes (or times out)                                      */
        if (DCUTOCONTROL(msg).dcu_flags & PKT_SEND_IN_PROG)
        {
            DCUTOCONTROL(msg).dcu_flags |= PKT_FREE;
#if (INCLUDE_FRAG)
            msg = next_msg;
            continue;
#else
            return;
#endif
        }

        /* turn off all flags but PKT_FLAG_KEEP   */
        DCUTOCONTROL(msg).dcu_flags &= PKT_FLAG_KEEP;

        KS_TL_SPLX(sp)  /* Disable and  push interrupts */
        current_free_packets += 1;
        current_free_packets_array[msg->root_dcu_index]++;

        root_dcu_array[msg->root_dcu_index] =
              os_list_add_front_off(root_dcu_array[msg->root_dcu_index],
                                    (POS_LIST)msg, ZERO_OFFSET);

#if (INCLUDE_TRK_PKTS)
        msg->ctrl.requestor = -1;
        DCUTOPACKET(msg)->ctrl.list_id = IN_FREE_LIST;
#endif
        KS_TL_SPL(sp)

#if (INCLUDE_FRAG)
        msg = next_msg;
#endif

    }       /* end of loop thru fragments */

#if (INCLUDE_NO_DCU_BLOCK)
    /* **************************************************   */
    /* check if should signal ports blocked waiting for DCU */
    port = (PANYPORT)ports_blocked_for_dcu;
    while (port)
    {
        port = (PANYPORT)ports_blocked_for_dcu;

        /* count number of packets available which satisfy the request   */
        num_pkts = cnt_num_avail_pkts(port->block_on_nbytes);

        if (num_pkts > CFG_DCU_RESUME)
        {
            /* remove from front of list of blocking ports - tbd how protected   */
            ports_blocked_for_dcu = 
                os_list_remove_off(ports_blocked_for_dcu,
                                   (POS_LIST)port,
                                   PORT_DCU_OFFSET);

#if (DEBUG_NO_DCU_BLOCK)
            DEBUG_ERROR("WAKEUP TASK BLOCKED ON DCU SIGNAL: num, threshold", 
                EBS_INT2, num_pkts, CFG_DCU_RESUME);
#endif
            /* signal os_alloc_packet since a enough packets are available   */
            OS_SET_DCU_SIGNAL(port);
            break;
        }
        port = (PANYPORT)
            os_list_next_entry_off(ports_blocked_for_dcu,
                                   (POS_LIST)port,
                                    PORT_DCU_OFFSET);
    }           /* end of while loop thru port on blocked list */
#endif          /* INCLUDE_NO_DCU_BLOCK */
}
#endif /* INCLUDE_MALLOC_DCU_AS_NEEDED */

/* ********************************************************************   */
/* VARIABLE PACKET POOL ROUTINES                                          */
/* ********************************************************************   */

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)

/* ********************************************************************   */
/* os_init_packet_pools() - initialize packet pools                       */
/*                                                                        */
/*    Initialize all the packet pools by putting the packets into         */
/*    linked lists.  If INCLUDE_MALLOC_DCU_INIT is set, the packets       */
/*    are allocated using dynamic memory otherwise the packets            */
/*    are defined as arrays.                                              */
/*                                                                        */

void os_init_packet_pools(void)
{
#if (INCLUDE_MALLOC_DCU_INIT)
    if (CFG_NUM_PACKETS0)
    {
        /* allocate memory for the packet pools   */
        packet_pool0 = (PFBYTE)
            KS_MALLOC_PKT(CFG_PACKET_SIZE0, CFG_NUM_PACKETS0, 
                          PACKET_POOL_MALLOC);
        os_init_freelist_entry( 0, 
                                (PFBYTE)packet_pool0,   
                                CFG_NUM_PACKETS0,
                                CFG_PACKET_SIZE0+CFG_PACKET_ADJ);
    }
    else
        packet_pool0 = (PFBYTE)0;

    if (CFG_NUM_PACKETS1)
    {
        packet_pool1 = 
            KS_MALLOC_PKT(CFG_PACKET_SIZE1, CFG_NUM_PACKETS1, 
                          PACKET_POOL_MALLOC);
        /* Initialize the second free list.    */
        os_init_freelist_entry( 1, 
                                (PFBYTE)packet_pool1,  
                                CFG_NUM_PACKETS1,
                                CFG_PACKET_SIZE1+CFG_PACKET_ADJ);
    }
    else
        packet_pool1 = (PFBYTE)0;

    if (CFG_NUM_PACKETS2)
    {
        packet_pool2 = 
            KS_MALLOC_PKT(CFG_PACKET_SIZE2, CFG_NUM_PACKETS2,
                          PACKET_POOL_MALLOC);

        os_init_freelist_entry( 2, 
                                (PFBYTE)packet_pool2,  
                                CFG_NUM_PACKETS2,
                                CFG_PACKET_SIZE2+CFG_PACKET_ADJ);
    }
    else
        packet_pool2 = (PFBYTE)0;

    if (CFG_NUM_PACKETS3)
    {
        packet_pool3 = 
                KS_MALLOC_PKT(CFG_PACKET_SIZE3 * CFG_NUM_PACKETS3, 1, 
                              PACKET_POOL_MALLOC);
            os_init_freelist_entry( 3, 
                                    (PFBYTE)packet_pool3, 
                                    CFG_NUM_PACKETS3,
                                    CFG_PACKET_SIZE3+CFG_PACKET_ADJ);
    }
    else
        packet_pool3 = (PFBYTE)0;
    if (CFG_NUM_PACKETS4)
    {
        packet_pool4 = 
            KS_MALLOC_PKT(CFG_PACKET_SIZE4, CFG_NUM_PACKETS4, 
                          PACKET_POOL_MALLOC);
        os_init_freelist_entry( 4, 
                                (PFBYTE)packet_pool4,
                                CFG_NUM_PACKETS4,
                                CFG_PACKET_SIZE4+CFG_PACKET_ADJ);
    }
    else
        packet_pool4 = (PFBYTE)0;

    if (CFG_NUM_PACKETS5)
    {
            packet_pool5 = 
                KS_MALLOC_PKT(CFG_PACKET_SIZE5, CFG_NUM_PACKETS5,
                              PACKET_POOL_MALLOC);
            /* Initialize the sixth free list.    */
            os_init_freelist_entry( 5, 
                                    (PFBYTE)packet_pool5, 
                                    CFG_NUM_PACKETS5,
                                    CFG_PACKET_SIZE5+CFG_PACKET_ADJ);
    }
    else
        packet_pool5 = (PFBYTE)0;

#else           /* INCLUDE_MALLOC_DCU_INIT */

#if (CFG_NUM_PACKETS0)
    /* Initialize the first free list.   */
    os_init_freelist_entry( 0, 
                            (PFBYTE)&packet_pool0[0][0],
                            CFG_NUM_PACKETS0,
                            CFG_PACKET_SIZE0+CFG_PACKET_ADJ);
#endif

#if (CFG_NUM_PACKETS1)
    /* Initialize the second free list.  */
    os_init_freelist_entry( 1, 
                            (PFBYTE)&packet_pool1[0][0],
                            CFG_NUM_PACKETS1,
                            CFG_PACKET_SIZE1+CFG_PACKET_ADJ);
#endif
#if (CFG_NUM_PACKETS2)
    /* Initialize the third free list.   */
    os_init_freelist_entry( 2, 
                            (PFBYTE)&packet_pool2[0][0],
                            CFG_NUM_PACKETS2,
                            CFG_PACKET_SIZE2+CFG_PACKET_ADJ);
#endif
#if (CFG_NUM_PACKETS3)
    /* Initialize the fourth free list.  */
    os_init_freelist_entry( 3, 
                            (PFBYTE)&packet_pool3[0][0],
                            CFG_NUM_PACKETS3,
                            CFG_PACKET_SIZE3+CFG_PACKET_ADJ);
#endif
#if (CFG_NUM_PACKETS4)
    /* Initialize the fifth free list.   */
    os_init_freelist_entry( 4, 
                            (PFBYTE)&packet_pool4[0][0],
                            CFG_NUM_PACKETS4,
                            CFG_PACKET_SIZE4+CFG_PACKET_ADJ);
#endif
#if (CFG_NUM_PACKETS5)
    /* Initialize the sixth free list.  */
    os_init_freelist_entry( 5, 
                            (PFBYTE)&packet_pool5[0][0],
                            CFG_NUM_PACKETS5,
                            CFG_PACKET_SIZE5+CFG_PACKET_ADJ);
#endif
#endif          /* INCLUDE_MALLOC_DCU_INIT */
}

#endif /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
/* ********************************************************************     */
/* Initialize one free list. Take a piece of core, the size of the core
   and the size of the packet. Chop up the core and initialize the list 
   then use DCUS to build a free list for this region of core 
*/ 
void os_init_freelist_entry(int index, PFBYTE pcore, int num_packets, int packet_size)
{
int i;
#if (KS_PKT_ALIGNMENT != 1)
int align_mask;
dword total_core;
#endif
DCU msg;

#if (INCLUDE_GUARD_PACKET)
    packet_size += PKT_GUARD_SIZE;
#endif

#if (KS_PKT_ALIGNMENT != 1)
    /* If the packet core is not on a KS_PKT_ALIGNMENT byte boundary make it so.    */
    align_mask = KS_PKT_ALIGNMENT - 1;
    total_core = (dword) (num_packets * packet_size);

    /* calculate pcore to point to first aligned byte; reduce num_packets   */
    /* and total_core as a result                                           */
    if ((dword)pcore & align_mask)
    {
        while((dword)pcore & align_mask) pcore--;
        pcore += KS_PKT_ALIGNMENT;

        /* Reduce num_packets by one since we walked into it   */
        num_packets -= 1;

        /* Reduce total core since we moved into the packet   */
        total_core -= KS_PKT_ALIGNMENT;
    }

    /* Increment packet size until it is aligned which will force
       all subsequent packets to also be aligned; can reduce number
       of packets as a result */
    if (packet_size & align_mask)
    {
        while(packet_size & align_mask) packet_size++;

        /* Now recalculate number of packets with our new size   */
        num_packets = (int) (total_core / packet_size);
    }
#endif /* (KS_PKT_ALIGNMENT) */

    /* Remember the size of the data so we can alloc by size in os_alloc_packet   */
#if (INCLUDE_GUARD_PACKET)
    dcu_size_array[index] = packet_size - PKT_GUARD_SIZE;
#else
    dcu_size_array[index] = packet_size;
#endif

    highest_free_packets_array[index] = num_packets;    /* never changes */
    lowest_free_packets_array[index] =  num_packets;
    current_free_packets_array[index] = num_packets;

    /* Build the freelist for these packets   */
    for (i = 0; i < num_packets; i++)
    {
#if (INCLUDE_GUARD_PACKET)
        tc_memset(pcore+packet_size-PKT_GUARD_SIZE, 0x29, PKT_GUARD_SIZE);
#endif
        /* Take the DCU off of the original list we allocated and put it
           on the free list array segregated by data size */
        msg = (DCU) root_dcu;
        root_dcu = os_list_remove_off(root_dcu, root_dcu, ZERO_OFFSET);

        /* This should always be true   */
        if (msg)
        {
            /* Attach the core to the DCU   */
            DCUDATA(msg) = (PFBYTE)pcore;

#if (INCLUDE_TRK_PKTS)
            DCUTOCONTROL(msg).requestor = -1;
#endif

            /* Save which free list we are on. We'll use this in
               os_free_packet */
            msg->root_dcu_index = index;
            root_dcu_array[index] =
                os_list_add_front_off(root_dcu_array[index],
                                      (POS_LIST)msg, ZERO_OFFSET);
        }
        else
        {
            DEBUG_ERROR("PANIC: os_init_freelist_entry", NOVAR, 0, 0);
        }
        pcore += packet_size;
    }
}
#endif /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (INCLUDE_GUARD_PACKET)
void break_guard(void)
{
  LogError(ERROR_LOG_ID_RTIP_MEM_GUARD);
  SignalEventToPowerDown(POWER_DOWN_EVENT);
}

RTIP_BOOLEAN check_guard(PFBYTE pcore, int packet_size)
{
int i;
PFBYTE curr_byte_ptr;
byte   curr_byte;

    for (i=0; i < PKT_GUARD_SIZE; i++)
    {
        curr_byte_ptr = pcore+packet_size+i;
        curr_byte = *curr_byte_ptr;
        if (curr_byte != 0x29)
        {
            DEBUG_ERROR("GUARD ERROR: byte = ", EBS_INT1,
                curr_byte, 0);
            DEBUG_ERROR("GUARD ERROR: pcore, bad addr = ", DINT2,
                pcore, curr_byte_ptr);

            break_guard();
            return(FALSE);
        }
    }
    return(TRUE);
}
#endif

/* ********************************************************************   */
/* ROUTINES the emulate malloc and free using DCU data.                   */
/* ********************************************************************   */

/* allocate memory from DCUs; dcu_free_core should be called to free   */
/* the memory when done with it                                        */
/* returns memory address or 0 if failure                              */
PFBYTE dcu_alloc_core(int size)
{
DCU msg;
PFBYTE p;
DCU *pmsg;

    /* Allocate a DCU of a size (size + 4). We will store the DCU
       address in the beginning of the data area so that we can 
       free it when the dcu_free_core function is called  */
    msg = os_alloc_packet((int)ADDR_ADD(size,4), DCU_ALLOC_CORE_ALLOC);  
    if (!msg)
        return(0);
    p = (PFBYTE) DCUTODATA(msg);
    pmsg = (DCU *) p;
    *pmsg = msg;

    return ((PFBYTE) p+4);
}

/* frees memory allocated by dcu_alloc_core   */
void dcu_free_core(PFBYTE p)
{
DCU *pmsg;

    /* Free a DCU that was alloced by mf_alloc_path()   */
    p -= 4;
    pmsg = (DCU *) p;

    os_free_packet(*pmsg);
}




/* ********************************************************************   */
/* SPECIFIC LIST ROUTINES (DCUs)                                          */
/* ********************************************************************   */

#if (INCLUDE_ARP)
/* ********************************************************************   */
/* os_rcvx_arpcache_list() - remove the first DCU from arp cache list     */
/*                                                                        */
/*   This function removes the first DCU off the arpcache list at entry   */
/*   offset.                                                              */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void os_rmvx_arpcache_list(int index, DCU msg)
{
     OS_ENTER_CRITICAL(ARPRM_CLAIM_CRITICAL)

     if (os_check_list_off(tc_arpcache[index].ctrl.msg_list, (POS_LIST)msg,
                           ZERO_OFFSET))
        tc_arpcache[index].ctrl.msg_list =
            os_list_remove_off(tc_arpcache[index].ctrl.msg_list, 
                               (POS_LIST)msg, PACKET_OFFSET);
    
     OS_EXIT_CRITICAL();
}

/* ********************************************************************   */
/* os_sndx_arpcache_list() - remove the first DCU from input list         */
/*                                                                        */
/*   This function adds the DCU to the end of the doubly linked arp       */
/*   cache list at offset index.                                          */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void os_sndx_arpcache_list(int index, DCU msg)    /*__fn__*/
{
     OS_ENTER_CRITICAL(ARPS_CLAIM_CRITICAL)
     tc_arpcache[index].ctrl.msg_list = 
         os_list_add_rear_off(tc_arpcache[index].ctrl.msg_list, 
                              (POS_LIST) msg, PACKET_OFFSET);
     OS_EXIT_CRITICAL();
}

/* ********************************************************************   */
/* os_rcvx_arpcache_list() - remove the first DCU from arp cache list     */
/*                                                                        */
/*   This function removes the first DCU off the arpcache list at entry   */
/*   offset.                                                              */
/*                                                                        */
/*   Returns the removed DCU.                                             */
/*                                                                        */

DCU os_rcvx_arpcache_list(int index)               /*__fn__*/
{
DCU msg;

     msg = 0;
     OS_ENTER_CRITICAL(ARPRC_CLAIM_CRITICAL)
     if (tc_arpcache[index].ctrl.msg_list)
     {
        msg = (DCU)tc_arpcache[index].ctrl.msg_list;
        tc_arpcache[index].ctrl.msg_list = 
             os_list_remove_off(tc_arpcache[index].ctrl.msg_list, 
                                tc_arpcache[index].ctrl.msg_list, 
                                PACKET_OFFSET);
    }
    OS_EXIT_CRITICAL();
    return(msg);
}
#endif      /* INCLUDE_ARP */

/* ********************************************************************   */
/* GENERAL EXCHANGE ROUTINES                                              */
/* ********************************************************************   */

/* ********************************************************************   */
/* os_exchange_bind() - bind an exchange                                  */
/*                                                                        */
/*   This function binds an exchange to a task.  This is necessary for    */
/*   kernels which must must specify a task id when setting a signal.     */
/*   For kernels which do not require spicifying a task id, this routine  */
/*   does nothing (i.e. bind macros are defined as do nothing)            */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void os_exchange_bind(word sig_handle, PFVOID pobject)        /*__fn__*/
{
    ARGSUSED_PVOID(pobject);

    if (sig_handle &  OS_OB_IFACE)
    {
        OS_IFACE_SIGNAL_BIND(OS_HNDL_TO_SIGNAL(sig_handle), (PIFACE) pobject);
    }
#if (USE_PORT_EXCHANGE)
    else
    {
        OS_PORT_SIGNAL_BIND(OS_HNDL_TO_SIGNAL(sig_handle), pobject);
    }
#endif
}

/* ********************************************************************   */
/* os_exchange_send() - send a DCU to an exchange                         */
/*                                                                        */
/*   This function sends a DCU to an exchange, i.e. the DCU is added      */
/*   at the end of the exchange linked list and the associated signal     */
/*   is set.  Exchanges can be attached to an interface structure or a    */
/*   port structure.                                                      */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void os_exchange_send(word sig_handle, PFVOID pobject, DCU msg)        /*__fn__*/
{
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
word exchange;
#if (USE_PORT_EXCHANGE)
PANYPORT port;
#endif
PIFACE pi;
#if (DEBUG_IP_LIST)
POS_LIST list_before, list_after;
#endif

    if (!msg)       
        return;

    exchange = OS_HNDL_TO_EXCH(sig_handle); /* Index into the exch table
                                               for the IFACE or port */
    if (sig_handle & OS_OB_IFACE)           /* Put msg on the right list*/
    {
        pi = (PIFACE) pobject;
        KS_TL_SPLX(sp)  /* Disable and  push interrupts */;
#if (INCLUDE_TRK_PKTS)
        switch (sig_handle)
        {
        case IF_EX_IP:
            DCUTOPACKET(msg)->ctrl.list_id = IN_IP_LIST;
            break;
        }
#endif

#if (DEBUG_IP_LIST)
        list_before = pi->ctrl.exch_list[exchange];
#endif

        pi->ctrl.exch_list[exchange]=
            os_list_add_rear_off(pi->ctrl.exch_list[exchange], (POS_LIST) msg, 
                                 PACKET_OFFSET);
#if (DEBUG_IP_LIST)
        list_after = pi->ctrl.exch_list[exchange];
        if (!_check_ip_list(1))
        {
            print_ip_values(pi, list_before, list_after, msg);
            DEBUG_ERROR("EXCHANGE = ", EBS_INT1, exchange, 0);
        }
#endif
        KS_TL_SPL(sp)
        /* Signal the correct signal   */
        OS_IFACE_SIGNAL_SET(OS_HNDL_TO_SIGNAL(sig_handle), pi);
    }
#if (USE_PORT_EXCHANGE)
    else /* if (sig_handle & OS_OB_PORT) */    
    {
        port = (PANYPORT) pobject;
        KS_TL_SPLX(sp)  /* Disable and  push interrupts */;
#if (INCLUDE_TRK_PKTS)
        switch (sig_handle)
        {
        case PO_EX_PING:
            DCUTOPACKET(msg)->ctrl.list_id = IN_PING_LIST;
            break;
        case PO_EX_UDP:
            DCUTOPACKET(msg)->ctrl.list_id = IN_UDP_LIST;
            break;
        }
#endif
        port->ctrl.exch_list[exchange] =
            os_list_add_rear_off(port->ctrl.exch_list[exchange], (POS_LIST) msg, 
                                 PACKET_OFFSET);
        KS_TL_SPL(sp) 
        OS_PORT_SIGNAL_SET(OS_HNDL_TO_SIGNAL(sig_handle), pobject);
     }
#endif
}

/* ********************************************************************       */
/* os_exchange_rcv() - receive a DCU on an exchange                           */
/*                                                                            */
/*   This function receives a DCU to an exchange, i.e. this function          */
/*   blocks at most wait_count ticks for a DCU to be put on the exchange.     */
/*   If a DCU is put on or already on the exchange then this function removes */
/*   the DCU from the exchanges and is returned to the caller.  Exchanges     */
/*   can be attached to an interface structure or a port structure.           */
/*                                                                            */
/*   Returns the DCU from the exchange or 0 if a timeout occurs.              */
/*                                                                            */

DCU  os_exchange_rcv(word sig_handle, PFVOID pobject, word wait_count) /*__fn__*/
{
DCU msg;
word exchange;
#if (USE_PORT_EXCHANGE)
PANYPORT port;

#endif
PIFACE pi;


RTIP_BOOLEAN signalled = FALSE;

    /* first wait for a signal    */
    if (sig_handle &  OS_OB_IFACE) 
        signalled = OS_IFACE_SIGNAL_TEST(OS_HNDL_TO_SIGNAL(sig_handle), (PIFACE) pobject, wait_count);
#if (USE_PORT_EXCHANGE)
    else
        signalled = OS_PORT_SIGNAL_TEST(OS_HNDL_TO_SIGNAL(sig_handle), pobject, wait_count);
#endif

    if (!signalled)
        msg = (DCU) 0;
    else
    {
        exchange = OS_HNDL_TO_EXCH(sig_handle);   /* Index into the exch table
                                                     for the IFACE or port */
        if (sig_handle & OS_OB_IFACE)         /* Get msg from the right list*/
        {
            pi = (PIFACE) pobject;
            KS_TL_DISABLE() /* Disable interrupts */;
            msg = (DCU) pi->ctrl.exch_list[exchange];
            if (msg)
            {
                pi->ctrl.exch_list[exchange] = 
                    os_list_remove_off(pi->ctrl.exch_list[exchange], 
                                    (POS_LIST)msg, 
                                    PACKET_OFFSET);
                KS_TL_ENABLE()
            }
            else
            {
                KS_TL_ENABLE()
                DEBUG_ERROR("WARNING: os_exchange_rcv: signalled but exchange empty", NOVAR,
                    0, 0);
            }
        }
#if (USE_PORT_EXCHANGE)
        else /* if (sig_handle & OS_OB_PORT)*/
        {
            port = (PANYPORT) pobject;
            KS_TL_DISABLE() /* Disable interrupts */;
            msg = (DCU) port->ctrl.exch_list[exchange];
            if (msg)
            {
                port->ctrl.exch_list[exchange] = 
                    os_list_remove_off(port->ctrl.exch_list[exchange], (POS_LIST)msg, 
                                       PACKET_OFFSET);
                KS_TL_ENABLE()
            }
            else
            {
                KS_TL_ENABLE()
                DEBUG_ERROR("WARNING: os_exchange_rcv: port signalled but exchange empty", NOVAR,
                    0, 0);
            }

         }
#endif
    }
    return(msg);
}


/* ********************************************************************   */
/* os_exchange_clear() - clears an exchange                               */
/*                                                                        */
/*   This function removes all the DCU from and exchange and frees them.  */
/*   It also clears all the signals set for the exchange.                 */
/*                                                                        */
/*   Returns nothing.                                                     */
/*                                                                        */

void os_exchange_clear(word sig_handle, PFVOID pobject)                /*__fn__*/
{
DCU msg;
word exchange;
#if (USE_PORT_EXCHANGE)
PANYPORT port;
#endif
PIFACE pi;

    exchange = OS_HNDL_TO_EXCH(sig_handle);   /* Index into the exch table
                                                 for the IFACE or port */
    if (sig_handle & OS_OB_IFACE)
    {
        pi = (PIFACE) pobject;
        while(pi->ctrl.exch_list[exchange])
        {
            /* Note funky use of disable/enable We are reenabling 
               interrupts as often as possible to allow isrs to run */
            KS_TL_DISABLE()
            msg = (DCU) pi->ctrl.exch_list[exchange];
            if (msg)
            {
                pi->ctrl.exch_list[exchange] = 
                    os_list_remove_off(pi->ctrl.exch_list[exchange],
                                       (POS_LIST)msg, PACKET_OFFSET);
                KS_TL_ENABLE()
                /* Let ISRs run. It is safe   */
                KS_TL_DISABLE()
                os_free_packet(msg);
                KS_TL_ENABLE()
            }
            else
            {
                KS_TL_ENABLE()
            }

        }
    }
#if (USE_PORT_EXCHANGE)
    else /* if (sig_handle & OS_OB_PORT) */
    {
        port = (PANYPORT) pobject;
        while(port->ctrl.exch_list[exchange])
        {
            KS_TL_DISABLE()
            msg = (DCU) port->ctrl.exch_list[exchange];
            if (msg)
            {
                port->ctrl.exch_list[exchange] = 
                    os_list_remove_off(port->ctrl.exch_list[exchange], 
                                       (POS_LIST) msg, PACKET_OFFSET);
                KS_TL_ENABLE()
                /* Let ISRs run   */
                KS_TL_DISABLE()
                os_free_packet(msg);
                KS_TL_ENABLE()
            }
            else
            {
                KS_TL_ENABLE()
            }
        }
    }
#endif

    /* clear the correct signal   */
    if (sig_handle & OS_OB_IFACE)
    {
        OS_IFACE_SIGNAL_CLEAR(OS_HNDL_TO_SIGNAL(sig_handle), (PIFACE) pobject);
    }
#if (USE_PORT_EXCHANGE)
    else
    {
        OS_PORT_SIGNAL_CLEAR(OS_HNDL_TO_SIGNAL(sig_handle), pobject);
    }
#endif
}

/* ********************************************************************   */
/* SPECIFIC EXCHANGE ROUTINES                                             */
/* ********************************************************************   */

#if (INCLUDE_UDP || INCLUDE_RAW)

/* ********************************************************************   */
/* os_clear_udpapp_exchg() - clear the UDP exchange                       */
/*                                                                        */
/*    Frees packets on the UDP application (input packets) exchange       */
/*    and clears all associated signals.                                  */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_clear_udpapp_exchg(PUDPPORT port)           /*__fn__*/
{
    DEBUG_LOG("os_clear_udpapp_exchg called", LEVEL_3, NOVAR, 0, 0);
    port->no_udp_que = 0;
    os_exchange_clear(PO_EX_UDP, (PFVOID) port);
}

/* ********************************************************************     */
/* os_sndx_udpapp_exchg() - send IP packet to the UDP exchange              */
/*                                                                          */
/*    Queues a input UDP packet on the IP exchange and signal the exchange. */
/*    The UDP exchange is attached to the port (socket).                    */
/*                                                                          */
/*    Returns nothing                                                       */
/*                                                                          */

void os_sndx_udpapp_exchg(PUDPPORT port, DCU msg)           /*__fn__*/
{
int i;

    port->no_udp_que++;
    os_exchange_send(PO_EX_UDP, (PFVOID) port, msg);
    for (i=0; i < CFG_NUM_SELECT_P_SOCK; i++)
    {
        if ( (port->ap.ctrl.select_root[i]) && 
             (port->ap.ctrl.select_flags[i] & READ_SELECT) )
        {
#if (DEBUG_SELECT)
            DEBUG_ERROR("sndx udp exchage: set select signal", NOVAR, 0, 0);
#endif
            os_set_select_signal((PANYPORT)port, i);
        }
    }
}

/* ********************************************************************   */
/* os_udp_pkt_avail() - checks if a UDP packet is on the UDP exchange     */
/*                                                                        */
/*    Checks if a UDP IP packet is queued on the UDP exchange.            */
/*                                                                        */
/*    Returns TRUE if there are any packets on the UDP exchange or FALSE  */
/*    if the UDP exchange is empty.                                       */
/*                                                                        */

RTIP_BOOLEAN os_udp_pkt_avail(PUDPPORT pport)    /*__fn__*/
{
word exchange;

    exchange = OS_HNDL_TO_EXCH(PO_EX_UDP); /* Index into the exch table
                                               for the port */
    if (pport->ap.ctrl.exch_list[exchange])
        return(TRUE);
    else
        return(FALSE);
}

/* ********************************************************************   */
/* os_udp_first_pkt_size() - returns packet size of first UDP packet      */
/*                                                                        */
/*    Returns packet size of first packet queued on the UDP exchange.     */
/*                                                                        */
/*    Returns packet size of first packet if there are any packets on     */
/*    the UDP exchange or 0 if the UDP exchange is empty.                 */
/*                                                                        */

int os_udp_first_pkt_size(PUDPPORT pport)    /*__fn__*/
{
word exchange;
DCU msg;

    exchange = OS_HNDL_TO_EXCH(PO_EX_UDP); /* Index into the exch table
                                               for the port */
    msg = (DCU)(pport->ap.ctrl.exch_list[exchange]);
    if (msg)
        return(xn_pkt_data_size(msg, TRUE));
    else
        return(0);
}

/* ********************************************************************   */
/* os_rcvx_udpapp_exchg() - receive a IP packet from the UDP exchange     */
/*                                                                        */
/*    Dequeues a input UDP packet from the IP exchange.  If there are     */
/*    no packets on the exchange, this function will block until          */
/*    a packet is queued (i.e. until the exchange is signalled).  This    */
/*    function will block at most wait_count ticks.                       */
/*    The UDP exchange is attached to the port (socket).                  */
/*                                                                        */
/*    Returns the dequeued packet or 0 if a timeout occurs                */
/*                                                                        */

DCU os_rcvx_udpapp_exchg(PUDPPORT pport, word wait_count, RTIP_BOOLEAN delete_from_queue)   /*__fn__*/
{
DCU     msg;
RTIP_BOOLEAN signalled;

    if (!delete_from_queue)
    {
        msg = (DCU) pport->ap.ctrl.exch_list[0]; 
        if (!msg)
        {
            signalled = OS_PORT_SIGNAL_TEST(OS_HNDL_TO_SIGNAL(PO_EX_UDP), 
                                            (PFVOID)pport, wait_count);
            if (signalled)
            {
                /* resignal since MSG_PEEK; i.e. signals are one-to-one   */
                /* (one signal per packet); since we are not taking       */
                /* the packet off the exchange we need to signal again    */
                OS_PORT_SIGNAL_SET(OS_HNDL_TO_SIGNAL(PO_EX_UDP), 
                                   (PFVOID)pport);
            }
            msg = (DCU) pport->ap.ctrl.exch_list[0]; 
        }
    }
    else
    {
        msg = os_exchange_rcv(PO_EX_UDP, (PFVOID) pport, wait_count);

        if (msg)
        {
            pport->no_udp_que--;
        }
    }
    return(msg);
}

#endif /* INCLUDE_UDP || INCLUDE_RAW */

/* ********************************************************************   */
/* SIGNAL ROUTINES                                                        */
/* ********************************************************************   */

/* ********************************************************************   */
/* os_set_sent_signal() - signal the sent signal                          */
/*                                                                        */
/*    Signals the sent signal associated with a port.                     */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_set_sent_signal(PANYPORT port, RTIP_BOOLEAN success)           /*__fn__*/
{
    if (!success)
        port->ctrl.sent_status = port->ctrl.rtip_errno;
    else
        port->ctrl.sent_status = 0;    /* success */
    OS_PORT_SIGNAL_SET(OS_HNDL_TO_SIGNAL(PO_SIG_SENT), (PFVOID) port);
}

/* ********************************************************************   */
/* os_set_write_signal() - signal the write signal                        */
/*                                                                        */
/*    Signals the write signal associated with a port.                    */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_set_write_signal(PANYPORT port, RTIP_BOOLEAN success)         /*__fn__*/
{
int i;

    if (!success)
        port->ctrl.write_status = port->ctrl.rtip_errno;
    else
        port->ctrl.write_status = 0;    /* success */

    OS_PORT_SIGNAL_SET(OS_HNDL_TO_SIGNAL(PO_SIG_WRITE), (PFVOID) port);

    for (i=0; i < CFG_NUM_SELECT_P_SOCK; i++)
    {
        if ( (port->ctrl.select_root[i]) )
        {
            if (success)
            {
                if ( (port->ctrl.select_flags[i] & WRITE_SELECT) &&
                     (((PTCPPORT)port)->interp_port_flags & SELECT_WRITE_WAKEUP) )
                {
#if (DEBUG_SELECT)
                DEBUG_ERROR("os_set_write_signal: port, index", EBS_INT2,
                    port->ctrl.index, i);
#endif
                    os_set_select_signal(port, i);
                }
            }
            else
            {          /* error */
                if ( (port->ctrl.select_flags[i] & 
                      (WRITE_SELECT|EXCEPTION_SELECT)) &&
                     (((PTCPPORT)port)->interp_port_flags & 
                      SELECT_WRITE_WAKEUP) )
                {
#if (DEBUG_SELECT)
                    DEBUG_ERROR("os_set_write_signal: ERROR: port, index", 
                        EBS_INT2, port->ctrl.index, i);
#endif
                    os_set_select_signal(port, i);
                }
            }       /* end of if (success) else */
        }           /* end of if (port->ctrl.select_root) */
    }               /* end of for loop */
}

/* ********************************************************************   */
/* os_set_read_signal() - signal the read signal                          */
/*                                                                        */
/*    Signals the read signal associated with a port.                     */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_set_read_signal(PANYPORT port, RTIP_BOOLEAN success)          /*__fn__*/
{
int i;

    if (!success)
        port->ctrl.read_status = port->ctrl.rtip_errno;
    else
        port->ctrl.read_status = 0;    /* success */

    OS_PORT_SIGNAL_SET(OS_HNDL_TO_SIGNAL(PO_SIG_READ), (PFVOID) port);

    for (i=0; i < CFG_NUM_SELECT_P_SOCK; i++)
    {
        if (port->ctrl.select_root[i])
        {
           if (success)
           {
                if ( port->ctrl.select_flags[i] & READ_SELECT )
                {
#if (DEBUG_SELECT)
                    DEBUG_ERROR("os_set_read_signal: port, index", EBS_INT2,
                        port->ctrl.index, i);
#endif
                    os_set_select_signal(port, i);
                }
           }
           else
           {   /* error */
                if ( port->ctrl.select_flags[i] & (READ_SELECT | EXCEPTION_SELECT) )
                {
#if (DEBUG_SELECT)
                    DEBUG_ERROR("os_set_read_signal: ERROR: port, index", EBS_INT2,
                        port->ctrl.index, i);
#endif
                    os_set_select_signal(port, i);
               }
            }   /* end of if (success) else */
        }       /* end of if (port->select_root) */
    }           /* end of for loop */
}

/* ********************************************************************   */
/* os_set_select_signal() - signal the select signal                      */
/*                                                                        */
/*    Signals the select signal associated with a port.                   */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_set_select_signal(PANYPORT port, int select_off)          /*__fn__*/
{
PANYPORT root_port;

    root_port = port->ctrl.select_root[select_off];
    if (root_port)
    {
#if (DEBUG_SELECT)
        DEBUG_ERROR("os_set_select_signal: port, index", EBS_INT2,
            port->ctrl.index, select_off);
        DEBUG_ERROR("set select signal - root port", NOVAR, 0, 0);
#endif
        OS_PORT_SIGNAL_SET(OS_HNDL_TO_SIGNAL(PO_SIG_SELECT+select_off), 
                                             (PFVOID)root_port);
    }
}

/* ********************************************************************   */
/* PORT AND TCP WINDOW ALLOCATION ROUTINES                                */
/* ********************************************************************   */

#if (INCLUDE_UDP || INCLUDE_RAW)
/* ********************************************************************   */
/* os_alloc_udpport() - allocate a UDP port (socket)                      */
/*                                                                        */
/*    Allocates a UDP port from the free list.  Performs any required     */
/*    initialization including clearing all the fields and signals        */
/*    associated with the port.  Then it sets up some of the fields       */
/*    such as port type and index.                                        */
/*                                                                        */
/*    Returns the allocated port                                          */
/*                                                                        */

PUDPPORT os_alloc_udpport(void)                                     /*__fn__*/
{
PUDPPORT port;
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
int      i;
#elif (INCLUDE_MALLOC_PORTS)
int      i;
#endif
int      index;
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
KS_RTIPSIG save_portsig[NUM_SIG_PER_PORT];
#endif

    OS_ENTER_CRITICAL(ALLOCU_CLAIM_CRITICAL)
#if (!INCLUDE_MALLOC_PORTS)
    DEBUG_LOG("os_alloc_udpport - enter - root_udp_port = ", LEVEL_3,
        DINT1, root_udp_port, 0);
#endif

    port = (PUDPPORT)0;
#if (!INCLUDE_MALLOC_PORTS)
    if (root_udp_port)
    {
        port = (PUDPPORT) root_udp_port;
        root_udp_port = os_list_remove_off(root_udp_port, root_udp_port, 
                                           ZERO_OFFSET);
    }
#else
#if (INCLUDE_MALLOC_PORT_LIMIT)
    MALLOC_WITH_LIMIT(port, sizeof(UDPPORT), total_sockets, UDPPORT_MALLOC)
#else
    port = (PUDPPORT)ks_malloc(1, sizeof(UDPPORT), UDPPORT_MALLOC);
#endif

    /* find a free port in alloced ports   */
    for (i = FIRST_UDP_PORT; i < FIRST_UDP_PORT+TOTAL_UDP_PORTS; i++)
    {
        if (!alloced_ports[i])
        {
            port->ap.ctrl.index = i;
            alloced_ports[i] = (PANYPORT)port;
            break;
        }
    }
    if (i >= FIRST_UDP_PORT+TOTAL_UDP_PORTS)
    {
#if (INCLUDE_MALLOC_PORT_LIMIT)
        FREE_WITH_LIMIT(port, sizeof(UDPPORT), total_sockets);
#else
        ks_free((PFBYTE)port, sizeof(UDPPORT), 1);
#endif
        port = (PUDPPORT)0;
    }
#endif

    if (port)
    {
#if (!INCLUDE_MALLOC_PORTS)
        DEBUG_LOG("os_alloc_udpport - just removed - root_udp_port = ", LEVEL_3,
            DINT1, root_udp_port, 0);
#endif
        /* zero the port structure (which is required by other parts   */
        /* of the stack but certain fields cannot be cleared so they   */
        /* are saved and restored                                      */
        index = port->ap.ctrl.index;     /* save index before clear */
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
        /* need to attach signals to port so can free them   */
        for (i=0; i < NUM_SIG_PER_PORT; i++)
            save_portsig[i] = port->ap.portsig[i];
#endif
        tc_memset((PFBYTE) port, 0, sizeof(*port));         /* (required) */

        /* set port type; assume UDP (used for RAW also) so if INCLUDE_RAW   */
        /* is not set only need to do this once (i.e. not every time the     */
        /* socket is allocated)                                              */
        port->ap.port_type = UDPPORTTYPE;                 
        port->ap.ctrl.index = index;     /* restore index */
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
        /* need to attach signals to port so can free them   */
        for (i=0; i < NUM_SIG_PER_PORT; i++)
            port->ap.portsig[i] = save_portsig[i];
#endif
        /* save in structure so can convert socket integer to port structure   */
        alloced_ports[index] = (PANYPORT)port;

#if (INCLUDE_MALLOC_PORTS)
        for (i = 0; i < NUM_SIG_PER_PORT; i++)
        {
            /* Create write, read, send & select signals for each port   */
            KS_SIGNAL_BUILD(port->ap.portsig[i])
        }
#endif

        /* Now "bind" the current task to the udp port so kernels that   */
        /* need a destination task id for signals will work..            */
        OS_BIND_SENT_SIGNAL((PANYPORT)port);
        OS_BIND_WRITE_SIGNAL((PANYPORT) port);
        /* NOTE: caller will set list type   */

        /* clear signals   */
        OS_CLEAR_WRITE_SIGNAL((PANYPORT) port);
        /* Not needed right now since arp is asynchronous   */
        /* will need it later                               */
        OS_CLEAR_SENT_SIGNAL((PANYPORT) port);
    }
    else
        port = (PUDPPORT) 0;
    OS_EXIT_CRITICAL();

    /* set up udp exchange after exiting critical since exchange routines   */
    /* use enter and exit critical also                                     */
    if (port)
    {
        OS_BIND_UDPAPP_EXCHG(port);
        os_clear_udpapp_exchg(port);   
    }
    return(port);
}

/* ********************************************************************   */
/* os_free_udpport() - frees a UDP port                                   */
/*                                                                        */
/*    Frees a UDP port by putting in on the free list of UDP ports        */
/*    (root_udp_port).                                                    */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_free_udpport(PUDPPORT port)                               /*__fn__*/
{
#if (INCLUDE_MALLOC_PORTS)
int i;
#endif

    OS_ENTER_CRITICAL(FREEU_CLAIM_CRITICAL)
    alloced_ports[port->ap.ctrl.index] = (PANYPORT)0;
    port->ap.port_flags = 0;                /* reset bound flag */
    port->udp_connection.udp_source = 0;    /* reset port number  */

#if (!INCLUDE_MALLOC_PORTS)
    root_udp_port = os_list_add_front_off(root_udp_port, (POS_LIST) port, 
                                          ZERO_OFFSET);
#else
    for (i = 0; i < NUM_SIG_PER_PORT; i++)
    {
        /* Create write, read, send & select signals for each port   */
        KS_SIGNAL_DELETE(port->ap.portsig[i]);
    }
#if (INCLUDE_MALLOC_PORT_LIMIT)
    FREE_WITH_LIMIT(port, sizeof(UDPPORT), total_sockets);
#else
    ks_free((PFBYTE)port, 1, sizeof(UDPPORT));
#endif
#endif
    OS_EXIT_CRITICAL();
}
#endif

#if (INCLUDE_TCP)
/* ********************************************************************   */
/* os_alloc_tcpport() - allocate a TCP port (socket)                      */
/*                                                                        */
/*    Allocates a TCP port from the free list.  Performs any required     */
/*    initialization including clearing all the fields and signals        */
/*    associated with the port.  Then it sets up some of the fields       */
/*    such as port type and index.                                        */
/*                                                                        */
/*    Returns the allocated port                                          */
/*                                                                        */

PTCPPORT os_alloc_tcpport(void)                                       /*__fn__*/
{
PTCPPORT port;
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
int      i;
#elif (INCLUDE_MALLOC_PORTS)
int      i;
#endif
int      index;
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
KS_RTIPSIG save_portsig[NUM_SIG_PER_PORT];
#endif

    OS_ENTER_CRITICAL(ALLOCT_CLAIM_CRITICAL)
    port = (PTCPPORT)0;
#if (!INCLUDE_MALLOC_PORTS)
    if (root_tcp_port)
    {
        port = (PTCPPORT)root_tcp_port;
        root_tcp_port = os_list_remove_off(root_tcp_port, root_tcp_port, 
                                           ZERO_OFFSET);
    }
#else
#if (INCLUDE_MALLOC_PORT_LIMIT)
    MALLOC_WITH_LIMIT(port, sizeof(TCPPORT), total_sockets, TCPPORT_MALLOC)
#else
    port = (PTCPPORT)ks_malloc(1, sizeof(TCPPORT), TCPPORT_MALLOC);
#endif

    /* find a free port in alloced ports   */
    for (i = FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
    {
        if (!alloced_ports[i])
        {
            port->ap.ctrl.index = i;
            alloced_ports[i] = (PANYPORT)port;
            break;
        }
    }

    if (i >= TOTAL_TCP_PORTS)
    {

        DEBUG_ERROR("os_alloc_tcpport: out of alloced_ports", NOVAR, 0, 0);

#if (INCLUDE_MALLOC_PORT_LIMIT)
        FREE_WITH_LIMIT(port, sizeof(TCPPORT), total_sockets);
#else
        ks_free((PFBYTE)port, 1, sizeof(TCPPORT));
#endif
        port = (PTCPPORT)0;
    }
#endif

    if (port)
    {
        /* zero the port structure (which is required by other parts   */
        /* of the stack but certain fields cannot be cleared so they   */
        /* are saved and restored                                      */
        index = port->ap.ctrl.index;
#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
        /* need to attach signals to port so can free them   */
        for (i=0; i < NUM_SIG_PER_PORT; i++)
            save_portsig[i] = port->ap.portsig[i];
#endif

        tc_memset((PFBYTE) port, 0, sizeof(*port)); 

        port->ap.port_type = TCPPORTTYPE;          
        port->ap.ctrl.index = index;               

#if (INCLUDE_RUN_TIME_CONFIG && !INCLUDE_MALLOC_PORTS)
        /* need to attach signals to port so can free them   */
        for (i=0; i < NUM_SIG_PER_PORT; i++)
            port->ap.portsig[i] = save_portsig[i];
#endif

        /* save in structure so can convert socket integer to port structure   */
        alloced_ports[index] = (PANYPORT)port;

#if (INCLUDE_MALLOC_PORTS)
        for (i = 0; i < NUM_SIG_PER_PORT; i++)
        {
            /* Create write, read, send & select signals for each port   */
            KS_SIGNAL_BUILD(port->ap.portsig[i])
        }
#endif

        /* bind signals   */
        OS_BIND_READ_SIGNAL((PANYPORT) port);
        OS_BIND_WRITE_SIGNAL((PANYPORT) port);
        /* Not needed right now since arp is asynchronous   */
        /* will need it later                               */
        OS_BIND_SENT_SIGNAL((PANYPORT) port);

        /* clear signals   */
        OS_CLEAR_READ_SIGNAL((PANYPORT) port);
        OS_CLEAR_WRITE_SIGNAL((PANYPORT) port);
        /* Not needed right now since arp is asynchronous   */
        /* will need it later                               */
        OS_CLEAR_SENT_SIGNAL((PANYPORT) port);

        /* a socket is allocated, so set timer frequency based upon config   */
        /* parameter                                                         */
        timer_freq = CFG_TIMER_FREQ;
    }
    else
        port = (PTCPPORT)0;
    OS_EXIT_CRITICAL();
    /* DEBUG_LOG("os_alloc_tcpport - port = ", LEVEL_3, LINT1, (long)port, 0);   */

    return(port);
}

/* ********************************************************************   */
/* os_free_tcpport() - frees a TCP port                                   */
/*                                                                        */
/*    Frees a TCP port by putting in on the free list of TCP ports        */
/*    (root_tcp_port).                                                    */
/*                                                                        */
/*    Returns nothing                                                     */
/*                                                                        */

void os_free_tcpport(PTCPPORT port)                              /*__fn__*/
{
#if (INCLUDE_MALLOC_PORTS)
int i;
#endif

    OS_ENTER_CRITICAL(FREET_CLAIM_CRITICAL)

    alloced_ports[port->ap.ctrl.index] = (PANYPORT)0;
    port->state = TCP_S_FREE;
    port->ap.port_flags = 0;        /* reset bound flag */

#if (!INCLUDE_MALLOC_PORTS)
    root_tcp_port = os_list_add_front_off(root_tcp_port, (POS_LIST) port, 
                                          ZERO_OFFSET);
#else
    for (i = 0; i < NUM_SIG_PER_PORT; i++)
    {
        /* Create write, read, send & select signals for each port   */
        KS_SIGNAL_DELETE(port->ap.portsig[i]);
    }
#if (INCLUDE_MALLOC_PORT_LIMIT)
    FREE_WITH_LIMIT(port, sizeof(TCPPORT), total_sockets);
#else
    ks_free((PFBYTE)port, 1, sizeof(TCPPORT));
#endif
#endif

    OS_EXIT_CRITICAL();
}
#endif

#if (INCLUDE_TRK_PKTS)

/* ********************************************************************   */
/* TRACKING PACKETS ROUTINES - DEBUG ROUTINES (normally disabled;         */
/*                             (see xnconf.h)                             */
/* ********************************************************************   */

void track_frags(PTRACK_DCU pdcu_track_info);
void track_udp_exch(PTRACK_DCU pdcu_track_info);
void track_window(PTRACK_DCU pdcu_track_info);
void track_input_list(PTRACK_DCU pdcu_track_info);
void track_output_list(PTRACK_DCU pdcu_track_info);
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
void track_free(PTRACK_DCU pdcu_track_info);
#endif
void track_ip_exch(PTRACK_DCU pdcu_track_info);
void track_arp_cache(PTRACK_DCU pdcu_track_info);
void track_rs232_info(PTRACK_DCU pdcu_track_info);
void display_packet_info(PTRACK_DCU pdcu_track_info);

/* ********************************************************************   */
/* DCU TABLE ROUTINES                                                     */
/* ********************************************************************   */
/* DCU TABLE keeps track if each packet is accounted for                  */
RTIP_BOOLEAN alloc_dcu_table(PTRACK_DCU pdcu_track_info)
{
int tdcu;

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    tdcu = current_alloc_packets;
#else
    tdcu = CFG_NDCUS;
#endif

    pdcu_track_info->dcu_table_ptr = 
        (RTIP_BOOLEAN *)ks_malloc(sizeof(RTIP_BOOLEAN), tdcu,
                                  DCU_TABLE_MALLOC);
    if (!pdcu_track_info->dcu_table_ptr)
        return(FALSE);
    pdcu_track_info->total_dcu_cnt = tdcu;
    return(TRUE);
}

void free_dcu_table(PTRACK_DCU pdcu_track_info)
{
    ks_free((PFBYTE)pdcu_track_info->dcu_table_ptr, 1,
            pdcu_track_info->total_dcu_cnt);
}

void update_dcu_table(DCU msg, PTRACK_DCU pdcu_track_info)
{
int index;

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    DCUTOPACKET(msg)->ctrl.index = pdcu_track_info->dcu_cnt;
    pdcu_track_info->dcu_cnt++;
#endif

    index = DCUTOPACKET(msg)->ctrl.index;
    if (index < ndcus_alloced)
    {
        if (pdcu_track_info->dcu_table_ptr[index] == TRUE)
        {
            DEBUG_ERROR("OOPS: DCU in two lists: who = ", EBS_INT1,
                DCUTOCONTROL(msg).requestor, 0);
        }
        pdcu_track_info->dcu_table_ptr[index] = TRUE;
    }
    else
    {
        DEBUG_ERROR("update_dcu_table - out of range index = ", EBS_INT1,
            index, 0);

    }
}

/* ********************************************************************   */
/* xn_stats_gather_packet_info() - Gather packet (DCU) information        */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   void xn_stats_gather_packet_info(display)                            */
/*       PTRACK_DCU pdcu_track_info - packet info stored in here          */
/*       RTIP_BOOLEAN display - set to TRUE to display packet info via    */
/*                              DEBUG_ERROR                               */
/*                                                                        */
/* tracks all the packets (DCUs) by searching thru all the queues and     */
/* keeping track of which queue they are on.  Then loops thru all the     */
/* packets to check that all the packets are accounted for                */
/*                                                                        */
/* if the parameter display is TRUE, a summary of the queues the packets  */
/* are on will be displayed (using DEBUG_ERROR).  In either case,         */
/* it will print a error message (using DEBUG_LOG) if any packets are     */
/* lost                                                                   */
/*                                                                        */
/* Fills in structure of gathered information; the num_dropped field can  */
/* be checked to see if any packets are lost                              */
/*                                                                        */
/* Returns 0 upon success; -1 upon error                                  */

int xn_stats_gather_packet_info(PTRACK_DCU pdcu_track_info, RTIP_BOOLEAN display)
{
int i;
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
int who;
int dcu_flags;
#endif
    if (!pdcu_track_info)
    {
        DEBUG_ERROR("xn_stats_packet_gather_info: bad parameter", 
            NOVAR, 0, 0);
        return(-1);
    }
    tc_memset(pdcu_track_info, 0, sizeof(struct track_dcu));

#if (INCLUDE_MALLOC_DCU_AS_NEEDED)
    ndcus_alloced = current_alloc_packets;
    pdcu_track_info->dcu_cnt = 0;
#endif

    /* allocate a table to track the DCUs   */
    if (!alloc_dcu_table(pdcu_track_info))
    {
        DEBUG_ERROR("xn_stats_packet_gather_info: alloc of dcu table failed", 
            NOVAR, 0, 0);
        return(-1);
    }

    for (i = 0; i < ndcus_alloced; i++)
        pdcu_track_info->dcu_table_ptr[i] = FALSE;

    /* **************************************************   */
    track_frags(pdcu_track_info);
    track_udp_exch(pdcu_track_info);
    track_window(pdcu_track_info);
    track_input_list(pdcu_track_info);
    track_output_list(pdcu_track_info);
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
    track_free(pdcu_track_info);
#endif
    track_ip_exch(pdcu_track_info);
    track_arp_cache(pdcu_track_info);
    track_rs232_info(pdcu_track_info);

    /* **************************************************   */
    /* count lost packets                                   */
    /* **************************************************   */
    pdcu_track_info->num_dropped = 0;
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
    for (i = 0; i < ndcus_alloced; i++)
    {
        if (!pdcu_track_info->dcu_table_ptr[i])
        {
            pdcu_track_info->num_dropped++;
            who       = dcu_pool_ptr[i].ctrl.requestor;
            dcu_flags = dcu_pool_ptr[i].ctrl.dcu_flags;
        }
    }
#endif

    if (display)
    {
        display_packet_info(pdcu_track_info);
    }
    free_dcu_table(pdcu_track_info);
    return(0);
}

/* ********************************************************************   */
/* DISPLAY USING DEBUG_ERROR                                              */
/* ********************************************************************   */
void display_packet_info(PTRACK_DCU pdcu_track_info)
{
int i;
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
int who;
int dcu_flags;
#endif

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
    for (i = 0; i < ndcus_alloced; i++)
    {
        if (!pdcu_track_info->dcu_table_ptr[i])
        {
            who       = dcu_pool_ptr[i].ctrl.requestor;
            dcu_flags = dcu_pool_ptr[i].ctrl.dcu_flags;

            DEBUG_ERROR("OOPS - lost packet; index, who = ", 
                EBS_INT2, i, who);
            DEBUG_ERROR("OOPS - lost packet; dcu_flags  = ", 
                EBS_INT1, dcu_flags, 0);
        }
    }       /* end of for loop */
    if (pdcu_track_info->num_dropped > 0)
    {
        DEBUG_ERROR("OOPS - number of lost packets = ", EBS_INT1, 
            pdcu_track_info->num_dropped, 0);
    }
    else
    {
        DEBUG_ERROR("all packets accounted for", NOVAR, 0, 0);
    }
#endif  /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

    if (pdcu_track_info->frag_tbl > 0)
    {
        DEBUG_ERROR("fragment table's number of packets = ",    EBS_INT1, pdcu_track_info->frag_tbl, 0);
    }
    if (pdcu_track_info->out_wind > 0)
    {
        DEBUG_ERROR("TCP output window's number of packets = ", EBS_INT1, pdcu_track_info->out_wind, 0);
    }
    if (pdcu_track_info->in_wind > 0)
    {
        DEBUG_ERROR("TCP input window's number of packets = ",  EBS_INT1, pdcu_track_info->in_wind, 0);
    }
    if (pdcu_track_info->ip_exch  > 0)
    {
        DEBUG_ERROR("IP exchange's number of packets = ",       EBS_INT1, pdcu_track_info->ip_exch , 0);
    }
    if (pdcu_track_info->udp_exch > 0)
    {
        DEBUG_ERROR("UDP exchange number of packets = ",        EBS_INT1, pdcu_track_info->udp_exch, 0);
    }
    if (pdcu_track_info->ping_exch > 0)
    {
        DEBUG_ERROR("PING exchange's number of packets = ",     EBS_INT1, pdcu_track_info->ping_exch, 0);
    }
    if (pdcu_track_info->input_list > 0)
    {
        DEBUG_ERROR("INPUT list's number of packets = ",        EBS_INT1, pdcu_track_info->input_list, 0);
    }
    if (pdcu_track_info->output_list > 0)
    {
        DEBUG_ERROR("OUTPUT list's number of packets = ",       EBS_INT1, pdcu_track_info->output_list, 0);
    }
    if (pdcu_track_info->arp_cache > 0)
    {
        DEBUG_ERROR("ARP cache's number of packets = ",         EBS_INT1, 
            pdcu_track_info->arp_cache, 0);
    }
    if (pdcu_track_info->rs232_info > 0)
    {
        DEBUG_ERROR("RS232 INFO array's number of packets = ",  EBS_INT1, 
            pdcu_track_info->rs232_info, 0);
    }
    if (pdcu_track_info->free_list > 0)
    {
        DEBUG_ERROR("FREE packets = ",                          EBS_INT1, 
            pdcu_track_info->free_list, 0);
    }

#if (TRACK_WAIT)
    tm_cputs("Press any key to continue . . .");
    tm_getch();
#endif

}

/* ********************************************************************   */
void display_xmit_info(void)
{
int i;
PIFACE pi;

    LOOP_THRU_IFACES(i)
    {
        pi = tc_ino2_iface(i, DONT_SET_ERRNO);
        if (pi)
        {
#if (INCLUDE_XMIT_QUE)
            DEBUG_ERROR("XMIT DCU: interface, xmit dcu", DINT2, 
                         i, pi->ctrl.list_xmit);
#else
            DEBUG_ERROR("XMIT DCU: interface, xmit_dcu", DINT2, 
                         i, pi->xmit_dcu);
#endif
            DEBUG_ERROR("XMIT DCU: xmit_done_counter, xmit_done_timer", EBS_INT2, 
                        pi->xmit_done_counter, pi->xmit_done_timer);
        }
    }
}   

/* ********************************************************************   */
/* ********************************************************************   */
void cnt_pos_list(int *cnt, DCU msg, DCU root_msg, PTRACK_DCU pdcu_track_info)
{
POS_LIST entry;
DCU frag_msg;

    if (!msg)
        return;

    do          /* loop thru the pos_list */
    {
        /* loop thru fragments - only udpapp exchange (UDP input) and TCP input   */
        /* window can have fragments but it is easiest to do it for all           */
        frag_msg = msg;
        while (frag_msg)
        {
            (*cnt)++;
            if (pdcu_track_info)
                update_dcu_table(frag_msg, pdcu_track_info);
#            if (INCLUDE_FRAG)
                frag_msg = DCUTOPACKET(frag_msg)->frag_next;
#            else
                frag_msg = (DCU)0;
#            endif

        }

        entry = (POS_LIST)DCUTOPACKET(msg);
        entry = entry->pnext;
        msg = (DCU)entry;
    } while (msg != root_msg);
}


/* ********************************************************************   */
void track_frags(PTRACK_DCU pdcu_track_info)
{
#if (INCLUDE_FRAG)
int i;
DCU msg;

    for (i=0; i < CFG_FRAG_TABLE_SIZE; i++)
    {
        /* if entry is empty   */
        if (!frag_table[i].ipf_next)
            continue;

        msg = frag_table[i].ipf_next;

        while (msg)
        {
            pdcu_track_info->frag_tbl++;
            update_dcu_table(msg, pdcu_track_info);
            msg = DCUTOPACKET(msg)->frag_next;
        }
    }
#endif
}

/* ********************************************************************   */
void track_udp_exch(PTRACK_DCU pdcu_track_info)
{
#if (INCLUDE_UDP)
int i;
DCU msg;
DCU root_msg;
PANYPORT port;
word exchange;

    /* UDP PORTS (NOTE: includes PING exchange also)   */
    exchange = OS_HNDL_TO_EXCH(PO_EX_UDP); 

#if (!INCLUDE_MALLOC_PORTS)
    for (i = 0; i < CFG_NUDPPORTS; i++)
    {
        port = (PANYPORT) &tc_udpp_pool[i];
        root_msg = msg = (DCU) port->ctrl.exch_list[exchange];
        cnt_pos_list(&pdcu_track_info->udp_exch, msg, root_msg, pdcu_track_info);
    }
#else
    for (i = 0; i < TOTAL_PORTS; i++)
    {
        if (alloced_ports[i])
        {
            port = (PANYPORT)alloced_ports[i];
            if (port->port_type == UDPPORTTYPE)
            {
                root_msg = msg = (DCU) port->ctrl.exch_list[exchange];
                cnt_pos_list(&pdcu_track_info->udp_exch, msg, root_msg, pdcu_track_info);
            }
        }
    }

#endif
#endif
}

/* ********************************************************************   */
void track_window(PTRACK_DCU pdcu_track_info)
{
#if (INCLUDE_TCP)
int i;
DCU msg;
PTCPPORT port;

    /* TCP PORTS   */
    for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
    {
        port = (PTCPPORT)(alloced_ports[i]);
        if (!port || !IS_TCP_PORT(port))
            continue;

        msg = (DCU)(port->out.dcu_start); 
        while (msg)
        {
            pdcu_track_info->out_wind++;
            update_dcu_table(msg, pdcu_track_info);
            msg = (DCU)
                os_list_next_entry_off((POS_LIST)port->out.dcu_start,
                                       (POS_LIST)msg,
                                        TCP_WINDOW_OFFSET);
        }

        msg = (DCU)(port->in.dcu_start); 
        while (msg)
        {
            DEBUG_ERROR("TCP INPUT WINDOW: port index, state = ", EBS_INT2,
                port->ap.ctrl.index, port->state);
            pdcu_track_info->in_wind++;
            update_dcu_table(msg, pdcu_track_info);
            msg = (DCU)
                os_list_next_entry_off((POS_LIST)port->in.dcu_start,
                                       (POS_LIST)msg,
                                        TCP_WINDOW_OFFSET);
        }
    }
#endif
}

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
/* ********************************************************************   */
void track_free(PTRACK_DCU pdcu_track_info)
{
DCU msg;
DCU root_msg;
KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */
int i;
int temp;


    KS_TL_SPLX(sp)  /* Disable and  push interrupts */

    temp = 0;

    for ( i = 0; i < CFG_NUM_FREELISTS; i++)
    {
        msg = root_msg = (DCU)root_dcu_array[i];
        if (msg)
        {
            temp = 0;
            cnt_pos_list(&temp, msg, root_msg, pdcu_track_info);
/*          temp += pdcu_track_info->free_list;   */
            pdcu_track_info->free_list += temp;
        }
    }

    KS_TL_SPL(sp)
}
#endif /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

/* ********************************************************************   */
void track_ip_exch(PTRACK_DCU pdcu_track_info)
{
int iface_off;
DCU msg;
DCU root_msg;
word ex1;
PIFACE pi;
int cnt = 0;

    ex1 = OS_HNDL_TO_EXCH(IF_EX_IP); 

    pdcu_track_info->ip_exch = 0;
    LOOP_THRU_IFACES(iface_off)
    {
        PI_FROM_OFF(pi, iface_off)
        if (pi)
        {
            root_msg = msg = (DCU) pi->ctrl.exch_list[ex1];
            cnt_pos_list(&cnt, msg, root_msg, pdcu_track_info);
        }
        pdcu_track_info->ip_exch += cnt;
    }
}

/* ********************************************************************   */
void track_input_list(PTRACK_DCU pdcu_track_info)
{
int iface_off;
DCU msg;
DCU root_msg;
PIFACE pi;

    LOOP_THRU_IFACES(iface_off)
    {
        PI_FROM_OFF(pi, iface_off)
        if (pi)
        {
            root_msg = msg = (DCU) pi->ctrl.list_input;
            cnt_pos_list(&pdcu_track_info->input_list, msg, root_msg, pdcu_track_info);
        }
    }
}

/* ********************************************************************   */
void track_output_list(PTRACK_DCU pdcu_track_info)
{
int iface_off;
DCU msg;
PIFACE pi;

    for (iface_off = 0; iface_off < CFG_NIFACES; iface_off++)
    {
        PI_FROM_OFF(pi, iface_off)
        if (pi)
        {
            msg = (DCU) pi->ctrl.list_output;
            while (msg)
            {
                pdcu_track_info->output_list++;
                update_dcu_table(msg, pdcu_track_info);
                msg = (DCU)
                    os_list_next_entry_off((POS_LIST)pi->ctrl.list_output,
                                           (POS_LIST)msg,
                                            OUTPUT_LIST_OFFSET);
            }
        }
    }
}

/* ********************************************************************   */
void track_arp_cache(PTRACK_DCU pdcu_track_info)
{
#if (INCLUDE_ARP)
int i;
DCU msg;
DCU root_msg;

    for (i=0; i < CFG_ARPCLEN; i++) 
    {
        msg = root_msg = (DCU)tc_arpcache[i].ctrl.msg_list;
        cnt_pos_list(&pdcu_track_info->arp_cache, msg, root_msg, pdcu_track_info);
    }
#endif
}

/* ********************************************************************   */
void track_rs232_info(PTRACK_DCU pdcu_track_info)
{
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP)
int i;
DCU msg;

    for (i=0; i < CFG_NUM_RS232; i++) 
    {
        msg = rs232_if_info_arry[i].msg_in;
        if (msg)
        {
            pdcu_track_info->rs232_info++;
            update_dcu_table(msg, pdcu_track_info);
        }
    }
#else
    ARGSUSED_PVOID(pdcu_track_info);
#endif
}

/* ********************************************************************   */
/* TRACK SEMAPHORE STUFF                                                  */
/* NOTE: only one semaphore tracked for each type, for example,           */
/*       if using more than one IFACE, it doesn't work.  Need             */
/*       to track per iface                                               */
/* ********************************************************************   */

#define NUM_SEMS 10  /* 10 = room for 4 interfaces; add one for each new  */
                     /* interface opened   */
#define NUM_TRACK_PEND 9
int KS_FAR sem_who_claim[NUM_SEMS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int KS_FAR sem_who_pend[NUM_SEMS][NUM_TRACK_PEND] = 
{
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* TABLE */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* UDP */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* TCP */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* CRITICAL */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* MEMFILE */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* SYSLOG */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* IFACE - 0 */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* IFACE - 1 */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* IFACE - 2 */
    {-1,-1,-1,-1,-1,-1,-1,-1,-1},       /* IFACE - 3 */
};

void break_track(void)
{
}

/* who just claimed sem_no   */
void track_sem_claim(int who, int sem_no, int offset)
{
int i;

    if (resource_initialized != INIT_DONE)
        return;

    if ( (sem_no + offset) >= NUM_SEMS)
    {
        DEBUG_ERROR("track_sem_claim: increase NUM_SEMS in os.c", EBS_INT2, sem_no, offset);
        return;
    }

#if (DEBUG_IFACE_SEM)
    if (sem_no == IFACE_CLAIM)
    {
        DEBUG_ERROR("who just claimed sem_no ", EBS_INT2, who, sem_no);
    }
#endif

    /* take who out of pending list   */
    for (i=0; i<NUM_TRACK_PEND; i++)
    {
        if (sem_who_pend[sem_no+offset][i] == who)
        {
            sem_who_pend[sem_no+offset][i] = -1;
            break;
        }
    }
    /* check semaphore is not working correctly   */
    if (sem_who_claim[sem_no+offset] != -1)
    {
        DEBUG_ERROR("track_sem_claim: claimed sem which is already claimed: sem:",
            EBS_INT1, sem_no+offset, 0);
        DEBUG_ERROR("                 : set break point at debug_track()",
            NOVAR, 0, 0);
        break_track();
    }

    /* put who in the claimed list   */
    sem_who_claim[sem_no+offset] = who;
}

/* who is about to claim sem_no   */
void track_sem_pend(int who, int sem_no, int offset)
{
int i;

    if ( (sem_no + offset) >= NUM_SEMS)
    {
        DEBUG_ERROR("track_sem_pend: increase NUM_SEMS in os.c", EBS_INT2, sem_no, offset);
        return;
    }

#if (DEBUG_IFACE_SEM)
    if (sem_no == IFACE_CLAIM)
    {
        DEBUG_ERROR("who is TRYING claim sem_no", EBS_INT2, who, sem_no+offset);    
    }
#endif
    if (resource_initialized != INIT_DONE)
        return;

    for (i=0; i<NUM_TRACK_PEND; i++)
    {
        if (sem_who_pend[sem_no+offset][i] == -1)
        {
            sem_who_pend[sem_no+offset][i] = who;
            return;
        }
    }
    DEBUG_ERROR("track_sem_pend: NUM_TRACK_PEND is not large enough", NOVAR,
        0, 0);
}

/* sem_no about to be released   */
void track_sem_rel(int sem_no, int offset)
{
int who;

    if (resource_initialized != INIT_DONE)
        return;

    if ( (sem_no + offset) >= NUM_SEMS)
    {
        DEBUG_ERROR("track_sem_rel: increase NUM_SEMS in os.c", EBS_INT2, sem_no, offset);
        return;
    }

    who = sem_who_claim[sem_no+offset];
    sem_who_claim[sem_no+offset] = -1;
#if (DEBUG_IFACE_SEM)
    if (sem_no == IFACE_CLAIM)
    {
        DEBUG_ERROR("releasing sem_no by who ", EBS_INT2, sem_no+offset, who);
    }
#endif
}

void display_sem_info(void)
{
int i;
int sem_no;

    /* claimed and pending calls to claim sems    */
    /* tbd - offset is number of iface structures */
    for (sem_no=0; sem_no<NUM_SEMS; sem_no++)
    {
        if (sem_who_claim[sem_no] < 0)
        {
/*          DEBUG_ERROR("sem NOT claimed", EBS_INT1, sem_no, 0);   */
        }
        else
        {
            DEBUG_ERROR("sem claimed: sem, who", EBS_INT2, 
                sem_no, sem_who_claim[sem_no]);
        }

        for (i=0; i<NUM_TRACK_PEND; i++)
        {
            if (sem_who_pend[sem_no][i] >= 0)
            {
                DEBUG_ERROR("sem, who pending", EBS_INT2, sem_no, 
                    sem_who_pend[sem_no][i]);
            }
        }
    }
}
#endif      /* INCLUDE_TRK_PKTS */

/* ********************************************************************   */
/* DISPLAY LOWWATER                                                       */
/* ********************************************************************   */
void display_packet_lowwater(void)
{
#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
int i;

    DEBUG_ERROR("TOTAL packet counts: current free, lowwater free = ", EBS_INT2,
        current_free_packets, lowest_free_packets);
    DEBUG_ERROR("---------------", NOVAR, 0, 0);
#if (LOW_WAIT)
    tm_cputs("Press any key to continue . . .");
    tm_getch();
    tm_puts(" ");
#endif
    for (i = 0; i < CFG_NUM_FREELISTS; i++)
    {
        DEBUG_ERROR("packet counts: pool number, data size, ", EBS_INT2,
            i, dcu_size_array[i]);
        DEBUG_ERROR("packet counts: total packets (highwater)", EBS_INT1,
            highest_free_packets_array[i], 0);
        DEBUG_ERROR("packet counts: current free, lowwater free = ", EBS_INT2,
            current_free_packets_array[i], 
            lowest_free_packets_array[i]);
        DEBUG_ERROR("    ", NOVAR, 0, 0);
    }
#else
    DEBUG_ERROR("packet counts: current alloced, high = ", EBS_INT2,
        current_alloc_packets, highest_alloc_packets);
#endif      /* !INCLUDE_MALLOC_DCU_AS_NEEDED */
}

#if (DEBUG_MEMSTATS)
/* ********************************************************************   */
/* DEBUG MEMSTATS                                                         */
/* ********************************************************************   */
long os_mem_track_stats[12 * 3]; /* 12 sets of {count, min, max} */

void os_track_allocation_stats_init(void)
{                                       
  int i;
  
  tc_memset(os_mem_track_stats, 0, sizeof(os_mem_track_stats));
  for(i = 0; i < sizeof(os_mem_track_stats)/sizeof(os_mem_track_stats[0]); i+=3)
  {
    os_mem_track_stats[i+1] = LONG_MAX;
  }
}

void os_track_allocation_stats(int nbytes)
{                                         
  int log2val;
  int i;
  
  for (log2val = i = 0; (1UL << i) < (unsigned long)nbytes; i++, log2val++)
    ;  
  if (log2val >= sizeof(os_mem_track_stats)/(sizeof(os_mem_track_stats[0])*3))
    log2val = sizeof(os_mem_track_stats)/(sizeof(os_mem_track_stats[0])*3) - 1;
    
  /* log2val now is index into os_mem_track_stats[] array...   */
  log2val *= 3;
  os_mem_track_stats[log2val]++;
  if (nbytes < os_mem_track_stats[log2val+1])
    os_mem_track_stats[log2val+1] = nbytes;
  if (nbytes > os_mem_track_stats[log2val+2])
    os_mem_track_stats[log2val+2] = nbytes;
}
   
void os_track_allocation_stats_dump(void)
{                                         
  int i;
  
  DIAG_MSG((DIAG_SECTION_MEMSTATDUMP,LOG_NOTICE
           ,"OS_MEM Memory Allocation Statistics:\n"
            "====================================\n"
            "Count      | Min. Size  | Max. Size \n"
            "-----------+------------+-----------\n"
           ));
  for(i = 0; i < sizeof(os_mem_track_stats)/sizeof(os_mem_track_stats[0]); i+=3)
  {
    DIAG_MSG((DIAG_SECTION_MEMSTATDUMP,LOG_NOTICE
             ,"%10lu | %10lu | %10lu\n" 
             ,os_mem_track_stats[i]
             ,(os_mem_track_stats[i+1] == LONG_MAX ? 0 : os_mem_track_stats[i+1])
             ,os_mem_track_stats[i+2]
            ));
  }
  DIAG_MSG((DIAG_SECTION_MEMSTATDUMP,LOG_NOTICE
           ,"-----------+------------+-----------\n"
           ));
}

#else

void os_track_allocation_stats_dump(void)
{
  return; /* no stat, no info... sorry.                                          */
}

#endif  /* DEBUG_MEMSTATS */

/* ********************************************************************   */
/* DEBUG DCU stuff - checks if freeing a packet which is already free     */
/* ********************************************************************   */
#if (DEBUG_FREE || DEBUG_FREE_TCP_WIN)
/* called if trying to free an already freed packet   */
void break_it(void)
{
    DEBUG_ERROR("ERROR freeing a packet - set breakpoint at break_it",
        NOVAR, 0, 0);
}

/* assumes offset is 0     */
/* returns TRUE if on list */
RTIP_BOOLEAN check_pos_list(DCU msg, DCU root_msg, DCU check_msg)
{
POS_LIST entry;
DCU frag_msg;

    if (!msg)
        return(FALSE);

    do          /* loop thru the pos_list */
    {
        /* loop thru fragments - only udpapp exchange (UDP input) and TCP input   */
        /* window can have fragments but it is easiest to do it for all           */
        frag_msg = msg;
        while (frag_msg)
        {
            if (frag_msg == check_msg)
            {
                break_it();     /* oops - packet already on list */
                return(TRUE);   /* on list */
            }
#            if (INCLUDE_FRAG)
                frag_msg = DCUTOPACKET(frag_msg)->frag_next;
#            else
                frag_msg = (DCU)0;
#            endif

        }

        entry = (POS_LIST)DCUTOPACKET(msg);
        entry = entry->pnext;
        msg = (DCU)entry;
    } while (msg != root_msg);

    return(FALSE);      /* not on list */
}

#if (!INCLUDE_MALLOC_DCU_AS_NEEDED)
/* returns TRUE if already on free list   */
RTIP_BOOLEAN check_free(DCU free_msg)
{
DCU msg;
DCU root_msg;
RTIP_BOOLEAN ret_val;
int i;

    if (!free_msg)
    {
        break_it();
        return TRUE;
    }

    if (!DCUTODATA(free_msg))
    {
        break_it();
        return TRUE;
    }

    for ( i = 0; i < CFG_NUM_FREELISTS; i++)
    {
        msg = root_msg = (DCU)root_dcu_array[i];
        if (msg)
        {
            ret_val = check_pos_list(msg, root_msg, free_msg);
            if (ret_val)
            {
                DEBUG_ERROR("ATTEMPT TO FREE ALREADY FREE PKT", NOVAR, 0, 0);
            }
            return(ret_val);
        }
    }
    return FALSE;
}
#endif  /* !INCLUDE_MALLOC_DCU_AS_NEEDED */

#if (INCLUDE_TCP && DEBUG_FREE_TCP_WIN && !INCLUDE_MALLOC_DCU_AS_NEEDED)
RTIP_BOOLEAN check_tcp_window(DCU check_msg)
{
int i;
DCU msg;
PTCPPORT port;

    /* TCP PORTS   */
    for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
    {
        port = (PTCPPORT)(alloced_ports[i]);
        if (!port || !IS_TCP_PORT(port))
            continue;

        /* check for DCU in input window   */
        msg = (DCU)(port->out.dcu_start); 
        while (msg)
        {
            if (msg == check_msg)
            {
                break_it();
                return TRUE;
            }
            msg = (DCU)
                os_list_next_entry_off((POS_LIST)port->out.dcu_start,
                                       (POS_LIST)msg,
                                        TCP_WINDOW_OFFSET);
        }
        /* check for DCU in input window   */
        msg = (DCU)(port->in.dcu_start); 
        while (msg)
        {
            if (msg == check_msg)
            {
                break_it();
                return TRUE;
            }
            msg = (DCU)
                os_list_next_entry_off((POS_LIST)port->in.dcu_start,
                                       (POS_LIST)msg,
                                        TCP_WINDOW_OFFSET);
        }
    }
    return FALSE;
}
#endif  /* INCLUDE_TCP && DEBUG_FREE_TCP_WIN && !INCLUDE_MALLOC_DCU_AS_NEEDED */

#endif  /* DEBUG_FREE */

