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
/* CLASS NAME       : DigitalInputConfListView                              */
/*                                                                          */
/* FILE NAME        : DigitalInputConfListView.H                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class implements the digital input configuration ListView           */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __DIGITAL_INPUT_CONF_LISTVIEW_H__
#define __DIGITAL_INPUT_CONF_LISTVIEW_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "ListView.h"
#include "MultiString.h"
#include "DataPointText.h"

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
    class DigitalInputConfListView : public Observer, public ListView
    {
      public:
        /********************************************************************
        LIFECYCLE - Default constructor.
        ********************************************************************/
        DigitalInputConfListView(Component* pParent = NULL);
        /********************************************************************
        LIFECYCLE - Destructor.
        ********************************************************************/
        ~DigitalInputConfListView();
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
        void AddAvailableIfSets(int rowIndex, int checkValue);
        void CreateExtraFaultRows(int firstRowIndex);
        void CreateUserDefinedCounterRows(int firstRowIndex);

        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        MultiString* mpExtraEditName[NO_OF_EXTRA_FAULTS];
        DataPointText* mpExtraShowName[NO_OF_EXTRA_FAULTS];
        DataPointText* mpUSDCounterName[NO_OF_USD_COUNTERS];
        
        /*******************************************************************
        TYPE DEFINES
        ********************************************************************/
        typedef enum
        {
          COLUMN_LABEL = 0,
          COLUMN_EDIT_NAME,
          COLUMN_CHECK_BOX,
          COLUMN_AVAILABLE_FSW,
          COLUMN_AVAILABLE_PUMPS,
          COLUMN_VFD_INSTALLED
        }COLUMN;
    };

  } // namespace display
} // namespace mpc

#endif
