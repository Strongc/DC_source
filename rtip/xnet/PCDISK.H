/*****************************************************************************
*Filename: PCDISK.H - Defines & structures for ms-dos utilities
*                     
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren , 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
*
*
* Description: 
*   
*
*
*
****************************************************************************/

#ifndef __PCDISK__
#define __PCDISK__ 1


/*
 * if not built 'Stand-Alone' and you defined INCLUDE_RTIP, load
 * xnconf.h to set the desired value for INCLUDE_RTIP and others.
 */              
#if defined(INCLUDE_RTIP)
/* load RTIP configuration settings   */
#include "xnconf.h"
#endif

#if (INCLUDE_RTIP)
#include "rtip.h"
#if (INCLUDE_ERTFS)
#include "rtfsapi.h"
#endif
#else   /* INCLUDE_RTIP */
#include "ebs.h"
#include "os.h"          /* OS specific */
#if (INCLUDE_ERTFS)
#include "rtfsapi.h"
#endif
#endif  /* INCLUDE_RTIP */


#include "pcconf.h"
#if (INCLUDE_ERTFS)
typedef unsigned long int BLOCKT;   /* 32 BIT unsigned */
#define BLOCKEQ0 0L

/* Include kernel specific definitions   */
#if (USE_ATA)
#include <ide_drv.h>
#endif

#if (USE_FLOPPY)
#include <fl_drver.h>
#endif


/* TBD: porting issue   */
#define tm_printf_1(X)      tm_printf(X)
#define tm_printf_2(X,Y)    tm_printf(X,Y)
#define tm_printf_3(X,Y,Z)  tm_printf(X,Y,Z)


#if (FAT32)
typedef unsigned long  CLUSTERTYPE;
#else
typedef unsigned short CLUSTERTYPE;
#endif

/* Device types (floppy or ide)   */
#define DEV_FLOPPY                  1
#define DEV_PCMCIA_ATA              2
#define DEV_PCMCIA_SRAM             3
#define DEV_TRUE_IDE_ATA            4
#define DEV_RAMDISK                 5
#define DEV_ROMDISK                 6


/* 10-24-2000 added LBA formatting. Submit to main tree   */

/* Structure used to reserve arrays of blocks in the BSS   */
typedef struct block_alloc {
        byte    core[512];
        } BLOCK_ALLOC;

#define PCDELETE (byte) 0xE5       /* MS-DOS file deleted char */

#if (VFAT)
#define FIRST_NAMESEG 0x40
#define NAMESEG_ORDER 0x3F
#endif

    /* Structure used to track cached fat blocks   */
typedef struct fatswap
{
    word  n_to_swap;  /* Next to swap. For implementing round robin */
        /* These two counters track cache usage as we fill it. Eventually the 
           FAT fills and we go into swapping mode at steady state */
    word  n_blocks_used;  /* How many blocks in the cache have we used */
    word  n_blocks_total; /* How many blocks are available in the cache */
    RTIP_BOOLEAN block_0_is_valid; /*  If TRUE then data_map[0] contains the offset
                                  of the first block of the FAT */
#if (FAT_BUFFER_SIZE > 63)
    /* This should work on systems with 32 bit integers and segments 
       but we'll leave it as an option for now */
    byte   pdirty[FAT_BUFFER_SIZE/8];
    byte   data_map[FAT_BUFFER_SIZE*4];
#else
    byte   pdirty[32];   /* BIT-map of blocks needing flushing */
    byte   data_map[256];   /* Table that maps block numbers in the fat to
                              block offsets in our data array. zero means 
                              the block is not mapped. Except.. data_map[0]
                              contains block zero of the FAT which
                              is at location 0 in the data array */
#endif
    PFBYTE data_array;   /* Block buffer area */
} FATSWAP;


    /* Structure to contain block 0 image from the disk   */
