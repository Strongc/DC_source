/*                                                                      */
/* OSTIME.C - OSTIME functions                                          */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module contains TOD functions                              */

#include "rtip.h"
#include "sock.h"
#include "ostime.h"
#if (INCLUDE_SNTP)
#include "sntpapi.h"
#include "sntp.h"
#include "rtipext.h"
#endif

#if (WEBC_WINSOCK)
#include "windows.h"
#endif
#if (defined(VXWORKS) || defined(__BORLANDC__) )
#include "time.h"
#endif

/* ********************************************************************   */
#define DISPLAY_TIME              0
#define DISPLAY_SMTP_TIME         0
#define DISPLAY_TIME_TIMER_UPDATE 0

/* ********************************************************************   */
/* requesting SNTP during timer routine is done in 2 steps to avoid       */
#define SNTP_REQ_STEP  0
#define SNTP_RESP_STEP 1

/* ********************************************************************   */
#if (INCLUDE_SNTP)
int get_sntp_time(PSNTP_TIMEOUT_DATA sntp_data);
#endif
#if (!KS_SUPPORTS_TIME)
static void ebs_tod_timeout(void KS_FAR *vp);
#endif
int   ebs_date_to_dayofweek(int day, int month, int year);
long  ebs_get_seconds(int year, int month, int day, int hour, int minute, int second);
#if (INCLUDE_WEB_BROWSER)
static int _getint(char *str, int offset, int length);
#endif

/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
#if (!KS_SUPPORTS_TIME || INCLUDE_SNTP)
EBSTIME          ebs_tod = {0, 0};      
EBS_TIMER KS_FAR ebs_tod_timer_info;        
dword            ticks_since_last_set;
int              elapsed_seconds_updated;
RTIP_BOOLEAN     ebs_tod_set = FALSE;
#else
RTIP_BOOLEAN     ebs_tod_set = TRUE;
#endif

#if (INCLUDE_SNTP)
SNTP_TIMEOUT_DATA sntp_tmeout_data;
#endif

#if (INCLUDE_RUN_TIME_CONFIG)
CFG_SNTP_DATA KS_FAR cfg_sntp_data =
{
    _CFG_TOD_FREQ,
#if (INCLUDE_SNTP)
    _CFG_SNTP_TOD_FREQ,
    _CFG_SNTP_RCV_TMO,
    _CFG_GMT_DIFF,
#endif
};
#endif

static int month_start[] =      {0, 31, 59, 90, 120, 151, 181, 212, 243, 
                                 273, 304, 334, 365};
static int month_start_leap[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 
                                 274, 305, 335, 366};
static char *weekday_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"};
static char *wkday_names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", 
    "Sat"};
static char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/* ********************************************************************        */
/* API TIME FOR SYSTEMS - !KS_SUPPORTS_TIME                                    */
/* ********************************************************************        */
/* ********************************************************************.       */
/* xn_ebs_set_time() - set TOD from an SNTP/NTP server or from parameters      */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "ostime.h"                                                       */
/*                                                                             */
/*   int xn_ebs_set_time(ip_sntp_server,  year,  month,  day,                  */
/*                   hour,  minute,  second)                                   */
/*       PFBYTE ip_sntp_server - IP addresses of SNTP/NTP server               */
/*       int year   -  year to set TOD                                         */
/*       int month  -  month to set TOD                                        */
/*       int day    -  day to set TOD                                          */
/*       int hour   -  hour to set TOD                                         */
/*       int minute -  minute to set TOD                                       */
/*       int second -  second to set TOD                                       */
/*                                                                             */
/* Description:                                                                */
/*   If SNTP is included: retrieves the current time of day from               */
/*   SNTP/NTP server specified in the parameter ip_sntp_server.                */
/*   If SNTP is not included but KS_SUPPORT_TIME is set to 1,                  */
/*   sets the current time of day from library calls.                          */
/*   If SNTP is not included and KS_SUPPORT_TIME is set to 0,                  */
/*   sets the current time of day from the parameters.                         */
/*                                                                             */
/*   The time of day is stored in the global variable ebs_tod which is         */
/*   of type struct _ebs_timeval.                                              */
/*                                                                             */
/*   A timer is set up to increase the time of day every CFG_TOD_FREQ seconds. */
/*   Every CFG_SNTP_TOD_FREQ seconds, SNTP will be called to retrieve the      */
/*   time of day.                                                              */
/*                                                                             */
/*   The routine xn_ebs_get_system_time() may be called to retrieve the        */
/*   time of day.                                                              */
/*   The routine xn_ebs_print_time() may be called to write the time of        */
/*   day into a string.                                                        */
/*                                                                             */
/* Returns 0 upon success and -1 upon error                                    */
/*                                                                             */

