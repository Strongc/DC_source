/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/* Point-to-Point Protocol                                              */
/*                                                                      */
/*  Based (in part) upon previous implementations by:                   */
/*  1989    -- Drew Daniel Perkins      (ddp@andrew.cmu.edu)            */
/*         Carnegie Mellon University                                   */
/*  09-90   -- Katie Stevens        (dkstevens@ucdavis.edu)             */
/*         UC Davis, Computing Services                                 */
/*                                                                      */
/*  Jan 91  Bill_Simpson@um.cc.umich.edu                                */
/*      Computer Systems Consulting Services                            */
/*                                                                      */
/*  Feb 91  Glenn McGregor          (ghm@merit.edu)                     */
/*      Testing and suggestions.                                        */
/*                                                                      */
/*  May 91  Bill Simpson & Glenn McGregor                               */
/*      Update to newest LCP and IPCP draft RFCs.                       */
/*      Add quick installation features.                                */
/*      Add support for echo and discard message sending.               */
/*                                                                      */
/*  Jul 91  Glenn McGregor & Bill Simpson                               */
/*      Improve PAP user interface and fix related bugs.                */
/*      Remove pwaits and "phase machine".                              */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_PPP)
#if (INCLUDE_PPPOE)
#include "pppoe.h"
#endif
#if (INCLUDE_TRK_PPP  && TRACK_WAIT)
#include "terminal.h"
#endif

#define PPP_ORIG 1

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DISPLAY_FRAME_ERROR 1   /* set to 1 if want to display frame error */
                                /* on input   */
#define DISPLAY_GIVE_STRING 0
#define DISPLAY_PKTS        0
#define DISPLAY_INPUT_PKTS  0
#define DISPLAY_OUTPUT_PKTS 0
#define DEBUG_PPP_XMIT      0

/***************************************************************************  */
#define USE_RS232_RTIP 1    /* use RTIP's RS232 */

/***************************************************************************  */
#define PPPFCS(fcs, c)  (((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0x00ff])

/***************************************************************************  */
/* EXTERNAL GLOBAL DATA                                                       */
KS_EXTERN_GLOBAL_CONSTANT word KS_FAR fcstab[256];
extern RS232_IF_INFO KS_FAR rs232_if_info_arry[CFG_NUM_RS232];
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR dummy_en_addr[ETH_ALEN];

extern struct ppp_s KS_FAR ppp_cb[CFG_NUM_PPP];     /* PPP  control block */
extern struct lcp_s KS_FAR lcp_cb[CFG_NUM_PPP];     /* LCP  control block */
extern struct ipcp_s KS_FAR ipcp_cb[CFG_NUM_PPP];   /* IPCP control block */
extern struct pap_s KS_FAR pap_cb[CFG_NUM_PPP];     /* PAP  control block */
#if (INCLUDE_CHAP)
extern struct chap_s KS_FAR chap_cb[CFG_NUM_PPP];       /* CHAP control block */
#endif

/***********************************************************************  */
/* Routines local to this file                                            */
#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
void        ppp_char_to_escape_buffer(PPPPS ppp_p, PRS232_IF_INFO pif_info, char c);
#endif
static void ppp_error (DCU msg, int rtip_errno);
void        ppp_skipped (PPPPS ppp_p, DCU msg, int rtip_errno);
int         ppp_send(PIFACE pi, DCU msg, RTIP_BOOLEAN ether_encap);
static int  ppp_raw(PIFACE pi, DCU msg, struct ppp_hdr ph, int offset);
void        ppp_queue(byte c, PIFACE pi, dword accm);
void        ppp_proc(PIFACE pi, DCU msg, struct ppp_hdr_alias ph);
RTIP_BOOLEAN alloc_msg_in(PRS232_IF_INFO pif_info);

#if (INCLUDE_TRK_PPP)
#define TRACK_WAIT 0
void track_ppp_input(word protocol, PFBYTE pb, int len);
void track_ppp_output(word protocol, PFBYTE pb, int len);
#endif

/***********************************************************************  */
/* Routines in other files                                                */
void    rs232_close(PIFACE pi);
RTIP_BOOLEAN rs232_init(PIFACE pi);

/***************************************************************************   */
/* INTERFACE to RS232 layser                                                   */
/***************************************************************************   */
int if_rs232_open_init(PIFACE pi)
{
#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
    return(rs232_init(pi));
#else
/*#error: implement if_rs232_open_init   */
    return(0);
#endif
}

void if_rs232_close(PIFACE pi)
{
#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
    rs232_close(pi);
#else
/*#error: implement if_rs232_close   */
#endif
}

/* initialize output buffers;                                      */
/* offset is offset in DCUTODATA(msg) where data to be sent starts */
void if_rs232_init_out_buffers(PIFACE pi, DCU msg, int offset)
{
#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
PRS232_IF_INFO pif_info;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];   
    rs232_xmit_init(pif_info, msg, offset);
#else
/*#error: if_rs232_init_out_buffers: need to implement   */
#endif
}

/* Data to be sent is not escaped; it has PPP header               */
/* offset is offset in DCUTODATA(msg) where data to be sent starts */
/* Returns: REPORT_XMIT_DONE if xfer is done                       */
/*          0 if not done                                          */
int if_rs232_start_send(PIFACE pi, DCU msg, int offset)
{
#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
PPPPS ppp_p = pi->edv;
PRS232_IF_INFO pif_info;
#else
PFBYTE buffer;
int length;
#endif

#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
    ARGSUSED_PVOID(offset)

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];   

    /* Send an packet seperator even if the previous packet is still   */
    /* being transmitted.                                              */
    /* Flushes out any line garbage                                    */
    ppp_char_to_escape_buffer(ppp_p, pif_info, HDLC_FLAG);

    /* escape buffer and start transmitter                                 */
    if (rs232_xmit_done(pi, msg, TRUE))                                 
    {                                                                   
        return(REPORT_XMIT_DONE);                                       
    }                                                                   
    else                                                                
        return(0);          /* xmit not done */

#else
/*#error: implement   */
    buffer = DCUTODATA(msg) + offset;                                   
    length = DCUTOPACKET(msg)->length - offset;                         

    /* IMPLEMENT TRANSFER HERE   */

    return(REPORT_XMIT_DONE);                                       
#endif
}

/***********************************************************************  */
#if (DEBUG_SIGNAL)
void ppp_signal_set(void)
{
    DEBUG_ERROR("ppp_signal_set: ", NOVAR, 0, 0);
    ks_sleep((word)(10*ks_ticks_p_sec()));
}
#endif

/***********************************************************************  */
/* Bad packet                                                             */
static void ppp_error(DCU msg, int rtip_errno)
{
    if (msg)
        os_free_packet(msg);
    if (rtip_errno)
        set_errno(rtip_errno);
}


/***********************************************************************  */
/* Unknown type input packet                                              */
void ppp_skipped(PPPPS ppp_p, DCU msg, int rtip_errno)
{
PIPCPS ipcp_p;

    ipcp_p = (PIPCPS)ppp_p->fsm[IPcp].pdv;
    if (ipcp_p) 
    {
#if (INCLUDE_PPP_VANJC)
        slhc_toss( ipcp_p->slhcp );
#endif
    }
    ppp_error(msg, rtip_errno);
}


/* ********************************************************************   */
/* INPUT ROUTINES (called from rs232 input routine)                       */
/* ********************************************************************   */
/* set accm value globally for use when inputting PPP packet;             */
/* it is saved in pif_info->rs232_accm for use by process_ppp_char        */
void set_ppp_accm(PIFACE pi)
{
PPPPS ppp_p;
PRS232_IF_INFO pif_info;

    /* get PPP control block   */
    ppp_p = pi->edv;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    /* Use negotiated values if LCP finished   */
    pif_info->rs232_accm = CFG_LCP_ACCM_DFLT;
    if (ppp_p->fsm[Lcp].ppp_state == fsmOPENED) 
    {
        PLCPS lcp_p = (PLCPS)ppp_p->fsm[Lcp].pdv;

        if (lcp_p->local_entry.work.negotiate & LCP_N_ACCM) 
        {
            pif_info->rs232_accm = lcp_p->local_entry.work.accm;
        }
    }
}

/* ********************************************************************   */
/* process input PPP header                                               */
/* Input:  pif_info->header contains first 4 bytes of packet              */
/* OUTPUT: pif_info->ph.protocol is protocol, i.e. IP, LCP etc.           */
/*         hdr_size is set to the size of the PPP header                  */
int process_ppp_header(PRS232_IF_INFO pif_info, int *hdr_size)
{
int negotiated;
PPPPS ppp_p;
int prot_offset;

    /* get PPP control block   */
    ppp_p = pif_info->rs232_pi->edv;

    negotiated = 0;

    /* Use negotiated values if LCP finished   */
    if (ppp_p->fsm[Lcp].ppp_state == fsmOPENED) 
    {
        PLCPS lcp_p = (PLCPS)ppp_p->fsm[Lcp].pdv;
        negotiated = lcp_p->local_entry.work.negotiate;
    }

    /* We do not want the PPP header to be apart                    */
    /* of the packet, therefore, discard it                         */
    /* NOTE: the header is either 3 or 4 bytes, therefore, after    */
    /* four bytes have been read process it                         */
    /* if header is 3 bytes, the fourth byte already read is really */
    /* the first byte of the information field                      */
    /* NOTE: ppp_raw will output append the header before sending   */
    /*       by sending the header first                            */

    /* since 4 bytes have been read, get PPP header info since   */
    /* it will not be saved in the input packet                  */
    /* NOTE: header info will be overwritten below               */

    /* HDLC address and control fields may be compressed out   */
    if ( (pif_info->header[0] != HDLC_ALL_ADDR) || 
            (pif_info->header[1] != HDLC_UI) )
    {
        DEBUG_LOG("process_ppp_header - AFCF - cp[0] = ", LEVEL_3, EBS_INT1, *(pb_in-4), 0);

        /* must be compressed; if compression not   */
        /* negoitiated it is an error               */
        if (!(negotiated & LCP_N_ACFC)) 
        {
#        if (DISPLAY_FRAME_ERROR)
            DEBUG_ERROR("process_ppp_header: ACFC not neg:", 
                PKT, pif_info->header, 10);
            DEBUG_ERROR("process_ppp_header: ACFC not neg: header = ", 
                EBS_INT2, pif_info->header[0], 
                pif_info->header[1]);
#        endif
            goto frame_error;
        }
        else
            prot_offset = 0;    /* i.e. no address or control field */
    } 
    else
        prot_offset = 2;

    /* **************************************************   */
    /* Initialize the expected header                       */
    pif_info->ph.addr = HDLC_ALL_ADDR;
    pif_info->ph.control = HDLC_UI;
    pif_info->ph.protocol = pif_info->header[prot_offset];

    /* **************************************************        */
    /* First byte of PPP protocol field may be compressed out    */
    /* only odd byte of protocol is last byte                    */
    if ( pif_info->ph.protocol & 0x01 )      
    {
        if (!(negotiated & LCP_N_PFC)) 
        {
#        if (DISPLAY_FRAME_ERROR)
            DEBUG_ERROR("process_ppp_header: PFC not neg:", 
                PKT, pif_info->header, 10);
            DEBUG_ERROR("process_ppp_header: PFC not neg:protocol", 
                EBS_INT1, pif_info->ph.protocol, 0);
#        endif
            goto frame_error;
        }
        *hdr_size = (word)(1+prot_offset);  /* header size = 1 or 3 */
    } 

    /* first byte of PPP protocol field not compressed out   */
    else 
    {
        pif_info->ph.protocol = 
            (word)((pif_info->ph.protocol << 8) | 
                    pif_info->header[prot_offset+1]);

        /* Second byte of PPP protocol field must be odd   */
        if ( !(pif_info->ph.protocol & 0x01) ) 
        {
#        if (DISPLAY_FRAME_ERROR)
            DEBUG_ERROR("process_ppp_header: second byte not odd:", 
                PKT, pif_info->header, 10);
            DEBUG_ERROR("process_ppp_header: second byte not odd:", 
                EBS_INT1, pif_info->ph.protocol, 0);
#        endif
            goto frame_error;
        }
        else
            *hdr_size = (word)(2+prot_offset);      /* header size = 2 or 4 */
    }

    /* no compression is allowed if LCP is not   */
    /* opened (i.e. LCP echo and replys are      */
    /* allowed to be compressed)                 */
    if ( (ppp_p->fsm[Lcp].ppp_state != fsmOPENED) &&
            (*hdr_size != PPP_HDR_LEN) )
    {
#        if (DISPLAY_FRAME_ERROR)
            DEBUG_ERROR("process_ppp_header: Framing error: hdr_size = ", 
                EBS_INT1, *hdr_size, 0);
            DEBUG_ERROR("process_ppp_header: Framing error: pkt = ", 
                PKT, &(pif_info->header[0]), 4);
#        endif
    }

    return(0);

frame_error:
#if (DISPLAY_FRAME_ERROR)
    DEBUG_ERROR("process_ppp_header: Framing error", NOVAR, 0, 0);
#endif
    return(-1);
}

