/** \file MPCBasicFirmwareUpdater.cpp
*
* Written by: Jesper Larsen, jla@iotech.dk
* Company: IO Technologies A/S, Egaa, Denmark
*
* Project: Grundfos MPC Firmware Upgrade via Ethernet, Basic
* Projectno.: 5075
* File: MPCBasicFirmwareUpdater.cpp
*
* $Log: MPCBasicFirmwareUpdater.cpp,v $
* Revision 1.3  2005/08/22 08:50:48  jla
* After changes at Grundfos
*
* Revision 1.1  2005/07/13 12:22:48  jla
* *** empty log message ***
*
*/


#include "MPCBasicFirmwareUpdater.h"

#include "ipconfig/RTIPConfigurationInterface.h"
#include "ipconfig/MPCNetworkDaemon.h"
#include "ipconfig/IPUtils.h"
#include <string>
#include <c1648.h>
#include "tftpapi.h"

#define RTIP_ERRNO       0      			/* allows errno values to be offset */
#define ETFTPERROR       (806+RTIP_ERRNO)  	/* TFTP Client - TTFTPERROR */ /*Copied from rtipapi.h because of compile error*/
/*(typedef word in both c1648.h and rtipapi.h)*/

#define DEFAULT_TFTP_SERVER_IP		192,168,0,1

#ifdef _RAM_32_ROM_16_UPGRADE_
#define	DEFAULT_FIRMWARE_FILENAME	 "MPC/cu362_firmware.bin"
#define DEFAULT_BOOTLOAD_FILENAME  "MPC/cu352_bootloader.bin"
#else
#define	DEFAULT_FIRMWARE_FILENAME	 "MPC/cu361_firmware.bin"
#define DEFAULT_BOOTLOAD_FILENAME  "MPC/cu351_bootloader.bin"
#endif


MPCBasicFirmwareUpdater* MPCBasicFirmwareUpdater::mpinstance = 0;

MPCBasicFirmwareUpdater::MPCBasicFirmwareUpdater()
:	msuspended(true),
mdoingupdate(false),
mupdatingbootloader(false),
mpprogbuffer(0),
mpfstatus(0),
mserveripaddr(DEFAULT_TFTP_SERVER_IP)
{
  mMainFirmwareFilename = DEFAULT_FIRMWARE_FILENAME;
  mBootFirmwareFilename = DEFAULT_BOOTLOAD_FILENAME;

  start(FW_UPDATE_TASK_PRIO, "Firmware update task", FW_UPDATE_TASK_STACKSIZE);
  suspend();
}

MPCBasicFirmwareUpdater::~MPCBasicFirmwareUpdater()
{
}

MPCBasicFirmwareUpdater* MPCBasicFirmwareUpdater::getInstance()
{
  if (mpinstance == 0)
  {
    mpinstance = new MPCBasicFirmwareUpdater;
  }

  return mpinstance;
}

/**Set callback for reporting status
*
* \param pfcb Pointer to callback function
*/
void MPCBasicFirmwareUpdater::setStatusCallback(void (*pfcb)(FIRMWARE_UPDATE_STATE_TYPE))
{
  mpfstatus = pfcb;
}

/**This method starts the upgrade procedure. It returns immediately after setting start flag.
*
* Update procedure:
*  - Decide which flash image to update
*  - Connect to TFTP server on default IP address.
*  - Start receiving file
*  - After receiving each packet add to buffer
*  - When buffer contains enough data to program flash page, do it
*  - After receiving and programming all packets, return
*
*/
void MPCBasicFirmwareUpdater::doUpdate()
{
  if (msuspended)
  {
    mupdatingbootloader = false;
    msuspended = false;
    resume();
  }
}

/**This method starts to bootloader upgrade procedure.
* It returns immediately after setting start flag.
*
*
*/
void MPCBasicFirmwareUpdater::doUpdateBootloader()
{
  if (msuspended)
  {
    mupdatingbootloader = true;
    msuspended = false;
    resume();
  }
}


