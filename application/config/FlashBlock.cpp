/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CR Monitor                                       */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos                                     */
/*               All rights reserved                                        */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/****************************************************************************/
/* CLASS NAME       : FlashBlock                                            */
/*                                                                          */
/* FILE NAME        : FlashBlock.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 21-09-2007                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See header file                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef __PC__
#include <iostream>
#include <fstream>
#include "BaseDirectory.h"
#endif

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <MpcTime.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <FlashBlock.h>

/*****************************************************************************
  TYPE DEFINITIONS
 *****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define ERROR_FLAG_ERASE_FLASH_BLOCK                              (1 << 0)
#define ERROR_FLAG_SAVE_TO_FLASH_FAILED                           (1 << 1)
#define ERROR_FLAG_LOAD_FROM_FLASH_FAILED                         (1 << 2)
#define ERROR_FLAG_SUBJECT_DID_NOT_SAVE_EXPECTED_NUMBER_OF_BYTES  (1 << 3)
#define ERROR_FLAG_SUBJECT_ID_OR_FLASH_ID_HAS_CHANGED             (1 << 4)
#define ERROR_FLAG_FLASH_BLOCK_TOO_SMALL                          (1 << 5)
#define ERROR_FLAG_SUBJECT_NOT_FOUND                              (1 << 6)

/*****************************************************************************
  CONSTANTS
 *****************************************************************************/
const int SUBJECT_FLASH_HEADER_SIZE = sizeof(SUBJECT_FLASH_HEADER_TYPE);

/*****************************************************************************
 * FUNCTION - Constructor
 * DESCRIPTION:
 ****************************************************************************/
FlashBlock::FlashBlock(ConfigControl* pConfigControl, const char* szFlashID, U32 maxBlockSize, const NON_VOLATILE_SUBJECT_TYPE* pSubjects, const U16 subjectCount, FLASH_CONTROL_BLOCK_ID_TYPE flashControlBlockId) :
  mpConfigControl(pConfigControl),
	mFlashID(szFlashID),
	mMaxBlockSize(maxBlockSize),
	mpSubjects(pSubjects),
	mSubjectCount(subjectCount),
  mFlashControlBlockId(flashControlBlockId)
{
	// create block memory cache
	mpBlockCache = new U8[maxBlockSize];
	memset(mpBlockCache, 0xCC, maxBlockSize);

	// set block header and data pointer
	mpBlockHdr = (FLASH_BLOCK_HEADER_TYPE*)mpBlockCache;
	mpBlockData = (mpBlockCache + FLASH_BLOCK_HEADER_SIZE);

	// set block version
	mpBlockHdr->version = 1;

	// set block flash ID
	strncpy(mpBlockHdr->szFlashID, mFlashID.c_str(), sizeof(mpBlockHdr->szFlashID) - 1);
	mpBlockHdr->szFlashID[sizeof(mpBlockHdr->szFlashID) - 1] = '\0';

	// reset block size and subject count
	mpBlockHdr->blockSize = FLASH_BLOCK_HEADER_SIZE;
	mpBlockHdr->subjectCount = 0;

	// clear reserved area
	memset(&mpBlockHdr->reserved, 0x00, sizeof(mpBlockHdr->reserved));

	// misc. init
	mWritePos = 0;
  mModified = true;

  mErrorFlags = 0;
}

/*****************************************************************************
 * FUNCTION - Destructor
 * DESCRIPTION:
 ****************************************************************************/
FlashBlock::~FlashBlock()
{
	delete [] mpBlockCache;
}

