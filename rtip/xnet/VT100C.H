/*                                                                        */
/* VT100C.H  - VT100 terminal header file                                 */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains declarations for VT100 terminal driver       */

#ifndef _VT100C_H
#define _VT100C_H

#include "rtip.h"

#if (INCLUDE_VT100C)

#include "pcvid.h"

#define VT100C_CMDBUFSIZ    20

/* error codes for vt100c_update()   */
#define VT100C_INVALIDESC           1   /* invalid escape code */
#define VT100C_UNSUPPORTEDESC       2   /* unsupported escape code */
#define VT100C_UNSUPPORTEDPARAM     3   /* unsupported parameter in escape code */
#define VT100C_BADPARAM             4   /* bad parameter in escape code */

/* This structure is used by the VT100/ANSI client   */
struct _vt100c_context
{
    /* stream processing mode   */
    byte mode;
    /* command buffer   */
    char cmdbuf[VT100C_CMDBUFSIZ];
    int  icmdbuf;
    /* keeps tracks of which columns have tabs set   */
    byte tab_table[PCVID_SCREEN_COLS];
    byte application_keypad;
    byte scroll_in_region;
    struct pcvid_context scrolling_term;
    struct pcvid_context KS_FAR *main_ctx;
};

/* VT100/ANSI Client functions   */
int  vt100c_init(void);
int  vt100c_close(void);
int  vt100c_update(char c);
int  vt100c_getch(void);
int  vt100_erase_cur_to_endl(void);
int  vt100_erase_line_to_cur(void);
int  vt100_erase_cur_line(void);
int  vt100_erase_cur_to_ends(void);
void vt100_savecurpos(void);
void vt100_savecurattrib(void);
void vt100_restorecurpos(void);
void vt100_restorecurattrib(void);

#endif /* INCLUDE_VT100C */
#endif /* _VT100C_H */
