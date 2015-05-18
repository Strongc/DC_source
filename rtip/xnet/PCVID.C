/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"
#include "rtip.h"
#include "pcvid.h"

#if (USE_PCVID_OUTPUT)

#if (defined(__TURBOC__) )
#include <bios.h>    /* comment out for borland 5.5 build __st__ */
#endif

#if (!USE_KEYSCAN )
#if (defined(__BORLANDC__) )
#include  <dos.h>
#include  <bios.h>
#include  <conio.h>
#endif
#endif

/* ********************************************************************   */
#if (EBSENV || USE_KEYSCAN)
word func_key_map(word k);

#endif

#if (defined(PEGRTIP))
extern RTIP_BOOLEAN hand_kb_to_peg;
#endif


#if (defined(__TURBOC__))
#define _NKEYBRD_READ           0x10    /* read next character from keyboard */
#define _NKEYBRD_READY          0x11    /* check for keystroke */
#define _NKEYBRD_SHIFTSTATUS    0x12    /* get current shift key status */
word bios_getkey(void);
#endif


#if (INCLUDE_VT100C)
#define CPCVID_ATTRIB_BYTE(ctx)     (ctx->attrib_byte)
#define CPCVID_BACKCOLOR(ctx)       (ctx->backcolor)
#define PCVID_INVERSE_ON(ctx)       (ctx->inverse_on)
#define PCVID_COLOR(ctx)            (ctx->color)
#define PCVID_TABSIZE(ctx)          (ctx->tabsize)
#define CTX() cur_pcvid_context
#endif

/* ********************************************************************   */
/* GLOBAL DATA                                                            */

#if (INCLUDE_VT100C)
RTIP_BOOLEAN pcvid_initialized=FALSE;
PFBYTE pvideo;
PCVID_CONTEXT default_pcvid_context;
PPCVID_CONTEXT cur_pcvid_context = &default_pcvid_context;
#endif /* INCLUDE_VT100C */

#if (!INCLUDE_VT100C)
int KS_FAR cursor_x;
int KS_FAR cursor_y;
PFBYTE pvideo;
RTIP_BOOLEAN KS_FAR linewrap_flag;
#endif

/* ********************************************************************   */
/* INTERNAL FUNCTION DECLARATIONS                                         */

/* Internal functions for color/mono PC video   */

#define CPCVID_MEM pvideo
#define PC_VIDEO_INT(x) _int86(0x10,x,x)


/*
 * [i_a] added flag to indicate if we have VGA/EGA/CGA or HGC video hardware
 * in this machine.
 *
 * 0: VGA/EGA/CGA
 * 1: HGC
 *
 * use function 'pcvid_set_video_mode()' to set the mode if you must. DEFAULT
 * is 0:VGA.
 */
int pcvid_mode = 0; /* 0: VGA/EGA/CGA, 1: HGC */
                                               

/* ********************************************************************   */
/* !VT100C                                                                */
/* ********************************************************************   */
#if (!INCLUDE_VT100C)
/* 
*
* The next 100 lines or so implement pcvid_putc() for systems that include
* pc video output but are not using the vt100c.c calls to implement a 
* VT100 terminal driver on the PC. These routines make it possible for
* the calls in terminal.c to send characters to the video screen without
* incurring the rom overhead required for the full pcvid output package.
*
*/

int pcvid_init_term(PFVOID param)
{
PFBYTE pto;
int i,j;

    ARGSUSED_PVOID(param) 

    if (!pvideo)
    {
        pvideo = (PFBYTE) pcvid_video_address();
        cursor_x = cursor_y = 0;
        linewrap_flag = FALSE;
        pto = pvideo;
        for (i = 0; i < 24; i++)
            for (j = 0; j < 79; j++)
            {
                *pto++ = ' ';
                *pto++ = 0x0f;
            }
    }
    
    return(0);
}