/*****************************************************************************
 * FUNCTION - WriteBool
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteBool(const bool value)
{
  if (CanWrite(1))
  {
    if (value)
      mModified |= (*((U8*)(mpBlockData + mWritePos)) == 0);
    else
      mModified |= (*((U8*)(mpBlockData + mWritePos)) == 1);
  	*((U8*)(mpBlockData + mWritePos)) = (U8)(value ? 1 : 0);
  	mWritePos += 1;
    mWriteAvailable -= 1;
  }
}

/*****************************************************************************
 * FUNCTION - ReadBool
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
bool FlashBlock::ReadBool(const bool defaultValue)
{
	bool value = defaultValue;
  if (CanRead(1))
	{
		value = (*((U8*)(mpBlockData + mReadPos)) != 0) ? true : false;
		mReadPos += 1;
		mReadAvailable -= 1;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteU8
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteU8(const U8 value)
{
  if (CanWrite(1))
  {
    mModified |= (*((U8*)(mpBlockData + mWritePos)) != value);
  	*((U8*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 1;
    mWriteAvailable -= 1;
  }
}

/*****************************************************************************
 * FUNCTION - ReadU8
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
U8 FlashBlock::ReadU8(const U8 defaultValue)
{
	U8 value = defaultValue;
  if (CanRead(1))
	{
		value = *((U8*)(mpBlockData + mReadPos));
		mReadPos += 1;
		mReadAvailable -= 1;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteU16
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteU16(const U16 value)
{
  if (CanWrite(2))
  {
    mModified |= (*((U16*)(mpBlockData + mWritePos)) != value);
  	*((U16*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 2;
    mWriteAvailable -= 2;
  }
}

/*****************************************************************************
 * FUNCTION - ReadU16
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
U16 FlashBlock::ReadU16(const U16 defaultValue)
{
	U16 value = defaultValue;
  if (CanRead(2))
	{
		value = *((U16*)(mpBlockData + mReadPos));
		mReadPos += 2;
		mReadAvailable -= 2;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteI16
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteI16(const I16 value)
{
  if (CanWrite(2))
  {
    mModified |= (*((I16*)(mpBlockData + mWritePos)) != value);
  	*((I16*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 2;
    mWriteAvailable -= 2;
  }
}

/*****************************************************************************
 * FUNCTION - ReadI16
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
I16 FlashBlock::ReadI16(const I16 defaultValue)
{
	I16 value = defaultValue;
  if (CanRead(2))
	{
		value = *((I16*)(mpBlockData + mReadPos));
		mReadPos += 2;
		mReadAvailable -= 2;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteU32
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteU32(const U32 value)
{
  if (CanWrite(4))
  {
    mModified |= (*((U32*)(mpBlockData + mWritePos)) != value);
  	*((U32*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 4;
    mWriteAvailable -= 4;
  }
}

/*****************************************************************************
 * FUNCTION - ReadU32
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
U32 FlashBlock::ReadU32(const U32 defaultValue)
{
	U32 value = defaultValue;
  if (CanRead(4))
	{
		value = *((U32*)(mpBlockData + mReadPos));
		mReadPos += 4;
		mReadAvailable -= 4;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteI32
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteI32(const I32 value)
{
  if (CanWrite(4))
  {
    mModified |= (*((I32*)(mpBlockData + mWritePos)) != value);
  	*((I32*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 4;
    mWriteAvailable -= 4;
  }
}

/*****************************************************************************
 * FUNCTION - ReadI32
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
I32 FlashBlock::ReadI32(const I32 defaultValue)
{
	I32 value = defaultValue;
  if (CanRead(4))
	{
		value = *((I32*)(mpBlockData + mReadPos));
		mReadPos += 4;
		mReadAvailable -= 4;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteFloat
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteFloat(const float value)
{
  if (CanWrite(4))
  {
    mModified |= (*((float*)(mpBlockData + mWritePos)) != value);
  	*((float*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 4;
    mWriteAvailable -= 4;
  }
}

/*****************************************************************************
 * FUNCTION - ReadFloat
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
float FlashBlock::ReadFloat(const float defaultValue)
{
	float value = defaultValue;
	if (CanRead(4))
	{
		value = *((float*)(mpBlockData + mReadPos));
		mReadPos += 4;
		mReadAvailable -= 4;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteDouble
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteDouble(const double value)
{
  if (CanWrite(8))
  {
    mModified |= (*((double*)(mpBlockData + mWritePos)) != value);
  	*((double*)(mpBlockData + mWritePos)) = value;
  	mWritePos += 8;
    mWriteAvailable -= 8;
  }
}

/*****************************************************************************
 * FUNCTION - ReadDouble
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
double FlashBlock::ReadDouble(const double defaultValue)
{
	float value = defaultValue;
	if (CanRead(8))
	{
		value = *((double*)(mpBlockData + mReadPos));
		mReadPos += 8;
		mReadAvailable -= 8;
	}
	return value;
}

/*****************************************************************************
 * FUNCTION - WriteString
 * DESCRIPTION: IFlashWriter implementation
 * NOTICE: szValue buffer must be large enough to hold maxLen + 1 chars!
 ****************************************************************************/
