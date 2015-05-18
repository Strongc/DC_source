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
/* CLASS NAME       : Axis                                                  */
/*                                                                          */
/* FILE NAME        : Axis.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a Axis.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <math.h>
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Axis.h"

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
      Axis::Axis(AxisType axisType, const char* name)
      {
          
        mNumOfPoints = 0;
        mAxisGroups = 0;
        mMinValue = -HUGE_VAL;
        mMaxValue = HUGE_VAL;
        mAxisType = axisType;
        mpName = NULL;
        SetName(name);
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Dectructor
      ***********************************************************************/
      Axis::~Axis()
      {
        int size = mDataGroups.size();
        for(int i = 0; i < size;++i)
        {
          delete mDataGroups[i];
        }
      }

      void Axis::SetType(AxisType type)
      {
        mAxisType = type;
      }

      AxisType Axis::GetType()
      {
        return mAxisType;
      }

      void  Axis::SetName(const char* name)
      {
        if(mpName != NULL)
          delete mpName;
        if(name != NULL)
        {
          mpName = new char[strlen(name) + 1];
          strcpy(mpName, name);
        }
        else
        {
          mpName = NULL;
        }
      }

      const char* Axis::GetName()
      {
        return (const char*)mpName;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Adds a data group to the Axis. The datagroup helds values for the 
      * uppersit axis. Meaning if this is an y axis the values in the AxisData
      * is x value.
      ***********************************************************************/
      void Axis::AddDataGroup(AxisData* pData)
      {
        AxisData* p_copy = new AxisData(pData->GetName());
        *p_copy = *pData;

        mDataGroups.push_back(p_copy);
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Adds a chart to the axis.
      ***********************************************************************/
      void Axis::AddDataGroup(const char* name, vector<float>& data)
      {
        AxisData* p_data = new AxisData(name);
        int size = data.size();
        p_data->SetValue(size-1,0.0);

        for(int i = 0; i < size;++i)
        {
          p_data->SetValue(i,data[i]);
        }
        mDataGroups.push_back(p_data);
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Removes all data groups with a matching name.
      ***********************************************************************/
      void Axis::RemoveDataGroup(const char* name)
      {
        vector<AxisData*>::iterator  iter = mDataGroups.begin();
        vector<AxisData*>::iterator  iter_end = mDataGroups.end();

        AxisData* p_data;
        for(;iter < iter_end;++iter)
        {
          p_data = *iter;
          if(strcmp(name, p_data->GetName()) == 0)
          {

            mDataGroups.erase(iter);
            delete p_data;
          }
        }
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Returns a direct pointer to the axis data of the given name. Use this
      * pointer to manipulate the data.
      ***********************************************************************/
      AxisData* Axis::GetDataGroup(const char* name)
      {
        vector<AxisData*>::iterator  iter = mDataGroups.begin();
        vector<AxisData*>::iterator  iter_end = mDataGroups.end();

        AxisData* p_data;
        for(;iter < iter_end;++iter)
        {
          p_data = *iter;
          if(strcmp(name, p_data->GetName()) == 0)
          {
            return p_data;
          }
        }
        return NULL;
      }

      AxisData* Axis::GetDataGroup(unsigned int index)
      {
        if(index >= mDataGroups.size())
          return NULL;
        return mDataGroups[index];
      }

      unsigned int Axis::GetDataGroupCount()
      {
        return mDataGroups.size();
      }
      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the number of points on this axis.
      ***********************************************************************/
      void Axis::SetNumOfPoints(unsigned int points)
      {
        mNumOfPoints = points;
      }


      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Returns the number of points on this axis.
      ***********************************************************************/
      unsigned int Axis::GetNumOfPoints()
      {
        return mNumOfPoints;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the number of groups this axis should be divided into.
      ***********************************************************************/
      void Axis::SetGroups(unsigned int groups)
      {
        mAxisGroups = groups;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Gets the number of groups this axis is divided into.
      ***********************************************************************/
      unsigned int Axis::GetGroups()
      {
        return mAxisGroups;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the min value for the axis.
      ***********************************************************************/
      void Axis::SetMinValue(float value)
      {
        mMinValue = value;
      }

      float Axis::GetMinValue()
      {
        return mMinValue;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Sets the max value for the axis.
      ***********************************************************************/
      void Axis::SetMaxValue(float value)
      {
        mMaxValue = value;
      }

      float Axis::GetMaxValue()
      {
        return mMaxValue;
      }

      /***********************************************************************
      * Function
      * DESCRIPTION:
      * Fits the min and max values to the values in the data groups currently
      * assigned to the axis.
      ***********************************************************************/
      void Axis::FitMinMax()
      {
        float min_found = HUGE_VAL;
        float max_found = -HUGE_VAL;

        float cur_value = 0.0;

        int group_size = mDataGroups.size();
        int num_of_points = 0;
        AxisData* axisdata;
        for(int group=0; group < group_size; ++group)
        {
          axisdata = mDataGroups[group];
          num_of_points = axisdata->GetNumOfPoints();
          for(int point = 0; point < num_of_points; ++point)
          {
            cur_value = axisdata->GetValue(point);
            if(cur_value > max_found)
              max_found = cur_value;
            if(cur_value < min_found)
              min_found = cur_value;
          }
        }
        SetMinValue(min_found);
        SetMaxValue(max_found);
      }

    } // namespace graph
  }// namespace display
} // namespace mpc