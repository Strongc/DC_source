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
/* FILE NAME        : InfoFieldObserverGroup.cpp                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
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
#include <vector>
#include <Factory.h>
#include "InfoFieldObserverGroup.h"
#include "Label.h"
#include "Image.h"
#include "MPCFonts.h"
#include <FactoryTypes.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmInfo;
#define WIDTH 240
#define HEIGHT 30
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    InfoFieldObserverGroup::InfoFieldObserverGroup(Component* pParent)
    {
      mUpdated = true;
      mStartShow = true;
      mUpdateCounter = 0;

      mpActFrame = new Frame();
      mpActFrame->SetClientArea(0,0,WIDTH-1,HEIGHT-1);
      mpActFrame->SetFillBackground(true);
      mpActFrame->SetFrame(true);
      mpActFrame->SetColour(GUI_COLOUR_FRAME_DEFAULT);
      mpActFrame->SetBackgroundColour(GUI_COLOUR_INFOLINE_BACKGROUND);
      mpActFrame->SetVisible(true);
      AddChild(mpActFrame);

      mpInfoImage = new Image();
      mpInfoImage->SetBitmap(&bmInfo);
      mpActFrame->AddChild(mpInfoImage);
      mpInfoImage->SetClientArea(2,mpActFrame->GetHeight()/2-6,15,mpActFrame->GetHeight()/2+6);
      mpInfoImage->SetVisible(true);

      mpFirstLine = new Label();
      mpFirstLine->SetAlign(GUI_TA_VCENTER|GUI_TA_LEFT);
      mpFirstLine->SetBackgroundColour(GUI_COLOUR_INFOLINE_BACKGROUND);
      mpFirstLine->SetChildPos(17,1);
      mpFirstLine->SetColour(GUI_COLOUR_INFOLINE_FOREGROUND);
      mpFirstLine->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpFirstLine->SetHeight(13);
      mpFirstLine->SetWidth(WIDTH-17-1);
      mpFirstLine->SetLeftMargin(0);
      mpFirstLine->SetRightMargin(0);
      mpFirstLine->SetStringId(0);
      mpFirstLine->SetVisible(false);
      mpActFrame->AddChild(mpFirstLine);

      mpSecondLine = new Label();
      mpSecondLine->SetAlign(GUI_TA_VCENTER|GUI_TA_LEFT);
      mpSecondLine->SetBackgroundColour(GUI_COLOUR_INFOLINE_BACKGROUND);
      mpSecondLine->SetChildPos(17,15);
      mpSecondLine->SetColour(GUI_COLOUR_INFOLINE_FOREGROUND);
      mpSecondLine->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpSecondLine->SetHeight(13);
      mpSecondLine->SetWidth(WIDTH-17-1);
      mpSecondLine->SetLeftMargin(0);
      mpSecondLine->SetRightMargin(0);
      mpSecondLine->SetStringId(0);
      mpSecondLine->SetVisible(false);
      mpActFrame->AddChild(mpSecondLine);

      mpOnlyOneLine = new Label();
      mpOnlyOneLine->SetAlign(GUI_TA_VCENTER|GUI_TA_LEFT);
      mpOnlyOneLine->SetBackgroundColour(GUI_COLOUR_INFOLINE_BACKGROUND);
      mpOnlyOneLine->SetChildPos(17,3);
      mpOnlyOneLine->SetColour(GUI_COLOUR_INFOLINE_FOREGROUND);
      mpOnlyOneLine->SetFont(DEFAULT_FONT_18_LANGUAGE_DEP);
      mpOnlyOneLine->SetHeight(20);
      mpOnlyOneLine->SetWidth(WIDTH-17-1);
      mpOnlyOneLine->SetLeftMargin(0);
      mpOnlyOneLine->SetRightMargin(0);
      mpOnlyOneLine->SetStringId(0);
      mpOnlyOneLine->SetVisible(false);
      mpActFrame->AddChild(mpOnlyOneLine);
      SetVisible(false);
    }

    InfoFieldObserverGroup::~InfoFieldObserverGroup()
    {
      delete mpInfoImage;
      delete mpFirstLine;
      delete mpSecondLine;
      delete mpActFrame;
      delete mpOnlyOneLine;
    }

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void InfoFieldObserverGroup::Update(Subject* Object)
    {
      mUpdated = true;
    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void InfoFieldObserverGroup::SubscribtionCancelled(Subject* pSubject)
    {
      mpRemoteBusContolled.Detach(pSubject);
      mpRemoteServiceContolled.Detach(pSubject);
      mpRemoteVNCContolled.Detach(pSubject);
    }

    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void InfoFieldObserverGroup::SetSubjectPointer(int Id,Subject* pSubject)
    {
      switch (Id)
      {
      case SP_SG_INFO_FIELD_REMOTE_BUS_ACTIVE:
        mpRemoteBusContolled.Attach(pSubject);
        break;
      case SP_SG_INFO_FIELD_REMOTE_SERVICE_ACTIVE:
        mpRemoteServiceContolled.Attach(pSubject);
        break;
      case SP_SG_INFO_FIELD_REMOTE_VNC_ACTIVE:
        mpRemoteVNCContolled.Attach(pSubject);
        break;
      }
    }

    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void InfoFieldObserverGroup::ConnectToSubjects(void)
    {
      INFLUENCE_BOOL_SID temp_bool_sid;

      if (mpRemoteVNCContolled.IsValid())
      {
        mpRemoteVNCContolled->Subscribe(this);
        temp_bool_sid.sid =  SID_INFO_REMOTE_VNC_ACTIVE;
        temp_bool_sid.db =  &mpRemoteVNCContolled;
        mBoolDataPoints.push_back(temp_bool_sid);
      }
      if (mpRemoteBusContolled.IsValid())
      {
        mpRemoteBusContolled->Subscribe(this);
        temp_bool_sid.sid =  SID_INFO_REMOTE_BUS_ACTIVE;
        temp_bool_sid.db =  &mpRemoteBusContolled;
        mBoolDataPoints.push_back(temp_bool_sid);
      }
      if (mpRemoteServiceContolled.IsValid())
      {
        mpRemoteServiceContolled->Subscribe(this);
        temp_bool_sid.sid =  SID_INFO_REMOTE_SERVICE_ACTIVE;
        temp_bool_sid.db =  &mpRemoteServiceContolled;
        mBoolDataPoints.push_back(temp_bool_sid);
      }
    }

    void InfoFieldObserverGroup::Run(void)
    {
      if (mUpdated)
      {
        mUpdated = false;
        mpFirstLine->SetStringId(0);
        mpSecondLine->SetStringId(0);
        mpOnlyOneLine->SetStringId(0);

        mpOnlyOneLine->SetVisible(false);

        std::vector< INFLUENCE_BOOL_SID >::iterator iterBoolInfluences = mBoolDataPoints.begin();
        std::vector< INFLUENCE_BOOL_SID >::iterator iterBoolInfluencesEnd = mBoolDataPoints.end();

        for( ; iterBoolInfluences != iterBoolInfluencesEnd; ++iterBoolInfluences )
        {
	    if (/* (*((*iterBoolInfluences).db)) &&*/ ((*((*iterBoolInfluences).db))->GetValue())  )
          {
            if ( mpFirstLine->GetStringId() )
            {
              if ( mpSecondLine->GetStringId() )
              {
                break;  // We are done - the 2 line are filled out!
              }
              else
              {
                mpSecondLine->SetStringId(((*iterBoolInfluences).sid));
                mpSecondLine->SetVisible(true);
              }
            }
            else
            {
              mpFirstLine->SetStringId(((*iterBoolInfluences).sid));
              mpFirstLine->SetVisible(true);
            }
          }
        }

        if ( (mpOnlyOneLine->GetStringId() == 0) && (mpFirstLine->GetStringId() == 0) && (mpSecondLine->GetStringId() == 0) )
        {
          SetVisible(false);
          if (mpParent)
          {
            mpParent->Invalidate();
          }
        }
        else
        {

          if (mUpdateCounter == 0)
          {
            mUpdateCounter = HEIGHT;
            mStartShow = true;
          }
          mpActFrame->SetClientArea(0,mUpdateCounter,WIDTH-1, mUpdateCounter + HEIGHT-1);
          SetVisible(true);
        }
      }
      
      if (mStartShow)
      {
        mUpdateCounter -= mUpdateCounter/4 + 1;
        if (mUpdateCounter <= 0)
        {
          mStartShow = false;
          mUpdateCounter = 0;
        }
        mpActFrame->SetClientArea(0,mUpdateCounter,WIDTH-1, mUpdateCounter + HEIGHT-1);
      }
      ObserverGroup::Run();
    }


#ifdef __PC__
    void InfoFieldObserverGroup::CalculateStringWidths(bool forceVisible)
    {
      mpFirstLine->SetVisible();
      
      mpFirstLine->SetStringId(SID_INFO_REMOTE_VNC_ACTIVE);
      mpFirstLine->CalculateStringWidths(true);     
      mpFirstLine->SetStringId(SID_INFO_REMOTE_BUS_ACTIVE);
      mpFirstLine->CalculateStringWidths(true);
      mpFirstLine->SetStringId(SID_INFO_REMOTE_SERVICE_ACTIVE);
      mpFirstLine->CalculateStringWidths(true);
      
      mpFirstLine->SetVisible(false);
    }
#endif // __PC__

  } // namespace display
} // namespace mpc


