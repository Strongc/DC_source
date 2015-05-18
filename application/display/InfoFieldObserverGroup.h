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
/* CLASS NAME       : InfoFieldObserverGroup                                */
/*                                                                          */
/* FILE NAME        : InfoFieldObserverGroup.h                              */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayInfoFieldObserverGroup_h
#define mpc_displayInfoFieldObserverGroup_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <BoolDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverGroup.h"
#include "Image.h"
#include "Label.h"
#include "Frame.h"

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
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class InfoFieldObserverGroup : public ObserverGroup
    {
    public:
      InfoFieldObserverGroup(Component* pParent = NULL);
      virtual ~InfoFieldObserverGroup();

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object);
      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);
      /* --------------------------------------------------
      * Called to set the subject pointer (used by class
      * factory)
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);

      virtual void Run(void);

#ifdef __PC__
      virtual void CalculateStringWidths(bool forceVisible);
#endif

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      SubjectPtr<BoolDataPoint*>  mpRemoteVNCContolled;
      SubjectPtr<BoolDataPoint*>  mpRemoteBusContolled;
      SubjectPtr<BoolDataPoint*>  mpRemoteServiceContolled;

      #define NUMBER_OF_INFORMATIONS 3

      typedef struct
      {
      	int sid;
        SubjectPtr<BoolDataPoint*>* db;
      } INFLUENCE_BOOL_SID;

      std::vector<INFLUENCE_BOOL_SID> mBoolDataPoints;

      bool              mUpdated;
      bool              mStartShow;
      int               mUpdateCounter;

      Image*            mpInfoImage;
      Label*            mpFirstLine;
      Label*            mpSecondLine;
      Label*            mpOnlyOneLine;
      Frame*            mpActFrame;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/

    };
  } // namespace display
} // namespace mpc

#endif
