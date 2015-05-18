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
/*                                                                          */
/* FILE NAME        : PcDevToolService.h                                    */
/*                                                                          */
/* CREATED DATE     : 28-07-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_PcDevToolService_h
#define mrc_PcDevToolService_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
struct SubjectValueParameters
{
  int id;
  float fvalue;
  char value[300];
};

struct QualityParameters
{
  int id;
  int quality;
};

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PcDevToolService
{
public:
  static PcDevToolService* GetInstance(void);

  HWND GetControllerWindow(void);

  int LoadDisplay(const char* displayname);
  int LoadDisplay(const int displayid);
  int SetSubjectValue(SubjectValueParameters* idValuePair);
  int GetSubjectValue(SubjectValueParameters* idValuePair);
  int SetSubjectQuality(QualityParameters* idQualityPair);
  int SelectListViewItem(int index);

private:
  PcDevToolService();
  ~PcDevToolService(){}

  static PcDevToolService* mInstance;

};

#endif
