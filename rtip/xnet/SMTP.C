/* SMTP.C - Simple Mail Transfer Protocol                                  */
/*                                                                         */
/* EBS - RTIP                                                              */
/*                                                                         */
/* Copyright Peter Van Oudenaren , 1993                                    */
/* All rights reserved.                                                    */
/* This code may not be redistributed in source or linkable object form    */
/* without the consent of its author.                                      */
/*                                                                         */

#define DIAG_SECTION_KERNEL DIAG_SECTION_MAIL


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_SMTP)
#include "smtp.h"
#include "vfile.h"  /* include for the system file include files */
#include "base64.h"
#include "netutil.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define SMTP_VERBOSE    0   /* prints out text sent and text received */
#define SMTP_DEBUG      0   /* prints out error messages */
#define SMTP_DEBUG_AUTH 0

/* ********************************************************************   */
/* configuration defines -- modify to control client behavior             */
/* ********************************************************************   */
#define SMTP_RAW        1   /* the user must format To: and From addresses */
#define SMTP_MAFD       5   /* num of times to check for message accepted for delivery __st__ */

/* ********************************************************************     */
#if (INCLUDE_ESMTP)
int  process_auth(PSMTP_INFO info, int sock, PFCHAR buf, int len);
#endif
int  smtp_send(int sock, KS_CONSTANT char *string);
int  smtp_send_binary(int sock,char *buf,int len);
int  smtp_send_file(int sock, PFCHAR filename,int type);
int  smtp_get_resp(int sock, char *buf, int bufsize, RTIP_BOOLEAN check_code);
int _smtp_get_resp(int sock, char *buf, int bufsize, RTIP_BOOLEAN check_code, 
                   int timeout);
int  smtp_HELLO(int sock, char *whoami);
int  smtp_EHELLO(int sock, char *whoami);
int  smtp_MAIL(int sock, char *reverse_path);
int  smtp_RCPT(int sock, char *forward_path);
int  smtp_QUIT(int sock);
int  smtp_send_base64(int sock, char *buffer, int len);

/* ********************************************************************      */
/* EXTERNAL GLOBAL DATA                                                      */
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str1a1;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str1a2;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str1b;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str1c;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str2;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str2a;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str3;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_boundary;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_helo;
#if (INCLUDE_ESMTP)
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_ehlo;
#if (INCLUDE_ESMTP)
extern char KS_FAR *smtp_auth;
extern char KS_FAR *smtp_resp_334;
extern char KS_FAR smtp_space_char;
extern char KS_FAR *smtp_login_plain;
extern char KS_FAR *smtp_auth_login_reply;
#endif
#endif
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str6;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_FROM;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str8;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str9;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str10;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_str11; 
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_7bit;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *smtp_base64;


/* ********************************************************************      */
/* SMTP API                                                                  */
/* ********************************************************************      */

/* ********************************************************************         */
/* xn_smtp() - send mail message through SMTP                                   */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "pop.h"                                                           */
/*                                                                              */
/*   int xn_smtp(PSMTP_INFO info)                                               */
/*                                                                              */
/*     PSMTP_INFO info  - structure containing mail info                        */
/*                                                                              */
/* Description:                                                                 */
/*                                                                              */
/*   This function sends an email message using the SMTP protocol.  The         */
/*   string info->from will appear in the From: section of the message.         */
/*   Recipients are not listed in info->rcpt.                                   */
/*                                                                              */
/*   For more details see Mail Reference's Manual.                              */
/*                                                                              */
/* Returns:                                                                     */
/*   Zero is returned on success. Otherwise -1                                  */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set     */
/*   by this function.  For error values returned by this function see          */
/*   RTIP Reference's Manual.                                                   */
/*                                                                              */

