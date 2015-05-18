/*                                                                      */
/* VFAPI.H - Virtual File system API                                    */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */


#ifndef __VFAPI__
#define __VFAPI__ 1

#include "rtip.h"
#include "sock.h"
#include "vfconf.h"

#if (INCLUDE_ERTFS_API)
#ifndef BOOLEAN
typedef int  BOOLEAN;           /* Don't change */
#endif
#include "rtfsapi.h"
#endif

#if (INCLUDE_ERTFS_PRO)
#include "rtfsapi.h"
#endif

#include "ostime.h"

#if (INCLUDE_RTFILES)
#include <rtfiles.h>
#endif

#if (INCLUDE_WIN32_FS)
#if (defined(__BORLANDC__))
#include "io.h"
#include <dir.h>

#elif (defined(__WATCOM__))
#include "io.h"
#endif
#endif

#if (INCLUDE_DOS_FS)
#if (defined(__BORLANDC__) )
#include <dos.h>
#include <conio.h>
#endif
#endif

#if (INCLUDE_DOS_FS && !INCLUDE_WIN32_FS)
#if (defined(__BORLANDC__) && !IS_BC_PM)
#include <dirent.h>
#include <dir.h>
#elif (defined(__WATCOM__))
#include <direct.h>
#endif
#endif

#if (INCLUDE_NFS_CLI)
#include "nc.h"
#endif

/************************************************************************
 * File System API abstraction layer constants                          *
 ************************************************************************/

#define VO_RDONLY  0x0001  /* Open a file for reading only */
#define VO_WRONLY  0x0002  /* Open a file for writing only */
#define VO_RDWR    0x0004  /* Open a file for reading and writing */
#define VO_NDELAY  0x0008
#define VO_APPEND  0x0010  /* Open a file for appending */
#define VO_CREAT   0x0020
#define VO_TRUNC   0x0040
#define VO_EXCL    0x0080
#define VO_BINARY  0x0100
#define VO_TEXT    0x0200
#define VS_IWRITE  0x0001
#define VS_IREAD   0x0002
#define VSEEK_SET 0        /* For fs_fi_lseek - seek from beginning of file */
#define VSEEK_CUR 1        /* For fs_fi_lseek - seek from file pointer */ 
#define VSEEK_END 2        /* For fs_fi_lseek - seek from end of file */

#define VF_BAD_FILE -1     /* return value from vf_open if open failed */

typedef struct vdstat *PVDSTAT;
typedef struct vstat  *PVSTAT;

/************************************************************************
 * VFILEAPI - File system API function pointers                         *
 ************************************************************************/

typedef int  (KS_FAR *MPT_INITFN)(PFVOID argv);
typedef int  (KS_FAR *API_OPENFN)(PFCHAR name, word flag, word mode);
typedef int  (KS_FAR *API_READFN)(int fd,  PFBYTE buf, word count);
typedef int  (KS_FAR *API_WRITEFN)(int fd, PFBYTE buf, word count);
typedef long (KS_FAR *API_LSEEKFN)(int fd, long offset, int origin);
typedef int  (KS_FAR *API_CLOSEFN)(int fd);
typedef RTIP_BOOLEAN (KS_FAR *API_TRUNCATEFN)(int fd, long offset);
typedef RTIP_BOOLEAN (KS_FAR *API_FLUSHFN)(int fd);
typedef RTIP_BOOLEAN (KS_FAR *API_RENAMEFN)(PFCHAR name, PFCHAR newname);
typedef RTIP_BOOLEAN (KS_FAR *API_DELETEFN)(PFCHAR name);
typedef RTIP_BOOLEAN (KS_FAR *API_MKDIRFN)(PFCHAR name);
typedef RTIP_BOOLEAN (KS_FAR *API_RMDIRFN)(PFCHAR name);
typedef RTIP_BOOLEAN (KS_FAR *API_SETCWDFN)(PFCHAR name);
typedef RTIP_BOOLEAN (KS_FAR *API_PWDFN)(PFCHAR name);
typedef RTIP_BOOLEAN (KS_FAR *API_GFIRSTFN)(PVDSTAT dirobj, PFCHAR name);
typedef RTIP_BOOLEAN (KS_FAR *API_GNEXTFN)(PVDSTAT dirobj);
typedef void    (KS_FAR *API_GDONEFN)(PVDSTAT dirobj);
typedef RTIP_BOOLEAN (KS_FAR *API_MOUNTFSFN)(PFCHAR mountpath, PFCHAR nativepath, PFVOID filesys, MPT_INITFN initfn, PFVOID init_arg);
typedef RTIP_BOOLEAN (KS_FAR *API_UNMOUNTFSFN)(PFCHAR mountpath);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
typedef PFCHAR  (KS_FAR *API_GETMODDATE)(int fd,  PFBYTE buf, word count);
#endif
#if (INCLUDE_STAT)
typedef RTIP_BOOLEAN (KS_FAR *API_STATFN)(PFCHAR name, PVSTAT stat);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
typedef RTIP_BOOLEAN (KS_FAR *API_CHMODEFN)(PFCHAR name, int attributes);
typedef RTIP_BOOLEAN (KS_FAR *API_CHSIZEFN)(int fd, long size);
#endif
#if (INCLUDE_GET_FREE)
typedef RTIP_BOOLEAN (KS_FAR *API_GET_FREEFN)(PFCHAR name, dword *blocks, dword *bfree);
#endif

