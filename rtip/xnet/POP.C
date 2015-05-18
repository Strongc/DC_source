/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */

/* POP.C - Post Office Protocol   */

#define DIAG_SECTION_KERNEL DIAG_SECTION_MAIL

#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif

#if (INCLUDE_POP3)
#include "popapi.h"
#include "vfile.h"
#include "pop.h"
#include "netutil.h"
#include "base64.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_POP           0       /* print out error msgs */
#define DEBUG_POP_VERBOSE   0       /* print out text sent & text recv'd */
#define DEBUG_TERM          0

/* ********************************************************************   */
/* internal functions                                                     */
int pop_finish(PPOP_CONTEXT context, PMIME_PARSE_INFO parse_info);
int pop_get_resp(PPOP_CONTEXT context, char *buf, int bufsize);
int pop_resp_stat(char *buf);
int pop_send(PPOP_CONTEXT context, KS_CONSTANT char *string);
int pop_LIST(PPOP_CONTEXT context, PPOP_MSG_SIZE mail_sizes, int first_msg, int num_messages);
int pop_login(PPOP_CONTEXT context, PFCHAR username, PFCHAR pass);
int pop_STAT(PPOP_CONTEXT context, PFINT pmessages, PFDWORD pmsgsize);
int pop_connect(PPOP_CONTEXT context, PFBYTE pop_server_ip);
int pop_TOP(PPOP_CONTEXT context, PPOP_MSG_INFO info, int msg_num);
int mime_get_type(PFCHAR buf_ptr);
int pop_get_whole_line(PPOP_CONTEXT context,  PFCHAR *lnbuffer, long tmeout, int type);
void mime_save_info(PMIME_PARSE_INFO parse_info, int *parse_off, 
                    int total_parse, int mime_type, PFCHAR buf_ptr);
RTIP_BOOLEAN mime_extract_subfield(PPOP_CONTEXT context, PFCCHAR sub_hdr, 
                              PFCHAR dest, int dest_len);
int mime_read_boundary(PPOP_CONTEXT context, RTIP_BOOLEAN *pend_boundary);
RTIP_BOOLEAN mime_check_for_boundary(PPOP_CONTEXT context, RTIP_BOOLEAN *end_boundary);
int mime_parse_one_field(PFCHAR buf_ptr, int buf_size, 
                         PMIME_PARSE_INFO parse_info);
void skip_space(PFCHAR *buf_ptr);
void skip_to_space(PFCHAR *buf, PFCHAR buf_end);
int do_mime_read_select(int socket, long tmeout);

/* EXTERNAL GLOBAL DATA   */
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mimesub_boundary_str;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mimesub_filename_str;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mime_msg;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *multipart_str;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *base64_content_type;

KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str1;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str2;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str3;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str4;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str5;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str6;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str7;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str8;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str9;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str10;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str11;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str12;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str13;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str14;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *pop_str15;

#ifdef __cplusplus
extern "C" {
#endif
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mime_term_field;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *base64_alphabet;
#ifdef __cplusplus
}
#endif

/* ********************************************************************   */
/* POP API                                                                */
/* ********************************************************************   */

/* ********************************************************************      */
/* xn_pop_checkmail()    - returns mailbox info                              */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_checkmail(context, info)                                     */
/*     PPOP_CONTEXT context - client context                                 */
/*     PPOP_INFO nfo        - structure in which to put mail info            */
/*                                                                           */
/* Description:                                                              */
/*   Once you are connected to a POP server (i.e. once xn_pop_client()       */
/*   has been called) you may call xn_pop_checkmail() to obtain the          */
/*   number of messages in the mailbox and the size of the mail file.        */
/*   The number of the messages will be put into the num_messages field,     */
/*   and the total size of the mail file will be put into the field          */
/*   mail_size.                                                              */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */
    
int xn_pop_checkmail(PPOP_CONTEXT context, PPOP_INFO info)
{
    /* get number of messages and total size   */
    return(pop_STAT(context, (PFINT)&(info->num_messages), 
                    (PFDWORD)&(info->mail_size)));
}

/* ********************************************************************      */
/* xn_pop_mailsize()     - returns mailbox info                              */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_mailsize(context, mail_sizes, first_msg, len)                */
/*      PPOP_CONTEXT context  - client context                               */
/*      PFLONG mail_sizes     - array to be filled in with mail sizes        */
/*      int first_msg         - first mail message to get mail sizes for     */
/*      int num_msgs          - size of mail_sizes                           */
/*                                                                           */
/* Description:                                                              */
/*   Once you are connected to a POP server (i.e. once xn_pop_client()       */
/*   has been called) you may call xn_pop_mailsize() to obtain the           */
/*   sizes of the mail messages.                                             */
/*                                                                           */
/*   The sizes of messages first_msg through (num_msgs-first_msg) are        */
/*   obtained. The sizes obtained are written to the array                   */
/*   mail_sizes.  For each message, the message size will be put at the      */
/*   index (message number - first_msg), i.e. the size of mail message       */
/*   first_msg is written at offset 0, the size of the first_msg+1           */
/*   is written at offset 1, etc.  Any unsued entries in mail_sizes          */
/*   are set to -1.                                                          */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */
    
int xn_pop_mailsize(PPOP_CONTEXT context, PPOP_MSG_SIZE mail_sizes, 
                    int first_msg, int num_msgs)
{
    /* get number and size    */
    return(pop_LIST(context, mail_sizes, first_msg, num_msgs));
}

