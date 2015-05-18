/*                                                                      */
/* VT100C.C - VT100/102 emulation functions                             */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */


#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"
#include "vt100c.h"
#include "vt100.h"
#include "pcvid.h"

#if (INCLUDE_VT100C)

/* *********************************************************************   */
struct _vt100c_context KS_FAR* vt100c_ctx = 0;
struct _vt100c_context KS_FAR vt100c_ctx_core;
static int KS_FAR saved_x;
static int KS_FAR saved_y;


TEXT_ATTRIB KS_FAR saved_attrib;

/* *********************************************************************   */
/* mode defines for VT100/ANSI stream processing                           */

#define VT100C_NORMAL               0
#define VT100C_ESCAPE               1
#define VT100C_POUND                3
#define VT100C_LEFTBRACKET          4
#define VT100C_PLAINCMD             5
#define VT100C_POUNDCMD             6
#define VT100C_QUESBRAK             8
#define VT100C_QUESBRAKCMD          10
#define VT100C_BRAKCMD              11
#define VT100C_BRAKCMDSET           12
#define VT100C_BRAKCMDRESET         13
#define VT100C_QUESBRAKSET          14
#define VT100C_QUESBRAKRESET        15
#define VT100C_BRAKCMDEND           16
#define VT100C_LEFTPAREN            17
#define VT100C_RIGHTPAREN           18
#define VT100C_LEFTPARENSPACE       19
#define VT100C_RIGHTPARENSPACE      20
#define VT100C_LEFTPARENCMD         21
#define VT100C_RIGHTPARENCMD        22
#define VT100C_K                    23
#define VT100C_KCMD                 25
#define VT100C_KCMDEND              26

/* *********************************************************************   */
/* internal functions for VT100/ANSI client                                */
static int vt100c_updatemode(char c);
static int vt100c_set_scroll_win(void);
static int vt100c_get_next_param(PFCHAR *pp);
static int vt100c_allattribsoff(void);
static int vt100c_curposreport(void);
static int vt100c_sends(PFCHAR str);

/* *********************************************************************   */
/* VT100/ANSI Client functions                                             */
/* *********************************************************************   */

int vt100c_init(void)
{
    if(vt100c_ctx)
        return(0);

    vt100c_ctx = &vt100c_ctx_core;

    /* tbd: connect, etc.   */
    tc_memset(vt100c_ctx,0,sizeof(struct _vt100c_context));

    vt100c_ctx->mode=VT100C_NORMAL;
    vt100c_ctx->main_ctx=pcvid_get_cur_context();

    pcvid_set_cur_context(&vt100c_ctx->scrolling_term);

    if(pcvid_init_term(0) < 0)
    {
        return(-1);
    }

    pcvid_set_cur_context(vt100c_ctx->main_ctx);

    if(pcvid_init_term(0) < 0)
    {
        return(-1);
    }
    return(0);
}

/* *********************************************************************   */
int vt100c_close(void)
{
    vt100c_ctx = 0;
    pcvid_close();
    return 0;
}