void FlashBlock::WriteString(const char* szValue, int maxLen)
{
	const int len = strlen(szValue);
  int flashLen;

  if (len <= maxLen)
  {
    if (CanWrite(8 + maxLen))
    {
      // get length of string stored in flash
      flashLen = *(int*)(mpBlockData + mWritePos);

      // write length and max length
    	WriteI32(len);
    	WriteI32(maxLen);

      // determine whether the string has been modified (or initial write)
      if ((len != flashLen) || (strncmp((char*)(mpBlockData + mWritePos), szValue, len) != 0))
      {
        mModified = true;
      }

      // write if modified
      if (mModified)
      {
        // write string value
      	memcpy(mpBlockData + mWritePos, szValue, len);
      	mWritePos += len;

        // pad with '\0' if necessary
      	if (len != maxLen)
      	{
      		memset(mpBlockData + mWritePos, 0, maxLen - len);
      		mWritePos += (maxLen - len);
      	}
      }
      else
      {
        mWritePos += maxLen;
      }

      // adjust write available
      mWriteAvailable -= maxLen;
    }
  }
}

/*****************************************************************************
 * FUNCTION - ReadString
 * DESCRIPTION: IFlashReader implementation
 * NOTICE: szValue buffer must be large enough to hold maxLen + 1 chars!
 ****************************************************************************/
void FlashBlock::ReadString(char* szValue, int maxLen, const char* szDefaultValue)
{
  int flashLen, flashMaxLen;

  if (CanRead(8))
  {
    flashLen = ReadI32(0);    // string length stored in flash
    flashMaxLen = ReadI32(0); // max string length in flash

    if (CanRead(flashMaxLen))
    {
      if (flashLen <= maxLen)
      {
        memcpy(szValue, mpBlockData + mReadPos, flashLen);
        szValue[flashLen] = '\0';
      }
      else  // string stored in flash to long, use default
      {
        strncpy(szValue, szDefaultValue, maxLen);
        szValue[maxLen] = '\0';
      }

      mReadPos += flashMaxLen;
      mReadAvailable -= flashMaxLen;
    }
    else  // not enough data
    {
      strncpy(szValue, szDefaultValue, maxLen);
      szValue[maxLen] = '\0';
    }
  }
  else // not enough data
  {
    strncpy(szValue, szDefaultValue, maxLen);
    szValue[maxLen] = '\0';
  }
}

/*****************************************************************************
 * FUNCTION - WriteMpcTime
 * DESCRIPTION: IFlashWriter implementation
 ****************************************************************************/
void FlashBlock::WriteMpcTime(MpcTime* pMpcTime)
{
  if (CanWrite(9))
  {
    // write data part
    if (pMpcTime->IsDateValid())
    {
      WriteBool(true);
      WriteU8((U8)pMpcTime->GetDate(YEAR)-2000);
      WriteU8((U8)pMpcTime->GetDate(MONTH));
      WriteU8((U8)pMpcTime->GetDate(DAY));
      WriteU8((U8)pMpcTime->GetDate(DAY_OF_WEEK));
    }
    else
    {
      WriteBool(false);
      WriteU8((U8)0);
      WriteU8((U8)0);
      WriteU8((U8)0);
      WriteU8((U8)0);
    }

    // write time part
    if (pMpcTime->IsTimeValid())
    {
      WriteBool(true);
      WriteU8((U8)pMpcTime->GetTime(HOURS));
      WriteU8((U8)pMpcTime->GetTime(MINUTES));
      WriteU8((U8)pMpcTime->GetTime(SECONDS));
    }
    else
    {
      WriteBool(false);
      WriteU8((U8)0);
      WriteU8((U8)0);
      WriteU8((U8)0);
    }
  }
}

/*****************************************************************************
 * FUNCTION - ReadMpcTime
 * DESCRIPTION: IFlashReader implementation
 ****************************************************************************/
