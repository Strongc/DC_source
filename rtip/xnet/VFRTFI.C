/*                                                                         */
/* VFRTFI.C - RTFiles driver for EBS Virtual File System                   */
/*                                                                         */
/* EBS - RTIP                                                              */
/*                                                                         */
/* Copyright EBSnet Inc, 1998                                              */
/* All rights reserved.                                                    */
/* This code may not be redistributed in source or linkable object form    */
/* without the consent of its author.                                      */
/*                                                                         */

#define DIAG_SECTION_KERNEL DIAG_SECTION_VFS



#include "sock.h"
#include "rtip.h"
#include "vfile.h"

#if (INCLUDE_RTFILES)

#include "rtfiles.h"

/* ********************************************************************   */
int  vfrtfi_open(PFCHAR name, word flag, word mode);
long vfrtfi_lseek(int fd, long offset, int origin);
int vfrtfi_read(int fd,  PFBYTE buf, word count);
int vfrtfi_write(int fd,  PFBYTE buf, word count);
int vfrtfi_close(int fd);

RTIP_BOOLEAN vfrtfi_chdir(PFCHAR path);
RTIP_BOOLEAN vfrtfi_pwd(PFCHAR to);
RTIP_BOOLEAN vfrtfi_truncate(int fd, long offset);
RTIP_BOOLEAN vfrtfi_flush(int fd);
RTIP_BOOLEAN vfrtfi_rename(PFCHAR from, PFCHAR to);
RTIP_BOOLEAN vfrtfi_remove(PFCHAR to);
RTIP_BOOLEAN vfrtfi_mkdir(PFCHAR to);
RTIP_BOOLEAN vfrtfi_rmdir(PFCHAR to);

RTIP_BOOLEAN vfrtfi_gfirst(PVDSTAT dirobj,PFCHAR name);
RTIP_BOOLEAN vfrtfi_gnext(PVDSTAT dirobj);
void    vfrtfi_gdone(PVDSTAT dirobj);
RTIP_BOOLEAN vfrtfi_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *filesys, MPT_INITFN initfn, PFVOID init_arg);
RTIP_BOOLEAN vfrtfi_unmountfs(PFCHAR mountpath);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
char    *vfrtfi_modified_date(int fd, PFCHAR buffer, int buflen);
#endif
#if (INCLUDE_STAT)
RTIP_BOOLEAN vfrtfi_stat(char *name, PVSTAT stat);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN vfrtfi_chmode(char *name, int attributes);
RTIP_BOOLEAN vfrtfi_get_free(char *name, dword *blocks, dword *bfree);
RTIP_BOOLEAN vfrtfi_chsize(int fd, long size);
#endif      /* INCLUDE_NFS_SRV */




/* vfrtfi_api_init - Inintialize a filesystem API structure tht allows
*  mounting RTFS as a node of a virtual file tree.
*
*  This routine takes an API structure and assigns functions to
*  the function pointer in the structure. The functions include
*  open close read write close mkdir rmdir etc.
*
*
*
*
*
*/
    
