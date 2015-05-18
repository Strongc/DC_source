/*                                                                      */
/* MFAPI.H - Definititions for memory file driver for                   */
/*           the EBS Virtual File System                                */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */


#ifndef __MFAPI__
#define __MFAPI__ 1

#include "sock.h"
#include "vfconf.h"
#include "vfapi.h"
#include "ostime.h"

/************************************************************************
 * MFVNODE - File system mountpoint                                     *
 ************************************************************************/

typedef struct mfvnode 
{
    /* Path of the entry point in the mounted file system   */
    char native_path[CFG_MF_MAXPATH];
    int  native_pathlen;

    /* Mount point initialization information   */
    MPT_INITFN init;
    PFVOID     init_arg;

    /* API of the file system mounted here   */
    PVFILEAPI filesys;
} MFVNODE;
typedef MFVNODE KS_FAR *PMFVNODE;


/************************************************************************
 * MFBLOCK - File block structure                                       *
 ************************************************************************/

#define MF_BLOCK_STATIC 0x01
#define MF_BLOCK_RDONLY 0x02

typedef struct mfblock 
{
#if (INCLUDE_BLOCKS)
    struct mfblock KS_FAR *pprev;
    struct mfblock KS_FAR *pnext;
#endif /* INCLUDE_BLOCKS */
    dword  blsize;
    byte   flags;
    PFBYTE data;
} MFBLOCK;
typedef MFBLOCK KS_FAR *PMFBLOCK;

#if (INCLUDE_CALLBACK)
typedef void (KS_FAR *CALLBACK_OPENFN)(struct mfdirent KS_FAR *pdir, const char *name, word flag, word mode);
typedef int  (KS_FAR *CALLBACK_READFN)(struct mfdirent KS_FAR *pdir,  PFBYTE buf, word count);
typedef int  (KS_FAR *CALLBACK_WRITEFN)(struct mfdirent KS_FAR *pdir, PFBYTE buf, word count);
typedef long (KS_FAR *CALLBACK_LSEEKFN)(struct mfdirent KS_FAR *pdir, long offset, int origin);

typedef void (KS_FAR *CALLBACK_CLOSEFN)(struct mfdirent KS_FAR *pdir);
#endif


/************************************************************************
 * MFDIRENT - Memory Filesystem Directory object structure              *
 ************************************************************************/

#define MF_FLAGS_ISROOT  0x0001 /* Is the root. */
#define MF_FLAGS_ISDIR   0x0002 /* Is a dir. not a file */
#define MF_FLAGS_ISFILE  0x0004 /* Is a file. not a dir */
#define MF_FLAGS_ISDOT   0x0008 /* pprev points to parent root == ISDOT, pprev==0*/
#define MF_FLAGS_STATIC  0x0010 /* File. Storage is never allocated dynamically */
#define MF_FLAGS_ISCLONE 0x0020 /* File. This is a copy of a dirent. */
#define MF_FLAGS_VNODE   0x0040 /* This is a mount point for another filesystem */
#define MF_FLAGS_RDONLY  0x0080 
#define MF_FLAGS_WRONLY  0x0100
#define MF_FLAGS_ORDONLY 0x0200 
#define MF_FLAGS_OWRONLY 0x0400

typedef struct mfdirent 
{
        char   fname[CFG_MF_NAMESIZE];
#if (INCLUDE_FILES)
        struct mfdirent KS_FAR *pprev;
        struct mfdirent KS_FAR *pnext;
#endif
        struct mfdirent KS_FAR *pparent;

        union 
        {
            struct mfdirent KS_FAR *pchild;
            struct mfblock  KS_FAR *pdata;
            struct mfvnode  KS_FAR *pvnode;
        } contents;

        word  flags;            /* File attributes */
        int   mfhandle;         /* use this as a lookup */ 
        int   opencount;        /* number of times open */
        EBSTIME ebs_mod_time;   /* time file last modified  */
        word  ftime;            /* time & date last modified */
        word  fdate;            /* time & date last modified */
        dword fsize;            /* File size amount written */
        dword fptr;
        dword findex;           /* index into pdata of fptr */

#if (INCLUDE_STATIC)
        MFBLOCK    s_data;
#endif

#if (INCLUDE_CALLBACK)
        CALLBACK_OPENFN  openfn;
        CALLBACK_CLOSEFN closefn;
        CALLBACK_READFN  readfn;
        CALLBACK_WRITEFN writefn;
        CALLBACK_LSEEKFN seekfn;
        PFVOID           callback_data;
#endif /* INCLUDE_CALLBACK */

        } MFDIRENT;
typedef MFDIRENT KS_FAR *PMFDIRENT;


/************************************************************************
 * MFVFILE - Virtual file structure                                     *
 ************************************************************************/

typedef struct mfvfile {
        int fd;
        PMFVNODE pvnode;
        } MFVFILE;