void FlashBlock::ReadMpcTime(MpcTime* pMpcTime)
{
  if (CanRead(9))
  {
    // read data part
    if (ReadBool(false))
    {
      pMpcTime->SetDate(YEAR, ReadU8((U8)pMpcTime->GetDate(YEAR))+2000);
      pMpcTime->SetDate(MONTH, ReadU8((U8)pMpcTime->GetDate(MONTH)));
      pMpcTime->SetDate(DAY, ReadU8((U8)pMpcTime->GetDate(DAY)));
      pMpcTime->SetDate(DAY_OF_WEEK, ReadU8((U8)pMpcTime->GetDate(DAY_OF_WEEK)));
    }
    else
    {
      pMpcTime->SetDateInValid();
  		mReadPos += 4;
  		mReadAvailable -= 4;
    }

    // read time part
    if (ReadBool(false))
    {
      pMpcTime->SetTime(HOURS, ReadU8((U8)pMpcTime->GetTime(HOURS)));
      pMpcTime->SetTime(MINUTES, ReadU8((U8)pMpcTime->GetTime(MINUTES)));
      pMpcTime->SetTime(SECONDS, ReadU8((U8)pMpcTime->GetTime(SECONDS)));
    }
    else
    {
      pMpcTime->SetTimeInValid();
  		mReadPos += 3;
  		mReadAvailable -= 3;
    }
  }
}

/*****************************************************************************
 * FUNCTION - SaveToCache
 * DESCRIPTION: Saves a single subject to cache.
 * Returns true if subject was contained in this block
 ****************************************************************************/
bool FlashBlock::SaveToCache(Subject* pSubject)
{
  bool found = false;

  if (pSubject)
  {
    SUBJECT_FLASH_INFO_MAP_ITR itr = mSubjectInfo.find(pSubject->GetSubjectId());

    if (itr != mSubjectInfo.end())
    {
      // subject found in this block
      found = true;

      // get save type and write pos
      FLASH_SAVE_TYPE saveType = itr->second.saveType;
      mWritePos = itr->second.writePos;

      // store write pos and get subject header pointer
      const U32 startWritePos = mWritePos;
      SUBJECT_FLASH_HEADER_TYPE* pSubjectHdr = (SUBJECT_FLASH_HEADER_TYPE*)(mpBlockData + mWritePos);

      // verify subject id and flash id
      if ((pSubjectHdr->subjectId == pSubject->GetSubjectId()) &&
        (pSubjectHdr->flashId == pSubject->GetFlashId()))
      {
        // advance to data
        mWritePos += SUBJECT_FLASH_HEADER_SIZE;

        // reset write error and set write available
        mWriteError = false;
        mWriteAvailable = pSubjectHdr->dataLength;

        // save subject
        pSubject->SaveToFlash(this, saveType);

        // verify number of bytes written by the subject
        if (!mWriteError && (pSubjectHdr->dataLength == (mWritePos - startWritePos - SUBJECT_FLASH_HEADER_SIZE)))
        {
          // success
        }
        else
        {
          // fatal error: subject did not save expected number of bytes
          FatalErrorOccured("Wrong number of bytes saved in FLASH!");
          mErrorFlags |= ERROR_FLAG_SUBJECT_DID_NOT_SAVE_EXPECTED_NUMBER_OF_BYTES;
        }
      }
      else
      {
        // fatal error: subject id / flash id has changed...
        FatalErrorOccured("Subject <-> flash id conflict!");
        mErrorFlags |= ERROR_FLAG_SUBJECT_ID_OR_FLASH_ID_HAS_CHANGED;
      }
    }
    else
    {
      // subject not in this block
    }
  }

  return found;
}

/*****************************************************************************
 * FUNCTION - SaveAllSubjectsToCache
 * DESCRIPTION: Saves all subjects to the cache which also re-organizes
 * the flash block if necessary
 ****************************************************************************/