/* ********************************************************************   */
/* ppp_give_string() - receive string from driver                         */
/*                                                                        */
/*   Called by rs232_give_string when a packet has arrived.               */

#if (USE_RS232_RTIP)    /* use RTIP's RS232 */
void ppp_give_string(PRS232_IF_INFO pif_info, PFBYTE buffer, int nbytes)
{
PPPPS ppp_p;
int length = 0;
int hdr_size, i, j;
byte chr, in_chr;
RTIP_BOOLEAN toss_chars = FALSE;
RTIP_BOOLEAN esc_chars = FALSE;
RTIP_BOOLEAN resynch_vars;
RTIP_BOOLEAN in_header=FALSE;
PFBYTE pb_in = (PFBYTE)0;
PFBYTE pb_in_base = (PFBYTE)0;
dword rs232_accm;      
word calc_fcs = 0;

#if (DISPLAY_GIVE_STRING)
    DEBUG_ERROR("PPP DATA IN = ", PKT, buffer, nbytes);
#endif

    /* get PPP control block   */
    ppp_p = pif_info->rs232_pi->edv;
    
    if (ppp_p->ppp_phase == pppDEAD) 
    {
        DEBUG_ERROR("ppp_give_string - phase dead!!! = ", EBS_INT1, 
            ppp_p->ppp_phase, 0);
        DEBUG_LOG("ppp_give_string - phase dead!!! = ", LEVEL_3, EBS_INT1, 
            ppp_p->ppp_phase, 0);
        ppp_error((DCU)0, 0);
        ppp_p->OutError++;
        return;
    }

    set_ppp_accm(pif_info->rs232_pi);
    rs232_accm = pif_info->rs232_accm;

    /* Resyncronize state variables to start   */
    resynch_vars = TRUE;

    /* **************************************************   */
    for (i=0; i<nbytes; i++)        /* loop thru chars in buffer */
    {
        /* get next character   */
        in_chr = chr = *buffer++;

        /* IF We are tossing chars and got a framing char    */
        /* resync so we can start fresh with the frame       */
        if (resynch_vars)
            toss_chars =    pif_info->toss_chars;

        /* if tossing packet and at packet boundary   */
        if (toss_chars && (in_chr == HDLC_FLAG))
        {
            DEBUG_ERROR("TOSSED PKT since toss_chars set and HDLC_FLAG", 
                NOVAR, 0, 0);
            toss_chars = FALSE;
            pif_info->toss_chars = FALSE;
            if (pif_info->msg_in)
                os_free_packet(pif_info->msg_in);
            /* clear the input. This will cause the state to be reset   */
            pif_info->msg_in = (DCU)0;
            length = 0;
        }

        /* Allocate a buffer and reset state if needed      */
        if (!pif_info->msg_in)
        {
            alloc_msg_in(pif_info);
            if (!pif_info->msg_in)      /* If the alloc failed keep track  */
            {                           /* allow the vj compression code to know */
                ppp_p->InMemory++;
                ppp_skipped(ppp_p, 0, 0);   /*Uncompressed TCP/IP not enabled */
                DEBUG_ERROR("ppp_give_string: alloc msg in failed", NOVAR, 0, 0);
                return;                     /*Return. Tossing the input buffer */
                                            /*since msg_in is 0 the system    */
                                            /* will resync                    */
            }
            resynch_vars = TRUE;
        }
        if (resynch_vars)
        {
            resynch_vars = FALSE;
            length       = pif_info->length;
            pb_in        = pif_info->pb_in;
            pb_in_base   = pb_in;
            esc_chars    =  pif_info->esc_chars;
            toss_chars   =  pif_info->toss_chars;
            calc_fcs     =  pif_info->calc_fcs;
            in_header    =  pif_info->in_header;
        }

        /* **************************************************   */
        if (!toss_chars)
        {
            if ( chr < SP_CHAR && rs232_accm && (rs232_accm & (1L << chr)) ) 
                continue;

            /* escape char if prev char is HDLC_ESC_ASYNC; or is this   */
            /* is the escape char (HDLC_ESC_ASYNC) then set ecs_chars   */
            /* and go get the next character                            */
            if (esc_chars) 
            {
                esc_chars = FALSE;
                chr ^= HDLC_ESC_COMPL;
            } 
            else if (chr == HDLC_ESC_ASYNC) 
            {
                esc_chars = TRUE;
                continue;
            }

            /* if not beginning/end of packets calc checksum (FCS)   */
            if (in_chr != HDLC_FLAG)
            {
                calc_fcs = (word)(PPPFCS(calc_fcs, chr));

                /* if have read past the 4 byte header   */
                if (!in_header)
                {
                    if (length < MAX_PACKETSIZE)
                        *pb_in++ = chr;
                    length++;
                }
                else
                {
                    pif_info->header[length++] = chr;

                    /* **************************************************      */
                    /* if we have 4 bytes read (header and if compression used */
                    /* part of packet); determine what is really the           */
                    /* header due to compression                               */
                    /* **************************************************      */
                    if (length == PPP_HDR_LEN)
                    {
                        if (process_ppp_header(pif_info, &hdr_size) < 0)
                        {
#                            if (DISPLAY_FRAME_ERROR)
                                DEBUG_ERROR("ppp_give_string: Framing error", NOVAR, 0, 0);
#                            endif
                            ppp_skipped( ppp_p, 0, 0);   /* missing ALL address */
                            ppp_p->InFrame++;
                            toss_chars = TRUE;
                            continue;
                        }

                        in_header =  FALSE;

                        /* **************************************************   */
                        /* remember where the beginning of data is just beyond the 
                        the header area is. Then copy the data into position.
                        If we have an IP packet copy it to just past the 
                        ethernet pseudo frame. Otherwise move it to the beginning 
                        */
                        if ( (pif_info->ph.protocol == PPP_IP_PROTOCOL) ||
                             (pif_info->ph.protocol == PPP_UNCOMP_PROTOCOL) ||
                             (pif_info->ph.protocol == PPP_COMPR_PROTOCOL) )
                        {
                            /* force it to look like an ethernet packet   */
                            /* since will be sent to IP exchange          */
                            ((PETHER) pb_in_base)->eth_type = EIP_68K;
                            tc_movebytes(((PETHER)pb_in_base)->eth_dest, 
                                dummy_en_addr, ETH_ALEN);
                            length = ETH_HLEN_BYTES;
                        }
                        else
                        {
                            length = 0;
                        }
                        pb_in_base = DCUTODATA(pif_info->msg_in);
                        pb_in = pb_in_base + length;

                        for (j=hdr_size; j<PPP_HDR_LEN; j++)
                        {
                            *pb_in++ = pif_info->header[j];
                            length++;
                        }
                    }
                }
            }
        }

        /* if a full packet has been received   */
        if (in_chr == HDLC_FLAG)
        {
            /* Count # empty frames   */
            if (in_header)
                ppp_p->InOpenFlag++;
            else if ( calc_fcs != HDLC_FCS_FINAL ) 
            {
                DEBUG_LOG("ERROR: ppp_give_string() - checksum error", LEVEL_3, NOVAR, 0, 0);
                DEBUG_ERROR("ERROR: ppp_give_string() - checksum error", NOVAR, 0, 0);
                ppp_skipped(ppp_p, 0, 0); /* checksum error */
                ppp_p->InChecksum++;
                toss_chars = TRUE;
                /* Fall through   */
            } 

            /* If in the data and not tossing we're cool   */
            if (!in_header && !toss_chars)
            {
                /* Submit the packet   */
                length -= 2;    /* Remove the FCS (2 byte checksum) */
                pif_info->stats.bytes_in += length;

                DCUTOPACKET(pif_info->msg_in)->length = (word) length;
                /* process the packet to IP.   */

                ppp_proc(pif_info->rs232_pi, pif_info->msg_in, pif_info->ph);
            }
            else if (pif_info->msg_in)
            {
                os_free_packet(pif_info->msg_in);
            }

            /* clear the input. This will cause the state to be reset   */
            pif_info->msg_in = (DCU)0;
            length = 0;
        }       /* end of if HDLC_FLAG */

        /* Overflow condition; -2 for the FCS   */
        if ((length-2) > MAX_PACKETSIZE)
        {
            DEBUG_ERROR("ppp_give_string: length to long: length = ",
                EBS_INT1, length, 0);
            toss_chars = TRUE;
        }
    }       /* end of for loop */

    /* We're leaving so update the pif_info structure   */
    pif_info->pb_in      = pb_in;
    pif_info->toss_chars = toss_chars;
    pif_info->esc_chars  = esc_chars;
    pif_info->length     = length;
    pif_info->in_header  = in_header;
    pif_info->calc_fcs   = calc_fcs;
}

