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
/* CLASS NAME       : AnalogInputConfListView                              */
/*                                                                          */
/* FILE NAME        : AnalogInputConfListView.H                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class implements the digital input configuration ListView           */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcANALOG_INPUT_SETUP_LISTVIEW_H
#define mpcANALOG_INPUT_SETUP_LISTVIEW_H

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include "AppTypeDefs.h"
#include "DisplayTypes.h"

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ObserverGroup.h>
#include <FloatDataPoint.h>
#include <EnumDataPoint.h>

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
    class ListView;
    
    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class AnalogInputSetupListView : public ObserverGroup
    {
      public:
        /********************************************************************
        LIFECYCLE - Default constructor.
        ********************************************************************/
        AnalogInputSetupListView(Component* pParent = NULL);
        /********************************************************************
        LIFECYCLE - Destructor.
        ********************************************************************/
        ~AnalogInputSetupListView();
        /********************************************************************
        ASSIGNMENT OPERATOR
        ********************************************************************/

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
        /* --------------------------------------------------
        * Sets the font of this text element
        * --------------------------------------------------*/
        virtual void SetFont(const GUI_FONT** Font);

        /* --------------------------------------------------
        * Sets the component to be displayed when this tab
        * is active.
        * --------------------------------------------------*/
        virtual void SetDisplay(Display* pDisplay);

        /* --------------------------------------------------
        * Sets next list view 
        * --------------------------------------------------*/
        virtual void SetNextList(ListView* pList);
        
        /* --------------------------------------------------
        * Sets previous list view 
        * --------------------------------------------------*/
        virtual void SetPrevList(ListView* pList);

        /* --------------------------------------------------
        * Sets selected listview item 
        * --------------------------------------------------*/
        bool SetSelection(int itemIndex);
        
        /* --------------------------------------------------
        * Get first listview 
        * --------------------------------------------------*/
        ListView* GetFirstListView();

        /* --------------------------------------------------
        * Get last listview 
        * --------------------------------------------------*/
        ListView* GetLastListView();

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
        MODE_CHECK_BOX_LABEL_VALUE* mpAnalogInputSetupListViewListData;
        int mAnalogInputSetupListViewListDataLength;

        ListView* mpAnaInSetup;

        ListView* mpAnaInConfHeader;
        ListView* mpAnaInConf;

        ListView* mpAnaInRangeHeader;
        ListView* mpAnaInRange;

        SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*> mpSensorElectric;
        SubjectPtr<FloatDataPoint*> mpSensorMin;
        SubjectPtr<FloatDataPoint*> mpSensorMax;
    };

  } // namespace display
} // namespace mpc

#endif
