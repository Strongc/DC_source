/*                                                                      */
/* VFCONF.H - Virtual File Configuration FIle                           */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __MFCONFIG_H_INCLUDED__
#define __MFCONFIG_H_INCLUDED__

#include "xnconf.h"

#if (!INCLUDE_RTIP)
#if (INCLUDE_ERTFS || INCLUDE_ERTFS_PRO ||  INCLUDE_DOS_FS || INCLUDE_RTFILES)
#define INCLUDE_FILESYSTEM 1
#else
#define INCLUDE_FILESYSTEM 0    /* set to 1 if you are using your own file system */
                                /* which is integrated with the virtual file   */
                                /* system                                      */
#endif
#endif

/* Set to force Virtual File system to be on

     Set this to 1 to unconditionally include the virtual file system.

     Set to 0 to include the virtual file system only if an EBS add-on
     which requires it is turned on.

     Note: If any EBS addon that requires virtual file system support is
     included then this value of INCLUDE_VFS will overridden and forced to
     one.
*/
#if (INCLUDE_FTP_CLI  || INCLUDE_FTP_SRV   ||                       \
     INCLUDE_TFTP_CLI || INCLUDE_TFTP_SRV  || INCLUDE_IMAP ||       \
     INCLUDE_WEB      || INCLUDE_SMTP || INCLUDE_POP3 ||            \
     INCLUDE_WEB_BROWSER || INCLUDE_NFS_SRV || INCLUDE_SSL || INCLUDE_SMB_SRV)
#define INCLUDE_VFS             1
#else
#define INCLUDE_VFS             0
#endif /* INCLUDE_FTP  || INCLUDE_WEB .. */

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                                    */
/* Set to force inclusion of virtual table support; the virtual table        */
/* (in virttbl.c) contains virtual files which are available at compile time */
/* If set to 0, files are read from disk or memory but there are no          */
/* initial files available;                                                  */
/* NOTE: turning this on will force INCLUDE_MFS on                           */
#if (INCLUDE_WEB)
#define INCLUDE_VIRTUAL_TABLE 1
#else
#define INCLUDE_VIRTUAL_TABLE 0
#endif

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Set to force Memory file system on.

     Set to 1 to include the memory file system even if not using EBS add-ons
     that require a file system.

     Set to 0 to include the memory file system only if add-ons are present
     which require it.

     Note: This value of INCLUDE_MFS will be overridden and forced to 1
           in the following cases.
     . An addon that requires a file system is being used and NO external
       file system support (DOS, ERTFS, RTFILES) etc is present. This will
       force a memory file system to be present for use by the application.
     . The web server is included. The web server requires a subset of the
       memory file system to access virtual pages.
*/
#if (INCLUDE_RTFILES)
/* RTFiles has its own memory file system   */
#define INCLUDE_MFS         0
#elif (INCLUDE_WEB || INCLUDE_VIRTUAL_TABLE)
#define INCLUDE_MFS         1
#elif (INCLUDE_VFS && !INCLUDE_FILESYSTEM)
#define INCLUDE_MFS         1
#else
#define INCLUDE_MFS         0
#endif

#if (BUILD_NEW_BINARY)
#endif

/* ************************************************************************         */
#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_SMB_SRV || INCLUDE_VF_ALL)
#define INCLUDE_STAT 1
#else
#define INCLUDE_STAT 0
#endif

#if (INCLUDE_NFS_SRV || INCLUDE_SMB_SRV || INCLUDE_VF_ALL)
#define INCLUDE_GET_FREE 1
#else
#define INCLUDE_GET_FREE 0
#endif


#if (!INCLUDE_RTIP)
/* ************************************************************************         */
#define INCLUDE_DOS_FS          0
#define INCLUDE_WIN32_FS        1
#if (INCLUDE_WIN32_FS)
#undef INCLUDE_DOS_FS
#define INCLUDE_DOS_FS          1
#define INCLUDE_VF_ALL          1

#undef IS_MS_PM
#define IS_MS_PM                1
#endif

#endif      /* if (!INCLUDE_RTIP) */

/* Set to 1 to include all virtual file calls including routines that
   may not be required by the included ebs applications */
#define INCLUDE_VF_ALL  0

/* ************************************************************************         */
#define VF_FILENAMESIZE  160
#define VF_MAXPATHLEN    200

/* ************************************************************************          */
/* *********************     Virtual file system    ***********************          */
/* ************************************************************************          */

/* Maximum dir or file name size including 0   */
#define CFG_MF_NAMESIZE VF_FILENAMESIZE

/* Maximum path size including 0   */
#define CFG_MF_MAXPATH  VF_MAXPATHLEN

/* If 1 Include support for Files with more than one data block   */
#define INCLUDE_BLOCKS      1

/* If 1 Include support for manually assigning data blocks to files   */
#define INCLUDE_STATIC      1

/* If 1 Include support for assigning subroutines to perform
   open/read/write/close operations */
#define INCLUDE_CALLBACK    0

/*  If 1 Include support for Dynamic block allocation/deallocation
    using bget and bfree */
#define INCLUDE_DYNAMIC     1

/*  If 1 Include support for creating more than one file per file
    system. If this is set to 1 we have a usable traditional file
    system. If not 1 a file system with one file is supported.
    This save memory */
#define INCLUDE_FILES       1

/*  If 1 Include support for subdirectories   */
#define INCLUDE_SUBDIRS     1

/*  If 1 Include support for File input   */
#define INCLUDE_READ        1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING    */
/*  If 1 Include support for  File output    */
#define INCLUDE_WRITE       1

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/*  If 1 Include support for Mounting alternative file systems;
    if set and a file system is included, then vf_init() mounts
    \A: and \C:;
    NOTE: if there is a native file system, vf_xxx calls will
          call the native file system;  the only way to
          call the memory file system is to call it directly */
#define INCLUDE_MTPOINTS    0

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING           */
/* The total number of directory entries allowed    */
#define CFG_MF_NDIRENTS       64

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING                         */
/* The total number of native file systems that may be mounted    */
#define CFG_MF_NVNODES        3

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* The total number of native files and sub-directories that may be
   accessed at any one time through MFS mount points */
#define CFG_MF_NVFILES        20

/* YOU MIGHT WANT TO CHANGE THE FOLLOWING   */
/* Size of core in bites handed to bget. The memory file system will
   use this core to create directory entries and data files. The default
   value is 1024. If you plan to use the virtual file system this should
   be increased.
   Note: the routine vf_init calls bpool() to assign free core to the lower
   level allocator. We currently only give it on block of CFG_MFCORESIZE
   bytes. bpool can be called multiple times to assign multiple discontiguous
   block of core */

#if (INCLUDE_MFS && INCLUDE_DYNAMIC)
#define CFG_MFCORESIZE 1024
/*#define CFG_MFCORESIZE 32000  // Set to 32000 for FTP/TFTP tests   */
#endif

/* ************************************************************************          */
/* THE REST OF THE FILE IS NOT CONFIGURATION DATA                                    */
/* ************************************************************************          */
#if (!INCLUDE_FILES)
#undef  INCLUDE_SUBDIRS
#define INCLUDE_SUBDIRS  0
#undef  INCLUDE_MTPOINTS
#define INCLUDE_MTPOINTS 0
#endif

#endif/*__MFCONFIG_H_INCLUDED__ */