typedef struct ddrive {
        RTIP_BOOLEAN  mount_valid;       /* True if valid volume and 
                                       BLOCK 0, fat buffer, block pool 
                                       etc are valid */
        dword volume_serialno;      /* Volume serial number block 0 */
        byte  volume_label[14];     /* Volume entry from block 0 */
        word  bytespcluster;        /*  */
        dword byte_into_cl_mask;    /* And this with file pointer to get the
                                       byte offset into the cluster */
        word    fasize;             /* Nibbles per fat entry. (2 or 4) */
        BLOCKT  rootblock;          /* First block of root dir */
        BLOCKT  firstclblock;       /* First block of cluster area */
        word    driveno;            /* Driveno. Set when open succeeds */
        CLUSTERTYPE maxfindex;      /* Last element in the fat - FAT32*/
        BLOCKT  fatblock;           /* First block in fat */
        word    secproot;           /* blocks in root dir */
        RTIP_BOOLEAN fat_is_dirty;
        FATSWAP fat_swap_structure; /* Fat swap structure if swapping */
        dword   bootaddr;
        char    oemname[9];
        word    bytspsector;        /* Must be 512 for this implementation */
        byte    secpalloc;          /* Sectors per cluster */
        word    log2_secpalloc;     /* Log of sectors per cluster */
        word    secreserved;        /* Reserved sectors before the FAT */
        byte    numfats;            /* Number of FATS on the disk */
        word    numroot;            /* Maximum # of root dir entries */
        BLOCKT  numsecs;            /* Total # sectors on the disk */
        byte    mediadesc;          /* Media descriptor byte */
        CLUSTERTYPE secpfat;        /* Size of each fat */
        word    secptrk;            /* sectors per track */
        word    numhead;            /* number of heads */
        BLOCKT  numhide;            /* # hidden sectors */
        CLUSTERTYPE  free_contig_base;   /* Guess of where file data would most */
        CLUSTERTYPE  free_contig_pointer;/* Efficiently stored */  
#if (FAT32)
        long    known_free_clusters;/* If non-zero pc_free may use this value - FAT32 */
        word    infosec;
#else
        word    known_free_clusters;/* If non-zero pc_free may use this value */
#endif
    

/* NEWNEW                                                                        */
/* These arent new but they are moved to this section because they play a larger */
/* role. These values are loaded by the routine pc_read_partition_table(). And   */
/* used by formatting and read/write routines.                                   */


        BLOCKT partition_base;      /* Start of the partition */
        dword  partition_size;      /* Size of the partition  */
        int    partition_type;      /* Partition type */
        int    begin_user_area;     /* Beyond this is initialized at
                                       run time and must not be written
                                       by ertfs */

/* user init is required for the following elements required   */
        IOADDRESS register_file_address;
        int       interrupt_number;      /* note -1 is polled for IDE */

/* Flags - These must be set by the pc_ertfs_init   */
#define DRIVE_FLAGS_PARTITIONED     0x0002  /* Partitioned device  */
#define DRIVE_FLAGS_PCMCIA          0x0004  /* Pcmcia device */
#define DRIVE_FLAGS_PCMCIA_ATA      0x0008
/* Flags - These must be set by the warmstrt IOCTL call to the driver   */
#define DRIVE_FLAGS_VALID           0x0001  /* Flags have been set */
#define DRIVE_FLAGS_REMOVABLE       0x0040  /* Device is removable */
#define DRIVE_FLAGS_INSERTED        0x0080  /* Device drivers use to 
                                               remember states */
/* VALID is set by the device driver as a result of a successful call to 
   the device ioctl call DEVCTL_WARMSTART. If the driver does not set this 
   flag then it i assumed that the driver probe or init sequence failed
*/
        word drive_flags;
        int partition_number;
        int lock_unit;
        int pcmcia_slot_number;
        int pcmcia_controller_number;
        byte pcmcia_cfg_opt_value;

        int controller_number;
        int logical_unit_number;

        /* These two routines are attached to device driver specific routines   */
        RTIP_BOOLEAN (*dev_table_drive_io)(int driveno, dword sector, void KS_FAR *buffer, word count, RTIP_BOOLEAN readin);
        int (*dev_table_perform_device_ioctl)(int driveno, int opcode, PFVOID arg);
/* end user init required   */
        } DDRIVE;

    /* Dos Directory Entry Memory Image of Disk Entry   */
#define INOPBLOCK 16                /* 16 of these fit in a block */
typedef struct dosinode {
        byte    fname[8];
        byte    fext[3];
        byte    fattribute;      /* File attributes */
#if (FAT32)
        byte    resarea[8];
        word    fclusterhi; /* This is where FAT32 stores file location */
#else
        byte    resarea[10];
#endif
        word  ftime;              /* time & date lastmodified */
        word  fdate;
        word  fcluster;        /* Cluster for data file */
        dword   fsize;            /* File size */
        } DOSINODE;



#if (VFAT)

/* LFNINODE - This is an image of lfn extended names in a subdirectory   */
/* Note: lfn file names are null terminated unicode 00, the lfninode is
   padded mod 13 with ffff */
typedef struct lfninode {
                              /* The first LNF has 0x40 + N left   */
        byte    lfnorder;     /* 0x45, 0x04, 0x03, 0x02, 0x01 they are stored in
                               reverse order */
        byte    lfname1[10];
/*      UNICODE lfname1[5];   */
        byte    lfnattribute; /* always 0x0F */
        byte    lfnres;       /* reserved */
        byte    lfncksum;   /* All lfninode in one dirent have the same chksum */
        byte    lfname2[12];
/*      UNICODE lfname2[6];   */
        word    lfncluster; /* always 0x0000 */
        byte    lfname3[4];
/*      UNICODE lfname3[2];   */
        } LFNINODE;             

/* Trag lfn segments. segblock[0] is the block that contains beginning of the
   file name. segindex is the segment in that block where the beginning is
   stored. If the lfn spans > 1 block the next block # is in segblock[2] */
typedef struct segdesc {
    int nsegs;      /* # segs in the lfn */
    int segindex;
    byte ncksum;    /* checksum of the associated DOSINODE */
    BLOCKT segblock[2];
    } SEGDESC;