void FlashBlock::SaveAllSubjectsToCache(void)
{
  Subject* pSubject;
  U32 startWritePos;
  SUBJECT_FLASH_HEADER_TYPE* pSubjectHdr;

  // reset
  mpBlockHdr->blockSize = FLASH_BLOCK_HEADER_SIZE;
  mpBlockHdr->subjectCount = 0;
  mWritePos = 0;
  memset(mpBlockData, 0xCC, mMaxBlockSize - FLASH_BLOCK_HEADER_SIZE);
  mErrorFlags = 0;

  // save all subjects
  for (int i = 0; i < mSubjectCount; i++)
  {
    pSubject = mpConfigControl->GetSubject(mpSubjects[i].subjectId);

    if (pSubject)
    {
      // set subject header pointer and store write pos
      pSubjectHdr = (SUBJECT_FLASH_HEADER_TYPE*)(mpBlockData + mWritePos);
      startWritePos = mWritePos;

      // store write pos and save type, first time only
      SUBJECT_FLASH_INFO_TYPE sfi;
      sfi.writePos = mWritePos;
      sfi.saveType = mpSubjects[i].saveType;
      mSubjectInfo[pSubject->GetSubjectId()] = sfi;

      // set subject id, flash id and reset data length
      pSubjectHdr->subjectId = pSubject->GetSubjectId();
      pSubjectHdr->flashId = pSubject->GetFlashId();
      pSubjectHdr->saveType = mpSubjects[i].saveType;
      pSubjectHdr->dataLength = 0;

      // advance to data
      mWritePos += SUBJECT_FLASH_HEADER_SIZE;

      // reset write error and calculate write available
      mWriteError = false;
      mWriteAvailable = mMaxBlockSize - FLASH_BLOCK_HEADER_SIZE - mWritePos;

      // save subject
      pSubject->SaveToFlash(this, mpSubjects[i].saveType);

      // check for write error
      if (mWriteError)
      {
        FatalErrorOccured("FLASH block too small!");
        mErrorFlags |= ERROR_FLAG_FLASH_BLOCK_TOO_SMALL;
      }
      else
      {
        // set subject header data length
        pSubjectHdr->dataLength = mWritePos - startWritePos - SUBJECT_FLASH_HEADER_SIZE;

        // increment subject count
        mpBlockHdr->subjectCount++;
      }
    }
    else
    {
      FatalErrorOccured("FLASH - no subject!");
      mErrorFlags |= ERROR_FLAG_SUBJECT_NOT_FOUND;
    }
	}

	// set block size
	mpBlockHdr->blockSize = FLASH_BLOCK_HEADER_SIZE + mWritePos;
}

/*****************************************************************************
 * FUNCTION - LoadAllSubjectsFromCache
 * DESCRIPTION:
 * Loads all subjects from cache (calls LoadFromFlash on each subject)
 ****************************************************************************/
bool FlashBlock::LoadAllSubjectsFromCache(void)
{
  bool success = true;
  Subject* pSubject;
  SUBJECT_FLASH_HEADER_TYPE* pSubjectHdr;
  U32 startReadPos;

  // reset read position
  mReadPos = 0;

  // read all subjects
  for (int i = 0; i < mpBlockHdr->subjectCount; i++)
  {
    // get pointer to subject header
    pSubjectHdr = (SUBJECT_FLASH_HEADER_TYPE*)(mpBlockData + mReadPos);

    // get subject - we need to search to ensure that the subject
    // is still in the list for this block
    pSubject = NULL;
    for (int j = 0; j < mSubjectCount; j++)
    {
      if (mpSubjects[j].subjectId == pSubjectHdr->subjectId)
      {
        pSubject = mpConfigControl->GetSubject(pSubjectHdr->subjectId);
        break;
      }
    }

    // subject found?
    if (pSubject)
    {
      // set read pos to start of data
      mReadPos += SUBJECT_FLASH_HEADER_SIZE;

      // verify flash ID
      if (pSubjectHdr->flashId == pSubject->GetFlashId())
      {
        // store start read pos, set number of bytes available for read and clear read error flag
        startReadPos = mReadPos;
        mReadAvailable = pSubjectHdr->dataLength;
        mReadError = false;

        // load subject
        pSubject->LoadFromFlash(this, (FLASH_SAVE_TYPE)pSubjectHdr->saveType);

        // verify number of bytes read by the subject
        if (mReadError || ((mReadPos - startReadPos) != pSubjectHdr->dataLength))
        {
          // adjust read pos to get back on track
          mReadPos = startReadPos + pSubjectHdr->dataLength;

          // clear success flag
          success = false;
        }
      }
      else
      {
        // subject type has changed, adjust read position to get back on track
        mReadPos += pSubjectHdr->dataLength;

        // This is ok in case the pc edition of the controller loads an old configuration file 
        // that contains subjects which have been removed in the current version.
        #ifndef __PC__
        // clear success flag
        success = false;
        #endif
      }
    }
    else
    {
      // subject not found (no longer in DB or moved to another block)
      // adjust read position to get back on track
      mReadPos += (SUBJECT_FLASH_HEADER_SIZE + pSubjectHdr->dataLength);
    }
  }

  return success;
}

/*****************************************************************************
 * FUNCTION - ValidateAndSetBlockHeader
 * DESCRIPTION: Returns true if the block header is valid
 ****************************************************************************/
