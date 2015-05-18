/*                                                                        */
/* PCVID.H  - terminal header file                                        */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains declaration for terminal modules             */

#ifndef __PCVID_H
#define __PCVID_H

/* PC Memory-mapped video stuff   */

typedef struct pcvid_text_attrib
{
    int     forecolor;
    int     backcolor;
    RTIP_BOOLEAN inverse;
    RTIP_BOOLEAN blink;
} TEXT_ATTRIB;

typedef TEXT_ATTRIB KS_FAR* PTEXTATTRIB;

typedef struct pcvid_context
{
    RTIP_BOOLEAN initialized;  /* used internally -- set to zero on startup */
    TEXT_ATTRIB cur_attrib;
    int cursor_x,cursor_y;
    int tabsize;
    int x1,y1,x2,y2; /* window coords, if applicable */

    /* WARNING!  These *must* be bytes.   */
    byte head;
    byte tail;
    int keybuff[256];

    RTIP_BOOLEAN color;
    byte attrib_byte;
    byte backcolor;
    byte inverse_on;
    RTIP_BOOLEAN linewrap_flag;

} PCVID_CONTEXT;

typedef PCVID_CONTEXT KS_FAR* PPCVID_CONTEXT;


int     pcvid_init_term(PFVOID param);
int     pcvid_gotoxy(int x, int y);
int     pcvid_set_tabsize(int tabsize);
int     pcvid_wherex(void);
int     pcvid_wherey(void); 
int     pcvid_get_width(void);
int     pcvid_get_height(void);
int     pcvid_putc(KS_CONSTANT char c);
char    pcvid_readc(int x, int y, PTEXTATTRIB attrib);
int     pcvid_set_forecolor(int color);
int     pcvid_set_backcolor(int color);
int     pcvid_set_inverse(RTIP_BOOLEAN inverse);
int     pcvid_set_blink(RTIP_BOOLEAN blink);
int     pcvid_cls(void);    
int     pcvid_refresh(void);
int     pcvid_kbhit(void);
int     pcvid_getch(void);
int     pcvid_set_cur_context(PPCVID_CONTEXT ctx);
PPCVID_CONTEXT pcvid_get_cur_context(void);
int     pcvid_scroll_up(void);
int     pcvid_scroll_down(void);
void    pcvid_savecurpos(void);
void    pcvid_restorecurpos(void);
void    pcvid_updatecursor(void);
void    pcvid_savecurattrib(void);
void    pcvid_restorecurattrib(void);
int     pcvid_cputs(PFCCHAR string);
int     pcvid_kbhit(void);
int     pcvid_getch(void);
int     pcvid_get_text_attrib(PTEXTATTRIB attrib);
int     pcvid_set_text_attrib(PTEXTATTRIB attrib);
void    pcvid_isr_putc(int location, char ch);
void    pcvid_close(void);
unsigned short KS_FAR *pcvid_video_address(void);
void    pcvid_set_video_mode(int mode);

#define PCVID_SCREEN_COLS     80
#define PCVID_SCREEN_ROWS     25
#define DEF_PCVID_TABSIZE     8
#define DEF_PCVID_FORECOLOR   7
#define DEF_PCVID_BACKCOLOR   1

#define PCVID_HOME            0xff
#define PCVID_UP_ARROW        0xf9
#define PCVID_PGUP            0xfb
#define PCVID_LEFT_ARROW      0xf7
#define PCVID_RIGHT_ARROW     0xf6
#define PCVID_DOWN_ARROW      0xf8
#define PCVID_PGDOWN          0xfa
#define PCVID_DELETE          0xfb


#endif /* __PCVID_H */

