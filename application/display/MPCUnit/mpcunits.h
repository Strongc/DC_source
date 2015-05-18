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
/* CLASS NAME       : mpcunits                                              */
/*                                                                          */
/* FILE NAME        : mpcunits.h                                            */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcmpcunits_h
#define mpcmpcunits_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "UnitTypes.h"
#include "mpcunits.conf.h"
#include <Observer.h>
#include <Subject.h>
#include <I32DataPoint.h>
#include <BoolDataPoint.h>
#include <string_id.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*#define Q_PRESSURE_M        6    // REMENBER TO COORDINATE with the units.xls
#define Q_DIFF_PRESSURE_M   6
#define Q_FLOW_M3H          1
#define Q_HEAD_M            0
*/
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/

  class MpcUnits : public Observer, public Subject
  {
    public:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      static MpcUnits* GetInstance();
      float GetFromStandardToActualUnit(float value, QUANTITY_TYPE quantity);
      float GetFromActualUnitToStandard(float value, QUANTITY_TYPE quantity);

      double GetFromStandardToActualUnit(double value, QUANTITY_TYPE quantity);
      double GetFromActualUnitToStandard(double value, QUANTITY_TYPE quantity);

      /*****************************************************************************
      * Function
      * DESCRIPTION:
      * Returns the value of 'value' converted from the standard SI unit to a unit
      * given by 'no'
      *****************************************************************************/
      float GetFromStandardToUnitNo(float value, QUANTITY_TYPE quantity, int no);

      /*****************************************************************************
      * Function
      * DESCRIPTION:
      * Returns the value of 'value' converted from the unit given by 'no' to the
      * standard SI unit
      *****************************************************************************/
      float GetFromUnitNoToStandard(float value, QUANTITY_TYPE quantity, int no);


      float GetFromStandardToDefaultSiUnit(float value, QUANTITY_TYPE quantity);
      float GetFromDefaultSiUnitToStandard(float value, QUANTITY_TYPE quantity);
      float GetFromStandardToDefaultUsUnit(float value, QUANTITY_TYPE quantity);
      float GetFromDefaultUsUnitToStandard(float value, QUANTITY_TYPE quantity);

      STRING_ID GetActualUnitString(QUANTITY_TYPE quantity);
      STRING_ID GetStandardUnitString(QUANTITY_TYPE quantity);

      void SetDefaultSiUnit();
      void SetDefaultUsUnit();

      bool GetDefaultSiUnit();
      bool GetDefaultUsUnit();

      void SetTempDefaultSiUnit();
      void ReleaseTempDefaultSiUnit();

      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      virtual void ConnectToSubjects(void);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /****************************************************************
      *              IMPORTENT
      *
      * Please Notice that the constructor  and destructor is Private
      *
      *
      ****************************************************************/

      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
        MpcUnits();
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
        virtual ~MpcUnits();
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
        static MpcUnits* mInstance;
        bool mTempDefaultSiUnitMode;

        SubjectPtr<I32DataPoint*> mUnitsDataPoints[Q_LAST_UNIT];
        SubjectPtr<BoolDataPoint*> mDefaultSI;
        SubjectPtr<BoolDataPoint*> mDefaultUS;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
  };


#endif