bool FlashBlock::ValidateAndSetBlockHeader(const FLASH_BLOCK_HEADER_TYPE* pHdr)
{
	if ((pHdr->version == 1) &&
		  (strncmp(mFlashID.c_str(), pHdr->szFlashID, sizeof(pHdr->szFlashID) - 1) == 0) &&
			(pHdr->blockSize <= mMaxBlockSize))
	{
		// set block size and subject count
		mpBlockHdr->blockSize = pHdr->blockSize;
		mpBlockHdr->subjectCount = pHdr->subjectCount;

		// valid
		return true;
	}

	// NOT valid
	return false;
}

/*****************************************************************************
 * FUNCTION - ValidateBlockData
 * DESCRIPTION: Returns true if the block data is valid
 ****************************************************************************/
bool FlashBlock::ValidateBlockData(void)
{
  bool valid = false;
	SUBJECT_FLASH_HEADER_TYPE* pSubjectHdr;
	U32 readPos = 0;

	// verify data size
	for (int i = 0; i < mpBlockHdr->subjectCount; i++)
	{
		// get pointer to subject header
		pSubjectHdr = (SUBJECT_FLASH_HEADER_TYPE*)(mpBlockData + readPos);

		// calculate next read pos
		readPos += SUBJECT_FLASH_HEADER_SIZE;
    readPos += pSubjectHdr->dataLength;

		// check size
		if (readPos > GetDataSize())
		{
			// NOT valid, break out
      valid = false;
			break;
		}
	}

  // valid ?
  valid = ((FLASH_BLOCK_HEADER_SIZE + readPos) == mpBlockHdr->blockSize);
	return valid;
}

/*****************************************************************************
 * FUNCTION - SaveToFlash
 * DESCRIPTION: Returns true on success
 ****************************************************************************/
bool FlashBlock::SaveToFlash(void)
{
  bool success = false;

#ifndef __PC__

  int count = mpBlockHdr->blockSize;

  // flash is 16-bit - adjust to 2-byte boundary if necessary
  if ((count % 2) != 0)
  {
    count++;
  }

  success = FlashControl::GetInstance()->SaveData(mpBlockCache, count, mFlashControlBlockId);
#else
  success = SaveToDisk();
#endif

  // set / reset error flag
  if (success)
  {
    mErrorFlags &= ~ERROR_FLAG_SAVE_TO_FLASH_FAILED;
  }
  else
  {
    LoadFromFlash(); // Restore previous data (assumed to be valid)
    mErrorFlags |= ERROR_FLAG_SAVE_TO_FLASH_FAILED;
  }

  mModified = false; // Clear this flag here to avoid repeat of SaveToFlash in case of error

  return success;

}

/*****************************************************************************
 * FUNCTION - LoadFromFlash
 * DESCRIPTION: Returns true on success
 ****************************************************************************/
bool FlashBlock::LoadFromFlash(void)
{
  bool success = false;

#ifndef __PC__
  FlashControl* pFlashControl = FlashControl::GetInstance();
  FLASH_BLOCK_HEADER_TYPE header;

  // flash block valid?
  if (pFlashControl->IsBlockValid(mFlashControlBlockId))
  {
    // read header
    if (pFlashControl->ReadData((U8*)&header, 0, FLASH_BLOCK_HEADER_SIZE, mFlashControlBlockId))
    {
      // validate and set header
      if (ValidateAndSetBlockHeader(&header))
      {
        int count = mpBlockHdr->blockSize - FLASH_BLOCK_HEADER_SIZE;

        // flash is 16-bit - adjust to 2-byte boundary if necessary
        if ((count % 2) != 0)
        {
          count++;
        }

        // read data
        if (pFlashControl->ReadData(mpBlockData, FLASH_BLOCK_HEADER_SIZE, count, mFlashControlBlockId))
        {
          // validate block data
  				if (ValidateBlockData())
  				{
            // success so far, load all subjects
  					success = LoadAllSubjectsFromCache();
  				}
        }
      }
    }
  }
#else
  success = LoadFromDisk();
#endif

	// always save all subjects to cache after loaf to ensure proper organisation of the flash block
  SaveAllSubjectsToCache();

  // set / reset error flag
  if (success)
  {
    mErrorFlags &= ~ERROR_FLAG_LOAD_FROM_FLASH_FAILED;
  }
  else
  {
    mErrorFlags |= ERROR_FLAG_LOAD_FROM_FLASH_FAILED;
  }

  return success;
}

