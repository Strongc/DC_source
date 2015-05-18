/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MR                                            */
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
/* CLASS NAME       : FirmwareUpdateState                                   */
/*                                                                          */
/* FILE NAME        : FirmwareUpdateState.cpp                               */
/*                                                                          */
/* CREATED DATE     : 2007-10-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a FirmwareUpdateState.         */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <AppTypeDefs.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "FirmwareUpdateState.h"

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
    const StateStringId mFirmwareUpdateStatesStringIds[] =
    {
      { FIRMWARE_UPDATE_STATE_START,                SID_FIRMWARE_STARTING                 },
      { FIRMWARE_UPDATE_STATE_ERASINGPRI,           SID_FIRMWARE_DELETING_PRIMARY         },
      { FIRMWARE_UPDATE_STATE_ERASINGSEC,           SID_FIRMWARE_DELETING_SECONDARY       },
      { FIRMWARE_UPDATE_STATE_IDLE,                 SID_FIRMWARE_IDLE                     },
      { FIRMWARE_UPDATE_STATE_PROGRAMMINGPRI,       SID_FIRMWARE_PROGRAMMING_PRIMARY      },
      { FIRMWARE_UPDATE_STATE_PROGRAMMINGSEC,       SID_FIRMWARE_PROGRAMMING_SECONDARY    },
      { FIRMWARE_UPDATE_STATE_SUCCESS,              SID_FIRMWARE_UPGRADE_SUCCESFUL        },
      { FIRMWARE_UPDATE_STATE_RESETPRIMARYCOUNT,    SID_DASH                              },
      { FIRMWARE_UPDATE_STATE_RESETSECONDARYCOUNT,  SID_DASH                              },
      { FIRMWARE_UPDATE_STATE_FAILUREINTERN,        SID_FIRMWARE_INTERNAL_FAILURE         },
      { FIRMWARE_UPDATE_STATE_FAILUREFLASHWRITE,    SID_FIRMWARE_FAILURE_WRITING          },
      { FIRMWARE_UPDATE_STATE_FAILUREPING,          SID_FIRMWARE_UNABLE_TO_REACH          },
      { FIRMWARE_UPDATE_STATE_FAILURENETWORKDOWN,   SID_FIRMWARE_NETWORK_UNAVAILABLE      },
      { FIRMWARE_UPDATE_STATE_FAILURENOTFTPSERVER,  SID_FIRMWARE_TFTP_SERVER_NOT          },
      { FIRMWARE_UPDATE_STATE_FAILURETFTPERROR,     SID_FIRMWARE_TFTP_SERVER_REPORTED     },
      { FIRMWARE_UPDATE_STATE_STARTBL,              SID_FIRMWARE_STARTING_BOOT_LOADER     },
      { FIRMWARE_UPDATE_STATE_RECEIVINGBL,          SID_FIRMWARE_RECEIVING_BOOT_LOADER    },
      { FIRMWARE_UPDATE_STATE_ERASINGBL,            SID_FIRMWARE_DELETING_BOOT_LOADER     },
      { FIRMWARE_UPDATE_STATE_PROGRAMMINGBL,        SID_FIRMWARE_PROGRAMMING_BOOT_LOADER  },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    FirmwareUpdateState::FirmwareUpdateState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mFirmwareUpdateStatesStringIds;
      mStringIdCount = sizeof( mFirmwareUpdateStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    FirmwareUpdateState::~FirmwareUpdateState()
    {
    }
    
    void FirmwareUpdateState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