int xn_ebs_set_time(PFBYTE ip_sntp_server, int year, int month, int day, 
                    int hour, int minute, int second)
{
#if (!INCLUDE_SNTP)
    ARGSUSED_PVOID(ip_sntp_server)
#endif
#if (KS_SUPPORTS_TIME)
    ARGSUSED_INT(year)
    ARGSUSED_INT(month)
    ARGSUSED_INT(day)
    ARGSUSED_INT(hour)
    ARGSUSED_INT(minute)
    ARGSUSED_INT(second)

#else 
    ebs_tod.year = year;

    ebs_tod.second = ebs_get_seconds(ebs_tod.year, month, day,
                                      hour, minute, second);
    ticks_since_last_set = ks_get_ticks();
    elapsed_seconds_updated = 0;
#endif

#if (INCLUDE_SNTP)
    /* initialize timer for timer routine   */
    tc_memset(&sntp_tmeout_data, 0, sizeof(struct sntp_timeout_data));

    /* save SNTP SERVER ADDRESS, MODE and VERSION globally for timeout routine   */
    /* NOTE: most servers respond to NTP_SYM_ACTIVE but some only                */
    /*       respond to NTP_REQUEST; it looks like NTP servers respond           */
    /*       to NTP_SYM_ACTIVE and SNTP respond to NTP_REQUEST                   */
    tc_mv4(sntp_tmeout_data.sntp_ip_addr, ip_sntp_server, 4);
    sntp_tmeout_data.version = SNTP_VERSION_3;
    sntp_tmeout_data.mode = NTP_SYM_ACTIVE;

    if (get_sntp_time(&sntp_tmeout_data) < 0)
    {
        sntp_tmeout_data.mode = NTP_REQUEST;  
        if (get_sntp_time(&sntp_tmeout_data) < 0)
        {
            DEBUG_ERROR("xn_ebs_set_time failed: timer not started", 
                NOVAR, 0, 0);
            return(-1);
        }
    }
#endif

#if (!KS_SUPPORTS_TIME)
    ticks_since_last_set = ks_get_ticks();
    elapsed_seconds_updated = 0;

    /* check to make sure not starting second timer   */
    if (!ebs_tod_set)
    {
        /* set up timer to increase time of day   */
        ebs_tod_timer_info.func = ebs_tod_timeout;    /* routine to execute */
        ebs_tod_timer_info.arg = 0;                   /* dummy arg list - not used */
        ebs_set_timer(&ebs_tod_timer_info, CFG_TOD_FREQ, TRUE);
        ebs_start_timer(&ebs_tod_timer_info);
    }
#endif

    ebs_tod_set = TRUE;
    return(0);
}

