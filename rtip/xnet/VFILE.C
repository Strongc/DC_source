/*                                                                      */
/*                                                                      */
/* VFILE.C - Virtual File system Initialization and low level calls     */
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
/*#if (INCLUDE_RTIP)   */
#include "rtip.h"
#include "rtipext.h"
/*#endif   */
#include "dymconf.h"
#include "vfile.h"
#include "vfapi.h"
#include "vfsext.h"

#if (INCLUDE_VFS)
#if (INCLUDE_DOS_FS)
#include <stdlib.h>
#include <direct.h>
#endif
#if (INCLUDE_MFS)
#include "mfapi.h"
#include "memfile.h"
#endif

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_VF_INIT       0
#define DEBUG_BUILD_VFILE   0

/* ********************************************************************   */
#if (INCLUDE_MFS)

#if (INCLUDE_DYNAMIC && INCLUDE_BGET)
/* __ebs_core1 is the core we commit to the bget() memory allocator.
  the variable __ebs_core2 to __ebs_core4 are commented out but left in
  to demonstrate that we can commit several discontiguous regions to
  bget()'s heap
*/
extern int KS_FAR __ebs_core1[CFG_MFCORESIZE];  // FKA was char
#endif

#if (INCLUDE_FILES)
/* If we are supporting more than one file in the filesystem   */
extern MFDIRENT KS_FAR __mf_dirent_core[CFG_MF_NDIRENTS];
#   if (INCLUDE_MTPOINTS)
/* If we are supporting mounting of file systems onto the memory file sys   */
extern MFVNODE  KS_FAR __mf_vnode_core[CFG_MF_NVNODES];
extern MFVFILE  KS_FAR __mf_vfile_core[CFG_MF_NVFILES];
#   endif
#endif
extern MFSYSTEM KS_FAR __mf_system;
#if (INCLUDE_FILES || INCLUDE_DYNAMIC)
/* Parameters we pass to mf_init. Must be static because mf_init stores
   it internally */
extern MFPARMS KS_FAR  __mf_parms;
#endif

#endif  /* INCLUDE_MFS */

#if (!INCLUDE_MFS)
/* ********************************************************************   */
/* VF INIT (no MFS)                                                       */
/* ********************************************************************   */
/* If MFS is not enabled then vfile consists of a simple function 
   that invokes initializers to map the vf_xx api calls to native 
   file system calls. We provide initializers for ERTFS and DOS 
   but other file systems could easily be added */
int vf_init(void)
{
#if (DEBUG_VF_INIT)
    DEBUG_ERROR("vf_init (NO MFS) called", NOVAR, 0, 0);
#endif

    /* Return if already initialized   */
    if (vf_api)
        return (0);
#if (INCLUDE_ERTFS_API)
    vfrtfs_api_init(&vfrtfs_api);   /* Initialize the rtfs api structure */
    vf_api = &vfrtfs_api;
#elif (INCLUDE_DOS_FS)
    vfdos_api_init(&vfdos_api);     /* Initialize the dos api structure */
    vf_api = &vfdos_api;
#elif (INCLUDE_RTFILES)
    vfrtfi_api_init(&vfrtfi_api);   /* Initialize the rtkernel file system
                                       api structure */
    vf_api = &vfrtfi_api;
#elif (INCLUDE_NOMAD_FS)
    vfnomad_api_init(&vfnomad_api);   /* Initialize the Second Wind Nomad file system
                                       api structure */
    vf_api = &vfnomad_api;
#elif (INCLUDE_NFS_CLI)
    /* NOTE: NFS client should be last in the #if, #elsif since we    */
    /*       only support one API                                     */
    vfnc_api_init(&vfnc_api);       /* Initialize the nfs api structure */
    vf_api = &vfnc_api;
#else
#error - Add api_init function call.
#endif
    return (0);
}

#else       /* if INCLUDE_MFS */
/* ********************************************************************   */
/* VF INIT (with MFS)                                                     */
/* ********************************************************************   */
/* This is the initializer funtion for the virtual file system.
   When virtual file code is included. See the instance of vf_init 
   up above for the case where INCLUDE_MFS is disabled.
   The virtual file system may be customized by modifying config 
   values on xnconf.h and by modifying this routine directly */