/* ********************************************************************      */
/* xn_pop_mailinfo()     - returns mailbox info                              */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_mailinfo(context, info, msg_num)                             */
/*      PPOP_CONTEXT context  - client context                               */
/*      PPOP_MSG_INFO info    - structure in which to put mail info          */
/*      int msg_num           - message number                               */
/*                                                                           */
/* Description:                                                              */
/*   Once you are connected to a POP server (i.e. once xn_pop_client()       */
/*   has been called) you may call xn_pop_mailinfo() to obtain the           */
/*   other information about a mail message, i.e. this function fills in     */
/*   the fields in an instance of struct pop_info.                           */
/*   For each message, the sender's address and the subject will             */
/*   be written to the parameter info.                                       */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */
    
int xn_pop_mailinfo(PPOP_CONTEXT context, PPOP_MSG_INFO info, int msg_num)
{
    /* get from and subject   */
    return(pop_TOP(context, info, msg_num));
}


/* ********************************************************************      */
/* xn_pop_client()  - connects to POP server and checks mail                 */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_client(context, pop_server_ip, username, pass)               */
/*     PPOP_CONTEXT context  - client context                                */
/*     PFBYTE pop_server_ip  - servers IP address                            */
/*     PFCHAR username       - username to log in to POP server              */
/*     PFCHAR pass           - password to log in to POP server              */
/*                                                                           */
/* Description:                                                              */
/*   This function connects to a POP server and logs in.  The function       */
/*   xn_pop_QUIT needs to be called to close the connection after            */
/*   all the information needed (i.e. all calls to xn_pop_RETR_next,         */
/*   xn_pop_checkmail etc) have been made.                                   */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */
    
int xn_pop_client(PPOP_CONTEXT context, PFBYTE pop_server_ip, 
                  PFCHAR username, PFCHAR pass)
{
PIO_CONTEXT pio_context;
int ret_val;

    ret_val = 0;

    pio_context = context->io_context = 
        (PIO_CONTEXT)dcu_alloc_core(sizeof(IO_CONTEXT));

    if (!pio_context)
        ret_val = -1;

    if (ret_val >= 0)
    {
        if (!xn_line_init(pio_context, LINE_INPUT))
            ret_val = -1;
        else
        {
            context->gbuffer = (PFCHAR)pio_context->pb_in;

            context->pop_state = POP_S_START;
            context->pop_save_char = 0;

            ret_val = pop_connect(context, pop_server_ip);

            if (ret_val == 0)
                ret_val = pop_login(context, username, pass);
        }
    }
    if ( (ret_val < 0) && pio_context)
    {
        dcu_free_core(context->io_context);
        context->io_context = (PFBYTE)0;
    }
    return(ret_val);
}