#endif


    /* Internal representation of DOS entry   */
    /* The first 8 fields !MUST! be identical to the DOSINODE structure.
       The code will be changed later so finode contains a DOSINODE (bug) */
typedef struct finode {
        byte   fname[8];
        byte   fext[3];
        byte   fattribute;       /* File attributes */
#if (FAT32)
        byte    resarea[8];
        word    fclusterhi; /* This is where FAT32 stores file location */
#else
        word    resarea[10];
#endif
        word  ftime;              /* time & date lastmodified */
        word  fdate;
        word  fcluster;        /* Cluster for data file */
        dword   fsize;            /* File size */

        dword   alloced_size;      /* Size rounded up to the hearest cluster 
                                       (only maintained for files */
        int   opencount;
/* If the finode is an open file the following flags control the sharing.
   they are maintained by po__open                                    */
#ifdef OF_WRITE
/* The watcom Windows include files define OF_WRITE too   */
#undef OF_WRITE
#endif
#define OF_WRITE            0x01    /* File is open for write by someone */
#define OF_WRITEEXCLUSIVE   0x02    /* File is open for write by someone 
                                       they wish exclusive write access */ 
#define OF_EXCLUSIVE        0x04    /* File is open with exclusive access not
                                       sharing write or read */
        int   openflags;          /* For Files. Track how files have it open */
        DDRIVE  *my_drive;
        BLOCKT  my_block;
        int   my_index;
        struct finode *pnext;      
        struct finode *pprev;
#if (VFAT)
        SEGDESC s;
#endif
        } FINODE;


/* contain location information for a directory   */
    typedef struct dirblk {
        BLOCKT  my_frstblock;      /* First block in this directory */
        BLOCKT  my_block;          /* Current block number */
        int     my_index;          /* dirent number in my cluster   */
    } DIRBLK;


/* Block buffer   */
typedef struct blkbuff {
        struct blkbuff *pnext;
        dword lru;
        DDRIVE *pdrive;
        BLOCKT blockno;
        int  use_count;
        int  pad_to_4_align;
        byte  data[512];
        } BLKBUFF;

/* Object used to find a dirent on a disk and its parent's   */
typedef struct drobj {
        DDRIVE  *pdrive;
        FINODE  *finode;
        DIRBLK  blkinfo;
        RTIP_BOOLEAN isroot;      /* True if this is the root */
        BLKBUFF *pblkbuff;
        } DROBJ;

/* Internal file representation   */
typedef struct pc_file {
    DROBJ *     pobj;           /* Info for getting at the inode */
    word        flag;           /* Acces flags from po_open(). */
    dword       fptr;           /* Current file pointer */
    CLUSTERTYPE fptr_cluster;   /* Current cluster boundary for fptr */
    dword       fptr_block;     /* Block address at boundary of fprt_cluster */
    RTIP_BOOLEAN     needs_flush;    /* If TRUE this FILE must be flushed */
    RTIP_BOOLEAN     is_free;        /* If TRUE this FILE may be used (see pc_memry.c) */
    RTIP_BOOLEAN     at_eof;         /* True if fptr was > alloced size last time we set
                                   it. If so synch_file pointers will fix it if the
                                   file has expanded. */
    } PC_FILE;

/* INTERNAL !!                                         */
/* Structure to contain block 0 image from the disk    */
#if (FAT32)
struct pcblk0 {
        byte  jump;               /* Should be E9 or EB on formatted disk */
        char  oemname[9];
        word  bytspsector;        /* Must be 512 for this implementation */
        byte  secpalloc;          /* Sectors per cluster */
        word  secreserved;        /* Reserved sectors before the FAT */
        byte  numfats;            /* Number of FATS on the disk */
        word  numroot;            /* Maximum # of root dir entries */
        word  numsecs;            /* Total # sectors on the disk */
        byte  mediadesc;          /* Media descriptor byte */
        word  secpfat;            /* Size of each fat */
        word  secptrk;            /* sectors per track */
        word  numhead;            /* number of heads */
        word  numhide;            /* # hidden sectors High word if DOS4 */
        word  numhide2;           /* # hidden sectors Low word if DOS 4 */
        dword numsecs2;           /* # secs if numhid+numsec > 32M (4.0) */
        dword secpfat2;           /* Size of FAT in sectors */
        byte  physdrv;            /* Physical Drive No. (4.0) */
        byte  filler;             /* Reserved (4.0) */
        byte  xtbootsig;          /* Extended signt 29H if 4.0 stuf valid */
        dword volid;              /* Unique number per volume (4.0) */
        char  vollabel[11];       /* Volume label (4.0) */
        byte  filler2[8];         /* Reserved (4.0) */

        word  flags;              /* Defined below */

#define NOFATMIRROR 0x0080
#define ACTIVEFAT   0x000F