int vf_init(void)
{
#if (INCLUDE_DOS_FS)
#if (INCLUDE_MTPOINTS)
    /* For dos systems we do a get cwd and then a virtual set cwd so the
       initial working directory is the current directory that the
       program started from; this way when the program exits we
       will be at same directory we started from */
    char cwd_string[_MAX_PATH+5];
#endif
#endif

#if (DEBUG_VF_INIT)
    DEBUG_ERROR("vf_init (MFS) called", NOVAR, 0, 0);
#endif

    /* Return if already initialized   */
    if (vf_api)
        return (0);

#if (INCLUDE_FILES)
    /* Support for > 1 directory entry   */
    __mf_parms.dirent_array = __mf_dirent_core;
    __mf_parms.n_dirents    = CFG_MF_NDIRENTS;
#endif

#if (INCLUDE_MTPOINTS)
    /* Support for mounting file systems on directories   */
    __mf_parms.vnode_array  = __mf_vnode_core;
    __mf_parms.n_vnodes     = CFG_MF_NVNODES;
    __mf_parms.vfile_array  = __mf_vfile_core;
    __mf_parms.n_vfiles     = CFG_MF_NVFILES;
#endif

#if (INCLUDE_DYNAMIC)
#if (INCLUDE_BGET)
    /* Commit core to the BGET allocator. Chunks of this core will be 
       returned by bget(). brel will return the core to the heap */
    bpool(__ebs_core1,CFG_MFCORESIZE);
    /* More than one pool can be added   */
/*  bpool(__ebs_core2,0xB000);           */
/*  bpool(__ebs_core3,0xB000);           */
/*  bpool(__ebs_core4,0xB000);           */
#endif
    
    /* Set up the block size and allocator/deallocator functions for 
       extending files */
    __mf_parms.blksize      = 512;
    __mf_parms.blkallocfn   = (MF_ALLOCATOR) vf_alloc;
    __mf_parms.blkdeallocfn = (MF_DEALLOCATOR) vf_free;
#endif

    /* ********************************************************************   */
    /* init MFS                                                               */

#if (INCLUDE_FILES || INCLUDE_DYNAMIC)
    /* Create a memory file system that supports more than
       one file and/or dynamic file extension */
    /* Note: vf_api is a global variable that points to a vfileapi
             structure. This structure contains pointers to functions that 
             implement the file system. The vf_xxx macros in vfile.h call these
             functions */
    vf_api = mf_init(&__mf_system, &__mf_parms);
#else
    /* Create a memory file system that only has one file that must
       have staticly assigned data */
    vf_api = mf_init(&__mf_system, 0);
#endif
#if (DEBUG_VF_INIT)
    DEBUG_ERROR("vfs_init: vf_api init to ", DINT1, vf_api, 0);
#endif

    /* ********************************************************************   */
    /* init native file system                                                */

#if (INCLUDE_MTPOINTS)
    /* Create mount points for the DOS or RTFS A: and C: drives 
       and for an nfs remote mmount */ 
#if (INCLUDE_FILESYSTEM)
#    if (DEBUG_VF_INIT)
        DEBUG_ERROR("vf_init (MFS): mkdir A: and C:", NOVAR, 0, 0);
#    endif
    vf_mkdir("\\A:");
    vf_mkdir("\\C:");
#endif

#endif  /* MTPOINTS */

#if (INCLUDE_VFS && INCLUDE_STATIC && INCLUDE_VIRTUAL_TABLE)
    vf_set_cwd("\\");
#endif

#if (INCLUDE_ERTFS_API)
    /* Initialize a structure that holds the API for ERTFS   */
    vfrtfs_api_init(&vfrtfs_api);   /* Initialize the rtfs api structure */
#if (INCLUDE_MTPOINTS)
    /* Mount the A: and C: drives for rtfs   */
    vf_mountfs("\\A:", "A:\\", &vfrtfs_api, 0, 0);
    vf_mountfs("\\C:", "C:\\", &vfrtfs_api, 0, 0);
#else
    vf_api = &vfrtfs_api;
#endif  /* MTPOINTS */

#elif (INCLUDE_DOS_FS)
#    if (DEBUG_VF_INIT)
        DEBUG_ERROR("vf_init: init vfdos_api ", NOVAR, 0, 0);
#    endif

    /* Initialize a strucure that holds the API for DOS file access   */
    vfdos_api_init(&vfdos_api); /* Initialize the dos api structure */
#if (INCLUDE_MTPOINTS)
    /* Mount the A: and C: drives for dos   */
    vf_mountfs("\\A:",  "A:\\", &vfdos_api, 0, 0);
    vf_mountfs("\\C:",  "C:\\", &vfdos_api, 0, 0);
#    if (DEBUG_VF_INIT)
        DEBUG_ERROR("vf_init: mount A: and C: ", NOVAR, 0, 0);
#    endif
#else
    vf_api = &vfdos_api;
#    if (DEBUG_VF_INIT)
        DEBUG_ERROR("vf_init: set vf_api to vfdos_api ", DINT1,
            vf_api, 0);
#    endif

#endif  /* MTPOINTS */


#elif (INCLUDE_RTFILES)
    /* Initialize a strucure that holds the API fro ERTFS   */
    vfrtfi_api_init(&vfrtfi_api);   /* Initialize the rtfs api structure */
#if (INCLUDE_MTPOINTS)
    /* Mount the A: and C: drives for rtfiles   */
    vf_mountfs("\\A:", "A:\\", &vfrtfi_api, 0, 0);
    vf_mountfs("\\C:", "C:\\", &vfrtfi_api, 0, 0);
#else
    vf_api = &vfrtfi_api;
#endif  /* MTPOINTS */

#elif (INCLUDE_NFS_CLI)
    /* NOTE: application will mount drives   */
    vfnc_api_init(&vfnc_api);       /* Initialize the nfs api structure */
#endif


    /* ********************************************************************   */
#if (INCLUDE_MTPOINTS)
#if (INCLUDE_DOS_FS)
    /* If DOS set cwd to the directory the app was launched from     */
    /* Get current working DRIVE:DIRECTORY from DOS then cd to it    */
    getcwd(&cwd_string[1], _MAX_PATH);
    cwd_string[0] = '\\';
    if (!vf_set_cwd(cwd_string))
    {
        DEBUG_ERROR("vf_init: vf_set_cwd (MFS) failed", NOVAR, 0, 0);
    }
    DEBUG_ERROR("vf_init: set cwd to ", STR1, cwd_string, 0);

    /* verify it worked   */
    vf_pwd(cwd_string);
    DEBUG_ERROR("pwd was set to ", STR1, cwd_string, 0);
#endif
#endif  /* MTPOINTS */
    return (0);
}

