
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSnet Inc, 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_MFS

/*#include "sock.h"   */
/*#if (INCLUDE_RTIP)  */
#include "rtip.h"
/*#endif   */
#include "dymconf.h"
#include "vfsext.h"
#include "vfile.h"
#include "memfile.h"


#if (INCLUDE_MFS)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_MFS_API   0
#define DEBUG_MFS_OPEN  0
#define DEBUG_MFS_READ  0
#define DEBUG_MFS_WRITE 0
#define DEBUG_MFS_BF    0
#define DEBUG_MFS_PWD   0
#define DEBUG_MFS_CWD   0
#define DEBUG_MFS_MKDIR 0
#define DEBUG_MFS_RMDIR 0
#define DEBUG_MFS_STAT  0
#define DEBUG_MOUNT_FS  0
#define DEBUG_DIRENT    0

/*************************************************************************
 * mf_init() - Initialize an instance of the memory file system.         *
 * This routine takes as arguments a system block to initialize          *
 * the address of an array of DIRENTS to commit to the system            *
 * and the size of that array in units. The address of a routine         *
 * to call to allocate blocks, and the address of a routine              *
 * to call to free blocks.                                               *
 *                                                                       *
 * Inputs:                                                               *
 * mfsys - The address of a structure of type MFSYSTEM. The address of   *
 * this structure will be saved internally so the scope of the structure *
 * should be valid while the memory file system is using it. Normally    *
 * this should be a global varable.                                      *
 * parms - The address of a structure containing the initialization      *
 * parameters.  See mfapi.h for a description of this structure.         *
 *                                                                       *
 * Notes:                                                                *
 * 1. The maximum number of directory entries possible in this           *
 * system is one less then what is comitted because one dirent is        *
 * always used as the root entry.                                        *
 * 2. If block allocate and free routines are not provided (IE           *
 * set to zero.) Then the memory file system can not randomly add        *
 * data to files. The files must be assigned static blocks.              *
 * 3. The block at mfsys is initialized and the global variable pmfsys   *
 *    is set to point to mfsys. It is possible to have multiple memory   *
 *    file systems in the same system but you must manually change       *
 *    pmfsys to point to the correct system block before making          *
 *    calls to the memory system.                                        *
 *                                                                       *
 *************************************************************************/

PVFILEAPI mf_init(PMFSYSTEM mfsys, PMFPARMS parms)
{
    int i;

    pmfsys = mfsys;
    tc_memset(pmfsys, 0, sizeof(MFSYSTEM));

#if (INCLUDE_FILES)
    /* If INCLUDE_FILES is TRUE we initialize a directory structure:
    Otherwise (see below) we create a file system with one entry.
    If INCLUDE_FILES is 0 we get a minimum file system with only
    one directory entry. This allows FTP'ing to one file that may
    be mapped to a memory location. */

    if (parms->dirent_array == 0)
    {
        DEBUG_ERROR("mf_init: parms->dirent_array is 0", NOVAR, 0, 0);
        return((PVFILEAPI)0);
    }
    tc_memset(parms->dirent_array, 0, sizeof(MFDIRENT)*parms->n_dirents);
    pmfsys->pdirent_array  = parms->dirent_array;
    pmfsys->pdirent_pool   = &(parms->dirent_array[1]);
    pmfsys->ntotal_dirents = parms->n_dirents;
    pmfsys->navail_dirents = parms->n_dirents - 1;
    /* Make the first element in the array the root of the system
       this means the root will always have handle zero */
    parms->dirent_array->flags = (MF_FLAGS_ISROOT|MF_FLAGS_ISDIR);
    parms->dirent_array->pparent = parms->dirent_array;
    tc_strcpy(parms->dirent_array->fname, STRING_BACKSLASH);

    /* Link the dirents and assign each a file descriptor   */
    for (i = 0; i < parms->n_dirents; i++)
    {
        parms->dirent_array[i].mfhandle = i;
        if (i > 1)
            parms->dirent_array[i-1].pnext = &(parms->dirent_array[i]);
    }
    parms->dirent_array[i-1].pnext = 0;
    pmfsys->mfroot = parms->dirent_array;

#if (INCLUDE_SUBDIRS)
    /* Call the Enter/Exit API functions. This will allocate a pwd structure
       and st it to root */
    MF_API_ENTER();
    MF_API_EXIT();
#endif

#if (INCLUDE_MTPOINTS)
    /* If INCLUDE_MTPOINTS is 1 we include structures that allow
       us to access files and directories whose roots appears to
       be subdirectories of the memory file system. See vf_init
       in vfile.c for an example of mounting the DOS or RTFS
       A: and C: drives to \fd0 and \hd0 of the memory file system
    */
    /* Initialize vnode array and vfile array. The vnodes define a
       mountable file system. The vnode contains the path to the
       mount point in the mounted file system and a pointer
       to a VFILEAPI structure that contains pointers to functions
       that operate on that file system including open, close, mkdir
       etc. */

    if (parms->vnode_array)
        tc_memset(parms->vnode_array, 0, sizeof(MFVNODE)*parms->n_vnodes);

    /* Initialize the vfile array. The vfile array is used to perform
       access to files in mounted file systems. The memory file system
       uses file descriptors 1 through n_dirents - 1 for memory files and
       descriptors n_dirents to n_dirents + n_fviles for files on mounted
       volumes. When a file descriptor is > vf_base then the native
       file descriptor comes out of the vfile array at vfile_array[fd-vf_base].
       The function (open close et al) comes out of the VFILEAPI structure
       pointer stored in the vfile array.
     */
    if (parms->vfile_array)
        tc_memset(parms->vfile_array, 0, sizeof(MFVFILE)*parms->n_vfiles);

    pmfsys->pvnode_array   = parms->vnode_array;
    pmfsys->pvfile_table   = parms->vfile_array;
    pmfsys->ntotal_vnodes  = parms->n_vnodes;
    pmfsys->vf_base        = parms->n_dirents;
    pmfsys->ntotal_vfiles  = parms->n_vfiles;
#endif /* INCLUDE_MTPOINTS */

#else
    /* INCLUDE_FILES was 0 so just create a single directory
    entry. This will create a file system with one file. mf_build()
    can be called to assign memory regions to this file which can then
    be uploaded to. This is useful for loading files to linear memory
    regions.
    */

    tc_memset(&(pmfsys->mf_dirent), 0, sizeof(MFDIRENT));
    pmfsys->mf_dirent.pparent = &(pmfsys->mf_dirent);

#endif /* INCLUDE_FILES */

#if (INCLUDE_DYNAMIC)
    /* If INCLUDE_DYNAMIC is set memory blocks of size blksize are allocated
       by the function in blkallocfn as files are extended. When the files
       are deleted the blocks are deallocated by calls to blkdeallocfn.
       The default allocator is bget in bget.c. See vf_init() for the
       default settings of these parameters */
    pmfsys->mf_block_size  = parms->blksize;
    pmfsys->mf_alloc_mem   = parms->blkallocfn;
    pmfsys->mf_free_mem    = parms->blkdeallocfn;
#endif /* INCLUDE_DYNAMIC */

    return (mf_get_api());
}

PVFILEAPI mf_get_api(void)
{
    mfs_api.fs_open      = (API_OPENFN)      mf_open;
    mfs_api.fs_read      = (API_READFN)      mf_read;
    mfs_api.fs_write     = (API_WRITEFN)     mf_write;
    mfs_api.fs_lseek     = (API_LSEEKFN)     mf_lseek;
    mfs_api.fs_truncate  = (API_TRUNCATEFN)  mf_truncate;
    mfs_api.fs_flush     = (API_FLUSHFN)     mf_flush;
    mfs_api.fs_close     = (API_CLOSEFN)     mf_close;
    mfs_api.fs_rename    = (API_RENAMEFN)    mf_rename;
    mfs_api.fs_delete    = (API_DELETEFN)    mf_delete;
    mfs_api.fs_mkdir     = (API_MKDIRFN)     mf_mkdir;
    mfs_api.fs_rmdir     = (API_RMDIRFN)     mf_rmdir;
    mfs_api.fs_set_cwd   = (API_SETCWDFN)    mf_set_cwd;
    mfs_api.fs_pwd       = (API_PWDFN)       mf_pwd;
    mfs_api.fs_gfirst    = (API_GFIRSTFN)    mf_gfirst;
    mfs_api.fs_gnext     = (API_GNEXTFN)     mf_gnext;
    mfs_api.fs_gdone     = (API_GDONEFN)     mf_gdone;
    mfs_api.fs_mountfs   = (API_MOUNTFSFN)   mf_mountfs;
    mfs_api.fs_unmountfs = (API_UNMOUNTFSFN) mf_unmountfs;
#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
    mfs_api.fs_get_file_modified_date   =
                           (API_GETMODDATE)  mf_modified_date;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_VF_ALL)
    mfs_api.fs_stat     = (API_STATFN)     mf_stat;
#endif
#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
    mfs_api.fs_chmode   = (API_CHMODEFN)   mf_chmode;
    mfs_api.fs_get_free = (API_GET_FREEFN) mf_get_free;
    mfs_api.fs_chsize   = (API_CHSIZEFN)   mf_chsize;
#endif

    return (&mfs_api);
}

/*****************************************************************************
 * mf_open - Open a file stream                                              *
 *                                                                           *
 * Parameters:                                                               *
 *      name - the name of file to open                                      *
 *                                                                           *
 *      flag - open flags; may be set to any combination of the following:   *
 *                                                                           *
 *          VO_NDELAY - not used                                             *
 *          VO_APPEND - seek to the end of the file upon opening             *
 *          VO_CREAT  - create the file if it doesn't exist; mf_open will    *
 *                      fail if name doesn't exist and VO_CREAT isn't set    *
 *          VO_TRUNC  - truncate file to length 0 upon opening               *
 *          VO_EXCL   - used with VO_CREAT; if VO_CREAT and VO_EXCL are      *
 *                      both set, the open will fail if the file exists.     *
 *                      if VO_EXCL is set without VO_CREAT, it has no        *
 *                      effect.                                              *
 *          VO_BINARY - open the file in binary mode                         *
 *          VO_TEXT   - open the file in text mode                           *
 *                                                                           *
 *      mode - file mode selector; can set to one of the following:          *
 *          VS_IWRITE - sets the file as writable when creating a file       *
 *          VS_IREAD  - sets the file as readable when creating a file       *
 *                                                                           *
 *          The mode parameter is only used when creating a file             *
 *                                                                           *
 * Returns:                                                                  *
 *      A valid file descriptor if the open was successful                   *
 *  -1 otherwise                                                             *
 *                                                                           *
 ****************************************************************************/

