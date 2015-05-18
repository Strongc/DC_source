/*                                                                      */
/* SNTP.C - SNTP functions                                              */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains SNTP (Simple Network Time Protocol)        */


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
#if (INCLUDE_SNTP)
#include "sntpapi.h"
#include "sntp.h"
#include "timeapi.h"

#define DELAY_TICKS 0   
#define ADD_OFFSET  0   /* Add offset to convert GMT to local time - */
                        /* NOTE: not working   */

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DISPLAY_TIMES        0
#define DEBUG_DELAY_TICKS    0
#define DISPLAY_RCV_TIME     0
#define DISPLAY_INT_TIMES    0
#define DISPLAY_DELAY_OFFSET 0

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define SEC_PER_HOUR 3600
/* 1000/(2**32) = 125/(2**29) = 125/0x20000000   */
#define FRAC_TO_MSEC_CONV(X) ((((X)>>7)*125)>>(29-7))

/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
static char *weekday_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"};

/* ********************************************************************   */
/* EXTERNAL GLOBAL DATA                                                   */
/* ********************************************************************   */
#if (INCLUDE_RUN_TIME_CONFIG)
extern CFG_SNTP_DATA KS_FAR cfg_sntp_data;
#endif

/* ********************************************************************   */
/* LOCAL FUNCTION DECLARATIONS                                            */
/* ********************************************************************   */
void convert_ebs_time_to_time(PEBSTIME ebs_time, PNTP_TIME_FORMAT sntp_time);
void convert_time_to_ebs_time(dword seconds, PEBSTIME ebs_time);
void time_sub(PNTP_TIME_FORMAT res, PNTP_TIME_FORMAT t1, PNTP_TIME_FORMAT t2);
void time_add(PNTP_TIME_FORMAT res, PNTP_TIME_FORMAT t1, PNTP_TIME_FORMAT t2);
void time_div_2(PNTP_TIME_FORMAT res);
#if (DISPLAY_INT_TIMES || DISPLAY_RCV_TIME)
void display_sntp_time(dword seconds);
void display_total_time(char *comment, dword total_seconds);
#endif

/* ********************************************************************   */
/* GLOBAL FUNCTION DECLARATIONS                                           */
/* ********************************************************************   */
int   ebs_date_to_dayofweek(int day, int month, int year);

/* ********************************************************************   */
/* API                                                                    */
/* UNICAST SUPPORT ONLY                                                   */
/* ********************************************************************.  */

/* ********************************************************************.   */
/* xn_sntp_get_time() - retrieve TOD from an SNTP/NTP server               */
/*                                                                         */
/* Summary:                                                                */
/*   #include "sntpapi.h"                                                  */
/*                                                                         */
/*   int xn_sntp_get_time(ip_sntp_server, version)                         */
/*       PFBYTE ip_sntp_server - IP addresses of SNTP/NTP server           */
/*       int version           - protocol version (SNTP_VERSION_3)         */
/*                                                                         */
/* Description:                                                            */
/*   Retrieves the current time of day from SNTP/NTP server specified      */
/*   in the parameter ip_sntp_server.                                      */
/*   The time of day is is returned in the parameter tme.                  */
/*   The global tod (ebs_tod) is not modified.  The routine                */
/*   xn_ebs_set_time() should be called to modify the global               */
/*   tod (ebs_tod).                                                        */
/*                                                                         */
/*   The routine xn_ebs_get_system_time() may be called to retrieve the    */
/*   time of day.                                                          */
/*   The routine xn_ebs_print_time() may be called to write the time of    */
/*   day into a string.                                                    */
/*                                                                         */
/* Returns 0 upon success and -1 upon error                                */
/*                                                                         */