        word    fs_version;         /* Version of FAT32 used */
        dword   rootbegin;          /* Location of 1st cluster in root dir */
        word    infosec;            /* Location of information sector */
        word    backup;             /* Location of backup boot sector */
        dword   free_alloc;         /* Free clusters on drive (-1 if unknown) */
        dword   next_alloc;         /* Most recently allocated cluster */
        };

struct fat32_info {
        dword   fs_sig;             /* Signature of FAT32 (0x61417272) */

#define FSINFOSIG 0x61417272ul

        dword   free_alloc;         /* Free clusters on drive (-1 if unknown) */
        dword   next_alloc;         /* Most recently allocated cluster */
        dword   reserved;           /* Reserved - ignore */
        };
#else
struct pcblk0 {
        byte  jump;             /* Should be E9 or EB on formatted disk */
        char  oemname[9];
        word  bytspsector;      /* Must be 512 for this implementation */
        byte  secpalloc;        /* Sectors per cluster */
        word  secreserved;      /* Reserved sectors before the FAT */
        byte  numfats;          /* Number of FATS on the disk */
        word  numroot;          /* Maximum # of root dir entries */
        word  numsecs;          /* Total # sectors on the disk */
        byte  mediadesc;        /* Media descriptor byte */
        word  secpfat;          /* Size of each fat */
        word  secptrk;          /* sectors per track */
        word  numhead;          /* number of heads */
        word  numhide;          /* # hidden sectors High word if DOS4 */
        word  numhide2;         /* # hidden sectors Low word if DOS 4 */
        dword numsecs2;         /* # secs if numhid+numsec > 32M (4.0) */
        byte  physdrv;          /* Physical Drive No. (4.0) */
        byte  filler;           /* Reserved (4.0) */
        byte  xtbootsig;        /* Extended signt 29H if 4.0 stuf valid */
        dword volid;            /* Unique number per volume (4.0) */
        char  vollabel[11];     /* Volume label (4.0) */
        byte  filler2[8];       /* Reserved (4.0) */
        };
#endif


/* Table for calculating sectors per cluster   */
typedef struct format_dec_tree {
   word  sec_p_alloc;
   word  ents_p_root;
   dword n_blocks;
} FORMAT_DEC_TREE;


/* Partition table descriptions.   */
/* One disk partition table        */
typedef struct ptable_entry {
    byte  boot;
    byte  s_head;
    word  s_cyl;
    byte  p_typ;
    byte  e_head;
    word e_cyl;
    dword  r_sec;   /* Relative sector of start of part */
    dword  p_size;  /* Size of partition */
    } PTABLE_ENTRY;

typedef struct ptable {
    PTABLE_ENTRY ents[4];
    word signature; /* should be 0xaa55 */
    } PTABLE;


#ifndef GLOBAL_INSTANCE
extern FINODE KS_FAR *inoroot;
extern DDRIVE KS_FAR *drv_array[NDRIVES];     /* Array of open drives */
extern DDRIVE KS_FAR *drno_to_dr_map[NDRIVES];

extern DDRIVE  KS_FAR *mem_drives_structures;
extern BLKBUFF KS_FAR *mem_block_pool;
extern PC_FILE KS_FAR *mem_file_pool;
extern BLOCK_ALLOC KS_FAR fat_drive_0[FAT_BUFFER_SIZE];     
#if (NDRIVES>1)
extern BLOCK_ALLOC KS_FAR fat_drive_1[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>2)
extern BLOCK_ALLOC KS_FAR fat_drive_2[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>3)
extern BLOCK_ALLOC KS_FAR fat_drive_3[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>4)
extern BLOCK_ALLOC KS_FAR fat_drive_4[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>5)
extern BLOCK_ALLOC KS_FAR fat_drive_5[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>6)
extern BLOCK_ALLOC KS_FAR fat_drive_6[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>7)
extern BLOCK_ALLOC KS_FAR fat_drive_7[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>8)
extern BLOCK_ALLOC KS_FAR fat_drive_8[FAT_BUFFER_SIZE];
#endif
#if (NDRIVES>9)
#error extern BLOCK_ALLOC KS_FAR fat_drive_9[FAT_BUFFER_SIZE];
#endif

#endif

#ifndef BUILDING_CONST
/* String constants   */
KS_EXTERN_GLOBAL_CONSTANT char string_star[2];
KS_EXTERN_GLOBAL_CONSTANT char string_padded_dot_dot[9];
KS_EXTERN_GLOBAL_CONSTANT char string_3_spaces[4];
KS_EXTERN_GLOBAL_CONSTANT char string_dot[2];
KS_EXTERN_GLOBAL_CONSTANT char string_dot_dot[3];
KS_EXTERN_GLOBAL_CONSTANT char string_null[1];
KS_EXTERN_GLOBAL_CONSTANT char string_backslash[3];
KS_EXTERN_GLOBAL_CONSTANT char string_fat_12[6];
KS_EXTERN_GLOBAL_CONSTANT char string_fat_16[6];
KS_EXTERN_GLOBAL_CONSTANT char char_backslash;