int pcvid_scroll_up(void)
{
int line,n;
PFBYTE pto;
PFBYTE pfr;

    pcvid_init_term(0);

    pto = pvideo;
    pfr = pvideo+160;
    for(line=0; line < 24; line++)
    {
        tc_movebytes(pto,pfr,160);
        pto += 160;
        pfr += 160;
    }
    for(n=0; n < 80; n++)
    {
        *pto++=' ';
        *pto++=0x0f;
    }
    return(0);
}

int pcvid_putc(KS_CONSTANT char c)
{
PFBYTE p;

    pcvid_init_term(0);
    if(linewrap_flag && c!='\r' && c!='\n')
    {
        cursor_x=1;
        if(cursor_y<24)
        {
            cursor_y++;
        }
        else
        {
            pcvid_scroll_up();
        }
    }

    linewrap_flag=FALSE;

    if(c!='\t'&&c!='\b'&&c!='\n'&&c!='\r')
    {
        p = pvideo + (cursor_y*(PCVID_SCREEN_COLS*2)) + (cursor_x*2);
        *p++ = c;
        *p = 0x07;  /* white on black */
        cursor_x++;
    }
    else if(c=='\b')
    {
        if(cursor_x>0)
        {
            cursor_x--;
            p = pvideo + (cursor_y*(PCVID_SCREEN_COLS*2)) + (cursor_x*2);
            *p++ = ' ';
            *p = 0x07;  /* white on black */
        }
    } 
    else if(c=='\t')
    {
        cursor_x--;
        cursor_x+=8;
        cursor_x/=8;
        cursor_x*=8;
        cursor_x++;
    } 
    else if(c=='\r')
    {
        cursor_x=1;
    } 
    else if(c=='\n')
    {
        if(cursor_y<24)
        {
            cursor_y++;
        }
        else
        {
            pcvid_scroll_up();
        }
    }
    
    if(cursor_x>79)
    {
        cursor_x=0;
        linewrap_flag=TRUE;
    }

#if (!defined (DJGPP) )
    {
        union _REGS regs;
    
        /* put cursor at appropriate position   */
        regs.h.ah=2;
        regs.h.dl=(byte)cursor_x;
        regs.h.dh=(byte)cursor_y;
        regs.h.bh=0;
        _int86(0x10,&regs,&regs);
    }
#elif (AT_MOTHERBOARD && (IS_MS_PM || IS_BCC_PM || IS_WC_PM || IS_HC_PM))
    /*
     * [i_a] added code to update cursor position for any VGA/EGA/CGA or HGC
     * card IFF we can access the video hardware registers using I/O 
     * instructions. Maybe the condition above is a bit too restrictive, but
     * it works for them :-)
     */
    {
        unsigned int pos = cursor_y * PCVID_SCREEN_COLS + cursor_x;
                     
        if (pcvid_mode == 0)
        {             
            /*
                3d4h index  Eh (R/W):  CRTC: Cursor Location High Register
                bit 0-7  Upper 8 bits of the address of the cursor
                
                3d4h index  Fh (R/W):  CRTC: Cursor Location Low Register
                bit 0-7  Lower 8 bits of the address of the cursor
             */   
            OUTBYTE(0x3D4,0x0E);        
            OUTBYTE(0x3D5,pos >> 8);      
            OUTBYTE(0x3D4,0x0F);        
            OUTBYTE(0x3D5,pos);      
        }
        else 
        {             
            /*
                3b4h index  Eh (R/W):  CRTC: Cursor Location High Register
                bit 0-7  Upper 8 bits of the address of the cursor
                
                3b4h index  Fh (R/W):  CRTC: Cursor Location Low Register
                bit 0-7  Lower 8 bits of the address of the cursor
             */   
            OUTBYTE(0x3B4,0x0E);        
            OUTBYTE(0x3B5,pos >> 8);      
            OUTBYTE(0x3B4,0x0F);        
            OUTBYTE(0x3B5,pos);      
        }
    }
#endif
    return(0);
}