/* ********************************************************************      */
/* xn_pop_RETR_next() - retrieves next part of message from POP server       */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_RETR_next(context, num, parse_info)                          */
/*     PPOP_CONTEXT context        - client context                          */
/*     int num                     - number of mail message to retrieve      */
/*     PMIME_PARSE_INFO parse_info - results are written to this structure   */
/*                                                                           */
/* Description:                                                              */
/*   This function retrieves the next field of a mail message where          */
/*   the possible fields are a MAIL header field, a MIME header field,       */
/*   as much data of MAIL or ATTACHMENT message as can fit in                */
/*   context->pop_buffer (or up to a \r\n).                                  */
/*   The results of the field read are put in parse_info, i.e. the           */
/*   type of field read is put in parse_info->mime_type and                  */
/*   a pointer to the body of the field is written to                        */
/*   parse_info->mime_field_ptr.  The length of the body of the              */
/*   field is written to parse_info->mime_field_len.                         */
/*                                                                           */
/*   NOTE: the same context structure must be passed between calls           */
/*         to this function                                                  */
/*   NOTE: calling this routine overwrites data in context->pop_buffer       */
/*         (i.e parse_info->mime_field_ptr is set to the start of the        */
/*         body in context->pop_buffer), therefore, any data in the          */
/*         buffer from the previous call must be processed before            */
/*         calling this routine.                                             */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_pop_RETR_next(PPOP_CONTEXT context, int num, 
                     PMIME_PARSE_INFO parse_info)
{
PIO_CONTEXT pio_context;
int     len, result;
RTIP_BOOLEAN end_boundary;
RTIP_BOOLEAN done;
int     ret_val;

    pio_context = (PIO_CONTEXT)(context->io_context);  

    done = FALSE;
    end_boundary = FALSE;
    ret_val = 0;

    parse_info->mime_type = -1;
    parse_info->mime_field_ptr = 0;

/*  DEBUG_ERROR("xn_pop_RETR_next() called.",NOVAR,0,0);   */
    for (;;)
    {
        switch (context->pop_state)
        {
        case POP_S_START:
            tc_strcpy(context->gbuffer, pop_str1);
            tc_itoa(num, context->gbuffer+tc_strlen(context->gbuffer), 10);
            tc_strcat(context->gbuffer, MIME_TERM_FIELD);
            pop_send(context, context->gbuffer);

            /* reset flags for the message about to process   */
            context->pop_flags = 0;
           
            pio_context->pb_in[pio_context->begin_offset_in] = '\0';
            len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT, GET_LINE);
            if ((len == -1) || (len == -2) || (len == -3))
                return set_errno(ESRVDOWN);

            result = pop_resp_stat(context->gbuffer);
            if (result == -1)
            {
                xn_pop_RETR_done(context);
                return set_errno(ESRVBADRESP);
            }
            context->pop_state = POP_S_MAIL_HDR;

            /* fall thru to return the first field of the MIME header   */

        case POP_S_MAIL_HDR:
        case POP_S_MIME_MAIL_HDR:
            len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,GET_MAIL);
            if ((len == -1) || (len == -2) || (len == -3))
                return set_errno(ESRVDOWN);
            if (len == TERM_MSG)
            {
#if (DEBUG_TERM)
                DEBUG_ERROR("TERM_MSG: when MAIL_HDR", NOVAR, 0, 0);
#endif
                context->pop_state = POP_S_MSG_DONE;
                context->next_pop_state = POP_S_START;
                done = TRUE;
                break;
            }

            /*DEBUG_ERROR("case mail: len=",DINT1,(dword)len,0);   */

            if (len == TERM_HEADER)
            {
                if (context->pop_state == POP_S_MAIL_HDR)
                {

                    if (context->pop_flags & MULTIPART_FLAG)
                    {
                        context->pop_state = POP_S_MIME_MAIL_HDR;
                        /* read until hit the boundary; this will skip the   */
                        /* "This is a multi-part message in MIME format"     */
                        if (mime_read_boundary(context, 
                                               (RTIP_BOOLEAN *)&end_boundary) < 0)
                        {
                            DEBUG_ERROR("xn_pop_RETR_next: reading boundary failed",
                                NOVAR, 0, 0);
                            return(-1); /* should never happen */
                                        /* if mime_read_boundary returns   */
                                        /* fail, need to set errno         */
                        }
                        if (end_boundary)
                        {
                            ret_val = pop_finish(context, parse_info);
                            done = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        /* if no multipart mime header then the actual   */
                        /* mail message is after the first mime header   */
                        context->pop_state = POP_S_MAIL_START;
                    }
                }       /* end of if MAIL HEADER */
                else if (context->pop_flags & ATTACHMENT_FLAG)
                    context->pop_state = POP_S_ATTACH_START;
                else
                    context->pop_state = POP_S_MAIL_START;
                break;
            }

            else        /* MIME header is not done */
            {
                /* parse the mime header field read above   */
                mime_parse_one_field(context->gbuffer, len, parse_info);

                /* save any relevent info in the mime header   */
                if ( (context->pop_state == POP_S_MAIL_HDR) &&
                     (parse_info->mime_type == MIME_CONTENT_TYPE) )
                {
                    if (tc_strnicmp(parse_info->mime_field_ptr, MULTIPART_STR,
                                tc_strlen(MULTIPART_STR)) == 0)
                    {
                        /* expect a MIME multipart message   */
                        context->pop_flags |= MULTIPART_FLAG;

                        /* save the boundary information   */
                        mime_extract_subfield(context, mimesub_boundary_str,
                                              context->pop_boundary,CFG_POP_MAX_BOUNDARY);
                    }
                }

                /* save any relevent info in the mime header   */
                else if ( (context->pop_state == POP_S_MIME_MAIL_HDR) &&
                          (parse_info->mime_type == MIME_CONTENT_DISP) )
                {
                    /* save the boundary information   */
                    if (mime_extract_subfield(context, mimesub_filename_str,
                                              context->pop_filename,CFG_MAX_FILE_LEN))
                    {
                        context->pop_flags |= ATTACHMENT_FLAG;
                    }
                }

                else if ( (context->pop_state == POP_S_MIME_MAIL_HDR) &&
                          (parse_info->mime_type == MIME_CONTENT_ENCODE) )
                {
                    if (!tc_stricmp(parse_info->mime_field_ptr, 
                                    base64_content_type))
                    {
                        context->pop_flags |= BASE64_FLAG;
                    }
                }
                done = TRUE;
                break;
            }

        case POP_S_MSG_DONE:
            context->pop_state = context->next_pop_state;
            parse_info->mime_type = MSG_DONE;
            done = TRUE;
            break;

        case POP_S_MAIL_START:
            context->pop_state = POP_S_MAIL;
            parse_info->mime_type = MAIL_START;
            done = TRUE;
            break;

        case POP_S_ATTACH_START:
            context->pop_state = POP_S_ATTACH;
            parse_info->mime_type = ATTACH_START;
            done = TRUE;
            break;

        case POP_S_MAIL:
        case POP_S_ATTACH:

            len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,GET_LINE);
            if ((len == -1) || (len == -2) || (len == -3))
                return set_errno(ESRVDOWN);

            /*DEBUG_ERROR("case attach: len=",DINT1,(dword)len,0);   */

            /* fix up what pop_get_whole_line returns; usually /r/n means   */
            /* end of header when reading a MIME header but while reading   */
            /* mail it just means a new line                                */
            if (len == TERM_HEADER)
            {
#if (DEBUG_TERM)
                DEBUG_ERROR("TERM_HEADER", NOVAR, 0, 0);
#endif
                len = 0;
            }

            if (len >= 0) 
            {
                if ( (context->pop_flags & MULTIPART_FLAG) &&
                     mime_check_for_boundary(context, &end_boundary) )
                {
/*                  DEBUG_ERROR("Got boundary at end of ATTACH.",NOVAR,0,0);   */

                    context->next_pop_state = POP_S_MIME_MAIL_HDR;
                    if (context->pop_state == POP_S_MAIL)
                        context->pop_state = POP_S_MAIL_END;
                    else
                        context->pop_state = POP_S_ATTACH_END;

                    if (end_boundary)
                    {
                        ret_val = pop_finish(context, parse_info);
                        done = TRUE;
                        break;
                    }
                }
                else
                {
/*                  if (context->pop_state == POP_S_MAIL)   */
/*                      parse_info->mime_type = MAIL;       */
/*                  else                                    */
                        parse_info->mime_type = ATTACH;
                    parse_info->mime_field_ptr = context->gbuffer;
                    parse_info->mime_field_len = len;
                    done = TRUE;
                    break;
                }
                break;
            }

            /* if TERM_MSG we are done   */
            else if (len == TERM_MSG)
            {
#if (DEBUG_TERM)
                DEBUG_ERROR("TERM_MSG", NOVAR, 0, 0);
#endif
#if (1)
                /* we are done, so return MAIL_END or ATTACH_END;      */
                /* Then next call to xn_pop_RETR_next, return MSG_DONE */
                context->next_pop_state = POP_S_MSG_DONE;
                if (context->pop_state == POP_S_MAIL)
                {
                    parse_info->mime_type = MAIL_END;
                }
                else
                {
                    parse_info->mime_type = ATTACH_END;
                }
#else
                context->next_pop_state = POP_S_MSG_DONE;
                if (context->pop_state == POP_S_MAIL)
                    context->pop_state = POP_S_MAIL_END;
                else
                    context->pop_state = POP_S_ATTACH_END;
                done = TRUE;
                break;
#endif
            }
            /* FALL THRU since did not return   */

        case POP_S_MAIL_END:
        case POP_S_ATTACH_END:
            if (context->pop_state == POP_S_MAIL_END)
                parse_info->mime_type = MAIL_END;
            else
                parse_info->mime_type = ATTACH_END;

            context->pop_state = context->next_pop_state;
            if (context->pop_state == POP_S_MSG_DONE)
                context->next_pop_state = POP_S_START;
            context->pop_flags &= ~(BASE64_FLAG | ATTACHMENT_FLAG);
            done = TRUE;
            break;

        default:
            DEBUG_ERROR("xn_pop_RETR_next: illegal state", NOVAR, 0, 0);
        }       /* end of switch */

        if (done)
            break;
    }   /* end of loop forever */

    if (parse_info->mime_type >= 0)
    {
        parse_info->mime_type_str = 
            mime_headers[parse_info->mime_type].mime_header;
    }
    return(ret_val);
}

