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
/* CLASS NAME       : Axis                                                 */
/*                                                                          */
/* FILE NAME        : Axis.h                                               */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a Axis with an X and Y axis   */
/****************************************************************************/

/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayAxis_h
#define mpc_displayAxis_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <vector>

using namespace std;

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

enum AxisType
{
  X_AXIS = 0,
  Y_AXIS = 1,
  X_AXIS2 = 2 // TBD: Currently 2 X axis isn't supperted, but this is
              // intended to be the right X axis.
};             

namespace mpc
{
  namespace display
  {
    namespace graph
    {
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a Axis with an x and y axis and
    * a set of data.
    *
    *****************************************************************************/
    class Axis
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      Axis(AxisType axisType, const char* name = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Axis();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void  SetName(const char* name);
      virtual const char* GetName();
      virtual void SetType(AxisType type);
      virtual AxisType GetType();

      virtual void SetNumOfPoints(unsigned int points);
      virtual unsigned int GetNumOfPoints();
      virtual void SetGroups(unsigned int groups);
      virtual unsigned int GetGroups();

      virtual void SetMinValue(float value);
      virtual float GetMinValue();
      virtual void SetMaxValue(float value);
      virtual float GetMaxValue();
      virtual void FitMinMax();

      virtual void AddDataGroup(const char* name, vector<float>& data);
      virtual void AddDataGroup(AxisData*  pData);
      virtual void RemoveDataGroup(const char* name);
      virtual unsigned int GetDataGroupCount();

      virtual AxisData* GetDataGroup(const char* name);
      virtual AxisData* GetDataGroup(unsigned int index);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      AxisType     mAxisType;     // Type of axis. X_AXIS or Y_AXIS
      char*        mpName;        // Name of the axis.
      unsigned int mNumOfPoints;  // total number of points on this axis
      unsigned int mAxisGroups;   // number of groups to divide axis into
      double       mMinValue;
      double       mMaxValue;

      vector<AxisData*>  mDataGroups;

    };
    } // namespace graph
  }// namespace display
} // namespace mpc

#endif