#endif
#define RTFS_FILE(X,Y)   /* This is used to split source files into 1 subroutrine
                           per file */
/* Make sure memory is initted prolog for api functions   */
#define CHECK_MEM(TYPE, RET)  if (!mem_drives_structures) return((TYPE) RET);
#define VOID_CHECK_MEM()  if (!mem_drives_structures) return;
#define IS_AVOLORDIR(X) ((X->isroot) || (X->finode->fattribute & AVOLUME|ADIRENT))

/* Prototypes   */

/* File INIT.C:   */
RTIP_BOOLEAN pc_ertfs_init(void);

/* File API.C:   */
long _po_lseek(PC_FILE *pfile, long offset, int origin);
RTIP_BOOLEAN _po_flush(PC_FILE *pfile);
RTIP_BOOLEAN pc_is(int op, char *path);
dword pc_find_contig_clusters(DDRIVE *pdr, CLUSTERTYPE startpt, CLUSTERTYPE  *pchain, CLUSTERTYPE min_clusters, int method);
int po_chsize(PCFD fd, long offset);

/* File APIUTIL.C:   */
RTIP_BOOLEAN pc_i_dskopen(int driveno);
int check_drive(char *name);
PC_FILE *pc_fd2file(PCFD fd,int flags);
PCFD pc_allocfile(void);
void pc_freefile(PCFD fd);
int pc_enum_file(DDRIVE *pdrive, int chore);
void pc_free_all_fil(DDRIVE *pdrive);
RTIP_BOOLEAN pc_flush_all_fil(DDRIVE *pdrive);
int pc_test_all_fil(DDRIVE *pdrive);
int pc_log_base_2(word n);
RTIP_BOOLEAN pc_dskinit(int driveno);
RTIP_BOOLEAN get_disk_volume(int driveno, byte *pvollabel, dword *pserialno);
RTIP_BOOLEAN pc_idskclose(int driveno);
DROBJ *pc_get_cwd(DDRIVE *pdrive);
void pc_upstat(DSTAT *statobj);
void  _synch_file_ptrs(PC_FILE *pfile);
void pc_finode_stat(FINODE *pi, STAT *pstat);
int pc_read_partition_table(int driveno, DDRIVE *pdr);

/* File BLOCK.C:   */
RTIP_BOOLEAN pc_alloc_blk(BLKBUFF **ppblk, DDRIVE *pdrive, BLOCKT blockno);
BLKBUFF *pc_blkpool(DDRIVE *pdrive);
void pc_free_all_blk(DDRIVE *pdrive);
void pc_free_buf(BLKBUFF *pblk, RTIP_BOOLEAN waserr);
BLKBUFF *pc_init_blk(DDRIVE *pdrive, BLOCKT blockno);
BLKBUFF *pc_read_blk(DDRIVE *pdrive, BLOCKT blockno);
RTIP_BOOLEAN pc_write_blk(BLKBUFF *pblk);
BLKBUFF *pc_scratch_blk(void);

/* File CHKMEDIA.C:   */
RTIP_BOOLEAN check_media_entry(int driveno);
RTIP_BOOLEAN check_media_io(int driveno, RTIP_BOOLEAN raw);
RTIP_BOOLEAN check_media(int driveno, RTIP_BOOLEAN ok_to_automount, RTIP_BOOLEAN raw_access_requested, RTIP_BOOLEAN call_crit_err);

int check_media_status(int driveno);
/* File DEVIO.C:   */
RTIP_BOOLEAN devio_read(int driveno, dword blockno, byte * buf, word n_to_read, RTIP_BOOLEAN raw);
RTIP_BOOLEAN devio_write(int driveno, dword blockno, byte * buf, word n_to_write, RTIP_BOOLEAN raw);
/* File DROBJ.C:   */
DROBJ *pc_fndnode(char *path);
DROBJ *pc_get_inode( DROBJ *pobj, DROBJ *pmom, byte *filename, byte *fileext, RTIP_BOOLEAN dowildcard);
#if (VFAT)
void pcdel2lfi(LFNINODE *lfi, int nsegs);
void pc_zeroseglist(SEGDESC *s);
void pc_addtoseglist(SEGDESC *s, BLOCKT my_block, int my_index);
void pc_reduceseglist(SEGDESC *s);
RTIP_BOOLEAN pc_deleteseglist(DDRIVE *pdrive, SEGDESC *s);
void text2lfi(char *lfn, LFNINODE *lfi, int nsegs, byte ncksum, byte order);
RTIP_BOOLEAN pc_seglist2disk(DDRIVE * pdrive, SEGDESC *s, char *lfn);
void lfi2text(char *lfn, LFNINODE *lfi, int nsegs);
char *pc_seglist2text(DDRIVE * pdrive, SEGDESC *s, char *lfn);
#endif