#else   /* USE_RS232_RTIP */

/* buffer contains full packet where ACCM is taken care of        */
/* INPUT:                                                         */
/*   buffer contains PPP header                                   */
/*   pif_info->msg_in contains DCU with buffer and length, nbytes */
/*                                                                */
void ppp_give_string(PRS232_IF_INFO pif_info, PFBYTE buffer, int nbytes)
{
int length = 0;
int i;
int hdr_size;
PPPPS ppp_p;

    /* this is here just to show that they should be the same   */
    buffer = DCUTODATA(pif_info->msg_in);
    nbytes = DCUTOPACKET(pif_info->msg_in)->length;

    /* get PPP control block   */
    ppp_p = pif_info->rs232_pi->edv;

    for (i=0; i<PPP_HDR_LEN; i++)
    {
        pif_info->header[length++] = *(buffer+i);
    }

    /* check PPP header; set up pif_info->ph.protocol and return   */
    /* header size                                                 */
    if (process_ppp_header(pif_info, &hdr_size) < 0)
    {
        ppp_skipped( ppp_p, 0, 0);   /* missing ALL address */
        ppp_p->InFrame++;
        return;
    }

    if ( (pif_info->ph.protocol == PPP_IP_PROTOCOL) ||
            (pif_info->ph.protocol == PPP_UNCOMP_PROTOCOL) ||
            (pif_info->ph.protocol == PPP_COMPR_PROTOCOL) )
    {
        /* leave room for ethernet header   */
        tc_memmove(buffer+ETH_ALEN, buffer + hdr_size, nbytes - hdr_size);

        /* force it to look like an ethernet packet   */
        /* since will be sent to IP exchange          */
        ((PETHER) buffer)->eth_type = EIP_68K;
        tc_movebytes(((PETHER)buffer)->eth_dest, 
            dummy_en_addr, ETH_ALEN);
        DCUTOPACKET(pif_info->msg_in)->length += (ETH_HLEN_BYTES - hdr_size);
    }

    ppp_proc(pif_info->rs232_pi, pif_info->msg_in, pif_info->ph);
}
#endif


/* ********************************************************************   */
/* CHECK PPP OPEN                                                         */
/* ********************************************************************   */
/* checks if PPP connection is open; if not returns errno;                */
/* otherwise returns 0 if open                                            */
int check_ppp_open(PIFACE pi)
{
PPPPS ppp_p;

    if (!pi) 
        return(EBADIFACE);

    if (!pi->edv) 
        return(EIFACECLOSED);

    ppp_p = pi->edv;

    /* state must be opened   */
    if (CHECK_PPP_NOT_OPEN(pi))
    {
        DEBUG_ERROR("write error: IPCP state addr and value = ", DINT2, 
            &(ppp_p->fsm[IPcp].ppp_state),  ppp_p->fsm[IPcp].ppp_state);
        ppp_error((DCU)0, 0);
        ppp_p->OutError++;
        return(EPPPNOTOPEN);
    }
    return(0);
}

/* ********************************************************************      */
/* XMIT                                                                      */
/* ********************************************************************      */
/*                                                                           */
/* ppp_xmit() - send a PPPoE or PPP packet; do VanJacobson compression       */
/*                                                                           */
/* Send IP datagram with Point-to-Point Protocol or PPP negotiation          */
/* packet (LCP, IPCP, PAP, CHAP etc)                                         */
/*                                                                           */
/* Uses the protocol field in the msg structure to determine protocol;       */
/* If it is PPPoE, it calls tc_interface_send() to send over ethernet        */
/* If it is IP, it deals with Van Jacobson compression                       */
/* If it is not PPPoE, calls ppp_send()                                      */
/*                                                                           */
/* The output packet is formatted as follows:                                */
/*    If IP, it has room for ethernet header                                 */
/*    If not IP, it has room for PPP and CONFIG headers, i.e. it is          */
/*      a complete PPP packet with room for 4 bytes of the PPP header.       */
/*      the PPP header has not been filled in yet and due to ACF and         */
/*      ACFC could actually need 1, 3 or 4 bytes hense the offset parameter  */
/*      to ppp_raw()                                                         */
/*    NOTE: the offset can also be ETH_HLEN_BYTES-PPP_HDR_LEN for IP packets */
/*                                                                           */
/* Returns 0 if successful, errno if unsuccessful or REPORT_XMIT_DONE        */
/*                                                                           */
int ppp_xmit(PIFACE pi, DCU msg)
{
PPPPS ppp_p;
#if (INCLUDE_PPP_VANJC)
PIPCPS ipcp_p;
#endif
word ret_val = 0;
#if (INCLUDE_PPPOE)
DCU msg_send;
#endif
RTIP_BOOLEAN ether_encap = FALSE;

#if (DEBUG_PPP_XMIT)
    DEBUG_ERROR("ppp_xmit called", NOVAR, 0, 0);
#endif

    DEBUG_LOG("ppp_xmit() - entered", LEVEL_3, NOVAR, 0, 0);

    /* check if interface and it has a PPP control block   */
    if (!pi) 
        return(EBADIFACE);

    if (!pi->edv) 
        return(EIFACECLOSED);

    ppp_p = pi->edv;

    if (DCUTOPACKET(msg)->protocol != PPP_IP_PROTOCOL)
    {
#if (INCLUDE_PPPOE)
        if (IS_PPPOE(pi))
        {
            msg_send = convert_pkt_to_pppoe(msg, pi->pppoe_session_handle);
            if (!msg_send)
            {
                DEBUG_ERROR("ppp_xmit: convert_pkt_to_pppoe failed",
                    NOVAR, 0, 0);
            }
            else
            {
                /* send the packet on the ethernet interface   */
                DCUTOPACKET(msg_send)->protocol = 0;  /* TBD */
                if (tc_interface_send(pi->pppoe_ether_pi, msg_send))
                {
                    DEBUG_ERROR("ppp_xmit: failed", NOVAR, 0, 0);
                    return(-1);
                }
            }
            ret_val = REPORT_XMIT_DONE;
        }
        else
#endif /* INCLUDE_PPPOE */
        {
            ether_encap = FALSE;
        }
    }
    else    /* PROTOCOL is IP */
    {
#if (INCLUDE_PPP_VANJC)
        /* IP PACKET: compress it if possible   */
        ipcp_p = (PIPCPS)ppp_p->fsm[IPcp].pdv;
        if (ipcp_p->remote_entry.work.negotiate & IPCP_N_COMPRESS) 
        {
            /* Attempt IP/TCP header compression   */
            /* packet points to IP part of packet  */
            switch (slhc_compress(pi, ipcp_p->slhcp, 
                                  &msg,
                                  ipcp_p->remote_entry.work.slot_compress))
            {
            case SL_TYPE_IP:
                DCUTOPACKET(msg)->protocol = PPP_IP_PROTOCOL;
                break;
            case SL_TYPE_COMPRESSED_TCP:
                DCUTOPACKET(msg)->protocol = PPP_COMPR_PROTOCOL;
                break;
            case SL_TYPE_UNCOMPRESSED_TCP:
                DCUTOPACKET(msg)->protocol = PPP_UNCOMP_PROTOCOL;
                break;
            default:
                ppp_error((DCU)0, 0);
                ppp_p->OutError++;
                return(EPPPBADPKT);
            };
        }
#endif
        /* initialize output buffers;                                                  */
        /* IP packet therefore,                                                        */
        /* packet points to ethernet header but only want to send IP packet            */
        /* (along with PPP header which will be sent first by ppp_raw), therefore,     */
        /* rs232_xmit_init sets up pointer to IP part of output packet                 */
        /* NOTE: size of ether header is larger than PPP header, therefore,            */
        /*       overall PPP packet is smaller than ether packet, therefore,           */
        /*       input will not overflow packet; NOTE: largest packet size             */
        /*       is based on xn_pkt_data_max (based upon MAX_PACKETSIZE) and MTU value */
        /* NOTE: this has to be done after compressing in case compression             */
        /*       allocates a new packet                                                */
#if (INCLUDE_PPPOE)
        if (IS_PPPOE(pi))
        {
            msg_send = convert_pkt_to_pppoe(msg, pi->pppoe_session_handle);
            if (!msg_send)
            {
                DEBUG_ERROR("ppp_xmit: convert_pkt_to_pppoe failed",
                    NOVAR, 0, 0);
            }
            else
            {
                if (tc_interface_send(pi->pppoe_ether_pi, msg_send))
                {
                    DEBUG_ERROR("xn_pppoed_connect: send failed", NOVAR, 0, 0);
                    return(-1);
                }
            }
            ret_val = REPORT_XMIT_DONE;
        }
        else
#endif /* INCLUDE_PPPOE */
        {
            ether_encap = TRUE;
        }
    }   /* end of if IP protocol or not */

#if (INCLUDE_TRK_PPP)
    track_ppp_output(DCUTOPACKET(msg)->protocol, DCUTODATA(msg), 
                     DCUTOPACKET(msg)->length);
#endif
#if (INCLUDE_PPPOE)
    if (!IS_PPPOE(pi))
#endif
    {
        ret_val = (word)ppp_send(pi, msg, ether_encap);
    }
    return(ret_val);
}