/**
* \param sipaddr New IP address to use for contacting tftp server
*/
void MPCBasicFirmwareUpdater::setServerIPAddress(IPAddress sipaddr)
{
  mserveripaddr = sipaddr;
}

/**
* \return The current IP address used to contact tftp server.
*/
IPAddress MPCBasicFirmwareUpdater::getServerIPAddress()
{
  return mserveripaddr;
}

std::string MPCBasicFirmwareUpdater::getMainFirmwareFilename()
{
  return mMainFirmwareFilename;
}

void MPCBasicFirmwareUpdater::setMainFirmwareFilename(const char* szFilename)
{
  mMainFirmwareFilename = szFilename;
}

std::string MPCBasicFirmwareUpdater::getBootFirmwareFilename()
{
  return mBootFirmwareFilename;
}

void MPCBasicFirmwareUpdater::setBootFirmwareFilename(const char* szFilename)
{
  mBootFirmwareFilename = szFilename;
}
/**
* Task main rutine. The task is suspended until an update is required by Geni.
* When the task is resumed, updating the firmware or bootloader starts.
*
*/
void MPCBasicFirmwareUpdater::run()
{
  FIRMWARE_UPDATE_STATE_TYPE status = FIRMWARE_UPDATE_STATE_IDLE;
  while(1)
  {
    if(!mpfstatus)
    {
      msuspended = true;
      suspend(); // In suspended state until doUpdate is called then
      continue;  // continue to make the test of the mpfstatus function
      // pointer after resume.
    }

    // Thread resumed. Lets do some updating.
    mdoingupdate = true;

    status = update();

    // Set the callback status
    mpfstatus(status);

    // Clean up
    if(mpprogbuffer)
    {
      delete[] mpprogbuffer;
      mpprogbuffer = 0;
    }

    // Update done, suspend until next update.
    mdoingupdate = false;
    msuspended = true;

    // In suspended state until doUpdate is called
    suspend();
  }
}

/**
* This method updates the firmware in flash mem. There are 2 kinds of update,
* firmware or bootloader. The function tests the ethernet link status,
* pings the TFTP server, connects to the TFTP server and calls updateFirmware
* or updateBootloader, depending on the update requested.
*
*/
FIRMWARE_UPDATE_STATE_TYPE MPCBasicFirmwareUpdater::update()
{
  int socket = 0;
  mbytesinbuffer = 0;
  mpagesprogrammed = 0;
  msuccess = false;
  IPAddress ipaddr = mserveripaddr;

  // Make sure ethernet is up available before programming flash
  bool linkstatus = RTIPConfigurationInterface::getInstance()->getLinkStatus();
  if (!linkstatus)
  {
    return FIRMWARE_UPDATE_STATE_FAILURENETWORKDOWN;
  }

  // Make sure host is running IP.
  if(!(IPUtils::ping(ipaddr)))
  {
    return FIRMWARE_UPDATE_STATE_FAILUREPING;
  }

  // Connect to TFTP server
  socket = tftpcli_connect(ipaddr.getIPAddress());
  if (socket <= 0)
  {
    return FIRMWARE_UPDATE_STATE_FAILURENOTFTPSERVER;
  }

  if (!mupdatingbootloader)
  {
    return updateFirmware(socket);
  }
  else
  {
    return updateBootloader(socket);
  }
}