#endif  /* if (!INCLUDE_MFS) .. ELSE */

#if (INCLUDE_MALLOC)
/* ********************************************************************   */
/* DYNAMIC MEMORY                                                         */
/* ********************************************************************   */
PFBYTE vf_alloc(int size)
{
PFBYTE ptr;

    ptr = ks_malloc(1, size, VF_MALLOC);
    return(ptr);
}

void vf_free(PFBYTE mem)
{
    ks_free(mem, 1, 1);
}
#endif

/* ********************************************************************   */
/* FGETS                                                                  */
/* ********************************************************************   */
PFCHAR vf_fgets(PVFILEAPI p_api, int fd, PFCHAR buf, word length)
{
    dword l;
    word n;
    word i;

    if (length == 0)
        return (0);
    if (length == 1)
    {
        buf[0] = 0;
        return (buf);
    }
    length = (word)(length - 1);

    /* Get the current position   */
    l = _vf_lseek(p_api, fd, 0, VSEEK_CUR);
    n = (word) _vf_read(p_api, fd, (byte *)buf, (word)length);
    if (n == 0 || n == 0xffffU)
    {
        _vf_lseek(p_api, fd, l, VSEEK_SET);
        return (0);
    }
    for (i = 0; i < n; i++)
    {
        if (buf[i] == '\n')
        {
            buf[i+1] = 0;
            _vf_lseek(p_api, fd, l+(i+1), VSEEK_SET);
            return (buf);
        }
    }

    buf[n] = 0;
    _vf_lseek(p_api, fd, l+n, VSEEK_SET);
    return (buf);
}