int mf_open(char *name, word flag, word mode)
{
    PMFDIRENT pdir;
#if (INCLUDE_FILES)
    PMFDIRENT pclone;

#if (DEBUG_MFS_API || DEBUG_MFS_OPEN)
    DEBUG_ERROR("mf_open: ", STR1, name, 0);
#endif

    MF_API_ENTER();

    /* This function will parse the pathname and return a pointer to
    the directory structure at the pathname. If the path crosses a
    mount point then the dirent returned will be the vnode of the
    mount point. When a vnode is returned mf_find_dirent() will append
    the path to the directory entry in the mounted file system to the
    native_path field in the vnode. This is used later to access the
    directory entry. */

    pdir = mf_find_dirent(name);

    /* File not found   */
    if (!pdir)
    {
#        if (DEBUG_MFS_OPEN)
            DEBUG_ERROR("mf_open: file not found", NOVAR, 0, 0);
#        endif
        if (flag & VO_CREAT)
        {
            pdir = mf_make_dirent(name);
            if (!pdir)
                MF_API_RETURN(-1);

            pdir->flags |= MF_FLAGS_ISFILE;

#if (INCLUDE_READ && INCLUDE_WRITE)
            if (mode == VS_IREAD)
                pdir->flags |= MF_FLAGS_RDONLY;
            else if (mode == VS_IWRITE)
                pdir->flags |= MF_FLAGS_WRONLY;
            else if (mode != (VS_IREAD | VS_IWRITE))    
            {
                DEBUG_ERROR("mf_open: mode is set to illegal value of : ",
                    EBS_INT1, mode, 0);
                MF_API_RETURN(-1);
            }
#elif (INCLUDE_READ)
            if (mode | VS_IREAD)
                pdir->flags |= MF_FLAGS_RDONLY;
#elif (INCLUDE_WRITE)
            if (mode | VS_IWRITE)
                pdir->flags |= MF_FLAGS_WRONLY;
#else   
            DEBUG_ERROR("mf_open: INCLUDE_READ and INCLUDE_WRITE are both off",
                NOVAR, 0, 0);
            pdir->flags = MF_FLAGS_RDONLY;
#endif
        }
        else
        {
#            if (DEBUG_MFS_OPEN)
                DEBUG_ERROR("mf_open: file not found and not create", NOVAR, 0, 0);
#            endif
            MF_API_RETURN(-1);
        }
    }
    else  /* file is found */
    {
#        if (DEBUG_MFS_OPEN)
            DEBUG_ERROR("mf_open: file found", NOVAR, 0, 0);
#        endif

#if (INCLUDE_SUBDIRS)
#if (INCLUDE_MTPOINTS)
        if (pdir->flags & MF_FLAGS_VNODE)
        {
            /* pdir is a mount point.   */
            PFCHAR fs_name;
            int fd, vfile;
            PMFVNODE pvnode = pdir->contents.pvnode;

            DEBUG_ASSERT(pvnode != NULL, "mf_open", NOVAR, 0, 0);
            if (mf_init_vnode(pvnode) >= 0) /* mf_init_vnode is a NOOP */
            {
                fs_name = mf_alloc_path();

                if (fs_name)
                {
                    /* Get the full path to the directory entry into fs_name
                    and then restore the vnode's native path field back
                    to the full path to the mount point */
                    if (mf_get_native_path(fs_name, pvnode))
                    {
#                        if (DEBUG_MFS_OPEN)
                            DEBUG_ERROR("mf_open: open native_path ", 
                                STR1, fs_name, 0);
#                        endif

                        /* Call the vnode's native open routine   */
                        fd = pvnode->filesys->fs_open(fs_name, flag, mode);
                        if (fd >= 0)
                        {
                            /* Grab a free virtual file descriptor and then
                            put the native file descriptor and a pointer to
                            the vnode in the virtual file table at the
                            new offset */
                            vfile = mf_alloc_vfile();
                            if (vfile)
                            {
                                mf_free_path(fs_name);
                                mf_bind_vfile(vfile, pvnode, fd);
                                MF_API_RETURN(vfile);
                            }
                        }
                    }
                    mf_free_path(fs_name);
                }
            }
            MF_API_RETURN(-1);
        }
#endif /* INCLUDE_MTPOINTS */
#endif /* INCLUDE_SUBDIRS */

        /* Path is a dir, not a file; use mf_gfirst, mf_gnext   */
        if ((pdir->flags & MF_FLAGS_ISDIR) ||
            ((flag & VO_CREAT) && (flag & VO_EXCL)))
            MF_API_RETURN(-1);
    }  /*end of file is found */

#else /* if (!INCLUDE_FILES) */
    /* If INCLUDE_FILES is zero then there is only one file in the system
    so we always succeed. The PDIR in this case is the only dirent in the
    system */
    if ((flag & VO_CREAT) && (flag & VO_EXCL))
        return (-1);

    MF_API_ENTER();

    pdir = &(pmfsys->mf_dirent);
    tc_strcpy(pdir->fname,name);

#endif /* INCLUDE_FILES */

    if (!pdir->opencount)
    {

        pdir->opencount++;

        MF_API_EXIT();
        if (flag & VO_TRUNC)
            mf_truncate(pdir->mfhandle,0);

        if (flag & VO_APPEND)
            mf_lseek(pdir->mfhandle,0,VSEEK_END);
        else
            mf_lseek(pdir->mfhandle,0,VSEEK_SET);

#if (INCLUDE_CALLBACK)
        /* If CALLBACKS are enabled call the open function for the
        file. Callbacks allow you to bind an open, close, read and
        write function to a file. (see mf_set_callback()). When
        the vf_api open, close,read,write fuctions are called the
        respective registered callback functions are called.
        As of this writing we need to add ioctl, lseek and private
        data to the callback scheme to increase it's usefulness. */
        if (pdir->openfn)
           pdir->openfn(pdir, name,flag,mode);
#endif

        return (pdir->mfhandle);
    }

#if (INCLUDE_FILES)
    /* The file is already opened so clone it. A clone is a directory
    entry whose parent pointer points to the file's directory entry.
    The clone file has a private file pointer for managing the seek
    pointer. */
    /* Open a clone of the file for reading only   */
    pclone = mf_alloc_dirent();
    if (!pclone)
        MF_API_RETURN(-1);
    pclone->pparent = pdir;
    pclone->flags = MF_FLAGS_ISCLONE;
    pclone->contents.pdata = pdir->contents.pdata;
    pdir->opencount++;
    MF_API_EXIT();
    mf_lseek(pclone->mfhandle,0,VSEEK_SET);

#if (INCLUDE_CALLBACK)
    pclone->openfn = pdir->openfn;
    pclone->closefn = pdir->closefn;
    pclone->readfn = pdir->readfn;
    pclone->writefn = pdir->writefn;
    pclone->seekfn = pdir->seekfn;
    pclone->callback_data = pdir->callback_data;

    if (pdir->openfn)
        pdir->openfn(pclone, name, flag, mode);
#endif

    return (pclone->mfhandle);

#else
    MF_API_RETURN(-1);
#endif /* INCLUDE_FILES */
}


/*************************************************************************
 * mf_read - Read from an open file stream                                       *
 *                                                                                               *
 * Parameters:                                                                               *
 *      fd - file descriptor of stream to read from                                  *
 *      buf - buffer to fill                                                                 *
 *      count - the number of bytes to read                                          *
 *                                                                                               *
 * Returns:                                                                                  *
 *      The number of bytes read                                                         *
 *                                                                                               *
 *************************************************************************/

int mf_read(int fd, byte *buf, word count)
{
#if (INCLUDE_CALLBACK)
PMFDIRENT pdir;
#endif

#if (DEBUG_MFS_API || DEBUG_MFS_READ)
    DEBUG_ERROR("mf_read entered - count = ", EBS_INT1, count, 0);
#endif

#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native read fn    */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
#if (DEBUG_MFS_API)
        DEBUG_ERROR("mf_read: virtual file: call native file system", 
            NOVAR, 0, 0);
#endif
        return (pmfsys->pvfile_table[vfile].pvnode->filesys->fs_read(pmfsys->pvfile_table[vfile].fd, buf, count));
    }
#endif /* INCLUDE_MTPOINTS */

#if (INCLUDE_CALLBACK)

    pdir = mf_fd_to_dirent(fd);

    if (pdir->readfn)
        return(pdir->readfn(pdir, buf, count));
#endif

    /* Otherwise call block io.   */
    return (mf_blockio(fd,buf,count,MF_BLOCK_READ));
}


/*************************************************************************
 * mf_write - Write to an open file stream                                       *
 *                                                                                               *
 * Parameters:                                                                               *
 *      fd - file descriptor of stream to write to                               *
 *      buf - buffer to copy into file                                               *
 *      count - the size of buf                                                          *
 *                                                                                               *
 * Returns:                                                                                  *
 *      The number of bytes written                                                  *
 *                                                                                               *
 *************************************************************************/

int mf_write(int fd, PFBYTE buf, word count)
{

#if (INCLUDE_CALLBACK)
PMFDIRENT pdir;
#endif

#if (DEBUG_MFS_API || DEBUG_MFS_WRITE)
    DEBUG_ERROR("mf_write: ", NOVAR, 0, 0);
#endif

#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native write fn   */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        return (pmfsys->pvfile_table[vfile].pvnode->filesys->fs_write(pmfsys->pvfile_table[vfile].fd, buf, count));
    }
#endif /* INCLUDE_MTPOINTS */

#if (INCLUDE_CALLBACK)

    pdir = mf_fd_to_dirent(fd);

    if (pdir->writefn)
        return(pdir->writefn(pdir, buf, count));
#endif

    return (mf_blockio(fd,buf,count,MF_BLOCK_WRITE));
}


/*************************************************************************
 * mf_nputc - Write count characters to an open file stream              *
 *                                                                       *
 * Parameters:                                                           *
 *      fd - file descriptor of stream to write to                       *
 *      to_put - the character to write                                  *
 *      count - the number of characters to write                        *
 *                                                                       *
 * Returns:                                                              *
 *      The number of characters written                                 *
 *                                                                       *
 *************************************************************************/

int mf_nputc(int fd, byte to_put, word count)
{
#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_nputc: ", NOVAR, 0, 0);
#endif
    return (mf_blockio(fd,&to_put,count,MF_BLOCK_NPUTC));
}


/*************************************************************************
 * mf_lseek - Set the file pointer of a file stream                      *
 *                                                                       *
 * Parameters:                                                           *
 *      fd - file descriptor of an open file stream                      *
 *      offset - number of bytes to move the file pointer from the origin  *
 *  origin - May be one of the following:                                *
 *              VSEEK_SET - offset is relative to beginning of the file  *
 *              VSEEK_CUR - offset is relative to the current file pointer   *
 *              VSEEK_END - offset is relative to the end of the file    *
 *                                                                       *
 * Returns:                                                              *
 *      New file pointer index                                           *
 *                                                                       *
 *************************************************************************/

/* Note: we need to add a callback seek function. This may be used for
   any purpose but the most practical use will be to seek to the end as
   a way of reporting the amount of data available for  subsequant read */

long mf_lseek(int fd, long offset, int origin)
{
    dword new_fptr;
    PMFDIRENT pdir;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_lseek: ", NOVAR, 0, 0);
#endif

    MF_API_ENTER();

    if (!mf_validate_fd(fd))
        MF_API_RETURN(0);

#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native lseek fn   */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        MF_API_RETURN(pmfsys->pvfile_table[vfile].pvnode->filesys->fs_lseek(pmfsys->pvfile_table[vfile].fd, offset, origin));
    }
#endif /* INCLUDE_MTPOINTS */

    pdir = mf_fd_to_dirent(fd);

#if (INCLUDE_CALLBACK)
    if (pdir->seekfn)
        return(pdir->seekfn(pdir, offset, origin));
#endif

    switch (origin)
    {
        case VSEEK_SET:
            pdir->fptr = 0;
            break;

        case VSEEK_END:
            if (pdir->flags & MF_FLAGS_ISCLONE)
                pdir->fptr = pdir->pparent->fsize;
            else
                pdir->fptr = pdir->fsize;
    }

    new_fptr = (pdir->fptr + offset);
    if (offset < 0)
    {
        if (new_fptr > pdir->fptr)
            pdir->fptr = 0;
        else
            pdir->fptr = new_fptr;
    }
    else
    {
        if (new_fptr < pdir->fptr)
            pdir->fptr = (dword)(-1);
        else
            pdir->fptr = new_fptr;
    }

#if (INCLUDE_BLOCKS)
    /* Traverse the block list. So the pdata pdata:index aligns with the      */
    /*   file pointer. pdata points to the block, pindex is the offset in the */
    /*   block                                                                */
    /* Get the first block                                                    */
    pdir->findex = 0;
    if (pdir->flags & MF_FLAGS_ISCLONE)
        pdir->contents.pdata = pdir->pparent->contents.pdata;

    if (pdir->contents.pdata)
    {
        dword n_to_move;

        /* Rewind to the first block   */
        while (pdir->contents.pdata->pprev)
            pdir->contents.pdata = pdir->contents.pdata->pprev;

        /* Set number of bytes to move   */
        n_to_move = pdir->fptr;
        while (n_to_move)
        {
            /* if fptr is in this block, set the block index and break   */
            if (pdir->contents.pdata->blsize >= n_to_move)
            {
                pdir->findex = n_to_move;
                break;
            }
            n_to_move -= pdir->contents.pdata->blsize;
            if (!pdir->contents.pdata->pnext)
            {
                pdir->findex = pdir->contents.pdata->blsize;
                break;
            }
            pdir->contents.pdata = pdir->contents.pdata->pnext;
        }
    }
#else
    /* Set the block index   */
    if (pdir->contents.pdata)
    {
        pdir->findex = (pdir->contents.pdata->blsize < pdir->fptr)?
            pdir->contents.pdata->blsize : pdir->fptr;
    }
#endif /* INCLUDE_BLOCKS */

    MF_API_RETURN(pdir->fptr);
}


/*************************************************************************
 * mf_truncate - Truncate a file to specified length                     *
 *                                                                       *
 * Parameters:                                                           *
 *      fd - file descriptor of open stream to truncate                  *
 *      offset - the size to truncate the file to                        *
 *                                                                       *
 * Returns:                                                              *
 *      TRUE if the file was successfully truncated                      *
 *      FALSE otherwise                                                  *
 *                                                                       *
 *************************************************************************/

RTIP_BOOLEAN mf_truncate(int fd, long offset)
{
    PMFDIRENT pdir;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_truncate: ", NOVAR, 0, 0);
#endif

    MF_API_ENTER();

    if (!mf_validate_fd(fd))
        MF_API_RETURN(FALSE);

#if (INCLUDE_MTPOINTS)
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        MF_API_RETURN(pmfsys->pvfile_table[vfile].pvnode->filesys->fs_truncate(pmfsys->pvfile_table[vfile].fd, offset));
    }
