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
/* CLASS NAME       : AxisData                                                  */
/*                                                                          */
/* FILE NAME        : AxisData.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AxisData.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AxisData.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    namespace graph
    {
      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Constructor
      ***********************************************************************/
      AxisData::AxisData(const char* name)
      {
        SetName(name);
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Dectructor
      ***********************************************************************/
      AxisData::~AxisData()
      {
        delete mName;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets a name identifying the data associated with this object.
      ***********************************************************************/
      void AxisData::SetName(const char* name)
      {
        mName = new char[strlen(name)+1];
        strcpy(mName,name);
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Returns the name identifying this object
      ***********************************************************************/
      const char* AxisData::GetName()
      {
        return mName;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets a value for a given point. If the point is outside the range of
      * points currently in the AxisData, the range is resized up to the 
      * point.
      ***********************************************************************/
      float  AxisData::SetValue(unsigned int point, float value)
      {
        if(point >= mData.size())
        {
          int old_size = mData.size();
          mData.resize(point+1);
          int new_size = mData.size();

          // Reset new points to zero
          for(;old_size < new_size; ++old_size)
          {
            mData[old_size] = 0.0;
          }
        }

        float old_value  = mData[point];
        mData[point] = value;
        return old_value;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Gets the value for a given point. Returning 0.0 if the point dosn't
      * exist.
      ***********************************************************************/
      float AxisData::GetValue(unsigned int point)
      {
        if(point >= mData.size())
          return 0.0;

        return mData[point];
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Gets the number of points currently held.
      ***********************************************************************/
      unsigned int AxisData::GetNumOfPoints()
      {
        return mData.size();
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Returns a AxisData object scaled from the number of points held by 
      * this object to the number of points given by the numOfPoints 
      * parameter.
      ***********************************************************************/
      AxisData AxisData::ScaleTo(unsigned int numOfPoints)
      {
        if(numOfPoints > mData.size())
          return ScaleUpTo(numOfPoints);
        else if(numOfPoints < mData.size())
          return ScaleDownTo(numOfPoints);
        return *this;
      }


      //// PROTECTED ////


      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Returns a AxisData object scaled up from this object to the number of
      * point given by the numOfPoints parameter.
      ***********************************************************************/
      AxisData AxisData::ScaleUpTo(unsigned int numOfPoints)
      {
        AxisData data(mName);
        float whenToAddPoint = (float)numOfPoints / mData.size();
        int size = mData.size();
        unsigned int new_point = 0;
        float  calc_value;

        for(int i = 0; i < size;++i)
        {
          if( i > whenToAddPoint)
          {
            if(i+1 < size)
            {
              calc_value = mData[i] + (mData[i] - mData[i+1]);
            }
            else
            {
              calc_value = mData[i];
            }
            data.SetValue(new_point,calc_value);
            ++new_point;
            whenToAddPoint += i;
          }
          data.SetValue(new_point,mData[i]);
          ++new_point;
        }
        return data;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Retrurns a AxisData object scaled down from this object to the number
      * of points given by the numOfPoints parameter.
      ***********************************************************************/
      AxisData AxisData::ScaleDownTo(unsigned int numOfPoints)
      {
        AxisData data(mName);
        data.SetValue(numOfPoints-1,0.0);

        float whenToRemovePoint = (float)mData.size() / numOfPoints;

        int size = mData.size();
        unsigned int new_point = 0;

        for(int i=0; i < size;++i)
        {
          if(i < whenToRemovePoint)
          {
            data.SetValue(new_point,mData[i]);
            ++new_point;
          }
          else
          {
            whenToRemovePoint += i;
          }
        }

        return data;
      }
    } // namespace graph
  }// namespace display
} // namespace mpc
