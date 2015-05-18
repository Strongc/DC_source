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
/* FILE NAME        : FlashBlock.h                                          */
/*                                                                          */
/* CREATED DATE     : 21-09-2004                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __FLASH_BLOCK_H__
#define __FLASH_BLOCK_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

#ifdef __PC__
#pragma warning(disable : 4250)
#pragma warning(disable : 4786)
#endif

#include <map>
#include <string>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <Subject.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <NonVolatileDataTables.h>
#include <FlashControl.h>
#include <ConfigControl.h>

/*****************************************************************************
  TYPE DEFINITIONS
 *****************************************************************************/
#pragma pack(push, 1)

typedef struct	// 48 bytes
{
	U8  version;
	char szFlashID[21];
	U32 blockSize;
	U16 subjectCount;
	U8  reserved[20];
} FLASH_BLOCK_HEADER_TYPE;

typedef struct
{
  U16 subjectId;        // 0 -> 65535
  U8  flashId;          // 0 -> 255
  U16 saveType : 2;     // 0 -> 3
  U16 dataLength : 14;  // 0 -> 16383
} SUBJECT_FLASH_HEADER_TYPE;

typedef struct
{
	U32 writePos;
	FLASH_SAVE_TYPE saveType;
} SUBJECT_FLASH_INFO_TYPE;

#pragma pack(pop)

typedef std::map<U16, SUBJECT_FLASH_INFO_TYPE> SUBJECT_FLASH_INFO_MAP_TYPE;
typedef SUBJECT_FLASH_INFO_MAP_TYPE::iterator SUBJECT_FLASH_INFO_MAP_ITR;

/*****************************************************************************
  CONSTANTS
 *****************************************************************************/
const int FLASH_BLOCK_HEADER_SIZE = sizeof(FLASH_BLOCK_HEADER_TYPE);

/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class ConfigControl;

/*****************************************************************************
* CLASS: FlashBlock
* DESCRIPTION: Subject flash block
*****************************************************************************/
class FlashBlock : public IFlashWriter, public IFlashReader
{
public:
	FlashBlock(ConfigControl* pConfigControl, const char* szFlashID, U32 maxBlockSize, const NON_VOLATILE_SUBJECT_TYPE* pSubjects, const U16 subjectCount, FLASH_CONTROL_BLOCK_ID_TYPE flashControlBlockId);
	~FlashBlock();

  // IFlashWriter / IFlashReader operations
	virtual void WriteBool(const bool value);
	virtual bool ReadBool(const bool defaultValue);
	virtual void WriteU8(const U8 value);
	virtual U8 ReadU8(const U8 defaultValue);
	virtual void WriteU16(const U16 value);
	virtual U16 ReadU16(const U16 defaultValue);
	virtual void WriteI16(const I16 value);
	virtual I16 ReadI16(const I16 defaultValue);
	virtual void WriteU32(const U32 value);
	virtual U32 ReadU32(const U32 defaultValue);
	virtual void WriteI32(const I32 value);
	virtual I32 ReadI32(const I32 defaultValue);
	virtual void WriteFloat(const float value);
	virtual float ReadFloat(const float defaultValue);
	virtual void WriteDouble(const double value);
	virtual double ReadDouble(const double defaultValue);
	virtual void WriteString(const char* szValue, int maxLen);
	virtual void ReadString(char* szValue, int maxLen, const char* szDefaultValue);
  virtual void WriteMpcTime(MpcTime* pTime);
  virtual void ReadMpcTime(MpcTime* pTime);

	bool SaveToCache(Subject* pSubject);

	U32 GetBlockSize(void) { return mpBlockHdr->blockSize; };
	U16 GetSubjectCount(void) { return mpBlockHdr->subjectCount; };
	U32 GetDataSize(void) { return mpBlockHdr->blockSize - FLASH_BLOCK_HEADER_SIZE; };

  bool IsModified(void) { return mModified; }

  bool SaveToFlash(void);
  bool LoadFromFlash(void);

  int GetErrorFlags(void);
  void ClearErrorFlags(void);

private:
#ifdef __PC__
  char* GetFileName(void);
	bool SaveToDisk(void);
	bool LoadFromDisk(void);
#endif

	bool ValidateAndSetBlockHeader(const FLASH_BLOCK_HEADER_TYPE* pHdr);
	bool ValidateBlockData(void);
	void SaveAllSubjectsToCache(void);
	bool LoadAllSubjectsFromCache(void);
  bool CanWrite(const U32 noOfBytes);
  bool CanRead(const U32 noOfBytes);

private:
  ConfigControl* mpConfigControl;   // GetSubject
	const std::string mFlashID;
	const U32 mMaxBlockSize;
	const NON_VOLATILE_SUBJECT_TYPE* mpSubjects;
	const U16 mSubjectCount;
  const FLASH_CONTROL_BLOCK_ID_TYPE mFlashControlBlockId;

	U8* mpBlockCache;
	FLASH_BLOCK_HEADER_TYPE* mpBlockHdr;
	U8* mpBlockData;

	U32 mWritePos;
	U32 mWriteAvailable;
  bool mWriteError;
	U32 mReadPos;
	U32 mReadAvailable;
  bool mReadError;
	SUBJECT_FLASH_INFO_MAP_TYPE mSubjectInfo;
  bool mModified;

  int mErrorFlags;
};

#endif