/* ********************************************************************   */
int pop_finish(PPOP_CONTEXT context, PMIME_PARSE_INFO parse_info)
{
int len;

    len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,
                             GET_LINE);
    if ((len == -1) || (len == -2) || (len == -3))
        return set_errno(ESRVDOWN);

    if (len != TERM_MSG)
    {
        DEBUG_ERROR("hit end boundary after header", NOVAR, 0, 0);
        return(-1);
    }

    /* return MAIL_END or ATTACH_END                                   */
    /* next call to xn_pop_RETR_next, the state will be POP_S_MSG_DONE */
    /* then POP_S_START                                                */
    if (context->pop_state == POP_S_MAIL)
    {
        parse_info->mime_type = MAIL_END;
    }
    else
    {
        parse_info->mime_type = ATTACH_END;
    }
    context->next_pop_state = POP_S_START;
    context->pop_state = POP_S_MSG_DONE;

    return(0);
}

/* ********************************************************************      */
/* xn_pop_RETR_done() - cleans up after calls to xn_pop_RETR_next            */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_RETR_done(context)                                           */
/*     PPOP_CONTEXT context  - client context                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This routine should be called after all calls to xn_pop_RETR_next       */
/*   to retrieve a mail message have been done.  It reads any                */
/*   residue data from the socket and sets the context to the                */
/*   proper state for reading another mail message.                          */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_pop_RETR_done(PPOP_CONTEXT context)
{
int len;
int ret_val;

    ret_val = 0;

    if (context->pop_state != POP_S_START)
    {
DEBUG_ERROR("xn_pop_RETR_done: state = ", EBS_INT1, context->pop_state, 0);
        for (;;)
        {
            len = pop_get_whole_line(context, &context->gbuffer, 
                                     CFG_POP_TIMEOUT, GET_BUF);
            if ((len == -1) || (len == -2) || (len == -3))
            {
                ret_val = -1;
                set_errno(ESRVDOWN);
                break;
            }
            if (len == TERM_MSG)
                break;
        }
    }
    context->pop_state = POP_S_START;
    return(ret_val);
}

/* ********************************************************************      */
/* xn_pop_DELE() - deletes a mail message                                    */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_DELE(context, num)                                           */
/*     PPOP_CONTEXT context  - client context                                */
/*     int num               - index of message to delete                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This routine deletes a mail message from the POP server.  Each          */
/*   message on the server is identified with an index.  The message         */
/*   whose index is num will be deleted from the server.                     */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_pop_DELE(PPOP_CONTEXT context, int num)
{
char str[30];
int len, result;

    tc_strcpy(context->gbuffer, pop_str2);
    tc_itoa(num, str, 10);
    tc_strcat(context->gbuffer, str);
    tc_strcat(context->gbuffer, MIME_TERM_FIELD);
    pop_send(context, context->gbuffer);

    len = pop_get_resp(context, context->gbuffer, CFG_POP_BUF_SIZE);
    if (len == -1)
        return set_errno(ESRVDOWN);
    result = pop_resp_stat(context->gbuffer);
    if (result == -1)
        return set_errno(ESRVBADRESP);
    return(result);
}