RTIP_BOOLEAN pc_findin( DROBJ *pobj, byte *filename, byte *fileext, RTIP_BOOLEAN dowildcard);
DROBJ *pc_get_mom(DROBJ *pdotdot);
DROBJ *pc_mkchild( DROBJ *pmom);
DROBJ *pc_mknode(DROBJ *pmom ,char *filename, char *fileext, byte attributes);
#if (VFAT)
RTIP_BOOLEAN pc_insert_inode( DROBJ *pobj, DROBJ *pmom, char *lfn);
#else
RTIP_BOOLEAN pc_insert_inode( DROBJ *pobj, DROBJ *pmom);
#endif
RTIP_BOOLEAN pc_rmnode( DROBJ *pobj);
RTIP_BOOLEAN pc_update_inode(DROBJ *pobj, RTIP_BOOLEAN set_archive, RTIP_BOOLEAN set_date);
DROBJ *pc_get_root( DDRIVE *pdrive);
BLOCKT pc_firstblock( DROBJ *pobj);
RTIP_BOOLEAN pc_next_block( DROBJ *pobj);
BLOCKT pc_l_next_block(DDRIVE *pdrive, BLOCKT curblock);
void pc_marki( FINODE *pfi, DDRIVE *pdrive, BLOCKT sectorno, int  index);
FINODE *pc_scani( DDRIVE *pdrive, BLOCKT sectorno, int index);
DROBJ *pc_allocobj(void);
FINODE *pc_alloci(void);
void pc_free_all_i( DDRIVE *pdrive);
void pc_freei( FINODE *pfi);
void pc_freeobj( DROBJ *pobj);
void pc_dos2inode (FINODE *pdir, DOSINODE *pbuff);
void pc_init_inode(FINODE *pdir, KS_CONSTANT char *filename, 
            KS_CONSTANT char *fileext, byte attr, 
            CLUSTERTYPE cluster, dword size, DATESTR *crdate);
void pc_ino2dos (DOSINODE *pbuff, FINODE *pdir);
RTIP_BOOLEAN pc_isavol( DROBJ *pobj);
RTIP_BOOLEAN pc_isadir( DROBJ *pobj);
RTIP_BOOLEAN pc_isroot( DROBJ *pobj);


/* File FORMAT.C:                     */
/* See end of pcdisk.h for prototypes */


/* File IDE_DRV.C:   */
/* See ide_drv.h     */

/* RAMDISK/ROMDISK   */
RTIP_BOOLEAN ramdisk_io(int driveno, dword block, void KS_FAR *buffer, word count, RTIP_BOOLEAN reading);
int ramdisk_perform_device_ioctl(int driveno, int opcode, PFVOID pargs);
RTIP_BOOLEAN romdisk_io(int driveno, dword block, void KS_FAR *buffer, word count, RTIP_BOOLEAN reading);
int romdisk_perform_device_ioctl(int driveno, int opcode, PFVOID pargs);
RTIP_BOOLEAN pcmsram_io(word driveno, dword block, void KS_FAR *buffer, word count, RTIP_BOOLEAN reading);
int pcmsram_perform_device_ioctl(int driveno, int opcode, PFVOID pargs);
RTIP_BOOLEAN hostdisk_io(word driveno, dword block, void KS_FAR *buffer, word count, RTIP_BOOLEAN reading);
int hostdisk_perform_device_ioctl(int driveno, int opcode, PFVOID pargs);
                
/* File LOWL.C:   */
CLUSTERTYPE pc_alloc_chain(DDRIVE *pdr, CLUSTERTYPE *pstart_cluster, CLUSTERTYPE n_clusters);
CLUSTERTYPE pc_find_free_cluster(DDRIVE *pdr, CLUSTERTYPE startpt, CLUSTERTYPE endpt);
CLUSTERTYPE pc_clalloc(DDRIVE *pdr, CLUSTERTYPE clhint);
CLUSTERTYPE  pc_clgrow(DDRIVE *pdr, CLUSTERTYPE  clno);
CLUSTERTYPE pc_clnext(DDRIVE *pdr, CLUSTERTYPE  clno);
void pc_clrelease(DDRIVE    *pdr, CLUSTERTYPE  clno);
RTIP_BOOLEAN pc_faxx(DDRIVE *pdr, CLUSTERTYPE clno, CLUSTERTYPE *pvalue);
RTIP_BOOLEAN pc_flushinfo(DDRIVE *pdr);
RTIP_BOOLEAN pc_flushfat(int driveno);
void pc_freechain(DDRIVE *pdr, CLUSTERTYPE cluster);
CLUSTERTYPE  pc_cl_truncate(DDRIVE *pdr, CLUSTERTYPE cluster, CLUSTERTYPE l_cluster);
CLUSTERTYPE pc_get_chain(DDRIVE *pdr, CLUSTERTYPE start_cluster, CLUSTERTYPE *pnext_cluster, CLUSTERTYPE n_clusters);
RTIP_BOOLEAN pc_pfaxxterm(DDRIVE   *pdr, CLUSTERTYPE  clno);
RTIP_BOOLEAN pc_pfaxx(DDRIVE *pdr, CLUSTERTYPE  clno, CLUSTERTYPE  value);
PFBYTE pc_pfswap(DDRIVE *pdr, CLUSTERTYPE index, RTIP_BOOLEAN for_write);
RTIP_BOOLEAN pc_fword(DDRIVE *pdr, CLUSTERTYPE index, word *pvalue, RTIP_BOOLEAN putting);
RTIP_BOOLEAN pc_pfpdword(DDRIVE *pdr, dword index, dword *pvalue);
RTIP_BOOLEAN pc_pfgdword(DDRIVE *pdr, dword index, dword *value);
RTIP_BOOLEAN pc_pfflush(DDRIVE *pdr);
RTIP_BOOLEAN pc_gblk0(word driveno, struct pcblk0 *pbl0);
RTIP_BOOLEAN pc_clzero(DDRIVE *pdrive, CLUSTERTYPE cluster);
DDRIVE  *pc_drno2dr(int driveno);
DDRIVE  *pc_drno_to_drive_struct(int driveno);
RTIP_BOOLEAN pc_dskfree(int driveno);