#if (INCLUDE_SNTP)
/* ********************************************************************   */
int get_sntp_time(PSNTP_TIMEOUT_DATA sntp_data)
{
    /* xn_sntp_get_time   */
    if (sntp_request_time(sntp_data->sntp_ip_addr, 
                        &sntp_data->sntp_socket, 
                        &sntp_data->current_timestamp, 
                        sntp_data->version,
                        sntp_data->mode,
                        &sntp_data->ticks_when_sent,
                        (PEBSTIME)&ebs_tod) < 0)
    {
        DEBUG_ERROR("ebs_tod_timeout: sntp_request_time failed", 
            NOVAR, 0, 0);
        return(-1);
    }

    if (do_read_select(sntp_data->sntp_socket, CFG_SNTP_RCV_TMO))
    {
        if (sntp_process_result(sntp_data->sntp_socket, 
                                &sntp_data->current_timestamp, 
                                sntp_data->version,
                                sntp_data->ticks_when_sent,
                                &ebs_tod) >= 0)
        {
#if (DISPLAY_TIME)
            {
            char time_str[60];
            xn_ebs_print_time(time_str, &ebs_tod, 2);
            DEBUG_ERROR("TIME AT INITIALIZATION: ", STR1, time_str, 0);
            DEBUG_ERROR("TIME AT INITIALIZATION: ", DINT2, 
                ebs_tod.year, ebs_tod.second);
            }
#endif
            return(0);
        }
        else
        {
            DEBUG_ERROR("xn_ebs_set_time: sntp_process_result failed: mode = ", 
                EBS_INT1, sntp_data->mode, 0);
            return(-1); 
        }
    }        
    DEBUG_ERROR("xn_ebs_set_time: timed out waiting for response: mode = ", 
            EBS_INT1, sntp_data->mode, 0);
    closesocket(sntp_data->sntp_socket);
    return(set_errno(ETIMEDOUT));
}

/* ********************************************************************   */
/* setup for sntp step in timer routine                                   */
void setup_req_step(void)
{
    sntp_tmeout_data.sntp_step_timeout = SNTP_REQ_STEP;
    /* set timer back from every second   */
    ebs_set_timer(&ebs_tod_timer_info, CFG_TOD_FREQ, TRUE);
    sntp_tmeout_data.sntp_elapsed_sec = 0;
}
#endif /* INCLUDE_SNTP */

