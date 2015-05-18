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
/* CLASS NAME       : AxisData                                                 */
/*                                                                          */
/* FILE NAME        : AxisData.h                                               */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AxisData with an X and Y AxisData   */
/****************************************************************************/

/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayAxisData_h
#define mpc_displayAxisData_h

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
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a AxisData with an x and y AxisData and
    * a set of data.
    *
    *****************************************************************************/
    class AxisData
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      AxisData(const char* name);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~AxisData();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SetName(const char* name);
      virtual const char* GetName();

      virtual float SetValue(unsigned int point, float value);
      virtual float GetValue(unsigned int point);

      virtual unsigned int GetNumOfPoints();

      virtual AxisData  ScaleTo(unsigned int numOfPoints);

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
      virtual AxisData  ScaleUpTo(unsigned int numOfPoints);
      virtual AxisData  ScaleDownTo(unsigned int numOfPoints);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      char*           mName;
      vector<float>  mData;
    };
    } // namespace graph
  }// namespace display
} // namespace mpc

#endif