/**
* This method updates the firmware in flash mem. There are two images in flash.
*
* - determines wich image is about to be updated
* - erases the flash image that is about to be updated
* - receives the image file while writting it to flash
*
*/
FIRMWARE_UPDATE_STATE_TYPE MPCBasicFirmwareUpdater::updateFirmware(int socket)
{
  // Updating primary or secondary image.
  bool linkstatus = true; // This function should never be called without a link.

  // Allocate temp. programming buffer.
  if (!mpprogbuffer)
  {
    mpprogbuffer = new char[FLASH_PAGE_SIZE];
    if (!mpprogbuffer)
    {
      // Oooops no mem available for prog buffer.
      return FIRMWARE_UPDATE_STATE_FAILUREINTERN;
    }
  }

  // Find address of image to update
  mupdatingflash2 = findImageToUpdate();
  unsigned int addr = mupdatingflash2 ? FLASH_ADDR_PRIMARY_IMAGE : FLASH_ADDR_SECONDARY_IMAGE;

  // Erease image (prepare for programming).
  mpfstatus(mupdatingflash2 ? FIRMWARE_UPDATE_STATE_ERASINGPRI : FIRMWARE_UPDATE_STATE_ERASINGSEC);
  prepareFlash(addr); // XHM: Shouldn't we stop if prepareFlash fails ????

  // Start receiving file
  mpfstatus(mupdatingflash2 ? FIRMWARE_UPDATE_STATE_PROGRAMMINGPRI : FIRMWARE_UPDATE_STATE_PROGRAMMINGSEC);
  if (tftpcli_recvfile(socket, (char*)mMainFirmwareFilename.c_str(), "octet"))
  {
    // Error receiving file.
    int errorcode = IPUtils::getLastError();
    linkstatus = RTIPConfigurationInterface::getInstance()->getLinkStatus();
    if (!linkstatus)
    {
      return FIRMWARE_UPDATE_STATE_FAILURENETWORKDOWN;
    }
    else if ( errorcode == ETFTPERROR)
    {
      return FIRMWARE_UPDATE_STATE_FAILURETFTPERROR;
    }
    else if ( errorcode == ETIMEDOUT)
    {
      return FIRMWARE_UPDATE_STATE_FAILURENOTFTPSERVER;
    }
    else
    {
      return FIRMWARE_UPDATE_STATE_FAILURETFTPERROR;
    }
  }

  // msuccess is set in callback rutine when whole image has been programmed
  if (!msuccess)
  {
    return FIRMWARE_UPDATE_STATE_FAILURETFTPERROR;
  }

  // Test checksum of received image.
  if( checkCheckSum() )
  {
    // Set image valid
    if (!finalizeFlash(true))
    {
      return FIRMWARE_UPDATE_STATE_FAILUREFLASHWRITE;
    }
    else
    {
      return FIRMWARE_UPDATE_STATE_SUCCESS;
    }
  }
  else
  {
    // Set image invalid
    if ( !finalizeFlash(false))
    {
      return FIRMWARE_UPDATE_STATE_FAILUREFLASHWRITE;
    }
    else
    {
      return FIRMWARE_UPDATE_STATE_FAILUREINTERN;
    }
  }

  //return FWUFAILUREINTERN;   // we cannot get here
}