/*****************************************************************************
 * FUNCTION - GetErrorFlags
 * DESCRIPTION:
 ****************************************************************************/
int FlashBlock::GetErrorFlags()
{
  return mErrorFlags;
}

/*****************************************************************************
 * FUNCTION - ClearErrorFlag
 * DESCRIPTION:
 ****************************************************************************/
void FlashBlock::ClearErrorFlags()
{
  mErrorFlags = 0;
}

/*****************************************************************************
 * FUNCTION - GetFileName
 * DESCRIPTION: For use on PC platform only
 ****************************************************************************/
#ifdef __PC__
char* FlashBlock::GetFileName(void)
{
  static char szFileName[MAX_PATH];

  switch (mFlashControlBlockId)
  {
  case FLASH_CONTROL_BLOCK_ID_CONFIG:
    sprintf(szFileName, "%sflash_block_config.bin", BaseDirectory::GetInstance()->Get() );
    break;
  case FLASH_CONTROL_BLOCK_ID_LOG:
    sprintf(szFileName, "%sflash_block_log.bin", BaseDirectory::GetInstance()->Get() );
    break;
  case FLASH_CONTROL_BLOCK_ID_GSC:
    sprintf(szFileName, "%sflash_block_gsc.bin", BaseDirectory::GetInstance()->Get() );
    break;
  case FLASH_CONTROL_BLOCK_ID_NO_BOOT:
    sprintf(szFileName, "%sflash_block_no-boot.bin", BaseDirectory::GetInstance()->Get() );
    break;
  case FLASH_CONTROL_BLOCK_ID_USER_LOG_1:
    sprintf(szFileName, "%sflash_block_user_log_1.bin", BaseDirectory::GetInstance()->Get() );
    break;
  case FLASH_CONTROL_BLOCK_ID_USER_LOG_2:
    sprintf(szFileName, "%sflash_block_user_log_2.bin", BaseDirectory::GetInstance()->Get() );
    break;
  default:
    sprintf(szFileName, "%sflash_block_UNKNOWN-BLOCK-ID.bin", BaseDirectory::GetInstance()->Get() );
    break;
  }

  return szFileName;
}
#endif

/*****************************************************************************
 * FUNCTION - SaveToDisk
 * DESCRIPTION: For use on PC platform only
 ****************************************************************************/
#ifdef __PC__
bool FlashBlock::SaveToDisk(void)
{
	std::ofstream of(GetFileName(), std::ios::out | std::ios::binary);

	const char* p = (const char*)mpBlockCache;

  of.write(p, mpBlockHdr->blockSize);
  of.flush();
  of.close();
  return of.good();
}
#endif

/*****************************************************************************
 * FUNCTION - LoadFromDisk
 * DESCRIPTION: For use on PC platform only
 ****************************************************************************/
#ifdef __PC__
bool FlashBlock::LoadFromDisk(void)
{
	std::ifstream in(GetFileName(), std::ios::in | std::ios::binary);

	if (in.good())
	{
		FLASH_BLOCK_HEADER_TYPE blockHdr;

		in.read((char*)&blockHdr, FLASH_BLOCK_HEADER_SIZE);

		if (ValidateAndSetBlockHeader(&blockHdr))
		{
			in.read((char*)mpBlockData, blockHdr.blockSize - FLASH_BLOCK_HEADER_SIZE);

			if (in.good())
			{
				return LoadAllSubjectsFromCache();
			}
		}
	}

	return false;
}
#endif

/*****************************************************************************
 * FUNCTION - CanWrite
 * DESCRIPTION:
 ****************************************************************************/
 bool FlashBlock::CanWrite(const U32 noOfBytes)
{
  if (!mWriteError && mWriteAvailable >= noOfBytes)
  {
    return true;
  }
  else
  {
    mWriteError = true;
    return false;
  }
}

/*****************************************************************************
 * FUNCTION - CanRead
 * DESCRIPTION:
 ****************************************************************************/
bool FlashBlock::CanRead(const U32 noOfBytes)
{
  if (!mReadError && mReadAvailable >= noOfBytes)
  {
    return true;
  }
  else
  {
    mReadError = true;
    return false;
  }
}
