/*                                                                      */
/* POLLOS.C  -- Support for POLLOS, i.e. no kernel                      */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/* ********************************************************************   */
void pollos_dummy(void) {}