void vfrtfi_api_init(VFILEAPI *papi)
{
    papi->fs_open       =   (API_OPENFN)      vfrtfi_open;
    papi->fs_read       =   (API_READFN)      vfrtfi_read;
    papi->fs_write      =   (API_WRITEFN)     vfrtfi_write;
    papi->fs_lseek      =   (API_LSEEKFN)     vfrtfi_lseek;
    papi->fs_truncate   =   (API_TRUNCATEFN)  vfrtfi_truncate;
    papi->fs_flush      =   (API_FLUSHFN)     vfrtfi_flush;
    papi->fs_close      =   (API_CLOSEFN)     vfrtfi_close;
    papi->fs_rename     =   (API_RENAMEFN)    vfrtfi_rename;
    papi->fs_delete     =   (API_DELETEFN)    vfrtfi_remove;
    papi->fs_mkdir      =   (API_MKDIRFN)     vfrtfi_mkdir;
    papi->fs_rmdir      =   (API_RMDIRFN)     vfrtfi_rmdir;
    papi->fs_set_cwd    =   (API_SETCWDFN)    vfrtfi_chdir;
    papi->fs_pwd        =   (API_PWDFN)       vfrtfi_pwd;
    papi->fs_gfirst     =   (API_GFIRSTFN)    vfrtfi_gfirst;
    papi->fs_gnext      =   (API_GNEXTFN)     vfrtfi_gnext;
    papi->fs_gdone      =   (API_GDONEFN)     vfrtfi_gdone;
    papi->fs_mountfs    =   (API_MOUNTFSFN)   vfrtfi_mountfs;
    papi->fs_unmountfs  =   (API_UNMOUNTFSFN) vfrtfi_unmountfs;
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
    papi->fs_get_file_modified_date     =
                        (API_GETMODDATE)  vfrtfi_modified_date;
#endif
#if (INCLUDE_STAT)
    papi->fs_stat       =   (API_STATFN)      vfrtfi_stat;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
    papi->fs_chmode     =   (API_CHMODEFN)    vfrtfi_chmode;
    papi->fs_get_free   =   (API_GET_FREEFN)  vfrtfi_get_free;
    papi->fs_chsize     =   (API_CHSIZEFN)    vfrtfi_chsize;
#endif
}


int vfrtfi_open(PFCHAR name, word flag, word mode)
{
    DWORD f;
    
    ARGSUSED_INT(mode);    
    f = RTF_OPEN_NO_DIR|RTF_OPEN_SHARED;
    f |= (flag & VO_RDONLY)? RTF_READ_ONLY: 0;
/*  f |= (flag & VO_WRONLY)? PO_WRONLY: 0;      */
    f |= (flag & VO_RDWR)?   RTF_READ_WRITE:   0;
/*  f |= (flag & VO_APPEND)? PO_APPEND: 0;     */
    f |= (flag & VO_CREAT)
         ? ((flag & VO_TRUNC) ? RTF_CREATE_ALWAYS : RTF_CREATE)
         : 0;
/*  f |= (flag & VO_EXCL)?   PO_EXCL:   0;       */
/*  f |= (flag & VO_BINARY)? PO_BINARY: 0;       */
/*  f |= (flag & VO_TEXT)?   PO_TEXT:   0;       */

    
    return (RTFOpen((PFCHAR )name, f));
}

/* ********************************************************************     */
int vfrtfi_read(int fd,  PFBYTE buf, word count)
{
int n_read;

    if (RTFRead(fd, buf, count, (UINT *)&n_read) == RTF_NO_ERROR)
        return(n_read);
    else
        return(-1);
}

/* ********************************************************************     */
int vfrtfi_write(int fd,  PFBYTE buf, word count)
{
int n_written;

    if (RTFWrite(fd, buf, count, (UINT *)&n_written) == RTF_NO_ERROR)
        return(n_written);
    else
        return(-1);
}

/* ********************************************************************     */
int vfrtfi_close(int fd)
{
    return(RTFClose(fd));
}


/* ********************************************************************     */
long vfrtfi_lseek(int fd, long offset, int origin)
{
    int o;

    switch (origin)
    {
        case VSEEK_SET:
            o = RTF_FILE_BEGIN;
            break;

        case VSEEK_CUR:
            o = RTF_FILE_CURRENT;
            break;

        case VSEEK_END:
            o = RTF_FILE_END;
            break;
    
        default:
            o = RTF_FILE_BEGIN;
    }

    return ( RTFSeek (fd, offset, o) );
}

/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_truncate(int fd, long offset)
{
    if (RTFSeek(fd, offset, RTF_FILE_BEGIN) == offset)
    {
        if ( RTFTruncate(fd) == RTF_NO_ERROR )
            return(TRUE);
    }
    return(FALSE);
}


/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_flush(int fd)
{
    if (RTFCommit(fd) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);
}


/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_rename(PFCHAR from, PFCHAR to)
{
    if (RTFRename(from, to) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);

}