/* *********************************************************************   */
int vt100c_update(char c)
{
int n;
PFCHAR pfc;
int retval=0;
int width;
int x;
int tabcol;
int cur_x,cur_y;

    vt100c_init();

    vt100c_updatemode(c);
    /* process character based on existing mode   */
    if(vt100c_ctx->mode==VT100C_NORMAL)
    {
        /* tbd: don't hardcode pcvid                          */
/*      pcvid_set_cur_context(&(vt100c_ctx->scrolling_term)); */

        if(c!='\t')
        {
            pcvid_putc(c);
        }
        else
        {
            width=pcvid_get_width();
            x=pcvid_wherex();
            for(n=x;n<width;n++)
            {
                if(vt100c_ctx->tab_table[n])
                {
                    tabcol=n+1;
                    for(n=x;n<tabcol;n++)
                        pcvid_putc(' ');
                    n=0;
                    break;
                }
            }
            /* If no tab found in table, let _putc() do the tab   */
            if(n)
                pcvid_putc('\t');
        }
    }
    if(vt100c_ctx->mode==VT100C_NORMAL)
    {
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_LEFTPARENCMD)
    {
#if (REALLY_SLOW)
        if(c=='A')
            retval=VT100C_UNSUPPORTEDPARAM; /* United Kingdom (UK) 
                                                        charset on */
        else if(c=='B');    /* United States (USASCII) charset on */
        else if(c=='0')
            retval=VT100C_UNSUPPORTEDPARAM; /* Special graphic/line drawing 
                                                        charset on */
        else if(c=='1')
            retval=VT100C_UNSUPPORTEDPARAM; /* Alternative character ROM on */
        else if(c=='2')
            retval=VT100C_UNSUPPORTEDPARAM; /* Alternative graphic ROM on */
        else
            retval=VT100C_BADPARAM;
#else
        retval=VT100C_UNSUPPORTEDPARAM;
#endif
    }
    if(vt100c_ctx->mode==VT100C_RIGHTPARENCMD)
    {
#if (REALLY_SLOW)
        if(c=='A')
            retval=VT100C_UNSUPPORTEDPARAM; /* United Kingdom (UK) 
                                                        charset off */
        else if(c=='B');    /* United States (USASCII) charset off */
        else if(c=='0')
            retval=VT100C_UNSUPPORTEDPARAM; /* Special graphic/line drawing 
                                                        charset off */
        else if(c=='1')
            retval=VT100C_UNSUPPORTEDPARAM; /* Alternative character ROM off */
        else if(c=='2')
            retval=VT100C_UNSUPPORTEDPARAM; /* Alternative graphic ROM off */
        else
            retval=VT100C_BADPARAM;
#else
        retval=VT100C_UNSUPPORTEDPARAM;
#endif
    }
    if(vt100c_ctx->mode==VT100C_PLAINCMD)
    {
        if(c=='D')
        {
            /* Index                                              */
/*          pcvid_set_cur_context(&(vt100c_ctx->scrolling_term)); */
            if(pcvid_wherex()==1 && pcvid_wherey()==pcvid_get_height())
            {
                pcvid_scroll_up();
            }
            else
            {
                DEBUG_ERROR("Bad Index",NOVAR,0,0);
            }
        }
        if(c=='M')
        {
            /* Reverse Index   */
            pcvid_set_cur_context(&(vt100c_ctx->scrolling_term));
            if(pcvid_wherex()==1 && pcvid_wherey()==1)
            {
                pcvid_scroll_down();
            }
            else
            {
                DEBUG_ERROR("Bad Reverse Index",NOVAR,0,0);
            }
        }
        if(c=='7')
        {
            /* Save cursor and attributes   */
            vt100_savecurpos();
            vt100_savecurattrib();
        }
        if(c=='8')
        {
            /* Restore cursor and attributes   */
            vt100_restorecurpos();
            vt100_restorecurattrib();
        }
        if(c=='H')
        {
            /* set tab at current column   */
            vt100c_ctx->tab_table[pcvid_wherex()-1]=1;
        }
        if(c=='c')
        {
            /* Power-up reset   */
            retval=VT100C_UNSUPPORTEDESC;
        }
    }
    if(vt100c_ctx->mode==VT100C_POUNDCMD)
    {
        if(c=='3')
        {
            /* Change this line to double-height top half   */
            return(VT100C_UNSUPPORTEDESC);
        }
        if(c=='4')
        {
            /* Change this line to double-height bottom half   */
            return(VT100C_UNSUPPORTEDESC);
        }
        if(c=='5')
        {
            /* Change this line to single-width single-height   */
            return(VT100C_UNSUPPORTEDESC);
        }
        if(c=='6')
        {
            /* Change this line to double-width single height   */
            return(VT100C_UNSUPPORTEDESC);
        }
        if(c=='8')
        {
            /* fill screen with E   */
            retval=VT100C_UNSUPPORTEDESC;
        }
    }
    if(vt100c_ctx->mode==VT100C_BRAKCMD||vt100c_ctx->mode==VT100C_KCMD)
    {
        if(vt100c_ctx->icmdbuf<VT100C_CMDBUFSIZ-1)
            vt100c_ctx->cmdbuf[vt100c_ctx->icmdbuf++]=c;
    }
    if(vt100c_ctx->mode==VT100C_QUESBRAKCMD)
    {
        vt100c_ctx->cmdbuf[0]=c;
    }
    /* is this the right code ??   */
    if(vt100c_ctx->mode==VT100C_KCMDEND)
    {
        vt100c_ctx->cmdbuf[vt100c_ctx->icmdbuf]='\0';       

        retval=vt100c_set_scroll_win();

        vt100c_ctx->icmdbuf=0;
    }
    if(vt100c_ctx->mode==VT100C_BRAKCMDEND)
    {
        vt100c_ctx->cmdbuf[vt100c_ctx->icmdbuf]='\0';

        if(c=='y')
        {
            if((pfc=tc_strchr(vt100c_ctx->cmdbuf,';'))!=0)
            {
                n=tc_atoi(pfc+1);
                if(n>0&&n<16)
                {
                    retval=VT100C_UNSUPPORTEDESC;
                }
                else
                {
                    retval=VT100C_INVALIDESC;
                }
            }
            else
            {
                retval=VT100C_INVALIDESC;
            }
        }
        if(c=='m'||c=='q')
        {
            if(vt100c_ctx->icmdbuf)
            {
                pfc=vt100c_ctx->cmdbuf;
                while((n=vt100c_get_next_param(&pfc))!=-1)
                {
                    if(c=='m')
                    {
                        if(n==0)
                        {
                            /* all attributes off   */
                            vt100c_allattribsoff();
                        }
                        else if(n==1)
                        {
                            /* bold on   */
                            pcvid_set_forecolor(15);
                        }
                        else if(n==4)
                        {
                            /* underscore on   */
                            retval=VT100C_UNSUPPORTEDPARAM;
                        }
                        else if(n==5)
                        {
                            /* blink on   */
                            pcvid_set_blink(TRUE);
                        }
                        else if(n==7)
                        {
                            /* inverse on   */
                            pcvid_set_inverse(TRUE);
                        }
                        else
                        {
                            /* bad parameter   */
                            retval=VT100C_BADPARAM;
                        }
                    }
                    if(c=='q')
                    {
                        if(n==0)
                            retval=VT100C_UNSUPPORTEDPARAM; /* all LEDs off */
                        else if(n==1)
                            retval=VT100C_UNSUPPORTEDPARAM; /* LED 1 on */
                        else if(n==2)
                            retval=VT100C_UNSUPPORTEDPARAM; /* LED 2 on */
                        else if(n==3)
                            retval=VT100C_UNSUPPORTEDPARAM; /* LED 3 on */
                        else if(n==4)
                            retval=VT100C_UNSUPPORTEDPARAM; /* LED 4 on */
                        else
                            retval=VT100C_BADPARAM;
                    }
                }
            }
            else
            {
                if(c=='m')
                {
                    vt100c_allattribsoff();
                }
                if(c=='q')
                {
                    retval=VT100C_UNSUPPORTEDPARAM; /* all LEDs off */
                }
            }
        }
        if(c=='H'||c=='f')
        {
            /* gotoxy   */

            pcvid_set_cur_context(&(vt100c_ctx->scrolling_term));

            if((pfc=tc_strchr(vt100c_ctx->cmdbuf,';'))!=0)
            {
                *pfc='\0';

                cur_x=tc_atoi(pfc+1);
                cur_y=tc_atoi(vt100c_ctx->cmdbuf);

                if(cur_x<1)
                    cur_x=1;
                if(cur_x>pcvid_get_width())
                    cur_x=pcvid_get_width();

                if(cur_y<1)
                    cur_y=1;
                if(cur_y>pcvid_get_height())
                    cur_y=pcvid_get_height();

                if(vt100c_ctx->scroll_in_region)
                    pcvid_gotoxy(cur_x,cur_y);
                else
                    pcvid_gotoxy(1+cur_x-vt100c_ctx->scrolling_term.x1,
                        1+cur_y-vt100c_ctx->scrolling_term.y1);
            }
            else
            {
                pcvid_gotoxy(1,1);
            }
        }
        if(c=='g')
        {
            if(!tc_strcmp(vt100c_ctx->cmdbuf,"")||!tc_strcmp(vt100c_ctx->cmdbuf,"0"))
            {
                /* clear tab at current column   */
                vt100c_ctx->tab_table[pcvid_wherex()-1]=0;
            }
            else if(!tc_strcmp(vt100c_ctx->cmdbuf,"3"))
            {
                /* clear all tabs   */
                for(n=0;n<PCVID_SCREEN_COLS;n++)
                    vt100c_ctx->tab_table[n]=0;
            }
            else
            {
                retval=VT100C_INVALIDESC;
            }
        }
        if(c=='n')
        {
            if(!tc_strcmp(vt100c_ctx->cmdbuf,"6"))
            {
                /* Cursor position report   */
                vt100c_curposreport();
            }
            else if(!tc_strcmp(vt100c_ctx->cmdbuf,"5"))
            {
                /* Status report   */
                vt100c_sends("\x27 [ c");
            }
            else
            {
                retval=VT100C_INVALIDESC;
            }
        }
        if(c=='c')
        {
            if(!tc_strcmp(vt100c_ctx->cmdbuf,"")||!tc_strcmp(vt100c_ctx->cmdbuf,"0"))
            {
                /* What are you?   */
                vt100c_sends("\x27 [?1;0 c");   /* base VT100, no options */
            }   
            else
            {
                retval=VT100C_INVALIDESC;
            }
        }
        if(c=='A'||c=='B'||c=='C'||c=='D')
        {
            vt100c_ctx->cmdbuf[vt100c_ctx->icmdbuf]='\0';
            n=tc_atoi(vt100c_ctx->cmdbuf);
            if(!n)
                n=1;

            pcvid_set_cur_context(&(vt100c_ctx->scrolling_term));

            if(c=='A')
                pcvid_gotoxy(pcvid_wherex(),pcvid_wherey()-n); /* cursor up */
            if(c=='B')
                pcvid_gotoxy(pcvid_wherex(),pcvid_wherey()+n); /* cursor down */
            if(c=='C')
                pcvid_gotoxy(pcvid_wherex()+n,pcvid_wherey()); /* cursor right */
            if(c=='D')
                pcvid_gotoxy(pcvid_wherex()-n,pcvid_wherey()); /* cursor left */
        }
        if(c=='J')
        {
            if(!tc_strcmp(vt100c_ctx->cmdbuf,"")||!tc_strcmp(vt100c_ctx->cmdbuf,"0"))
            {
                /* erase from cursor to end of screen   */
                vt100_erase_cur_to_ends();
            }
            else if(!tc_strcmp(vt100c_ctx->cmdbuf,"2"))
            {
                /* clear screen   */
                pcvid_cls();                
            }
            else
            {
                retval=VT100C_INVALIDESC;
            }
        }
        if(c=='K')
        {

            if(!tc_strcmp(vt100c_ctx->cmdbuf,"")||!tc_strcmp(vt100c_ctx->cmdbuf,"0"))
            {
                /* erase from cursor to end of line   */
                vt100_erase_cur_to_endl();
            }
            else if(!tc_strcmp(vt100c_ctx->cmdbuf,"1"))
            {
                /* erase from beginning of line to cursor   */
                vt100_erase_line_to_cur();
            }
            else if(!tc_strcmp(vt100c_ctx->cmdbuf,"2"))
            {
                /* erase line containing cursor   */
                vt100_erase_cur_line();
            }
            else
            {
                retval=VT100C_INVALIDESC;
            }
        }
        if(c=='r')
        {
            retval=vt100c_set_scroll_win();
        }

        vt100c_ctx->icmdbuf=0;
    }   
    if(vt100c_ctx->mode==VT100C_BRAKCMDSET||vt100c_ctx->mode==VT100C_BRAKCMDRESET)
    {
        vt100c_ctx->cmdbuf[vt100c_ctx->icmdbuf]='\0';
        vt100c_ctx->icmdbuf=0;

        if(!tc_strcmp(vt100c_ctx->cmdbuf,"20"))
        {
            if(vt100c_ctx->mode==VT100C_BRAKCMDSET)
                retval=VT100C_UNSUPPORTEDESC; /* Set Mode: Line feed/new */
            else
                retval=VT100C_UNSUPPORTEDESC; /* Reset Mode: Line feed/new */
        }
        else
        {
            retval=VT100C_INVALIDESC;
        }
    }
    if(vt100c_ctx->mode==VT100C_QUESBRAKSET)
    {
        if(vt100c_ctx->cmdbuf[0]=='1')  /* Set Mode: Application keypad */
        {
            vt100c_ctx->application_keypad=1;
        }
        else if(vt100c_ctx->cmdbuf[0]=='3')
            {retval=VT100C_UNSUPPORTEDESC;} /* Set Mode: Column Mode */
        else if(vt100c_ctx->cmdbuf[0]=='4')
            {retval=VT100C_UNSUPPORTEDESC;} /* Set Mode: Scrolling */
        else if(vt100c_ctx->cmdbuf[0]=='5')
            {retval=VT100C_UNSUPPORTEDESC;} /* Set Mode: Screen Mode */
        else if(vt100c_ctx->cmdbuf[0]=='6')
            {
                vt100c_ctx->scroll_in_region=1; /* Set Mode: Origin Mode */
            }
        else if(vt100c_ctx->cmdbuf[0]=='7')
            {retval=VT100C_UNSUPPORTEDESC;} /* Set Mode: Wraparound */
        else if(vt100c_ctx->cmdbuf[0]=='8')
            {retval=VT100C_UNSUPPORTEDESC;} /* Set Mode: Autorepeat */
        else if(vt100c_ctx->cmdbuf[0]=='9')
            {retval=VT100C_UNSUPPORTEDESC;} /* Set Mode: Interface */
        else
            {retval=VT100C_INVALIDESC;}
    }
    if(vt100c_ctx->mode==VT100C_QUESBRAKRESET)
    {
        if(vt100c_ctx->cmdbuf[0]=='1')  /* Reset Mode: Application keypad */
        {
            vt100c_ctx->application_keypad=0;
        }
        else if(vt100c_ctx->cmdbuf[0]=='2')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: ANSI/VT52 */
        else if(vt100c_ctx->cmdbuf[0]=='3')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: Column Mode */
        else if(vt100c_ctx->cmdbuf[0]=='4')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: Scrolling */
        else if(vt100c_ctx->cmdbuf[0]=='5')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: Screen Mode */
        else if(vt100c_ctx->cmdbuf[0]=='6')
            {vt100c_ctx->scroll_in_region=0;} /* Reset Mode: Origin Mode */
        else if(vt100c_ctx->cmdbuf[0]=='7')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: Wraparound */
        else if(vt100c_ctx->cmdbuf[0]=='8')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: Autorepeat */
        else if(vt100c_ctx->cmdbuf[0]=='9')
            {retval=VT100C_UNSUPPORTEDESC;} /* Reset Mode: Interface */
        else
            {retval=VT100C_INVALIDESC;}
    }
    return(retval);
}