/***********************************************************************   */
/* ppp_send() - send a PPP packet                                          */
/*                                                                         */
/* Send a PPP packet filling it in with PPP header                         */
/*                                                                         */
/* Calculates offset of start of packet                                    */
/* NOTE: IP packet has ethernet header where PPP header is written over it */
/* NOTE: non-IP packet has 4 bytes of room for PPP header                  */
/* NOTE: PPP header can be 4,3 or 1 byte depending upon ACFC and PFC       */
/*                                                                         */
/* Returns 0 if successful or errno if unsuccessful                        */
/*                                                                         */
int ppp_send(PIFACE pi, DCU msg, RTIP_BOOLEAN ether_encap)
{
PPPPS ppp_p;
PLCPS lcp_p;
struct ppp_hdr hdr;
struct ppp_hdr *phdr;
PFBYTE pptr;
int is_lcp;
int ptr_off;
int offset;

    DEBUG_LOG("ppp_send - protocol = ", LEVEL_3, EBS_INT1, DCUTOPACKET(msg)->protocol, 0);

    /* check if interface and it has a PPP control block   */
    if (!pi) 
        return(EBADIFACE);

    ppp_p = pi->edv;
    if (!pi->edv) 
        return(EIFACECLOSED);

    lcp_p = (PLCPS)ppp_p->fsm[Lcp].pdv;
    DEBUG_LOG("ppp_send - phase = ", LEVEL_3, EBS_INT1, ppp_p->ppp_phase, 0);

    /* set up structure with PPP header information   */
    hdr.addr     = HDLC_ALL_ADDR;
    hdr.control  = HDLC_UI;
    hdr.protocol = DCUTOPACKET(msg)->protocol;

    is_lcp = (hdr.protocol == PPP_LCP_PROTOCOL);

    offset = 0;
    ptr_off = 0;
    if (ether_encap)
    {
        /* packet points to ethernet header but only want to send IP packet;   */
        /* leave room for PPP header to be overwritten in the last bytes       */
        /* of the ethernet header                                              */
        offset += (ETH_HLEN_BYTES - PPP_HDR_LEN);
        ptr_off += (ETH_HLEN_BYTES - PPP_HDR_LEN);  /* point to PPP hdr */
    }

    /* set up PPP header (which will be sent first); work backwards due   */
    /* to compression                                                     */
    pptr = DCUTODATA(msg);
    phdr = (struct ppp_hdr *)(pptr + offset);

    /* **************************************************              */
    /* FILL IN PPP HEADER                                              */
    /*                                                                 */
    /* FIRST: PROTOCOL FIELD                                           */
    /* always do the lower byte but write both and upper byte possibly */
    /* will be overwritten below                                       */
    phdr->protocol = net2hs(DCUTOPACKET(msg)->protocol);

    /* set up for ADDR and CONTROL FIELDS               */
    /*   protocol was written in bytes 2 and 3          */
    /*   ptr_off is offset on where to write next       */
    /*   offset is offset where packet will start       */
    /* no compression is  (x,x,p,p) where p is protocol */
    if (!is_lcp && (lcp_p->remote_entry.work.negotiate & LCP_N_PFC) &&
        (hdr.protocol < 0x00ff) )
    {
        /* compression (-,x,x,p) where p is protocol   */
        ptr_off += 1;   /* point to address field (offset 1) */
        offset  += 1;
    }

    /* SECOND: ADDR and CONTROL   */
    if (is_lcp || !(lcp_p->remote_entry.work.negotiate & LCP_N_ACFC)) 
    {
        *(pptr + ptr_off)     = hdr.addr;
        *(pptr + ptr_off + 1) = (byte)hdr.control;
    }
    else
    {
        offset += 2;
    }

    return ppp_raw(pi, msg, hdr, offset);
}


/***********************************************************************  */
/* queue char into escape buffer (i.e. into pif_info->out_buf_esc_in)     */
/* NOTE: if char needs to be escaped, 2 chars will be queued              */
#define PPP_QUEUE(C)                             \
    if ( ((C < SP_CHAR) &&                       \
         (pif_info->out_buf_accm &&              \
         (pif_info->out_buf_accm & (1L << C)))) || (C == HDLC_ESC_ASYNC) ||  (C == HDLC_FLAG)) \
    {*pif_info->out_buf_esc_in++ = (byte)HDLC_ESC_ASYNC;   \
     *pif_info->out_buf_esc_in++ = (byte)((word)C ^ HDLC_ESC_COMPL); \
     pif_info->nout_esc += 2;}                   \
    else {  *pif_info->out_buf_esc_in++ = C; pif_info->nout_esc++;}

/***********************************************************************  */
/* ppp_raw() - send packet                                                */
/*                                                                        */
/* Encode a raw packet in PPP framing, put on link output queue           */
/*                                                                        */
/* offset is the offset in the PPP packet where the PPP header starts     */
/*                                                                        */
/* Returns REPORT_XMIT_DONE if all chars queued in circular buffer,       */
/*         0 if successful (xfer started but not done) or                 */
/*         errno if unsuccessful                                          */

static int ppp_raw(PIFACE pi, DCU msg, struct ppp_hdr ph, int offset)
{
PPPPS ppp_p = pi->edv;
PLCPS lcp_p = (PLCPS)ppp_p->fsm[Lcp].pdv;
int is_lcp;
PRS232_IF_INFO pif_info;

    if (!msg)
    {
        DEBUG_LOG("ERROR : ppp_raw - no packet", LEVEL_3, NOVAR, 0, 0);
        ppp_error((DCU)0, 0);
        ppp_p->OutError++;
        return(EPPPNOLINKHDR);
    }

#if (DISPLAY_PKTS || DISPLAY_OUTPUT_PKTS)
    DEBUG_ERROR("PPP OUT: ", PKT, 
        DCUTODATA(msg)+offset, DCUTOPACKET(msg)->length);
#endif

    is_lcp = (ph.protocol == PPP_LCP_PROTOCOL);

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    /* set up escape information and END-OF-PACKET char   */
    pif_info->out_buf_accm = CFG_LCP_ACCM_DFLT;

    /* if not LCP and negotiated ACCM, get map of which bits to escape   */
    if ( !is_lcp && (lcp_p->remote_entry.work.negotiate & LCP_N_ACCM) ) 
    {
        pif_info->out_buf_accm = lcp_p->remote_entry.work.accm;
    }

    ppp_p->OutTxOctetCount += DCUTOPACKET(msg)->length + PPP_HDR_LEN + 2;  
                                /* count FCS bytes                  */
                                /* tbd - what about compression???? */

    /* set up for FCS calculation   */
    pif_info->out_buf_calc_fcs = HDLC_FCS_START;

    /* **********************************************************    */
    /* INTERFACE WITH RS232 LAYER - for escaping chars and xmit      */
    /* **********************************************************    */
    /* set up escape buffer information                              */
    if_rs232_init_out_buffers(pi, msg, offset);

    /* patch - SPR   */
    return(if_rs232_start_send(pi, msg, offset));
}

#if (USE_RS232_RTIP)
/***********************************************************************           */
/* ESCAPE OUTPUT CHARS TO ESCAPE BUFFER - called from RS232 layer                  */
/***********************************************************************           */
/* ppp_char_to_escape_buffer() - write a char to escape buffer without escaping it */
/*                                                                                 */
void ppp_char_to_escape_buffer(PPPPS ppp_p, PRS232_IF_INFO pif_info, char c)
{
    *pif_info->out_buf_esc_in++ = c; 
    pif_info->nout_esc++;
    ppp_p->OutOpenFlag++;
}

/* escape chars from pif_info->packet_out to pif_info->out_buf_esc_in    */
/* returns TRUE if xmit complete or FALSE if xmit error or xmit not done */
RTIP_BOOLEAN ppp_xmit_escape(PRS232_IF_INFO pif_info)
{
byte c;
RTIP_BOOLEAN queued_char = FALSE;

#if (DEBUG_PPP_XMIT)
    DEBUG_ERROR("ppp_xmit_escape called", NOVAR, 0, 0);
#endif

    /* Copy input (pif_info->packet_out) to                                  */
    /* escape buffer (pif_info->out_buf_esc_in), escaping special characters */
    while (pif_info->packet_out_left)
    {
        if ( (pif_info->nout_esc > CFG_RS232_XMBUFSIZE) ||
             (pif_info->out_buf_esc_base+CFG_RS232_XMBUFSIZE <=     
              pif_info->out_buf_esc_in+1) )         /* end of buf */
                                                    /* where write char;   */
                                                    /* +1 incase esc char  */
        {
#if (DEBUG_PPP_XMIT)
            DEBUG_ERROR("ppp_xmit_escape: can't queue all; call rs232_xmit_uart", NOVAR, 0, 0);
#endif
            rs232_xmit_uart(pif_info);
            uart_setouttrigger(pif_info, 0);
            return(FALSE);
        }

        queued_char = TRUE;
        c = *pif_info->packet_out++;
        pif_info->packet_out_left--;

        /* Fold char value into FCS calculated so far   */
        pif_info->out_buf_calc_fcs = 
            (word)(PPPFCS(pif_info->out_buf_calc_fcs, c));
        PPP_QUEUE(c)
    }       /* end of while */

    if (queued_char)    /* SPR: patch added */
    {
        /* Final FCS calculation   */
        pif_info->out_buf_calc_fcs ^= 0xffff;
        c = (byte)(pif_info->out_buf_calc_fcs & 0x00ff); /* Least significant byte first */
        PPP_QUEUE(c)

        c = (byte)(pif_info->out_buf_calc_fcs >> 8);     /* Most significant byte next */
        PPP_QUEUE(c)

        /* Tie off the packet   */
        *pif_info->out_buf_esc_in = HDLC_FLAG; 
        pif_info->nout_esc++;

#if (DEBUG_PPP_XMIT)
        DEBUG_ERROR("ppp_xmit_escape: all queued: call rs232_xmit_uart", NOVAR, 0, 0);
#endif
    }

    /* send anything left in the buffer   */
    DEBUG_LOG("ppp_raw - call rs232_send", LEVEL_3, NOVAR, 0, 0);
    if (!rs232_xmit_uart(pif_info))
    {
#if (DEBUG_PPP_XMIT)
        DEBUG_ERROR("ppp_xmit_escape: all queued but rs232_xmit_uart failed",
            NOVAR, 0, 0);
#endif
        return(FALSE);
    }

    DEBUG_LOG("ppp_raw - send succeeded", LEVEL_3, NOVAR, 0, 0);
    pif_info->stats.packets_out += 1;
#if (DEBUG_PPP_XMIT)
    DEBUG_ERROR("ppp_xmit_escape: all queued: return TRUE",
        NOVAR, 0, 0);
#endif  

#if (!PPP_ORIG)
    ks_invoke_output(pif_info->rs232_pi, 1);
/*  if (pif_info->rs232_pi)                    */
/*      pif_info->rs232_pi->xmit_dcu = (DCU)0; */
#endif

    return(TRUE);
}
#endif /* USE_RS232_RTIP */

/* ********************************************************************            */
/* XMIT PACKET                                                                     */
/* ********************************************************************            */
/* Send IP datagram with Point-to-Point Protocol or PPP negotiation
   packet (LCP, IPCP, PAP, CHAP etc) */
/* Returns 0 if successful or errno if unsuccessful. This is for UART devices
   with packet transmit capabilities which don't need to use a circular buffer
   for transmit   */