typedef struct vfileapi 
{
    API_OPENFN      fs_open;
    API_READFN      fs_read;
    API_WRITEFN     fs_write;
    API_LSEEKFN     fs_lseek;
    API_TRUNCATEFN  fs_truncate;
    API_FLUSHFN     fs_flush;
    API_CLOSEFN     fs_close;
    API_RENAMEFN    fs_rename;
    API_DELETEFN    fs_delete;
    API_MKDIRFN     fs_mkdir;
    API_RMDIRFN     fs_rmdir;
    API_SETCWDFN    fs_set_cwd;
    API_PWDFN       fs_pwd;
    API_GFIRSTFN    fs_gfirst;
    API_GNEXTFN     fs_gnext;
    API_GDONEFN     fs_gdone;
    API_MOUNTFSFN   fs_mountfs;
    API_UNMOUNTFSFN fs_unmountfs;
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
    API_GETMODDATE  fs_get_file_modified_date;
#endif
#if (INCLUDE_STAT)
    API_STATFN      fs_stat;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
    API_CHMODEFN    fs_chmode;
    API_CHSIZEFN    fs_chsize;
#endif
#if (INCLUDE_GET_FREE)
    API_GET_FREEFN  fs_get_free;
#endif
} VFILEAPI;
typedef VFILEAPI KS_FAR *PVFILEAPI;

/************************************************************************
 * VFSTAT - File Status                                                 *
 ************************************************************************/
typedef struct vstat
{
    /* Values for the vs_mode field   */
    #define VSMODE_IFDIR  0x01  /* directory */
    #define VSMODE_IFREG  0x02  /* regular */
    #define VSMODE_IWRITE 0x04  /* Write permitted  */
    #define VSMODE_IREAD  0x08  /* Read permitted. */
    int      vs_mode;  

    dword    vs_size;       /* file size, in bytes */
    word     vs_date;       /* last access date */
    word     vs_time;       /* last access time */
    dword    vs_etime;      /* elapsed time since 1/1/70 */
} VSTAT;

#define VF_IS_DIR(PATH, STAT)   ( vf_stat(PATH, (STAT)) &&             \
                                  ((STAT)->vs_mode & VSMODE_IFDIR) )
#define VF_IS_FILE(PATH, STAT)  ( vf_stat(PATH, (STAT)) &&             \
                                  ((STAT)->vs_mode & VSMODE_IFREG) )
#define VF_EXISTS(PATH, STAT)   ( vf_stat(PATH, (STAT)) &&             \
                                  (((STAT)->vs_mode & VSMODE_IFDIR) || \
                                  ((STAT)->vs_mode & VSMODE_IFREG)) )
#define VF_IS_WRITE(PATH, STAT) ( vf_stat(PATH, (STAT)) &&             \
                                  ((STAT)->vs_mode & VSMODE_IWRITE) )
#define VF_IS_READ(PATH, STAT)  ( vf_stat(PATH, (STAT)) &&             \
                                  ((STAT)->vs_mode & VSMODE_IREAD) )

