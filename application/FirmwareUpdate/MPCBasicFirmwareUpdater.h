/** \file MPCBasicFirmwareUpdater.h
*
* Written by: Jesper Larsen, jla@iotech.dk
* Company: IO Technologies A/S, Egaa, Denmark
*
* Project: Grundfos MPC Firmware Update via Ethernet, Basic
* Projectno.: 5075
* File: MPCBasicFirmwareUpdater.h
*
* $Log: MPCBasicFirmwareUpdater.h,v $
* Revision 1.3  2005/08/22 08:50:48  jla
* After changes at Grundfos
*
* Revision 1.1  2005/07/13 12:22:48  jla
* *** empty log message ***
*
*/

#ifndef _MPCBASICFIRMWAREUPDATER_H_
#define _MPCBASICFIRMWAREUPDATER_H_

extern "C"
{
#include "MPCBasicFirmwareUpdater_C.h"
}

#include "ipconfig/Thread.h"
#include "ipconfig/IPAddress.h"
#include <AppTypeDefs.h>
#include <string>

using namespace std;

//TCSL - Define for the new size changes to the RAM and ROM
#ifdef _RAM_32_ROM_16_UPGRADE_
  #define FLASH_PAGE_SIZE            0x20000      //!<Size of flash page in bytes
  #define FLASH_PAGE_CNT             0x001C       //!<Number of pages per image in flash
  #define FLASH_START                0xBF000000
  #define FLASH_ADDR_SECONDARY_IMAGE 0x000000     //!<Start address (in bytes) of flash image 1 (Secondary Image)
  #define FLASH_ADDR_PRIMARY_IMAGE   0x600000     //!<Start address (in bytes) of flash image 2 (Primary Image)
  #define FLASH_ADDR_BOOTLOADER      0xC00000     //!<Start address (in bytes) of bootloader
#else
  #define FLASH_PAGE_SIZE            0x10000      //!<Size of flash page in bytes
  #define FLASH_PAGE_CNT             0x0038       //!<Number of pages per image in flash
  #define FLASH_START                0xBF800000
  #define FLASH_ADDR_SECONDARY_IMAGE 0x080000     //!<Start address (in bytes) of flash image 1 (Secondary Image)
  #define FLASH_ADDR_PRIMARY_IMAGE   0x480000     //!<Start address (in bytes) of flash image 2 (Primary Image)
  #define FLASH_ADDR_BOOTLOADER      0x400000     //!<Start address (in bytes) of bootloader
#endif


#define META_DATA_OFFSET     0x37FFE4         //!<Offset of metadata related to start address (in bytes)

#define APP_CODE             0x87654321       //!<Code for valid bit
#define UNIQUE_CODE_PRI      0xAABBCCDD       //!<Unique code for primary image (image 2)
#define UNIQUE_CODE_SEC      0xDDCCBBAA       //!<Unique code for secondary image (image 1)

//Image size back to original size that is 3.5 MB
#define IMAGE_SIZE           0x0037FFE4       //!<Size of images (in bytes)

#define FW_UPDATE_TASK_PRIO       50          //!<Priority of firmware update task
#define FW_UPDATE_TASK_STACKSIZE  (4*1024)    //!<Size of stack for firmware update task

#define STD_PACKET_SIZE           512         //!<Standard TFTP packet size in bytes

/**Used when describing the status of the firmware update
*
*/
void firmwareUpdateStatusCallback();

/**\brief Class for handling update of firmware via ethernet
*
* Flash address space:				
* Bootloader address:        0xBFC00000	
* Primary Image valid bits	 0xBF97FF..
* Primary Image address: 	   0xBF600000	
* Secondary Image valid bits 0xBF37FF..
* Secondary Image address: 	 0xBF000000	
* Baseaddress:             	 0xBF000000	
*
*/
class MPCBasicFirmwareUpdater : public Thread
{
  friend int packetRecvCallback_C(const char *data, int packetsize);

public:
  virtual ~MPCBasicFirmwareUpdater();

  static MPCBasicFirmwareUpdater* getInstance();

  void doUpdate();						  		//!<Start updating
  void doUpdateBootloader();						//!<Start updating bootloader

  void setServerIPAddress(IPAddress);		  		//!<Set IP address for tftp server
  IPAddress getServerIPAddress();			  		//!<Get IP address for tftp server
  std::string getMainFirmwareFilename();
  void setMainFirmwareFilename(const char* szFilename);
  std::string getBootFirmwareFilename();
  void setBootFirmwareFilename(const char* szFilename);

  void setStatusCallback(void (*)(FIRMWARE_UPDATE_STATE_TYPE));

protected:
  MPCBasicFirmwareUpdater();
  virtual void run();						  		//!<The upgrade runs in its own thread

  FIRMWARE_UPDATE_STATE_TYPE  update();
  FIRMWARE_UPDATE_STATE_TYPE  updateFirmware(int socket);
  FIRMWARE_UPDATE_STATE_TYPE  updateBootloader(int socket);

private:
  bool packetRecvCallback(const char *data, int packetsize);	//!<Callback for TFTP client

  bool prepareFlash(unsigned int startaddr);		//!<Prepare flash memory space to be programmed
  bool programBlock(int blockno, short* data);	//!<Program a block in the flash memory space
  bool finalizeFlash(bool checksumok); 			//!<Finalize flash memory space after programming
  bool checkCheckSum();				  			//!<Checks the checksum of the programmed image
  bool findImageToUpdate();			  			//!<Find out which image should be updated

  IPAddress mserveripaddr;			   			//!<IP address of tftp server
  bool msuspended;					   			//!<True if update should be started
  bool mdoingupdate;					   			//!<Doing update true/false
  bool msuccess;						   			//!<Update success/failure flag
  bool mupdatingflash2;							//!<True: Updating flash image 2, false: Updating flash image 1
  bool mupdatingbootloader;						//!<True: Updating bootloader, false: Updating firmware
  char* mpprogbuffer;								//!<Pointer to buffer for flashcontent
  int mbytesinbuffer;								//!<Number of bytes in mpprogbuffer
  int mpagesprogrammed;							//!<Number of pages in flash programmed
  unsigned int mpStartAddr;             //!<Start address for programming
  std::string mMainFirmwareFilename;
  std::string mBootFirmwareFilename;

  void (*mpfstatus)(FIRMWARE_UPDATE_STATE_TYPE);	//!<Function pointer to status callback

  static MPCBasicFirmwareUpdater* mpinstance;

  unsigned int read32BitsFromFlash(unsigned int);	//!<Reads a 32 bit value from flash
};

#endif /*_MPCBASICFIRMWAREUPDATER_H_*/