int xn_sntp_get_time(PEBS_SYS_TIME tme, PFBYTE ip_sntp_server, int version)
{
SOCKET socket_no;
NTP_TIME_FORMAT current_timestamp= {0, 0};  /* local time */
dword start_ticks;
int rel_val;
EBSTIME res_ebs_tod = {0,0};
int day_of_week;

    /* send the request                                                */
    /* NOTE: most servers respond to NTP_SYM_ACTIVE but some only      */
    /*       respond to NTP_REQUEST; it looks like NTP servers respond */
    /*       to NTP_SYM_ACTIVE and SNTP respond to NTP_REQUEST         */
    if (sntp_request_time(ip_sntp_server, &socket_no, &current_timestamp, 
                          version, NTP_SYM_ACTIVE, &start_ticks,
                          (PEBSTIME)&res_ebs_tod) < 0)

    {
            return(-1);
    }

    /* wait for response   */
    if (!do_read_select(socket_no, CFG_SNTP_RCV_TMO))
    {
        DEBUG_ERROR("xn_sntp_get_time: (NTP_SYM_ACTIVE)timed out waiting for response", NOVAR, 0, 0);
        closesocket(socket_no);
        if (sntp_request_time(ip_sntp_server, &socket_no, &current_timestamp, 
                              version, NTP_REQUEST, &start_ticks,
                              (PEBSTIME)&res_ebs_tod) < 0)
        {
            return(-1);
        }

        if (!do_read_select(socket_no, CFG_SNTP_RCV_TMO))
        {
            DEBUG_ERROR("xn_sntp_get_time: (NTP_REQUEST) timed out waiting for response", NOVAR, 0, 0);
            closesocket(socket_no);
            return(set_errno(ETIMEDOUT));
        }
    }

    /* process the response   */
    rel_val = sntp_process_result(socket_no, &current_timestamp, version,
                                  start_ticks, (PEBSTIME)&res_ebs_tod);
    if (rel_val == -1)
        return(-1);

#if (DISPLAY_TIMES)
    DEBUG_ERROR("xn_sntp: TIME: seconds, frac_sec ",
        DINT2, res_ebs_tod.year, res_ebs_tod.second);
#endif

    convert_ebs_to_time((PEBSTIME)&res_ebs_tod, 
                        &tme->year, &tme->month, &tme->day,
                        &tme->hour, &tme->minute, &tme->second);
    tme->month++;
    tme->msec = FRAC_TO_MSEC_CONV(current_timestamp.frac_sec);

    /* set day of the week name   */
    day_of_week = ebs_date_to_dayofweek(tme->day, tme->month, tme->year);
    tc_strcpy(tme->day_of_week, weekday_names[day_of_week % 7]);

#if (DISPLAY_TIMES)
    DEBUG_ERROR("xn_sntp: year, month: ",
                        DINT2, tme->year, tme->month);
    DEBUG_ERROR("xn_sntp: day, hour: ",
                        DINT2, tme->day, tme->hour);
    DEBUG_ERROR("xn_sntp: minute, second: ",
                        DINT2, tme->minute, tme->second);
#endif
    return(0);
}

