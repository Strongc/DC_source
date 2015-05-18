/*                                                                      */
/* VFRTFS.C - RTFS driver for EBS Virtual File System                   */
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

#if (INCLUDE_VFS && INCLUDE_ERTFS_API)

#ifndef BOOLEAN
typedef int BOOLEAN;
#endif
#include "rtfsapi.h"

/* ********************************************************************   */
int  vfrtfs_open(PFCHAR name, word flag, word mode);
long vfrtfs_lseek(int fd, long offset, int origin);
int  vfrtfs_read(int fd,  PFBYTE buf, word count);
int  vfrtfs_write(int fd,  PFBYTE buf, word count);
int  vfrtfs_close(int fd);

RTIP_BOOLEAN vfrtfs_chdir(char *path);
RTIP_BOOLEAN vfrtfs_pwd(char *to);
RTIP_BOOLEAN vfrtfs_truncate(int fd, long offset);
RTIP_BOOLEAN vfrtfs_flush(int fd);
RTIP_BOOLEAN vfrtfs_rename(char *from, char *to);
RTIP_BOOLEAN vfrtfs_remove(char *to);
RTIP_BOOLEAN vfrtfs_mkdir(char *to);
RTIP_BOOLEAN vfrtfs_rmdir(char *to);

RTIP_BOOLEAN vfrtfs_gfirst(PVDSTAT dirobj, PFCHAR name);
RTIP_BOOLEAN vfrtfs_gnext(PVDSTAT dirobj);
void    vfrtfs_gdone(PVDSTAT dirobj);
RTIP_BOOLEAN vfrtfs_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *filesys, MPT_INITFN initfn, PFVOID init_arg);
RTIP_BOOLEAN vfrtfs_unmountfs(PFCHAR mountpath);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
char    *vfrtfs_modified_date(int fd, char *buffer, int buflen);
#endif
#if (INCLUDE_STAT)
RTIP_BOOLEAN vfrtfs_stat(char *name, PVSTAT stat);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN vfrtfs_chmode(char *name, int attributes);
RTIP_BOOLEAN vfrtfs_get_free(char *name, dword *blocks, dword *bfree);
RTIP_BOOLEAN vfrtfs_chsize(int fd, long size);
#endif



/* ********************************************************************   */
/* vfrtfs_api_init - Inintialize a filesystem API structure tht allows
*  mounting RTFS as a node of a virtual file tree.
*
*  This routine takes an API structure and assigns functions to
*  the function pointer in the structure. The functions include
*  open close read write close mkdir rmdir etc.
*/
    
void vfrtfs_api_init(VFILEAPI *papi)
{
    papi->fs_open       =   (API_OPENFN)      vfrtfs_open;
    papi->fs_read       =   (API_READFN)      vfrtfs_read;
    papi->fs_write      =   (API_WRITEFN)     vfrtfs_write;
    papi->fs_lseek      =   (API_LSEEKFN)     vfrtfs_lseek;
    papi->fs_truncate   =   (API_TRUNCATEFN)  vfrtfs_truncate;
    papi->fs_flush      =   (API_FLUSHFN)     vfrtfs_flush;
    papi->fs_close      =   (API_CLOSEFN)     vfrtfs_close;
    papi->fs_rename     =   (API_RENAMEFN)    vfrtfs_rename;
    papi->fs_delete     =   (API_DELETEFN)    vfrtfs_remove;
    papi->fs_mkdir      =   (API_MKDIRFN)     vfrtfs_mkdir;
    papi->fs_rmdir      =   (API_RMDIRFN)     vfrtfs_rmdir;
    papi->fs_set_cwd    =   (API_SETCWDFN)    vfrtfs_chdir;
    papi->fs_pwd        =   (API_PWDFN)       vfrtfs_pwd;
    papi->fs_gfirst     =   (API_GFIRSTFN)    vfrtfs_gfirst;
    papi->fs_gnext      =   (API_GNEXTFN)     vfrtfs_gnext;
    papi->fs_gdone      =   (API_GDONEFN)     vfrtfs_gdone;
    papi->fs_mountfs    =   (API_MOUNTFSFN)   vfrtfs_mountfs;
    papi->fs_unmountfs  =   (API_UNMOUNTFSFN) vfrtfs_unmountfs;
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
    papi->fs_get_file_modified_date     =
                        (API_GETMODDATE)  vfrtfs_modified_date;
#endif
#if (INCLUDE_STAT)
    papi->fs_stat       =   (API_STATFN)      vfrtfs_stat;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
    papi->fs_chmode     =   (API_CHMODEFN)    vfrtfs_chmode;
    papi->fs_get_free   =   (API_GET_FREEFN)  vfrtfs_get_free;
    papi->fs_chsize     =   (API_CHSIZEFN)    vfrtfs_chsize;
#endif
}