/* ********************************************************************   */
/* FILE LENGTH                                                            */
/* ********************************************************************   */
dword vf_filelength(PVFILEAPI p_api,int fd)
{
    dword curr;
    dword end;

    /* Get the current position   */
    curr = _vf_lseek(p_api, fd, 0, VSEEK_CUR);

    /* Get the end of file position   */
    end = _vf_lseek(p_api, fd, 0, VSEEK_END);

    /* seek to origional position   */
    _vf_lseek(p_api, fd, curr, VSEEK_SET);

    return (end);
}

/* ********************************************************************   */
/* ISDIR                                                                  */
/* ********************************************************************   */
int vdstat_isdir(word flags)
{
    return (flags & VF_ATTRIB_ISDIR);
}

#if (INCLUDE_MFS)
/* ********************************************************************   */
/* ENTER MFS                                                              */
/* ********************************************************************   */
RTIP_BOOLEAN mfs_enter_done = FALSE;
void rtip_enter_mfs(void)
{
#if (INCLUDE_SUBDIRS && INCLUDE_RTIP)
PSYSTEM_USER puser;
#endif

    OS_CLAIM_MEMFILE(ENTER_CLAIM_MEMFILE);

#if (INCLUDE_SUBDIRS && INCLUDE_RTIP)
    puser = get_system_user();
    if (puser->mfcwd == 0)
    {
        /* set up current working directory           */
        /* NOTE: memory will be freed by os_exit_task */
        puser->mfcwd = mf_alloc_path();
        tc_strcpy(puser->mfcwd, STRING_BACKSLASH);
    }
    
    pmfsys->mfcwd = puser->mfcwd;
#elif (INCLUDE_SUBDIRS)
    if (!mfs_enter_done)
    {
        mfs_enter_done = TRUE;
        pmfsys->mfcwd = mf_alloc_path();
        DEBUG_ERROR("pmfsys->mfcwd = ", DINT1, pmfsys->mfcwd, 0);
        tc_strcpy(pmfsys->mfcwd, STRING_BACKSLASH);
    }
#endif
}
#endif


/* ********************************************************************   */
/*
 * Create the complete path specified in <filename> EXCLUDING the file name itself in <filename>.
 * Relative paths in <filename> assume <current_path> as base directory.
 *
 * Returns TRUE on success and will set the present working directory to <current_path>.
 *
 * Returns FALSE on failure and may have changed the present working directory.
 */
RTIP_BOOLEAN vf_create_filepath(char *current_path, char *filename)
{
char f[VF_MAXPATHLEN];
char *slashp;
char *fname;
         
    /* copy the path string as we're going to destroy it during directory processing   */
    tc_strcpy(f, filename);
    fname = f;
              
    if (f[0] == '\\' || f[0] == '/')
    {
        if (!vf_set_cwd("\\"))
        {
            DEBUG_ERROR("vf_create_filepath: cannot set root directory: ", 
                STR1, "\\", 0);
            return FALSE;
        }         
        /* skip root dir   */
        fname++;
    }
    else if (current_path && !vf_set_cwd(current_path))
    {
        DEBUG_ERROR("vf_create_filepath: cannot set base directory: ", STR1, current_path, 0);
        return FALSE;
    }
        
    for ( ; *fname; )
    {
        slashp = tc_strchr(fname, '\\');
        if (!slashp)
            slashp = tc_strchr(fname, '/');
        if (!slashp)
            break;
        *slashp++ = 0;
                             
        /* go one directory level deeper                        */
        vf_mkdir(fname);
        if (!vf_set_cwd(fname))
            return FALSE;
        fname = slashp;
    }

    if (current_path && !vf_set_cwd(current_path))
    {
        DEBUG_ERROR("vf_create_filepath: cannot set base directory: ", STR1, current_path, 0);
        return FALSE;
    }
    return TRUE;
}       


