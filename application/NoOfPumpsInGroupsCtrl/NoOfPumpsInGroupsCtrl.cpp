/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : NoOfPumpsInGroupsCtrl                                 */
/*                                                                          */
/* FILE NAME        : NoOfPumpsInGroupsCtrl.cpp                             */
/*                                                                          */
/* CREATED DATE     : 06-04-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
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
#include <NoOfPumpsInGroupsCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
NoOfPumpsInGroupsCtrl::NoOfPumpsInGroupsCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
NoOfPumpsInGroupsCtrl::~NoOfPumpsInGroupsCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void NoOfPumpsInGroupsCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  mpNoOfPumps.SetUpdated();
  mpPumpGroupsEnabled.SetUpdated();
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void NoOfPumpsInGroupsCtrl::RunSubTask()
{
  U8 no_of_pumps = mpNoOfPumps->GetValue();
  U8 no_of_pumps_in_group[NO_OF_PUMP_GROUPS];
  U8 no_of_groups_used = 0;
  U8 max_no_of_groups = no_of_pumps;
  bool updated;

  if (mpNoOfPumps.IsUpdated())
  {
    if (mpNoOfPumps->GetValue() < 2)
    {
      mpFirstPumpInGroup2->SetQuality(DP_NEVER_AVAILABLE);
    }
    else
    {
      mpFirstPumpInGroup2->SetMaxAsInt(mpNoOfPumps->GetValue());
    }
  }

  updated =  mpPumpGroupsEnabled.IsUpdated();
  updated |= mpFirstPumpInGroup2.IsUpdated();
  if (updated)
  {
    if (mpPumpGroupsEnabled->GetAsBool())
    {
      mpFirstPumpInGroup2->SetQuality(DP_AVAILABLE);

      for (U8 i = FIRST_PUMP_NO; i < no_of_pumps; i++)
      {
        if ((i+1) >= mpFirstPumpInGroup2->GetValue())
        {
          mpGroupOfPump[i]->SetValue(2);
        }
        else
        {
          mpGroupOfPump[i]->SetValue(1);
        }
      }
    }
    else
    {
      mpFirstPumpInGroup2->SetQuality(DP_NEVER_AVAILABLE);

      for (U8 i = FIRST_PUMP_NO; i < no_of_pumps; i++)
      {
        mpGroupOfPump[i]->SetValue(1);
      }
    }
  }

  for (int group_no = FIRST_PUMP_GROUP; group_no <= LAST_PUMP_GROUP; group_no++)
  {
    no_of_pumps_in_group[group_no] = 0;
  }

  if (max_no_of_groups > NO_OF_PUMP_GROUPS)
  {
    max_no_of_groups = NO_OF_PUMP_GROUPS;
  }

  for (U8 i = FIRST_PUMP_NO; i < no_of_pumps; i++)
  {
    int group_index = mpGroupOfPump[i]->GetValue() - 1;

    if (group_index > LAST_PUMP_GROUP)
    {
      FatalErrorOccured("unknown pump group");
    }

    no_of_pumps_in_group[group_index]++;

    mpGroupOfPump[i]->SetMaxAsInt(max_no_of_groups);
  }
  
  for (int i = FIRST_PUMP_GROUP; i <= LAST_PUMP_GROUP; i++)
  {
    mpNoOfPumpsInGroup[i]->SetValue(no_of_pumps_in_group[i]);

    if (i == FIRST_PUMP_GROUP || mpPumpGroupsEnabled->GetAsBool())
    {
      // set maxvalue of "max started pumps" to number of pumps in group if any is assigned.
      mpMaxStartedPumps[i]->SetMaxAsInt(max(1, no_of_pumps_in_group[i]));

      mpMinStartedPumps[i]->SetMaxAsInt(max(1, mpMaxStartedPumps[i]->GetValue()));
    }
  }

  mpTotalMaxStartedPumps->SetMaxAsInt(no_of_pumps);
  mpTotalMinStartedPumps->SetMaxAsInt(mpTotalMaxStartedPumps->GetValue());

  for (U8 i = FIRST_PUMP_GROUP; i <= LAST_PUMP_GROUP; i++)
  {
    if (mpNoOfPumpsInGroup[i]->GetValue() > 0)
    {
      no_of_groups_used++;
    }
  }

  mpNoOfGroupsUsed->SetValue(no_of_groups_used);

  // clear flag after last set-operation to avoid recursive updates
  mRunRequestedFlag = false;
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void NoOfPumpsInGroupsCtrl::ConnectToSubjects()
{
  mpNoOfPumps->Subscribe(this);

  for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
  {
    mpGroupOfPump[pump_no]->Subscribe(this);
  }

  // subscribe to "max started pumps" to update maxvalue of "min started pumps"
  for (U8 i = FIRST_PUMP_GROUP; i <= LAST_PUMP_GROUP; i++)
  {
    mpMaxStartedPumps[i]->Subscribe(this);
  }

  mpTotalMaxStartedPumps->Subscribe(this);

  mpPumpGroupsEnabled->Subscribe(this);
  mpFirstPumpInGroup2->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void NoOfPumpsInGroupsCtrl::Update(Subject* pSubject)
{
  if(mpPumpGroupsEnabled.Update(pSubject)){}
  else if(mpFirstPumpInGroup2.Update(pSubject)){}
  else if(mpNoOfPumps.Update(pSubject)){}

  if (mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void NoOfPumpsInGroupsCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void NoOfPumpsInGroupsCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_NOPIG_NO_OF_PUMPS:
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_NOPIG_PUMP_1_GROUP_NO:
      mpGroupOfPump[PUMP_1].Attach(pSubject);
      break;
    case SP_NOPIG_PUMP_2_GROUP_NO:
      mpGroupOfPump[PUMP_2].Attach(pSubject);
      break;
    case SP_NOPIG_PUMP_3_GROUP_NO:
      mpGroupOfPump[PUMP_3].Attach(pSubject);
      break;
    case SP_NOPIG_PUMP_4_GROUP_NO:
      mpGroupOfPump[PUMP_4].Attach(pSubject);
      break;
    case SP_NOPIG_PUMP_5_GROUP_NO:
      mpGroupOfPump[PUMP_5].Attach(pSubject);
      break;
    case SP_NOPIG_PUMP_6_GROUP_NO:
      mpGroupOfPump[PUMP_6].Attach(pSubject);
      break;
    case SP_NOPIG_NO_OF_PUMPS_IN_GROUP_1:
      mpNoOfPumpsInGroup[PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_NOPIG_NO_OF_PUMPS_IN_GROUP_2:
      mpNoOfPumpsInGroup[PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_NOPIG_NO_OF_GROUPS_USED:
      mpNoOfGroupsUsed.Attach(pSubject);
      break;
    case SP_NOPIG_GROUP_1_MIN:
      mpMinStartedPumps[PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_NOPIG_GROUP_1_MAX:
      mpMaxStartedPumps[PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_NOPIG_GROUP_2_MIN:
      mpMinStartedPumps[PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_NOPIG_GROUP_2_MAX:
      mpMaxStartedPumps[PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_NOPIG_TOTAL_MIN:
      mpTotalMinStartedPumps.Attach(pSubject);
      break;
    case SP_NOPIG_TOTAL_MAX:
      mpTotalMaxStartedPumps.Attach(pSubject);
      break;
    case SP_NOPIG_GROUPS_ENABLED:
      mpPumpGroupsEnabled.Attach(pSubject);
      break;
    case SP_NOPIG_FIRST_PUMP_IN_GROUP_2:
      mpFirstPumpInGroup2.Attach(pSubject);
      break;

    default:
      break;

  }
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