dword pc_ifree(int driveno);
CLUSTERTYPE pc_sec2cluster(DDRIVE *pdrive, BLOCKT blockno);
word pc_sec2index(DDRIVE *pdrive, BLOCKT blockno);
BLOCKT pc_cl2sector(DDRIVE *pdrive, CLUSTERTYPE cluster);
int ext_partition_init(int driveno, dword *pstart, dword *pend);
CLUSTERTYPE pc_finode_cluster(DDRIVE *pdr, FINODE *finode);
void pc_pfinode_cluster(DDRIVE *pdr, FINODE *finode, CLUSTERTYPE value);

/* File MINIEXEC.C:   */
void pc_report_error(int error_number);
DATESTR *pc_getsysdate(DATESTR *pd);
byte os_floppy_type(int driveno);

/* File PCKERNEL.C:   */
RTIP_BOOLEAN pc_kernel_init(void);
void  pc_free_all_users(int driveno);
/* File POWMGMT.C   */
int pcntrl_get_power_count(void);

/* File PC_MEMRY.C:   */
int pc_num_drives(void);
int pc_num_users(void);
int pc_nuserfiles(void);
RTIP_BOOLEAN pc_validate_driveno(int driveno);
RTIP_BOOLEAN pc_memory_init(void);
void pc_memory_close(void);
DROBJ *pc_memory_drobj(DROBJ *pobj);
FINODE *pc_memory_finode(FINODE *pinode);

/* File UTIL.C:   */
int pc_parse_raw_drive(char  *path);
RTIP_BOOLEAN pc_allspace(byte *p, int i);
void pc_cppad(byte *to, byte *from, int size);
RTIP_BOOLEAN pc_isdot(byte *fname, byte *fext);
RTIP_BOOLEAN pc_isdotdot(byte *fname, byte *fext);
byte *pc_mfile(byte *to, byte *filename, byte *ext);
byte *pc_mpath(byte *to, byte *path, byte *filename);
char *pc_parsedrive(int *driveno, char  *path);
RTIP_BOOLEAN pc_fileparse(char *filename, char *fileext, char *p);
char *pc_nibbleparse(char *filename, char *fileext, char *path);
#if (VFAT)
RTIP_BOOLEAN pc_parsepath(char *topath, char *filename, char *fileext, char *path);
#else
RTIP_BOOLEAN pc_parsepath(char *topath, char *filename, char *fileext, char *path);
#endif
#if (VFAT)
RTIP_BOOLEAN pc_patcmp(byte *pat, byte *name, RTIP_BOOLEAN dowildcard);
#else
RTIP_BOOLEAN pc_patcmp(byte *p, byte *pattern, int size, RTIP_BOOLEAN dowildcard);
#endif
char pc_byte2upper(char c);
void pc_str2upper(char *to, char *from);
void pc_strn2upper(char *to, char *from, int n);
RTIP_BOOLEAN validate_filename(PFCHAR name, int len);
RTIP_BOOLEAN pc_valid_sfn(char *filename);
RTIP_BOOLEAN pc_valid_lfn(char *filename);
RTIP_BOOLEAN pc_malias(char *fname, char *fext, byte *input_file, DROBJ *dest);
byte pc_cksum(byte *test);
dword to_DWORD ( byte *from);
word to_WORD ( byte *from);
void fr_WORD ( byte *to,  word from);
void fr_DWORD ( byte *to,  dword from);
int  critical_error_handler(int driveno, int media_status, dword sector);
#endif      /* INCLUDE_ERTFS */


