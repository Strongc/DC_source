/*                                                                      */
/* DEBUG.H - DISPLAY functions                                          */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */


#ifndef __RTIP_DEBUG__
#define __RTIP_DEBUG__ 1

#include "rtip.h"
#include "xnconf.h"

/* types passed to say_init (do not change these)   */
#define DO_RS232_IO   1     /* DEBUG_LOG info written to rs232 */
#define DO_FILE_IO    2     /* DEBUG_LOG info written to disk */
#define DO_SCREEN_IO  3     /* DEBUG_LOG info written to screen */

/* ********************************************************************   */
/* YOU MIGHT WANT TO CHANGE VALUES IN THE FOLLOWING CONFIGUATION SECTION  */
/* ********************************************************************   */

/* ERROR LOGGING:                                                             */
/* Two macros DEBUG_ERROR and DEBUG_LOG are used throughout                   */
/* the code to log error and diagnostic messages.                             */
/* NOTE: ERROR LOGGING (DEBUG_ERROR) goes to the screen                       */
/*       and possiby out thru the RS232;  (it is also logged to RS232/FILE if */
/*       diagnostic messages are being logged or LOG_ERRS is set to 1).       */
/*       Diagnostic messages (DEBUG_LOG) are either sent out on RS232,        */
/*       written to a FILE (a:debug.out), or written to the SCREEN            */
/*       depending upon what IO_TYPE_FSR is set to                            */

/* Set error logging level such that:                                          */
/*   Level 0 = neither error messages or debug messages                        */
/*             i.e. set none of the calls to the DEBUG_ERROR                   */
/*             or DEBUG_LOG will be compiled in                                */
/*   Level 1 = error messages only (DEBUG_ERROR and DEBUG_ERROR_LOG),          */
/*             i.e. none of the calls to DEBUG_LOG will be compiled in         */
/*   Level 2 = error messages + application messages (DEBUG and DEBUG_LOG)     */
/*             NOTE: DEBUG_LOG calls in application are set to level 2         */
/*             NOTE: this level is very verbose and can change the timing      */
/*                   of RTIP                                                   */
/*   Level 3 = error messages + application messages + general flow of program */
/*             (DEBUG_ERROR and DEBUG_LOG)                                     */
/*             NOTE: DEBUG_LOG calls in application are set to level 2 and     */
/*                   DEBUG_LOG calls in RTIP are set to level 3                */
/*             NOTE: this level is very verbose and can change the timing      */
/*                   of RTIP                                                   */
/*   Level 4 = error messages + general flow of program +                      */
/*             more specific debug information (DEBUG and DEBUG_LOG)           */
/*             NOTE: DEBUG_LOG calls in application are set to level 2 and     */
/*                   DEBUG_LOG calls in RTIP are set to level 3                */
/*             NOTE: this level is very verbose and can change the timing      */
/*                   of RTIP                                                   */
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0   
#endif

#define DEBUG_SYSLOG_LEVEL(level)   diag_debug_syslog_level_limit

/* used if DEBUG_LEVEL=1; if set DEBUG_ERROR will                         */
/* be logged via FILE, SCREEN or RS232 (as determined by IO_TYPE_FSR) in  */
/* addition to logging error to the screen;                               */
/* NOTE: this does not make sense if IO_TYPE_FSR is SCREEN_TYPE or        */
/*       it will get written twice                                        */
#if (IS_16550_DEVICE)               /* if CONDEV_DEVICE is 16650 */
#define SEND_LEVEL1_TO_FSR  0       /* DO NOT CHANGE */
#else
#define SEND_LEVEL1_TO_FSR  1
#endif

/* DEBUG_LOG: type of device to output messages to (screen, file or rs232)   */
/* only one of the following should be uncommented                           */
/* NOTE: you only need to worry about this if DEBUG_LEVEL > 1 OR             */
/*       DEBUG_LEVEL is 1 and SEND_LEVEL1_TO_FSR is 1                        */
#define IO_TYPE_FSR DO_SCREEN_IO     /* DEBUG_LOG will be written to the screen */  /*OS*/ 
/*OS*/ /* #define IO_TYPE_FSR DO_FILE_IO  */           /* DEBUG_LOG will be written to a file      */
/* #define IO_TYPE_FSR DO_RS232_IO */          /* DEBUG_LOG will be sent out over RS232       */         