/* *********************************************************************   */
static int vt100c_updatemode(char c)
{
    if(c==(char)27)
    {
        vt100c_ctx->mode=VT100C_ESCAPE;
        return(0);
    }
    /* Nothing more needs be done   */
    if(vt100c_ctx->mode==VT100C_NORMAL)
    {
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_ESCAPE)
    {
        if(c=='[')
            vt100c_ctx->mode=VT100C_LEFTBRACKET;
        else if(c=='#')
            vt100c_ctx->mode=VT100C_POUND;
        else if(c=='(')
            vt100c_ctx->mode=VT100C_LEFTPAREN;
        else if(c==')')
            vt100c_ctx->mode=VT100C_RIGHTPAREN;
        else if(c=='K')
            vt100c_ctx->mode=VT100C_K;
        else
            vt100c_ctx->mode=VT100C_PLAINCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_K)
    {
        vt100c_ctx->mode=VT100C_KCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_KCMD)
    {
        if(c=='r')
            vt100c_ctx->mode=VT100C_KCMDEND;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_KCMDEND)
    {
        vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_LEFTPAREN)
    {
        if(c==' ')
            vt100c_ctx->mode=VT100C_LEFTPARENSPACE;
        else
            vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_RIGHTPAREN)
    {
        if(c==' ')
            vt100c_ctx->mode=VT100C_RIGHTPARENSPACE;
        else
            vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_LEFTPARENSPACE)
    {
        vt100c_ctx->mode=VT100C_LEFTPARENCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_RIGHTPARENSPACE)
    {
        vt100c_ctx->mode=VT100C_RIGHTPARENCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_LEFTPARENCMD)
    {
        vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_RIGHTPARENCMD)
    {
        vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_POUND)
    {
        vt100c_ctx->mode=VT100C_POUNDCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_PLAINCMD||vt100c_ctx->mode==VT100C_POUNDCMD)
    {
        vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_LEFTBRACKET)
    {
        if(c=='?')
            vt100c_ctx->mode=VT100C_QUESBRAK;
        else if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))
            vt100c_ctx->mode=VT100C_BRAKCMDEND;
        else
            vt100c_ctx->mode=VT100C_BRAKCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_QUESBRAK)
    {
        vt100c_ctx->mode=VT100C_QUESBRAKCMD;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_BRAKCMD)
    {
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))
            vt100c_ctx->mode=VT100C_BRAKCMDEND;
        if(c=='h')
            vt100c_ctx->mode=VT100C_BRAKCMDSET;
        if(c=='l')
            vt100c_ctx->mode=VT100C_BRAKCMDRESET;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_BRAKCMDEND||vt100c_ctx->mode==VT100C_BRAKCMDSET
        ||vt100c_ctx->mode==VT100C_BRAKCMDRESET)
    {
        vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }   
    if(vt100c_ctx->mode==VT100C_QUESBRAKCMD)
    {
        if(c=='h')
            vt100c_ctx->mode=VT100C_QUESBRAKSET;
        else if(c=='l')
            vt100c_ctx->mode=VT100C_QUESBRAKRESET;
        else
            vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }
    if(vt100c_ctx->mode==VT100C_QUESBRAKSET||vt100c_ctx->mode==VT100C_QUESBRAKRESET)
    {
        vt100c_ctx->mode=VT100C_NORMAL;
        return(0);
    }

    return(0);
}

