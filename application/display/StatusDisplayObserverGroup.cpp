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
/* CLASS NAME       : StatusDisplayObserverGroup                                */
/*                                                                          */
/* FILE NAME        : StatusDisplayObserverGroup.cpp                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <Factory.h>
#include "Component.h"
#include "StatusDisplayObserverGroup.h"
#include "StatusGraphics.h"
#include "ListView.h"

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
    StatusDisplayObserverGroup::StatusDisplayObserverGroup(Component* pParent): ObserverGroup(pParent)
    {
      
    }

    StatusDisplayObserverGroup::~StatusDisplayObserverGroup()
    {
    }

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void StatusDisplayObserverGroup::Run(void)
    {
      if ( !IsValid() )
      {
        int             application         = ((IIntegerDataPoint*)(mpSubject))->GetAsInt();
        Group*          the_status_graphics = NULL;
        ListView*       the_list_view       = NULL;

        Component*      a_component         = GetFirstChild();
        while ( (a_component) &&  (dynamic_cast<Group*>(a_component) == NULL) )
        {
          a_component = GetNextChild(a_component);
        }
        if (a_component)
        {
          the_status_graphics = dynamic_cast<Group*>(a_component);
        }
          
        a_component         = GetFirstChild();
        while ( (a_component) &&  (dynamic_cast<ListView*>(a_component) == NULL) )
        {
          a_component = GetNextChild(a_component);
        }
        if (a_component)
        {
          the_list_view = dynamic_cast<ListView*>(a_component);
        }

        // If no children are pressent, don't do anything
        if ( (the_list_view) && (the_status_graphics) )
        {
          switch (application)
          {
            case 0:
                    break;
            case 1:  // 1. Pressure boosting in buildings
            case 2:  // 2. Pumping out system in waterworks
            case 3:  // 3. Pressure boosting in water mains
            case 4:  // 4. Process water supply
            case 5:  // 5. Irrigation system with empty pipes at start up
            case 6:  // 6. Irrigation system with pressurized pipes at start up
              the_status_graphics->SetClientArea(0,1,239,170);
              the_list_view->SetClientArea(0,172,239,271);
              break;
            case 7:  // 7. Main circulator pump
            case 8:  // 8. Boiler shunt pump
            case 9:  // 9. Domestic hot water circulation
            case 10: // 10. District heating, main circulator
            case 11: // 11. District heating, booster pump
            case 12: // 12. System pressure keeping system

            case 13: // 13. Chilled water, primary circulator pump
            case 14: // 14. Chilled water, secondary circulator pump
            case 15: // 15. Chilled water, primary only
            case 16: // 16. Condensing water circulator pump (closed system)
            case 17: // 17. Cooling tower circulator pump (open system)
            case 18: // 18. System pressure keeping system

            case 19: // 19. Water transfer to roof tank
            case 20: // 20. Filling tank
            case 21: // 21. Emptying tank
            case 22: // 22. Boiler feed
            case 23: // 23. Const. value press. or differential pressure

            case 24: // 24. Const. value temp. or differential temperature
            case 25: // 25. Constant flow
            case 26: // 26. Constant curve (open loop)
              the_status_graphics->SetClientArea(0,1,239,170+15);
              the_list_view->SetClientArea(0,172+15,239,271);
              break;
            default: 
              the_status_graphics->SetClientArea(0,1,239,170);
              the_list_view->SetClientArea(0,172,239,271);
          }
        }
      }
      ObserverGroup::Run();
    }


  } // namespace display
} // namespace mpc