/*                                                      */
int ppp_xmit_pkt(PIFACE pi, DCU msg)
{
PPPPS ppp_p;
#if (INCLUDE_PPP_VANJC)
PIPCPS ipcp_p;
#endif
PRS232_IF_INFO pif_info;

#if (DEBUG_PPP_XMIT)
    DEBUG_ERROR("ppp_xmit_pkt called", NOVAR, 0, 0);
#endif

    DEBUG_LOG("ppp_xmit_pkt() - entered", LEVEL_3, NOVAR, 0, 0);

    /* check if interface and it has a PPP control block     */
    if (!pi) 
        return(EBADIFACE);

    if (!pi->edv) 
        return(EIFACECLOSED);
    ppp_p = pi->edv;
    if (!msg)
    {
        DEBUG_LOG("ERROR : ppp_raw - no packet", LEVEL_3, NOVAR, 0, 0);
        ppp_error((DCU)0, 0);
        ppp_p->OutError++;
        return(EPPPNOLINKHDR);
    }

    if (DCUTOPACKET(msg)->protocol != PPP_IP_PROTOCOL)
    {
        /* initialize output buffers;     */
        pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
#if (PPP_ORIG) 
        pif_info->out_buf_esc_in = pif_info->out_buf_esc_out = 
               pif_info->out_buf_esc_base;
        pif_info->nout_esc = 0;
#endif
        pif_info->packet_out = DCUTODATA(msg);
        pif_info->packet_out_left = DCUTOPACKET(msg)->length;
    }
    else
    {
#if (INCLUDE_PPP_VANJC)
        /* IP PACKET: compress it if possible     */
        ipcp_p = (PIPCPS)ppp_p->fsm[IPcp].pdv;
        if (ipcp_p->remote_entry.work.negotiate & IPCP_N_COMPRESS) 
        {
            /* Attempt IP/TCP header compression      */
            /* packet points to IP part of packet     */
            switch (slhc_compress(pi, ipcp_p->slhcp, 
                                  &msg,
                                  ipcp_p->remote_entry.work.slot_compress))
            {
            case SL_TYPE_IP:
                DCUTOPACKET(msg)->protocol = PPP_IP_PROTOCOL;
                break;
            case SL_TYPE_COMPRESSED_TCP:
                DCUTOPACKET(msg)->protocol = PPP_COMPR_PROTOCOL;
                break;
            case SL_TYPE_UNCOMPRESSED_TCP:
                DCUTOPACKET(msg)->protocol = PPP_UNCOMP_PROTOCOL;
                break;
            default:
                ppp_error((DCU)0, 0);
                ppp_p->OutError++;
                return(EPPPBADPKT);
            };
        }
#endif
        /* initialize output buffers;                                                     */
        /* IP packet therefore,                                                           */
        /* packet points to ethernet header but only want to send IP packet               */
        /* (along with PPP header which will be sent first by ppp_raw), therefore,        */
        /* rs232_xmit_init sets up pointer to IP part of output packet                    */
        /* NOTE: size of ether header is larger than PPP header, therefore,               */
        /*       overall PPP packet is smaller than ether packet, therefore,              */
        /*       input will not overflow packet; NOTE: largest packet size                */
        /*       is based on xn_pkt_data_max (based upon MAX_PACKETSIZE) and MTU value    */
        /* NOTE: this has to be done after compressing in case compression                */
        /*       allocates a new packet                                                   */
        pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
#if (PPP_ORIG)
        pif_info->out_buf_esc_in = pif_info->out_buf_esc_out = 
               pif_info->out_buf_esc_base;
        pif_info->nout_esc = 0;
#endif
        pif_info->packet_out = DCUTODATA(msg) + ETH_HLEN_BYTES;
        pif_info->packet_out_left = DCUTOPACKET(msg)->length - ETH_HLEN_BYTES;
    }
#if (INCLUDE_TRK_PPP)
    track_ppp_output(DCUTOPACKET(msg)->protocol, DCUTODATA(msg), 
                     DCUTOPACKET(msg)->length);
#endif
    {
        struct ppp_hdr ph;
        PLCPS lcp_p = (PLCPS)ppp_p->fsm[Lcp].pdv;
        int full_lcp;
        byte chr;
        /* set up PPP header (which will be sent first)     */
        ph.addr     = HDLC_ALL_ADDR;
        ph.control  = HDLC_UI;
        ph.protocol = DCUTOPACKET(msg)->protocol;

        pif_info->out_buf_calc_fcs = HDLC_FCS_START;
        pif_info->out_buf_accm = CFG_LCP_ACCM_DFLT;

        ppp_p->OutTxOctetCount += DCUTOPACKET(msg)->length + PPP_HDR_LEN + 2;  
                                /* count FCS bytes                    */

        /* if not LCP and negotiated ACCM, get map of which bits to escape     */
        if ( ( full_lcp = (ph.protocol == PPP_LCP_PROTOCOL) ) == FALSE &&
             (lcp_p->remote_entry.work.negotiate & LCP_N_ACCM) ) 
        {
            pif_info->out_buf_accm = lcp_p->remote_entry.work.accm;
        }

        /* Send an opening flag even if the previous packet is still      */
        /* being transmitted. tbd                                         */
        *pif_info->out_buf_esc_in++ = HDLC_FLAG; 
        pif_info->nout_esc++;
        ppp_p->OutOpenFlag++;

        /* Copy header with proper values                                          */
        /* Discard HDLC address and control fields if not LCP and negotiated       */
        if (full_lcp || !(lcp_p->remote_entry.work.negotiate & LCP_N_ACFC)) 
        {
            chr = ph.addr;
            PPP_QUEUE(chr)
            pif_info->out_buf_calc_fcs = (word)(PPPFCS(pif_info->out_buf_calc_fcs, ph.addr));
            chr = (byte)ph.control;
            PPP_QUEUE(chr)
            pif_info->out_buf_calc_fcs = (word)(PPPFCS(pif_info->out_buf_calc_fcs, ph.control));
        }

        if (full_lcp || !(lcp_p->remote_entry.work.negotiate & LCP_N_PFC) ||
            (ph.protocol >= 0x00ff) )
        {
            chr = (byte)(ph.protocol >> 8);
            PPP_QUEUE(chr)
            pif_info->out_buf_calc_fcs = (word)(PPPFCS(pif_info->out_buf_calc_fcs, 
                                                   (ph.protocol >> 8)));
        }
        chr = (byte)(ph.protocol & 0x00ff);
        PPP_QUEUE(chr)
        pif_info->out_buf_calc_fcs = (word)(PPPFCS(pif_info->out_buf_calc_fcs, 
                                               ph.protocol & 0x00ff));
        }

        /* escape buffer and start transmitter     */
        if(ppp_xmit_escape(pif_info))
        {
            return(0);
        }
        else
        {
            return(-1);
        }
}

/* ********************************************************************     */
RTIP_BOOLEAN ppp_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PRS232_IF_INFO pif_info;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];

    if (success)
    {
        if(msg) 
        {
            /* Update total number of successfully transmitted bytes.         */
            pif_info->stats.bytes_out += DCUTOPACKET(msg)->length;
        }
    }
    else
    {
      /* Record error statistics                                            */
      pif_info->stats.errors_out++;
    }
    return(TRUE);
}