#endif

    pdir = mf_fd_to_dirent(fd);
    if ((pdir->flags & MF_FLAGS_ISCLONE) || !pdir->opencount || (pdir->opencount > 1))
        MF_API_RETURN(FALSE);

    if ( (pdir->fsize==0) || (offset >=0 && (dword) offset > pdir->fsize))
        MF_API_RETURN(TRUE);

    MF_API_EXIT();
    mf_lseek(fd, offset, VSEEK_SET);
    MF_API_ENTER();
    pdir->fsize = offset;

#if (INCLUDE_DYNAMIC)
#if (INCLUDE_BLOCKS)
    mf_free_block_chain(pdir->contents.pdata->pnext);
    pdir->contents.pdata->pnext = 0;
#endif
    if (offset == 0)
    {
        mf_free_block(pdir->contents.pdata);
        pdir->contents.pdata = 0;
    }
#endif /* INCLUDE_DYNAMIC */

    MF_API_RETURN(TRUE);
}


/*************************************************************************
 * mf_flush - Pseudo flush function                                                  *
 *                                                                                               *
 *      The concept of flushing a file's buffers makes no sense for memory *
 * files.  This function is included to make the MFS API compatible with *
 * the Virtual File System API abstraction layer.                                *
 *                                                                                               *
 *************************************************************************/
RTIP_BOOLEAN mf_flush(int fd)
{
#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_flush: ", NOVAR, 0, 0);
#endif

    MF_API_ENTER();

    if (!mf_validate_fd(fd))
        MF_API_RETURN(FALSE);

#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native read fn    */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        MF_API_RETURN (pmfsys->pvfile_table[vfile].pvnode->filesys->fs_flush(pmfsys->pvfile_table[vfile].fd));
    }
#endif /* INCLUDE_MTPOINTS */
    MF_API_RETURN(TRUE);
}


/*************************************************************************
 * mf_build - Add a static block to a memory file                        *
 *                                                                       *
 * Parameters:                                                           *
 *      fd - file descriptor of file to add block to                     *
 *      to_add - the static block to add                                 *
 *      data_size - number of bytes in to_add->data to use as data (may be *
 *          less than to_add->blsize)                                    *
 *                                                                       *
 * Returns:                                                              *
 *      TRUE if the block was successfully appended onto the file        *
 *      FALSE otherwise                                                  *
 *                                                                       *
 * See also:                                                             *
 *      MFBLOCK                                                          *
 *                                                                       *
 *************************************************************************/

RTIP_BOOLEAN mf_build(int fd, PMFBLOCK to_add, dword data_size)
{
#if (INCLUDE_STATIC)
    PMFDIRENT pdir;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_build: ", NOVAR, 0, 0);
#endif
    MF_API_ENTER();

    if (!mf_validate_fd(fd) || !to_add)
        MF_API_RETURN(FALSE);

#if (INCLUDE_MTPOINTS)
    /* build is only valid for memory files   */
    if (mf_is_vfile(fd))
        MF_API_RETURN(FALSE);
#endif

    pdir = mf_fd_to_dirent(fd);
    /*
        Return failure if:
            - pdir is a clone of a file (clones are read-only)
            - pdir is not open
            - no block is specified for build
            - pdir is a dynamic (non-static) file with blocks already attached
    */
    if ((pdir->flags & MF_FLAGS_ISCLONE) || !pdir->opencount || !to_add ||
        (!(pdir->flags & MF_FLAGS_STATIC) && pdir->contents.pdata))
        MF_API_RETURN(FALSE);

#if (INCLUDE_BLOCKS)
    MF_API_EXIT();
    mf_lseek(fd,0,VSEEK_END);
    MF_API_ENTER();
    pdir->flags |= MF_FLAGS_STATIC;
    to_add->pprev = pdir->contents.pdata;
    if (pdir->contents.pdata)
        pdir->contents.pdata->pnext = to_add;
    else
        pdir->contents.pdata = to_add;
#else
    if (pdir->contents.pdata)
        MF_API_RETURN(FALSE);
    pdir->contents.pdata = to_add;
#endif
    to_add->flags = MF_BLOCK_STATIC;
    if (data_size > to_add->blsize)
        data_size = to_add->blsize;
    pdir->fsize += data_size;

    MF_API_RETURN(TRUE);
#else
    return (FALSE);
#endif /* INCLUDE_STATIC */
}


/*************************************************************************  * 
 *mf_build_file - Build a memory file                                       *
 *                                                                          *
 * Parameters:                                                              *
 *                                                                          *
 * Returns:                                                                 *
 *                                                                          *
 * See also:                                                                *
 *      MFBLOCK                                                             *
 *                                                                          *
 *************************************************************************/

#if (INCLUDE_STATIC)
RTIP_BOOLEAN mf_build_file(char *path, char *filename, PFBYTE buf, long buf_len)
{
    PMFDIRENT pdir;
    int fd;
    RTIP_BOOLEAN ret = FALSE;
    
#if (DEBUG_MFS_API || DEBUG_MFS_BF)
    if (path && filename)
    {
        DEBUG_ERROR("mf_build_file: path, filename ", STR2, path, filename);
    }
    else if (filename)  
    {
        DEBUG_ERROR("mf_build_file: NO path, filename ", 
            STR1, filename, 0);
    }
#endif

    /* Assign compiled in web pages in virttbl.c to directory entries
       in the root  of the file system. */
    if (!vf_set_cwd(path))
        return FALSE;
        
#if (DEBUG_MFS_API || DEBUG_MFS_BF)
    DEBUG_ERROR("mf_build_file: filename ", STR1, filename, 0);
#endif

    /* Create the file   */
    fd = mf_open(filename, VO_CREAT /*|VO_TRUNC*/ |VO_EXCL|VO_RDWR, 
                    VS_IWRITE|VS_IREAD);
    if (fd >= 0)
    {
        pdir = mf_fd_to_dirent(fd);
#if (INCLUDE_BLOCKS)
        pdir->s_data.pnext = 0;
        pdir->s_data.pprev = 0;
#endif
        pdir->s_data.blsize = (dword)buf_len;
        pdir->s_data.data = buf;
        /* Assign the contents to the file   */
        ret = mf_build(fd, &pdir->s_data, buf_len);
        mf_close(fd);                              
    }
#if (DEBUG_MFS_BF)
    else
    {
        DEBUG_ERROR("mf_build_file: mf_open failed: filename ", 
            STR1, filename, 0);
    }
#endif
    return ret;
}
#endif


/*************************************************************************
 * mf_close - Close an open file stream                                          *
 *                                                                                               *
 * Parameters:                                                                               *
 *      fd - the file discriptor                                                         *
 *                                                                                               *
 * Returns:                                                                                  *
 *      0 if the file was successfully closed                                        *
 *  -1  otherwise                                                                        *
 *                                                                                               *
 *************************************************************************/

int mf_close(int fd)
{

PMFDIRENT pdir;

#if (INCLUDE_FILES)
    PMFDIRENT pclone;
#endif /* INCLUDE_FILES */

    MF_API_ENTER();

    if (!mf_validate_fd(fd))
        MF_API_RETURN(-1);

#if (INCLUDE_CALLBACK)

    pdir = mf_fd_to_dirent(fd);

    /* Call the callback close function if there is one   */
    if (pdir->closefn)
        pdir->closefn(pdir);
#endif


#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native close fn   */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        int ret;
        MF_API_EXIT();
        ret = pmfsys->pvfile_table[vfile].pvnode->filesys->fs_close(pmfsys->pvfile_table[vfile].fd);
        MF_API_ENTER();
        if (ret >= 0)
            mf_free_vfile(fd);
        MF_API_RETURN(ret);
    }
#endif /* INCLUDE_MTPOINTS */

    pdir = mf_fd_to_dirent(fd);
#if (INCLUDE_FILES)
    if (pdir->flags & MF_FLAGS_ISCLONE)
    {
        pclone = pdir;
        pdir = pclone->pparent;
        if (pdir->opencount)
        {
            mf_free_dirent(pclone);
            pdir->opencount--;
            MF_API_RETURN(0);
        }
        MF_API_RETURN(-1);
    }
#endif /* INCLUDE_FILES */

    if (!pdir->opencount)
        MF_API_RETURN(-1);

    MF_API_EXIT();
    mf_lseek(fd, 0, VSEEK_SET);
    MF_API_ENTER();
    pdir->opencount--;
    MF_API_EXIT();

    return (0);
}


/*************************************************************************
 * mf_validate_fd - Validate a file discriptor                                   *
 *                                                                                               *
 * Parameters:                                                                               *
 *      fd - the file descriptor                                                         *
 *                                                                                               *
 * Returns:                                                                                  *
 *      TRUE if fd is an open file stream                                            *
 *      FALSE otherwise                                                                  *
 *                                                                                               *
 *************************************************************************/

RTIP_BOOLEAN mf_validate_fd(int fd)
{
#if (INCLUDE_FILES && INCLUDE_MTPOINTS)
    if ((0 < fd) && (fd < pmfsys->vf_base + pmfsys->ntotal_vfiles))
        return(TRUE);
    else
        return(FALSE);

#elif (INCLUDE_FILES)
    if ((0 < fd) && (fd < pmfsys->ntotal_dirents))
        return(TRUE);
    else
        return(FALSE);


#else
    if (fd == 1)
        return(TRUE);
    else
        return(FALSE);

#endif /* INCLUDE_FILES */
}


/*************************************************************************
 * mf_rename - Rename a file or directory                                            *
 *                                                                                               *
 *      This function can be used to move a file or directory.               *
 *                                                                                               *
 * Parameters:                                                                               *
 *      name - the old name of the file to rename                                    *
 *      newname - the new name for the file                                          *
 *                                                                                               *
 * Returns:                                                                                  *
 *      TRUE if the file or directory was successfully renamed               *
 *      FALSE otherwise                                                                  *
 *                                                                                               *
 *************************************************************************/

RTIP_BOOLEAN mf_rename(char *name, char *newname)
{
    PMFDIRENT pdir;
    PMFDIRENT newparent;
#if (INCLUDE_SUBDIRS)
    char *newpath;
#endif /* INCLUDE_SUBDIRS */

    MF_API_ENTER();

#if (INCLUDE_FILES)
    pdir = mf_find_dirent(name);

    /* File not found   */
    if (!pdir)
        MF_API_RETURN(FALSE);
#else
    pdir = &(pmfsys->mf_dirent);
#endif /* INCLUDE_FILES */

#if (INCLUDE_MTPOINTS)
    if (pdir->flags & MF_FLAGS_VNODE)
    {
        PFCHAR fs_name;
        char *fs_newname;
        PMFVNODE pvnode = pdir->contents.pvnode;

        DEBUG_ASSERT(pvnode != NULL, "mf_rename", NOVAR, 0, 0);
        if (mf_init_vnode(pvnode) >= 0)
        {
            fs_name = mf_alloc_path();
            if (fs_name)
            {
                /* Get the native path of name into fs_name   */
                if (mf_get_native_path(fs_name, pvnode))
                {   /* Check if in the same filesystem
                    This has the side effect of priming vnode to
                    create the full native path of newnam when
                    get_native_path is next called. */
                    if (pvnode == mf_find_dirent(newname)->contents.pvnode)
                    {
                        fs_newname = mf_alloc_path();

                        if (fs_newname)
                        {
                            if (mf_get_native_path(fs_newname, pvnode))
                            {
                                /* This will break under RTFS. RTFS is broken   */
                                RTIP_BOOLEAN ret_val = pvnode->filesys->fs_rename(fs_name,fs_newname);

                                mf_free_path(fs_name);
                                mf_free_path(fs_newname);
                                MF_API_RETURN(ret_val);
                            }
                            mf_free_path(fs_newname);
                        }
                    }
                }
                mf_free_path(fs_name);
            }
        }
        MF_API_RETURN(FALSE);
    }
#endif /* INCLUDE_MTPOINTS */

    /* Fail if the file is open   */
    if (pdir->opencount)
        MF_API_RETURN(FALSE);

#if (INCLUDE_FILES)
    newparent = mf_find_dirent(newname);
    /* New name already in use   */
    if (newparent)
        MF_API_RETURN(FALSE);
#endif /* INCLUDE_FILES */

#if (INCLUDE_SUBDIRS)
    newpath = mf_alloc_path();
    if (!newpath)
        MF_API_RETURN(FALSE);
    /* Put the file name in newnam, the path in newpath   */
    newname = parse_parent(newpath,CHAR_BACKSLASH,newname);
    newparent = mf_find_dirent(newpath);

    if (newparent)
    {
        if (newparent->flags & MF_FLAGS_ISDIR)
        {
            /* Copy new name into dirent   */
            tc_strncpy(pdir->fname, newname, VF_FILENAMESIZE);

            /* Unlink from old dir list and link into the new. If
            the from and to directories are the same no harm will be
            done.
            */
            if (pdir->pprev)
                pdir->pprev->pnext = pdir->pnext;
            else
                pdir->pparent->contents.pchild = pdir->pnext;

            if (pdir->pnext)
                pdir->pnext->pprev = pdir->pprev;

            /* Link into new dir list   */
            pdir->pparent = newparent;
            pdir->pnext = newparent->contents.pchild;
            pdir->pprev = 0;
            if (newparent->contents.pchild)
                newparent->contents.pchild->pprev = pdir;
            newparent->contents.pchild = pdir;

            mf_free_path(newpath);
            MF_API_RETURN(TRUE);
        }
    }

    mf_free_path(newpath);
    MF_API_RETURN(FALSE);

#else
    /* If we have a root only file system, just copy newname into fname   */
    if (*newname == CHAR_BACKSLASH)
        newname++;
    tc_strncpy(pdir->fname, newname, VF_FILENAMESIZE);

    MF_API_RETURN(TRUE);
#endif /* INCLUDE_SUBDIRS */
}