int xn_smtp(PSMTP_INFO info)
{
struct sockaddr_in sin;
int    smtp_sock;
PFCHAR buf;
int    result;
int    n;
PFCHAR outgoing1;
PFCHAR outgoing2;
PFCHAR outgoing3;
int    type;
PFCHAR file_start;
PFCHAR local_attach_file;
int    out1len;
int    ret_val;
int    option;
char   command[6];
int    cmd = 0;
int    i   = 0;
EBSTIME ptime;
RTIP_BOOLEAN time_set;

    outgoing1 = outgoing2 = outgoing3 = buf = 0;

    /* **************************************************      */
    /* connect to SMTP server                                  */
    if ( (smtp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        DEBUG_ERROR("xn_smtp: can't allocate socket", NOVAR,0,0);
        return(-1);
    }

    option = 1;
    if ( setsockopt(smtp_sock, SOL_SOCKET, SO_REUSESOCK,
                    (PFCCHAR)&option, sizeof(int)) )
    {
        DEBUG_ERROR("could not set socket option", NOVAR, 0, 0);
        ret_val = -1;
        goto xn_smtp_done;
    }

    /* move set_non_blocking_mode before connect __st__ 2002.05.09   */
    if (!set_non_blocking_mode(smtp_sock))
    {
        DEBUG_ERROR("xn_smtp: set_non_blocking_mode failed", NOVAR, 0, 0);
    }

    sin.sin_family = AF_INET;
    tc_mv4((PFBYTE)&sin.sin_addr, info->smtp_server_ip, IP_ALEN);
    sin.sin_port = htons(SMTP_PORT_NO);
    if (connect(smtp_sock, (PSOCKADDR)&sin, sizeof(sin)) < 0)
    {
        /* socket is non-blocking and the WINSOCK says connect should start   */
        /* the handshaking but return immediatly with EWOULDBLOCK;            */
        /* BSD says EINPROGRESS                                     __st__    */
        if ( (xn_getlasterror() != EWOULDBLOCK) &&
             (xn_getlasterror() != EINPROGRESS) )
        {
#            if (SMTP_DEBUG)
            DEBUG_ERROR("xn_smtp: connect failed.",NOVAR,0,0);
#            endif
            ret_val = -1;       /* michele */
            goto xn_smtp_done;
        }
    }

    /* do select now that connect is in non-blocking mode __st__ 2002.05.09   */
    if (!do_write_select(smtp_sock, CFG_SMTP_TIMEOUT))
    {
#        if (SMTP_DEBUG)
        DEBUG_ERROR("xn_smtp: select failed.",NOVAR,0,0);
#        endif
        ret_val = -1;       /* michele */
        goto xn_smtp_done;
    }

    /* Allocate buffers     */
    outgoing1 = (PFCHAR) dcu_alloc_core(CFG_MAX_PACKETSIZE-4);
    outgoing2 = (PFCHAR) dcu_alloc_core(CFG_MAX_PACKETSIZE-4);
    buf = (PFCHAR) dcu_alloc_core(CFG_SMTP_BUF_SIZE);

    if (!(buf && outgoing1 && outgoing2))
    {
        set_errno(ENOPKTS);
        ret_val = -1;
        goto xn_smtp_done;
    }

    /* **************************************************      */
    /* SAY HELLO                                               */
    outgoing3 = outgoing2 + 600;

    outgoing2[0] = '\0';
    outgoing3[0] = '\0';


    result = _smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE, 
                            CFG_SMTP_INIT_TIMEOUT);
    if (result == -1)
    {
        DEBUG_ERROR("xn_sntp: after connect: no response from server", 
            NOVAR, 0, 0);
        goto xn_smtp_error;
    }

#if (INCLUDE_ESMTP)
    if (info->user_name[0] != 0)
    {
        if (smtp_EHELLO(smtp_sock, info->reverse_path) < 0)
            goto xn_smtp_error;
    }
    else
#endif
    {
        if (smtp_HELLO(smtp_sock, info->reverse_path) < 0)
        {
            DEBUG_ERROR("smtp_HELLO failed", NOVAR, 0, 0);
            goto xn_smtp_error;
        }
    }

    result = smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE);
    if (result == -1)
    {
        DEBUG_ERROR("smtp_HELLO response failed", NOVAR, 0, 0);
        goto xn_smtp_error;
    }

    /* **************************************************     */
    /* AUTHENTICATION PROCESSING                              */
