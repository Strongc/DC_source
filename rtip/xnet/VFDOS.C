/*                                                                      */
/* VFDOS.C - MS-DOS driver for EBS Virtual File System                  */
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

#if (INCLUDE_DOS_FS || INCLUDE_WIN32_FS)
#include <stdio.h>
#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <time.h>
#include <sys\stat.h>
#include <direct.h>

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_DOSFS_API     0
#define DEBUG_DOSFS_OPEN    0
#define DEBUG_DOSFS_CHDIR   0
#define DEBUG_DOSFS_MKDIR   0


/* ********************************************************************   */
/* ********************************************************************   */
int     vfdos_open(PFCHAR name, word flag, word mode);
int     vfdos_read(int fd,  PFBYTE buf, word count);
int     vfdos_write(int fd,  PFBYTE buf, word count);
int     vfdos_close(int fd);
long    vfdos_lseek(int fd, long offset, int origin);
RTIP_BOOLEAN vfdos_truncate(int fd, long offset);
RTIP_BOOLEAN vfdos_flush(int fd);
RTIP_BOOLEAN vfdos_pwd(PFCHAR to);
RTIP_BOOLEAN vfdos_rename(PFCHAR from, PFCHAR to);
RTIP_BOOLEAN vfdos_remove(PFCHAR to);
RTIP_BOOLEAN vfdos_mkdir(PFCHAR to);
RTIP_BOOLEAN vfdos_chdir(PFCHAR to);
RTIP_BOOLEAN vfdos_rmdir(PFCHAR to);
RTIP_BOOLEAN vfdos_gfirst(PVDSTAT dirobj, PFCHAR name);
RTIP_BOOLEAN vfdos_gnext(PVDSTAT dirobj);
void    vfdos_gdone(PVDSTAT dirobj);
RTIP_BOOLEAN vfdos_mountfs(PFCHAR mountpath, PFCHAR nativepath, void *pfilesys, MPT_INITFN initfn, PFVOID init_arg);
void    vfdos_unmountfs(PFCHAR mountpath);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
PFCHAR  vfdos_modified_date(int fd, PFCHAR buffer, int buflen);
#endif
#if (INCLUDE_STAT)
RTIP_BOOLEAN vfdos_stat(PFCHAR name, PVSTAT vstat);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN vfdos_chmode(PFCHAR name, int attributes);
RTIP_BOOLEAN vfdos_get_free(PFCHAR name, dword *blocks, dword *bfree);
RTIP_BOOLEAN vfdos_chsize(int fd, long size);
#endif

/* ********************************************************************   */
/* vfdos_api_init - Inintialize a filesystem API structure tht allows
*  mounting MSDOS as a node of a virtual file tree.
*
*  This routine takes an API structure and assigns functions to
*  the function pointer in the structure. The functions include
*  open close read write close mkdir rmdir etc.
*/
    
void vfdos_api_init(VFILEAPI *papi)
{
    papi->fs_open       =   (API_OPENFN)      vfdos_open;
    papi->fs_read       =   (API_READFN)      vfdos_read;
    papi->fs_write      =   (API_WRITEFN)     vfdos_write;
    papi->fs_lseek      =   (API_LSEEKFN)     vfdos_lseek;
    papi->fs_truncate   =   (API_TRUNCATEFN)  vfdos_truncate;
    papi->fs_flush      =   (API_FLUSHFN)     vfdos_flush;
    papi->fs_close      =   (API_CLOSEFN)     vfdos_close;
    papi->fs_rename     =   (API_RENAMEFN)    vfdos_rename;
    papi->fs_delete     =   (API_DELETEFN)    vfdos_remove;
    papi->fs_mkdir      =   (API_MKDIRFN)     vfdos_mkdir;
    papi->fs_rmdir      =   (API_RMDIRFN)     vfdos_rmdir;
    papi->fs_set_cwd    =   (API_SETCWDFN)    vfdos_chdir;
    papi->fs_pwd        =   (API_PWDFN)       vfdos_pwd;
    papi->fs_gfirst     =   (API_GFIRSTFN)    vfdos_gfirst;
    papi->fs_gnext      =   (API_GNEXTFN)     vfdos_gnext;
    papi->fs_gdone      =   (API_GDONEFN)     vfdos_gdone;
    papi->fs_mountfs    =   (API_MOUNTFSFN)   vfdos_mountfs;
    papi->fs_unmountfs  =   (API_UNMOUNTFSFN) vfdos_unmountfs;
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
    papi->fs_get_file_modified_date     =
                            (API_GETMODDATE)  vfdos_modified_date;
#endif
#if (INCLUDE_STAT)
    papi->fs_stat       =   (API_STATFN)      vfdos_stat;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
    papi->fs_chmode     =   (API_CHMODEFN)    vfdos_chmode;
    papi->fs_get_free   =   (API_GET_FREEFN)  vfdos_get_free;
    papi->fs_chsize     =   (API_CHSIZEFN)    vfdos_chsize;
#endif

}

