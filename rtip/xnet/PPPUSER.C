/*                                                                            */
/*  PPPUSER.C   -- USER Accounts for Password Authentication Protocol for PPP */
/*                                                                            */
/*  EBS - RTIP                                                                */
/*                                                                            */
/*  Copyright Peter Van Oudenaren , 1993                                      */
/*  All rights reserved.                                                      */
/*  This code may not be redistributed in source or linkable object form      */
/*  without the consent of its author.                                        */
/*                                                                            */
/*  This implementation of PPP is declared to be in the public domain.        */
/*                                                                            */
/*  Jan 91  Bill_Simpson@um.cc.umich.edu                                      */
/*      Computer Systems Consulting Services                                  */
/*                                                                            */
/*  Acknowledgements and correction history may be found in PPP.C             */
/*                                                                            */

#define DIAG_SECTION_KERNEL DIAG_SECTION_PPP

#include "sock.h"
#include "rtip.h"

/* tbd - some routines can be under INCLUDE_PPP_SRV)   */
#if (INCLUDE_PPP && INCLUDE_PAP)

#if (PAP_FILE)
    #include <stdio.h>
#endif

/***********************************************************************  */
#if (PAP_FILE)
    static void crunch(char *buf,char *path);
#endif

/***********************************************************************  */
#if (!PAP_FILE)
    extern char KS_FAR pap_users[CFG_PAP_USERS][CFG_USER_REC_LEN];
#endif

/***********************************************************************  */
#if (!PAP_FILE)
void user_init()
{
int i;

    for (i=0; i < CFG_PAP_USERS; i++)
    {
        pap_users[i][0] = NULLCHAR;
    }
}

/* Set user/password - used for sending PAP request when enter authentication
   mode if PAP successfully negotiated */
int pap_add_user(PFCHAR user_name, PFCHAR pass_word)
{
int i;
PFCHAR buf_ptr;

    if (!user_name || !pass_word)
        return(-1);

    for (i=0; i < CFG_PAP_USERS; i++)
    {
        if (pap_users[i][0] == NULLCHAR)
        {
            buf_ptr = (PFCHAR)&(pap_users[i][0]);
            tc_strncpy(buf_ptr, user_name, CFG_PAP_NAMELEN);
            buf_ptr += tc_strlen(user_name);
            *buf_ptr++ = ' ';

            tc_strncpy(buf_ptr, pass_word, CFG_PAP_PWDLEN);

            return(0);
        }
    }
    return(-1);
}


#endif