/* ********************************************************************   */
int vfrtfs_open(PFCHAR name, word flag, word mode)
{
    int f,m;
    
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

    m = 0;
    m |= (mode & VS_IWRITE)? PS_IWRITE: 0;
    m |= (mode & VS_IREAD)?  PS_IREAD:  0;
    
    return (po_open((char *)name, (word)f, (word)m));
}

/* ********************************************************************   */
int vfrtfs_read(int fd,  PFBYTE buf, word count)
{
    return(po_read(fd, buf, count));
}

/* ********************************************************************   */
int vfrtfs_write(int fd,  PFBYTE buf, word count)
{
    return(po_write(fd, buf, count));
}

/* ********************************************************************   */
int vfrtfs_close(int fd)
{
    return(po_close(fd));
}


/* ********************************************************************   */
long vfrtfs_lseek(int fd, long offset, int origin)
{
    int o;

    switch (origin)
    {
        case VSEEK_SET:
            o = PSEEK_SET;
            break;

        case VSEEK_CUR:
            o = PSEEK_CUR;
            break;

        case VSEEK_END:
            o = PSEEK_END;
            break;
    
        default:
            o = PSEEK_SET;
    }

    return ( po_lseek (fd, offset, o) );
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_truncate(int fd, long offset)
{
    return(po_truncate(fd, offset));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_flush(int fd)
{
    return(po_flush(fd));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_rename(char *from, char *to)
{
    return(pc_mv(from, to));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_remove(char *d)
{
    return(pc_unlink(d));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_mkdir(char *d)
{
    return(pc_mkdir(d));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_rmdir(char *d)
{
    return(pc_rmdir(d));
}


/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_chdir(char *path)
{
int currdrv;
char currdrvstr[3];
char *p;
    char drive[3];
    
    if (path[1] == ':')
    {
        /* Remember the default driveno   */
        currdrv = pc_getdfltdrvno();
        drive[0] = path[0];
        drive[1] = path[1];
        drive[2] = 0;
        if (!pc_set_default_drive(drive))
            return(FALSE);
        path += 2;
        /* If there is no path then do a CWD to the root. IE:
            the user type cd C: */
        if (!*path)
        {
            drive[0] = '\\';
            drive[1] = 0;
            p = drive;    /* point at "\\" */
        }
        else
            p = path;     /* point at the path specifier */
        if (pc_set_cwd(p))
            return(TRUE);
        else
        {
            /* the chdir failed so restore the drive   */
           currdrvstr[0] = 'A' + currdrv;
           currdrvstr[1] = ':';
           currdrvstr[2] = '\0';
           pc_set_default_drive (currdrvstr);
           return(FALSE);
        }
    }
    else
        return (pc_set_cwd(path));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_pwd(char *to)
{                 
    return ( pc_pwd("", to) );
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_gfirst(PVDSTAT dirobj, PFCHAR name)
{
    word flags;

    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));

    if (!pc_gfirst(&dirobj->fs_obj.rtfs,(char *)name))
        return (FALSE);

#if (VFAT)
    tc_strncpy(dirobj->filename, (char *) dirobj->fs_obj.rtfs.lfname, VF_FILENAMESIZE);
#else
    tc_strncpy(dirobj->filename, (char *) dirobj->fs_obj.rtfs.filename, VF_FILENAMESIZE);
#endif
    flags = dirobj->fs_obj.rtfs.fattribute;
    dirobj->fattributes  = (word)((flags & ADIRENT)? VF_ATTRIB_ISDIR:  0);
    dirobj->fattributes |= (word)((flags & AVOLUME)? VF_ATTRIB_ISVOL:  0);
    dirobj->fattributes |= (word)((flags & ARDONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR);
    dirobj->ftime = dirobj->fs_obj.rtfs.ftime;
    dirobj->fdate = dirobj->fs_obj.rtfs.fdate;
    dirobj->fsize = dirobj->fs_obj.rtfs.fsize;
    dirobj->patt_match = TRUE;

    return (TRUE);  
}

RTIP_BOOLEAN vfrtfs_gnext(PVDSTAT dirobj)
{
    word flags;
 
    if (!pc_gnext(&dirobj->fs_obj.rtfs))
        return (FALSE);

#if (VFAT)
    tc_strncpy(dirobj->filename, (char *) dirobj->fs_obj.rtfs.lfname, VF_FILENAMESIZE);
#else
    tc_strncpy(dirobj->filename, (char *) dirobj->fs_obj.rtfs.filename, VF_FILENAMESIZE);
#endif
    flags = dirobj->fs_obj.rtfs.fattribute;
    dirobj->fattributes  = (word)((flags & ADIRENT)? VF_ATTRIB_ISDIR:  0);
    dirobj->fattributes |= (word)((flags & AVOLUME)? VF_ATTRIB_ISVOL:  0);
    dirobj->fattributes |= (word)((flags & ARDONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR);
    dirobj->ftime = dirobj->fs_obj.rtfs.ftime;
    dirobj->fdate = dirobj->fs_obj.rtfs.fdate;
    dirobj->fsize = dirobj->fs_obj.rtfs.fsize;
    dirobj->patt_match = TRUE;

    return (TRUE);  
}

void vfrtfs_gdone(PVDSTAT dirobj)
{
    pc_gdone(&dirobj->fs_obj.rtfs);
    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *filesys, MPT_INITFN initfn, PFVOID init_arg)
{
    ARGSUSED_PVOID(mountpath);
    ARGSUSED_PVOID(nativepath);
    ARGSUSED_PVOID(filesys);
    ARGSUSED_PVOID(init_arg);

    return (FALSE);
}

RTIP_BOOLEAN vfrtfs_unmountfs(PFCHAR mountpath)
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

char *vfrtfs_modified_date(int handle, char *buffer, int buflen)
{
    ARGSUSED_INT(handle);
    ARGSUSED_PVOID(buffer);
    ARGSUSED_INT(buflen);

    /* Not implemented for rtfs   */
    return((char *)0);
}
#endif

#if (INCLUDE_STAT)
/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_stat(char *name, PVSTAT stat)
{
STAT ertfs_stat;

    if (pc_stat(name, &ertfs_stat) == -1)
        return(FALSE);

    stat->vs_mode = 0;
    stat->vs_mode |= (ertfs_stat.st_mode & S_IFDIR) ? VSMODE_IFDIR:  0;
    stat->vs_mode |= (ertfs_stat.st_mode & S_IFREG) ? VSMODE_IFREG:  0;
    stat->vs_mode |= (ertfs_stat.st_mode & S_IWRITE)? VSMODE_IWRITE: 0;
    stat->vs_mode |= (ertfs_stat.st_mode & S_IREAD) ? VSMODE_IREAD:  0;
                            /* always TRUE for ertfs   */

    stat->vs_size = ertfs_stat.st_size; /* file size, in bytes */
    stat->vs_time = ertfs_stat.st_atime.time;
    stat->vs_date = ertfs_stat.st_atime.date;

    return(TRUE);
}
#endif

#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_chmode(char *name, int attributes)
{
byte rtfs_mode;

    rtfs_mode = 0;

    /* rtfs supports read-only but not write-only; so if write is not      */
    /* permitted, set to read-only                                         */
    if (!(attributes & VS_IWRITE))
        rtfs_mode |= ARDONLY;
    
    return(pc_set_attributes(name, rtfs_mode));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_chsize(int fd, long size)
{
    if (po_chsize(fd, size) == 0)
        return(TRUE);
    return(FALSE);
}

#endif      /* INCLUDE_NFS_SRV */

#if (INCLUDE_GET_FREE)
/* ********************************************************************   */
RTIP_BOOLEAN vfrtfs_get_free(char *name, dword *blocks, dword *bfree)
{
    if (pc_free(name, blocks, bfree))
        return(TRUE);
    return(FALSE);
}
#endif

#endif      /* INCLUDE_VFS and INCLUDE_ERTFS_API */
