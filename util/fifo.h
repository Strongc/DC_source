/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : Fifo                                                  */
/*                                                                          */
/* FILE NAME        : fifo.h                                                */
/*                                                                          */
/* CREATED DATE     : 26-11-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Fifo is a first-in-first-out buffer to handle   */
/*                          Queues of different object, e.q. Subjects       */
/*                          It has been introduced in mpc to replace use of */
/*                          STL components like vector, queue, deque        */
/*                          Note the following:                             */
/*                          - fifo is not re-entrant, guard use by          */
/*                            semaphores                                    */
/*                          - if objects can have the value "0" or "NULL",  */
/*                            use member function empty() to check if an    */
/*                            element can be retrieved, read() returns 0    */
/*                            in case of an empty fifo                      */
/*                          - fifo can only be used for types, that can     */
/*                            have the value "0", i.e. not class objects.   */
/*                            Trying to use fifo for such types should      */
/*                            generate a compile time error                 */
/*                                                                          */
/****************************************************************************/
#ifndef _FIFO_H_
#define _FIFO_H_

template<class T, int mCapacity>
class Fifo
{
private:
  int mReadPos, mWritePos;
  bool mEmpty;
  T mBuffer[mCapacity];

public:
  Fifo()
  {
    mReadPos  = 0;
    mWritePos = 0;
    mEmpty = true;   //initially empty
  }

  void write(T wElement)
  {
    if(mWritePos != mReadPos)   //there's room
    {
      mBuffer[mWritePos] = wElement;
      mEmpty = false;
      mWritePos++;
      if(mWritePos >= mCapacity)
      {
        mWritePos = 0;
      }
    }
    else   //read and write ptr. in same position, fifo either full or empty
    {
      if(mEmpty)
      {
        mBuffer[mWritePos] = wElement;
        mEmpty = false;
        mWritePos++;
        if(mWritePos >= mCapacity)
        {
          mWritePos = 0;
        }
      }
    }
  }

  T read(void)
  {
    T ret_val = 0;

    if(!mEmpty)
    {
      ret_val = mBuffer[mReadPos];
      mReadPos++;
      if(mReadPos >= mCapacity)
      {
        mReadPos = 0;
      }
      if(mReadPos == mWritePos)
      {
        mEmpty = true;
      }
    }
    return ret_val;
  }

  bool empty(void)
  {
    return mEmpty;
  }

  int size(void)
  {
    int ret_val = 0;

    if(mWritePos > mReadPos)
    {
      ret_val = mWritePos - mReadPos;
    }
    else if(mWritePos < mReadPos)
    {
      ret_val = mCapacity - (mReadPos - mWritePos);
    }
    else   //mWritePos == mReadPos
    {
      if(mEmpty)
      {
        ret_val = 0;
      }
      else
      {
        ret_val = mCapacity;
      }
    }
    return ret_val;
  }

  int capacity(void)
  {
    return mCapacity;
  }
};

#endif   //_FIFO_H_