/**
* Updates the bootloader, there is no room for error here. The bootloader
* update must success, otherwise the MPC will not reboot, nor can it be
* saved by a hexload update (well maybe it can, depending on the amount of
* flash programmed before the error).
*/
FIRMWARE_UPDATE_STATE_TYPE MPCBasicFirmwareUpdater::updateBootloader(int socket)
{
  // Update bootloader
  bool linkstatus = true; // This function should never be called without a link.

  if(!mpprogbuffer)
  {
    mpprogbuffer = new char [FLASH_PAGE_SIZE];
    if (!mpprogbuffer)
    {
      return FIRMWARE_UPDATE_STATE_FAILUREINTERN;
    }
  }

  // Start receiving bootloader
  mpfstatus(FIRMWARE_UPDATE_STATE_RECEIVINGBL);
  if (tftpcli_recvfile(socket, (char*)mBootFirmwareFilename.c_str(), "octet"))
  {
    // Error receiving bootloader.
    int errorcode = IPUtils::getLastError();
    linkstatus = RTIPConfigurationInterface::getInstance()->getLinkStatus();

    if (!linkstatus)
    {
      return FIRMWARE_UPDATE_STATE_FAILURENETWORKDOWN;
    }
    else if ( errorcode == ETFTPERROR)
    {
      return FIRMWARE_UPDATE_STATE_FAILURETFTPERROR;
    }
    else if ( errorcode == ETIMEDOUT)
    {
      return FIRMWARE_UPDATE_STATE_FAILURENOTFTPSERVER;
    }
    else
    {
      return FIRMWARE_UPDATE_STATE_FAILURETFTPERROR;
    }
  }

  // Fill unused progbuffer with zeros.
  char *pprogdata = mpprogbuffer;
  while (mbytesinbuffer < FLASH_PAGE_SIZE)
  {
    mpprogbuffer[mbytesinbuffer++] = 0;
  }

  // Erease old bootloader.
  mpStartAddr = FLASH_ADDR_BOOTLOADER;
  mpfstatus(FIRMWARE_UPDATE_STATE_ERASINGBL);

  if (FlashBlockErase(mpStartAddr/FLASH_PAGE_SIZE) != Flash_Success)
  {
    // Very fatal error. Some of the bootloader may have been deleted!!!
    return FIRMWARE_UPDATE_STATE_FAILUREINTERN;
  }

  // Program flash with new bootloader.
  mpfstatus(FIRMWARE_UPDATE_STATE_PROGRAMMINGBL);

  if (!programBlock(0, reinterpret_cast<short*>(pprogdata)))
  {
    // Very fatal error. The new bootloader was not written to flash!!!
    return FIRMWARE_UPDATE_STATE_FAILUREFLASHWRITE;
  }
  mupdatingbootloader = false;
  return FIRMWARE_UPDATE_STATE_SUCCESS;
}

/**This method must be called after receiving each packet of the bin file
* to be programmed into the flash.
*
* Procedure:\n
*  - Add packet to programming buffer
*  - If buffer full, program to the flash
*    - If programming succeeds, return true. Return false if failure
*  - If buffer not full, return true
*
* \param data Pointer to data
* \param packetsize Number of bytes pointed to by data.
*        If packetsize is less than STDPACKETSIZE, programming is ended.
*
* \return True if OK to proceed with file transfer, false if not
*/
bool MPCBasicFirmwareUpdater::packetRecvCallback(const char *data, int packetsize)
{
  if (!mpprogbuffer || !mdoingupdate)
    return false;

  if (!mupdatingbootloader)
  {
    if (mbytesinbuffer+packetsize <= FLASH_PAGE_SIZE)
    {
      memcpy(&mpprogbuffer[mbytesinbuffer], data, packetsize);
      mbytesinbuffer += packetsize;
    }
    else
    {
      mdoingupdate = false;
      return false;
    }

    if (packetsize != STD_PACKET_SIZE || mbytesinbuffer == FLASH_PAGE_SIZE)
    {
      //If last packet, fill block buffer with empty spaces
      while (mbytesinbuffer < FLASH_PAGE_SIZE)
      {
        mpprogbuffer[mbytesinbuffer++] = 0xFF;
      }

      //About to program last page containing valid bit?
      if (mpagesprogrammed == FLASH_PAGE_CNT-1)
      {
        // Then make sure valib bit and unique codes are set correctly:
        unsigned int *pbuf;
        if (mupdatingflash2)
        {
          pbuf = reinterpret_cast<unsigned int*>(&mpprogbuffer[META_DATA_OFFSET+0 - (FLASH_PAGE_CNT-1)*FLASH_PAGE_SIZE]);
          *pbuf = UNIQUE_CODE_PRI;

          pbuf = reinterpret_cast<unsigned int*>(&mpprogbuffer[META_DATA_OFFSET+12 - (FLASH_PAGE_CNT-1)*FLASH_PAGE_SIZE]);
          *pbuf = APP_CODE;
        }
        else
        {
          pbuf = reinterpret_cast<unsigned int*>(&mpprogbuffer[META_DATA_OFFSET+0 - (FLASH_PAGE_CNT-1)*FLASH_PAGE_SIZE]);
          *pbuf = UNIQUE_CODE_SEC;
        }
      }

      if (programBlock(mpagesprogrammed, reinterpret_cast<short*>(mpprogbuffer)))
      {
        mpagesprogrammed++;
        mbytesinbuffer = 0;

        if (packetsize != STD_PACKET_SIZE || mpagesprogrammed == FLASH_PAGE_CNT)
          msuccess = true;
      }
      else
      {
        mdoingupdate = false;
        return false;
      }
    }
  }
  else                //!mupdatingbootloader
  {
    if (mbytesinbuffer+packetsize <= FLASH_PAGE_SIZE)
    {
      memcpy(&mpprogbuffer[mbytesinbuffer], data, packetsize);
      mbytesinbuffer += packetsize;
    }
    else
    {
      mdoingupdate = false;
      return false;
    }
  }
  return true;
}