/* *********************************************************************   */
static int vt100c_set_scroll_win()
{
PFCHAR pfc;
int top,bottom;

    if((pfc=tc_strchr(vt100c_ctx->cmdbuf,';'))!=0)
    {
        *pfc='\0';
        /*_puts("Command: Set scrolling window; top=");   */
        /*_puts(vt100c_ctx->cmdbuf);                      */
        /*_puts(", bottom=");                             */
        /*_puts(pfc+1);                                   */
        /*_puts(".\n");                                   */
        top=tc_atoi(vt100c_ctx->cmdbuf);
        bottom=tc_atoi(pfc+1);
        pcvid_set_cur_context(vt100c_ctx->main_ctx);
        top+=(vt100c_ctx->main_ctx->y1-1);
        bottom+=(vt100c_ctx->main_ctx->y1-1);
        if(top<1 || bottom<top || bottom>pcvid_get_height())
        {
            pcvid_cputs("Not enough room in this town for the both of us.");
            pcvid_set_cur_context(&(vt100c_ctx->scrolling_term));
        }
        else
        {
            vt100c_ctx->scrolling_term.y1=top;
            vt100c_ctx->scrolling_term.y2=bottom;   
            pcvid_set_cur_context(&(vt100c_ctx->scrolling_term));
            pcvid_gotoxy(1,1);
        }

        return(0);
    }
    else
    {
        return(VT100C_INVALIDESC);
    }
}