/*************************************************************************
 * mf_delete - Delete a file or directory                                            *
 *                                                                                               *
 *      This function will fail in the following cases:                          *
 *          - name does not exist                                                        *
 *          - name is open                                                                   *
 *          - name is the root                                                           *
 *                                                                                               *
 * Parameters:                                                                               *
 *      name - the name of the file or directory to delete                       *
 *                                                                                               *
 * Returns:                                                                                  *
 *      TRUE if the file or directory was successfully deleted               *
 *      FALSE otherwise                                                                  *
 *                                                                                               *
 *************************************************************************/

RTIP_BOOLEAN mf_delete(char *name)
{
#if (INCLUDE_FILES)
    PMFDIRENT pdir;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_delete: ", NOVAR, 0, 0);
#endif
    MF_API_ENTER();

    pdir = mf_find_dirent(name);

    /* File not found   */
    if (!pdir)
        MF_API_RETURN(FALSE);

#if (INCLUDE_MTPOINTS)
    if (pdir->flags & MF_FLAGS_VNODE)
    {
        PFCHAR fs_name;
        PMFVNODE pvnode = pdir->contents.pvnode;

        DEBUG_ASSERT(pvnode != NULL, "mf_delete", NOVAR, 0, 0);
        if (mf_init_vnode(pvnode) >= 0)
        {
            fs_name = mf_alloc_path();
            if (fs_name)
            {
                if (mf_get_native_path(fs_name, pvnode))
                {
                    RTIP_BOOLEAN ret_val = pvnode->filesys->fs_delete(fs_name);

                    mf_free_path(fs_name);
                    MF_API_RETURN(ret_val);
                }
                mf_free_path(fs_name);
            }
        }
        MF_API_RETURN(FALSE);
    }
#endif /* INCLUDE_MTPOINTS */

    /* Can not delete an open file or the root   */
    if (pdir->opencount || (pdir->flags & MF_FLAGS_ISROOT))
        MF_API_RETURN(FALSE);

    /* Unlink from dir list   */
    if (pdir->pprev)
        pdir->pprev->pnext = pdir->pnext;
    else
        pdir->pparent->contents.pchild = pdir->pnext;

    if (pdir->pnext)
        pdir->pnext->pprev = pdir->pprev;

#if (INCLUDE_DYNAMIC)
#if (INCLUDE_BLOCKS)
    mf_free_block_chain(pdir->contents.pdata);
    pdir->contents.pdata = 0;
#else
    mf_free_block(pdir->contents.pdata);
    pdir->contents.pdata = 0;
#endif
#endif /* INCLUDE_DYNAMIC */

    mf_free_dirent(pdir);

    MF_API_RETURN(TRUE);
#else
    return (FALSE);
#endif /* INCLUDE_FILES */
}


/*************************************************************************
 * mf_mkdir - Create a subdirectory                                                  *
 *                                                                                               *
 *      This function will fail in the following cases:                          *
 *          - name already exists                                                        *
 *                                                                                               *
 * Parameters:                                                                               *
 *      name - the name of the subdirectory to create                            *
 *                                                                                               *
 * Returns:                                                                                  *
 *      TRUE if the directory was successfully created                           *
 *      FALSE otherwise                                                                  *
 *                                                                                               *
 *************************************************************************/

RTIP_BOOLEAN mf_mkdir(char *name)
{
#if (INCLUDE_SUBDIRS)
    PMFDIRENT pdir;

#if (DEBUG_MFS_API || DEBUG_MFS_MKDIR)
    DEBUG_ERROR("mf_mkdir: ", STR1, name, 0);
#endif
    MF_API_ENTER();

    pdir = mf_find_dirent(name);

    /* Directory already exists   */
    if (pdir)
    {
#if (INCLUDE_MTPOINTS)
        if (pdir->flags & MF_FLAGS_VNODE)
        {
            PFCHAR fs_name;
            PMFVNODE pvnode = pdir->contents.pvnode;

            DEBUG_ASSERT(pvnode != NULL, "mf_mkdir", NOVAR, 0, 0);
            if (mf_init_vnode(pvnode) >= 0)
            {
                fs_name = mf_alloc_path();
                if (fs_name)
                {
                    if (mf_get_native_path(fs_name, pvnode))
                    {
                        RTIP_BOOLEAN ret_val = pvnode->filesys->fs_mkdir(fs_name);
#if (DEBUG_MFS_MKDIR)
                        DEBUG_ERROR("mf_mkdir: MTPOINTS: dir exists: call native",
                            NOVAR, 0, 0);
#endif
                        mf_free_path(fs_name);
                        MF_API_RETURN(ret_val);
                    }
                    mf_free_path(fs_name);
                }
            }
            MF_API_RETURN(FALSE);
        }
#endif /* INCLUDE_MTPOINTS */
#if (DEBUG_MFS_MKDIR)
        DEBUG_ERROR("mf_mkdir: NO MTPOINTS: dir exists: return FALSE", NOVAR, 0, 0);
#endif
        MF_API_RETURN(FALSE);
    }

    pdir = mf_make_dirent(name);
    if (!pdir)
    {
#if (DEBUG_MFS_MKDIR)
        DEBUG_ERROR("mf_mkdir: NO MTPOINTS: mf_make_dirent failed: ", 
            STR1, name, 0);
#endif
        MF_API_RETURN(FALSE);
    }
    pdir->flags |= MF_FLAGS_ISDIR;

    MF_API_RETURN(TRUE);
#else
    ARGSUSED_PVOID(name)
    return (FALSE);
#endif /* INCLUDE_SUBDIRS */
}


/*************************************************************************
 * mf_rmdir - Remove a subdirectory                                                  *
 *                                                                                               *
 *      This function will fail in the following cases:                          *
 *          - name is not a subdirectory                                                 *
 *          - name is not empty                                                          *
 *          - name does not exist                                                        *
 *          - name is a file system mountpoint                                       *
 *          - name is the root                                                           *
 *                                                                                               *
 * Parameters:                                                                               *
 *      name - the name of the subdirectory to remove                            *
 *                                                                                               *
 * Returns:                                                                                  *
 *      TRUE if the directory was successfully removed                           *
 *      FALSE otherwise                                                                  *
 *                                                                                               *
 *************************************************************************/

RTIP_BOOLEAN mf_rmdir(char *name)
{
#if (INCLUDE_SUBDIRS)
    PMFDIRENT pdir;

#if (DEBUG_MFS_API || DEBUG_MFS_RMDIR)
    DEBUG_ERROR("mf_rmdir: ", STR1, name, 0);
#endif

#if (DEBUG_MFS_API || DEBUG_MFS_RMDIR)
{
char ppp[80];
    DEBUG_ERROR("mf_rmdir: ", STR1, name, 0);
    if (mf_pwd(ppp))
    {
        DEBUG_ERROR("mf_rmdir: pwd is ", STR1, ppp, 0);
    }
}
#endif
    MF_API_ENTER();

    pdir = mf_find_dirent(name);

#if (INCLUDE_MTPOINTS)
    if (pdir && pdir->flags & MF_FLAGS_VNODE)
    {
        PFCHAR fs_name;
        PMFVNODE pvnode = pdir->contents.pvnode;
#        if (DEBUG_MFS_RMDIR)
            DEBUG_ERROR("mf_rmdir: it is a VNODE", NOVAR, 0, 0);
#        endif
        DEBUG_ASSERT(pvnode != NULL, "mf_rmdir", NOVAR, 0, 0);
        if (mf_init_vnode(pvnode) >= 0)
        {
#            if (DEBUG_MFS_RMDIR)
                DEBUG_ERROR("mf_rmdir: mf_init worked", NOVAR, 0, 0);
#            endif
            fs_name = mf_alloc_path();
            if (fs_name)
            {
                if (mf_get_native_path(fs_name, pvnode))
                {
                    RTIP_BOOLEAN ret_val = pvnode->filesys->fs_rmdir(fs_name);

                    mf_free_path(fs_name);
                    MF_API_RETURN(ret_val);
                }
                mf_free_path(fs_name);
            }
        }
        MF_API_RETURN(FALSE);
    }
#endif /* INCLUDE_MTPOINTS */

#    if (DEBUG_MFS_RMDIR)
        DEBUG_ERROR("mf_rmdir: no MTPOINTS or didn't find node:", STR1, name, 0);
#    endif

    /* File not found   */
    if (!pdir)
        MF_API_RETURN(FALSE);

    /* Can not delete if its a mounted file system or the root dir
        or if the directory is not empty */
    if ((pdir->flags & (MF_FLAGS_VNODE | MF_FLAGS_ISROOT)) ||
        pdir->contents.pchild)
        MF_API_RETURN(FALSE);

#    if (DEBUG_MFS_RMDIR)
        DEBUG_ERROR("mf_rmdir: call mf_delete: ", STR1, name, 0);
#    endif
    MF_API_RETURN(mf_delete(name));
#else
    ARGSUSED_PVOID(name)
    return (FALSE);
#endif /* INCLUDE_SUBDIRS */
}


/*************************************************************************
 * mf_isdir - Determine if a path refers to a directory                      *
 *                                                                                               *
 * Parameters:                                                                               *
 *      path - the name of the file to test                                          *
 *                                                                                               *
 * Returns:                                                                                  *
 *      TRUE if path is a directory                                                  *
 *      FALSE otherwise                                                                  *
 *                                                                                               *
 * Note:                                                                                         *
 *      mf_isdir will return TRUE on the root directory                          *
 *                                                                                               *
 *************************************************************************/

RTIP_BOOLEAN mf_isdir(char *path)
{
#if (INCLUDE_SUBDIRS)
    PMFDIRENT pdir;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_isdir: ", NOVAR, 0, 0);
#endif
    MF_API_ENTER();

    pdir = mf_find_dirent(path);

    if (!pdir)
        MF_API_RETURN(FALSE);

    if (pdir->flags & MF_FLAGS_ISDIR)
    {
        MF_API_RETURN(TRUE);
    }
    else
        MF_API_RETURN(FALSE);
#else
    ARGSUSED_PVOID(path)
    return (FALSE);
#endif /* INCLUDE_SUBDIRS */
}


/*************************************************************************
 * mf_set_cwd - Set the current working directory                                *
 *                                                                                               *
 *   MFS just keeps track of current working directory and does
 *   it operations on the full path name; i.e. MFS never changes 
 *   actual cwd.
 *
 * Parameters:                                                                           *
 *      name - the path to set as the current working directory              *
 *                                                                                               *
 * Returns:                                                                                  *
 *      FALSE if name was not found                                                          *
 *    TRUE otherwise (success)                                                          *
 *                                                                                           *
 *************************************************************************/

