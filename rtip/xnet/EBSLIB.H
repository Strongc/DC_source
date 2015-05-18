/*                                                                        */
/* EBSLIB.H  - EBS header file                                            */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains prototypes and declarations for library      */
/*        routines.                                                       */
/*                                                                        */

#ifndef __EBSLIB__
#define __EBSLIB__ 1

#if (!KS_USE_ASCII_LIB || KS_USE_TC_PRINTF)
#include <stdarg.h>         /* needed for printf */
#endif

/* *********************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

#if (KS_USE_ASCII_LIB)
/* *********************************************************************   */
/* ******           Maros for use of standard ANSII library ************   */
/* *********************************************************************   */
#include <string.h>
#if (!KS_USE_TC_PRINTF)
#include <stdio.h>  /* for sprintf.c */
#endif
#include <ctype.h>  /* for isdigit.c && isprintf() */
#include <stdlib.h> /* for itoa, etc. */

#define tc_strcpy(A,B)      strcpy((A),(B))
#define tc_strlen(A)        strlen((A))
#define tc_strcat(A,B)      strcat((A),(B))
#if (!KS_USE_TC_PRINTF)
#define tc_sprintf          sprintf
#define tc_vsprintf         vsprintf
#define tc_snprintf         snprintf
#define tc_vsnprintf        vsnprintf
#endif

#define tc_strncpy(A,B,C)   strncpy((A),(B),(C))
#define tc_memmove(A,B,C)   memmove((A),(B),(C))
#define tc_memchr(str,chr,len)  memchr(str, chr, len)
#define tc_strncat(A,B,C)   strncat((A),(B),(C))
#define tc_strcmp(A,B)      strcmp((A),(B))
#define tc_strncmp(A,B,C)   strncmp((A),(B),(C))
/*#define tc_stricmp(A,B)       stricmp((A),(B))   */
#define tc_strspn           strspn
#define tc_strcspn          strcspn 
/*#define tc_strnicmp(A,B,C)    strnicmp((A),(B),(C))   */
#define tc_strstr(A,B)      strstr((A),(B))
#define tc_strchr(A,B)      strchr((A),(B))
#define tc_strrchr(A,B)     strrchr((A),(B))
/*#define tc_stristr            stristr   */
/* #if (!TARGET_186ES)                    */
#define tc_toupper(A)       toupper((A))
#define tc_isupper(A)       isupper((A))
#define tc_tolower(A)       tolower((A))
#define tc_isprint(A)       isprint((A))
#define tc_iscntrl(A)       iscntrl((A))
#define tc_isdigit(A)       isdigit((A))
#define tc_isspace(A)       isspace((A))
#endif

/* strequ - return TRUE if strings x and y are the same           */
#define tc_strequ(x,y) (tc_strcmp((x), (y)) == 0)

/* table maps nibble to HEX character   */
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR tc_hmap[17];

/* *********************************************************************   */
/* ******                       PRINTF                      ************   */
/* *********************************************************************   */

#if (KS_USE_TC_PRINTF)

struct tc_printf_io
{
  int current_x;
  int current_y;

  union
  {              
    struct
    {
      char *output_buffer;
      int buffer_size;
      int bufpos;
    } str;     
    
    int handle;
    
    struct
    {
      int facility_and_priority;
      int option_and_mask;
      int option_or_mask;

      PSYSTEM_USER user;     /* = get_system_user(); */
      int taskindex;         /* = ks_get_task_index(); */
    } syslog;
    
    void *other;
  } channel;
    
  int (*write)(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen);
                    
  union
  {               
    int ivalue;
    unsigned long int value;
    void *ptr;
  } propagate;
};

int tc_zprintf(struct tc_printf_io *io, KS_CONSTANT char * f, ...);
int tc_vzprintf(struct tc_printf_io *io, KS_CONSTANT char * f, va_list ap);