#define SAY_COMM_PORT 3 /* comm port used to dump debug info */
                        /* MAKE SURE THIS DOES NOT CONFLICT WITH        */
                        /* COMM PORT USED BY SLIP AND PPP!!!!!!!!!!!!   */
                        /* NOTE: you may modify the port by setting the */
                        /*       variable say_port before any calls     */
                        /*       to DEBUG_ERROR or DEBUG_LOG            */


/* ********************************************************************   */
/* END OF CONFIGUATION SECTION                                            */
/* ********************************************************************   */

/* *****************************************************                 */
/* set internal define which are used to determine which code to include */
/* or exclude                                                            */
#if (DEBUG_LEVEL == 0)
#define USE_DB_L0 1
#define USE_DB_L1 0
#elif (DEBUG_LEVEL == 1)
#define USE_DB_L0 0
#define USE_DB_L1 1
#else
#define USE_DB_L0 0
#define USE_DB_L1 0
#endif

/* DETERMINE IF SHOULD SET UP FOR FILE OR RS232 OUTPUT               */
/* IO_TYPE_FSR determines whether will send to FILE, SCREEN or RS232 */
#if (!USE_DB_L1)
#define SEND_TO_FSR 0   /* MIGHT WANT TO CHANGE */
#elif (USE_DB_L1 && SEND_LEVEL1_TO_FSR)
#define SEND_TO_FSR 0   /* MIGHT WANT TO CHANGE */
#else
#define SEND_TO_FSR 0   /* MIGHT WANT TO CHANGE */
#endif

/* determine if FILE or RS232 should be compiled in   */
#define DO_FILE (SEND_TO_FSR && IO_TYPE_FSR == DO_FILE_IO)
                                /* set if want to compile in code to write   */
                                /* to a file                                 */
                                /* NOTE: makes calls to fopen, fprintf,      */
                                /*       fflush and fclose; if this is       */
                                /*       unaceptable and you want to         */
                                /*       to debug information to a           */
                                /*       file, modify these calls in         */
                                /*       debug.c                             */

#define DO_RS232 (SEND_TO_FSR && IO_TYPE_FSR == DO_RS232_IO)
                                /* set if want to compile in code to write   */
                                /* over RS232                                */

#define LEVEL_1  1
#define LEVEL_2  2
#define LEVEL_3  3
#define LEVEL_4  4

/* out_type parameter to say routines   */
#define FILE_TYPE         -1
#define SCREEN_TYPE       -2

/* ********************************************************************   */
/* DIAG_*() SECTION IDs                                                   */
typedef enum /* use 'enum' type as it 'auto-numbers' each element... */
{
  DIAG_SECTION_UNKNOWN = 0,
  DIAG_SECTION_DIRECTDIAG,         /* 1 */

  DIAG_SECTION_PPP,                /* 2 */
  DIAG_SECTION_UDP,                /* 3 */
  DIAG_SECTION_TCP,                /* 4 */
  DIAG_SECTION_IP,                 /* 5 */
  DIAG_SECTION_ARP,                /* 6 */
  DIAG_SECTION_RARP,               /* 7 */
  DIAG_SECTION_SNMP,               /* 8 */
  DIAG_SECTION_ICMP,               /* 9 */
  DIAG_SECTION_IGMP,               /* 10 */
  DIAG_SECTION_RIP,                /* 11 */
  DIAG_SECTION_DNS,                /* 12 */
  DIAG_SECTION_MAIL,               /* 13 */
  DIAG_SECTION_FTP,                /* 14 */
  DIAG_SECTION_TFTP,               /* 15 */
  DIAG_SECTION_TELNET,             /* 16 */
  DIAG_SECTION_WEB,                /* 17 */
  DIAG_SECTION_DHCP,               /* 18 */

  /* routines used by POP/SMTP/FTP/WEB   */
  DIAG_SECTION_IPLINE = 20,        /* 20 */

  DIAG_SECTION_OS = 30,            /* 30 */
  DIAG_SECTION_DRIVER,             /* 31 */

  DIAG_SECTION_BGET = 40,          /* 40 */
  DIAG_SECTION_MEMSTATDUMP,        /* 41 */

  DIAG_SECTION_ERTFS = 50,         /* 50 */
  DIAG_SECTION_NFS,                /* 51 */
  DIAG_SECTION_MFS,                /* 52 */
  DIAG_SECTION_VFS,                /* 53 */

  DIAG_SECTION_CRYPTTEST = 60,     /* 60 */
  DIAG_SECTION_CRYPTOLIB,          /* 61 */
  DIAG_SECTION_OPENSSL,            /* 62 */

  DIAG_SECTION_API = 70,           /* 70 */

  DIAG_SECTION_DIAGNOSTICS = 80,   /* 80 */

  DIAG_SECTION_USER = 100          /* 100 */
} xd_diag_section_t;