/************************************************************************
 * VFDSTAT - Directory Iterator; used for directory scans               *
 ************************************************************************/

#define VF_ATTRIB_ISROOT 0x0001
#define VF_ATTRIB_ISDIR  0x0002
#define VF_ATTRIB_ISVOL  0x0004
#define VF_ATTRIB_RDONLY 0x0008
#define VF_ATTRIB_WRONLY 0x0010
#define VF_ATTRIB_RDWR   0x0020

#ifndef find_t
#define find_t _find_t
#endif

typedef struct vdstat 
{
    PVFILEAPI fs_api;
    char      filename[VF_FILENAMESIZE];
    RTIP_BOOLEAN   patt_match;               /* set it pattern matched when gfirst,
                                            gnext returned a valid file */
    word      fattributes;
    EBSTIME   ebs_mod_time; /* time file last modified  */
    dword     ftime;
    dword     fdate;
    dword     fsize;

    /* ********************************************************************   */
    /*
     * union fs_obj: ETS: etshandle & .ets collide. 
                 VFS+???: VFS elements + ??? elements collide.
        */
    struct 
    {
        char         pattern[VF_FILENAMESIZE];
        void KS_FAR *pdir;
        void KS_FAR *pmom;
    } fs_obj_vfs;

    /* ********************************************************************   */
    union
    {
#if (INCLUDE_RTFILES)
        struct 
        {
            int rtfileshandle;
            RTFDOSDirEntry rtfiles;
        }   rtfi;
#endif

#if (INCLUDE_ERTFS_API)
        DSTAT rtfs;
#endif

#if (defined(ETS) && !defined(__BORLANDC__))
        struct
        {
            long               etshandle;
            struct _finddata_t ets;
        } e;

#elif (INCLUDE_WIN32_FS)
#if (defined(__BORLANDC__))
        struct
        {
            long          win32handle;
            struct ffblk  win32;
        }w;   
#endif

#elif (INCLUDE_DOS_FS)
        struct
        {
#if (defined(__BORLANDC__))
            struct ffblk  dos;
#endif      
/*            long int doshandle;   */
        } d;
#endif      /* DOS_FS */

    /* ********************************************************************   */
#if (INCLUDE_NFS_CLI)
        struct 
        {
            char          pattern[VF_FILENAMESIZE];
            char          path[VF_MAXPATHLEN];
            readdir_stats stat;
        } nfs;
#endif            
      
        int dummyfs;    /* prevent compiler errors if no other FS's */
    } fs_obj;
} VDSTAT;

/* ********************************************************************   */
/* EXTERN                                                                 */
/* ********************************************************************   */
extern PVFILEAPI vf_api;

/* ********************************************************************   */
/* These macros call the function in the file system API structure to
   perform the operation. */
#define _vf_open(API,A,B,C)        ((API)? (API)->fs_open(A,B,C)        :0)
#define _vf_read(API,A,B,C)        ((API)? (API)->fs_read(A,B,C)        :0)
/*#define _vf_readln(API,A,B,C)      ((API)? (API)->fs_readln(A,B,C)      :0)   */
#define _vf_readln(API,A,B,C)      ((API)? vf_fgets((API), A, B, C)     :0)
#define _vf_write(API,A,B,C)       ((API)? (API)->fs_write(A,B,C)       :0)
#define _vf_lseek(API,A,B,C)       ((API)? (API)->fs_lseek(A,B,C)       :0)
#define _vf_length(API,A)          ((API)? (API)->fs_length(A)          :0)
#define _vf_truncate(API,A,B)      ((API)? (API)->fs_truncate(A,B)      :0)
#define _vf_flush(API,A)           ((API)? (API)->fs_flush(A)           :0)
#define _vf_close(API,A)           ((API)? (API)->fs_close(A)           :0)
#define _vf_rename(API,A,B)        ((API)? (API)->fs_rename(A,B)        :0)
#define _vf_delete(API,A)          ((API)? (API)->fs_delete(A)          :0)
#define _vf_mkdir(API,A)           ((API)? (API)->fs_mkdir(A)           :0)
#define _vf_rmdir(API,A)           ((API)? (API)->fs_rmdir(A)           :0)
#define _vf_set_cwd(API,A)         ((API)? (API)->fs_set_cwd(A)         :0)
#define _vf_pwd(API,A)             ((API)? (API)->fs_pwd(A)             :0)
#define _vf_gfirst(API,A,B)        ((API)? (API)->fs_gfirst(A,B)        :0)
#define _vf_gnext(API,A)           ((API)? (API)->fs_gnext(A)           :0)
#define _vf_mountfs(API,A,B,C,D,E) ((API)? (API)->fs_mountfs(A,B,C,D,E) :0)
#define _vf_unmountfs(API,A)       ((API)? (API)->fs_unmountfs(A)       :0)
#define _vf_gdone(API,A)           { if (API) (API)->fs_gdone(A); }
#define _vf_rewind(API,A)          _vf_lseek((API),A,0,VSEEK_SET)
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
#define _vf_get_file_modified_date(API,A,B,C) ((API)? (API)->fs_get_file_modified_date(A,B,C)   :0)
#endif
#if (INCLUDE_STAT)
#define _vf_stat(API,A,B)          ((API)? (API)->fs_stat(A,B)          :0)
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
#define _vf_chmode(API,A,B)        ((API)? (API)->fs_chmode(A,B)        :0)
#define _vf_get_free(API,A,B,C)    ((API)? (API)->fs_get_free(A,B,C)    :0)
#define _vf_chsize(API,A,B)        ((API)? (API)->fs_chsize(A,B)        :0)
#endif

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

