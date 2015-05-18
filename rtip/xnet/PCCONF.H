/*****************************************************************************
*Filename: PCCONF.H - RTFS tuning constants
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren, 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
* Description:
*   This file contains tuning constants for configuring RTFS.
*   It is included by pcdisk.h.
*
****************************************************************************/

#ifndef __PCCONF__
#define __PCCONF__ 1

#if (!defined(ERTFS_SA))
#include "xnconf.h"
#endif

#if (INCLUDE_ERTFS)

/* The following five constants when set to 0 selectively exclude portions
   of RTFS to save executable image size  */

/* Note: After we implemented VFAT we learned that Microsoft patented
   the Win95 VFS implementation. US PATENT # 5,758,352.
   Leaving VFAT set to zero will exclude potential patent infringment
   problems.
   3-19-99
*/

#define VFAT              0  /* Set to 1 to support long filenames */
#define FAT32             1  /* Set to 1 to support 32 bit FATs */
#define RTFS_SHARE        0  /* Set to 0 to disable file share modes saves ~0.5 K */
#define RTFS_SUBDIRS      1  /* Set to 0 to disable subdir support saves 2.5 K */
#define RTFS_WRITE        1  /* Set to 0 to disable write support saves 5.5 K */


#if (VFAT)
typedef unsigned short int UNICODE;
#define FILENAMESIZE    256
#else
#define FILENAMESIZE      9
#endif

#if (defined(POLLOS))
#define NUM_USERS         1  /* Must be one for POLLOS */
#else
#define NUM_USERS         5  /* Maximum # of tasks that may use the file
                                system */
#endif

#define NBLKBUFFS        10  /* Number of blocks in the buffer pool. Uses 532
                                bytes per block. Impacts performance during
                                directory traversals Since disk Reads on
                                Sandisk is very fast we can keep this low
                                (must be at least 1) */

#define NUSERFILES       10  /* Maximum Number of open Files */

#define FAT_BUFFER_SIZE   2  /* Size of the internal FAT buffer in 512 byte
                                chunks (ie 12 = 12 * 512 bytes. We statically
                                allocate arrays based on this in pc_memry.c
                                Small values should be OK in the Sandisk
                                environment since IO is fast. Note that
                                reducing the Buffer size will increase the
                                amount of fat flushing. Must be at least 1. */

#if (VFAT)
#define EMAXPATH        260  /* Maximum path length. Change if you like */
#else
#define EMAXPATH        148  /* Maximum path length. Change if you like */
#endif

/* *****************************************************    */
/* Set to  1 to include CDFS                                */
/* *****************************************************    */
#define INCLUDE_CDFS 0


/* *****************************************************                   */
/* DEVICES                                                                 */
/* *****************************************************                   */
/* Device access is supposed to be purely table driven with                */
/* the responsibility for initializing the device table resting            */
/* on the routine pc_ertfs_init(void) in pc_init.c. But for floppy         */
/* and IDE drives and the PCMCIA subsystem there are some resources        */
/* in other files that are only required if these devices are included     */
/* so we define them here                                                  */

/* *****************************************************    */
/* FLOPPY                                                   */
/* *****************************************************    */
#define USE_FLOPPY              1  /* If 1 use the floppy interface */
#define N_FLOPPY_UNITS          2  /* If USE_FLOPPY is 1 this is the number of units */

/* *****************************************************    */
/* ATA                                                      */
/* *****************************************************    */
#define USE_ATA                 1  /* If 1 provide ATA drivers */
#define N_ATA_CONTROLLERS       2  /* If USE_ATA is 1 this is the number of controllers */

/* *****************************************************    */
/* PCMCIA                                                   */
/* *****************************************************    */
#ifndef INCLUDE_PCMCIA
#define INCLUDE_PCMCIA          0  /* If 1 provide PCMCIA drivers. Note:
                                  RTIP may have turned this on already */
#endif

/* Constants used by pcmctrl.c   */

/* Note: The PCMCIA configuration section is in the header file PCMCIA.H   */

/* The following constants may be found in pcmci.h
    ISA_MEM_WINDOW_0
    ISA_MEM_WINDOW_1
    MGMT_INTERRUPT
    NSOCKET 2
*/


/* *****************************************************    */
/* DEVICES -- MAXIMUM NUMBER OF UNITS                       */
/* *****************************************************    */


/* Number of drives to support A:,B:,C: ....   */
#define NDRIVES              8


/* *****************************************************    */
/* DERIVED CONSTANTS                                        */
/* *****************************************************    */

/* Directory Object Needs. Conservative guess is One CWD per user per drive +
   One per file + one per User for directory traversal */
/* This is calculated... do not change   */
#define NFINODES   (32 + NUM_USERS*NDRIVES + NUM_USERS + NUSERFILES)
#define NDROBJS    (32 + NUM_USERS*NDRIVES + NUM_USERS + NUSERFILES)

/* *****************************************************    */
/* Sanity Checks for SA vs RTIP vs OTHER                    */
/* *****************************************************    */

#if !defined(INCLUDE_RTIP)
#define INCLUDE_RTIP        0
#endif

#if !defined(INCLUDE_NE2000)
#define INCLUDE_NE2000      0
#endif
#if !defined(INCLUDE_SMC91C9X)
#define INCLUDE_SMC91C9X    0
#endif
#if !defined(INCLUDE_SMC8041_PCMCIA)
#define INCLUDE_SMC8041_PCMCIA 0
#endif
#if !defined(INCLUDE_3C589)
#define INCLUDE_3C589       0
#endif
#if !defined(INCLUDE_XIRCOM)
#define INCLUDE_XIRCOM      0
#endif
#if !defined(INCLUDE_ETH860)
#define INCLUDE_ETH860      0
#endif

#if !defined(INCLUDE_FAT32_BOOT_CODE)
#define INCLUDE_FAT32_BOOT_CODE  0
#endif

/* eth860.c related defines   */
#if !defined(INCLUDE_SLIP)
#define INCLUDE_SLIP             0
#endif
#if !defined(INCLUDE_CSLIP)
#define INCLUDE_CSLIP            0
#endif
#if !defined(INCLUDE_PPP)
#define INCLUDE_PPP              0
#endif



#endif  /* INCLUDE_ERTFS */
#endif /* __PCCONF__ */