/* *********************************************************************   */
static int vt100c_get_next_param(PFCHAR *pp)
{
PFCHAR p;
int retval;

    if(!**pp)
        return(-1);
    p=tc_strchr(*pp,';');
    if(p)
        *p='\0';
    retval=tc_atoi(*pp);
    if(p)
    {
        *p=';';
        (*pp)=p+1;
    }
    else
    {
        while(**pp)
            (*pp)++;        
    }
    return(retval);
}

/* *********************************************************************   */
static int vt100c_allattribsoff()
{
    pcvid_set_inverse(FALSE);
    pcvid_set_blink(FALSE);
    pcvid_set_forecolor(DEF_PCVID_FORECOLOR);
    return 0;
}

/* *********************************************************************   */
static int vt100c_curposreport()
{
char numbuf[30];
char outbuf[50];

    tc_strcpy(outbuf,"\x27 [ ");
    tc_itoa(pcvid_wherex(),numbuf,10);
    tc_strcat(outbuf,numbuf);
    tc_strcat(outbuf,";");
    tc_itoa(pcvid_wherey(),numbuf,10);
    tc_strcat(outbuf,numbuf);
    tc_strcat(outbuf,"R");
    vt100c_sends(outbuf);
    return(0);
}

/* *********************************************************************   */
static int vt100c_sends(PFCHAR str)
{
    /* tbd: send over network   */
    pcvid_cputs("\nVT100 client send: ");
    pcvid_cputs(str);
    pcvid_cputs("\n");
    return(0);
}