#else   /* INCLUDE_VT100C */
/* ********************************************************************   */
/* VT100C                                                                 */
/* ********************************************************************   */

/* ********************************************************************   */
/* Ouput routines                                                         */
/* Internal functions for color PC video                                  */
/* ********************************************************************   */

int pcvid_init_term(PFVOID param)
{
    tc_memset(CTX(),0,sizeof(PCVID_CONTEXT));

    if(param)
    {
        CTX()->color = (RTIP_BOOLEAN)(*((PFINT)param));
    }
    else
    {
        CTX()->color = TRUE;
    }

    if(!CTX()->x1 && !CTX()->x2 && !CTX()->y1 && !CTX()->y2)
    {
        CTX()->x1=1;
        CTX()->y1=1;
        CTX()->x2=PCVID_SCREEN_COLS;
        CTX()->y2=PCVID_SCREEN_ROWS;        
    }

    PCVID_TABSIZE(CTX())=DEF_PCVID_TABSIZE;
    pcvid_set_forecolor(DEF_PCVID_FORECOLOR);
    pcvid_set_backcolor(DEF_PCVID_BACKCOLOR);
    pcvid_set_inverse(FALSE);
    pcvid_set_blink(FALSE);
    CTX()->initialized=TRUE;

    if(!pcvid_initialized)
    {
        pvideo=(PFBYTE)pcvid_video_address();

        pcvid_initialized=TRUE;
    }

    pcvid_cls();

    return(0);

}

int pcvid_gotoxy(int x, int y)
{
    if(x<1||x>CTX()->x2||y<1||y>CTX()->y2)
        return(-1);

    CTX()->cursor_x=x;
    CTX()->cursor_y=y;
    pcvid_updatecursor();

    CTX()->linewrap_flag=FALSE;

    return(0);
}


int pcvid_set_tabsize(int tabsize)
{
    if(tabsize<1||tabsize>24)
        return(-1);
    PCVID_TABSIZE(CTX())=tabsize;   
    return(0);
}

int pcvid_wherex(void)
{
    return(CTX()->cursor_x);
}

int pcvid_wherey(void)
{
    return(CTX()->cursor_y);
}

int pcvid_get_width(void)
{
    return(CTX()->x2-CTX()->x1+1);
}

int pcvid_get_height(void)
{
    return(CTX()->y2-CTX()->y1+1);
}

int pcvid_putc(KS_CONSTANT char c)
{
    if(CTX()->linewrap_flag && c!='\r' && c!='\n')
    {
        CTX()->cursor_x=1;
        if(CTX()->cursor_y<(CTX()->y2-CTX()->y1+1))
        {
            CTX()->cursor_y++;
        }
        else
        {
            pcvid_scroll_up();
        }
    }

    CTX()->linewrap_flag=FALSE;

    if(c!='\t'&&c!='\b'&&c!='\n'&&c!='\r')
    {
        if(PCVID_COLOR(CTX()))
        {
            CPCVID_MEM[((CTX()->y1+CTX()->cursor_y-2)*PCVID_SCREEN_COLS
                +(CTX()->x1+CTX()->cursor_x-2))*2]=c;
            CPCVID_MEM[((CTX()->y1+CTX()->cursor_y-2)*PCVID_SCREEN_COLS
                +(CTX()->x1+CTX()->cursor_x-2))*2+1]=CPCVID_ATTRIB_BYTE(CTX());
        }
        else
        {
            /* tbd   */
        }
        CTX()->cursor_x++;
    }
    else if(c=='\b')
    {
        if(CTX()->cursor_x>1)
        {
            CTX()->cursor_x--;
            if(PCVID_COLOR(CTX()))
            {
                CPCVID_MEM[((CTX()->y1+CTX()->cursor_y-2)*PCVID_SCREEN_COLS
                    +(CTX()->x1+CTX()->cursor_x-2))*2]=' ';
                CPCVID_MEM[((CTX()->y1+CTX()->cursor_y-2)*PCVID_SCREEN_COLS
                    +(CTX()->x1+CTX()->cursor_x-2))*2+1]=CPCVID_ATTRIB_BYTE(CTX());
            }
            else
            {
                /* tbd   */
            }
        }
    } 
    else if(c=='\t')
    {
        CTX()->cursor_x--;
        CTX()->cursor_x+=PCVID_TABSIZE(CTX());
        CTX()->cursor_x/=PCVID_TABSIZE(CTX());
        CTX()->cursor_x*=PCVID_TABSIZE(CTX());
        CTX()->cursor_x++;
    } 
    else if(c=='\r')
    {
        CTX()->cursor_x=1;
    } 
    else if(c=='\n')
    {
        if(CTX()->cursor_y<(CTX()->y2-CTX()->y1+1))
        {
            CTX()->cursor_y++;
        }
        else
        {
            pcvid_scroll_up();
        }
    }
    
    if(CTX()->cursor_x>(CTX()->x2-CTX()->x1+1))
    {
        CTX()->cursor_x=CTX()->x2-CTX()->x1+1;
        CTX()->linewrap_flag=TRUE;
    }

    pcvid_updatecursor();
    return(0);
}

