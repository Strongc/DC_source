/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: Platform                                         */
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
/* CLASS NAME       : StringWidthCalculator                                 */
/*                                                                          */
/* FILE NAME        : StringWidthCalculator.H                               */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_StringWidthCalculator_h
#define mrc_StringWidthCalculator_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>
#include <map>
#include <stdio.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Languages.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Display.h>
#include <DisplayController.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
 
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef struct 
{
  int componentId;
  int stringId;
  int componentWidth;
  int stringWidth;
  int componentHeight;
  int stringHeight;
  bool wordwrap;
  bool fits;
  bool visible;
  bool forcedVisible;
} CSV_ENTRY;


/*****************************************************************************
* EXTERN DECLARATIONS
*****************************************************************************/
extern  bool  g_is_calculating_strings;

namespace mpc
{
  namespace display
  {

    struct StringWidthParameters
    {
      char filename[MAX_PATH];
      char firstcolumn[MAX_PATH];
      bool includeHeader;
      bool onlyRelationsInCurrentDisplay;
    };

    /*****************************************************************************
    * FOWARD DECLARATIONS
    *****************************************************************************/
    class DisplayController;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class StringWidthCalculator
    {
      public:
        /********************************************************************
        OPERATIONS
        ********************************************************************/

        static StringWidthCalculator* GetInstance(void);

        void    WriteToCSV(CSV_ENTRY entry);
        void    WriteHelpRefToCSV(STRING_ID id);
        void    ExportStringWidths(const char* filename);
        void    ExportStringWidthsAdv(StringWidthParameters* parameters);


      protected:
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        void CalculateAlarmStringWidths(void);
        void CalculateHelpStringWidths(void);
        void CalculateUnitStringWidths(void);
        void CalculateSpecialStringWidths(void);

        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        DisplayController* mpDisplayCtrl;

        FILE* mpStringLengthFile;
        char  mpFirstColumnContents[MAX_PATH];

    private:
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        StringWidthCalculator();
        ~StringWidthCalculator();
        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        static StringWidthCalculator* mInstance;


    };
  }
}

#endif