/* *********************************************************************   */
int vt100c_getch(void)
{
int key;

    vt100c_init();

    switch(key = pcvid_getch())
    {
    case PCVID_UP_ARROW:
        return(VT100_UP_ARROW);

    case PCVID_DOWN_ARROW:
        return(VT100_DOWN_ARROW);

    case PCVID_RIGHT_ARROW:
        return(VT100_RIGHT_ARROW);

    case PCVID_LEFT_ARROW:
        return(VT100_LEFT_ARROW);

    /* TBD - more stuff for escape, etc   */

    default:
        return(key);
    }
}

/* *********************************************************************   */
void vt100_savecurpos()
{
    saved_x=pcvid_wherex();
    saved_y=pcvid_wherey();
}

/* *********************************************************************   */
void vt100_savecurattrib()
{
    pcvid_get_text_attrib(&saved_attrib);
}

/* *********************************************************************   */
void vt100_restorecurpos()
{
    pcvid_gotoxy(saved_x,saved_y);
}

/* *********************************************************************   */
void vt100_restorecurattrib()
{
    pcvid_set_text_attrib(&saved_attrib);
}

/* *********************************************************************   */
int vt100_erase_cur_to_endl()
{
int width;
int x,y;
int n;

    if((width=pcvid_get_width())==-1)
        return(-1);
    x=pcvid_wherex();
    y=pcvid_wherey();
    /* tbd: erase whole line   */
    for(n=x;n<=width-1;n++)
        pcvid_putc(' ');
    pcvid_gotoxy(x,y);
    return 0;
}