RTIP_BOOLEAN mf_set_cwd(char *name)
{
#if (INCLUDE_SUBDIRS)
    PVDSTAT pstatobj;
    char *new_cwd;
#endif

#if (DEBUG_MFS_API || DEBUG_MFS_CWD)
    DEBUG_ERROR("mf_set_cwd: ", STR1, name, 0);
#endif

#if (INCLUDE_SUBDIRS)
    new_cwd = mf_alloc_path();
    if (!new_cwd)
        return(FALSE);
    pstatobj = (PVDSTAT) vf_alloc(sizeof(*pstatobj));
    if (!pstatobj)
    {
        mf_free_path(new_cwd);
        return(FALSE);
    }

    MF_API_ENTER();
    /* set new_cwd to current working dir with name concatonated   */
    mf_get_full_path(new_cwd, name);
    MF_API_EXIT();

#if (DEBUG_MFS_CWD)
    DEBUG_ERROR("mf_set_cwd: full path is ", STR1, new_cwd, 0);
#endif

    if (mf_gfirst(pstatobj,new_cwd))
    {
#if (DEBUG_MFS_CWD)
        DEBUG_ERROR("mf_set_cwd: mf_gfirst returned ", STR1, new_cwd, 0);
#endif

        if (!(pstatobj->fattributes & VF_ATTRIB_ISDIR))
        {
            mf_gdone(pstatobj);
            vf_free((PFBYTE) pstatobj);
            mf_free_path(new_cwd);
            return (FALSE);
        }
        mf_gdone(pstatobj);
        MF_API_ENTER();
        tc_strcpy(pmfsys->mfcwd,new_cwd);
#if (DEBUG_MFS_CWD)
        DEBUG_ERROR("mf_set_cwd: set pmfsys->mfcwd to ", STR1, new_cwd, 0);
#endif
        vf_free((PFBYTE) pstatobj);
        mf_free_path(new_cwd);
        MF_API_RETURN(TRUE);
    }
    vf_free((PFBYTE) pstatobj);
    mf_free_path(new_cwd);
    return (FALSE);
#else
    ARGSUSED_PVOID(name)
    return (TRUE);
#endif /* INCLUDE_SUBDIRS */
}


/*************************************************************************
 * mf_get_full_path - Convert relative path to full path. Full path is the
 * relative path appended to the current working directory.
 * Parameters:                                                                           *
 *      full_path - buffer to copy full path into                                    *
 *      relative_path - the relative path to convert                                 *
 *                                                                                               *
 * Returns:                                                                                  *
 *      full_path                                                                            *
 *                                                                                               *
 * Example:
 *
 *
 *************************************************************************/

char *mf_get_full_path(char *full_path, char *relative_path)
{
    if (*relative_path == CHAR_BACKSLASH)
    {
        /* Not a relative path since the root '\' was specified
           so eliminate all '.'s and '..'s and return the relative path
           as the full path */
        reduce_path(full_path,relative_path);
    }
    else
    {
        char *path;

        path = mf_alloc_path();
        if (!path)
        {
            return (full_path);
        }

#if (INCLUDE_SUBDIRS)
        tc_strncpy(path,pmfsys->mfcwd,CFG_MF_MAXPATH);
        path[CFG_MF_MAXPATH-1] = 0;
        if (*relative_path)
        {
            if (path[tc_strlen(path) - 1] != CHAR_BACKSLASH)
                tc_strcat(path,STRING_BACKSLASH);
            tc_strcat(path,relative_path);
        }
#else
        tc_strcpy(path,STRING_BACKSLASH);
        tc_strcat(path,relative_path);
#endif

        reduce_path(full_path,path);
        mf_free_path(path);
    }

    return (full_path);
}


/*************************************************************************
 * mf_pwd - Return the current working directory                                 *
 *                                                                                               *
 * Parameters:                                                                           *
 *      path - the buffer to fill with the path of the current working   *
 *              directory.                                                                   *
 *                                                                                               *
 * Returns:                                                                                  *
 *      Nothing                                                                              *
 *                                                                                           *
 *************************************************************************/
/* Note: we have a problem with PWD handling in multiuser systems   */
RTIP_BOOLEAN mf_pwd(char *path)
{
#if (DEBUG_MFS_PWD || DEBUG_MFS_API)
    DEBUG_ERROR("mf_pwd: ", NOVAR, 0, 0);
#endif

#if (INCLUDE_SUBDIRS)
    MF_API_ENTER();
    tc_strcpy(path, pmfsys->mfcwd);
#    if (DEBUG_MFS_PWD)
        DEBUG_ERROR("mf_pwd: SUBDIRS: return ", STR1, path, 0);
#    endif
    MF_API_RETURN(TRUE);
#else
    tc_strcpy(path, STRING_BACKSLASH);
#    if (DEBUG_MFS_PWD)
        DEBUG_ERROR("mf_pwd: NO SUBDIRS: return ", STR1, path, 0);
#    endif
    return(TRUE);
#endif
}

/*************************************************************************
 * mf_gfirst - Find the first entry in a subdirectory to match a pattern *
 *                                                                       *
 * Parameters:                                                           *
 *      dirobj - A vdstat structure that will contain the file name      *
 *               creation date and attributes of the match.              *
 *      name   - The pattern or file name to match on. Examples are      *
 *              "*.c", "datafile.dat", "*.*"                             *                                                                       *
 * Returns:                                                              *
 *      True if a match was found                                        *
 *                                                                       *
 * Note: mf_gdone must be called to free up resources if mf_gfirst       *
 *       succeeded.
 *************************************************************************/

RTIP_BOOLEAN mf_gfirst(PVDSTAT dirobj, char *name)
{
    char *pat;
    char *path;
    PMFDIRENT pmom;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_gfirst: ", NOVAR, 0, 0);
#endif
    /* dirobj MUST be allocated before calling gfirst, gnext, gdone   */
    if (!dirobj)
        return (FALSE);

    MF_API_ENTER();

    tc_memset(&dirobj->fs_obj_vfs, 0, sizeof(dirobj->fs_obj_vfs));
    dirobj->fs_api = NULL;

    path = mf_alloc_path();
    if (!path)
        MF_API_RETURN(FALSE);

    /* Get mom into path and the filename or wildcard expression in pat   */
    pat = parse_parent(path,CHAR_BACKSLASH,name);

#if (INCLUDE_SUBDIRS)
    /* Get the directory entry to search for instances of 'pat' in   */
    dirobj->fs_obj_vfs.pmom = mf_find_dirent(path);
    pmom = (PMFDIRENT)(dirobj->fs_obj_vfs.pmom);
#elif (INCLUDE_FILES)
    /* No subdirectory support so start from root   */
    dirobj->fs_obj_vfs.pmom = pmfsys->mfroot;
    pmom = (PMFDIRENT)(dirobj->fs_obj_vfs.pmom);
#else
    /* There is only one directory entry in the whole system   */
    pmom = &(pmfsys->mf_dirent);
#endif

    mf_free_path(path);

    if (pmom)
    {
#if (INCLUDE_MTPOINTS)
        if (pmom->flags & MF_FLAGS_VNODE)
        {
            PFCHAR fs_name;

            pmom = mf_find_dirent(name);

            if (pmom && (pmom->flags & MF_FLAGS_VNODE))
            {
                DEBUG_ASSERT(pmom->contents.pvnode != NULL, "mf_gfirst", NOVAR, 0, 0);
                if (mf_init_vnode(pmom->contents.pvnode) >= 0)
                {
                    fs_name = mf_alloc_path();
                    if (fs_name)
                    {
                        /* Do a gfirst specifying the full path and pattern   */
                        if (mf_get_native_path(fs_name, pmom->contents.pvnode))
                        {
                            RTIP_BOOLEAN ret_val;
                            dirobj->fs_api = pmom->contents.pvnode->filesys;
                            ret_val = dirobj->fs_api->fs_gfirst(dirobj, fs_name);
                            mf_free_path(fs_name);
                            MF_API_RETURN(ret_val);
                        }
                        mf_free_path(fs_name);
                    }
                }
            }
            MF_API_RETURN(FALSE);
        }
        dirobj->fs_api = &mfs_api;
#endif /* INCLUDE_MTPOINTS */

        if (*pat)
            tc_strncpy(dirobj->fs_obj_vfs.pattern,pat,VF_FILENAMESIZE);
        else
            tc_strcpy(dirobj->fs_obj_vfs.pattern,"*");

        dirobj->fs_obj_vfs.pdir = pmom->contents.pchild;

        tc_strcpy(dirobj->filename,".");
        dirobj->fattributes  = (word) ( (pmom->flags & MF_FLAGS_ISROOT)? VF_ATTRIB_ISROOT: 0);
        dirobj->fattributes |= (word)((pmom->flags & MF_FLAGS_ISDIR) ?
                                      VF_ATTRIB_ISDIR:  0);

        if ( !(pmom->flags & MF_FLAGS_RDONLY) &&
             !(pmom->flags & MF_FLAGS_WRONLY) )
            dirobj->fattributes |= VF_ATTRIB_RDWR;
        else if (pmom->flags & MF_FLAGS_WRONLY)
            dirobj->fattributes |= VF_ATTRIB_WRONLY;
        else if (pmom->flags & MF_FLAGS_RDONLY)
            dirobj->fattributes |= VF_ATTRIB_RDONLY;

        STRUCT_COPY(dirobj->ebs_mod_time, pmom->ebs_mod_time);
        dirobj->ftime = pmom->ftime;
        dirobj->fdate = pmom->fdate;
        dirobj->fsize = pmom->fsize;

        /* We are pointed at '.' if the pattern doesn't match '.' then
           call gnext to get an entry that does match the pattern. */
        if (!vf_patcmp(dirobj->fs_obj_vfs.pattern, (PFCHAR)".", TRUE))
        {
            MF_API_EXIT();
            return (mf_gnext(dirobj));
        }

        MF_API_RETURN(TRUE);
    }

    MF_API_RETURN(FALSE);
}


/*************************************************************************
 * mf_gnext - Continue a search that was started by mf_gfirst            *
 *                                                                       *
 * Parameters:                                                           *
 *      dirobj - A vdstat structure that was initialized by a sucessful  *
 *               call to mf_gfirst.                                      *
 *
 * Returns:                                                              *
 *      True if a match was found                                        *
 *                                                                       *
 * Note: mf_gdone must be called to free up resources if mf_gfirst       *
 *       succeeded.
 *************************************************************************/


RTIP_BOOLEAN mf_gnext(PVDSTAT dirobj)
{
    PMFDIRENT pdir;
    PMFDIRENT pmom;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_gnext: ", NOVAR, 0, 0);
#endif

    if (!dirobj)
        return (FALSE);

#if (INCLUDE_MTPOINTS)
    if (!dirobj->fs_api)
        return (FALSE);
    /* If the directory is on a mounted volume call it. Otherwise fall
       through if it is a memory file system */
    if (dirobj->fs_api->fs_gnext != mf_gnext)
        return (dirobj->fs_api->fs_gnext(dirobj));
#endif
    MF_API_ENTER();
    /* If we are at '.' and the match includes '..' make up '..'   */
    if (!tc_strcmp(dirobj->filename,".") &&
          vf_patcmp(dirobj->fs_obj_vfs.pattern, (PFCHAR)"..",TRUE))
    {
        pmom = (PMFDIRENT)(dirobj->fs_obj_vfs.pmom);
        tc_strcpy(dirobj->filename,"..");
        dirobj->fattributes  = (word)( (pmom->flags & MF_FLAGS_ISROOT) ?
                                       VF_ATTRIB_ISROOT: 0 );
        dirobj->fattributes |= (word)( (pmom->flags & MF_FLAGS_ISDIR) ?
                                       VF_ATTRIB_ISDIR:  0);

        if ( !(pmom->flags & MF_FLAGS_RDONLY) &&
             !(pmom->flags & MF_FLAGS_WRONLY) )
            dirobj->fattributes |= VF_ATTRIB_RDWR;
        else if (pmom->flags & MF_FLAGS_WRONLY)
            dirobj->fattributes |= VF_ATTRIB_WRONLY;
        else if (pmom->flags & MF_FLAGS_RDONLY)
            dirobj->fattributes |= VF_ATTRIB_RDONLY;

        STRUCT_COPY(dirobj->ebs_mod_time, pmom->pparent->ebs_mod_time);
        dirobj->ftime = pmom->pparent->ftime;
        dirobj->fdate = pmom->pparent->fdate;
        dirobj->fsize = pmom->pparent->fsize;
        dirobj->patt_match = TRUE;
        MF_API_RETURN(TRUE);
    }

    pdir = (PMFDIRENT)(dirobj->fs_obj_vfs.pdir);
    if (!pdir)
        MF_API_RETURN(FALSE);

    /* If not '.' and not '..' get the next entry   */
    if (tc_strcmp(dirobj->filename,"..") && tc_strcmp(dirobj->filename,"."))
    {
#if (INCLUDE_FILES)
        pdir = pdir->pnext;
#else
        if (pdir == &(pmfsys->mf_dirent))
            MF_API_RETURN(FALSE);
        pdir = &(pmfsys->mf_dirent);
#endif
    }

#if (INCLUDE_FILES)
    /* Find the next match. Note: the match is tested on pdir
       before it is incremented to pnext */
    pdir = mf_next_dirent(pdir, dirobj->fs_obj_vfs.pattern);
    if (!pdir)
        MF_API_RETURN(FALSE);
#endif

    tc_strncpy(dirobj->filename, pdir->fname, VF_FILENAMESIZE);
    dirobj->fattributes  = (word)((pdir->flags & MF_FLAGS_ISROOT) ?
                                  VF_ATTRIB_ISROOT: 0);
    dirobj->fattributes |= (word)((pdir->flags & MF_FLAGS_ISDIR) ?
                                  VF_ATTRIB_ISDIR:  0);

    if ( !(pdir->flags & MF_FLAGS_RDONLY) &&
         !(pdir->flags & MF_FLAGS_WRONLY) )
        dirobj->fattributes |= VF_ATTRIB_RDWR;
    else if (pdir->flags & MF_FLAGS_WRONLY)
        dirobj->fattributes |= VF_ATTRIB_WRONLY;
    else if (pdir->flags & MF_FLAGS_RDONLY)
        dirobj->fattributes |= VF_ATTRIB_RDONLY;

    dirobj->fs_obj_vfs.pdir = pdir;
    STRUCT_COPY(dirobj->ebs_mod_time, pdir->ebs_mod_time);
    dirobj->ftime = pdir->ftime;
    dirobj->fdate = pdir->fdate;
    dirobj->fsize = pdir->fsize;

    dirobj->patt_match = TRUE;
    MF_API_RETURN(TRUE);
}