extern int KS_FAR debug_handle;

/* ********************************************************************   */
/* NOTE: if 1 than can call says directly and always will be displayed;   */
/*       if 0 then direct calls will be compiled out                      */
/*       NOTE: this controls whether output from special keys in demo     */
/*             display results                                            */

#ifdef __cplusplus
extern "C" {
#endif

#if (!USE_DB_L0)
    void say_close(void);
    void say(int out_type, PFCCHAR comment);
    void say_hex(int out_type, PFCCHAR comment, PFBYTE p, int len);
    void say_ip_addr(int out_type, PFCCHAR comment, byte KS_FAR *addr);
    void say_ether_addr(int out_type, PFCCHAR comment, byte KS_FAR *addr);
    void say_int(int out_type, PFCCHAR comment, int val);
    void say_int2(int out_type, PFCCHAR comment, int val1, int val2);
    void say_lint(int out_type, PFCCHAR comment, long val1);
    void say_lint2(int out_type, PFCCHAR comment, long val1, long val2);
    void say_dint(int out_type, PFCCHAR comment, dword val1);
    void say_dint2(int out_type, PFCCHAR comment, dword val1, dword val2);
    void say_str(int out_type, PFCCHAR comment, PFCHAR val1);
    void say_str2(int out_type, PFCCHAR comment, PFCHAR val1, PFCHAR val2);
    void say_ports_tcp(int out_type, PFCCHAR comment, dword val1);
    void say_ports_udp(int out_type, PFCCHAR comment, dword val1);
    void say_ports_raw(int out_type, PFCCHAR comment, dword val1);
#if (INCLUDE_ROUTING_TABLE)
    void say_rt_entry(int out_type, int i, PROUTE prt);
    void say_rt(int out_type, dword val1);
#endif
    void say_arpcache_entry(int out_type, int index);

#else
    #define say_init(X,Y,Z)
    #define say_close()
    #define say_1(string)
    #define say_2(string)
    #define say_3(string)
    #define say_4(string)
    #define say_5(string)
    #define say_6(string)
    #define say_7(string)
    #define say_8(string)
    #define db_say_1_n(n)
    #define db_say_2_n(n)
    #define db_say_3_n(n)
    #define db_say_4_n(n)
    #define db_say_5_n(n)
    #define db_say_6_n(n)
    #define db_say_7_n(n)
    #define db_say_8_n(n)
    #define say_rt(n)
#endif

/* ********************************************************************   */
void debug_error(PFCCHAR string, int type, dword val1, dword val2);
void debug_log(PFCCHAR string, int level, int type, dword val1, dword val2);

void diag_debug_message(xd_diag_section_t section, int syslog_level, PFCCHAR string, ...); /* free format printf() style message */

#if (USE_DB_L0)
    #define DEBUG_ERROR(STR, TYPE, X1, X2)
    #define DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)
    #define DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)

    #define DIAG_ERROR(SECTION, STR, TYPE, X1, X2)                            \
                DEBUG_ERROR(STR, TYPE, X1, X2)
    #define DIAG_LOG(SECTION, STR, LEVEL, TYPE, X1, X2)                       \
                DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)
    #define DIAG_ASSERT(MUST_BE_TRUE,SECTION, STR,TYPE,X1,X2)                 \
                DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)
    #define DIAG_ASSERT_LOG(MUST_BE_TRUE,SECTION, STR,LEVEL,TYPE,X1,X2)       \
                DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)
    #define DIAG_MSG(msg_fmt)

#elif (USE_DB_L1)
    #define DEBUG_ERROR(STR, TYPE, X1, X2)                                    \
        debug_error((PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    #define DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)

#if (1)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2) \
        if (!(MUST_BE_TRUE)) DEBUG_ERROR(STR,TYPE,X1,X2)
#else
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)                           \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DEBUG_ERROR("Assertion failed:",STR1,#MUST_BE_TRUE,0);          \
                DEBUG_ERROR(STR,TYPE,X1,X2);                                    \
            }                                                                   \
        } while(0)
