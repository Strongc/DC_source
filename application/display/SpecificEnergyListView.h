/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : SpecificEnergyListView                               s */
/*                                                                          */
/* FILE NAME        : SpecificEnergyListView.h                              */
/*                                                                          */
/* CREATED DATE     : 2009-11-25                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Lists specific energy samples in range: 60 - 30 Hz with 2 Hz interval.   */
/* Lines with value < 0 are parsed as never available (hidden).             */
/* Lines with value == 0 are displayed as not available (--)                */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __SpecificEnergyListView_H__
#define __SpecificEnergyListView_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>
#include <FloatVectorDataPoint.h>
#include <FloatDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "ListView.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_NO_OF_GRAPH_SAMPLES 16

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
    class SpecificEnergyListView : public Observer, public ListView
    {
      public:
        /********************************************************************
        LIFECYCLE - Default constructor.
        ********************************************************************/
        SpecificEnergyListView(Component* pParent = NULL);
        /********************************************************************
        LIFECYCLE - Destructor.
        ********************************************************************/
        ~SpecificEnergyListView();
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

      private:
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        void InitializeList();
        void UpdateList();

        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        SubjectPtr<FloatVectorDataPoint*> mYValues; 

        FloatDataPoint*                   mpVirtualYValues[MAX_NO_OF_GRAPH_SAMPLES];

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
