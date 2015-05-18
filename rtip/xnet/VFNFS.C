/*                                                                      */
/* VFNFS.C - NFS Client driver for EBS Virtual File System              */
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
#include "vfile.h"

#if (INCLUDE_VFS && INCLUDE_NFS_CLI)
#include "nc.h"

/* ********************************************************************   */
int     vfnc_open(PFCHAR name, word flag, word mode);
PFCHAR  vfnc_modified_date(int fd, PFCHAR buffer, int buflen);
long    vfnc_lseek(int fd, long offset, int origin);
RTIP_BOOLEAN vfnc_set_cwd(PFCHAR path);
RTIP_BOOLEAN vfnc_pwd(PFCHAR to);
RTIP_BOOLEAN vfnc_gfirst(PVDSTAT dirobj, PFCHAR name);
RTIP_BOOLEAN vfnc_gnext(PVDSTAT dirobj);
void    vfnc_gdone(PVDSTAT dirobj);
RTIP_BOOLEAN vfnc_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *filesys, MPT_INITFN initfn, PFVOID init_arg);
RTIP_BOOLEAN vfnc_unmountfs(PFCHAR mountpath);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
PFCHAR  vfnc_modified_date(int handle, PFCHAR buffer, int buflen);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN vfnc_stat(PFCHAR name, PVSTAT stat);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN vfnc_chmode(PFCHAR name, int attributes);
RTIP_BOOLEAN vfnc_get_free(PFCHAR name, dword *blocks, dword *bfree);
RTIP_BOOLEAN vfnc_chsize(int fd, long size);
#endif
    
/* ********************************************************************   */
/* vfnc_api_init - Inintialize a filesystem API structure that allows
*  mounting an NFS client connection.
*
*  This routine takes an API structure and assigns functions to
*  the function pointer in the structure. The functions include
*  open close read write close mkdir rmdir etc.
*/

void vfnc_api_init(VFILEAPI *papi)
{
    papi->fs_open       =   (API_OPENFN)      vfnc_open;
    papi->fs_read       =   (API_READFN)      nc_fi_read;
    papi->fs_write      =   (API_WRITEFN)     nc_fi_write;
    papi->fs_lseek      =   (API_LSEEKFN)     vfnc_lseek;
    papi->fs_truncate   =   (API_TRUNCATEFN)  nc_fi_truncate;
    papi->fs_flush      =   (API_FLUSHFN)     nc_fi_flush;
    papi->fs_close      =   (API_CLOSEFN)     nc_fi_close;
    papi->fs_rename     =   (API_RENAMEFN)    nc_mv;
    papi->fs_delete     =   (API_DELETEFN)    nc_unlink;
    papi->fs_mkdir      =   (API_MKDIRFN)     nc_mkdir;
    papi->fs_rmdir      =   (API_RMDIRFN)     nc_rmdir;
    papi->fs_set_cwd    =   (API_SETCWDFN)    vfnc_set_cwd;
    papi->fs_pwd        =   (API_PWDFN)       vfnc_pwd;
    papi->fs_gfirst     =   (API_GFIRSTFN)    vfnc_gfirst;
    papi->fs_gnext      =   (API_GNEXTFN)     vfnc_gnext;
    papi->fs_gdone      =   (API_GDONEFN)     vfnc_gdone;
    papi->fs_mountfs    =   (API_MOUNTFSFN)   vfnc_mountfs;
    papi->fs_unmountfs  =   (API_UNMOUNTFSFN) vfnc_unmountfs;
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
    papi->fs_get_file_modified_date     =
                        (API_GETMODDATE)  vfnc_modified_date;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_VF_ALL)
    papi->fs_stat       =   (API_STATFN)      vfnc_stat;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
    papi->fs_chmode     =   (API_CHMODEFN)    vfnc_chmode;
    papi->fs_get_free   =   (API_GET_FREEFN)  vfnc_get_free;
    papi->fs_chsize     =   (API_CHSIZEFN)    vfnc_chsize;
#endif


}

/* ********************************************************************   */
int vfnc_open(PFCHAR name, word flag, word mode)
{
int f;

    ARGSUSED_INT(mode);
    
    f = 0;
    f |= (flag & VO_RDONLY)? PO_RDONLY: 0;
    f |= (flag & VO_WRONLY)? PO_WRONLY: 0;
    f |= (flag & VO_RDWR)?   PO_RDWR:   0;
    f |= (flag & VO_WRONLY)? PO_WRONLY: 0;
    f |= (flag & VO_RDWR)?   PO_RDWR:   0;
    f |= (flag & VO_APPEND)? PO_APPEND: 0;
    f |= (flag & VO_CREAT)?  PO_CREAT:  0;
    f |= (flag & VO_TRUNC)?  PO_TRUNC:  0;
    f |= (flag & VO_EXCL)?   PO_EXCL:   0;
    f |= (flag & VO_BINARY)? PO_BINARY: 0;
    f |= (flag & VO_TEXT)?   PO_TEXT:   0;
    
    return (nc_fi_open((PFCHAR)name, (word)f));
}