#endif
    #define DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)

    #define DIAG_ERROR(SECTION, STR, TYPE, X1, X2)                            \
                DEBUG_ERROR(STR, TYPE, X1, X2)
    #define DIAG_LOG(SECTION, STR, LEVEL, TYPE, X1, X2)                       \
                DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)
    #define DIAG_ASSERT(MUST_BE_TRUE,SECTION, STR,TYPE,X1,X2)                 \
                DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)
    #define DIAG_ASSERT_LOG(MUST_BE_TRUE,SECTION, STR,LEVEL,TYPE,X1,X2)       \
                DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)
#if (CFG_USE_DIAG_MSG)
    #define DIAG_MSG(msg_fmt)                                                 \
        diag_debug_message msg_fmt
#else
    #define DIAG_MSG(msg_fmt)
#endif

#else
    #define DEBUG_ERROR(STR, TYPE, X1, X2)                                    \
        debug_error((PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    #define DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)                               \
        debug_log((PFCCHAR)STR, LEVEL, TYPE, (dword)X1, (dword)X2)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)                           \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DEBUG_ERROR("Assertion failed:",STR1,#MUST_BE_TRUE,0);          \
                DEBUG_ERROR(STR,TYPE,X1,X2);                                    \
            }                                                                   \
        } while(0)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)                 \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DEBUG_LOG("Assertion failed:",LEVEL,STR1,#MUST_BE_TRUE,0);      \
                DEBUG_LOG(STR,LEVEL,TYPE,X1,X2);                                \
            }                                                                   \
        } while(0)

    #define DIAG_ERROR(SECTION, STR, TYPE, X1, X2)                            \
                DEBUG_ERROR(STR, TYPE, X1, X2)
    #define DIAG_LOG(SECTION, STR, LEVEL, TYPE, X1, X2)                       \
                DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)
    #define DIAG_ASSERT(MUST_BE_TRUE,SECTION, STR,TYPE,X1,X2)                 \
                DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)
    #define DIAG_ASSERT_LOG(MUST_BE_TRUE,SECTION, STR,LEVEL,TYPE,X1,X2)       \
                DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)
#if (CFG_USE_DIAG_MSG)
    #define DIAG_MSG(msg_fmt)                                                 \
        diag_debug_message msg_fmt
#else
    #define DIAG_MSG(msg_fmt)
#endif

#endif

/* ********************************************************************     */
/*                                                                          */
/* syslog/screen/serial: New Debug Style!                                   */
/*                                                                          */

#if (CFG_USE_DIAG_MSG)
#define DIRECTDEBUG_LINE(msg_set)              directdebug_line_printf msg_set
#define VDIRECTDEBUG_LINE(msg, va_arg_set)     vdirectdebug_line_printf(msg, va_arg_set)

void vdirectdebug_line_printf(PFCCHAR f, va_list ap);
void directdebug_line_printf(PFCCHAR f, ...);

#undef DEBUG_ERROR
#undef DEBUG_LOG
#undef DEBUG_ASSERT
#undef DEBUG_ASSERT_LOG

#undef DIAG_ERROR
#undef DIAG_LOG
#undef DIAG_ASSERT
#undef DIAG_ASSERT_LOG
#undef DIAG_MSG

#ifndef DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_UNKNOWN
#endif

#if (USE_DB_L0)
    #define DEBUG_ERROR(STR, TYPE, X1, X2)
    #define DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)
    #define DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)

    #define DIAG_ERROR(SECTION, STR, TYPE, X1, X2)
    #define DIAG_LOG(SECTION, STR, LEVEL, TYPE, X1, X2)
    #define DIAG_ASSERT(MUST_BE_TRUE,SECTION, STR,TYPE,X1,X2)
    #define DIAG_ASSERT_LOG(MUST_BE_TRUE,SECTION, STR,LEVEL,TYPE,X1,X2)
    #define DIAG_MSG(msg_fmt)                                                   \
        diag_debug_message msg_fmt