#if (INCLUDE_ESMTP)
    if (tc_strstr(buf, smtp_auth))
        process_auth(info, smtp_sock, buf, result);
#endif

    /* **************************************************      */
    /* SEND MAIL FROM                                          */
    if (smtp_MAIL(smtp_sock, info->from) < 0)
    {
        DEBUG_ERROR("smtp_MAIL failed", NOVAR, 0, 0);
        goto xn_smtp_error;
    }

    result = smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE);
    if (result == -1)
    {
        DEBUG_ERROR("smtp_MAIL get response failed", NOVAR, 0, 0);
        goto xn_smtp_error;
    }

    /* **************************************************     */
    /* SEND RCPT TO to each recipient (TO: and CC:)           */
    for (n = 0; n < (info->num_rcpts+info->num_cc); n++)
    {
        if (smtp_RCPT(smtp_sock, info->rcpts[n].name) < 0)
        {
            DEBUG_ERROR("smtp_RCPT failed", NOVAR, 0, 0);
            goto xn_smtp_error;
        }

        /* get the 250 recipient ok message for the above RCPT TO: message     */
        result = smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE);
        if (result == -1)
        {
            DEBUG_ERROR("smtp_RCPT response failed", NOVAR, 0, 0);
            goto xn_smtp_error;
        }
    }

    /* **************************************************     */
    /* send "DATA\r\n"                                        */
    if (smtp_send(smtp_sock, smtp_str9) < 0)
    {
        DEBUG_ERROR("smtp send DATA failed", NOVAR, 0, 0);
        goto xn_smtp_error;
    }

    result = smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE);
    if (result == -1)
    {
        DEBUG_ERROR("smtp send DATA response failed", NOVAR, 0, 0);
        goto xn_smtp_error;
    }

    /* **************************************************               */
    /* prepare strings for sending mail message below, i.e.             */
    /* create MIME header                                               */
    /* NOTE: the rest of the TO: and CC: fields will be set up below    */
    time_set = FALSE;
    if (xn_ebs_get_system_time((PEBSTIME)&ptime) == 0)
    {
        if (xn_ebs_print_time(outgoing2, (PEBSTIME)&ptime, 2) == 0)
        {
            time_set = TRUE;
            tc_sprintf(outgoing1, smtp_str1a1,
                outgoing2,
                info->subject,
                info->from,             /* from */
                info->rcpts[0].name);   /* to */
        }
    }

    if (!time_set)
    {
        tc_sprintf(outgoing1, smtp_str1a2,
                info->subject,
                info->from,             /* from */
                info->rcpts[0].name);   /* to */
    }

    /* **************************************************               */
    /* first add TO: fields makeing sure not to overflow outgoing1      */
    out1len = tc_strlen(outgoing1);
    for (n = 1; n < info->num_rcpts; n++)
    {
        out1len = out1len + tc_strlen(info->rcpts[n].name) + 2;
        if (out1len >= CFG_MAX_PACKETSIZE)
        {
            if (smtp_send(smtp_sock, outgoing1) < 0)
                goto xn_smtp_error;
            outgoing1[0] = '\0';
            out1len = tc_strlen(info->rcpts[n].name);
        }

        tc_strcat(outgoing1, info->rcpts[n].name);
        tc_strcat(outgoing1, ", ");
    }

    /* **************************************************                  */
    /* replace the last ", " with \r\n or if no recipients, leave out      */
    /* the To: field                                                       */
    if (info->num_rcpts == 0)
        tc_strcpy(tc_strstr(outgoing1, "To:"), "\r\n");
    else
        tc_strcpy(outgoing1+tc_strlen(outgoing1)-2, "\r\n");

    if (info->num_cc > 0)
    {
        tc_strcat(outgoing1, "cc: ");
        out1len = tc_strlen(outgoing1);
        for (n = 0; n < info->num_cc; n++)
        {
            out1len = out1len + tc_strlen(info->rcpts[n].name) + 2;
            if (out1len >= CFG_MAX_PACKETSIZE)
            {
                if (smtp_send(smtp_sock, outgoing1) < 0)
                    goto xn_smtp_error;
                outgoing1[0] = '\0';
                out1len = tc_strlen(info->rcpts[n].name);
            }
            tc_strcat(outgoing1, info->rcpts[info->num_rcpts+n].name);
            tc_strcat(outgoing1, ", ");
        }
        /* replace the last ", " with \r\n     */
        tc_strcpy(outgoing1+tc_strlen(outgoing1)-2, "\r\n");
    }

    /* **************************************************     */
    /* send here to make sure don't overflow                  */
    if (smtp_send(smtp_sock, outgoing1) < 0)
        goto xn_smtp_error;

    /* **************************************************     */
    /* send any mime fields set up by user                    */
    if (info->mime_fields)
    {
        if (smtp_send(smtp_sock, info->mime_fields) < 0)
            goto xn_smtp_error;
    }

    /* **************************************************     */
    /* SEND BODY AND ATTACHMENTS                              */
    if (info->num_attach_files)    /* if there are any attachments to send */
    {
        tc_sprintf(outgoing1, smtp_str1b,    /* adds multipart /mixed */
                   &(smtp_boundary[2]),
                   smtp_boundary);

        tc_strcat(outgoing1, smtp_str1c);
        if (smtp_send(smtp_sock, outgoing1) < 0)
            goto xn_smtp_error;
    }
    else
        if (smtp_send(smtp_sock, smtp_str1c) < 0)
            goto xn_smtp_error;

    if (info->body && (info->body[0] != '\0'))   /* now send body */
        if (smtp_send(smtp_sock, info->body) < 0)
            goto xn_smtp_error;

    if (info->body_file && (info->body_file[0] != '\0'))
        if (smtp_send_file(smtp_sock, info->body_file, TEXT_TYPE) < 0)
            goto xn_smtp_error;

   if (info->num_attach_files)  /* if there are attachments */
   {
       for (n = 0; n < info->num_attach_files; n++) /* now send the attachments */
        {
            /* find the name of file without path name or drive     */
            file_start = info->at->attach_file;
            if (file_start[1] == ':')
               file_start += 2;
            local_attach_file = tc_strrchr(file_start, '\\');
            if (!local_attach_file)
                local_attach_file = tc_strrchr(file_start, '/');

            if (local_attach_file)
                local_attach_file++;        /* point past the \ or the / */
            else
                local_attach_file = file_start;

            /* determine the type of the attachment     */
            if (tc_stricmp(info->at->attach_type, "text/plain") == 0)
                type = TEXT_TYPE;
            else
                type = BINARY_TYPE;


            if (info->at->attach_file && (info->at->attach_file[0] != '\0') )
            {
                /* tbd: parse file name, i.e. d:\dir\filename should go       */
                /*      to filename (only when passing to this tc_sprintf)    */
                tc_sprintf(outgoing2, smtp_str2,
                    smtp_boundary, info->at->attach_type,
                    local_attach_file,
                    (type==TEXT_TYPE) ? smtp_7bit : smtp_base64,
                    local_attach_file);
            }
            else
            {
                tc_sprintf(outgoing2, smtp_str2a, smtp_boundary,
                       info->at->attach_type,
                       ((type==TEXT_TYPE) ? smtp_7bit:smtp_base64));
            }

            /* send the attachment      */
            if (smtp_send(smtp_sock, outgoing2) < 0)
                goto xn_smtp_error;
            if (info->attach && info->attach[0])
                if (type == BINARY_TYPE)
                {
                    if (smtp_send_base64(smtp_sock, info->attach,
                                        info->attach_len>0?info->attach_len:
                                        tc_strlen(info->attach)) < 0)
                    {
                        goto xn_smtp_error;
                    }
                }
                else
                    if (smtp_send(smtp_sock, info->attach) < 0)
                        goto xn_smtp_error;

            if (info->at->attach_file && (info->at->attach_file[0] != '\0'))
                if (smtp_send_file(smtp_sock, info->at->attach_file,type) < 0)
                    goto xn_smtp_error;

           info->at++;    /* point to the next file info structure of the array */

        }

        /* write CRLFboundary--                                          */
        /* NOTE: since this is the last boundary it has -- at the end    */
        tc_sprintf(outgoing3, smtp_str3, smtp_boundary);

        if (smtp_send(smtp_sock, outgoing3) < 0)
            goto xn_smtp_error;
    }

    /* **************************************************     */
    /* send END MSG, i.e. "\r\n.\r\n"                         */
    if (smtp_send(smtp_sock, smtp_str10) < 0)
        goto xn_smtp_error;

    command[4] = '\0';

    do
    {   /* should receive a 250 message accepted for delivery __st__ */
        result = smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE);
        tc_strncpy(command, buf, 3);
        if(command[0])
            cmd = tc_atoi((PFCHAR)command);
        else
            cmd = -1;