/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_remove(PFCHAR d)
{
    if (RTFDelete(d) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);

}

/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_mkdir(PFCHAR d)
{
    if (RTFCreateDir(d) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);
}

/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_rmdir(PFCHAR d)
{
    if (RTFRemoveDir(d) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);
}


RTIP_BOOLEAN vfrtfi_chdir(PFCHAR path)
{
    if (RTFSetCurrentDir(path) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);
}

RTIP_BOOLEAN vfrtfi_pwd(PFCHAR to)
{                 
    if (RTFGetCurrentDir(to,80) == RTF_NO_ERROR)
        return(TRUE);
    else
        return(FALSE);
}


void vfrtfi_mapp_attr(PVDSTAT dirobj)
{
    byte flags;
    word t1;
    word t2;

    flags = dirobj->fs_obj.rtfi.rtfiles.Attributes;
    dirobj->fattributes  = (word)((flags & RTF_ATTR_DIR)? VF_ATTRIB_ISDIR:  0);
    dirobj->fattributes |= (word)((flags & RTF_ATTR_VOLUME)? VF_ATTRIB_ISVOL:  0);
    dirobj->fattributes |= (word)((flags & RTF_ATTR_READ_ONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR);

    t1 = (word)(dirobj->fs_obj.rtfi.rtfiles.DateTime.Hour);
    t1 <<= 11;
    t2 = t1;
    t1 = (word)(dirobj->fs_obj.rtfi.rtfiles.DateTime.Minute);
    t1 <<= 5;
    t2 |= t1;
    t1 = (word)(dirobj->fs_obj.rtfi.rtfiles.DateTime.Second2);
    t2 |= t1;
    dirobj->ftime = t2;


    t1 = (word)(dirobj->fs_obj.rtfi.rtfiles.DateTime.Year1980);
    t1 <<= 9;
    t2 = t1;
    t1 = (word)(dirobj->fs_obj.rtfi.rtfiles.DateTime.Month);
    t1 <<= 5;
    t2 |= t1;
    t1 = (word)(dirobj->fs_obj.rtfi.rtfiles.DateTime.Day);
    t2 |= t1;


    dirobj->fdate = t2;
    dirobj->fsize = dirobj->fs_obj.rtfi.rtfiles.FileSize;
}


RTIP_BOOLEAN vfrtfi_gfirst(PVDSTAT dirobj, PFCHAR name)
{
    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));

    dirobj->fs_obj.rtfi.rtfileshandle = 
        RTFFindFirst((PFCHAR )name,
                     0,
                     RTF_ATTR_VOLUME,       /* exclude volumes */
                     &dirobj->fs_obj.rtfi.rtfiles,
                     dirobj->filename);

    if (dirobj->fs_obj.rtfi.rtfileshandle < 0)
        return(FALSE);

    vfrtfi_mapp_attr(dirobj);
    dirobj->patt_match = TRUE;
    return (TRUE);  
}

RTIP_BOOLEAN vfrtfi_gnext(PVDSTAT dirobj)
{
    if (RTFFindNext(dirobj->fs_obj.rtfi.rtfileshandle,&dirobj->fs_obj.rtfi.rtfiles,dirobj->filename) != 0)
        return (FALSE);

    vfrtfi_mapp_attr(dirobj);

    dirobj->patt_match = TRUE;
    return (TRUE);  
}

void vfrtfi_gdone(PVDSTAT dirobj)
{
    if (dirobj->fs_obj.rtfi.rtfileshandle >= 0)
        RTFFindClose(dirobj->fs_obj.rtfi.rtfileshandle);
    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));      
}

RTIP_BOOLEAN vfrtfi_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *filesys, MPT_INITFN initfn, PFVOID init_arg)
{
    ARGSUSED_PVOID(mountpath);
    ARGSUSED_PVOID(nativepath);
    ARGSUSED_PVOID(filesys);
    ARGSUSED_PVOID(initfn);
    ARGSUSED_PVOID(init_arg);
    DEBUG_ERROR("vfrtfi_mountfs: cannot mount onto RTFILE", NOVAR, 0, 0);
    return (FALSE);
}