/* *********************************************************************   */
int vt100_erase_line_to_cur()
{
int x;
int n;

    x=pcvid_wherex();
    pcvid_gotoxy(1,pcvid_wherey());
    for(n=1;n<x;n++)
        pcvid_putc(' ');
    return(0);  
}

/* *********************************************************************   */
int vt100_erase_cur_line()
{
int width;
int x,y;
int n;

    if((width=pcvid_get_width())==-1)
        return(-1);
    x=pcvid_wherex();
    y=pcvid_wherey();
    pcvid_gotoxy(1,y);
    for(n=1;n<=width;n++)
        pcvid_putc(' ');
    pcvid_gotoxy(x,y);
    return 0;
}

/* *********************************************************************   */
int vt100_erase_cur_to_ends()
{
int width,height;
int x,y;
int n;

    if((width=pcvid_get_width())==-1||(height=pcvid_get_height())==-1)
        return(-1);
    x=pcvid_wherex();
    y=pcvid_wherey();
    /* erase to end of this line   */
    for(n=x;n<=width;n++)
        pcvid_putc(' ');
    /* erase rest of screen - leave 1 char to prevent scroll   */
    for(n=0;n<(height-y)*width-1;n++)
        pcvid_putc(' ');
    pcvid_gotoxy(x,y);
    return 0;
}

#endif /* INCLUDE_VT100C */