#if (SMTP_DEBUG)
    if(cmd != 250)
    {
        DEBUG_ERROR("smtp is waiting for a 250 message accepted for delivery: ", EBS_INT1, cmd, 0);
    }
#endif

        if (result == -1)
        {
#if (SMTP_DEBUG)
            DEBUG_ERROR("smtp_process_result(5) failed", NOVAR, 0, 0);
#endif
            goto xn_smtp_error;
        }
    }
    while ((cmd != 250) && (++i < SMTP_MAFD));


    ret_val = 0;
    goto xn_smtp_done;

xn_smtp_error:
    ret_val = -1;
#if (SMTP_DEBUG)
    DEBUG_ERROR("SMTP: Server returned error.",NOVAR,0,0);
#endif

xn_smtp_done:

    command[4] = '\0';
    if (smtp_QUIT(smtp_sock) < 0)
    {
        ret_val = -1;
        DEBUG_ERROR("SMTP: Quit not sent to server properly.",NOVAR,0,0);
    }
    else
    {
        /* should receive a 221/421 message __st__   */

        /*
         * FOLLOWING COMMENT IS TAKEN FROM RFC 821
         *
         * 221 <domain> Service closing transmission channel
         * 421 <domain> Service not available,
         *  closing transmission channel
         *  [this may be a reply to any command if the
         *       service knows it must shut down]
         */

        result = smtp_get_resp(smtp_sock, buf, CFG_SMTP_BUF_SIZE, TRUE);
        tc_strncpy(command, buf, 3);
        if(command[0])
            cmd = tc_atoi((PFCHAR)command);
        else
            cmd = -1;

#if (SMTP_VERBOSE)
        if( (cmd != 221) || (cmd != 421) )
        {
            DEBUG_ERROR("smtp is waiting for a 221/421 message: ", EBS_INT1, cmd, 0);
        }
#endif
        if(cmd != 421)  /* would be considered an error - not so in this case */
        {
            if (result == -1)
            {
#if (SMTP_DEBUG)
                DEBUG_ERROR("smtp_process_result(6) failed", NOVAR, 0, 0);
#endif
                ret_val = -1;
            }
        }
    }
    closesocket(smtp_sock);
    if (outgoing1) dcu_free_core((PFBYTE)outgoing1);
    if (outgoing2) dcu_free_core((PFBYTE)outgoing2);
    if (buf) dcu_free_core((PFBYTE)buf);
    return(ret_val);
}