/*************************************************************************
 * mf_gdone - Free up resources allocated by mf_gfirst                   *
 *                                                                       *
 * Parameters:                                                           *
 *      dirobj - A vdstat structure that was initialized by a sucessful  *
 *               call to mf_gfirst.                                      *
 *
 * Returns:                                                              *
 *      Nothing                                                          *
 *                                                                       *
 *************************************************************************/


void mf_gdone(PVDSTAT dirobj)
{
#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_gdone: ", NOVAR, 0, 0);
#endif
    if (!dirobj)
        return;

#if (INCLUDE_MTPOINTS)
    if (dirobj->fs_api)
    {
        if (dirobj->fs_api->fs_gdone != mf_gdone)
            dirobj->fs_api->fs_gdone(dirobj);
    }
    dirobj->fs_api = NULL;
#endif

    tc_memset(&dirobj->fs_obj_vfs, 0, sizeof(dirobj->fs_obj_vfs));
}

/**************************************************************************
 * mf_mountfs - Mount a file system onto a memory file system directory   *
 *              entry.                                                    *
 * Parameters:                                                            *
 *      mountpath - The memory file directory to mount to (mountpoint)    *
 *      nativepath- The "root" path in the native file system             *
 *      vfilesys  - Table of pointers to functions (read, open, write etc)*
 *      initfn    - Function that is called the first time the file system*
 *                  is accessed.                                          *
 *      init_arg  - Arguments passed to initfn                            *
 *                                                                        *
 * Returns:                                                               *
 *      TRUE if the mount was succesful
 *      FALSE if the mount failed                                         *
 **************************************************************************/


RTIP_BOOLEAN mf_mountfs(char *mountpath, char *nativepath, void * vfilesys, MPT_INITFN initfn, PFVOID init_arg)
{
#if (INCLUDE_MTPOINTS)
    PMFDIRENT mtpoint;
    PMFVNODE new_vnode;
    PVFILEAPI filesys;

#if (DEBUG_MFS_API || DEBUG_MOUNT_FS)
    DEBUG_ERROR("mf_mountfs: ", NOVAR, 0, 0);
#endif
    if (!vfilesys)
    {
        DEBUG_ERROR("mf_mountfs: no vfilesys: ", STR1,
            mountpath, 0);
        return (FALSE);
    }

    filesys = (PVFILEAPI) vfilesys;

    MF_API_ENTER();

    /* Find the Directory entry to mount on   */
    mtpoint = mf_find_dirent(mountpath);
    if (!mtpoint)
    {
        DEBUG_ERROR("mf_mountfs: could not find directory entry: ", STR1,
            mountpath, 0);
        MF_API_RETURN(FALSE);
    }

    /* Fail if it's the root, an active mount, a file or a populated subdir   */
    if ((mtpoint->flags & (MF_FLAGS_VNODE|MF_FLAGS_ISROOT|MF_FLAGS_ISFILE)) ||
         (mtpoint->contents.pchild))
    {
        DEBUG_ERROR("mf_mountfs: root, active mount or populated subdir",
            NOVAR, 0, 0);
        MF_API_RETURN(FALSE);
    }

    new_vnode = mf_alloc_vnode();
    if (!new_vnode)
    {
        DEBUG_ERROR("mf_mountfs: mf_alloc_vnode failed",
            NOVAR, 0, 0);
        MF_API_RETURN(FALSE);
    }

    /* Initialize new vnode   */
    tc_strncpy(new_vnode->native_path, nativepath, CFG_MF_MAXPATH);
    new_vnode->native_path[CFG_MF_MAXPATH-1] = 0;

    /* Make sure the te native path ends in '\' (this will break
       with unix directed files (we should fix this) */
    if (nativepath[tc_strlen(nativepath)-1] != CHAR_BACKSLASH)
        tc_strcat(new_vnode->native_path, STRING_BACKSLASH);
    new_vnode->native_pathlen = tc_strlen(new_vnode->native_path);
    new_vnode->filesys = filesys;
    new_vnode->init = initfn;
    new_vnode->init_arg = init_arg;
    mtpoint->contents.pvnode = new_vnode;
    mtpoint->flags |= MF_FLAGS_VNODE;

    MF_API_EXIT();

    return (TRUE);
#else
    DEBUG_ERROR("mf_mountfs: INCLUDE_MTPOINTS turned off",
            NOVAR, 0, 0);
    ARGSUSED_PVOID(vfilesys)
    ARGSUSED_PVOID(nativepath)
    ARGSUSED_PVOID(mountpath)
    ARGSUSED_PVOID(initfn)
    ARGSUSED_PVOID(init_arg)
    return (FALSE);
#endif
}


RTIP_BOOLEAN mf_unmountfs(char *mountpath)
{
#if (INCLUDE_MTPOINTS)
    PMFDIRENT pdir;

#if (DEBUG_MFS_API)
    DEBUG_ERROR("mf_unmountfs: ", NOVAR, 0, 0);
#endif
    MF_API_ENTER();

    pdir = mf_find_dirent(mountpath);

    if (!pdir || !(pdir->flags & MF_FLAGS_VNODE))
        return (FALSE);

    DEBUG_ASSERT(pdir->contents.pvnode != NULL, "mf_unmountfs", NOVAR, 0, 0);
    mf_free_vnode(pdir->contents.pvnode);
    pdir->contents.pvnode = 0;
    pdir->flags &= ~MF_FLAGS_VNODE;

    MF_API_RETURN(TRUE);
#else
    ARGSUSED_PVOID(mountpath)
    return (FALSE);
#endif
}


#if (INCLUDE_MTPOINTS)
PMFVNODE mf_alloc_vnode()
{
    int n;

    for (n=0; n<pmfsys->ntotal_vnodes; n++)
    {
        if (pmfsys->pvnode_array[n].filesys == 0)
            return (&pmfsys->pvnode_array[n]);
    }

    return (0);
}


void mf_free_vnode(PMFVNODE to_free)
{
    if (to_free)
        to_free->filesys = 0;
}


int mf_is_vfile(int fd)
{
    fd -= pmfsys->vf_base;
    return ((0 <= fd) && (fd < pmfsys->ntotal_vfiles));
}


int mf_get_native_path(PFCHAR fsname, PMFVNODE pvnode)
{
    DEBUG_ASSERT(pvnode != NULL, "mf_get_native_path", NOVAR, 0, 0);
    DEBUG_ASSERT(pvnode->native_path != NULL, "mf_get_native_path", NOVAR, 0, 0);
    DEBUG_ASSERT(pvnode->native_pathlen > 0, "mf_get_native_path", NOVAR, 0, 0);
    tc_strcpy(fsname, pvnode->native_path);
    pvnode->native_path[pvnode->native_pathlen] = 0;

    return (1);
}


int mf_alloc_vfile()
{
    int n;

    for (n=0; n<pmfsys->ntotal_vfiles; n++)
    {
        if (!pmfsys->pvfile_table[n].pvnode)
            return (n + pmfsys->vf_base);
    }

    return (0);
}


void mf_bind_vfile(int vfile, PMFVNODE pvnode, int to_fd)
{
    if (pmfsys->pvfile_table[vfile - pmfsys->vf_base].pvnode || !pvnode)
        return;

    pmfsys->pvfile_table[vfile - pmfsys->vf_base].fd = to_fd;
    pmfsys->pvfile_table[vfile - pmfsys->vf_base].pvnode = pvnode;
}


void mf_free_vfile(int vfile)
{
    int vf_index = vfile - pmfsys->vf_base;

    if ((0 <= vf_index) && (vf_index < pmfsys->ntotal_vfiles))
    {
        pmfsys->pvfile_table[vfile - pmfsys->vf_base].fd = 0;
        pmfsys->pvfile_table[vfile - pmfsys->vf_base].pvnode = 0;
    }
}

#endif /* INCLUDE_MTPOINTS */


/*************************************************************************
 * mf_fd_to_dirent - Return the dirent structure associated with a file  *
 *                          descriptor.                                                      *
 *                                                                                           *
 * Parameters:                                                                               *
 *      fd - the file descriptor to find the dirent of                           *
 *                                                                                               *
 * Returns:                                                                                  *
 *      The address of the MFDIRENT associated with fd                           *
 *                                                                                               *
 *************************************************************************/

PMFDIRENT mf_fd_to_dirent(int fd)
{
#if (INCLUDE_FILES)
    return (&(pmfsys->pdirent_array[fd]));
#else
    if (fd == 1)
        return (&(pmfsys->mf_dirent));
    return (0);
#endif
}


#if (INCLUDE_FILES)
/*************************************************************************
 * mf_find_dirent - Return the dirent structure associated with a path   *
 *                                                                       *
 * Parameters:                                                           *
 *    path - the full path of the directory entry to find                *
 *                                                                       *
 * Returns:                                                              *
 *    The address of the MFDIRENT corresponding to path                  *
 *                                                                       *
 *************************************************************************/

PMFDIRENT mf_find_dirent(char *raw_path)
{
    PMFDIRENT parent;
    char *path;
    char *_path;
    char dirent_name[CFG_MF_NAMESIZE];

    parent = pmfsys->mfroot;
#if (DEBUG_DIRENT)
    DEBUG_ERROR("mf_find_dirent: parent is ", DINT1, parent, 0);
#endif

    _path = mf_alloc_path();
    path = _path;
    if (!path)
    {
        DEBUG_ERROR("mf_find_dirent: mf_alloc_path failed", NOVAR, 0, 0);
        return (parent);
    }

    mf_get_full_path(path, raw_path);
    if (*path == CHAR_BACKSLASH)
        path++;

    dirent_name[0] = 0;
    while (parent)
    {
#if (INCLUDE_MTPOINTS)
        /* If we find a mountpoint somewhere along the path, save our offset
            into the path and return the mountpoint's MFDIRENT (MFVNODE) */
        if (parent->flags & MF_FLAGS_VNODE)
        {
            DEBUG_ASSERT(parent->contents.pvnode != NULL, "mf_find_dirent", NOVAR, 0, 0);
            parent->contents.pvnode->native_path[parent->contents.pvnode->native_pathlen] = 0;
            tc_strcat(parent->contents.pvnode->native_path, path);
            break;
        }
#endif /* INCLUDE_MTPOINTS */

        /* Parse up to the next path seperator - the parsed string
           will be written to dirent_name */
        path = parse_next(dirent_name, CHAR_BACKSLASH, path);
#if (DEBUG_DIRENT)
        DEBUG_ERROR("mf_find_dirent: parse_next returned dirent_name ", STR1,
            dirent_name, 0);
#endif

        /* If the next part of the path is null, we're done   */
        if (!dirent_name[0])
            break;

        if (parent->flags & MF_FLAGS_ISFILE)
        {
            mf_free_path(_path);
            DEBUG_ERROR("mf_find_dirent: parent is not a file", NOVAR, 0, 0);
            return (0);
        }

        /* Find the next piece of the path in the current directory   */
        parent = mf_next_dirent(parent->contents.pchild, dirent_name);
    }

    mf_free_path(_path);
    return (parent);
}


/*************************************************************************
 * mf_next_dirent - Return next DIRENT in pdir that matches pattern      *
 *                                                                                               *
 * Parameters:                                                                               *
 *      pdir - the first DIRENT to search                                            *
 *      pattern - the pattern to match (may contain * and ? as wildcards   *
 *                                                                                               *
 * Returns:                                                                                  *
 *      the address of the first DIRENT that matches the pattern             *
 *      or 0 if none match                                                               *
 *                                                                                               *
 *************************************************************************/

