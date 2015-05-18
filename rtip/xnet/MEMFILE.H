/*                                                                        */
/* MEMFILE.H  - Memory File system header file                            */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright Peter Van Oudenaren , 1993                                 */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains declarations for memory file system          */

#ifndef __MEMFILE__
#define __MEMFILE__ 1

#include "vfconf.h"
#include "mfapi.h"

void      rtip_enter_mfs(void);

#if (INCLUDE_RTIP)
#define MF_API_ENTER()   rtip_enter_mfs()
#define MF_API_EXIT()    OS_RELEASE_MEMFILE()
#else
#define MF_API_ENTER()   rtip_enter_mfs()
#define MF_API_EXIT()    {}
#endif
#define MF_API_RETURN(X) { MF_API_EXIT(); return (X); }

#define MF_BLOCK_READ   0
#define MF_BLOCK_WRITE  1
#define MF_BLOCK_NPUTC  2


PMFBLOCK  mf_alloc_block(void);
PMFDIRENT mf_alloc_dirent(void);
PFCHAR    mf_alloc_path(void);
int       mf_alloc_vfile(void);
PMFVNODE  mf_alloc_vnode(void);
void      mf_bind_vfile(int vfile, PMFVNODE pvnode, int to_fd);
int       mf_blockio(int fd, byte *buf, word count, int operation);
void      mf_free_block(PMFBLOCK pblk);
void      mf_free_block_chain(PMFBLOCK pblk);
void      mf_free_dirent(PMFDIRENT to_free);
void      mf_free_path(PFCHAR p);
void      mf_free_vfile(int vfile);
void      mf_free_vnode(PMFVNODE to_free);
PMFDIRENT mf_find_dirent(char *path);
PVFILEAPI mf_get_api(void);
char     *mf_get_full_path(char *full_path, char *relative_path);
int       mf_get_native_path(PFCHAR fsname, PMFVNODE pvnode);
void      mf_get_path(char *path, PMFDIRENT pdir);
int       mf_init_vnode(PMFVNODE pvnode);
int       mf_is_vfile(int fd);
PMFDIRENT mf_make_dirent(char *name);
PMFDIRENT mf_next_dirent(PMFDIRENT pdir, char *pattern);
RTIP_BOOLEAN   mf_validate_fd(int fd);

void mfs_to_c(char *filename, PMFSYSTEM pmfs);

#endif /*__MEMFILE__*/