/* ********************************************************************      */
/* SMTP UTILITIES                                                            */
/* ********************************************************************      */

int smtp_send(int sock, KS_CONSTANT char *string)
{
    return(smtp_send_binary(sock, (char *)string, tc_strlen(string)));
}

int smtp_send_binary(int sock, char *buf,int len)
{
fd_set fd_write;
timeval to;
unsigned long nleft_window;
int n_to_send;
int n_sent;
int n_left;

    if (len < 0)
        return(-1);

    n_left = len;
    while(n_left > 0)
    {
        /* Call select and wait for the window to open up     */
        FD_ZERO(&fd_write);
        FD_SET(sock, &fd_write);
        to.tv_sec = CFG_SMTP_TIMEOUT;
        to.tv_usec = 0;
        if (select(1, (PFDSET)0, (PFDSET)&fd_write, (PFDSET)0, (PCTIMEVAL)&to) < 1)
        {
            DEBUG_ERROR("smtp_send_failed: write select return 0", NOVAR, 0, 0);
            return(-1);
        }

        if (!FD_ISSET(sock, &fd_write))
        {
            DEBUG_ERROR("smtp_send: socket not ready", NOVAR, 0, 0);
            return(-1);
        }

        /* Get the amount of data in the output window     */
        if (ioctlsocket(sock, FIONWRITE,
                            (unsigned long KS_FAR *)&nleft_window) != 0)
        {
            DEBUG_ERROR("smtp_send: ioctlsocket (FIONWRITE) Failed", NOVAR, 0, 0);
            return(-1);
        }

        if ((unsigned long)n_left <= nleft_window)
        {
            n_to_send = n_left;
        }
        else
            n_to_send = (int)nleft_window;

        n_sent = send(sock, (PFCCHAR)buf, n_to_send, 0);
        if (n_sent <= 0)
        {
            DEBUG_ERROR("smtp_send: send failed", NOVAR, 0, 0);
            return(-1);
        }

        n_left -= n_sent;
        buf += n_sent;
    }
#if (SMTP_VERBOSE)
    buf[len] = '\0';
    DEBUG_ERROR("S: ", STR1, buf, 0);
#endif
    return(len);
}