/* ********************************************************************   */
long vfnc_lseek(int fd, long offset, int origin)
{
    return ( nc_fi_lseek (fd, offset, origin) );
}

/* ********************************************************************   */
RTIP_BOOLEAN vfnc_set_cwd(PFCHAR path)
{
char drive[3];
    
    if (path[1] == ':')
    {
        drive[0] = path[0];
        drive[1] = path[1];
        drive[2] = 0;
        nc_set_default_drive(drive);
        path += 2;
    }
    return ((RTIP_BOOLEAN)nc_set_cwd(path));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfnc_pwd(PFCHAR to)
{                 
    return ( (RTIP_BOOLEAN)nc_pwd("", to) );
}

/* ********************************************************************   */
void dir_get_file_info(PVDSTAT dirobj)
{
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
VSTAT  file_stats;
#endif
int    path_len;

/*DEBUG_ERROR("dir_get_file_info: file = ", STR1,   */
/*  dirobj->fs_obj.nfs.stat.curr_entry->name, 0);   */

#if (!INCLUDE_MFS)
    dirobj->patt_match = FALSE;
    if (vf_patcmp(dirobj->fs_obj.nfs.pattern, 
                  dirobj->fs_obj.nfs.stat.curr_entry->name, 1))
    {
#endif
        dirobj->patt_match = TRUE;
        tc_strncpy(dirobj->filename, 
                dirobj->fs_obj.nfs.stat.curr_entry->name, 
                VF_FILENAMESIZE);

        path_len = tc_strlen(dirobj->fs_obj.nfs.path);

#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
        tc_strcat(dirobj->fs_obj.nfs.path, dirobj->filename);
        if (vfnc_stat(dirobj->fs_obj.nfs.path, (PVSTAT)&file_stats))
        {
            dirobj->fattributes = 0;
            dirobj->fattributes |= (word)
                ((file_stats.vs_mode & VSMODE_IFDIR) ? VF_ATTRIB_ISDIR:  0);
            if ( (file_stats.vs_mode & VSMODE_IWRITE) &&
                (file_stats.vs_mode & VSMODE_IREAD) )
                dirobj->fattributes |= VF_ATTRIB_RDWR;
            else if (file_stats.vs_mode & VSMODE_IWRITE)
                dirobj->fattributes |= VF_ATTRIB_WRONLY;
            else if (file_stats.vs_mode & VSMODE_IREAD)
                dirobj->fattributes |= VF_ATTRIB_RDONLY;

            dirobj->ftime = file_stats.vs_time;
            dirobj->fdate = file_stats.vs_date;
            dirobj->fsize = file_stats.vs_size;
        }
        else
#endif
        {
            dirobj->fattributes = 0;
            dirobj->ftime = 0;
            dirobj->fdate = 0;
            dirobj->fsize = 0;
        }
        /* take filename off of path   */
        dirobj->fs_obj.nfs.path[path_len] = '\0';
#if (!INCLUDE_MFS)
    }
#endif
}

RTIP_BOOLEAN vfnc_gfirst(PVDSTAT dirobj, PFCHAR name)
{
PFCHAR patt;

    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));

    /* NFS client does not have pattern matching built into it; therefore,   */
    /* it is done here                                                       */
    /* parse_parent() sets up dirobj->fs_obj.nfs.path based upon name        */
    /* NOTE: this code assumes path\file, i.e. a dir of x where x            */
    /*       is a directory is not supported                                 */
    patt = parse_parent(dirobj->fs_obj.nfs.path, CHAR_BACKSLASH, name);
#if (!INCLUDE_MFS)
    tc_strncpy(dirobj->fs_obj.nfs.pattern, patt, VF_FILENAMESIZE);
#endif

#if (!INCLUDE_MFS)
    if (nc_gfirst(&dirobj->fs_obj.nfs.stat, dirobj->fs_obj.nfs.path, 1024))
#else
    if (nc_gfirst(&dirobj->fs_obj.nfs.stat, name, 1024))
#endif
    {
        if (dirobj->fs_obj.nfs.stat.entry_cnt)
        {
            dir_get_file_info(dirobj);
            return (TRUE);
        }
    }
    return(FALSE);
}

RTIP_BOOLEAN vfnc_gnext(PVDSTAT dirobj)
{
    if (nc_gnext(&dirobj->fs_obj.nfs.stat))
    {
        dir_get_file_info(dirobj);
        return (TRUE);
    }
    return (FALSE);
}