/* ********************************************************************   */
int vfdos_open(PFCHAR name, word flag, word mode)
{
unsigned int f,m;
    
#if (DEBUG_DOSFS_API || DEBUG_DOSFS_OPEN)
char to[80];
    DEBUG_ERROR("vfdos_open: open file ", STR1, name, 0);
    if (vfdos_pwd(to))
    {
        DEBUG_ERROR("vfdos_open: dos pwd ", STR1, to, 0);
    }
#endif
    f = 0;
    f |= O_BINARY; /* Always binary */

    f |= (flag & VO_RDONLY) ? O_RDONLY: 0;
    f |= (flag & VO_WRONLY) ? O_WRONLY: 0;
    f |= (flag & VO_RDWR)   ? O_RDWR:   0;
    f |= (flag & VO_APPEND) ? O_APPEND: 0;
    f |= (flag & VO_CREAT)  ? O_CREAT:  0;
    f |= (flag & VO_TRUNC)  ? O_TRUNC:  0;
    f |= (flag & VO_EXCL)   ? O_EXCL:   0;
    f |= (flag & VO_TEXT)   ? O_TEXT:   0;

    m = 0;
    m |= (mode & VS_IWRITE) ? S_IWRITE: 0;
    m |= (mode & VS_IREAD)  ? S_IREAD:  0;

    return ( open (name, (word)f, (word)m) );
}

/* ********************************************************************   */
int vfdos_read(int fd,  PFBYTE buf, word count)
{
#if (DEBUG_DOSFS_API)
    DEBUG_ERROR("vfdos_read: ", NOVAR, 0, 0);
#endif

    return(read(fd, buf, count));
}

/* ********************************************************************   */
int vfdos_write(int fd,  PFBYTE buf, word count)
{
    return(write(fd, buf, count));
}

/* ********************************************************************   */
int vfdos_close(int fd)
{
    return(close(fd));
}

