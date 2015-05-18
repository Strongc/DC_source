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
/* CLASS NAME       : StatusGraphics                                                 */
/*                                                                          */
/* FILE NAME        : StatusGraphics.h                                               */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text assigned to a state.    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayStatusGraphics_h
#define mpc_displayStatusGraphics_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <Observer.h>
#include <FloatDataPoint.h>
#include <Frame.h>
#include <NumberQuantity.h>
#include <Text.h>
#include <Animation.h>
#include <AppTypeDefs.h>
#include <InfoFieldObserverGroup.h>
/*****************************************************************************
DEFINES
*****************************************************************************/
#define ANIMATION_SPEED_MS  500
#define RUNNING_ANIMATION_SPEED_MS 300
#define SENSOR_COUNT  28
#define NUMBER_OF_SENSOR_POS 3
#define NUMBER_OF_PUMPS 7

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
    * This Class is responsible for how to show a State string given by a DataPoint
    *
    *****************************************************************************/
    class StatusGraphics : public Frame, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      StatusGraphics(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~StatusGraphics();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void Run();
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* pSubject);

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

      // Animation for drawing pumps. If a pump has an alarm, a 2
      // frame animation is started (showing a pump and a bell).
      // Otherwise the animation is stopped and placed on frame 0.
      U8  mLastNumberOfPumps;
      Animation*  mpImagePump[NUMBER_OF_PUMPS];

      // Animation for drawing the running pumps.
      Animation*  mpImageRunningPump[NUMBER_OF_PUMPS];

      // Datapoint and Components for pump speeds.
      FloatDataPoint* mpDpPumpSpeed[NUMBER_OF_PUMPS];
      NumberQuantity*   mpNQPumpSpeeds[NUMBER_OF_PUMPS];

      // Labels to show pump number.
      Text*             mpLabelPumps[NUMBER_OF_PUMPS];

      // Pump mounting datapoint detirmins if the pump are in the return pipe
      I32DataPoint* mpDpPumpMounting;


      // Datapoint deceiding the position of the primary sensor.
      // Used to 1 - place the system pressure, flow or temp.
      // 2 - Superceed wich sensor that should be drawn if more
      // than one sensor is valid for the position where the primary
      // sensor is placed.
      I32DataPoint*   mpDpPrimarySensorIndex;

      // Component taking care of drawing the value of the primary sensor.
      // Position depends on the mpDpConfPrimarySensor value.
      NumberQuantity* mpNqPrimarySensor;

      FloatDataPoint*   mpDpSensors[SENSOR_COUNT];
      Image*            mpImageSensors[SENSOR_COUNT];
      bool              mSensorLastNeverAvailable[SENSOR_COUNT];

      // Pipe arrows to indicate flow direction.
      Image*  mpImageArrow1;
      Image*  mpImageArrow2;
      Image*  mpImageArrow3;
      Image*  mpImageArrow4;

      // Info/status field
      InfoFieldObserverGroup*  mpInfoField;

      // Flag indicating that a datapoint has changed.
      bool mUpdated;

      // Flag indicating that a pump system mode state has changed.
      // This flag starts or stops pump alarm animation.
      bool mPumpSystemModeStateUpdated;

      // Flag indicating that a pump has changed operation mnode
      bool mPumpOperationModeStateUpdated;

      // Flag indicating that pump mounting has changed.
      bool  mPumpMountUpdated;

      // Flag indicating that one or more sensor has updated.
      bool mSensorUpdated;

      bool mSensorPosUsed[NUMBER_OF_SENSOR_POS];

    };
  } // namespace display
} // namespace mpc

#endif