/* ********************************************************************      */
/* xn_pop_QUIT()  - logs out of POP server and closes connection             */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "pop.h"                                                        */
/*                                                                           */
/*   int xn_pop_QUIT(context)                                                */
/*     PPOP_CONTEXT context  - client context                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This routine logs out of the POP server that was connected to via       */
/*   xn_pop_client() and closes the connection.  This routine must           */
/*   be called at the end of any POP client application (i.e. if             */
/*   xn_pop_client returns success).                                         */
/*                                                                           */
/*   For more details see Mail Reference Manual.                             */
/*                                                                           */
/* Returns:                                                                  */
/*   Zero is returned on success. Otherwise -1                               */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int xn_pop_QUIT(PPOP_CONTEXT context)
{
int len, result;
PIO_CONTEXT pio_context;

    pio_context = (PIO_CONTEXT)(context->io_context);  

    pop_send(context,pop_str3);
    len = pop_get_resp(context, context->gbuffer, CFG_POP_BUF_SIZE);
    if (len == -1)
        return set_errno(ESRVDOWN);
    result = pop_resp_stat(context->gbuffer);
    if (result == -1)
        return set_errno(ESRVBADRESP);

    closesocket(context->pop_sock);
    xn_line_done(pio_context);

    if (context->io_context)
        dcu_free_core(context->io_context);

    return(0);
}


/* ********************************************************************   */
/* POP UTILITIES                                                          */
/* ********************************************************************   */

/* reads the response to a POP request   */
int pop_get_resp(PPOP_CONTEXT context, char *buf, int bufsize)
{
int len;
#if (DEBUG_POP_VERBOSE)
int outbufpos;
char outbuf[80];
#endif
     
    if (!do_mime_read_select(context->pop_sock, CFG_POP_TIMEOUT))
        return(-1);

    len = recv(context->pop_sock, buf, bufsize, 0);
    if (len == -1)
    {
#if (DEBUG_POP)
        DEBUG_ERROR("Pop server not responding.", NOVAR, 0, 0);
#endif
        return set_errno(ESRVDOWN);
    }

#if (DEBUG_POP_VERBOSE)
{
char c;
int n;

    tc_strcpy(outbuf,pop_str4);
    outbufpos = 3;
    for (n = 0; n<len; n++)
    {
        if (buf[n] == ASCII_CR)
        {
            if (buf[n+1] == ASCII_LF)
            {
                outbuf[outbufpos] = '\0';
                DEBUG_ERROR(outbuf, NOVAR, 0, 0);
                outbufpos = 3;
                n++;
                if (buf[n+1] == '.' && buf[n+2] == ASCII_CR && buf[n+3] == ASCII_LF)
                    break;
            }
        }
        else
        {
            if (outbufpos<75)
                outbuf[outbufpos] = buf[n];
            outbufpos++;
        }
    }
}
#endif

    return(len);
}

/* processes the status of the response, i.e. returns 1 if the server   */
/* response says OK, 0 if server response says failure or -1 if it      */
/* is invalid                                                           */
int pop_resp_stat(char *buf)
{
char c;
int result = -1;

    c = buf[3];
    buf[3] = '\0';

    /* if +OK   */
    if (tc_strcmp(buf, pop_str5) == 0)
        result = 1;

    /* if -ERR   */
    if (tc_strcmp(buf, pop_str6) == 0)
        result = 0;

    buf[3] = c;

#if (DEBUG_POP)
    if (result == -1)
    {
        DEBUG_ERROR("Invalid POP server response.", NOVAR, 0, 0);
    }
    if (result == 0)
    {
        DEBUG_ERROR("POP Server returned error.", NOVAR, 0, 0);
    }
#endif

    return(result);
}

/* send string to server over socket context->pop_sock   */
int pop_send(PPOP_CONTEXT context, KS_CONSTANT char *string)
{
#if (DEBUG_POP_VERBOSE)
char outbuf[80];
#endif

#if (DEBUG_POP_VERBOSE)
    tc_strcpy(outbuf,pop_str7);
    DEBUG_ERROR(outbuf, STR1, string, 0);
#endif

    return(send(context->pop_sock, (PFCCHAR)string, tc_strlen(string), 0));
}

/* read the next line of input returning length read or   */
/* TERM_MSG if at end of message or TERM_HEADER if at end */
/* of mime header                                         */
int pop_get_whole_line(PPOP_CONTEXT context, PFCHAR *lnbuffer, long tmeout, int type)
{
int len;
PIO_CONTEXT pio_context;

    pio_context = (PIO_CONTEXT)(context->io_context);  

    len = xn_line_get(pio_context, lnbuffer, tmeout, type);

    /* if we encountered a term string (len = 0) then          */
    /* read ahead to see which term string we have encountered */
    if ( (len == 1) && (*lnbuffer[0] == '.') )
        return(TERM_MSG);
    else if (len == 0)
        return(TERM_HEADER);
    return(len);
} 