RTIP_BOOLEAN vfrtfi_unmountfs(PFCHAR mountpath)
{
    ARGSUSED_PVOID(mountpath);
    return (FALSE);
}

#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
/* This function returns a pointer to a string showing the file      */
/* creation date, or it returns null on failure.                     */
/* The format of the string it returns is very important.            */
/* It must be as follows:                                            */
/*                                                                   */
/*                 1111111111222222222                               */
/*       01234567890123456789012345678                               */
/*       DAY, DD MON YEAR hh:mm:ss GMT                               */
/* i.e. "Wed, 04 Oct 1995 22:33:27 GMT";                             */

PFCHAR vfrtfi_modified_date(int handle, PFCHAR buffer, int buflen)
{
    /* Not implemented for rtfs     */
    return((PFCHAR )0);
}
#endif

#if (INCLUDE_STAT)
/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_stat(char *name, PVSTAT stat)
{
RTFHANDLE fd;
RTFDOSDirEntry ertfi_stat;
char b[15];


    fd = RTFFindFirst(name, 0, 0, &ertfi_stat, b);

    if (fd < 0)
        return (FALSE);
    RTFFindClose(fd);


    stat->vs_mode = 0;
    stat->vs_mode |= (ertfi_stat.Attributes & RTF_ATTR_DIR) ? VSMODE_IFDIR:  0;
    if ( (ertfi_stat.Attributes & 
         (RTF_ATTR_HIDDEN|RTF_ATTR_SYSTEM|RTF_ATTR_VOLUME|RTF_ATTR_DIR)) == 0)
        stat->vs_mode |= VSMODE_IFREG;
    if (!(ertfi_stat.Attributes & RTF_ATTR_READ_ONLY))
        stat->vs_mode |= VSMODE_IWRITE;
    stat->vs_mode |= VSMODE_IREAD;

    stat->vs_size = ertfi_stat.FileSize;
    stat->vs_time =
         ( (ertfi_stat.DateTime.Hour << 11) |
          (ertfi_stat.DateTime.Month << 5)  |
           ertfi_stat.DateTime.Day);
    stat->vs_date = 
             ( (ertfi_stat.DateTime.Year1980 << 9) |
             (ertfi_stat.DateTime.Minute << 5) |
              ertfi_stat.DateTime.Day);

    return(TRUE);
}
#endif

#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_chmode(char *name, int attributes)
{
byte rtfi_mode;

    rtfi_mode = 0;

    /* rtfi supports read-only but not write-only; so if write is not         */
    /* permitted, set to read-only                                            */
    if (!(attributes & VS_IWRITE))
        rtfi_mode |= RTF_ATTR_READ_ONLY;
    
    return((RTIP_BOOLEAN) (RTFSetAttributes(name, rtfi_mode)==RTF_NO_ERROR));
}

/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_chsize(int fd, long size)
{
long l;

    l = RTFSeek(fd, 0, RTF_FILE_END);
     if (l > size)
        return(vfrtfi_truncate(fd, size));
     else
     {
        if (RTFSeek(fd, size, RTF_FILE_BEGIN) == size)
     {
            return(TRUE);
        }
    }
   return(FALSE);
}

#endif      /* INCLUDE_NFS_SRV */

#if (INCLUDE_GET_FREE)
/* ********************************************************************     */
RTIP_BOOLEAN vfrtfi_get_free(char *name, dword *blocks, dword *bfree)
{
RTFDiskInfo inf;
    if (RTFGetDiskInfo(name, &inf) != RTF_NO_ERROR)
        return(FALSE);
    *blocks = (dword)inf.SectorsPerCluster;                 /* Total block */
    *blocks = *blocks * (dword)inf.TotalClusters;
    *bfree  = (dword)inf.SectorsPerCluster;                 /* free blocks */
    *bfree  = *bfree * (dword)inf.FreeClusters;

    return(TRUE);
}
#endif


#endif








