char pcvid_readc(int x, int y,PTEXTATTRIB attrib)
{
char c;
byte attrib_byte;
int temp;

    if(PCVID_COLOR(CTX()))
    {
        c=CPCVID_MEM[((CTX()->y1+y-2)*PCVID_SCREEN_COLS
            +(CTX()->x1+x-2))*2];
        attrib_byte=CPCVID_MEM[((CTX()->y1+y-2)*PCVID_SCREEN_COLS
            +(CTX()->x1+x-2))*2+1];
        attrib->forecolor = attrib_byte&0x0f;
        attrib->backcolor = (attrib_byte&0x70)>>4;
        attrib->blink = (RTIP_BOOLEAN)((attrib_byte&0x80)>>7);
        attrib->inverse = (RTIP_BOOLEAN)(PCVID_INVERSE_ON(CTX()));
        if(PCVID_INVERSE_ON(CTX()))
        {
            temp=attrib->forecolor;
            attrib->forecolor=attrib->backcolor;
            attrib->backcolor=temp;
        }
        return(c);
    }
    else
    {
        return(-1); /* tbd */
    }
}

int pcvid_set_forecolor(int color)
{
    if(PCVID_COLOR(CTX()))
    {
        if(color<0||color>15)
            return(-1);
        CPCVID_ATTRIB_BYTE(CTX()) &= 0xf0;
        CPCVID_ATTRIB_BYTE(CTX()) |= (byte)color;           
    }
    else
    {
        return(-1);
    }
    return(0);
}

int pcvid_set_backcolor(int color)
{
    if(PCVID_COLOR(CTX()))
    {
        if(color<0||color>15)
            return(-1);
        CPCVID_BACKCOLOR(CTX()) = (byte)color;
        CPCVID_ATTRIB_BYTE(CTX()) &= 0x8f;
        CPCVID_ATTRIB_BYTE(CTX()) |= (byte)((color%8)<<4);          
    }
    else
    {
        return(-1);
    }
    return(0);
}


int pcvid_set_inverse(RTIP_BOOLEAN inverse)
{
int forecolor;

    if(inverse==PCVID_INVERSE_ON(CTX()))
        return(0);

    if(PCVID_COLOR(CTX()))
    {
        forecolor=CPCVID_ATTRIB_BYTE(CTX())&0x0f;
        pcvid_set_forecolor(CPCVID_BACKCOLOR(CTX()));
        pcvid_set_backcolor(forecolor);     
    }
    else
    {
        /* tbd   */
    }

    PCVID_INVERSE_ON(CTX())=(byte)inverse;

    return(0);
}

