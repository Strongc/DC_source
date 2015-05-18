/*                                                                        */
/* VT100.H  - VT100 terminal header file                                  */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains declaration for VT100 terminal driver        */

#ifndef _VT100_H
#define _VT100_H

#include "rtip.h"

/* TBD - these need to be set properly   */
#define VT100_UP_ARROW      1
#define VT100_DOWN_ARROW    2
#define VT100_RIGHT_ARROW   3
#define VT100_LEFT_ARROW    4
#define VT100_ESC           27

    int vt_init_term(PFVOID param);
    int vt_gotoxy(int x, int y);
    int vt_wherex(void);
    int vt_wherey(void);    
    int vt_get_width(void);
    int vt_get_height(void);
    int vt_putc(char c);
    int vt_set_inverse(RTIP_BOOLEAN inverse);
    int vt_set_blink(RTIP_BOOLEAN blink);
    int vt_cls(void);    

#endif