int smtp_send_file(int sock, PFCHAR filename,int type)
{
int  smtp_fd;
int  len;
int  flen;
PFCHAR read_buf;
PFCHAR base64_buf;
base64_encode_context  context;
PBASE64_ENCODE_CONTEXT pContext = &context;
int ret_val;

    DEBUG_ERROR("Sending file . . .",NOVAR,0,0);


    if ((smtp_fd = vf_open(filename, VO_RDONLY, 0)) < 0)
    {
        DEBUG_ERROR("smtp_send_file: could not open file ",STR1,
            filename, 0);
        return(-1);
    }

    read_buf = (PFCHAR) dcu_alloc_core(VF_BLK_SIZE);
    base64_buf = (PFCHAR) dcu_alloc_core((int)BASE64_MAX_ENCODED_SIZE(VF_BLK_SIZE));
    if (!(read_buf && base64_buf))
    {
        DEBUG_ERROR("smtp_send_file: out of core",NOVAR,0, 0);
        ret_val = -1;
        goto send_done;
    }

    ret_val = 0;

    base64_init_encode_context(pContext, BASE64_LINE_LENGTH);
    do
    {
        flen = len = vf_read(smtp_fd, (PFBYTE)read_buf, (word)VF_BLK_SIZE);

        if (type == BINARY_TYPE) /* if its binary, encode it first */
        {
            if (!flen) /* we gotta finish the context */
            {
                len = base64_encode_finish(pContext, (PFBYTE)base64_buf);
            }
            else
            {
                len = base64_encode(pContext, (PFBYTE)read_buf,
                                     (PFBYTE)base64_buf, len);
            }

            DEBUG_ASSERT(len <= BASE64_MAX_ENCODED_SIZE(VF_BLK_SIZE),
                "FATAL(smtp_send_file): base64 buf too small. expected, got",
                DINT2,BASE64_MAX_ENCODED_SIZE(flen),len);

        }
        else if (!flen)
            continue;

        if (smtp_send_binary(sock, type==BINARY_TYPE ?
                             base64_buf : read_buf,len) < 0)
        {
            DEBUG_ERROR("smtp_send_file: send failed",NOVAR,0,0);
            ret_val = -1;
            break;
        }
    } while(flen);

send_done:
    if (read_buf)
        dcu_free_core((PFBYTE)read_buf);
    if (base64_buf)
        dcu_free_core((PFBYTE)base64_buf);
    vf_close(smtp_fd);
    return(ret_val);
}