int pop_LIST(PPOP_CONTEXT context, PPOP_MSG_SIZE mail_sizes, int first_msg, int num_messages)
{
int len, result;
int msg_num;
PFCHAR buf_ptr;
PFCHAR buf_end;
int num;
PIO_CONTEXT pio_context;

    pio_context = (PIO_CONTEXT)(context->io_context);  

    pop_send(context,pop_str8);

    pio_context->pb_in[pio_context->begin_offset_in] = '\0';   /* reset the buffer since not interested in anything else in it */
    len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,
                             GET_LINE);
    if ((len == -1) || (len == -2) || (len == -3))
        return set_errno(ESRVDOWN);

    result = pop_resp_stat(context->gbuffer);
    if (result == -1)
        return set_errno(ESRVBADRESP);

    msg_num = 0;
    for (;;)
    {
        len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,GET_LINE);
        if (len == TERM_HEADER)
            continue;
        else if (len == TERM_MSG)
            break;
        else if (len <= 0)
            break;

        buf_ptr = context->gbuffer;
        buf_end = buf_ptr + len;

        /* parse response   */
        num = tc_atoi(buf_ptr);

        /* only log ones in range requested by user                    */
        /* NOTE: NOT EXACTLY RIGHT SINCE SOME SERVERS START WITH 0 AND */
        /*       SOME START WITH 1 (for msg number)                    */
        if ( (num < first_msg) || (num > (first_msg + num_messages)) )
            continue;

        mail_sizes[msg_num].num = num;

        skip_to_space(&buf_ptr, buf_end);
        buf_ptr++;

        mail_sizes[msg_num].size = tc_atol(buf_ptr);

        msg_num++;
    }

    /* fill in any unused entries just in case num_messages is to large   */
    for (; msg_num < num_messages; msg_num++)
    {
        mail_sizes[msg_num].num = -1;
        mail_sizes[msg_num].size = 0;
    }

    return(0);
}

int pop_login(PPOP_CONTEXT context, PFCHAR username, PFCHAR pass)
{
int len, result;

    tc_strcpy(context->gbuffer, pop_str9);
    tc_strcat(context->gbuffer, username);
    tc_strcat(context->gbuffer, MIME_TERM_FIELD);
    pop_send(context, context->gbuffer);

    len = pop_get_resp(context, context->gbuffer, CFG_POP_BUF_SIZE);
    if (len == -1)
        return set_errno(ESRVDOWN);
    result = pop_resp_stat(context->gbuffer);
    if (result == -1)
        return set_errno(ESRVBADRESP);

    tc_strcpy(context->gbuffer, pop_str10);
    tc_strcat(context->gbuffer, pass);
    tc_strcat(context->gbuffer, MIME_TERM_FIELD);
    pop_send(context, context->gbuffer);

    len = pop_get_resp(context, context->gbuffer, CFG_POP_BUF_SIZE);
    if (len == -1)
        return set_errno(ESRVDOWN);
    result = pop_resp_stat(context->gbuffer);
    if (result == -1)
        return set_errno(ESRVBADRESP);
    if (result == 0)
        return set_errno(ESRVBADRESP);      /* tbd - errno */
    return(0);
}

int pop_STAT(PPOP_CONTEXT context, PFINT pmessages, PFDWORD pmsgsize)
{
int n, m;
int len, result;
char c;

    pop_send(context,pop_str11);
    len = pop_get_resp(context, context->gbuffer, CFG_POP_BUF_SIZE);
    if (len == -1)
        return set_errno(ESRVDOWN);
    result = pop_resp_stat(context->gbuffer);
    if (result != 1)
        return set_errno(ESRVBADRESP);

    for (n = 5; n<len; n++)
    {
                if (context->gbuffer[n] == ' ')
        {
            c = context->gbuffer[n];
            context->gbuffer[n] = '\0';
            (*pmessages) = tc_atoi(&(context->gbuffer[4]));
            context->gbuffer[n] = c;
            break;
        }
    }

    for (m = n+1; m<len; m++)
    {
        if (context->gbuffer[m] == ' ' || context->gbuffer[m] == ASCII_CR)
        {
            c = context->gbuffer[m];
            context->gbuffer[m] = '\0';
            (*pmsgsize) = tc_atol(&(context->gbuffer[n+1]));
            context->gbuffer[m] = c;
            break;
        }
    }

    return(0);
}