/**Erase of all affected blocks.
* The image is erased from top to bottom, to make sure valid bits
* are erased first.
*
* \param startaddr Start address of blocks to be programmed later.
* \return True: success, false: failure
*/
bool MPCBasicFirmwareUpdater::prepareFlash(unsigned int startaddr)
{
  int i;

  mpStartAddr = startaddr;

  for (i=FLASH_PAGE_CNT-1 ; i>=0 ; i--)
  {
    if (FlashBlockErase(mpStartAddr/(FLASH_PAGE_SIZE)+i) != Flash_Success)
      return false;
    /*Immediately after erasing block containing valid bit,
    * set it again:
    */
    if (i == FLASH_PAGE_CNT-1)
    {
      unsigned int appcode = APP_CODE;
      if (FlashProgram((FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+12)/2, 2, &appcode) != Flash_Success)
        return false;
    }

    //delay(5);			//Maybe delay to relinquish processor
  }

  return true;
}

/**Programs a block of flash data:
*
* \param parname blockno Number of block to be programmed
* \param data Pointer to data to be programmed
* \return True: success, false: failure
*/
bool MPCBasicFirmwareUpdater::programBlock(int blockno, short* data)
{
  if (blockno < FLASH_PAGE_CNT)
  {
    //Address parameter is in words (16-bit)!:
    if (FlashProgram((blockno*FLASH_PAGE_SIZE+mpStartAddr)/2, FLASH_PAGE_SIZE/2, data) == Flash_Success)
      return true;
  }

  return false;
}

/**Code for this method was copied from BootLoad.c
*
* \return True if Checksum is OK, false if not
*/
bool MPCBasicFirmwareUpdater::checkCheckSum()
{
  unsigned int calculated_sum, image_sum, size;

  unsigned int *source = reinterpret_cast<unsigned int*>(FLASH_START + mpStartAddr);

  calculated_sum = 0;

  image_sum = *(reinterpret_cast<unsigned int*>(FLASH_START + mpStartAddr + META_DATA_OFFSET + 4));

  for (size = 0; size < (IMAGE_SIZE/4); size++)
  {
    calculated_sum = calculated_sum + *source++;
  }

  bool retval = (calculated_sum == image_sum );

  return retval;
}