#elif (USE_DB_L1)
    #define DEBUG_ERROR(STR, TYPE, X1, X2)                                      \
        diag_debug_errorlog(DIAG_SECTION_KERNEL, diag_map_errlevel2syslog_level(LEVEL_1), (PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    #define DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)                           \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DEBUG_ERROR("Assertion failed:",STR1,#MUST_BE_TRUE,0);          \
                DEBUG_ERROR(STR,TYPE,X1,X2);                                    \
            }                                                                   \
        } while(0)
    #define DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)

    #define DIAG_ERROR(SECTION, STR, TYPE, X1, X2)                              \
        diag_debug_errorlog(SECTION, diag_map_errlevel2syslog_level(LEVEL_1), (PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    #define DIAG_LOG(SECTION, STR, LEVEL, TYPE, X1, X2)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DIAG_ASSERT(MUST_BE_TRUE,SECTION,STR,TYPE,X1,X2)                    \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DIAG_ERROR(SECTION,"Assertion failed:",STR1,#MUST_BE_TRUE,0);   \
                DIAG_ERROR(SECTION,STR,TYPE,X1,X2);                             \
            }                                                                   \
        } while(0)
    #define DIAG_ASSERT_LOG(MUST_BE_TRUE,SECTION, STR,LEVEL,TYPE,X1,X2)
    #define DIAG_MSG(msg_fmt)                                                   \
        diag_debug_message msg_fmt