/***********************************************************************   */
/* PROCESS INCOMING PACKET                                                 */
/***********************************************************************   */
/* Process incoming PPP packets - called from interpret task               */
/* Will free packet                                                        */
/* NOTE: msg is incoming packet without PPP header and FSC                 */
/* NOTE: IP pkts have ethernet header                                      */
void ppp_proc(PIFACE pi, DCU msg, struct ppp_hdr_alias ph)
{
PPPPS ppp_p;
PFBYTE pfByte;
int i;
#if (INCLUDE_PPP_VANJC)
    PIPCPS ipcp_p;
#endif

    DEBUG_LOG("ppp_proc() - entered", LEVEL_3, NOVAR, 0, 0);
#if (DISPLAY_PKTS)
    DEBUG_LOG("PACKET IN = ", LEVEL_3, PKT, (PFBYTE)DCUTOETHERPKT(msg), 
        DCUTOPACKET(msg)->length);
#endif
#if (DISPLAY_PKTS || DISPLAY_INPUT_PKTS)
    DEBUG_ERROR("PPP PACKET IN = ", PKT, (PFBYTE)DCUTOETHERPKT(msg), 
        DCUTOPACKET(msg)->length);
#endif

#if (INCLUDE_TRK_PPP)
    track_ppp_input(ph.protocol, DCUTODATA(msg), DCUTOPACKET(msg)->length);
#endif

    ppp_p = pi->edv;
    ppp_p->InRxOctetCount += DCUTOPACKET(msg)->length + 2;   /* count FCS bytes */

    switch (ph.protocol) 
    {
    case PPP_IP_PROTOCOL:   /* Regular IP */
        DEBUG_LOG("ppp_proc() - IP pkt", LEVEL_3, NOVAR, 0, 0);
#if (DISPLAY_INPUT_PKTS)
        DEBUG_ERROR("IP PACKET INPUT", NOVAR, 0, 0);
#endif
        if ( ppp_p->fsm[IPcp].ppp_state != fsmOPENED ) 
        {
            DEBUG_LOG("ERROR: ppp_proc() - InError - IP pkt - IPcp state is not OPENED", LEVEL_3, NOVAR, 0, 0);
            ppp_error(msg, 0);
            ppp_p->InError++;
            break;
        }
        OS_SNDX_IP_EXCHG(pi, msg);
        break;

#if (INCLUDE_PPP_VANJC)
    case PPP_COMPR_PROTOCOL:    /* Van Jacobson Compressed TCP/IP */
#if (DISPLAY_INPUT_PKTS)
        DEBUG_ERROR("COMPR PACKET INPUT", NOVAR, 0, 0);
#endif
        if ( ppp_p->fsm[IPcp].ppp_state != fsmOPENED ) 
        {
            ppp_skipped(ppp_p, msg, 0); /* not open for Compressed TCP/IP traffic */
                                        /* NOTE: will free pkt   */
            ppp_p->InError++;
            break;
        }

        ipcp_p = (PIPCPS) ppp_p->fsm[IPcp].pdv;
        if (!(ipcp_p->local_entry.work.negotiate & IPCP_N_COMPRESS)) 
        {
            ppp_skipped( ppp_p, msg, 0); /*Compressed TCP/IP not enabled */
                                         /* NOTE: will free pkt   */
            ppp_p->InError++;
            break;
        }

        if ( slhc_uncompress(ipcp_p->slhcp, msg) <= 0 ) 
        {
            DEBUG_ERROR("Compressed TCP/IP packet error", NOVAR, 0, 0);
            ppp_error(msg, 0);
                                        /* NOTE: will free pkt   */
            ppp_p->InError++;
            break;
        }
        OS_SNDX_IP_EXCHG(pi, msg);
        break;

    case PPP_UNCOMP_PROTOCOL:   /* Van Jacobson Uncompressed TCP/IP */
#if (DISPLAY_INPUT_PKTS)
        DEBUG_ERROR("UNCOMP PACKET INPUT", NOVAR, 0, 0);
#endif
        if ( ppp_p->fsm[IPcp].ppp_state != fsmOPENED ) 
        {
            ppp_skipped( ppp_p, msg, 0); /*not open for Uncompressed TCP/IP traffic */
                                         /* NOTE: will free pkt   */
            ppp_p->InError++;
            break;
        }

        ipcp_p = (PIPCPS)ppp_p->fsm[IPcp].pdv;
        if (!(ipcp_p->local_entry.work.negotiate & IPCP_N_COMPRESS)) 
        {
            ppp_skipped( ppp_p, msg, 0);    /*Uncompressed TCP/IP not enabled */
                                            /* NOTE: will free pkt   */
            ppp_p->InError++;
            break;
        }

        if ( slhc_remember(ipcp_p->slhcp, msg) <= 0 ) 
        {
            ppp_error(msg, 0);  /* Uncompressed TCP/IP packet error */
                                /* NOTE: will free pkt   */
            ppp_p->InError++;
            break;
        }
        OS_SNDX_IP_EXCHG(pi, msg);
        break;
#endif

    case PPP_LCP_PROTOCOL:  /* Link Control Protocol */
        DEBUG_LOG("ppp_proc() - LCP pkt", LEVEL_3, NOVAR, 0, 0);
#if (DISPLAY_INPUT_PKTS)
        DEBUG_ERROR("LCP PACKET INPUT", NOVAR, 0, 0);
#endif
        ppp_p->InNCP[Lcp]++;
        fsm_proc(pi, &(ppp_p->fsm[Lcp]), msg);  /* NOTE: will free pkt */
        break;

#if (INCLUDE_PAP)
    case PPP_PAP_PROTOCOL:  /* Password Authenticate Protocol */
        DEBUG_LOG("ppp_proc() - PAP pkt", LEVEL_3, NOVAR, 0, 0);
#if (DISPLAY_INPUT_PAP)
        DEBUG_ERROR("IP PACKET INPUT", NOVAR, 0, 0);
#endif
        if (ppp_p->ppp_phase != pppAP && ppp_p->ppp_phase != pppREADY) 
        {
            ppp_error(msg, 0);  /* not ready for Authentication */
                                        /* NOTE: will free pkt   */
            DEBUG_LOG("ERROR: ppp_proc() - PAP pkt - incorrect phase", LEVEL_3, NOVAR, 0, 0);
            ppp_p->InError++;
            break;
        }
        ppp_p->InNCP[Pap]++;
        pap_proc(&(ppp_p->fsm[Pap]),msg);   /* NOTE: will free pkt */
        break;
#endif

#if (INCLUDE_CHAP)
    case PPP_CHAP_PROTOCOL: /* Challange-Handshake Authenticate Protocol */
        DEBUG_LOG("ppp_proc() - CHAP pkt", LEVEL_3, NOVAR, 0, 0);
#if (DISPLAY_INPUT_PKTS)
        DEBUG_ERROR("CHAP PACKET INPUT", NOVAR, 0, 0);
#endif
        if (ppp_p->ppp_phase != pppAP && ppp_p->ppp_phase != pppREADY) 
        {
            ppp_error(msg, 0);      /* not ready for Authentication */
                                    /* NOTE: will free pkt   */
            DEBUG_LOG("ERROR: ppp_proc() - CHAP pkt - incorrect phase", LEVEL_3, NOVAR, 0, 0);
            ppp_p->InError++;
            break;
        }
        ppp_p->InNCP[Chap]++;
        chap_proc(&(ppp_p->fsm[Chap]), msg, ppp_p->ppp_phase);  
                                                    /* NOTE: will free pkt   */
        break;
#endif

    case PPP_IPCP_PROTOCOL: /* IP Control Protocol */
        DEBUG_LOG("ppp_proc() - IPCP pkt", LEVEL_3, NOVAR, 0, 0);
#if (DISPLAY_INPUT_PKTS)
        DEBUG_ERROR("IPCP PACKET INPUT", NOVAR, 0, 0);
#endif
        if (ppp_p->ppp_phase != pppREADY) 
        {
            ppp_error(msg, 0);  /* not ready for IPCP traffic */
                                        /* NOTE: will free pkt   */
            DEBUG_LOG("ERROR: ppp_proc() - IPCP pkt - phase not ready", LEVEL_3, NOVAR, 0, 0);
            ppp_p->InError++;
            break;
        }
        ppp_p->InNCP[IPcp]++;
        fsm_proc(pi, &(ppp_p->fsm[IPcp]),msg);  /* NOTE: will free packet */
        break;

    default:
        DEBUG_LOG("ERROR: ppp_proc - unknown packet protocol", LEVEL_2, 
            DINT1, ph.protocol, 0);
        DEBUG_ERROR("ERROR: ppp_proc - unknown packet protocol", 
            EBS_INT1, ph.protocol, 0);
        ppp_p->InUnknown++;

        /* shift the rejected information over 6 bytes and insert the rejected   */
        /* protocol into the 5th and 6th bytes (the first 4 bytes for the        */
        /* PPP header                                                            */
        pfByte = DCUTODATA(msg) + DCUTOPACKET(msg)->length - 1;
        for (i= DCUTOPACKET(msg)->length; i>0; i--, pfByte--)
            *(pfByte + 6) = *pfByte;
        DCUTOPACKET(msg)->length = DCUTOPACKET(msg)->length + 
            IN_PKT_HDR_LEN + 2;
        *((PFWORD)(DCUTODATA(msg) + IN_PKT_HDR_LEN)) = hs2net(ph.protocol);

        /* Send Protocol Reject as an LCP packet   */
        fsm_send(&(ppp_p->fsm[Lcp]), PROT_REJ, 0, msg); 
                            /* NOTE: packet will be freed by IP layer   */
                            /*       when xmit done                     */
        break;
    };
}


/* ********************************************************************   */
/* PPP OPEN                                                               */
/* ********************************************************************   */
/* called by interface open                                               */
RTIP_BOOLEAN ppp_open(PIFACE pi)
{
PPPPS ppp_p;
PRS232_IF_INFO pif_info;

    /* Now put in a dummy ethernet address        */
    tc_movebytes(pi->addr.my_hw_addr, dummy_en_addr, ETH_ALEN); /* Get the ethernet address */

    /* set up interface structure   */
    ppp_p = os_alloc_ppp_cb(pi);
    if (!ppp_p)
    {
        DEBUG_ERROR("os_alloc_ppp_cb failed", NOVAR, 0, 0);
        set_errno(EIFACEOPENFAIL);  /* tbd - errno might already be set */
        return(FALSE);
    }

    pi->edv = ppp_p;
    ppp_p->iface = pi;
    ppp_p->ppp_phase = pppDEAD;

    /* Reset state machines i.e. state, retry info, config parameter   */
    ppp_p->fsm[Lcp].ppp_state  = fsmCLOSED;
    ppp_p->fsm[Pap].ppp_state  = fsmCLOSED;
    ppp_p->fsm[Chap].ppp_state = fsmCLOSED;
    ppp_p->fsm[IPcp].ppp_state = fsmCLOSED;

    /* allocate data structure for protocols   */
    if (!lcp_open(ppp_p) || !ipcp_open(ppp_p)
#if (INCLUDE_CHAP)
        || !chap_open(ppp_p)
#endif
#if (INCLUDE_PAP)
        || !pap_open(ppp_p)
#endif
       )
    {
        DEBUG_ERROR("PPP - protocol open failed", NOVAR, 0, 0);
        set_errno(EIFACEOPENFAIL);  /* tbd - errno might already be set */
        return(FALSE);
    }

    /* clear statistics; NOTE: minor_number was setup before this routine   */
    /* was called                                                           */
    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
    tc_memset((PFBYTE)&(pif_info->stats), 0, sizeof(struct rs232_statistics));

    OS_BIND_PPP_SIGNAL(pi);

    if (!pi->pdev)
    {
        DEBUG_ERROR("ppp_open: pdev not set up yet", NOVAR, 0, 0);
        return(FALSE);
    }

    if (pi->pdev->device_id != PPPOE_DEVICE)
    {
        if (!if_rs232_open_init(pi))
        {
            set_errno(EDEVOPENFAIL);
            DEBUG_ERROR("rs232_init failed", NOVAR, 0, 0);
            return(FALSE);
        }
    }

    return(TRUE);
}

/* ********************************************************************   */
/* PPP INIT                                                               */
/* ********************************************************************   */
/* Initialize PPP control structures for a Point-to-Point interface -
   call when PPP in initialized (xn_ppp_init) */
void ppp_init(PPPPS ppp_p, RTIP_BOOLEAN async_link)
{
    DEBUG_LOG("ppp_init called", LEVEL_3, NOVAR, 0, 0);

    ppp_p->ppp_phase = pppDEAD;
#if (DEBUG_PHASE)
    DEBUG_ERROR("ppp_init: go to DEAD", NOVAR, 0, 0);
#endif

    lcp_init(ppp_p, async_link);
#if (INCLUDE_PAP)
    pap_init(ppp_p);
#endif
#if (INCLUDE_CHAP)
    chap_init(ppp_p);
#endif
    ipcp_init(ppp_p);
}


/* ********************************************************************   */
/* PPP CLOSE                                                              */
/* ********************************************************************   */
int ppp_free(PIFACE pi)
{
PPPPS ppp_p = pi->edv;
int fsmi;

    for ( fsmi = Lcp; fsmi < fsmi_Size; ) 
    {
        fsm_free( (PFSMS)&(ppp_p->fsm[fsmi++]) );
    }
    ppp_p->peername[0] = '\0';
    os_free_ppp_cb(ppp_p);
    return 0;
}

/***********************************************************************  */
/* Close connection on PPP interface                                      */
int proc_fsm_close(PIFACE pi, int fsmi)
{
PFSMS fsm_p;
                                                    
    if (!pi->edv)
        return(-1);

    fsm_p = (PFSMS)&(pi->edv->fsm[fsmi]);

    fsm_p->flags &= (byte)(~(FSM_ACTIVE | FSM_PASSIVE));

    /* close the connection; possibly send term req depending upon state   */
    fsm_close(fsm_p);

    return 0;
}


