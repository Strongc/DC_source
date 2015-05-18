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
/* CLASS NAME       : MessageBox                                            */
/*                                                                          */
/* FILE NAME        : MessageBox.h                                          */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayMessageBox_h
#define mpc_displayMessageBox_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <PopupBox.h>
#include <Image.h>
/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
typedef enum _MB_STYLE
{
  MB_STYLE_NONE        = 0,
  MB_STYLE_HELP        = 1,
  MB_STYLE_INFORMATION = MB_STYLE_HELP << 1,
  MB_STYLE_WARNING     = MB_STYLE_INFORMATION << 1,
  MB_STYLE_ERROR       = MB_STYLE_WARNING << 1,
  MB_STYLE_CRITICAL    = MB_STYLE_ERROR
} MB_STYLE;
namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    FOWARD declarations
    *****************************************************************************/
    class Label;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class MessageBox : public PopupBox
    {
    public:
      MessageBox(Component* pParent = NULL, bool autoDelete=true);
      virtual ~MessageBox();

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SetTitle(STRING_ID title);
      virtual void SetMessage(STRING_ID message);
      virtual void SetStyle(MB_STYLE style);

      virtual bool HandleKeyEvent(Keys KeyID);

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
      bool  mAutoDelete;

      Label*  mpTitle;
      Label*  mpMessage;
      Image*  mpImage;
    };
  } // namespace display
} // namespace mpc

#endif