#if (!KS_SUPPORTS_TIME || INCLUDE_SNTP)
/* ********************************************************************   */
/* called every CFG_TOD_FREQ seconds to increment time                    */
static void ebs_tod_timeout(void KS_FAR *vp)
{
dword elapsed_ticks;
int   elapsed_seconds;
dword ticks;
#if (INCLUDE_SNTP)
RTIP_BOOLEAN updated_time;
#endif

    ARGSUSED_PVOID(vp)

#if (INCLUDE_SNTP)
    updated_time = FALSE;

    /* check if it is time to do another SNTP update;    */
    /* if not than do the else below and update the time */
    /* by CFG_TOD_FREQ                                   */
    sntp_tmeout_data.sntp_elapsed_sec += CFG_TOD_FREQ;
    if ( (sntp_tmeout_data.sntp_elapsed_sec >= CFG_SNTP_TOD_FREQ) ||
         (sntp_tmeout_data.sntp_step_timeout == SNTP_RESP_STEP) ) 
    {
        switch (sntp_tmeout_data.sntp_step_timeout)
        {
        case SNTP_REQ_STEP:
            if (sntp_request_time(sntp_tmeout_data.sntp_ip_addr, 
                              &sntp_tmeout_data.sntp_socket, 
                              &sntp_tmeout_data.current_timestamp, 
                              sntp_tmeout_data.version,
                              sntp_tmeout_data.mode,
                              &sntp_tmeout_data.ticks_when_sent,
                              (PEBSTIME)&ebs_tod) < 0)
            {
                DEBUG_ERROR("ebs_tod_timeout: sntp_request_time failed", 
                    NOVAR, 0, 0);
                return;
            }

            /* go to next step and setup for next step by setting timer   */
            /* for select and make timeout routine wake up every second   */
            /* to check for a response                                    */
            sntp_tmeout_data.sntp_step_timeout = SNTP_RESP_STEP;
            sntp_tmeout_data.sntp_select_timer = CFG_SNTP_RCV_TMO;
            ebs_set_timer(&ebs_tod_timer_info, 1, TRUE);
            break;

        case SNTP_RESP_STEP:
            /* stay in this case waiting for response   */
            if (do_read_select(sntp_tmeout_data.sntp_socket, 0))
            {
                if (sntp_process_result(sntp_tmeout_data.sntp_socket, 
                                        &sntp_tmeout_data.current_timestamp, 
                                        sntp_tmeout_data.version,
                                        sntp_tmeout_data.ticks_when_sent,
                                        &ebs_tod) >= 0)
                {
                    /* SUCCESS   */
                    setup_req_step();

                    updated_time = TRUE;
#if (DISPLAY_SMTP_TIME)
                    {
                    char time_str[60];
                        xn_ebs_print_time(time_str, &ebs_tod, 2);
                        DEBUG_ERROR("TIME AFTER NEW SMTP REQUEST: ", STR1, time_str, 0);
                    }
#endif
                }
            }

            /* RESPONSE IS NOT IN   */
            else
            {
                sntp_tmeout_data.sntp_select_timer--;
                if (sntp_tmeout_data.sntp_select_timer <= 0)
                {
                    /* TIMED OUT   */

#if (DISPLAY_SMTP_TIME)
                    {
                    char time_str[60];
                        xn_ebs_print_time(time_str, &ebs_tod, 2);
                        DEBUG_ERROR("TIME AFTER NEW SNTP REQUEST TIMED OUT: ", 
                            STR1, time_str, 0);
                    }
#endif
                    DEBUG_ERROR("ebs_tod_timeout: timed out waiting for response", NOVAR, 0, 0);
                    closesocket(sntp_tmeout_data.sntp_socket);

                    setup_req_step();
                }

            }
            break;
        }

    }
    if (!updated_time)
#endif
    {
        ticks = ks_get_ticks();
        if (ticks > ticks_since_last_set)
            elapsed_ticks = ticks - ticks_since_last_set;
        else
        {
            elapsed_ticks = ticks + (0xffffffff-ticks_since_last_set); 
        }

        elapsed_seconds = (int)(elapsed_ticks / ks_ticks_p_sec());
#    if (1)
        elapsed_seconds -= elapsed_seconds_updated;
        elapsed_seconds_updated += elapsed_seconds;
        if (elapsed_seconds_updated > 0x3fff)  /* about every 4 hours reset */
        {
            ticks_since_last_set = ticks;
            elapsed_seconds_updated = 0;
        }
#    else
        ticks_since_last_set = ticks;
#    endif

        ebs_add_seconds((PEBSTIME)&ebs_tod, elapsed_seconds);
#if (DISPLAY_TIME_TIMER_UPDATE)
        {
        char time_str[60];
            xn_ebs_print_time(time_str, &ebs_tod, 2);
            DEBUG_ERROR("TIME AFTER TIMER UPDATE: ", STR1, time_str, 0);
        }
#endif
    }

    /* restart the timer   */
    ebs_start_timer(&ebs_tod_timer_info); 
}
#endif /* !KS_SUPPORTS_TIME || INCLUDE_SNTP */

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */


/* ********************************************************************   */
/* GET TIME                                                               */
/* ********************************************************************   */
/* xn_ebs_get_system_time() - retrieve TOD                                */
/*                                                                        */
/* Summary:                                                               */
/*   #include "ostime.h"                                                  */
/*                                                                        */
/*   int xn_ebs_get_system_time(ptime)                                    */
/*      PEBSTIME ptime - TOD is written into ptime                        */
/*                                                                        */
/* Description:                                                           */
/*    Retrieve TOD set by calling xn_ebs_set_time.                        */
/*                                                                        */
/* Returns:                                                               */
/*    0 upon success, -1 upon failure                                     */