/* reads response and null terminates response string which is save in buf   */
/* returns length of data read or -1 (and sets errno) if error               */
int _smtp_get_resp(int sock, char *buf, int bufsize, RTIP_BOOLEAN check_code, 
                   int timeout)
{
int len;
fd_set fd_read;
timeval to;

    FD_ZERO(&fd_read);
    FD_SET(sock, &fd_read);
    to.tv_sec = timeout;
    to.tv_usec = 0;
    if (select(1, (PFDSET)&fd_read, (PFDSET)0, (PFDSET)0, (PCTIMEVAL)&to) < 1)
    {
        DEBUG_ERROR("smtp_get_resp: read select return 0", NOVAR, 0, 0);
        return(-1);
    }

    if (!FD_ISSET(sock, &fd_read))
    {
        DEBUG_ERROR("smtp_get_resp: socket not ready", NOVAR, 0, 0);
        return(-1);
    }

    len = recv(sock, buf, bufsize, 0);
    if (len == -1)
    {
        DEBUG_ERROR("SMTP Server not responding.", NOVAR, 0, 0);
        return set_errno(ESRVDOWN);
    }

    if (len >= bufsize)
        buf[bufsize-1] = '\0';
    else
        buf[len] = '\0';

    /* process the result from the SMTP server   */
    if (check_code)
    {
        if ((buf[0] == '4' || buf[0] == '5')) /* error response */
        {
        int i;
            for (i=0; i<bufsize; i++)
            {
                if (buf[i] < ' ')
                    buf[i] = '\0';
                if (buf[i] == '\0')
                    break;
            }
            if (i == bufsize)
                buf[bufsize-1] = '\0';
            else
                buf[len] = '\0';

            DEBUG_ERROR(buf, NOVAR, 0, 0);
            return set_errno(ESRVBADRESP);
        }
    }
    return(len);
}

int smtp_get_resp(int sock, char *buf, int bufsize, RTIP_BOOLEAN check_code)
{
    return(_smtp_get_resp(sock, buf, bufsize, check_code, CFG_SMTP_TIMEOUT));
}

#if (INCLUDE_ESMTP)
int smtp_decode_334(int sock, PFCHAR buf, int len)
{
PFCHAR  buf2;
int     result;
int     decode_len;
base64_decode_context base64_context;
PBASE64_DECODE_CONTEXT p_base64_context;

    result = smtp_get_resp(sock, buf, len, FALSE);
    if (result == -1)
        return(-1); /* errno - tbd */

    p_base64_context = &base64_context;

    buf2 = buf;
    if (tc_strncmp(buf2, smtp_resp_334, tc_strlen(smtp_resp_334)) == 0)
    {
        buf2 += tc_strlen(smtp_resp_334);
        while (*buf2 == smtp_space_char)
            buf2++;
    }

    /* decode servers response into nontext_buffer and null-terminate   */
    /* it;                                                              */
    /* p_base64_context keeps track of where we are at in the decoding  */
    xn_base64_decode_init(p_base64_context);
    decode_len = xn_decode_base64(p_base64_context,(PFBYTE)buf2, 
        buf, len);
    return(decode_len);
}

int process_334(int sock, PFCHAR buf, int len, PFCHAR resp_to_334)
{
    if (smtp_decode_334(sock, buf, len) > 0)
    {
        /* encode and send server name   */
        if ( smtp_send_base64(sock, resp_to_334, 
                              tc_strlen(resp_to_334)) )
        {
            return(TRUE);
        }

    }
    return(FALSE);
}