void vfnc_gdone(PVDSTAT dirobj)
{
    nc_gdone(&dirobj->fs_obj.nfs.stat);
    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));
}


/* ********************************************************************   */
RTIP_BOOLEAN vfnc_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *filesys, MPT_INITFN initfn, PFVOID init_arg)
{
    ARGSUSED_PVOID(mountpath);
    ARGSUSED_PVOID(nativepath);
    ARGSUSED_PVOID(filesys);
    ARGSUSED_PVOID(initfn);
    ARGSUSED_PVOID(init_arg);
    DEBUG_ERROR("vfnc_mountfs: cannot mount onto NFS Client", NOVAR, 0, 0);
    return (FALSE);
}

RTIP_BOOLEAN vfnc_unmountfs(PFCHAR mountpath)
{
    ARGSUSED_PVOID(mountpath);
    return (FALSE);
}

#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
/* ********************************************************************   */
/* This function returns a pointer to a string showing the file           */
/* creation date, or it returns null on failure.                          */
/* The format of the string it returns is very important.                 */
/* It must be as follows:                                                 */
/*                                                                        */
/*                 1111111111222222222                                    */
/*       01234567890123456789012345678                                    */
/*       DAY, DD MON YEAR hh:mm:ss GMT                                    */
/* i.e. "Wed, 04 Oct 1995 22:33:27 GMT";                                  */

PFCHAR vfnc_modified_date(int handle, PFCHAR buffer, int buflen)
{
    /* tbd   */
    return(0);
}
#endif      /* INCLUDE_WEB */

#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_VF_ALL)
/* ********************************************************************   */
RTIP_BOOLEAN vfnc_stat(PFCHAR name, PVSTAT vstat)
{
attrstat attr_stats;
dword    mode;
#if (SUPPORTS_DOS_CALLS)
struct tm *tme;
#endif

    if (!nc_stat(name, (PATTRSTAT)&attr_stats))
        return(FALSE);

    mode = attr_stats.attrstat_u.attributes.mode;

    vstat->vs_mode = 0;
    vstat->vs_mode |= (mode & NFSMODE_DIR)   ? VSMODE_IFDIR:  0;
    vstat->vs_mode |= (mode & NFSMODE_REG)   ? VSMODE_IFREG:  0;
    vstat->vs_mode |= (mode & NFSMODE_WRITE) ? VSMODE_IWRITE: 0;
    vstat->vs_mode |= (mode & NFSMODE_READ)  ? VSMODE_IREAD:  0;

    vstat->vs_size =    attr_stats.attrstat_u.attributes.size;  
                                        /* file size, in bytes   */
    vstat->vs_etime = attr_stats.attrstat_u.attributes.atime.seconds;
                                        /* time since 1/1/70   */

#if (SUPPORTS_DOS_CALLS)
    /* convert time to format virtual file system returns   */
    tme = localtime((const time_t *)&(vstat->vs_etime));

    vstat->vs_date  = (word)(((tme->tm_mon+1) & 0xf) << 5); /* Month */
    vstat->vs_date |= (word)(tme->tm_mday & 0x1f);              /* Day */
    vstat->vs_date |= (word)(((tme->tm_year-80) & 0xff) << 9);  /* Year */
    vstat->vs_time  = (word)((tme->tm_hour & 0x1f) << 11);  /* Hour */
    vstat->vs_time |= (word)((tme->tm_min  & 0x3f) << 5);   /* Minute */

#else
    vstat->vs_date  = 0;
    vstat->vs_time  = 0;
#endif

    return(TRUE);
}
#endif

#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
/* ********************************************************************   */
RTIP_BOOLEAN vfnc_chmode(PFCHAR name, int attributes)
{
int mode;

    if (!nc_get_attributes(name, (PFINT)&mode))
        return(FALSE);

    mode &= ~(NFSMODE_WRITE & NFSMODE_READ);
    if (attributes & VSMODE_IWRITE)
        mode |= NFSMODE_WRITE;
    else
        mode &= ~NFSMODE_WRITE;

    if (attributes & VSMODE_IREAD)
        mode |= NFSMODE_READ;
    else
        mode &= ~NFSMODE_READ;

    return((RTIP_BOOLEAN)(nc_set_attributes(name, mode)));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfnc_get_free(PFCHAR name, dword *blocks, dword *bfree)
{
FS_INFO fs_info;

    if (!nc_statfs(name, (PFSINFO)&fs_info))
        return(FALSE);

    *blocks = fs_info.blocks;
    *bfree = fs_info.bfree;
    return(TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN vfnc_chsize(int fd, long size)
{
    return((RTIP_BOOLEAN)(nc_fi_chsize(fd, size)));
}

#endif      /* INCLUDE_NFS_SRV */


#endif      /* INCLUDE_VFS and INCLUDE_ERTFS */










