int tc_snprintf(PFCHAR wh, int bufflen, KS_CONSTANT char * f, ...);
int tc_vsnprintf(PFCHAR wh, int bufflen, KS_CONSTANT char * f, va_list ap);

int tc_sprintf(PFCHAR wh, KS_CONSTANT char * f, ...);
int tc_vsprintf(PFCHAR wh, KS_CONSTANT char * f, va_list ap);

#endif /* KS_USE_TC_PRINTF */

/* *********************************************************************   */
/* STRING DECLARATIONS                                                     */
/* *********************************************************************   */
#ifndef tc_strlen
int       tc_strlen(PFCCHAR string);
#endif
#ifndef tc_strcpy
PFCHAR    tc_strcpy(PFCHAR targ, PFCCHAR src);
#endif
#ifndef tc_strcat
PFCHAR    tc_strcat(PFCHAR targ, PFCCHAR src);
#endif
#ifndef tc_strncat
PFCHAR    tc_strncat(PFCHAR targ, PFCCHAR src, KS_CONSTANT int n);
#endif
#ifndef tc_strncpy
int       tc_strncpy(PFCHAR targ, PFCCHAR src, KS_CONSTANT int n);
#endif
#ifndef tc_strcmp
int       tc_strcmp(PFCCHAR s1, PFCCHAR s2);
#endif
#ifndef tc_stricmp
int       tc_stricmp(PFCCHAR s1, PFCCHAR s2);
#endif
#ifndef tc_strncmp
int       tc_strncmp(PFCCHAR s1, PFCCHAR s2, KS_CONSTANT int n);
#endif
#ifndef tc_strnicmp
int       tc_strnicmp(PFCCHAR s1, PFCCHAR s2, KS_CONSTANT int n);
#endif
#ifndef tc_stristr
PFCHAR tc_stristr(PFCHAR str, PFCCHAR find_str);
#endif
#ifndef tc_strstr
PFCHAR    tc_strstr(PFCCHAR str, PFCCHAR find_str);
#endif
#ifndef tc_strchr
PFCHAR    tc_strchr(PFCCHAR str, KS_CONSTANT char find_chr);
#endif
#ifndef tc_isdigit
int       tc_isdigit(int ch);
#endif
#ifndef tc_isprint
int       tc_isprint(int ch);
#endif
#ifndef tc_isspace
int       tc_isspace(int ch);
#endif
#ifndef tc_iscntrl
int       tc_iscntrl(int ch);
#endif
#if (INCLUDE_WEB) || (INCLUDE_SSL)
unsigned int tc_hatoi(PFCHAR s);
#endif
#ifndef tc_memmove
void      tc_memmove(PFVOID to, PFCVOID fr, int n);
#endif
#ifndef tc_memchr
PFVOID  tc_memchr(PFCBYTE str, int chr, int n);
#endif
#ifndef tc_atoi
int       tc_atoi(PFCHAR s);
#endif
#ifndef tc_atol
long      tc_atol(PFCHAR s);
#endif
#ifndef tc_ltoa
PFCHAR    tc_ltoa(long int num, PFCHAR dest, int  base);
#endif
#ifndef tc_itoa
PFCHAR    tc_itoa(int  num, PFCHAR dest, int  base);
#endif
#ifndef tc_ultoa
PFCHAR    tc_ultoa(unsigned long value, PFCHAR string, int radix);
#endif
#ifndef tc_atoi
int       tc_atoi(PFCHAR s);
#endif
#ifndef tc_atol
long      tc_atol(PFCHAR s);
#endif
#ifndef tc_ltoa
PFCHAR    tc_ltoa(long int num, PFCHAR dest, int  base);
#endif
#ifndef tc_itoa
PFCHAR    tc_itoa(int  num, PFCHAR dest, int  base);
#endif
#ifndef tc_ultoa
PFCHAR    tc_ultoa(unsigned long value, PFCHAR string, int radix);
#endif

/* *********************************************************************   */
#ifdef __cplusplus
}
#endif

#endif /* __EBSLIB__ */