int xn_ebs_get_system_time(PEBSTIME ptime)
{
#if (KS_SUPPORTS_TIME)
int year, month, day, hour, minute, second;
#endif

    /* if xn_ebs_set_time() has not been called   */
    if (!ebs_tod_set)
        return(-1);
        
#if (!KS_SUPPORTS_TIME)
    ptime->year = ebs_tod.year;
    ptime->second = ebs_tod.second;
    return (0);
#endif

#if (KS_SUPPORTS_TIME)
#if (WEBC_WINSOCK )
    {
    SYSTEMTIME st;

        GetSystemTime((LPSYSTEMTIME)&st);
        year   = st.wYear;
        month  = st.wMonth;
        day    = st.wDay;
        hour   = st.wHour;
        minute = st.wMinute;
        second = st.wSecond;
    }

#else
    {
        time_t timeSecs;
        struct tm *timeDates;

        /* get elapsed time since 1/1/70 00:00:00 GMT   */
        time(&timeSecs);
        timeDates = localtime((const time_t *)&timeSecs);

        year   = timeDates->tm_year + 1900;
        month  = timeDates->tm_mon + 1;
        day    = timeDates->tm_mday;
        hour   = timeDates->tm_hour;
        minute = timeDates->tm_min;
        second = timeDates->tm_sec;
    }
#endif
    
#ifdef __MET__
#pragma Offwarn(64)
#endif

    ptime->year = year;
    ptime->second = ebs_get_seconds(year,month,day,hour,minute,second);

    return (0);

#ifdef __MET__
#pragma Onwarn(64)
#endif

#endif      /* KS_SUPPORTS_TIME */

}

/* ********************************************************************   */
/* xn_ebs_print_time() - writes the TOD in string format                  */
/*                                                                        */
/* Summary:                                                               */
/*   #include "ostime.h"                                                  */
/*                                                                        */
/*   int xn_ebs_print_time(str, ptime, style)                             */
/*      PFCHAR str     - string where TOD is written                      */
/*      PEBSTIME ptime - TOD to be written in string format               */
/*      int style      - style time is written where:                     */
/*                       style 1 = GMT                                    */
/*                       style 2 = local time                             */
/*                       all other styles = GMT                           */
/*                                                                        */
/* Description:                                                           */
/*    Retrieve TOD set by calling xn_sntp_get_time or xn_ebs_set_time.    */
/*                                                                        */
/* Returns:                                                               */
/*    0 upon success, -1 upon failure                                     */

int xn_ebs_print_time(char *str, PEBSTIME ptime, int style)
{
int year, month, day, hour, minute, second, day_of_week;

    convert_ebs_to_time(ptime, &year, &month, &day,
                         &hour, &minute, &second);
    day_of_week = ebs_date_to_dayofweek(day, month+1, ptime->year);

    switch (style)
    {
    case 1:
        tc_sprintf (str,"%s, %02d-%s-%02d %02d:%02d:%02d GMT", 
            weekday_names[day_of_week % 7],
            day,
            month_names[month % 12],
            ptime->year % 100,
            hour,
            minute,
            second);
        break;
    
    case 2:
        tc_sprintf (str,"%s %s %2d %02d:%02d:%02d %04d", 
            wkday_names[day_of_week % 7],
            month_names[month % 12],
            day,
            hour,
            minute,
            second,
            ptime->year);
        break;

    default:
        tc_sprintf (str,"%s, %02d %s %04d %02d:%02d:%02d GMT", 
            wkday_names[day_of_week % 7],
            day,
            month_names[month % 12],
            ptime->year,
            hour,
            minute,
            second);
        break;
    }

    return (0); 
}

/* ********************************************************************   */
/* UTILITIES                                                              */
/* ********************************************************************   */


