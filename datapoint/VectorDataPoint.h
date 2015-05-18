/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Plaform                                   */
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
/* CLASS NAME       : VectorDataPoint                                       */
/*                                                                          */
/* FILE NAME        : VectorDataPoint.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 30-05-2007   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION : A datapoint containing a set of int values.     */
/*  The IntVectorDataPoint is protected by semaphores, so none of its       */
/*  functionallity should be called from within an interrupt rutine.        */
/*                                                                          */
/****************************************************************************/
#ifndef __VECTOR_DATA_POINT_H__
#define __VECTOR_DATA_POINT_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <rtos.h>
#include <vector>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/
#include <DataPoint.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPEDEFS
*****************************************************************************/

/*****************************************************************************
* CLASS: VectorDataPoint
* DESCRIPTION: Abstract vector data point template class
*****************************************************************************/
template<typename VAL_TYPE>
class VectorDataPoint : public DataPoint
{
protected:
  /*****************************************************************************
  * LIFECYCLE - Constructor
  *****************************************************************************/
  VectorDataPoint(int initialSize, int maxSize, VAL_TYPE defaultValue) : mVector(initialSize, defaultValue), mInitialSize(initialSize), mMaxSize(maxSize), mDefaultValue(defaultValue)
  {
    mOffset = 0;
    OS_CreateRSema(&mRSem);
  }
  VectorDataPoint(int initialSize, int maxSize) : mVector(initialSize), mInitialSize(initialSize), mMaxSize(maxSize)
  {
    mOffset = 0;
    OS_CreateRSema(&mRSem);
  }

  /*****************************************************************************
  * LIFECYCLE - Destructor
  *****************************************************************************/
  virtual ~VectorDataPoint()
  {
  }

public:
  /*****************************************************************************
  * PUBLIC METHODS
  *****************************************************************************/

  /*****************************************************************************
  * FUNCTION - GetInitialSize
  * DESCRIPTION:
  *****************************************************************************/
  int GetInitialSize(void)
  {
    return mInitialSize;
  }

  /*****************************************************************************
  * FUNCTION - GetMaxSize
  * DESCRIPTION:
  * Returns the max. number of elements which can be set in the vector
  *****************************************************************************/
  int GetMaxSize(void)
  {
    return mMaxSize;
  }

  /*****************************************************************************
  * FUNCTION - Lock
  * DESCRIPTION:
  *
  *****************************************************************************/
  void Lock(void)
  {
    OS_Use(&mRSem);
  }

  /*****************************************************************************
  * FUNCTION - UnLock
  * DESCRIPTION:
  *
  *****************************************************************************/
  void UnLock(void)
  {
    OS_Unuse(&mRSem);
  }

  /*****************************************************************************
  * FUNCTION - SetDefaultValue
  * DESCRIPTION:
  * Set the default value for the vector
  *****************************************************************************/
  void SetDefaultValue(VAL_TYPE const defaultValue)
  {
    OS_Use(&mRSem);
    mDefaultValue = defaultValue;
    OS_Unuse(&mRSem);
  }

  /*****************************************************************************
  * FUNCTION - GetDefaultValue
  * DESCRIPTION:
  * Returns the default value for the vector
  *****************************************************************************/
  VAL_TYPE GetDefaultValue(void)
  {
    return mDefaultValue;
  }

  /*****************************************************************************
  * FUNCTION - GetSize
  * DESCRIPTION: Returns the number of elements currently in the vector
  *****************************************************************************/
  int GetSize(void)
  {
    int ret_value;
    OS_Use(&mRSem);
    ret_value = mVector.size();
    OS_Unuse(&mRSem);
    return ret_value;
  }

  /*****************************************************************************
  * FUNCTION - Clear
  * DESCRIPTION: Fill the default value in all of the vector.
  * NOTE:        Do NOT use mVector.clear, this will deallocate the vector.
  *****************************************************************************/
  void Clear()
  {
    OS_Use(&mRSem);
    unsigned int v_size = mVector.size();
    for (int i=0; i<v_size; i++ )
    {
      mVector[i] = mDefaultValue;
    }
    mOffset = 0;
    OS_Unuse(&mRSem);
  }

  /*****************************************************************************
  * FUNCTION - SetValue
  * DESCRIPTION: Sets the value at the specified index.
  * Returns true if observers was notified.
  *****************************************************************************/
  bool SetValue(const int index, const VAL_TYPE value)
  {
    bool notify = false;

    if (index >= 0)
    {
      OS_Use(&mRSem);
      if (index < mVector.size())
      {
        // within range, set new value
        int i = index;
        if (mOffset>0)
        {
          i = (index+mOffset) % mVector.size(); // cyclic insert
        }
        notify = mVector[i] != value;
        mVector[i] = value;
      }
      else
      {
        if (index < mMaxSize)
        {
          // enhance vector with default values
          while (mVector.size() <= index)
          {
            mVector.push_back(mDefaultValue);
          }
          // set new value
          mVector[index] = value;
          notify = true;
        }
      }
      OS_Unuse(&mRSem);

      if (notify)
      {
        NotifyObservers();
      }
      NotifyObserversE();
    }

    return notify;
  }

  /*****************************************************************************
  * FUNCTION - GetValue
  * DESCRIPTION: Gets the value at the specified index or the default value
  *              set during construction if the specified index is out of range
  *****************************************************************************/
  VAL_TYPE GetValue(const int index)
  {
    VAL_TYPE ret_value = mDefaultValue;
    OS_Use(&mRSem);
    if ((index >= 0) && (index < mVector.size()))
    {
      // within range, get the value
      int i = index;
      if (mOffset>0)
      {
        i = (index+mOffset) % mVector.size();
      }
      ret_value = mVector[i];
    }
    OS_Unuse(&mRSem);
    return ret_value;
  }