/* Parameter block for formatting: Used by format.c   */
typedef struct fmtparms {
        char    oemname[9];      /* Only first 8 bytes are used */
        byte   secpalloc;         /* Sectors per cluster */
        word  secreserved;      /* Reserved sectors before the FAT */
        byte   numfats;         /* Number of FATS on the disk */
#if (FAT32)
        dword  secpfat;            /* Sectors per fat */
        byte    fmttype;            /* As defined below */

#define FMT_FAT32   0x01
#define FMT_DOS     0x00

#else
        word    secpfat;            /* Sectors per fat */
#endif
        dword   numhide;                /* Hidden sectors */
        word  numroot;          /* Maximum # of root dir entries */
        byte   mediadesc;         /* Media descriptor byte */
        word  secptrk;          /* sectors per track */
        word  numhead;          /* number of heads */
        word  numcyl;            /* number of cylinders */
        byte physical_drive_no;
        dword binary_volume_label;
        char  text_volume_label[12];
        } FMTPARMS;




/* These opcodes must be supported by the device driver's 
   perform_device_ioctl() routine. 
*/

#define DEVCTL_CHECKSTATUS 0

/*      DEVCTL_WARMSTART - Called when RTIP initializes.
            When ERTFS initializes it calls xxx_device_ioctl() with this 
            argument in ascending order for each possible logical device.
            The device driver may use this call to perform spin up of the 
            device or to clear driver specific information. In most cases
            this routine can do nothing and simply return zero.
            The device driver must return zero. There is no error condition
*/

#define DEVCTL_WARMSTART        1

/* This is called by RTFS when it notices that power has been restored. IE
   if current power count is not the same as it was the last time that the
   device was accessed. */

#define DEVCTL_POWER_RESTORE    2

/* Called when RTFS when power is going down.
!!! RTFS does not make this call. It is provided as a mechanism
    that that system power management system may use to communicate a power 
    down event to the device driver */
#define DEVCTL_POWER_LOSS       3


/*  Perform a low level format on the media. (If required). Return 0 on
    success or return a driver specific format error code. ERTFS does not 
    interpret the error code but it will abort a file system format operation 
    if anything but zero is returned. */

#define DEVCTL_FORMAT           4


/* 
        DEVCTL_GET_GEOMETRY - Report back to ertfs the geometry of the 
            device in the structure pointed to by the arg pointer.
            The structure pointed to at parg is of type PDEV_GEOMETRY:

        Note: The geometry must be in DOS HCN format. If the device is a
        logical block oriented device (where blocks are a contiguous 
        array of blocks) then the device must convert lba units to HCN units.

        The device should return zero if it can report the geometry.
        Any other return code will cause ERTFS to abort a file 
        system format operation.

*/

#define DEVCTL_GET_GEOMETRY     5


typedef struct dev_geometry {
    int dev_geometry_heads;      /*- Must be < 256 */
    dword dev_geometry_cylinders;  /*- Must be < 1024 */
    int dev_geometry_secptrack;  /*- Must be < 64 */
    dword dev_geometry_lbas;     /*- For oversize media */
    RTIP_BOOLEAN fmt_parms_valid;     /*If the device io control call sets this */
                                 /*TRUE then it it telling the applications    */
                                 /*layer that these format parameters should   */
                                 /*be used. This is a way to format floppy     */
                                 /*disks exactly as they are fromatted by dos. */
    FMTPARMS fmt;
} DEV_GEOMETRY;

#define DEVCTL_REPORT_REMOVE    6 /* this can be called by an external  */
                                  /* interrupt to tell the driver that      */
                                  /* the device has been removed. For       */
                                  /* pcmcia management interrupt calls      */
                                  /* this and the pcmcia aware drivers      */
                                  /* become aware tht they have to re-mount */



typedef struct dev_geometry KS_FAR *PDEV_GEOMETRY;


/* These codes are to be returned from the device driver's 
   perform_device_ioctl() function when presented with
   DEVCTL_CHECKSTATUS as an opcode.
*/

#define DEVTEST_NOCHANGE        0 /* Status is 'UP' */
#define DEVTEST_NOMEDIA         1 /*  The device is empty */
#define DEVTEST_UNKMEDIA        2 /*  Contains unknown media */
#define DEVTEST_CHANGED         3 /*  Controller recognized and cleared a */
                                  /*  change condition   */
#define BIN_VOL_LABEL 0x12345678L  
#define STR_VOL_LABEL "VOLUMELABE" /* 11 chars max */

#define READ_PARTION_OK   0 /* Partition read succesfully  */
#define READ_PARTION_ERR  -1 /* Internal error (couldn't allocate buffers ?) */
#define READ_PARTION_NO_TABLE   -2 /* No partition table found */
#define READ_PARTION_NO_ENTRY   -3 /* Request entry not found */
#define READ_PARTION_IOERROR    -4 /* Device IO error  */



/* File FORMAT.C:                     */
/* See end of pcdisk.h for prototypes */
RTIP_BOOLEAN pc_get_media_parms(char *path, PDEV_GEOMETRY pgeometry);
RTIP_BOOLEAN pc_format_media(char *path, PDEV_GEOMETRY pgeometry);
RTIP_BOOLEAN pc_partition_media(char *path, PDEV_GEOMETRY pgeometry, PFDWORD partition_list);
RTIP_BOOLEAN pc_format_volume(char *path, PDEV_GEOMETRY pgeometry);




#endif      /* __PCDISK__ */