/* ********************************************************************   */
/* Takes unsigned format:                                                 */
/* Date:                                                                  */
/*      fedcba9876543210                                                  */
/*      yyyyyyymmmmddddd                                                  */
/*         d=1-31, m=1-12, y=0-119 (1980-2099)                            */
/* Time:                                                                  */
/*      fedcba9876543210                                                  */
/*      hhhhhmmmmmmsssss                                                  */
/*         h=hour, m=minutes, s=seconds/2 (i.e. 10 sssss = 20 seconds)    */

/* tbd: is this needed   */

void convert_time_to_ebs(int date, int time, PEBSTIME ptime)
{
int year;
int month;
int day;
int hour;
int minute;
int second;

    year = (int)((date >> 9) & 0x7f);
    year = year + 1980;
    month = (int)(date >> 5);
    month = (month & 0xf) - 1;
    if (month > 11 || month < 0) 
        month = 0;
    day = (int)(date & 0x1f);
    if (day == 0)
      day = 1;            

    hour = (time >> 11) & 0x1f;
    minute = (time & 0x3f8) >> 5;
    second = (time & 0x1f);

    ptime->year = year;
    ptime->second = ebs_get_seconds(year, month, day, hour, minute, second);
}

/* ********************************************************************   */
/* converts year,seconds to                                               */
/*          year,month,day,hour,minute,second                             */
void convert_ebs_to_time(PEBSTIME ptime, int *year, int *month, int *day,
                         int *hour, int *minute, int *second)
{
long total;
RTIP_BOOLEAN is_leap;

    *year = ptime->year;

    total = ptime->second;

    *second = (int)(total % 60);
    total /= 60;
    *minute = (int)(total % 60);
    total /= 60;
    *hour = (int)(total % 24);
    total /= 24;

    /* determine if year is leap year   */
    is_leap = TRUE;
    if (*year % 4 != 0)
        is_leap = FALSE;
    else if (*year % 100 != 0)
        is_leap = TRUE;
    else if (*year % 400 != 0)
        is_leap = FALSE;

    if (is_leap)
    {
        for (*month=0; month_start_leap[*month + 1] < total; (*month)++)
            ;
        *day = (int)(total - month_start_leap[*month] + 1);
    }
    else
    {
        for (*month=0; month_start[*month + 1] < total; (*month)++)
            ;
        *day = (int)(total - month_start[*month] + 1);
    }
}

/* ********************************************************************   */
int ebs_date_to_dayofweek(int day, int month, int year)
{
long jul;

    jul = 367*(long)year 
        - 7 *  ((long)year + ((long)month + 9)/12) / 4
        - 3 * (((long)year + ((long)month - 9)/7 ) / 100 + 1) / 4
        + (275*(long)month / 9) + (long)day + 1721030L;

    return (int)(jul % 7);
}

/* ********************************************************************   */
/* return number of seconds elapsed in year                               */
long ebs_get_seconds(int year, int month, int day, int hour, int minute, int second)
{
long total;
RTIP_BOOLEAN is_leap;

    total = 0;

    /* determine if year is leap year   */
    is_leap = TRUE;
    if (year % 4 != 0)
        is_leap = FALSE;
    else if (year % 100 != 0)
        is_leap = TRUE;
    else if (year % 400 != 0)
        is_leap = FALSE;

    /* get elapsed days in year   */
    if (is_leap)
    {
        total += month_start_leap[month-1];
    }
    else
    {
        total += month_start[month-1];
    }
    total += day-1;
    total *= 24;
    total += hour;
    total *= 60;
    total += minute;
    total *= 60;
    total += second;

    return (total);
}

/* ********************************************************************   */
long ebs_get_seconds_in_year(int year)
{
    /* get number of seconds since 12/31/year 59:59   */
    return (ebs_get_seconds(year,12,31,23,59,59)+1);
}