  /*****************************************************************************
  * FUNCTION - GetValues
  * DESCRIPTION: Gets a set of values from the specified index.
  *
  *****************************************************************************/
  bool GetValues(const unsigned int index, const unsigned int number, VAL_TYPE *p_destination)
  {
    bool return_value = false;
    int v_size;

    OS_Use(&mRSem);
    v_size = mVector.size();
    if (index+number <= v_size)
    {
      // within range, get the values
      int src_index = mOffset+index;
      for (int dst_index = 0; dst_index < number; dst_index++)
      {
        if (src_index >= v_size)
        {
          src_index = 0;
        }
        p_destination[dst_index] = mVector[src_index];
        src_index++;
      }
      return_value = true;
    }
    OS_Unuse(&mRSem);

    return return_value;
  }

  
  /*****************************************************************************
  * FUNCTION - CopyValues
  * DESCRIPTION: copy values between to vectors of same type and size
  *****************************************************************************/
  virtual bool CopyValues(VectorDataPoint<VAL_TYPE>* pSource)
  {
    bool notify = false;
    VAL_TYPE val;
    
    OS_EnterRegion();

    for (int i = 0; i < pSource->GetSize(); i++)
    {
      val = pSource->GetValue(i);
      if (val != mVector[i])
      {
        notify |= true;
        mVector[i] = val;
      }
    }
    OS_LeaveRegion();

    if (notify)
      NotifyObservers();

    NotifyObserversE();

		return notify;
  }

  /*****************************************************************************
  * FUNCTION - GetSumValid
  * DESCRIPTION: Gets the sum of a number of valid values from the specified index.
  *              Further, the valid values are counted.
  *
  *****************************************************************************/
  VAL_TYPE GetSumValid(const unsigned int index, const unsigned int number, const VAL_TYPE invalid_mark, unsigned int &valid_count)
  {
    VAL_TYPE ret_value = 0;
    VAL_TYPE value = 0;
    unsigned int i = index;
    unsigned int e = index + number;
    unsigned int v_size;

    OS_Use(&mRSem);
    v_size = mVector.size();
    if (e > v_size)
    {
      e = v_size;
    }
    valid_count = 0;
    while (i < e)
    {
      value = mVector[(i+mOffset)%v_size];
      if (value != invalid_mark)
      {
        ret_value += value;
        valid_count++;
      }
      i++;
    }
    OS_Unuse(&mRSem);
    return ret_value;
  }

  /*****************************************************************************
  * FUNCTION - GetSum
  * DESCRIPTION: Gets the sum of a number of valid values from the specified index.
  *
  *****************************************************************************/
  VAL_TYPE GetSum(const unsigned int index, const unsigned int number, const VAL_TYPE invalid_mark = 0)
  {
    unsigned int valid_count = 0;
    return GetSumValid(index, number, invalid_mark, valid_count);
  }

  /*****************************************************************************
  * FUNCTION - GetAverage
  * DESCRIPTION: Gets the average of a number of values from the specified index
  *
  *****************************************************************************/
  VAL_TYPE GetAverage(const unsigned int index, const unsigned int number, const VAL_TYPE invalid_mark = 0)
  {
    unsigned int valid_count = 0;
    VAL_TYPE sum = GetSumValid(index, number, invalid_mark, valid_count);
    if (valid_count == 0)
    {
      return 0;
    }
    else
    {
      return sum/valid_count;
    }
  }

  /*****************************************************************************
  * FUNCTION - PushValue
  * DESCRIPTION:  Works as a push function, but instead of moving all elements,
  *               an index-offset is just decremented. (cyclic)
  *****************************************************************************/
  void PushValue(const VAL_TYPE value, const unsigned int no_of_times = 1)
  {
    bool notify = false;

    // NOTE: Data may be corrupt if a vector is enhanced after using PushValue.
    // Therefore, PushValue is only allowed to work on fully allocated vectors.
    OS_Use(&mRSem);
    unsigned int v_size = mVector.size();
    if (v_size == mMaxSize)
    {
      for (unsigned int count = 0; count<no_of_times && count<v_size; count++)
      {
        if (mOffset == 0)
        {
          mOffset = mMaxSize-1;
        }
        else
        {
          mOffset--;
        }
        mVector[mOffset] = value;
        notify = true;
      }
    }
    else
    {
      FatalErrorOccured("VectorDataPoint: PushValue not allowed!");
    }
    OS_Unuse(&mRSem);

    if (notify)
    {
      NotifyObservers();
    }
    NotifyObserversE();
  }

  /*****************************************************************************
  * PUBLIC OPERATORS
  *****************************************************************************/

  /*****************************************************************************
  * OPERATOR - [int]
  * DESCRIPTION: Returns the value at the specified index or the default value
  *              set during construction if the specified index is out of range
  *****************************************************************************/
  VAL_TYPE operator[](int index)
  {
    return GetValue(index);
  }

private:
  /*****************************************************************************
  * PRIVATE ATTRIBUTES
  *****************************************************************************/
  const int mInitialSize;
  const int mMaxSize;
  unsigned int mOffset;
  VAL_TYPE mDefaultValue;
  std::vector<VAL_TYPE> mVector;
  OS_RSEMA  mRSem;
};

#endif