typedef MFVFILE KS_FAR *PMFVFILE;


typedef byte KS_FAR * (*MF_ALLOCATOR)(int);
typedef void (*MF_DEALLOCATOR)(byte KS_FAR *);

/************************************************************************
 * MFSYSTEM - File system context structure                             *
 ************************************************************************/

/* This is the system block used by the memory file system. It must
   be initialized before using the system. The block contains a free
   list of DIRENTS and calls to allocate and free blocks. If the routines
   to allocate and free  blocks are NULL then the system can only use
   assigned linear memory */
typedef struct mfsystem 
{

#if (INCLUDE_SUBDIRS)
        char           *mfcwd;
#endif

#if (INCLUDE_FILES)
        PMFDIRENT       mfroot;
        PMFDIRENT       pdirent_array;
        PMFDIRENT       pdirent_pool;
        int             ntotal_dirents;
        int             navail_dirents;
#else
        MFDIRENT        mf_dirent;
#endif

#if (INCLUDE_MTPOINTS)
        PMFVNODE        pvnode_array;
        PMFVFILE        pvfile_table;
        int             ntotal_vnodes;
        int             ntotal_vfiles;
        int             vf_base;
#endif

#if (INCLUDE_DYNAMIC)
        dword          mf_block_size;
        MF_ALLOCATOR   mf_alloc_mem;   /* Called to allocate blocks */
        MF_DEALLOCATOR mf_free_mem;  /* Called to free blocks */
#endif

} MFSYSTEM;
typedef MFSYSTEM KS_FAR *PMFSYSTEM;


/************************************************************************
 * MFPARMS - File system initialization parameters                      *
 ************************************************************************/

typedef struct mfparms 
{
        PMFDIRENT dirent_array;
        PMFVFILE  vfile_array;
        PMFVNODE  vnode_array;
        int       n_dirents;
        int       n_vfiles;
        int       n_vnodes;
        dword     blksize;
        MF_ALLOCATOR   blkallocfn;
        MF_DEALLOCATOR blkdeallocfn;
        } MFPARMS;
typedef MFPARMS KS_FAR *PMFPARMS;


/* ************************************************************************   */
extern PMFSYSTEM pmfsys;

/* ************************************************************************   */
#ifdef __cplusplus
extern "C" {
#endif

RTIP_BOOLEAN   mf_build(int fd, PMFBLOCK to_add, dword data_size);
#if (INCLUDE_STATIC)
RTIP_BOOLEAN      mf_build_file(char *path, char *filename, PFBYTE buf, long buf_len);
#endif
int       mf_close(int fd);
RTIP_BOOLEAN   mf_delete(char *name);
PMFDIRENT mf_fd_to_dirent(int fd);
RTIP_BOOLEAN   mf_flush(int fd);
void      mf_gdone(PVDSTAT dirobj);
RTIP_BOOLEAN   mf_gfirst(PVDSTAT dirobj, char *name);
RTIP_BOOLEAN   mf_gnext(PVDSTAT dirobj);
PVFILEAPI mf_init(PMFSYSTEM mfsys, PMFPARMS parms);
RTIP_BOOLEAN   mf_isdir(char *name);
long      mf_lseek(int fd, long offset, int origin);
RTIP_BOOLEAN   mf_mkdir(char *name);
RTIP_BOOLEAN   mf_mountfs(char *mountpath, char *nativepath, void * filesys, MPT_INITFN initfn, PFVOID init_arg);
int       mf_nputc(int fd, byte to_put, word count);
int       mf_open(char *name, word flag, word mode);
RTIP_BOOLEAN   mf_pwd(char *path);
int       mf_read(int fd,  byte *buf, word count);
RTIP_BOOLEAN   mf_rmdir(char *name);
RTIP_BOOLEAN   mf_rename(char *name, char *newname);
#if (INCLUDE_CALLBACK)
void      mf_set_callback(char *name, PFVOID pdata, CALLBACK_OPENFN openfn,CALLBACK_CLOSEFN closefn, CALLBACK_READFN readfn, CALLBACKWRITEFN writefn,CALLBACK_LSEEKFN seekfn);
#endif
RTIP_BOOLEAN   mf_set_cwd(char *name);
RTIP_BOOLEAN   mf_truncate(int fd, long offset);
RTIP_BOOLEAN   mf_unmountfs(char *mountpath);
int       mf_write(int fd, PFBYTE buf, word count);
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
char *    mf_modified_date(int handle, char *buffer, int buflen);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN   mf_stat(char *name, PVSTAT stat);
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
RTIP_BOOLEAN   mf_chmode(PFCHAR name, int attributes);
RTIP_BOOLEAN   mf_get_free(PFCHAR name, dword *blocks, dword *bfree);
RTIP_BOOLEAN   mf_chsize(int fd, long size);
#endif                                 

#ifdef __cplusplus
}
#endif

#endif /*__MFAPI__*/