int process_auth(PSMTP_INFO info, int sock, PFCHAR buf, int len)
{
PFCHAR  auth_str;
int     off;

    off = 0;

    while (off < len)
    {
        auth_str = tc_strstr(buf+off, smtp_auth);
#if (SMTP_DEBUG_AUTH)
        DEBUG_ERROR("150 AUTH= found: ", STR1, auth_str, 0);
#endif
        if (tc_strncmp(auth_str+tc_strlen(smtp_auth), smtp_login_plain, 
            tc_strlen(smtp_login_plain)) == 0)
        {
#if (SMTP_DEBUG_AUTH)
            DEBUG_ERROR("FOUND LOGIN_PLAIN", NOVAR, 0, 0);
#endif
            tc_strcpy(buf, smtp_auth_login_reply);
            if (smtp_send(sock, buf) < 0)
                return(-1); /* errno - tbd */

#if (SMTP_DEBUG_AUTH)
            DEBUG_ERROR("process first 334", NOVAR, 0, 0);
#endif
            if (process_334(sock, buf, len, info->user_name))
            {
#if (SMTP_DEBUG_AUTH)
                DEBUG_ERROR("process second 334", NOVAR, 0, 0);
#endif
                if (process_334(sock, buf, len, info->password))
                    return(0);
            }
            return(-1); /* errno */
        }
        auth_str = tc_strstr(buf+off, "\r\n");
        off = (int)ADDR_DIFF(auth_str, buf);
    }
    return(-1); /* errno - tbd */
}
#endif

/* **************************************************     */
#if (INCLUDE_ESMTP)
int smtp_EHELLO(int sock, char *whoami)
{
char buf[200];

    tc_strcpy(buf, smtp_ehlo);
    tc_strcat(buf, whoami);
    tc_strcat(buf,&(smtp_str6[1]));
    return(smtp_send(sock, buf));
}
#endif

/* **************************************************     */
int smtp_HELLO(int sock, char *whoami)
{
char buf[200];

    tc_strcpy(buf,smtp_helo);
    tc_strcat(buf, whoami);
    tc_strcat(buf,&(smtp_str6[1]));
    return(smtp_send(sock, buf));
}
 
/* **************************************************     */
int smtp_MAIL(int sock, char *reverse_path)
{
char buf[200];

#if SMTP_RAW
    tc_strcpy(buf, "MAIL FROM: ");
    tc_strcat(buf, reverse_path);
    tc_strcat(buf, "\r\n");
#else
    tc_strcpy(buf, smtp_FROM);
    tc_strcat(buf, reverse_path);
    tc_strcat(buf, smtp_str6);
#endif
    return(smtp_send(sock, buf));
}

int smtp_RCPT(int sock, char *forward_path)
{
char buf[200];

#if SMTP_RAW
    tc_strcpy(buf, "RCPT TO: ");
    tc_strcat(buf, forward_path);
    tc_strcat(buf, "\r\n");
#else
    tc_strcpy(buf,smtp_str8);
    tc_strcat(buf, forward_path);
    tc_strcat(buf,smtp_str6);
#endif
    return(smtp_send(sock, buf));
}

int smtp_QUIT(int sock)
{
    return(smtp_send(sock, smtp_str11));
}


int smtp_send_base64(int sock, char *buffer, int len)
{
#define SMTP_MSG_BASE64_OUTBUF_SIZE 45 /* space for nul-term and a few newlines */
#define SMTP_MSG_BASE64_INBUF_SIZE 30

base64_encode_context context;
byte base64_buff[SMTP_MSG_BASE64_OUTBUF_SIZE];
int elen;
int chunksize;

    base64_init_encode_context(&context, BASE64_LINE_LENGTH);

    while (len > 0)
    {
        /* chunksize is the size of the chunk of bytes we're going to encode     */
        chunksize = (len < SMTP_MSG_BASE64_INBUF_SIZE) ?
                    len : SMTP_MSG_BASE64_INBUF_SIZE;

        elen = base64_encode(&context, (PFBYTE)buffer, (PFBYTE)base64_buff, chunksize);

        DEBUG_ASSERT(elen<SMTP_MSG_BASE64_OUTBUF_SIZE,
            "smtp_send_base64(): elen too big, SMTP_MSG_BASE64_OUTBUF_SIZE needs to be bigger. elen =",EBS_INT1,elen,0);

        len -= chunksize;
        buffer += chunksize;

        if (smtp_send_binary(sock, (char *)base64_buff, elen) < 0)
            return(-1);
    }

    /* send anything remaining in the buffer.     */
    elen = base64_encode_finish(&context, base64_buff);
    return(smtp_send_binary(sock, (char *)base64_buff, elen));
}

#endif      /* INCLUDE_SMTP */