/* ********************************************************************   */
/* NOTE: closes socket upon failure                                       */
/* returns 0 upon success; -1 upon error                                  */
int sntp_request_time(PFBYTE ip_sntp_server, int *socket_no, 
                  PNTP_TIME_FORMAT current_timestamp,
                  int version, int mode, dword *ticks_when_sent,
                  PEBSTIME res_ebs_tod)
{
NTP_PKT ntp_request;
struct sockaddr_in sin;
int len;

    /* Allocate a socket   */
    if ( (*socket_no = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        DEBUG_ERROR("sntp_request_time: socket failed", EBS_INT1, 
            xn_getlasterror(), 0);
        return(-1);
    }

    /* Bind my ip address port NTP port   */
    sin.sin_family = AF_INET;
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
    sin.sin_addr = INADDR_ANY;
#else
    sin.sin_addr.s_addr = INADDR_ANY;
#endif
    sin.sin_port = hs2net(NTP_PORT);
    if (bind(*socket_no, (PSOCKADDR)&sin, sizeof(sin)) < 0)
    {
        DEBUG_ERROR("sntp_request_time: bind failed", EBS_INT1, 
            xn_getlasterror(), 0);
        closesocket(*socket_no);
        return(-1);
    }

    /* Connect to remote NTP server   */
    tc_mv4((PFBYTE)&sin.sin_addr, ip_sntp_server, IP_ALEN);
    if (connect(*socket_no, (PSOCKADDR)&sin, sizeof(sin)) < 0)
    {
        DEBUG_ERROR("sntp_request_time: connect failed", EBS_INT1, 
            xn_getlasterror(), 0);
        closesocket(*socket_no);
        return(-1);
    }

    /* format request   */
    tc_memset((PFBYTE)&ntp_request, 0, sizeof(NTP_PKT));
    ntp_request.li_vn_mode = (byte)mode;
    ntp_request.li_vn_mode |= version;
    ntp_request.poll = 6;

    /* get current time of day from ostime.c   */
    convert_ebs_time_to_time(res_ebs_tod, current_timestamp);

    *ticks_when_sent = ks_get_ticks();
#if (DEBUG_DELAY_TICKS)
    DEBUG_ERROR("ticks_when_sent set to : ", DINT1, *ticks_when_sent, 0);
#endif

    /* send request   */
    if ((len=send(*socket_no, (PFCHAR)&ntp_request, sizeof(NTP_PKT), 0)) < 
                  sizeof(NTP_PKT))
    {
        if (len < 0)
        {
            DEBUG_ERROR("sntp_request_time: send failed", EBS_INT1, 
                xn_getlasterror(), 0);
            closesocket(*socket_no);
            return(-1);
        }
        else
        {
            DEBUG_ERROR("sntp_request_time: sent less bytes: exp, act:", EBS_INT2, 
                sizeof(NTP_PKT), len);
        }
    }
    return(0);
}


/* ********************************************************************   */
int sntp_process_result(int socket_no, PNTP_TIME_FORMAT current_timestamp,
                        int version, dword ticks_when_sent,
                        PEBSTIME res_ebs_tod)

{
int             n_received;
NTP_PKT         ntp_response;
NTP_TIME_FORMAT temp_time1;
NTP_TIME_FORMAT temp_time2;
NTP_TIME_FORMAT t1;
NTP_TIME_FORMAT t2;
NTP_TIME_FORMAT t3;
NTP_TIME_FORMAT t4;
NTP_TIME_FORMAT local_offset;
NTP_TIME_FORMAT delay_time;         /* round trip delay */
#if (DELAY_TICKS)
dword           delay_ticks;
dword           curr_ticks;
#endif
RTIP_BOOLEAN    gmt_neg;

#if (DELAY_TICKS)
    curr_ticks = ks_get_ticks();
    if (CHECK_GREATER(curr_ticks, ticks_when_sent))
        delay_ticks = curr_ticks - ticks_when_sent;
    else
        delay_ticks = (int)( (long)((long)curr_ticks - 
                             (long)ticks_when_sent) );

#if (DEBUG_DELAY_TICKS)
    DEBUG_ERROR("ks_get_ticks, ticks_when_sent: ", DINT2,
        curr_ticks, ticks_when_sent);
#endif
#endif

    /* Get a packet   */
    n_received = recv(socket_no, (PFCHAR)&ntp_response, 
                      sizeof(ntp_response), 0);
    if (n_received < 0)
    {
        DEBUG_ERROR("sntp_process_result: recv failed", EBS_INT1, 
            xn_getlasterror(), 0);
    }

    closesocket(socket_no);

    /* verify response - mode   */
    if ( ((ntp_response.li_vn_mode & NTP_MODE_MASK) != NTP_SYM_PASSIVE) &&
         ((ntp_response.li_vn_mode & NTP_MODE_MASK) != NTP_RESPONSE) )
    {
        DEBUG_ERROR("sntp_process_result: response invalid: exp, act", 
            EBS_INT2, 
            NTP_SYM_PASSIVE, ntp_response.li_vn_mode & NTP_MODE_MASK);
        return(-1);
    }

    /* verify response - stratum   */
    if ( (ntp_response.stratum < 1) || (ntp_response.stratum > 14) )
    {
        DEBUG_ERROR("sntp_process_result: stratum invalid: ", EBS_INT1,
            ntp_response.stratum, 0);
        return(-1);
    }

    /* verify response - version   */
    if ( (ntp_response.li_vn_mode & NTP_VERSION_MASK) != version )
    {
        DEBUG_ERROR("sntp_process_result: response invalid - version : exp, act", 
            EBS_INT2, version, 
            ntp_response.li_vn_mode & NTP_VERSION_MASK);
        return(-1);
    }

    ntp_response.rcv_timestamp.seconds = 
        net2hl(ntp_response.rcv_timestamp.seconds);

    ntp_response.ref_timestamp.seconds = 
        net2hl(ntp_response.ref_timestamp.seconds);

    ntp_response.transmit_timestamp.seconds = 
        net2hl(ntp_response.transmit_timestamp.seconds);

    /* seconds since Jan 1, 1970                  */
/*  DEBUG_ERROR("TIME SECONDS: seconds: ", DINT2, */
/*      ntp_response.rcv_timestamp.seconds,       */
/*      ntp_response.rcv_timestamp.frac_sec);     */

#if (DISPLAY_RCV_TIME)
    DEBUG_ERROR("\n***** RCV TIME *****", NOVAR, 0, 0);
    display_sntp_time(ntp_response.rcv_timestamp.seconds);
#endif

#if (DISPLAY_INT_TIMES)
/*  DEBUG_ERROR("***** REF TIME *****", NOVAR, 0, 0);      */
/*  display_sntp_time(ntp_response.ref_timestamp.seconds); */


/*  DEBUG_ERROR("***** TRANSMIT TIME *****", NOVAR, 0, 0);      */
/*  display_sntp_time(ntp_response.transmit_timestamp.seconds); */
#endif

#if (DELAY_TICKS)
    t4.seconds = delay_ticks / ks_ticks_p_sec();
    t4.frac_sec = 0;    /* tbd */
    DEBUG_ERROR("delay_ticks, t4.sec", DINT2, delay_ticks, t4.seconds);
#endif

    /* set the following                    */
    /* t2 = time request received by server */
    /* t3 = time reply sent by server       */
    /* t4 = time reply received by client   */
    STRUCT_COPY(t1, *current_timestamp); 
    STRUCT_COPY(t2, ntp_response.rcv_timestamp);
    STRUCT_COPY(t3, ntp_response.transmit_timestamp);
#if (!DELAY_TICKS)
    STRUCT_COPY(t4, *current_timestamp);
#endif

    /* set time response took to arrive from time sent   */
    /* delay_time = (t4-t1) - (t2-t3)                    */
    time_sub(&temp_time1, &t4, &t1);
    time_sub(&temp_time2, &t2, &t3);
    time_sub(&delay_time, &temp_time1, &temp_time2);

    /* calculate local time offset   */
    /* local clock offset            */
    time_sub(&temp_time1, &t2, &t1);
    time_sub(&temp_time2, &t3, &t4);
    time_add(&local_offset, &temp_time1, &temp_time2);
    time_div_2(&local_offset);

#if (DISPLAY_DELAY_OFFSET)
    display_total_time("\nDELAY TIME: ", delay_time.seconds);
    display_total_time("OFFSET TIME: ",  local_offset.seconds);
#endif

    gmt_neg = FALSE;
#if (!ADD_OFFSET)
    if (CFG_GMT_DIFF < 0)
    {
        gmt_neg = TRUE;
        /* convert hours to seconds, hence multiply by 3600;   */
        /* make it positive and it will be subtracted below    */
        local_offset.seconds = -(CFG_GMT_DIFF * SEC_PER_HOUR);
    }
    else
    {
        /* CFG_GMT_DIFF is positive so seconds is always positive   */
        local_offset.seconds = (dword)(CFG_GMT_DIFF * SEC_PER_HOUR);
    }
#endif
    local_offset.frac_sec = 0;

    /* update time with delay   */
    time_add(&temp_time1, &t3, &delay_time);

    /* save current time for next request   */
    STRUCT_COPY(*current_timestamp, temp_time1); 

    /* update time with time zone   */
    if (gmt_neg)
        time_sub(&temp_time2, &temp_time1, &local_offset);
    else
        time_add(&temp_time2, &temp_time1, &local_offset);


/*  display_total_time("BEFORE CONVERSION TO ebs_tod ", temp_time2.seconds);   */
/*  display_total_time("FINAL TIME ", temp_time2.seconds);                     */

    /* convert time returned by SNTP server to RTIP internal format   */
    convert_time_to_ebs_time(temp_time2.seconds, res_ebs_tod);

    return(0);
}

/* ********************************************************************   */
/* DISPLAY ROUTINES                                                       */
/* ********************************************************************.  */
#if (DISPLAY_INT_TIMES || DISPLAY_RCV_TIME)
void display_sntp_time(dword seconds)
{
EBSTIME ebs_time;
char time_str[100];

    /* convert time returned by SNTP server to RTIP internal format   */
    convert_time_to_ebs_time(seconds, &ebs_time);

    xn_ebs_print_time(time_str, &ebs_time, 1);
    DEBUG_ERROR("TIME from xn_sntp_get_time: ", STR1, time_str, 0);
}
#endif

#if (DISPLAY_DELAY_OFFSET)
void display_total_time(char *comment, dword total_seconds)
{
int seconds;
int minutes;
int hours;

    seconds = (int)(total_seconds % 60);
    total_seconds /= 60;

    minutes = (int)(total_seconds % 60);
    total_seconds /= 60;

    hours = (int)(total_seconds % 24);
    total_seconds /= 24;

    if (hours == 0)
    {
        DEBUG_ERROR(comment, EBS_INT2, minutes, seconds);
    }
    else
    {
        DEBUG_ERROR(comment, EBS_INT1, hours, minutes);
        DEBUG_ERROR("hour, min, sec: ", EBS_INT1, seconds, 0);
    }
}
#endif


/* ********************************************************************   */
/* UTILITIES                                                              */
/* ********************************************************************   */
void convert_ebs_time_to_time(PEBSTIME ebs_time, PNTP_TIME_FORMAT sntp_time)
{
dword seconds;
int   current_year;
long  current_year_in_secs;

    seconds = ebs_time->second;
    current_year = ebs_time->year;

    while (current_year >= 1900)
    {
        current_year_in_secs = ebs_get_seconds_in_year(ebs_time->year);

/*      DEBUG_ERROR("year_in_secs, year = ", DINT2,    */
/*          current_year_in_secs, current_year->year); */

        seconds += current_year_in_secs;
        current_year--;
    }
    sntp_time->seconds  = seconds;
    sntp_time->frac_sec = 0;

    
}

/* time (seconds) is number of seconds since Jan 1, 1900 0:0:0   */
void convert_time_to_ebs_time(dword seconds, PEBSTIME ebs_time)
{
long current_year_in_secs;

    ebs_time->year   = 1900;

    while (seconds > 0)
    {
        current_year_in_secs = ebs_get_seconds_in_year(ebs_time->year);

/*      DEBUG_ERROR("year_in_secs, year = ", DINT2,   */
/*          current_year_in_secs, ebs_time->year);    */
        if ((dword)current_year_in_secs <= seconds)
        {
            ebs_time->year += 1;
            seconds -= (dword)current_year_in_secs;
        }
        else
        {
/*          DEBUG_ERROR("year, seconds on last year ", DINT2,    */
/*              ebs_time->year, seconds);                        */
            break;
        }
    }
    ebs_time->second = seconds;
}

/* ********************************************************************   */
/* TIME ADD, SUBRACT, SHIFT ROUTINES                                      */
/* ********************************************************************.  */
/* res = t1 - t2                                                          */
/* t1 must be >= t2                                                       */
void time_sub(PNTP_TIME_FORMAT res, PNTP_TIME_FORMAT t1, PNTP_TIME_FORMAT t2)
{
    res->seconds = t1->seconds - t2->seconds;

    res->frac_sec = t1->frac_sec - t2->frac_sec;
    if (t1->frac_sec < t2->frac_sec)
        res->seconds--;
}

void time_add(PNTP_TIME_FORMAT res, PNTP_TIME_FORMAT t1, PNTP_TIME_FORMAT t2)
{
    res->seconds = t1->seconds + t2->seconds;
    res->frac_sec = t1->frac_sec + t2->frac_sec;
    if ( (res->frac_sec < t1->frac_sec) || (res->frac_sec < t2->frac_sec) )
    {
        res->seconds++;
    }
}

void time_div_2(PNTP_TIME_FORMAT res)
{
    res->frac_sec >>= 1;
    if (res->seconds & 0x00000001ul)
        res->frac_sec |= res->frac_sec | 0x80000000ul;
        
    res->seconds >>= 1;
}

#endif /* INCLUDE_SNTP */