#define RESTORE_DIR 0

RTIP_BOOLEAN vf_build_virtual_file(char *drive_path, char *filename, PFBYTE buf, long buf_len)
{
int fd;
int file_off;
char file_name[VF_MAXPATHLEN];
#if (RESTORE_DIR)
char cwd_string[_MAX_PATH];
#endif
int  len;
RTIP_BOOLEAN ret_val;

#if (DEBUG_BUILD_VFILE)
    DEBUG_ERROR("\nvf_build_virtual_file: base, file: ", STR2, drive_path, filename);
#endif

    ret_val = TRUE;

#if (RESTORE_DIR)
    if (!vf_pwd(cwd_string))
    {
        DEBUG_ERROR("vf_build_virtual_file: vf_pwd failed", NOVAR, 0, 0);
    }
    DEBUG_ERROR("vf_build_virtual_file: pwd is ", STR1, cwd_string, 0);
#endif

    /* Assign compiled in web pages in virttbl.c to directory entries   */
    if (!vf_set_cwd(drive_path))
    {
        DEBUG_ERROR("VIRTUAL FILE: cannot set base directory: ", STR1, drive_path, 0);
        return FALSE;
    }
        
    /* create path in filename first   */
    if (!vf_create_filepath(drive_path, filename))
    {
        DEBUG_ERROR("VIRTUAL FILE: cannot create file path, basedir: ", STR2, filename, drive_path);
        return FALSE;
    }
    
#if (INCLUDE_MFS && INCLUDE_STATIC)
    /*
     * Attempt to create an MFS based file first. If this fails, it's probably
     * on another file system (SSL certificates, etc.)
     */
#if (DEBUG_BUILD_VFILE)
    DEBUG_ERROR("call mf_build_file: drive_path, filename: ",
        STR2, drive_path, filename);
#endif

    if (!mf_build_file(drive_path, filename, buf, (dword)buf_len))
#endif
    {   
#if (DEBUG_BUILD_VFILE)
        DEBUG_ERROR("call mf_build_file: drive_path, filename: ",
            STR2, drive_path, filename);
#endif
        /* build of memory file failed; try to build virtual file   */
        if (filename[0] != '\\' && filename[0] != '/')
            tc_strcpy(file_name, drive_path);
        else
            file_name[0] = 0;
        tc_strcat(file_name, filename);

#if (DEBUG_BUILD_VFILE)
        DEBUG_ERROR("vf_build_virtual_file: VFS file COPY: ", 
            STR1, file_name, 0);
#endif
        /* Create the file; overwrite existing   */
        fd = vf_open(file_name, VO_BINARY|VO_WRONLY|VO_CREAT|VO_TRUNC/*|VO_EXCL*/, VS_IWRITE|VS_IREAD);
        if (fd >= 0)
        {
            for (file_off = 0; file_off < buf_len; file_off += len)
            {
                len = VF_BEST_XFER_SIZE;
                if (len+file_off > buf_len)
                    len = (int)(buf_len - file_off);
                len = vf_write(fd, buf+file_off, (word)len);
                if (len <= 0)
                {
                    DEBUG_ERROR("VIRTUAL FILE: len, write error: ", EBS_INT2, buf_len, len);
                    break;
                }
            }
            vf_close(fd);
        }
        else
        {
            DEBUG_ERROR("vf_build_virtual_file failed", NOVAR, 0, 0);
            ret_val = FALSE;
        }
    }

#if (RESTORE_DIR)
    if (!vf_set_cwd(cwd_string))
    {
        DEBUG_ERROR("vf_build_virtual_file: vf_set_cwd failed ", 
            STR1, cwd_string, 0);
    }
#endif

    return(ret_val);
}

#endif  /* INCLUDE_VFS */