/* ********************************************************************   */
long vfdos_lseek(int fd, long offset, int origin)
{
int o;

    switch (origin)
    {
        case VSEEK_SET:
            o = SEEK_SET;
            break;

        case VSEEK_CUR:
            o = SEEK_CUR;
            break;

        case VSEEK_END:
            o = SEEK_END;
            break;
    
        default:
            o = SEEK_SET;
    }

    return ( lseek (fd, offset, o) );
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_truncate(int fd, long offset)
{
    return (FALSE);
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_flush(int fd)
{
int fd2;

    /* trick to flush file   */
    fd2 = dup(fd);
    if (fd2 == -1)
        return(FALSE);
    close(fd2);
    return (TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_rename(PFCHAR from, PFCHAR to)
{
    return((RTIP_BOOLEAN)(rename(from, to)==0));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_remove(PFCHAR d)
{
    return((RTIP_BOOLEAN)(unlink(d)==0));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_mkdir(PFCHAR d)
{
#if (DEBUG_DOSFS_MKDIR)
    DEBUG_ERROR("vfdos_mkdir: ", STR1, d, 0);
#endif
    return((RTIP_BOOLEAN)(mkdir(d)==0));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_rmdir(PFCHAR d)
{
    return((RTIP_BOOLEAN)(rmdir(d)==0));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_chdir(PFCHAR to)
{
int currdrive;
char dr;
PFCHAR p;
char temp[2];

#if (DEBUG_DOSFS_API || DEBUG_DOSFS_CHDIR)
DEBUG_ERROR("vfdos_chdir: to ", STR1, to, 0);
#endif

    /* If the path contains a drive designator change to it   */
    if (*(to+1) == ':')
    {
        /* Save the current drive designator in case the chdir fails   */
        dr = *to;
        if (dr >= 'a') 
            dr = (char) (dr-'a' + 'A');
        currdrive = _getdrive();

        /* Change the drive   */
        if (_chdrive(dr-'A'+1)!=0)
            return(FALSE);
        to = to + 2;

        /* If there is no path component (just C: then cd to C:\   */
        if (*to)
            p = to;
        else
        {
            temp[0] = '\\';
            temp[1] = 0;
            p = &temp[0];
        }
        /* Change the path   */
        if ((RTIP_BOOLEAN)(chdir(p)!=0))
        {
            /* Change path failed so restore the drive assignment   */
            _chdrive(currdrive);
            return(FALSE);
        }
        else
            return(TRUE);
    }
    else
        return((RTIP_BOOLEAN)(chdir(to)==0));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_pwd(PFCHAR to)
{
    if (getcwd(to, 80))
        return(TRUE);
    else
        return (FALSE);
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_gfirst(PVDSTAT dirobj, PFCHAR name)
{
word flags;
                           
    tc_memset(&dirobj->fs_obj, 0, sizeof(dirobj->fs_obj));
                           
#if (INCLUDE_WIN32_FS)

#if (defined(__BORLANDC__))
    if (findfirst(name, &dirobj->fs_obj.w.win32, FA_DIREC) != 0)
        return (FALSE);

    tc_strncpy(dirobj->filename, dirobj->fs_obj.w.win32.ff_name, VF_FILENAMESIZE);
    flags = (word)dirobj->fs_obj.w.win32.ff_attrib;
    dirobj->fattributes  = (word)((flags & FA_DIREC)?  VF_ATTRIB_ISDIR:  0);
    dirobj->fattributes |= (word)((flags & FA_LABEL)?  VF_ATTRIB_ISVOL:  0);
    dirobj->fattributes |= (word)((flags & FA_RDONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR);
    dirobj->ftime = dirobj->fs_obj.w.win32.ff_ftime;
    dirobj->fdate = dirobj->fs_obj.w.win32.ff_fdate;
    dirobj->fsize = dirobj->fs_obj.w.win32.ff_fsize;

#endif

#else /* DOS */
#if (defined(__BORLANDC__))
    if (findfirst(name, &dirobj->fs_obj.d.dos, FA_DIREC) != 0)
        return (FALSE);

    tc_strncpy(dirobj->filename, dirobj->fs_obj.d.dos.ff_name, VF_FILENAMESIZE);
    flags = (word)dirobj->fs_obj.d.dos.ff_attrib;
    dirobj->fattributes  = (word)((flags & FA_DIREC)?  VF_ATTRIB_ISDIR:  0);
    dirobj->fattributes |= (word)((flags & FA_LABEL)?  VF_ATTRIB_ISVOL:  0);
    dirobj->fattributes |= (word)((flags & FA_RDONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR);
    dirobj->ftime = dirobj->fs_obj.d.dos.ff_ftime;
    dirobj->fdate = dirobj->fs_obj.d.dos.ff_fdate;
    dirobj->fsize = dirobj->fs_obj.d.dos.ff_fsize;

#else
#error - implement gfirst in vfdos.c
#endif

#endif
    dirobj->patt_match = TRUE;      /* pattern matching is already done */
                                    /* by DOS    */
    return (TRUE);  
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_gnext(PVDSTAT dirobj)
{
    word flags;

#if (INCLUDE_WIN32_FS)

#if (defined(__BORLANDC__))
    dirobj->fs_obj.w.win32.attrib = _A_NORMAL|_A_SUBDIR;
    if (_findnext(dirobj->fs_obj.w.win32handle, &dirobj->fs_obj.w.win32) != 0)
        return (FALSE);

    tc_strncpy(dirobj->filename, dirobj->fs_obj.w.win32.ff_name, VF_FILENAMESIZE);
    flags = dirobj->fs_obj.w.win32.ff_attrib;
    dirobj->fattributes  = (flags & FA_DIREC)?  VF_ATTRIB_ISDIR:  0;
    dirobj->fattributes |= (flags & FA_LABEL)?  VF_ATTRIB_ISVOL:  0;
    dirobj->fattributes |= (flags & FA_RDONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR;
    dirobj->ftime = dirobj->fs_obj.w.win32.ff_ftime;
    dirobj->fdate = dirobj->fs_obj.w.win32.ff_fdate;
    dirobj->fsize = dirobj->fs_obj.w.win32.ff_fsize;

#endif

#else /* DOS */

#if (defined(__BORLANDC__))
    if (findnext(&dirobj->fs_obj.d.dos) != 0)
        return (FALSE);

    tc_strncpy(dirobj->filename, dirobj->fs_obj.d.dos.ff_name, VF_FILENAMESIZE);
    flags = dirobj->fs_obj.d.dos.ff_attrib;
    dirobj->fattributes  = (flags & FA_DIREC)?  VF_ATTRIB_ISDIR:  0;
    dirobj->fattributes |= (flags & FA_LABEL)?  VF_ATTRIB_ISVOL:  0;
    dirobj->fattributes |= (flags & FA_RDONLY)? VF_ATTRIB_RDONLY: VF_ATTRIB_RDWR;
    dirobj->ftime = dirobj->fs_obj.d.dos.ff_ftime;
    dirobj->fdate = dirobj->fs_obj.d.dos.ff_fdate;
    dirobj->fsize = dirobj->fs_obj.d.dos.ff_fsize;

#else
#error - implement gfirst
#endif

#endif

    dirobj->patt_match = TRUE;
    return (TRUE);  
}

/* ********************************************************************   */
void vfdos_gdone(PVDSTAT dirobj)
{
    ARGSUSED_PVOID(dirobj);
#if (INCLUDE_WIN32_FS)
    if (dirobj->fs_obj.w.win32handle)
        _findclose(dirobj->fs_obj.w.win32handle);
#endif        

    tc_memset(&dirobj->fs_obj,0,sizeof(dirobj->fs_obj));
}

/* ********************************************************************   */
RTIP_BOOLEAN vfdos_mountfs(PFCHAR mountpath, PFCHAR nativepath,  void * filesys, MPT_INITFN initfn, PFVOID init_arg)
{
    ARGSUSED_PVOID(mountpath);
    ARGSUSED_PVOID(nativepath);
    ARGSUSED_PVOID(filesys);
    ARGSUSED_PVOID(initfn);
    ARGSUSED_PVOID(init_arg);
    DEBUG_ERROR("vfdos_mountfs: cannot mount onto DOS", NOVAR, 0, 0);
    return (FALSE);
}

void vfdos_unmountfs(PFCHAR mountpath)
/* ********************************************************************   */
{
    ARGSUSED_PVOID(mountpath);
}

#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
/* ********************************************************************   */
/* convert time returned by ctime (tstr) to time value used by web        */
/* (time_str)                                                             */
/* It must be as follows:                                                 */
/*                                                                        */
/*                 1111111111222222222                                    */
/*       01234567890123456789012345678                                    */
/*       DAY, DD MON YEAR hh:mm:ss GMT                                    */
/* i.e. "Wed, 04 Oct 1995 22:33:27 GMT";                                  */
/*                                                                        */
void f_conv_time(PFCHAR time_string, PFCHAR tstr)
{
     /* First get the day name.   */
     tc_strncpy(time_string, tstr, 3);
     time_string[3] = '\0';
     tc_strcat(time_string, ", ");

     /* Now the day of the month   */
     tc_strncat(time_string, tstr+8, 3);

     /* Now the Month name   */
     tc_strncat(time_string, tstr+4, 4);

     /* Now the year   */
     tc_strncat(time_string, tstr+20, 4);
     tc_strcat(time_string, " ");

     /* Now the time   */
     tc_strncat(time_string, tstr+11, 9);

     tc_strcat(time_string, "GMT");
}

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

PFCHAR vfdos_modified_date(int handle, PFCHAR buffer, int buflen)
{
char time_string[35] = "\0";
PFCHAR tstr;
struct stat statbuf;
EBSTIME ebs_time;
    
    /* get information about the file   */
    fstat(handle, &statbuf);

    tstr = ctime(&statbuf.st_ctime);

    f_conv_time(time_string, tstr);

    tc_strncpy(buffer, time_string, buflen);

    return buffer;
}
#endif /* (INCLUDE_WEB || INCLUDE_FTP_SRV) */


#if (INCLUDE_STAT)
/* ********************************************************************   */

RTIP_BOOLEAN vfdos_stat(PFCHAR name, PVSTAT vstat)
{
struct stat dos_status;
struct tm *tme;

    if (stat(name, &dos_status) == -1)
    {
        DEBUG_ERROR("vfdos_stat failed for file ", STR1, name, 0);
        return(FALSE);
    }

    vstat->vs_mode = 0;
    vstat->vs_mode |= (dos_status.st_mode & S_IFDIR) ? VSMODE_IFDIR:  0;
    vstat->vs_mode |= (dos_status.st_mode & S_IFREG) ? VSMODE_IFREG:  0;
    vstat->vs_mode |= (dos_status.st_mode & S_IWRITE)? VSMODE_IWRITE: 0;
    vstat->vs_mode |= (dos_status.st_mode & S_IREAD) ? VSMODE_IREAD:  0;

    vstat->vs_size = dos_status.st_size;    /* file size, in bytes */

    vstat->vs_etime = dos_status.st_atime;

    tme = localtime((const time_t *)&(vstat->vs_etime));

    if (tme)
    {
        vstat->vs_date  = (word)(((tme->tm_mon+1) & 0xf) << 5); /* Month */
        vstat->vs_date |= (word)(tme->tm_mday & 0x1f);          /* Day */
        vstat->vs_date |= (word)(((tme->tm_year-80) & 0xff) << 9);  /* Year */
        vstat->vs_time  = (word)((tme->tm_hour & 0x1f) << 11);  /* Hour */
        vstat->vs_time |= (word)((tme->tm_min  & 0x3f) << 5);   /* Minute */
    }
    else
    {
        DEBUG_ERROR("vfdos_stat: localtime failed", NOVAR, 0, 0);
        vstat->vs_date  = 0;
        vstat->vs_time  = 0;
    }

    return(TRUE);
}
#endif

#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
/* ********************************************************************   */
RTIP_BOOLEAN vfdos_chmode(PFCHAR name, int attributes)
{
int mode;

    mode = 0;
    if (attributes & VS_IWRITE)
        mode |= S_IWRITE;
    if (attributes & VS_IREAD)
        mode |= S_IREAD;

    return((RTIP_BOOLEAN)(chmod(name, mode) == 0));
}
/* ********************************************************************   */
RTIP_BOOLEAN vfdos_chsize(int fd, long size)
{
    return((RTIP_BOOLEAN)(chsize(fd, size) == 0));
}

#endif      /* INCLUDE_NFS_SRV */

#if (INCLUDE_GET_FREE)
/* ********************************************************************   */
#if (defined(__BORLANDC__))
/* drive parameter is the path which always starts with x:   */
RTIP_BOOLEAN vfdos_get_free(PFCHAR name, dword *blocks, dword *bfree)
{
struct dfree dtable_entry;
char         drive;
int          drive_no;

    drive = name[0];
    if ( (drive >= 'A') && (drive <= 'Z') )
        drive_no = drive - 'A' + 1;
    else if ( (drive >= 'a') && (drive <= 'z') )
        drive_no = drive - 'a' + 1;
    else
    {
        DEBUG_ERROR("dos_get_free: drive not in range: name = ", STR1,
            name, 0);
        drive_no = 0;
    }

    getdfree((unsigned char)drive_no, &dtable_entry);

    *blocks = (dtable_entry.df_total * dtable_entry.df_bsec) / VF_BLK_SIZE;
    *bfree  = (dtable_entry.df_avail * dtable_entry.df_bsec) / VF_BLK_SIZE;
    return(TRUE);   /* tbd */
}
#endif
#endif /* INCLUDE_GET_FREE */

#endif      /* INCLUDE_DOS_FS */
