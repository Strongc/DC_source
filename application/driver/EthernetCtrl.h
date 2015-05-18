/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : EthernetCtrl                                          */
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
#ifndef __ETHERNET_CTRL_H__
#define __ETHERNET_CTRL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AppTypeDefs.h>
#include <AlarmDef.h>
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EventDataPoint.h>
#include <AlarmDataPoint.h>
#include <StringDataPoint.h>

#include "ipconfig/IPConfiguration.h"
#include "ipconfig/PasswordConfiguration.h"
#include "ipconfig/IPConfigErrorHandler.h"

#include "GUI_VNC.h"

// Ethernet Include
#ifndef __PC__
#include <rtipapi.h>
#include <Ethernet_hwc.h>
#endif // __PC__

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
  ETH_TEST_OK           = 0,
  ETH_TEST_ERROR        = 1,
  ETH_TESTING           = 2,
  ETH_NOT_TESTED        = 3,
  ETH_NOT_TO_BE_TESTED  = 3
}ETHERNET_LOOP_BACK_TEST_TYPE;


extern "C" void EthernetCtrlGetMacAddrString_C(char* szMACAddress);

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class EthernetCtrl : public SwTimerBaseClass, public SubTask, public ObserverIO
{
	friend void EthernetCtrlGetMacAddr(unsigned short *mac0, unsigned short *mac1, unsigned short *mac2);

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

    virtual void update(SubjectIO* pSubjectIo);

    virtual void InitSubTask();
    virtual void RunSubTask();

    ETHERNET_LOOP_BACK_TEST_TYPE GetEthernetLoopBackTestStatus();

    // methods called by the WebIfHandler
    void webifGetMacAddress(std::ostream& res);

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
    SubjectPtr<BoolDataPoint*> mpEthernetConfiguredCorrect;

    SubjectPtr<BoolDataPoint*> mpDHCPEnable;

    SubjectPtr<U32DataPoint*> mpIpAdd;
    SubjectPtr<U32DataPoint*> mpActIpAdd;

   	SubjectPtr<U32DataPoint*> mpDefaultGateWay;
   	SubjectPtr<U32DataPoint*> mpActDefaultGateWay;

    SubjectPtr<U32DataPoint*> mpSubnetMask;
    SubjectPtr<U32DataPoint*> mpActSubnetMask;

    SubjectPtr<U32DataPoint*> mpPrimaryDNS;
    SubjectPtr<U32DataPoint*> mpActPrimaryDNS;

    SubjectPtr<U32DataPoint*> mpSecondaryDNS;
    SubjectPtr<U32DataPoint*> mpActSecondaryDNS;

    SubjectPtr<U32DataPoint*> mpMacAddHi;
    SubjectPtr<U32DataPoint*> mpMacAddLo;

    SubjectPtr<StringDataPoint*> mpHostName;
    SubjectPtr<StringDataPoint*> mpPasswordName;

    SubjectPtr<BoolDataPoint*> mpNoDHCPAddressFault;
    SubjectPtr<BoolDataPoint*> mpMisuseFault;

    bool mFirstTimeToRun;
    bool mIpIoUpdate;
    bool mIpIoErrorUpdate;
    bool mUpdate;
    int mInterfaceNo;

    ETHERNET_LOOP_BACK_TEST_TYPE mEthernetLoopBackTestStatus;

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