/* connect to remote server              */
/* Returns 0 upon success. -1 upon error */
int pop_connect(PPOP_CONTEXT context, PFBYTE pop_server_ip)
{
int len, result;
struct sockaddr_in sin;
int option;
PIO_CONTEXT pio_context;

    pio_context = (PIO_CONTEXT)(context->io_context);  
   
    if ( (context->pop_sock = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
#if (DEBUG_POP)
        DEBUG_ERROR("pop_connect: can't allocate socket", NOVAR, 0, 0);
#endif
        return(-1);
    }
   
   option = 1;
    if ( setsockopt(context->pop_sock, SOL_SOCKET, SO_REUSESOCK, 
                    (PFCCHAR)&option, sizeof(int)) ) 
    {
        DEBUG_ERROR(" could not set socket option",
                     NOVAR, 0, 0);
        closesocket(context->pop_sock);
        return(-1);     /* internal error if fails */
    }
   
    pio_context->sock = context->pop_sock;
    sin.sin_family = AF_INET;
    tc_movebytes((PFBYTE)&sin.sin_addr, pop_server_ip, 4);
    sin.sin_port = htons(POP_PORT_NO);
    if (connect(context->pop_sock, (PSOCKADDR)&sin, sizeof(sin)) < 0)

    {
#if (DEBUG_POP)
        DEBUG_ERROR("pop_connect: connect failed.", NOVAR, 0, 0);
#endif
        closesocket(context->pop_sock);
 
        return(-1);
    }
   
    if (!set_non_blocking_mode(context->pop_sock))
    {
        DEBUG_ERROR("pop_connect: set_non_blocking_mode failed", NOVAR, 0, 0);
    }

    /* get greeting   */
    len = pop_get_resp(context, context->gbuffer, CFG_POP_BUF_SIZE);
    if (len == -1)
        return set_errno(ESRVDOWN);     
    result = pop_resp_stat(context->gbuffer);
    if (result != 1)
        return set_errno(ESRVBADRESP);
    return(0);
}

RTIP_BOOLEAN pop_get_field(KS_CONSTANT char *name, char *msg, int len, char *dest, int dest_len)
{
int n, l, m, i;
char c;

    dest_len--;
    l = tc_strlen(name);
    for (n = 0; n<len; n++)
    {
        c = msg[n+l];
        msg[n+l] = '\0';
        if (tc_strcmp(name, &(msg[n])) == 0)
        {
            msg[n+l] = c;

            /* now copy the information for the field to dest;   */
            /* copy up to \r or len                              */
            i = 0;
            for (m = n+l; (msg[m] != ASCII_CR) && (m<len); m++)
            {
                if (i >= dest_len)
                    break;
                dest[i] = msg[m];
                i++;
            }
            dest[i] = '\0';
            return(TRUE);
        }
        msg[n+l] = c;
    }
    return(FALSE);
}

int pop_TOP(PPOP_CONTEXT context, PPOP_MSG_INFO info, int msg_num)
{
char str[30];
int len, result;
RTIP_BOOLEAN found_from, found_subject;
PIO_CONTEXT pio_context;

    pio_context = (PIO_CONTEXT)(context->io_context);  

    tc_strcpy(context->gbuffer,pop_str12);
    tc_itoa(msg_num, str, 10);
    tc_strcat(context->gbuffer, str);
    tc_strcat(context->gbuffer,pop_str13);
    pop_send(context, context->gbuffer);

    /* reset so next call will get more data      */
    /* done with what is left in context->gbuffer */
    pio_context->pb_in[pio_context->begin_offset_in] = '\0';   

    len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,GET_LINE);
    if ((len == -1) || (len == -2) || (len == -3))
        return set_errno(ESRVDOWN);

    result = pop_resp_stat(context->gbuffer);
    if (result == -1)
        return set_errno(ESRVBADRESP);

    found_from = found_subject = FALSE;
    for (;;)
    {
        len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,GET_LINE);
        if (len == TERM_HEADER)
            continue;
        else if (len == TERM_MSG)
            break;
        else if (len <= 0)
        {
            return set_errno(ESRVDOWN);
        }

        /* process header:   */
        if (!found_from)
            found_from = pop_get_field(pop_str14, context->gbuffer, len, 
                                       info->from, CFG_MAX_FROM_LEN);
        if (!found_subject)
            found_subject = pop_get_field(pop_str15, context->gbuffer, 
                                          len, 
                                          info->subject, CFG_MAX_SUBJECT_LEN);

    }

    return(0);
}


/* ********************************************************************   */
/* MIME STUFF                                                             */
/* ********************************************************************   */

/* looks thru mime_headers array to convert the string to an integer   */
/* representing the mime_type; if string is not in the array, -1 is    */
/* returned                                                            */
int mime_get_type(PFCHAR buf_ptr)
{
int i;
int len;

    for (len=0; len<1000; len++)    /* prevent endless loop in case of error */
    {
        if (*(buf_ptr+len) == ASCII_COLON)
            break;
    }

    /* look at header field name to see if it is one we are interested in   */
    for (i=0; i < (sizeof(mime_headers)/sizeof(struct _mime_header)); i++)
    {
        /* header field names are case-insensitive (section 3.4.7 of RFC 822)   */
        if (!tc_strnicmp(buf_ptr, mime_headers[i].mime_header, len))
            return(i);
    }
    return(-1);
}

/* ********************************************************************   */
RTIP_BOOLEAN mime_extract_subfield(PPOP_CONTEXT context, PFCCHAR sub_hdr, 
                              PFCHAR dest, int dest_len)
{
PFCHAR bs, bs2;

    /* issolate the body of the subfield   */
    bs = tc_stristr(context->gbuffer, sub_hdr);
    if (bs)
    {
        /* skip field name and equal sign   */
        bs += tc_strlen(sub_hdr) + 1;

        /* for quoted string, the end is quote   */
        if (*bs == ASCII_QUOTE) 
        {
            bs++;       /* skip the quote */
            bs2 = bs;
            while ( (*bs2 != ASCII_QUOTE) ||
                    (*(bs2-1) == ASCII_BACKSLASH) )
            {
                if (tc_strncmp(bs2, MIME_TERM_FIELD, 2) == 0)
                    break;
                bs2++;
            }
        }

        /* for non-quoted strings, the end is ; or   */
        /* the end of the field                      */
        else
        {
            bs2 = bs;
            while ( (*bs2 != ASCII_SEMI_COLON) &&
                    (tc_strncmp(bs2, MIME_TERM_FIELD,
                                2) != 0) )
            {
                bs2++;
            }
        }
        *bs2 = ASCII_EOS;
        tc_strncpy(dest, bs, dest_len);
        return(TRUE);
    }
    return(FALSE);
}