int     vf_init(void);
PFCHAR  vf_fgets(PVFILEAPI p_api, int fd, PFCHAR buf, word length);
dword   vf_filelength(PVFILEAPI p_api,int fd);
int     vdstat_isdir(word flags);
RTIP_BOOLEAN vf_build_virtual_file(char *drive_path, char *filename, PFBYTE buf, long buf_len);
RTIP_BOOLEAN mf_build_file(char *path, char *filename, PFBYTE buf, long buf_len);

#ifdef __cplusplus
}
#endif

/* These are the macros as they are used in most of the system. The web server
   uses the _vf_ macros so it can give precedence to the compiled in pages */
#define vf_open(A,B,C)          _vf_open(vf_api, A,B,C)
#define vf_read(A,B,C)          _vf_read(vf_api, A,B,C)
#define vf_readln(A,B,C)        _vf_readln(vf_api, A,B,C)
#define vf_write(A,B,C)         _vf_write(vf_api, A,B,C)
#define vf_lseek(A,B,C)         _vf_lseek(vf_api, A,B,C)
#define vf_length(A)            _vf_length(vf_api, A)
#define vf_truncate(A,B)        _vf_truncate(vf_api, A,B)
#define vf_flush(A)             _vf_flush(vf_api, A)
#define vf_close(A)             _vf_close(vf_api, A)
#define vf_rename(A,B)          _vf_rename(vf_api, A,B)
#define vf_delete(A)            _vf_delete(vf_api, A)
#define vf_mkdir(A)             _vf_mkdir(vf_api, A)
#define vf_rmdir(A)             _vf_rmdir(vf_api, A)
#define vf_set_cwd(A)           _vf_set_cwd(vf_api, A)
#define vf_pwd(A)               _vf_pwd(vf_api, A)
#define vf_gfirst(A,B)          _vf_gfirst(vf_api, A,B)
#define vf_gnext(A)             _vf_gnext(vf_api, A)
#define vf_mountfs(A,B,C,D,E)   _vf_mountfs(vf_api, A,B,C,D,E)
#define vf_unmountfs(A)         _vf_unmountfs(vf_api, A)
#define vf_gdone(A)             _vf_gdone(vf_api, A)
#define vf_rewind(A)            _vf_rewind(vf_api, A)
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
#define vf_get_file_modified_date(A,B,C) _vf_get_file_modified_date(vf_api, A,B,C)
#endif
#if (INCLUDE_STAT)
#define vf_stat(A,B)            _vf_stat(vf_api, A,B)
#endif
#if (INCLUDE_GET_FREE)
#define vf_get_free(A,B,C)      _vf_get_free(vf_api, A,B,C)
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
#define vf_chmode(A,B)          _vf_chmode(vf_api, A,B)
#define vf_chsize(A,B)          _vf_chsize(vf_api, A,B)
#endif

#endif         /* VFAPI */

