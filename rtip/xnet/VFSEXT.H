/*                                                                      */
/*                                                                      */
/* VFEXT.H - Virtual File system data external declarations             */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __VFSEXT__
#define __VFSEXT__ 1

#include "vfile.h"
#if (INCLUDE_MFS)
    #include "memfile.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* ********************************************************************   */
/* MEMORY FILE SYSTEM                                                     */
/* ********************************************************************   */
/* API structures. These are initialized with pointers to file system
   specific api functions */
#if (INCLUDE_ERTFS_API)
    extern VFILEAPI vfrtfs_api;
#elif (INCLUDE_DOS_FS)
    extern VFILEAPI vfdos_api;
#elif (INCLUDE_RTFILES)
    extern VFILEAPI vfrtfi_api;
#elif (INCLUDE_NOMAD_FS)
    extern VFILEAPI vfnomad_api;
#endif
#if (INCLUDE_NFS_CLI)
    extern VFILEAPI vfnc_api;
#endif


#if (INCLUDE_MFS)
/* Array handed to bcore() for use by the bget and brel memory allocator
   pair. Only used if dynamic allocation of core for file data. */
#if (INCLUDE_DYNAMIC)
#if (INCLUDE_BGET)
extern int KS_FAR __ebs_core1[CFG_MFCORESIZE];  // FKA was char
#endif
#endif

extern VFILEAPI mfs_api;
extern PMFSYSTEM pmfsys;

#if (INCLUDE_FILES)
    /* Core to hold dirent structures   */
extern MFDIRENT KS_FAR __mf_dirent_core[CFG_MF_NDIRENTS];
#if (INCLUDE_MTPOINTS)
    /* Core to hold vnode structures (mount points)   */
extern MFVNODE  KS_FAR __mf_vnode_core[CFG_MF_NVNODES];
    /* Core to hold vfile structures (files)   */
extern MFVFILE  KS_FAR __mf_vfile_core[CFG_MF_NVFILES];
#endif
#endif

extern MFSYSTEM KS_FAR __mf_system;
#if (INCLUDE_FILES || INCLUDE_DYNAMIC)
extern MFPARMS  KS_FAR __mf_parms;
#endif

#endif  /* INCLUDE_MFS */

extern PVFILEAPI vf_api; /* initialized in xn_rtip_init() */

#ifdef __cplusplus
};
#endif

#endif      /* VFSEXT */