/* ********************************************************************   */
/* read the boundary                                                      */
int mime_read_boundary(PPOP_CONTEXT context, RTIP_BOOLEAN *pend_boundary)
{
int len;

    /* read the boundary; msg mime header should be next;           */
    /* read to the large buffer first just to make sure all of the  */
    /* boundary is read; then copy as much as possible to the saved */
    /* boundary string which will be used to determine the end of   */
    /* the mail message                                             */
    do
    {
        len = pop_get_whole_line(context, &context->gbuffer, CFG_POP_TIMEOUT,GET_MAIL);

#if (DEBUG_POP)
        DEBUG_ERROR("mime_read_boundary, read line:",NOVAR,0,0);
/*      DEBUG_ERROR(context->pop_buffer,NOVAR,0,0);   */
#endif
        
    } while (len >= 0 && !mime_check_for_boundary(context, pend_boundary));

    return(0);
}

/* ********************************************************************   */
/* check if the boundary was just read                                    */
RTIP_BOOLEAN mime_check_for_boundary(PPOP_CONTEXT context, RTIP_BOOLEAN *end_boundary)
{
int len;

/*  DEBUG_ERROR("pop_boundary=",STR1,context->pop_boundary,0);   */
/*  DEBUG_ERROR("boundary len=",DINT1,(dword)len,0);             */

    if (end_boundary)
        *end_boundary = FALSE;
    len = tc_strlen(context->pop_boundary);

/*  blah=context->pop_buffer[len+2];                       */
/*  context->pop_buffer[len+2]='\0';                       */
/*  DEBUG_ERROR("pop_buffer=",STR1,context->pop_buffer,0); */
/*  context->pop_buffer[len+2]=blah;                       */
    if ( (context->gbuffer[0] != ASCII_DASH) ||
             (context->gbuffer[1] != ASCII_DASH) ||
             (tc_strncmp(context->pop_boundary, &(context->gbuffer[2]),len) != 0) )
    {
/*      DEBUG_ERROR("Not a boundary.",NOVAR,0,0);    */

        /* nope, not a boundary    */
        return(FALSE);
    }
/*  DEBUG_ERROR("It's a boundary.",NOVAR,0,0);   */

    /* end of boundary should be -- or \0   */
        if ((context->gbuffer[len+2] == ASCII_DASH &&
             context->gbuffer[len+3] == ASCII_DASH ) ||
             context->gbuffer[len+2] == '\0' )
    {
        /* dashes after boundary indicates the end   */
        if (end_boundary)
        {
                        if ( (context->gbuffer[len+2] == ASCII_DASH &&
                              context->gbuffer[len+3] == ASCII_DASH) )
                *end_boundary = TRUE;
        }

        return(TRUE);
    }

    return(FALSE);
}

/* ********************************************************************   */
/* mime_parse_one_field() - parse a MIME field                            */
/*                                                                        */
/*   Parses the mime field in context->pop_buffer and stores the          */
/*   parsed info in parse_info.  If the mime type is known then           */
/*   the mime_type entry in parse_info is set to the mime type and        */
/*   the mime_field_ptr is set to the starting address of the field       */
/*   information (i.e. the character after the colon and any spaces).     */
/*   If the mime type is unknown, the mime_type is set to -1 and          */
/*   mime_type_ptr is set to the start of the mime field.                 */
/*                                                                        */
/*   Returns 0 if the mime field type is known and -1 if the              */
/*   mime type is unknown.                                                */
/*                                                                        */

int mime_parse_one_field(PFCHAR buf_ptr, int buf_size, 
                         PMIME_PARSE_INFO parse_info)
{
int    mime_type;
PFCHAR orig_buf_ptr;
int    diff;

    /* get type of MIME field; if it is not one we support (see mime_headers   */
    /* in popcons.c), it returns -1                                            */
    mime_type = mime_get_type(buf_ptr);
    if (mime_type >= 0)
    {
        orig_buf_ptr = buf_ptr;
        buf_ptr = tc_strchr(buf_ptr, ASCII_COLON);
        if (!buf_ptr)
            return(-1);
        (buf_ptr)++;        /* skip the colon */
        skip_space(&buf_ptr);
        parse_info->mime_type = mime_type;
        parse_info->mime_field_ptr = buf_ptr;
        diff = (int)ADDR_DIFF(buf_ptr, orig_buf_ptr);
        parse_info->mime_field_len = buf_size - diff;
        return(0);
    }
    parse_info->mime_type = -1;
    parse_info->mime_field_ptr = buf_ptr;
    parse_info->mime_field_len = buf_size;
    return(-1);     /* unknown field header */
}


void skip_space(PFCHAR *buf_ptr)
{
    if (**buf_ptr == ' ')
        *buf_ptr = *buf_ptr + 1;
}

void skip_to_space(PFCHAR *buf, PFCHAR buf_end)
{
    for (; *buf < buf_end; (*buf)++)
    {
        if (**buf == ' ')
        {
            break;
        }
    }
}


/* check if socket is ready for read/accept - wait   */
/* returns the following:                            */
/*      0 if neither is ready                        */
/*      1 if socket is ready                         */
int do_mime_read_select(int socket, long tmeout)
{
fd_set fd_read;
timeval to;

    FD_ZERO(&fd_read);
    FD_SET(socket, &fd_read);
    to.tv_sec = tmeout;
    to.tv_usec = 0;
    if (select(1, (PFDSET)&fd_read, (PFDSET)0, (PFDSET)0, (PCTIMEVAL)&to) < 1)
    {
/*      DEBUG_ERROR("read select return 0", NOVAR, 0, 0);   */
        return(0);
    }

    if (FD_ISSET(socket, &fd_read))
    {
        return(1);
    }

    DEBUG_ERROR("do_mime_read_select() - socket not set!", NOVAR, 0, 0);    
    return(0);
}
#endif /* INCLUDE_POP3 */