/**Erase checksum for image to be updated next time.
*
* \note The valid bit for the other image is deleted.
* \return True: success, false: failure
*/
bool MPCBasicFirmwareUpdater::finalizeFlash(bool checksumok)
{
  unsigned int zero = 0;
  ReturnType ret_val_1, ret_val_2;

  if (checksumok)									//Delete checksum for other image if checksum for updated image is ok:
  {
    if (mpStartAddr == FLASH_ADDR_SECONDARY_IMAGE)
    {
      ret_val_1 = FlashProgram((FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+4)/2, 2, &zero); //Erase checksum
      ret_val_2 = FlashProgram((FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+0)/2, 2, &zero); //Erase unique code
      if (ret_val_1  != Flash_Success || ret_val_2 != Flash_Success)
      {
        return false;
      }
    }
    else if (mpStartAddr == FLASH_ADDR_PRIMARY_IMAGE)
    {
      ret_val_1 = FlashProgram((FLASH_ADDR_SECONDARY_IMAGE+META_DATA_OFFSET+4)/2, 2, &zero); //Erase checksum
      ret_val_2 = FlashProgram((FLASH_ADDR_SECONDARY_IMAGE+META_DATA_OFFSET+0)/2, 2, &zero); //Erase unique code
      if (ret_val_1  != Flash_Success || ret_val_2 != Flash_Success)
      {
        return false;
      }
    }
  }
  else											//Delete checksum for current image:
  {
    if (mpStartAddr == FLASH_ADDR_PRIMARY_IMAGE)
    {
      ret_val_1 = FlashProgram((FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+4)/2, 2, &zero); //Erase checksum
      ret_val_2 = FlashProgram((FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+0)/2, 2, &zero); //Erase unique code
      if (ret_val_1  != Flash_Success || ret_val_2 != Flash_Success)
      {
        return false;
      }
    }
    else if (mpStartAddr == FLASH_ADDR_SECONDARY_IMAGE)
    {
      ret_val_1 = FlashProgram((FLASH_ADDR_SECONDARY_IMAGE+META_DATA_OFFSET+4)/2, 2, &zero); //Erase checksum
      ret_val_2 = FlashProgram((FLASH_ADDR_SECONDARY_IMAGE+META_DATA_OFFSET+0)/2, 2, &zero); //Erase unique code
      if (ret_val_1  != Flash_Success || ret_val_2 != Flash_Success)
      {
        return false;
      }
    }
  }

  return true;
}

/**Checks flash image metadata structures, and decides which image to update.
* This is done by checking version numbers and valid bits.
* The valid bits are checked first. If an image has an invalid valid bit, that
* image is updated. If both images have invalid valid bits, image 2 (primary) is updated.
*
* If both images have a valid version number,
* the oldest image is updated. If the two images have the same "age", image 2 (primary) is updated.
* If one of the images has an invalid version number (f.x. 0xFFFFFFFF), this image is
* updated. If both images have an invalid version number, image 2 (primary image) is updated.
*
* \return True if image 2 should be updated, and false if image 1 should be updated.
*/
bool MPCBasicFirmwareUpdater::findImageToUpdate()
{
  //unsigned int appcode = APPCODE;
  bool updateimage2 = true;

  {
    unsigned int image1count, image2count, checksum1, checksum2;
    image1count = read32BitsFromFlash(FLASH_ADDR_SECONDARY_IMAGE+META_DATA_OFFSET+8);
    image2count = read32BitsFromFlash(FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+8);
    checksum1 = read32BitsFromFlash(FLASH_ADDR_SECONDARY_IMAGE+META_DATA_OFFSET+4);
    checksum2 = read32BitsFromFlash(FLASH_ADDR_PRIMARY_IMAGE+META_DATA_OFFSET+4);

    if (checksum2 == 0x0 || checksum2 == 0xffffffff)
    {
      updateimage2 = true;
    }
    else if (checksum1 == 0x0 || checksum1 == 0xffffffff)
    {
      updateimage2 = false;
    }
    else if ( image2count == 0xffffffff)
    {
      updateimage2 = true;
    }
    else if ( image1count == 0xffffffff)
    {
      updateimage2 = false;
    }
    else if (image2count <= image1count)
    {
      updateimage2 = true;
    }
    else
    {
      updateimage2 = false;
    }
  }

  return updateimage2;
}

/**
*
* \param addr Address to read, byte-based address
* \return The value read from flash
*/
unsigned int MPCBasicFirmwareUpdater::read32BitsFromFlash(unsigned int addr)
{
  unsigned int value=(FlashRead(addr/2));
  value += (FlashRead(addr/2+1))<<16;

  return value;
}



extern "C"
{

  int packetRecvCallback_C(const char *data, int packetsize)
  {
    if (MPCBasicFirmwareUpdater::getInstance()->packetRecvCallback(data, packetsize))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

} //Extern "C"

