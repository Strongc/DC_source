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
/* FILE NAME        : IEthernetCtrlImpl.h                                   */
/*                                                                          */
/* CREATED DATE     : 01-03-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :   This is the "C++" driver for :                */
/*                             - Ethernet                                    */
/*                             - TCP/IP                                      */
/*                             - DHCP                                        */
/****************************************************************************/

#ifndef mpcIEthernetCtrlImpl_h
#define mpcIEthernetCtrlImpl_h

#include <Observer.h>


typedef enum
{
  ETH_TEST_OK,
  ETH_TESTING,
  ETH_NOT_TESTED,
  ETH_NOT_TO_BE_TESTED,
  ETH_TEST_ERROR
}ETHERNET_LOOP_BACK_TEST_TYPE;


class IEthernetCtrlImpl
{
  public:
    virtual ~IEthernetCtrlImpl() {}
    virtual void SubscribtionCancelled(Subject* pSubject)    = 0;
    virtual void Update(Subject* pSubject)                   = 0;
    virtual void SetSubjectPointer(int Id,Subject* pSubject) = 0;
    virtual void ConnectToSubjects()                         = 0;

    virtual void InitSubTask()                               = 0;
    virtual void RunSubTask()                                = 0;

    virtual ETHERNET_LOOP_BACK_TEST_TYPE GetEthernetLoopBackTestStatus() = 0; 
};
#endif