int pcvid_set_blink(RTIP_BOOLEAN blink)
{
    if(PCVID_COLOR(CTX()))
    {
        blink = (RTIP_BOOLEAN)!(!blink);
        CPCVID_ATTRIB_BYTE(CTX()) &= 0x7f;
        CPCVID_ATTRIB_BYTE(CTX()) |= (byte)(blink<<7);
    }
    else
    {
        return(-1);
    }
    return(0);
}

int pcvid_cls(void)
{
int line,start,end,n;

    for(line=CTX()->y1;line<=CTX()->y2;line++)
    {
        start=((line-1)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2;
        end=start+(CTX()->x2-CTX()->x1+1)*2;
        for(n=start;n<end;n+=2)
        {
            CPCVID_MEM[n]=' ';
            CPCVID_MEM[n+1]=CPCVID_ATTRIB_BYTE(CTX());
        }
    }

    pcvid_gotoxy(1,1);
    return(0);
}

int pcvid_refresh(void)
{
    if(!pcvid_initialized||!CTX()->initialized)
        return(0);
    pcvid_updatecursor();
    return(0);
}

int pcvid_scroll_up(void)
{
int line,n,start,end;

    if(PCVID_COLOR(CTX()))
    {
        for(line=CTX()->y1+1;line<=CTX()->y2;line++)
        {
            tc_movebytes(
                &(CPCVID_MEM[((line-2)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2]),
                &(CPCVID_MEM[((line-1)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2]),
                (CTX()->x2-CTX()->x1+1)*2);
        }
        start=((CTX()->y2-1)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2;
        end=start+(CTX()->x2-CTX()->x1+1)*2;
        for(n=start;n<end;n+=2)
        {
            CPCVID_MEM[n]=' ';
            CPCVID_MEM[n+1]=CPCVID_ATTRIB_BYTE(CTX());
        }
    }
    else
    {
    }
    return(0);
}

int pcvid_scroll_down(void)
{
int line,n,start,end;

    if(PCVID_COLOR(CTX()))
    {
        for(line=CTX()->y2-1;line>=CTX()->y1;line--)
        {
            tc_movebytes(
                &(CPCVID_MEM[((line)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2]),
                &(CPCVID_MEM[((line-1)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2]),
                (CTX()->x2-CTX()->x1+1)*2);
        }
        start=((CTX()->y1-1)*PCVID_SCREEN_COLS+(CTX()->x1-1))*2;
        end=start+(CTX()->x2-CTX()->x1+1)*2;
        for(n=start;n<end;n+=2)
        {
            CPCVID_MEM[n]=' ';
            CPCVID_MEM[n+1]=CPCVID_ATTRIB_BYTE(CTX());
        }
    }
    else
    {
    }
    return(0);
}

void pcvid_updatecursor(void)
{
    union _REGS regs;

    /* put cursor at appropriate position   */
    regs.h.ah=2;
    regs.h.dl=(byte)(CTX()->x1+CTX()->cursor_x-2);
    regs.h.dh=(byte)(CTX()->y1+CTX()->cursor_y-2);
    regs.h.bh=0;
    PC_VIDEO_INT(&regs);
}

int pcvid_cputs(PFCCHAR string)
{
    while(*string)
    {
        if(pcvid_putc(*string++)<0)
        {
            return(-1);
        }
    }
    return(0);
}

int pcvid_set_cur_context(PPCVID_CONTEXT ctx)
{
    if(!ctx)
        return(-1);

    cur_pcvid_context = ctx;

    return(pcvid_refresh());
}

PPCVID_CONTEXT pcvid_get_cur_context(void)
{
    return(cur_pcvid_context);
}

int pcvid_get_text_attrib(PTEXTATTRIB attrib)
{
    if(!attrib)
    {
        return(-1);
    }
    tc_movebytes(attrib,&(cur_pcvid_context->cur_attrib),sizeof(TEXT_ATTRIB));
    return(0);
}

int pcvid_set_text_attrib(PTEXTATTRIB attrib)
{
    if(!attrib)
    {
        return(-1);
    }
    tc_movebytes(&(cur_pcvid_context->cur_attrib),attrib,sizeof(TEXT_ATTRIB));
    return(0);
}

/* To be used for debugging inside ISRs   */
void pcvid_isr_putc(int location, char ch)
{
    ((PFWORD)pvideo)[location] = (word)(0x0800|(word)ch);
}


void pcvid_close(void)
{
    pcvid_set_backcolor(0);
    pcvid_cputs("\r\n");
}

#endif /* (!INCLUDE_VT100C) */

void pcvid_set_video_mode(int mode)
{
    switch (mode)
    {
    case 0:  /* VGA/EGA/CGA */
    case 1:  /* HGC */
        pcvid_mode = mode;
        pvideo = (PFBYTE)pcvid_video_address();
        break;        
    }
}    
    
unsigned short KS_FAR *pcvid_video_address(void)
{
word KS_FAR *answer;                                   

    phys_to_virtual((PFBYTE *)&answer, (unsigned long)(pcvid_mode == 0 ? 0xB8000UL : 0xB0000UL));

    return(answer);
}


#endif /* USE_PCVID_OUTPUT */

#if (USE_PCVID_INPUT)
#if (USE_KEYSCAN)

/* ********************************************************************   */
#define KEYBOARD_INT        0x01
#define KEY_BUFFER          0x60
#define KEY_CONTROL         0x61
#define INT_CONTROL         0x20

void InstallKeyHandler(void);
static void insert_ascii(word c);

static word KeyCode=0;
byte   keyscan_on=0;
static byte upper_case=0;
static byte e0_flag=0;
static byte caps_lock=0;
static byte num_lock=1;
static byte shift_down=0;
static byte control_down=0;
static byte alt_down=0;
static byte alt_keys_pressed=0;
static byte alt_ascii_code=0;
static byte al;
byte BufferReadPos = 0;
volatile byte BufferWritePos = 0;
word KeyBuffer[0x100];
word KeyToAscii[] = 
{ 
    0,      27,     '1',    '2',    '3',    '4',    '5',    '6',    
    '7',    '8',    '9',    '0',    '-',    '=',    '\b',   '\t',   
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',    
    'o',    'p',    '[',    ']',    '\r',   0,      'a',    's',    
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',    
    '\'',   '`',    0,      '\\',   'z',    'x',    'c',    'v',    
    'b',    'n',    'm',    ',',    '.',    '/',    0,      '*',    
    0,      ' ',    '\0',   '\0',   '\0',   '\0',   '\0',   '\0',   
    '\0',   '\0',   '\0',   '\0',   '\0',   '\0',   '\0',   0x4700, 
    0x4800, 0x4900, '-',    0x4b00, 0x4c00, 0x4d00, '+',    0x4f00, 
    0x5000, 0x5100, '0',    '.',    '\0',   '\0',   '\0',   '\0',   
    '\0',   
    
    0,      27,     '!',    '@',    '#',    '$',    '%',    '^',    
    '&',    '*',    '(',    ')',    '_',    '+',    '\b',   '\t',   
    'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',    
    'O',    'P',    '{',    '}',    '\r',   0,      'A',    'S',    
    'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',    
    '\"',   '~',    0,      '|',    'Z',    'X',    'C',    'V',    
    'B',    'N',    'M',    '<',    '>',    '?',    0,      '*',    
    0,      ' ',    '\0',   '\0',   '\0',   '\0',   '\0',   '\0',   
    '\0',   '\0',   '\0',   '\0',   '\0',   '\0',   '\0',   '7',    
    '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',    
    '2',    '3',    '0',    '.',    '\0',   '\0',   '\0',   '\0',   
    '\0'    
};                                              
#endif /* USE_KEYSCAN */

/* ********************************************************************   */
/* Input routines                                                         */
/* ********************************************************************   */
int pcvid_kbhit(void)
{
#if SMXVERSION >= 0x0340
dword smx_keyd;
#endif

#if (USE_KEYSCAN)
    if (keyscan_on == 0)
        InstallKeyHandler();
    return((int)(BufferReadPos != BufferWritePos));

#elif (defined(__TURBOC__) )
{
    int t;
    t = _bios_keybrd(_NKEYBRD_READY);
    return(t);
}
#else
    int yes=0;
    __asm 
    {
        mov ah, 0x01
        int 0x16
        jz a
        mov yes, 1
a:      nop
    }
    return(yes);
#endif
}

int pcvid_getch(void)
{                           
    int key;
    
#if (USE_KEYSCAN)
    if (keyscan_on == 0)
        InstallKeyHandler();
    
    while (BufferReadPos == BufferWritePos)
    {
        ks_sleep(0);
    }

    key = KeyBuffer[BufferReadPos++];        

    switch(key) 
    {
    case 0x4700:
        key = PCVID_HOME;
        break;
    case 0x4800:
        key = PCVID_UP_ARROW;
        break;
    case 0x4900:
        key = PCVID_PGUP;
        break;
    case 0x4b00:
        key = PCVID_LEFT_ARROW;
        break;
    case 0x4d00:
        key = PCVID_RIGHT_ARROW;
        break;
    case 0x5000:
        key = PCVID_DOWN_ARROW;
        break;
    case 0x5100:
        key = PCVID_PGDOWN;
        break;
    case 0x3b00:
        key = PCVID_DELETE;
        break;
    default:
        key = key & 0xff;
        break;
    }
#else /* !USE_KEYSCAN */

    __asm 
    {
        mov ah, 0x0
        int 0x16
        mov key, ax
    }
    switch(key) 
    {
    case 0x4700:
        key = PCVID_HOME;
        break;
    case 0x4800:
        key = PCVID_UP_ARROW;
        break;
    case 0x4900:
        key = PCVID_PGUP;
        break;
    case 0x4b00:
        key = PCVID_LEFT_ARROW;
        break;
    case 0x4d00:
        key = PCVID_RIGHT_ARROW;
        break;
    case 0x5000:
        key = PCVID_DOWN_ARROW;
        break;
    case 0x5100:
        key = PCVID_PGDOWN;
        break;
    case 0x3b00:
        key = PCVID_DELETE;
        break;
    default:
        key &= 0xff;
        break;
    }
#endif /* USE_KEYSCAN */

    return(key);    
}
#endif      /* USE_PCVID_INPUT */

#if (USE_KEYSCAN)
/* ********************************************************************   */
/* KEYSCAN                                                                */
/* ********************************************************************   */
void insert_ascii(word c)
{
    if (((BufferWritePos+1)) != BufferReadPos)
    {
        KeyBuffer[BufferWritePos++] = c;
    }
}

void PegKeyscanISR(void);
void KeyscanISR(void)
{

#if (defined(PEGRTIP))
    if (hand_kb_to_peg)
    {
        PegKeyscanISR();
        return;
    }
#endif

    KeyCode = INBYTE(KEY_BUFFER);          
    al = INBYTE(KEY_CONTROL);              
    OUTBYTE(KEY_CONTROL,(al | 0x82));        
    OUTBYTE(KEY_CONTROL,(al & 0x7f));      
    
    if (e0_flag == 1) 
    {
        e0_flag = 0;
        switch (KeyCode)
        {
        case 0x1d: 
            control_down |= 1; 
            break;
            
        case 0x9d:
            control_down &= ~1;
            break;
            
        case 0x38:
            alt_down |= 1;
            break;
            
        case 0xb8:
            alt_down &= ~1;
            break;
            
        case 0x35:
            insert_ascii('/');
            break;
            
        case 0x1c:
            insert_ascii('\r');
            break;

        default:
            if (KeyCode < 0x80 && KeyCode != 0x2a) 
            { 
                insert_ascii((word)(KeyCode<<8)); 
            }
            break;
        }
    }
    else
    {
        switch (KeyCode)
        {
        case 0x3a:
            caps_lock = (byte)!caps_lock;
            break;

        case 0x45:
            num_lock = (byte)!num_lock;
            break;

        case 0x2a:
            shift_down |= 1;
            break;

        case 0x36:
            shift_down |= 2;
            break;

        case 0xaa:
            shift_down &= ~1;
            break;

        case 0xb6:
            shift_down &= ~2;
            break;

        case 0x1d:
            control_down |= 2;
            break;

        case 0x9d:
            control_down &= ~2;
            break;

        case 0x38:
            alt_down |= 2;
            break;

        case 0xb8:
            alt_down &= ~2;  
            break;

        default:
            break;
        }
        if ( (KeyCode != 0xe0) && (KeyCode != 0xe1) && (KeyCode <= 0x58) )
        {
            if (((((shift_down != 0) && (caps_lock == 0)) 
                  || ((shift_down == 0) && (caps_lock != 0))) 
                 && (KeyCode < 0x47))
                || ((num_lock != 0) && (KeyCode >= 0x47)))
                upper_case = 1;
            else
                upper_case = 0;

            if (KeyToAscii[KeyCode] != 0) 
            {
                if (alt_down != 0)
                {
                    if ((KeyCode > 30) && (KeyToAscii[KeyCode] >= '0') && (KeyToAscii[KeyCode] <= '9'))
                    {
                        alt_keys_pressed++;
                        alt_ascii_code = (byte)(alt_ascii_code * 10);
                        alt_ascii_code = (byte)(alt_ascii_code + 
                                                (KeyToAscii[KeyCode]-'0'));
                    }
                }
                else
                {
                    upper_case = (byte)(upper_case * 0x59);      
                    upper_case = (byte)KeyToAscii[KeyCode + upper_case];
                    
                    if ((control_down != 0)
                         && (upper_case >= 'a' || 'z' >= upper_case))
                    {                                      
                        /* Ctrl-A ... Ctrl-Z   */
                        insert_ascii((word)(upper_case - 'a' + 1));
                    }                       
                    else if ((control_down != 0)
                         && (upper_case >= 'A' || 'Z' >= upper_case))
                    {                                      
                        /* Ctrl-A ... Ctrl-Z   */
                        insert_ascii((byte)(upper_case - 'A' + 1));
                    }                       
                    else 
                    {                                      
                        /* key   */
                        insert_ascii(upper_case);
                    }                       
                }
            }
            else if ( (KeyCode >= 0x3b && KeyCode <= 0x44) 
                      || (KeyCode == 0x57) || (KeyCode == 0x58))
            {
                if (KeyCode > 0x56)
                    upper_case = (byte)(upper_case * 2 + 0x2e);
                else
                    upper_case = (byte)(upper_case * 25);
                insert_ascii((word)((KeyCode + upper_case)<<8));
            }
        }
    }

    if (KeyCode == 0xe0)
    {
        e0_flag = 1;        
    }
    
    if ((alt_keys_pressed == 3) || ((alt_keys_pressed > 0) && (alt_down == 0)))
    {
        insert_ascii((word)alt_ascii_code);
        alt_keys_pressed = 0;
        alt_ascii_code = 0;
    }

    return;
}

void InstallKeyHandler(void)
{
    if (keyscan_on)
        return;

    /* Reset the keyboard controller   */
    INBYTE(KEY_BUFFER);          
    INBYTE(KEY_CONTROL);              
    OUTBYTE(KEY_CONTROL,(al | 0x82));        
    OUTBYTE(KEY_CONTROL,(al & 0x7f));      
    ks_hook_interrupt(KEYBOARD_INT, (PFVOID) 0,(RTIPINTFN_POINTER)KeyscanISR,
                      (RTIPINTFN_POINTER) 0,0);
    keyscan_on = 1;
}

#endif /* USE_KEYSCAN */