#else
    #define DEBUG_ERROR(STR, TYPE, X1, X2)                                      \
        diag_debug_errorlog(DIAG_SECTION_KERNEL, diag_map_errlevel2syslog_level(LEVEL_1), (PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    #define DEBUG_LOG(STR, LEVEL, TYPE, X1, X2)                                 \
        diag_debug_log(DIAG_SECTION_KERNEL, diag_map_errlevel2syslog_level(LEVEL), (PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)

    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DEBUG_ASSERT(MUST_BE_TRUE,STR,TYPE,X1,X2)                           \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DEBUG_ERROR("Assertion failed:",STR1,#MUST_BE_TRUE,0);          \
                DEBUG_ERROR(STR,TYPE,X1,X2);                                    \
            }                                                                   \
        } while(0)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DEBUG_ASSERT_LOG(MUST_BE_TRUE,STR,LEVEL,TYPE,X1,X2)                 \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DEBUG_LOG("Assertion failed:",LEVEL,STR1,#MUST_BE_TRUE,0);      \
                DEBUG_LOG(STR,LEVEL,TYPE,X1,X2);                                \
            }                                                                   \
        } while(0)

    #define DIAG_ERROR(SECTION, STR, TYPE, X1, X2)                              \
        diag_debug_errorlog(SECTION, diag_map_errlevel2syslog_level(LEVEL_1), (PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    #define DIAG_LOG(SECTION, STR, LEVEL, TYPE, X1, X2)                         \
        diag_debug_log(SECTION, diag_map_errlevel2syslog_level(LEVEL), (PFCCHAR)STR, TYPE, (dword)X1, (dword)X2)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DIAG_ASSERT(MUST_BE_TRUE,SECTION,STR,TYPE,X1,X2)                    \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DIAG_ERROR(SECTION,"Assertion failed:",STR1,#MUST_BE_TRUE,0);   \
                DIAG_ERROR(SECTION,STR,TYPE,X1,X2);                             \
            }                                                                   \
        } while(0)
    /* make sure this macro acts as a 'single statement' when
     * postfixed by a regular semicolon... */
    #define DIAG_ASSERT_LOG(MUST_BE_TRUE,SECTION,STR,LEVEL,TYPE,X1,X2)          \
        do                                                                      \
        {                                                                       \
            if (!(MUST_BE_TRUE))                                                \
            {                                                                   \
                DIAG_LOG(SECTION,"Assertion failed:",LEVEL,STR1,#MUST_BE_TRUE,0);\
                DIAG_LOG(SECTION,STR,LEVEL,TYPE,X1,X2);                          \
            }                                                                   \
        } while(0)
    #define DIAG_MSG(msg_fmt)                                                   \
        diag_debug_message msg_fmt
#endif


extern int diag_debug_syslog_level_limit;

int diag_map_errlevel2syslog_level(int level);

void diag_debug_errorlog(xd_diag_section_t section, int internal_level, PFCCHAR string, int type, dword val1, dword val2);
void diag_debug_log(xd_diag_section_t section, int internal_level, PFCCHAR string, int type, dword val1, dword val2);

void diag_say(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_hex(xd_diag_section_t section, int level, PFCCHAR comment, PFBYTE p, int len);
void diag_say_ip_addr(xd_diag_section_t section, int level, PFCCHAR comment, byte KS_FAR *addr);
void diag_say_ether_addr(xd_diag_section_t section, int level, PFCCHAR comment, byte KS_FAR *addr);
void diag_say_int(xd_diag_section_t section, int level, PFCCHAR comment, int val);
void diag_say_int2(xd_diag_section_t section, int level, PFCCHAR comment, int val1, int val2);
void diag_say_lint(xd_diag_section_t section, int level, PFCCHAR comment, long val1);
void diag_say_lint2(xd_diag_section_t section, int level, PFCCHAR comment, long val1, long val2);
void diag_say_dint(xd_diag_section_t section, int level, PFCCHAR comment, dword val1);
void diag_say_dint2(xd_diag_section_t section, int level, PFCCHAR comment, dword val1, dword val2);
void diag_say_str(xd_diag_section_t section, int level, PFCCHAR comment, PFCHAR val1);
void diag_say_str2(xd_diag_section_t section, int level, PFCCHAR comment, PFCHAR val1, PFCHAR val2);
void diag_say_ports(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_ports_tcp(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_ports_udp(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_ports_raw(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_ports_ping(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_rt_entry(xd_diag_section_t section, int level, PFCCHAR comment, int i, PROUTE prt);
void diag_say_rt(xd_diag_section_t section, int level, PFCCHAR comment);
void diag_say_arpcache_entry(xd_diag_section_t section, int level, PFCCHAR comment, int index);

#if (INCLUDE_SNMP)

void diag_asn1_output(xd_diag_section_t section, int level, PFCCHAR comment, struct asn1_object *obj);
void diag_snmpprint(xd_diag_section_t section, int level, PFCCHAR comment, PSNBENTRY bindl);
void diag_snmpprint_one(xd_diag_section_t section, int level, PFCCHAR comment, PSNBENTRY bindl);
void diag_snmpprint_val(xd_diag_section_t section, int level, PFCCHAR comment, PSNVAL val);
void diag_snmpprint_objid(xd_diag_section_t section, int level, PFCCHAR comment, POID objidp);

#endif



/* (user-replacable) routine which dispatches diagnostic messages to syslog/directio/etc.   */
int diag_debug_printf_write(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen);

/* output routines referenced by diag_debug_printf_write   */
int directdebug_printf_write(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen);






/* <internal use only>   */
#define __RTIP_TRACE(section, str, lin, f)                  \
  ___rtip_tracedo_say_screen(section, str, lin, f)

void  ___rtip_tracedo_say_screen(xd_diag_section_t section, const char *s1, int line, const char *fname);
/* </internal use only>   */

#define RTIP_TRACE(section, str)                            \
  __RTIP_TRACE(section, str, __LINE__, __FILE__)


#else       /* CFG_USE_DIAG_MSG */

#define DIRECTDEBUG_LINE(msg_set)
#define VDIRECTDEBUG_LINE(msg, va_arg_set)

#endif      /* CFG_USE_DIAG_MSG */

#ifdef __cplusplus
}
#endif


#if (INCLUDE_PROFILING)
/* Profiling package -
   This is an execution profiling package. It is used internally to
   profile code sections.
   Two macros: PROFILE_ENTER(TOKEN) and PROFILE_EXIT(TOKEN) are defined. If
   profiling is turned off they do nothing. If they are on they note the
   start time and end time for the section being profiled. The routine
   dump_profile() dumps the duration for each call. This profiling package
   does not do histogram analysis. It only logs the duretion of the last
   pass through the code being profiled.. so it must be used carefully.

   Currently the only platform that supports profiling is the NET186 board.
*/

struct timerecord
{
    dword start_micro;  /* Value of the 0 to 1000 clock at start */
    dword end_micro;  /* Value of the interrupt clock at start */
};

struct namerecord
{
    int token;
    char *name;
};


extern dword tick_every_10000_tenth_micros;

#define PROF_NOW (dword) (tick_every_10000_tenth_micros +  (dword) (INWORD((TMR2_COUNT))))

#define PROFILE_ENTER(TOKEN) profile_time_array[TOKEN].start_micro = PROF_NOW;
#define PROFILE_EXIT(TOKEN) profile_time_array[TOKEN].end_micro = PROF_NOW;

void dump_profile(void);

#else

#define PROFILE_ENTER(TOKEN)
#define PROFILE_EXIT(TOKEN)


#endif

/* Define ID's for regions baing profiled. Each token maps to an entry in
   the profile_time_array[] (see debug.c). Each token must also have an
   entry in the profile_name_array[] (see debug.c)
*/
#define PROF_TOKEN_IP_INTERPRET 0
#define PROF_N_RECORDS 1

#if (INCLUDE_PROFILING)

extern struct timerecord profile_time_array[PROF_N_RECORDS];

#endif


#endif  /* DEBUG */

