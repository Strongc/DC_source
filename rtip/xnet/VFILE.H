/*                                                                      */
/* VFILE.H - Virtual File system API                                    */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */


#ifndef __VFILE__
#define __VFILE__ 1

#include "rtip.h"
#include "sock.h"
#include "vfconf.h"
#include "vfapi.h"
#include "vfsext.h"

/* ********************************************************************   */
#define STRING_BACKSLASH   "\\"
#define CHAR_BACKSLASH     '\\'
#define STR_BACKSLASH      "\\"


#if (INCLUDE_GET_FREE || INCLUDE_SMTP)
#define VF_BLK_SIZE 512
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
#define VF_DSKOPEN(DNAME)   1
#define VF_DSKCLOSE(DNAME)  1
#endif
#define VF_BEST_XFER_SIZE 1024      /* optimal xfer size */


/* ********************************************************************   */
#if (INCLUDE_ERTFS_API)
    extern VFILEAPI vfrtfs_api;
    void vfrtfs_api_init(VFILEAPI *papi);
#elif (INCLUDE_DOS_FS)
    extern VFILEAPI vfdos_api;
    void vfdos_api_init(VFILEAPI *papi);
#elif (INCLUDE_RTFILES)
    extern VFILEAPI vfrtfi_api;
    void vfrtfi_api_init(VFILEAPI *papi);
#elif (INCLUDE_NOMAD_FS)
    extern VFILEAPI vfnomad_api;
    void vfnomad_api_init(VFILEAPI *papi);
#endif
#if (INCLUDE_NFS_CLI)
    extern VFILEAPI vfnc_api;
    void vfnc_api_init(VFILEAPI *papi);
#endif  


/* ********************************************************************   */
PFBYTE  vf_alloc(int size);
void    vf_free(PFBYTE mem);
int     vf_patcmp(PFCHAR pat, PFCHAR name, int dowildcard);
PFCHAR  parse_next(PFCHAR to, char c, PFCHAR str);
PFCHAR  parse_parent(PFCHAR to, char c, PFCHAR path);
int     reduce_path(PFCHAR dest, PFCHAR path);
RTIP_BOOLEAN vf_create_filepath(char *current_path, char *filename);
RTIP_BOOLEAN vf_build_filename(char *full_filename, char *current_path, char *filename);

#endif /*__VFILE__*/

