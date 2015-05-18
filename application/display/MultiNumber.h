/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/* CLASS NAME       : MultiNumber                                           */
/*                                                                          */
/* FILE NAME        : MultiNumber.h                                         */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a MultiNumber.                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayMultiNumber_h
#define mpc_displayMultiNumber_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Observer.h>
#include <U32DataPoint.h>
#include <IIntegerDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <Group.h>

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
    // FORWARD DECLARATIONS
    class Frame;
    class Number;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for how to show a State string given by a DataPoint
    *
    *****************************************************************************/
    class MultiNumber : public Group, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      MultiNumber(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~MultiNumber();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/
      
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();
      
      virtual void Run();
      /* --------------------------------------------------
      * Sets the number of fields for editing.
      * --------------------------------------------------*/
      virtual void  SetFieldCount(const int fields, int defaultValue = 0);

      /* --------------------------------------------------
      * Sets the min value for a field.
      * --------------------------------------------------*/
      virtual void SetFieldMinValue(const int minValue);
      /* --------------------------------------------------
      * Sets the max value for a field.
      * --------------------------------------------------*/
      virtual void SetFieldMaxValue(const int maxValue);

      virtual bool HandleKeyEvent(Keys KeyID);

      virtual Leds GetLedsStatus();
      virtual Keys GetLegalKeys();

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      virtual void ConnectToSubjects(void);

      virtual bool SetReadOnly(bool readOnly = true);

      

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
      virtual void CleanUp();
      virtual void UpdateDpsFromSubject();
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      int mFieldCount;
      int mFieldMinValue;
      int mFieldMaxValue;
      bool mMaxMinChanged;
      int mCurrentField;
      U32DataPoint* mpDps;  
      SubjectPtr<IIntegerDataPoint*> mpDpSubject;
      Frame*          mpFrames;
      Number*         mpNumbers;
    };
  } // namespace display
} // namespace mpc

#endif


