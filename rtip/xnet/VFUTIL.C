/*                                                                      */
/* VFUTIL.C - Utility functions for EBS Virtual File System             */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_VFS

#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif
#include "vfile.h"

#if (INCLUDE_VFS)

/*************************************************************************
 * vf_patcmp - Compare a string against a pattern using wildcards        *
 *                                                                                               *
 * Parameters:                                                                               *
 *      pat - the pattern to match the string against                            *
 *      name - the string to match                                                       *
 *  dowildcard - if set to 0, disables the use of wildcards in doing     *
 *          the comparison                                                                   *
 *                                                                                               *
 * Returns:                                                                                  *
 *      1 if the string matches                                                          *
 *      0 otherwise                                                                          *
 *                                                                                           *
 * Examples:                                                                                 * 
 *      vf_patcmp("he*, w*", "hello, world", 1) returns 1                        *
 *      vf_patcmp("*z*", "hello, world", 1) returns 0                            *
 *      vf_patcmp("he?lo, world", "hello, world", 1) returns 1               *
 *      vf_patcmp("he?lo, world", "hello, world", 0) returns 0               *
 *                                                                                               *
 *************************************************************************/

int vf_patcmp(PFCHAR pat, PFCHAR name, int dowildcard)
{
    int p,n,i;
    int res = 0;

    if (!tc_strcmp(pat,"*.*"))
        pat[1] = 0;
    if (!tc_strcmp(pat,"."))
    {
        pat[0] = '*';
        pat[1] = 0;
    }

    for(p=0,n=0; pat[p]!=0; p++,n++)
    {
        if (pat[p]=='*' && dowildcard)
        {
            for(i=n; name[i]!=0; i++)
                res |= vf_patcmp(&(pat[p+1]),&(name[i]),dowildcard);
            res |= vf_patcmp(&(pat[p+1]),&(name[i]),dowildcard);
            return(res);
        }

        if (name[n] == 0)
        {
            /* Match * with end of string   */
            if (pat[p]=='*' && pat[p+1] == 0 && dowildcard)
                return(1);
            else
                return(0);
        }

        if ((pat[p]!='?' || !dowildcard) && pat[p]!=name[n])
                return(0);
                
    }
    if (name[n]==0)
        return(1);
    else
        return(0);
}


/*************************************************************************
 * reduce_path, parse_next, parse_parent - Various parsing routines      *
 *                                                                       *
 *  reduce_path - takes a full path spec and correctly removes all ..'s  *
 *  and .'s as well as double path seperators.                           *
 *                                                                       *
 * parse_next - parses str up to the next occurance of c, and copies     *
 *      the parsed string into to.  A pointer into str of the next string  *
 *      to be parsed is returned.                                        *
 *                                                                       *
 * parse_path - parses str up the last occurance of c, copying the       *
 *      parsed string into to.  Returns the same as parse_next.          *
 *                                                                       *
 *************************************************************************/

int reduce_path(PFCHAR dest, PFCHAR path)
{
    PFCHAR nibble;
    nibble = (char *) vf_alloc(VF_MAXPATHLEN);

    if (!nibble)
        return(1);

    if (*path == CHAR_BACKSLASH)
    {
        *(dest++) = *(path++);
    }
    *dest = 0;
    while (*path)
    {
        path = parse_next(nibble, CHAR_BACKSLASH, path);

        if (!tc_strcmp(nibble,".."))
        {
            parse_parent(dest, CHAR_BACKSLASH, dest);
        }
        else if (tc_strcmp(nibble,".") && *nibble)
        {
            if (*dest)
                tc_strcat(dest, STRING_BACKSLASH);
            tc_strcat(dest, nibble);
        }
    }

    vf_free((PFBYTE)nibble);
    return (1);
}

PFCHAR parse_next(PFCHAR to, char c, PFCHAR str)
{
    while (*str)
    {
        if (*str == c)
        {
            while (*str == c)
                str++;
            break;
        }
        *(to++) = *(str++);
    }
    *to = 0;
    return (str);
}

/* Take the contents of path and copy it to 'to'. make 'to' a null terminated
string containing the contents of path up to the last instance of 'c' in
path. Return a pointer to the offset in path 1 character beyond the last
instance of 'c' in path */
PFCHAR parse_parent(PFCHAR to, char c, PFCHAR path)
{
    PFCHAR clip;
    PFCHAR ret;

    if (*path && *(path+1) == ':')
    {
        *(to++) = *(path++);
        *(to++) = *(path++);
    }

    if (*path == c)
        *(to++) = *(path++);
    
    clip = to;
    ret = path;
    while (*path)
    {
        if (*path == c && *(path + 1))
        {
            clip = to;
            ret = path + 1;
        }
        *(to++) = *(path++);
    }
    *clip = 0;

    return (ret);   
}
#endif /* INCLUDE_VFS */

