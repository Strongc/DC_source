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
/* CLASS NAME       : EthernetCtrlPumpCtr                                   */
/*                                                                          */
/* FILE NAME        : EthernetCtrl.h                                        */
/*                                                                          */
/* CREATED DATE     : 15-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :   This is the "C++" driver for :                */
/*                             - Ethernet                                    */
/*                             - TCP/IP                                      */
/*                             - DHCP                                        */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcEthernetCtrl_h
#define mpcEthernetCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <datapoint.h>
#include <Observer.h>
#include <SwTimerBassClass.h>
#include <SubTask.h>
#include <EventDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
struct ETHERNET_ADD_TYPE
{
  int ADD_0;
  int ADD_1;
  int ADD_2;
  int ADD_3;
};


typedef enum
{
  ETH_TEST_OK,
  ETH_TESTING,
  ETH_NOT_TESTED,
  ETH_NOT_TO_BE_TESTED,
  ETH_TEST_ERROR
}ETHERNET_LOOP_BACK_TEST_TYPE;



/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class EthernetCtrl : public SwTimerBaseClass, public SubTask
{

  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static EthernetCtrl* GetInstance();

    virtual void SubscribtionCancelled(Subject* pSubject) {};
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject);
    virtual void ConnectToSubjects();

//    virtual void update(SubjectIO* pSubjectIo);

    virtual void InitSubTask();
    virtual void RunSubTask();

    ETHERNET_LOOP_BACK_TEST_TYPE GetEthernetLoopBackTestStatus();
  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool IsEthernetLoopBackTestEnabled();
    void InitNormal();
    void InitLoopBackTest();

    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    EthernetCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~EthernetCtrl();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    static EthernetCtrl* mpInstance;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

};

#endif