/* Read through pap_users looking for user record (based upon username)
 * Copies line which matches username to buf or sets buf to empty string
 * when no match.
 * Sets password and directory to point to those entries within buf and
 * puts an end-of-string after each.  Copies and translates permission
 * and ip address.
 * NOTE: password, directory, permission, ip_address do not necessarily
 *       exist in the entry
*/
void userlookup(PFCHAR username, PFCHAR *password, PFCHAR *directory,
                PFINT permission, PFBYTE ip_address, PFCHAR buf)
{
#if (PAP_FILE)
    FILE *fp;
#else
    int i;
#endif
PFCHAR cp = 0;

#    if (PAP_FILE)
        if((fp = fopen(Userfile,READ_TEXT)) == NULLFILE)
            /* Userfile doesn't exist   */
            return;
#    else
        ARGSUSED_PVOID(directory);
        ARGSUSED_PVOID(ip_address);
#    endif

    /* read lines of the file until get user entry looking for   */
#if (PAP_FILE)
    while (fgets(buf, CFG_USER_REC_LEN, fp) != NULLCHARP)
    {
#else
    for (i=0; i < CFG_PAP_USERS; i++)
    {
        tc_strcpy(buf, pap_users[i]);   /* tbd - optimize this  */
        if (buf[0] == NULLCHAR)
            continue;
#endif
        /* if comment or empty entry   */
        if ( (buf[0] == '#') || (buf[0] == NULLCHAR) )
            continue;   

        /* set cp to point to password and replace the space inbetween   */
        /* the user name and the password with an end-of-string          */
        /* NOTE: bogus entry if no space                                 */
        if ((cp = tc_strchr(buf, (int)' ')) == (PFCHAR)0)
            continue;
        *cp++ = '\0';       /* Now points to password */

        if (tc_stricmp(username, buf) == 0)
            break;      /* Found user */
    }

#if (PAP_FILE)
    /* if username not found in file   */
    if (feof(fp))
    {
        fclose(fp);
        buf[0] = NULLCHAR;
        return;
    }

    fclose(fp);
#else
    if (i>= CFG_PAP_USERS)
        buf[0] = NULLCHAR;
#endif

    if ( password != 0 )
        *password = cp;

#if (PAP_FILE)
    /* Look for space after password field in file   */
    if ((cp = tc_strchr(cp, (int)' ')) == (PFCHAR)0) 
    {
        /* Invalid file entry   */
        buf[0] = NULLCHAR;
        return;
    }

    *cp++ = '\0';   /* Now points to directory */

    if (directory)
        *directory = cp;

    if ((cp = tc_strchr(cp, (int)' ')) == NULLCHARP) 
    {
        /* Permission field missing   */
        buf[0] = NULLCHAR;
        return;
    }
    *cp++ = '\0';   /* now points to permission field */

    if (permission)
        *permission = (int)strtol( cp, NULLCHARP, 0 );  /* tbd - far */

    if ((cp = tc_strchr(cp, (int)' ')) == NULLCHARP) 
    {
        /* IP address missing   */
        if ( !tc_cmp4(ip_address, (PFBYTE)ip_nulladdr, IP_ALEN) )
            tc_mv4(ip_address, (PFBYTE)ip_nulladdr, IP_ALEN);
    } 
    else 
    {
        *cp++ = '\0';   /* now points at IP address field */
        if ( !tc_cmp4(ip_address, (PFBYTE)ip_nulladdr, IP_ALEN) )
            resolve(cp, ip_address);   /* domain name to address  */
    }
#else
    /* always default to PPP priveledge since we are not using a file   */
    /* of user accounts                                                 */
    if (permission)
        *permission = PPP_ACCESS_PRIV;
#endif
}

/* Check the database for this user; get password if available   */
void pap_pwdlookup(PPAPS pap_p)
{
char buf[CFG_USER_REC_LEN];
PFCHAR password;
int permission;

    userlookup(pap_p->username, &password, NULLCHARP, (PFINT)&permission, 0, (PFCHAR)buf);
    if (buf[0] == NULLCHAR)
        return;

    /* Check permissions for this user   */
    if ( (permission & PPP_PWD_LOOKUP) == 0 ) 
    {
        /* Not database for password lookup   */
        return;
    }

    /* Save the password from this userfile record   */
    if ( tc_strlen((PFCHAR)password) != 0 )
        tc_strcpy(pap_p->password, password);
}


/* Subroutine for logging in the user whose name is name and password is pass.
 * The buffer path should be long enough to keep a line from the userfile.
 * If pwdignore is true, the password check will be overridden.
 * The return value is the permissions field or -1 if the login failed.
 * Path is set to point at the path field, and pwdignore will be true if no
 * particular password was needed for this user.
 */
/* len =  Length of buffer pointed at by *path   */
int userlogin(PFCHAR name, PFCHAR pass, PFCHAR path, int len, PFINT pwdignore)
{
char buf[CFG_USER_REC_LEN];
PFCHAR password;
PFCHAR directory;
#if (PAP_FILE)
    char full_dir[120];
    char *cp;
#endif
int permission;
int anonymous;

    ARGSUSED_PVOID(path);
    ARGSUSED_INT(len);

    if (!name)
        return -1;

    /* Get the password associated with user   */
    userlookup(name, &password, &directory, &permission, 0, buf);
    if (buf[0] == NULLCHAR)   /* if not legal user */
    {
        DEBUG_ERROR("pppuser: userlogin: not a legal user: ", 
            STR2, name, pass);
        return -1;
    }

    anonymous = *pwdignore;
    if ( tc_strcmp(password, (PFCHAR)"*") == 0 )
        anonymous = TRUE;   /* User ID is password-free */

    if (!anonymous)
    {
        /* if no password but one is not required           */
        /* NOTE: pass=0 means password len in message was 0 */
        if (!pass && tc_strlen(password) > 0)
        {
            DEBUG_ERROR("pppuser: userlogin: no password", 
                STR2, name, pass);
            return -1;
        }

        else if (tc_strcmp(password, pass) != 0) 
        {
            /* Password required, but wrong one given   */
            DEBUG_ERROR("pppuser: userlogin: invalid password", 
                STR2, name, pass);
            return -1;
        }
    }

#if (PAP_FILE)
    if ( tc_strlen(directory) + 1 > len ) 
    {
        /* not enough room for path   */
        return -1;
    }

#if   defined(AMIGA)
    /*
     * Well, on the Amiga, a file can be referenced by many names:
     * device names (DF0:) or volume names (My_Disk:).  This hunk of code
     * passed the pathname specified in the ftpusers file, and gets the
     * absolute path copied into the user's buffer.  We really should just
     * allocate the buffer and return a pointer to it, since the caller
     * really doesn't have a good idea how long the path string is..
     */
    pathname("", directory, full_dir); 

    if (full_dir[0] != NULLCHAR)
    {
        tc_strcpy(path, full_dir);
    } 
    else 
    {
        *path = '\0';
    }
#else
    tc_strcpy(path,full_dir);
    /* Convert any backslashes to forward slashes, for backward
     * compatibility with the old NET
     */
    while ((cp = tc_strchr(path, (int)'\\')) != NULLCHARP)
        *cp = '/';
#endif
#endif
    *pwdignore = anonymous;
    /* Finally return the permission bits   */
    return permission;
}


#if (PAP_FILE)

/* Given a working directory and an arbitrary pathname, resolve them into
 * an absolute pathname. Memory is allocated for the result, which
 * the caller must free
 */
void pathname(cd, path, full_dir)
char *cd;   /* Current working directory */
char *path; /* Pathname argument */
char *full_dir;
{
register char *buf;
#ifdef  MSDOS
    char *cp,c;
    char *tbuf;
    int tflag = 0;
#endif

    if (!cd || !path)
        full_dir[0] = NULLCHAR;

#ifdef  MSDOS
    /* If path has any backslashes, make a local copy with them
     * translated into forward slashes
     */
    if (tc_strchr(path, (int)'\\') != NULLCHARP)
    {
        tflag = 1;
        cp = tbuf = mallocw(tc_strlen(path));
        while((c = *path++) != '\0')
        {
            if(c == '\\')
                *cp++ = '/';
            else
                *cp++ = c;
        }
        *cp = '\0';
        path = tbuf;
    }
#endif

    /* Strip any leading white space on args   */
    while(*cd == ' ' || *cd == '\t')
        cd++;
    while(*path == ' ' || *path == '\t')
        path++;

    /* Allocate and initialize output buffer; user must free   */
    buf = mallocw((unsigned)tc_strlen(cd) + tc_strlen(path) + 10);  /* fudge factor */
    buf[0] = '\0';

    /* Interpret path relative to cd only if it doesn't begin with "/"   */
    if(path[0] != '/')
        crunch(buf,cd);

    crunch(buf,path);

    /* Special case: null final path means the root directory   */
    if(buf[0] == '\0')
    {
        buf[0] = '/';
        buf[1] = '\0';
    }
#ifdef  MSDOS
    if(tflag)
        free(tbuf);
#endif
    return buf;
}

/* Process a path name string, starting with and adding to
 * the existing buffer
 */
static void crunch(buf,path)
char *buf;
register char *path;
{
register char *cp;

    cp = buf + tc_strlen(buf);  /* Start write at end of current buffer */
    
    /* Now start crunching the pathname argument   */
    for (;;)
    {
        /* Strip leading /'s; one will be written later   */
        while (*path == '/')
            path++;
        if (*path == '\0')
            break;      /* no more, all done */
        /* Look for parent directory references, either at the end
         * of the path or imbedded in it
         */
        if (strcmp(path,"..") == 0 || strncmp(path,"../",3) == 0)
        {
            /* Hop up a level   */
            if ((cp = tc_strrchr(buf, (int)'/')) == NULLCHARP)
                cp = buf;   /* Don't back up beyond root */
            *cp = '\0';     /* In case there's another .. */
            path += 2;      /* Skip ".." */
            while(*path == '/') /* Skip one or more slashes */
                path++;
        /* Look for current directory references, either at the end
         * of the path or imbedded in it
         */
        } else if(strcmp(path,".") == 0 || strncmp(path,"./",2) == 0){
            /* "no op"   */
            path++;         /* Skip "." */
            while(*path == '/') /* Skip one or more slashes */
                path++;
        } else {
            /* Ordinary name, copy up to next '/' or end of path   */
            *cp++ = '/';
            while(*path != '/' && *path != '\0')
                *cp++ = *path++;
        }
    }
    *cp++ = '\0';
}
#endif

#endif  /* INCLUDE_PPP and INCLUDE_PAP and INCLUDE_PPP_SRV */
