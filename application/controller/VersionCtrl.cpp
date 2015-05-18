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
/* CLASS NAME       : VersionCtrl                                           */
/*                                                                          */
/* FILE NAME        : VersionCtrl.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 04-05-2005  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "VersionCtrl.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  C functions declarations
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
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
VersionCtrl::VersionCtrl(void)
{
  /* Initialize member variables */
  mpVersionCpuBcd = 0;
  mpVersionIobBcd = 0;
  mpVersionCpuString = 0;
  mpVersionIobString = 0;

  /* Force update */
  mVersionCpuChanged = true;
  mVersionIobChanged = true;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
VersionCtrl::~VersionCtrl(void)
{
  if (mpVersionCpuBcd)
    mpVersionCpuBcd->Unsubscribe(this);
  if (mpVersionIobBcd)
    mpVersionIobBcd->Unsubscribe(this);
  if (mpVersionCpuString)
    mpVersionCpuString->Unsubscribe(this);
  if (mpVersionIobString)
    mpVersionIobString->Unsubscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VersionCtrl::InitSubTask(void)
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VersionCtrl::RunSubTask(void)
{
  #define ASCII_0_OFFSET       0x30
  #define BCD_MASK             0x0F

  U32 IobBcdVersion;
  char IobVersion[] = "CU351IO V00.00.00";
  int i, string_pos;


  if( mVersionCpuChanged==true )
  {
    mpVersionCpuString->SetValue("CU351CPU V00.00.10");
  }

  if( mVersionIobChanged==true )
  {
    IobBcdVersion = mpVersionIobBcd->GetValue();

    string_pos = sizeof( IobVersion ) - 1;  // Last version number byte
    for ( i=0; i<6; i++ )
    {
      IobVersion[string_pos] = ( IobBcdVersion & BCD_MASK ) + ASCII_0_OFFSET;
      IobBcdVersion>>=4;
      string_pos--;
      if ( IobVersion[string_pos] == '.' )
      {
        string_pos--;
      }
    }

    mpVersionIobString->SetValue(IobVersion);
  }

  mVersionCpuChanged = false;
  mVersionIobChanged = false;
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void VersionCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpVersionCpuBcd)
    mVersionCpuChanged = true;
  if (pSubject == mpVersionIobBcd)
    mVersionIobChanged = true;

  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void VersionCtrl::SubscribtionCancelled(Subject* pSubject)
{
  if (pSubject == mpVersionCpuBcd)
    mpVersionCpuBcd = 0;
  if (pSubject == mpVersionIobBcd)
    mpVersionIobBcd = 0;
  if (pSubject == mpVersionCpuString)
    mpVersionCpuString = 0;
  if (pSubject == mpVersionIobString)
    mpVersionIobString = 0;
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void VersionCtrl::ConnectToSubjects(void)
{
  mpVersionCpuBcd->Subscribe(this);
  mpVersionIobBcd->Subscribe(this);
  mpVersionCpuString->Subscribe(this);
  mpVersionIobString->Subscribe(this);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void VersionCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case DP_VER_CONT_CPU_BCD :
      mpVersionCpuBcd = (DataPoint<U32>*)pSubject;
      break;
    case DP_VER_CONT_IOB_BCD :
      mpVersionIobBcd = (DataPoint<U32>*)pSubject;
      break;
//    case DP_VER_CONT_CPU_STRING :
//      mpVersionCpuString = (StringDataPoint*)pSubject;
//      break;
//    case DP_VER_CONT_IOB_STRING :
//      mpVersionIobString = (StringDataPoint*)pSubject;
//      break;
  }
}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

