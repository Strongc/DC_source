/*                                                                      */
/* VFDATA.C - Virtual File system data                                  */
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
#include "vfsext.h"

#if (INCLUDE_VFS)
/* ********************************************************************   */
/* MEMORY FILE SYSTEM                                                     */
/* ********************************************************************   */
/* API structures. These are initialized with pointers to file system
   specific api functions */
#if (INCLUDE_ERTFS_API)
    VFILEAPI vfrtfs_api;
#elif (INCLUDE_DOS_FS)
    VFILEAPI vfdos_api;
#elif (INCLUDE_RTFILES)
    VFILEAPI vfrtfi_api;
#elif (INCLUDE_NOMAD_FS)
    VFILEAPI vfnomad_api;
#endif
#if (INCLUDE_NFS_CLI)
    VFILEAPI vfnc_api;
#endif


#if (INCLUDE_MFS)
/* Array handed to bcore() for use by the bget and brel memory allocator
   pair. Only used if dynamic allocation of core for file data. */
#if (INCLUDE_DYNAMIC && INCLUDE_BGET)
int KS_FAR __ebs_core1[CFG_MFCORESIZE];  // FKA was char
#endif

VFILEAPI mfs_api = {0};
PMFSYSTEM pmfsys;

#if (INCLUDE_FILES)
    /* Core to hold dirent structures   */
MFDIRENT KS_FAR __mf_dirent_core[CFG_MF_NDIRENTS];
#if (INCLUDE_MTPOINTS)
    /* Core to hold vnode structures (mount points)   */
MFVNODE  KS_FAR __mf_vnode_core[CFG_MF_NVNODES];
    /* Core to hold vfile structures (files)   */
MFVFILE  KS_FAR __mf_vfile_core[CFG_MF_NVFILES];
#endif
#endif

MFSYSTEM KS_FAR __mf_system;
#if (INCLUDE_FILES || INCLUDE_DYNAMIC)
MFPARMS  KS_FAR __mf_parms;
#endif

#endif  /* INCLUDE_MFS */

/* vf_api is a global variable that points to a vfileapi              */
/* structure. This structure contains pointers to functions that      */
/* implement the file system. The vf_xxx macros in vfile.h call these */
/* functions                                                          */
#if (INCLUDE_RTIP)
PVFILEAPI vf_api; /* initialized in xn_rtip_init() */
#else
PVFILEAPI vf_api = NULL;
#endif

#endif      /* INCLUDE_VFS */