PMFDIRENT mf_next_dirent(PMFDIRENT pdir, char *pattern)
{
    while (pdir)
    {
#        if (DEBUG_DIRENT)
            DEBUG_ERROR("mf_next_dirent: pattern ", STR1, pattern, 0);
            DEBUG_ERROR("mf_next_dirent: fname ", STR1, pdir->fname, 0);
#        endif

        if (vf_patcmp(pattern, pdir->fname, 1))
            return (pdir);
        pdir = pdir->pnext;
    }
    return (0);
}


/*************************************************************************
 * mf_make_dirent - Allocate and initialize a DIRENT                             *
 *                                                                                               *
 * Parameters:                                                                               *
 *      parent - the directory to put the new DIRENT in.  if this is 0,  *
 *          the new DIRENT will not be linked into the file system tree      *
 *      name - the file name of the new DIRENT.  This string is copied       *
 *      into the fname field of the new DIRENT.                              *
 *                                                                                               *
 * Returns:                                                                                  *
 *      the address of the new MFDIRENT                                             *
 *                                                                                               *
 *************************************************************************/

PMFDIRENT mf_make_dirent(char *name)
{
    PMFDIRENT parent;
    PMFDIRENT new_dirent;
#if (INCLUDE_SUBDIRS)
    char path[CFG_MF_MAXPATH];

    name = parse_parent(path,CHAR_BACKSLASH,name);
    parent = mf_find_dirent(path);
    if (parent)
    {
        if (parent->flags & MF_FLAGS_ISDIR)
        {
#else
            parent = pmfsys->mfroot;
#endif /* INCLUDE_SUBDIRS */

            /* Return null if we're out of dirents to alloc   */
            new_dirent = mf_alloc_dirent();
            if (new_dirent)
            {

                /* Initialize new dirent   */
                tc_strncpy(new_dirent->fname, name, CFG_MF_NAMESIZE);
                new_dirent->pprev = 0;
                new_dirent->pparent = parent;
                new_dirent->contents.pchild = 0;
                new_dirent->flags = 0;
                new_dirent->fsize = 0;
                new_dirent->fptr = 0;
                new_dirent->findex = 0;
                new_dirent->contents.pdata = 0;
                new_dirent->opencount = 0;

                new_dirent->pnext = parent->contents.pchild;
                if (parent->contents.pchild)
                    parent->contents.pchild->pprev = new_dirent;
                parent->contents.pchild = new_dirent;
                return (new_dirent);
            }
#if (INCLUDE_SUBDIRS)
        }
    }
#endif /* INCLUDE_SUBDIRS */
    return (0);
}


/*************************************************************************
 * mf_alloc_dirent - Allocate a DIRENT from the free dirent pool         *
 *                                                                       *
 * Parameters:                                                           *
 *      None                                                             *
 *                                                                       *
 * Returns:                                                              *
 *      the address of the new MFDIRENT or 0 if the pool is empty        *
 *                                                                       *
 *************************************************************************/

PMFDIRENT mf_alloc_dirent(void)
{
PMFDIRENT to_ret;
int year, month, day, hour, minute, second;

    if (pmfsys->navail_dirents)
    {
        pmfsys->navail_dirents--;
        to_ret = pmfsys->pdirent_pool;
        pmfsys->pdirent_pool = to_ret->pnext;
        tc_memset(to_ret->fname,0,CFG_MF_NAMESIZE);
        to_ret->contents.pchild = 0;
        to_ret->pnext   = 0;
        to_ret->pprev   = 0;
        to_ret->pparent = 0;

        xn_ebs_get_system_time((PEBSTIME)&to_ret->ebs_mod_time);

        /* convert system time to format (fdate, ftime)   */
        /*  (yyyyyyymmmmddddd,  hhhhhmmmmmmsssss)         */
        convert_ebs_to_time((PEBSTIME)&to_ret->ebs_mod_time, &year, &month, 
                            &day, &hour, &minute, &second);
        /* hhhhhmmmmmmsssss   */
        to_ret->ftime   = (word)(
                          (hour << 11)                  /* Hour */
                          | (minute << 5)               /* Minute */
                          | (second >> 1));             /* Second */

        /* yyyyyyymmmmddddd                         */
        /* d=1-31, m=1-12, y=0-119 (1980-2099)      */
        to_ret->fdate   = (word)(
                          (month+1 << 5)                /* Month */
                          | (day)                       /* Day */
                          | ((year - 1980) << 9));      /* Year */

        to_ret->fsize   = 0;
        to_ret->fptr    = 0;
        to_ret->findex  = 0;
        to_ret->contents.pdata   = 0;
        return (to_ret);
    }

    return (0);
}


/*************************************************************************
 * mf_free_dirent - Return a DIRENT to the free dirent pool              *
 *                                                                       *
 * Parameters:                                                           *
 *      to_free - the address of the DIRENT to free                      *
 *                                                                       *
 * Returns:                                                              *
 *  Nothing                                                              *
 *                                                                       *
 *************************************************************************/

void mf_free_dirent(PMFDIRENT to_free)
{
    if (to_free)
    {
        to_free->flags = 0;
        pmfsys->navail_dirents++;
        to_free->pnext = pmfsys->pdirent_pool;
        pmfsys->pdirent_pool = to_free;
    }
}

#endif /* INCLUDE_FILES */

/* This code is disabled to save space because this function is not used in
the current implementation of the memory file system.  This code is fully
functional, and may be enabled if desired */
#if (INCLUDE_DYNAMIC)
/*************************************************************************
 * mf_alloc_block - Allocate a block for a file                                  *
 *                                                                                               *
 *      This function attempts to use the user supplied memory allocation  *
 *  functions stored in the file system context block.  The allocated    *
 *      MFBLOCK is stored at the beginning of the allocated block.  The  *
 *      default block size in the file system context is used.               *
 *                                                                                               *
 * Parameters:                                                                               *
 *      None                                                                                     *
 *                                                                                               *
 * Returns:                                                                                  *
 *      Pointer to a newly allocated and initialized MFBLOCK structure       *
 *                                                                                               *
 *************************************************************************/

PMFBLOCK mf_alloc_block()
{
    PMFBLOCK pblk;

    pblk = (PMFBLOCK) pmfsys->mf_alloc_mem((int)pmfsys->mf_block_size);
    if (pblk)
    {
#if (INCLUDE_BLOCKS)
        pblk->pprev = 0;
        pblk->pnext = 0;
#endif /* INCLUDE_BLOCKS */
        pblk->blsize = pmfsys->mf_block_size - sizeof(MFBLOCK);
        pblk->flags = 0;
        pblk->data = (PFBYTE)pblk + sizeof(MFBLOCK);
    }

    return (pblk);
}


/*************************************************************************
 * mf_free_block - Free a MFBLOCK                                                    *
 *                                                                                               *
 * Parameters:                                                                               *
 *      pblk - the MFBLOCK to free                                                       *
 *                                                                                               *
 * Returns:                                                                                  *
 *      Nothing                                                                              *
 *                                                                                               *
 *************************************************************************/

void mf_free_block(PMFBLOCK pblk)
{
    if (pblk)
    {
        if (!(pblk->flags & MF_BLOCK_STATIC))
            pmfsys->mf_free_mem((PFBYTE)pblk);
    }
}


#if (INCLUDE_BLOCKS)
/*************************************************************************
 * mf_free_block_chain - Free a chain of MFBLOCK structures                  *
 *                                                                                               *
 * Parameters:                                                                               *
 *      pblk - the first MFBLOCK in the chain to free                            *
 *                                                                                               *
 * Returns:                                                                                  *
 *      Nothing                                                                              *
 *                                                                                               *
 *************************************************************************/

void mf_free_block_chain(PMFBLOCK pblk)
{
    PMFBLOCK pnext;

    while (pblk)
    {
        pnext = pblk->pnext;
        mf_free_block(pblk);
        pblk = pnext;
    }
}
#endif /* INCLUDE_BLOCKS */
#endif /* INCLUDE_DYNAMIC */


/*************************************************************************
 * mf_blockio - MFBLOCK I/O operations                                               *
 *                                                                                               *
 * Parameters:                                                                               *
 *      fd - the file descriptor returned by mf_open of the stream to be     *
 *          used                                                                                 *
 *      buf - buffer to read out of or write into                                    *
 *      count - number of bytes to read/write                                        *
 *      operation - the operation to be performed. Can be any of these:  *
 *          MF_BLOCK_READ   - reads from an open stream                          *
 *          MF_BLOCK_WRITE  - writes to an open stream                               *
 *          MF_BLOCK_NPUTC  - writes count characters to an open stream      *
 *                                                                                               *
 *          The MF_BLOCK_NPUTC operation uses the first byte pointed to by   *
 *          buf.                                                                                 *
 *                                                                                               *
 * Returns:                                                                                  *
 *      Number of bytes actually read/written                                        *
 *                                                                                               *
 *************************************************************************/

int mf_blockio(int fd, byte *buf, word count, int operation)
{
    word n_to_do;
    word n_done;
    PMFDIRENT pdir;
    PMFDIRENT pclone;

    MF_API_ENTER();

#if (DEBUG_MFS_READ || DEBUG_MFS_WRITE)
    DEBUG_ERROR("mf_blockio: ", NOVAR, 0, 0);
#endif

    if (!mf_validate_fd(fd))
    {
        DEBUG_ERROR("mf_blockio: invalid file", NOVAR, 0, 0);
        MF_API_RETURN(0);
    }

    pdir = mf_fd_to_dirent(fd);
    pclone = pdir;
    if (pdir->flags & MF_FLAGS_ISCLONE)
    {
        pdir = pclone->pparent;
    }

    switch (operation)
    {
#if (INCLUDE_READ)
        case MF_BLOCK_READ:
#            if (DEBUG_MFS_READ || DEBUG_MFS_WRITE)
                DEBUG_ERROR("mf_blockio: MF_BLOCK_READ", NOVAR, 0, 0);
#            endif
            if (!pdir->opencount || pdir->flags & MF_FLAGS_WRONLY)
                MF_API_RETURN(0);

            /* Clip the read to the end of the file if necessary   */
            if (pclone->fptr + count > pdir->fsize)
                count = (word)(pdir->fsize - pclone->fptr);
            break;
#endif

#if (INCLUDE_WRITE)
        case MF_BLOCK_WRITE:
            if (pclone->fptr > pdir->fsize)
            {
                byte c=0;
                dword old_size;
                old_size = pclone->fptr = pdir->fsize;
                if (mf_blockio(fd,&c,(word)(pclone->fptr - pdir->fsize),MF_BLOCK_NPUTC) != (int) (pclone->fptr - pdir->fsize))
                {
                    DEBUG_ERROR("mf_blockio: BLOCK_WRITE: mf_blockio failed - truncate", 
                        NOVAR, 0, 0);
                    mf_truncate(fd,old_size);
                    MF_API_RETURN(0);
                }
            }

            /* FALL THROUGH   */

        case MF_BLOCK_NPUTC:
            if (!pdir->opencount || pdir->flags & MF_FLAGS_RDONLY)
            {
                DEBUG_ERROR("mf_blockio: BLOCK_NPUTC: failed: open cnt, rdonly flags", 
                    DINT2, pdir->opencount, pdir->flags & MF_FLAGS_RDONLY);
                MF_API_RETURN(0);
            }

            /* If the file is empty, try to allocate some memory for it   */
            if (!pclone->contents.pdata)
            {
#if (INCLUDE_DYNAMIC)
                /* Static files MUST be assigned block space manually   */
                if (pdir->flags & MF_FLAGS_STATIC)
                {
                    DEBUG_ERROR("mf_blockio: static file", NOVAR, 0, 0);
                    MF_API_RETURN(0);
                }

                /* If file is dynamic, allocate some memory for it   */
                pclone->contents.pdata = mf_alloc_block();

                /* No space is available   */
                if (!pclone->contents.pdata)
                {
                    DEBUG_ERROR("mf_blockio: no space availible", NOVAR, 0, 0);
                    MF_API_RETURN(0);
                }
#else
                /* Dynamic not supported; file is static, so return failure   */
                DEBUG_ERROR("mf_blockio: dynamic not supported, static", NOVAR, 0, 0);
                MF_API_RETURN(0);
#endif /* INCLUDE_DYNAMIC */
            }
            break;
#endif /* INCLUDE_WRITE */

        default:
            DEBUG_ERROR("mf_blockio: op not supported: (0=READ,1=WRITE,2=NPUTC)op = ", EBS_INT1, operation, 0);
            count = 0;
            break;
    }

#    if (DEBUG_MFS_READ || DEBUG_MFS_WRITE)
        DEBUG_ERROR("mf_blockio: switch done", NOVAR, 0, 0);
#    endif

    /* If already past end of file, bail   */
    if (pclone->fptr > pdir->fsize)
    {
        MF_API_RETURN(0);
    }

    n_done = 0;
    while (count)
    {
        if (count > pclone->contents.pdata->blsize - pclone->findex)
            n_to_do = (word)(pclone->contents.pdata->blsize - pclone->findex);
        else
            n_to_do = count;
        count -= n_to_do;

        if (n_to_do)
        {
            n_done += n_to_do;
            switch (operation)
            {
#if (INCLUDE_READ)
                case MF_BLOCK_READ:
#                if (DEBUG_MFS_READ || DEBUG_MFS_WRITE)
                    DEBUG_ERROR("mf_blockio: MF_BLOCK_READ: do movebytes: len=", 
                        EBS_INT1, n_to_do, 0);
#                endif
                    tc_movebytes((PFBYTE)buf,(PFBYTE)&(pclone->contents.pdata->data[pclone->findex]),
                                 (int)n_to_do);
                    buf += n_to_do;
                    break;
#endif /* INCLUDE_READ */

#if (INCLUDE_WRITE)
                case MF_BLOCK_WRITE:
                    tc_movebytes((PFBYTE)&(pclone->contents.pdata->data[pclone->findex]),buf,
                                 (int)n_to_do);
                    buf += n_to_do;
                    break;

                case MF_BLOCK_NPUTC:
                    tc_memset((PFBYTE)&(pclone->contents.pdata->data[pclone->findex]),*buf,
                              (int)n_to_do);
                    break;
#endif /* INCLUDE_WRITE */
            }
        }

#if (INCLUDE_BLOCKS)
        if (count)
        {
            if (pclone->contents.pdata->pnext)
            {
                pclone->contents.pdata = pclone->contents.pdata->pnext;
                pclone->findex = 0;
            }
            else
            {
#if (INCLUDE_DYNAMIC && INCLUDE_WRITE)
                if (operation == MF_BLOCK_READ)
                    break;

                /* Try allocate some memory for the file   */
                pclone->contents.pdata->pnext = mf_alloc_block();

                /* No space is available   */
                if (!pclone->contents.pdata->pnext)
                    break;

                /* Link the new block into the block chain and reset findex   */
                pclone->contents.pdata->pnext->pprev = pclone->contents.pdata;
                pclone->contents.pdata = pclone->contents.pdata->pnext;
                pclone->findex = 0;
#else
                break;
#endif /* INCLUDE_DYNAMIC && INCLUDE_WRITE */
            }
        }
        else
#endif /* INCLUDE_BLOCKS */
            pclone->findex += n_to_do;
    }
#    if (DEBUG_MFS_READ || DEBUG_MFS_WRITE)
        DEBUG_ERROR("mf_blockio: done with loop", NOVAR, 0, 0);
#    endif

    pclone->fptr += n_done;
    if (pclone->fptr > pdir->fsize)
        pdir->fsize = pclone->fptr;

    MF_API_RETURN(n_done);
}


