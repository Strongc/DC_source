/** \file MPCBasicFirmwareUpdater_C.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC Firmware Update via Ethernet, Basic
 * Projectno.: 5075
 * File: MPCBasicFirmwareUpdater_C.h
 *
 * \brief Headerfile with C interface to MPCBasicFirmwareUpdater class
 *
 * $Log: MPCBasicFirmwareUpdater_C.h,v $
 * Revision 1.1  2005/07/13 12:22:48  jla
 * *** empty log message ***
 *
 */

#ifndef _MPCBASICFIRMWAREUPDATER_C_H_
#define _MPCBASICFIRMWAREUPDATER_C_H_


int packetRecvCallback_C(const char *data, int packetsize);		//!<Call after receiving each TFTP packet

#endif /*_MPCBASICFIRMWAREUPDATER_C_H_*/