/* ********************************************************************   */
void ebs_add_seconds(PEBSTIME ptime, long seconds)
{
    if ((seconds > 0) && ((ptime->second + seconds) < ptime->second))
    {
        ptime->year++;
        ptime->second -= ebs_get_seconds_in_year(ptime->year);
    }

    if ((seconds < 0) && ((ptime->second + seconds) > ptime->second))
    {
        ptime->year--;
        ptime->second += ebs_get_seconds_in_year(ptime->year);
    }
    
    ptime->second += seconds;
}

#if (INCLUDE_WEB_BROWSER)
/* ********************************************************************   */
int ebs_parse_time(PEBSTIME ptime, char *str)
{
char *s;
int year, month, day, hour, minute, second, day_of_week;
int result;

    result = 0;

    for (month=0; month<12; month++)
    {
        if (tc_strstr(str, month_names[month]))
            break;
    }
    if (month == 12)
    {
        return (-1);
    }
    month += 1;

    for (day_of_week=0; day_of_week<7; day_of_week++)
    {
        s = tc_strstr(str, weekday_names[day_of_week]);
        if (s)
        {
            str = s + tc_strlen(weekday_names[day_of_week]);

            day = _getint(str,2,2);
            year = _getint(str,9,2) + 1900; /* WARNING: NOT Y2K friendly */
            if (year < 1925) /* Y2K Hack - will work until 2025 */
                year += 100;
            hour = _getint(str,12,2);
            minute = _getint(str,15,2);
            second = _getint(str,18,2);

            break;
        }
    }

    if (day_of_week == 7)
    {
        for (day_of_week=0; day_of_week<7; day_of_week++)
        {
            s = tc_strstr(str, wkday_names[day_of_week]);
            if (s)
            {
                str = s + tc_strlen(wkday_names[day_of_week]);
                switch(*str)
                {
                    case ',':
                        day = _getint(str,2,2);
                        year = _getint(str,9,4);
                        hour = _getint(str,14,2);
                        minute = _getint(str,17,2);
                        second = _getint(str,20,2);
                        break;
    
                    case ' ':
                        day = _getint(str,5,2);
                        year = _getint(str,17,4);
                        hour = _getint(str,8,2);
                        minute = _getint(str,11,2);
                        second = _getint(str,14,2);
                        break;

                    default:
                        break;
                }
                break;
            }
        }
    }

    if (day_of_week == 7)
    {
        year   = 1980;
        month  = 1;
        day    = 1;
        hour   = 0;
        minute = 0;
        second = 0;
        result = -1;
    }

    ptime->year = year;
    ptime->second = ebs_get_seconds(year,month,day,hour,minute,second);
    
    return (result);
}

/* ********************************************************************   */
int _getint(char *str, int offset, int length)
{
char c;
int i;

    c = str[offset + length];
    str[offset + length] = '\0';
    i = tc_atoi(&str[offset]);
    str[offset + length] = c;
    
    return (i);
}

/* ********************************************************************   */
int ebs_compare_time(PEBSTIME ptime1, PEBSTIME ptime2)
{
    if (ptime1->year > ptime2->year)
        return (1);

    if (ptime1->year < ptime2->year)
        return (-1);

    if (ptime1->second > ptime2->second)
        return (1);

    if (ptime1->second < ptime2->second)
        return (-1);

    return (0);
}

/* ********************************************************************   */
void ebs_set_time_forever(PEBSTIME ptime)
{
    ptime->year = 0xffff;
    ptime->second = 0xffffffffL;
}

/* ********************************************************************   */
long ebs_time_difference(PEBSTIME pearlier, PEBSTIME plater)
{
int year;
long difference = 0;
long sign = 1;

    if (ebs_compare_time(pearlier, plater) > 0)
    {
        PEBSTIME ptemp = pearlier;
        pearlier = plater;
        plater = ptemp;
        sign = -1;
    }
    
    for(year = pearlier->year; year < plater->year; year++)
    {
        difference += ebs_get_seconds_in_year(year++);
    }
    
    difference += plater->second - pearlier->second;
    return (difference * sign);
}

#endif  /* if (INCLUDE_WEB_BROWSER) */