/***********************************************************************  */
/* called from interface close                                            */
void ppp_close(PIFACE pi)
{
    fsm_down((PFSMS)&(pi->edv->fsm[Lcp]));
    fsm_down((PFSMS)&(pi->edv->fsm[Pap]));
    fsm_down((PFSMS)&(pi->edv->fsm[Chap]));
    fsm_down((PFSMS)&(pi->edv->fsm[IPcp]));

    ppp_free(pi);

    rs232_close(pi);

#if (DEBUG_SIGNAL)
    DEBUG_ERROR("ppp_close: set signal", NOVAR, 0, 0);
    ppp_signal_set();
#endif

    /* signal LCP open; NOTE: the state will not be OPEN so xn_lcp_open()   */
    /* will return failure                                                  */
    pi->ctrl.signal_status = PPP_SIGNAL_FAIL;
    OS_SET_PPP_SIGNAL(pi);
}

/* ********************************************************************   */
/* PPP STATISTICS                                                         */
/* ********************************************************************   */
RTIP_BOOLEAN ppp_statistics(PIFACE pi)
{
PPPPS ppp_p;
dword errs;
PRS232_IF_INFO pif_info;
PRS232_STATS p;

    pif_info = (PRS232_IF_INFO)&rs232_if_info_arry[pi->minor_number];
    p = (PRS232_STATS) &(pif_info->stats);

    UPDATE_SET_INFO(pi, interface_packets_in, p->packets_in)
    UPDATE_SET_INFO(pi, interface_packets_out, p->packets_out)
    UPDATE_SET_INFO(pi, interface_bytes_in, p->bytes_in)
    UPDATE_SET_INFO(pi, interface_bytes_out, p->bytes_out)
    UPDATE_SET_INFO(pi, interface_packets_lost, p->packets_lost)
    UPDATE_SET_INFO(pi, interface_errors_out, p->errors_out)

    /* check if interface has a PPP control block   */
    if (!pi || !pi->edv) 
        return(FALSE);

    ppp_p = pi->edv;

    errs = ppp_p->InUnknown +       /* # unknown packets received */
           ppp_p->InChecksum +      /* # packets with bad checksum */
           ppp_p->InFrame +         /* # packets with frame error */
           ppp_p->InError +         /* # packets with other error */
           ppp_p->InMemory +        /* # alloc failures */
           pif_info->stats.errors_in;
    
    if (errs != 0)
    {
        DEBUG_LOG("ERROR: InUnknown = ", LEVEL_3, LINT1, ppp_p->InUnknown, 0);
        DEBUG_LOG("ERROR: InChecksum = ", LEVEL_3, LINT1, ppp_p->InChecksum, 0);
        DEBUG_LOG("ERROR: InFrame = ", LEVEL_3, LINT1, ppp_p->InFrame, 0);
        DEBUG_LOG("ERROR: InError = ", LEVEL_3, LINT1, ppp_p->InError, 0);
        DEBUG_LOG("ERROR: InMemory = ", LEVEL_3, LINT1, ppp_p->InMemory, 0);
        DEBUG_LOG("ERROR: packets_lost, errors_out = ", LEVEL_3, LINT2, 
            p->packets_lost, p->errors_out);
    }

    UPDATE_SET_INFO(pi, interface_errors_in, errs)

    return(TRUE);
}

#if (INCLUDE_POSTMESSAGE)
/***********************************************************************   */
/* TRANS STATE AND PHASE                                                   */
/***********************************************************************   */
void ppp_trans_state(PFSMS fsm_s, byte new_state)
{
    fsm_s->ppp_state = new_state;

#if (INCLUDE_POSTMESSAGE)
    PostMessage(fsm_s->ppp_p->hwnd, fsm_s->ppp_p->iface->ctrl.index, 
                PPP_STATE_TRANS, new_state);
#endif
}

void ppp_trans_phase(PPPPS ppp_p, byte new_phase)
{
    ppp_p->ppp_phase = new_phase;

#if (INCLUDE_POSTMESSAGE)
    PostMessage(ppp_p->hwnd, ppp_p->iface->ctrl.index, 
        PPP_PHASE_TRANS, new_phase);
#endif
}
#endif  /* INCLUDE_POSTMESSAGE */

/***********************************************************************   */
/* MEMORY ALLOCATION                                                       */
/***********************************************************************   */

/***********************************************************************  */
PPPPS os_alloc_ppp_cb(PIFACE pi)
{
PPPPS ppp_p;
int   ppp_off;

    ppp_off = pi->minor_number;
    if (ppp_off  >= CFG_NUM_PPP)
        return((PPPPS)0);

    ppp_p = (PPPPS)&ppp_cb[ppp_off];
    tc_memset((PFBYTE)ppp_p, 0, sizeof(*ppp_cb));
    ppp_p->id = 1;
    return(ppp_p);
}

/***********************************************************************  */
void os_free_ppp_cb(PPPPS ppp_p)
{
    /* currently does not need to do anything   */
    ARGSUSED_PVOID(ppp_p);
}

/***********************************************************************  */
PIPCPS os_alloc_ipcp_cb(PIFACE pi)
{
PIPCPS ipcp_p;
int   ppp_off;

    ppp_off = pi->minor_number;
    if (ppp_off  >= CFG_NUM_PPP)
        return((PIPCPS)0);

    ipcp_p = (PIPCPS)&ipcp_cb[ppp_off];
    return(ipcp_p);
}

/***********************************************************************  */
void os_free_ipcp_cb(PIPCPS ipcp_p)
{
    /* currently does not need to do anything   */
    ARGSUSED_PVOID(ipcp_p);
}

/***********************************************************************  */
PPAPS os_alloc_pap_cb(PIFACE pi)
{
PPAPS pap_p;
int   ppp_off;

    ppp_off = pi->minor_number;
    if (ppp_off  >= CFG_NUM_PPP)
        return((PPAPS)0);

    pap_p = (PPAPS)&pap_cb[ppp_off];
    return(pap_p);
}

/***********************************************************************  */
void os_free_pap_cb(PPAPS pap_p)
{
    /* currently does not need to do anything   */
    ARGSUSED_PVOID(pap_p);
}

#if (INCLUDE_CHAP)
/***********************************************************************  */
PCHAPS os_alloc_chap_cb(PIFACE pi)
{
struct chap_s KS_FAR *chap_p;
int   ppp_off;

    ppp_off = pi->minor_number;
    if (ppp_off  >= CFG_NUM_PPP)
        return((PCHAPS)0);

    chap_p = (PCHAPS)&chap_cb[ppp_off];
    return(chap_p);
}

/***********************************************************************  */
void os_free_chap_cb(PCHAPS pap_p)
{
    /* currently does not need to do anything   */
    ARGSUSED_PVOID(pap_p);
}
#endif

/***********************************************************************  */
PLCPS os_alloc_lcp_cb(PIFACE pi)
{
struct lcp_s KS_FAR *lcp_p;
int   ppp_off;

    ppp_off = pi->minor_number;
    if (ppp_off  >= CFG_NUM_PPP)
        return((PLCPS)0);

    lcp_p = (PLCPS)&lcp_cb[ppp_off];
    return(lcp_p);
}

/***********************************************************************  */
void os_free_lcp_cb(PLCPS lcp_p)
{
    /* currently does not need to do anything   */
    ARGSUSED_PVOID(lcp_p);
}



#if (INCLUDE_TRK_PPP)
/***********************************************************************   */
/* TRACK PACKETS                                                           */
/***********************************************************************   */

struct _trk_ppp_msg KS_FAR track_ppp_msg[CFG_TRK_NUM_PPP];
int KS_FAR trk_ppp_off = 0;

KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR lcp_option_length[LCP_OPTION_LIMIT+1];
KS_EXTERN_GLOBAL_CONSTANT byte KS_FAR ipcp_option_length[IPCP_OPTION_LIMIT+1];

void track_ppp_reset(void)
{
    trk_ppp_off = 0;
}

void track_ppp_input(word protocol, PFBYTE pb, int len)
{
    if (trk_ppp_off < CFG_TRK_NUM_PPP)
    {
        track_ppp_msg[trk_ppp_off].input = TRUE;
        if (len > CFG_TRK_PPP_LEN)
            len = CFG_TRK_PPP_LEN;
        tc_movebytes(track_ppp_msg[trk_ppp_off].packet, pb, len);
        track_ppp_msg[trk_ppp_off].length = len;
        track_ppp_msg[trk_ppp_off].protocol = protocol;

        trk_ppp_off++;
    }
}

void track_ppp_output(word protocol, PFBYTE pb, int len)
{
    if (trk_ppp_off < CFG_TRK_NUM_PPP)
    {
        track_ppp_msg[trk_ppp_off].input = FALSE;
        if (len > CFG_TRK_PPP_LEN)
            len = CFG_TRK_PPP_LEN;
        tc_movebytes(track_ppp_msg[trk_ppp_off].packet, pb, len);
        track_ppp_msg[trk_ppp_off].length = len;
        track_ppp_msg[trk_ppp_off].protocol = protocol;

        trk_ppp_off++;
    }
}

void cat_opt_val(char *out_str, PFBYTE pb, int olen)
{
byte val;

    switch (olen)
    {
    case 0:
        break;
    case 1:
        val = *pb;
        tc_ltoa((dword)val, out_str+tc_strlen(out_str), 16);
        break;
    case 2:
        tc_ltoa(net2hs(*(PFWORD)pb), out_str+tc_strlen(out_str), 16);
        break;
    case 4:
        /* tc_ltoa(net2hl(*(PFDWORD)pb), out_str+tc_strlen(out_str), 16);   */
        tc_ltoa(net2hl(byte_to_long((PFDWORD)pb)), 
                out_str+tc_strlen(out_str), 16);
        break;
    default:
        break;
    }
}

void cat_opt_val_paran(char *out_str, PFBYTE pb, int olen)
{
    if (olen > 0)
        tc_strcat(out_str, "(");
    cat_opt_val(out_str, pb, olen);
    if (olen > 0)
        tc_strcat(out_str, "), ");
    else
        tc_strcat(out_str, ", ");
}