PFCHAR mf_alloc_path(void)
{
    /* Allocate core from a DCU of a size MAXPATH   */
    return((PFCHAR)vf_alloc(CFG_MF_MAXPATH));
}

void mf_free_path(PFCHAR p)
{
    /* Free a DCU that was alloced by mf_alloc_path()   */
    if (p)
    {
        vf_free((PFBYTE)p);
    }
}

int mf_init_vnode(PMFVNODE pvnode)
{
    if (!pvnode)
        return -1;
    if (pvnode->init)
    {
        if (pvnode->init(pvnode->init_arg) < 0)
            return(-1);
        pvnode->init = 0;
    }

    return (0);
}

#if (INCLUDE_CALLBACK)
/* pdata: passed to callback routines as a parameter inside first parameter pdir   */
void mf_set_callback(char *name, PFVOID pdata, CALLBACK_OPENFN openfn,
    CALLBACK_CLOSEFN closefn, CALLBACK_READFN readfn, CALLBACK_WRITEFN writefn,
    CALLBACK_LSEEKFN seekfn)
{
    PMFDIRENT pdir;

    MF_API_ENTER();

#if (INCLUDE_FILES)
    pdir = mf_find_dirent(name);
#else

    pdir = &(pmfsys->mf_dirent);
#endif

    if (pdir)
    {
        pdir->openfn = openfn;
        pdir->closefn = closefn;
        pdir->readfn = readfn;
        pdir->writefn = writefn;
        pdir->seekfn = seekfn;
        pdir->callback_data = pdata;
    }

        MF_API_EXIT();
}
#endif

#if (INCLUDE_WEB || INCLUDE_FTP_SRV)
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

char *mf_modified_date(int fd, char *buffer, int buflen)
{
PMFDIRENT pdir;

#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native write fn   */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        return (char *) 
            (pmfsys->pvfile_table[vfile].pvnode->filesys->fs_get_file_modified_date(pmfsys->pvfile_table[vfile].fd, 
                                                                                    (PFBYTE) buffer, (word) buflen));
    }
#endif /* INCLUDE_MTPOINTS */

    ARGSUSED_INT(buflen)

    pdir = mf_fd_to_dirent(fd);
    if (pdir)
    {
        xn_ebs_print_time(buffer, (PEBSTIME)&pdir->ebs_mod_time, 1);
        return(buffer);
    }

    return(0);
}
#endif      /* INCLUDE_WEB and INCLUDE_FTP_SRV */

#if (INCLUDE_NFS_SRV || INCLUDE_FTP_SRV || INCLUDE_VF_ALL)
/* ********************************************************************   */
RTIP_BOOLEAN mf_stat(char *name, PVSTAT stat)
{
    PMFDIRENT pdir; 
        
    MF_API_ENTER();

#if (DEBUG_MFS_STAT || DEBUG_MFS_API)
    DEBUG_ERROR("mf_stat entered for ", STR1, name, 0);
#endif

    pdir = mf_find_dirent(name);

    /* Directory already exists     */
    if (pdir)
    {
#if (INCLUDE_MTPOINTS)
#if (DEBUG_MFS_STAT)
        DEBUG_ERROR("mf_stat: dir exists: MTPOINTS", NOVAR, 0, 0);
#endif
        if (pdir->flags & MF_FLAGS_VNODE)
        {
            PFCHAR fs_name;
            PMFVNODE pvnode = pdir->contents.pvnode;

            DEBUG_ASSERT(pvnode != NULL, "mf_stat", NOVAR, 0, 0);
            if (mf_init_vnode(pvnode) >= 0)
            {
                fs_name = mf_alloc_path();
                if (fs_name)
                {
                    if (mf_get_native_path(fs_name, pvnode))
                    {
                        RTIP_BOOLEAN ret_val = pvnode->filesys->fs_stat(fs_name, stat);
                        mf_free_path(fs_name);
                        MF_API_RETURN(ret_val);
                    }
                    mf_free_path(fs_name);
                }
            }
            MF_API_RETURN(FALSE);
        }
#endif /* INCLUDE_MTPOINTS */
        /* MEMORY FILE: perform stat function   */
        /* Read write always okay               */
        stat->vs_mode = 0;

        if (!(pdir->flags & MF_FLAGS_RDONLY))
            stat->vs_mode |= VSMODE_IWRITE;
        if (!(pdir->flags & MF_FLAGS_WRONLY))
            stat->vs_mode |= VSMODE_IREAD;

        stat->vs_mode = (VSMODE_IWRITE|VSMODE_IREAD);
        if (pdir->flags & (MF_FLAGS_ISROOT | MF_FLAGS_ISDIR))
        {
#if (DEBUG_MFS_STAT)
            DEBUG_ERROR("mf_stat: exists: is dir", NOVAR, 0, 0);
#endif
            stat->vs_mode |= VSMODE_IFDIR;
        }
        else
        {
#if (DEBUG_MFS_STAT)
            DEBUG_ERROR("mf_stat: exists: NOT dir", NOVAR, 0, 0);
#endif
            stat->vs_mode |= VSMODE_IFREG;
        }
        stat->vs_size = pdir->fsize;
        stat->vs_date = pdir->fdate;
        stat->vs_time = pdir->ftime;
        stat->vs_etime = 0; /* Time since 1970 ?? don't know */
        MF_API_RETURN(TRUE);
    }

    MF_API_RETURN(FALSE);
}
#endif

#if (INCLUDE_NFS_SRV || INCLUDE_VF_ALL)
/* ********************************************************************   */
RTIP_BOOLEAN mf_chmode(PFCHAR name, int attributes)
{
    PMFDIRENT pdir;

    MF_API_ENTER();

    pdir = mf_find_dirent(name);

    /* Directory already exists     */
    if (pdir)
    {
#if (INCLUDE_MTPOINTS)
        if (pdir->flags & MF_FLAGS_VNODE)
        {
            PFCHAR fs_name;
            PMFVNODE pvnode = pdir->contents.pvnode;

            DEBUG_ASSERT(pvnode != NULL, "mf_chmode", NOVAR, 0, 0);
            if (mf_init_vnode(pvnode) >= 0)
            {
                fs_name = mf_alloc_path();
                if (fs_name)
                {
                    if (mf_get_native_path(fs_name, pvnode))
                    {
                        RTIP_BOOLEAN ret_val =
                            pvnode->filesys->fs_chmode(fs_name, attributes);
                        mf_free_path(fs_name);
                        MF_API_RETURN(ret_val);
                    }
                    mf_free_path(fs_name);
                }
            }
            MF_API_RETURN(FALSE);
        }
#endif /* INCLUDE_MTPOINTS */
        /* MEMORY FILE: perform chmode function   */
        /* tbd: do memfile system code here       */

        pdir->flags |= MF_FLAGS_RDONLY; /*always read only */

        if (attributes & VS_IWRITE)
            pdir->flags &=  ~MF_FLAGS_RDONLY;
        if (attributes & VS_IREAD)
            pdir->flags &=  ~MF_FLAGS_WRONLY;
         MF_API_RETURN(TRUE);
    }

    MF_API_RETURN(FALSE);
}

/* ********************************************************************   */
RTIP_BOOLEAN mf_get_free(PFCHAR name, dword *blocks, dword *bfree)
{
    PMFDIRENT pdir;

    MF_API_ENTER();

    pdir = mf_find_dirent(name);

    /* Directory already exists     */
    if (pdir)
    {
#if (INCLUDE_MTPOINTS)
        if (pdir->flags & MF_FLAGS_VNODE)
        {
            PFCHAR fs_name;
            PMFVNODE pvnode = pdir->contents.pvnode;

            DEBUG_ASSERT(pvnode != NULL, "mf_get_free", NOVAR, 0, 0);
            if (mf_init_vnode(pvnode) >= 0)
            {
                fs_name = mf_alloc_path();
                if (fs_name)
                {
                    if (mf_get_native_path(fs_name, pvnode))
                    {
                        RTIP_BOOLEAN ret_val;
                        
                        DEBUG_ASSERT(pvnode->filesys != NULL, "mf_get_free", NOVAR, 0, 0);
                        DEBUG_ASSERT(pvnode->filesys->fs_get_free != NULL, "mf_get_free", NOVAR, 0, 0);
                        DEBUG_ASSERT(pvnode->filesys->fs_get_free != (API_GET_FREEFN)mf_get_free, "mf_get_free", NOVAR, 0, 0);
                        ret_val = pvnode->filesys->fs_get_free(fs_name, blocks, bfree);
                        mf_free_path(fs_name);
                        MF_API_RETURN(ret_val);
                    }
                    mf_free_path(fs_name);
                }
            }
            MF_API_RETURN(FALSE);
        }
#endif /* INCLUDE_MTPOINTS */
        /* MEMORY FILE: perform get_free function   */
        /* tbd - always return zero. can fix later  */
        *blocks = 0;
        *bfree = 0;
         MF_API_RETURN(TRUE);
    }

    MF_API_RETURN(FALSE);
}

/* ********************************************************************   */
RTIP_BOOLEAN mf_chsize(int fd, long size)
{
PMFDIRENT pdir;
    MF_API_ENTER();

    if (!mf_validate_fd(fd))
        MF_API_RETURN(FALSE);

#if (INCLUDE_MTPOINTS)
    /* If fd is a virtual file, call the native lseek fn     */
    if (mf_is_vfile(fd))
    {
        int vfile = fd - pmfsys->vf_base;
        MF_API_RETURN(pmfsys->pvfile_table[vfile].pvnode->filesys->fs_chsize(pmfsys->pvfile_table[vfile].fd, size));
    }
#endif /* INCLUDE_MTPOINTS */
    pdir = mf_fd_to_dirent(fd);
    if ((pdir->flags & MF_FLAGS_ISCLONE) || !pdir->opencount || (pdir->opencount > 1))
        MF_API_RETURN(FALSE);
    if (size > (long)pdir->fsize)
        MF_API_RETURN(FALSE);
    MF_API_EXIT();
    return(mf_truncate(fd, size));
}
#endif  /* INCLUDE_NFS_SRV || INCLUDE_VF_ALL */


#endif      /* INCLUDE_MFS */