byte trk_format_code(char *out_str, PFBYTE pb, RTIP_BOOLEAN alt)
{
    switch (*pb)
    {
    case CONFIG_REQ :       /* case CONFIG_CHAL is same */
        if (!alt)
            tc_strcat(out_str, "CONF REQ: ");
        else
            tc_strcat(out_str, "CONF CHAL:");
        break;
    case CONFIG_ACK :       /* case CONFIG_RESP is same */
        if (!alt)
            tc_strcat(out_str, "CONF ACK: ");
        else
            tc_strcat(out_str, "CONF RESP:");
        break;
    case CONFIG_NAK :       /* case CONFIG_SUCC is same */
        if (!alt)
            tc_strcat(out_str, "CONF NAK: ");
        else
            tc_strcat(out_str, "CONF SUC: ");
        break;
    case CONFIG_REJ :       /* case CONFIG_FAIL is same */
        if (!alt)
            tc_strcat(out_str, "CONF REJ: ");
        else
            tc_strcat(out_str, "CONF FAIL:");
        break;
    case TERM_REQ:
        tc_strcat(out_str, "TERM REQ:");
        break;
    case TERM_ACK:
        tc_strcat(out_str, "TERM ACK:");
        break;
    case CODE_REJ:
        tc_strcat(out_str, "CODE REJ:");
        break;
    case PROT_REJ:
        tc_strcat(out_str, "PROT REJ:");
        break;
    case ECHO_REQ:
        tc_strcat(out_str, "ECHO REQ:");
        break;
    case ECHO_REPLY :
        tc_strcat(out_str, "ECHO REP:");
        break;
    case DISCARD_REQ:
        tc_strcat(out_str, "DISCARD: ");
        break;
    case QUALITY_REPORT:        /* case ID_LCP: */
        if (!alt)
            tc_strcat(out_str, "QUAL:     ");
        else
            tc_strcat(out_str, "ID LCP:   ");
        break;
    }
    return(*pb);
}

void trk_format_lcp(char *out_str, PFBYTE pb, int length)
{
int olen;
word alg;

    while (length)
    {
        if (*pb > LCP_OPTION_LIMIT)
        {
            tc_strcat(out_str, "UNKNOWN OPTION");
            return;     /* out of range: can't handle */
        }
        if (*pb == 0)
        {
            tc_strcat(out_str, "OPTION=0");
            return;     /* avoid infinite loop since olen will be 0 */
        }

        olen = lcp_option_length[*pb];  
        switch (*pb)
        {
        case LCP_MRU:
            tc_strcat(out_str, "MRU");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case LCP_ACCM:          
            tc_strcat(out_str, "ACCM");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case LCP_AUTHENT:
            tc_strcat(out_str, "AUTHENT");
            alg = net2hs(*((PFWORD)(pb+2)));
            if (alg == PPP_CHAP_PROTOCOL)  
            {
                tc_strcat(out_str, "(CHAP,ALGORITHM=");
                cat_opt_val_paran(out_str, pb+4, 1);
                tc_strcat(out_str, "),");
                olen++;
            }
            else if (alg == PPP_PAP_PROTOCOL) 
                tc_strcat(out_str, "(PAP),");
            else
                cat_opt_val_paran(out_str, pb+2, olen-2);   /* unknown protocol */
            break;
        case LCP_ENCRYPT:           
            tc_strcat(out_str, "ENCRYPT");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case LCP_MAGIC:         
            tc_strcat(out_str, "MAGIC");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case LCP_QUALITY:
            tc_strcat(out_str, "QUALITY");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case LCP_PFC:       
            tc_strcat(out_str, "PFC");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case LCP_ACFC:          
            tc_strcat(out_str, "ACFC");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        }
        length -= olen;
        pb     += olen;
    }
}

void trk_format_ipcp(char *out_str, PFBYTE pb, int length)
{
int olen;
byte option;

    while (length)
    {
        option = *pb;
        if (option ==  IPCP_DNS_PRIMARY)
            option = IPCP_DNS_PRIMARY_SH;  
        else if (option ==  IPCP_DNS_SECOND)
            option = IPCP_DNS_SECOND_SH;         

        if (option > IPCP_OPTION_LIMIT)
        {
            tc_strcat(out_str, "UNKNOWN OPTION");
            return;     /* out of range: can't handle */
        }
        if (option == 0)
        {
            tc_strcat(out_str, "OPTION=0");
            return;     /* avoid infinite loop since olen will be 0 */
        }

        olen = ipcp_option_length[option];
        switch (option)
        {
        case IPCP_ADDRESSES:
            tc_strcat(out_str, "ADDRS");
            cat_opt_val_paran(out_str, pb+2, olen-2);
            break;
        case IPCP_COMPRESS:
            tc_strcat(out_str, "COMP");
            tc_strcat(out_str, "(");
            cat_opt_val(out_str, pb+2, 2);
            tc_strcat(out_str, ",");
            cat_opt_val(out_str, pb+4, 1);
            tc_strcat(out_str, ",");
            cat_opt_val(out_str, pb+5, 1);
            tc_strcat(out_str, "), ");
            break;
        case IPCP_ADDRESS:
            tc_strcat(out_str, "ADDR");
            tc_strcat(out_str, "(");
            ip_bin_to_str(out_str+tc_strlen(out_str), (PFBYTE)(pb+2));
            tc_strcat(out_str, "), ");
            break;
        case IPCP_DNS_PRIMARY_SH:
            tc_strcat(out_str, "DNS1");
            tc_strcat(out_str, "(");
            ip_bin_to_str(out_str+tc_strlen(out_str), (PFBYTE)(pb+2));
            tc_strcat(out_str, "), ");
            break;
        case IPCP_DNS_SECOND_SH:
            tc_strcat(out_str, "DNS2");
            tc_strcat(out_str, "(");
            ip_bin_to_str(out_str+tc_strlen(out_str), (PFBYTE)(pb+2));
            tc_strcat(out_str, "), ");
            break;
        default:
            tc_strcat(out_str, "UNKNOWN OPTION");
            break;
        }
        length -= olen;
        pb     += olen;
    }
}

void trk_format_pap(char *out_str, PFBYTE pb, int length)
{
int len;

    ARGSUSED_INT(length);

    len = tc_strlen(out_str);
    tc_strcat(out_str+len, "(");
    tc_movebytes((PFBYTE)(out_str+len+1), pb+1, *pb);
    *(out_str + len + *pb + 1) = '\0';
    pb = pb + *pb + 1;

    tc_strcat(out_str, ",");
    len = tc_strlen(out_str);
    tc_movebytes((PFBYTE)(out_str+len), pb+1, *pb);
    *(out_str + len + *pb) = '\0';

    tc_strcat(out_str, "), ");
}

void break_trk_ppp(void)
{
}

void trk_format_chap(char *out_str, PFBYTE pb, int length, byte code)
{
int i, len;

    ARGSUSED_INT(length);

    if (length < 0)
    {
        break_trk_ppp();
        return;
    }
        
    if ( (code != CONFIG_FAIL) && (code != CONFIG_SUCC) )
    {
        tc_strcat(out_str, "(Value:");
        for (i=0; i<*pb; i++)
        {
            tc_itoa(*(pb+i+1)>>4, (PFCHAR)(out_str+tc_strlen(out_str)), 16);
            tc_itoa(*(pb+i+1)&0xf, (PFCHAR)(out_str+tc_strlen(out_str)), 16);
        }
        length = length - (*pb+1);
        if (length < 0)
        {
            break_trk_ppp();
            return;
        }
        pb = pb + *pb + 1;

        tc_strcat(out_str, ", Name:");
    }

    else
        tc_strcat(out_str, "(Message:");

    len = tc_strlen(out_str);
    tc_movebytes(out_str+len, pb, length);
    *(out_str + len + length) = '\0';
    tc_strcat(out_str, "), ");
}

void trk_format_pkt(char *out_str, PFBYTE pb, int length)
{
    if (length > (CFG_TRK_PPP_LEN-5))
        length = CFG_TRK_PPP_LEN-5;
    tc_movebytes(out_str, pb, length);
}

void display_track_ppp_info(void)
{
int  i;
char out_str[300];
byte code;
int  offset;

    DEBUG_ERROR("display_track_ppp_info: ", NOVAR, 0, 0);
    for (i=0; i < trk_ppp_off; i++)
    {
        if (track_ppp_msg[i].input)
        {
            tc_strcpy(out_str, "PPP: IN: ");
            offset = IN_PKT_HDR_LEN;
        }
        else
        {
            tc_strcpy(out_str, "PPP: OUT:");
            offset = OUT_PKT_HDR_LEN;
        }

        switch (track_ppp_msg[i].protocol)
        {
        case PPP_IP_PROTOCOL:
            tc_strcat(out_str, "IP  :");
            trk_format_pkt(out_str, &(track_ppp_msg[i].packet[0])+offset, 
                           track_ppp_msg[i].length-offset);
            break;
        case PPP_COMPR_PROTOCOL:
            tc_strcat(out_str, "COMP :");
            trk_format_pkt(out_str, &(track_ppp_msg[i].packet[0])+offset, 
                           track_ppp_msg[i].length-offset);
            break;
        case PPP_UNCOMP_PROTOCOL:
            tc_strcat(out_str, "UCOMP:");
            trk_format_pkt(out_str, &(track_ppp_msg[i].packet[0])+offset, 
                           track_ppp_msg[i].length-offset);
            break;
        case PPP_IPCP_PROTOCOL:
            tc_strcat(out_str, "IPCP:");
            trk_format_code(out_str, &(track_ppp_msg[i].packet[0]), FALSE);
            trk_format_ipcp(out_str, &(track_ppp_msg[i].packet[0])+offset, 
                           track_ppp_msg[i].length-offset);
            break;
        case PPP_LCP_PROTOCOL:
            tc_strcat(out_str, "LCP :");
            code = trk_format_code(out_str, &(track_ppp_msg[i].packet[0]), FALSE);
            if (code != TERM_REQ)
                trk_format_lcp(out_str, &(track_ppp_msg[i].packet[0])+offset, 
                               track_ppp_msg[i].length-offset);
            break;
        case PPP_PAP_PROTOCOL:
            tc_strcat(out_str, "PAP :");
            trk_format_code(out_str, (PFBYTE)&(track_ppp_msg[i].packet[0]), FALSE);
            trk_format_pap(out_str, (PFBYTE)&(track_ppp_msg[i].packet[0])+offset, 
                           track_ppp_msg[i].length-offset);
            break;
        case PPP_CHAP_PROTOCOL:
            tc_strcat(out_str, "CHAP:");
            code = trk_format_code(out_str, &(track_ppp_msg[i].packet[0]), TRUE);
            trk_format_chap(out_str, &(track_ppp_msg[i].packet[0])+offset, 
                            track_ppp_msg[i].length-offset, code);
            break;
        default:
            tc_strcat(out_str, "UNKN:");
            break;
        }
/*      DEBUG_ERROR(out_str, PKT, track_ppp_msg[i].packet,    */
/*                    track_ppp_msg[i].length);               */
        DEBUG_ERROR("", STR1, out_str, 0);

#if (TRACK_WAIT)
        if ( ((i % 20) == 19) || (i == (trk_ppp_off-1)) )
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
        }
#endif
    }
}
#endif      /* INCLUDE_TRK_PPP */
#endif      /* INCLUDE_PPP */
