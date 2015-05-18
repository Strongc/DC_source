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
/* FILE NAME        : EthernetCtrl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 15-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SwTimer.h>
#include <AppTypeDefs.h>
#include <FactoryTypes.h>
#include <StringDataPoint.h>

#ifndef __PC__
#include <lowleveldelay.h>
#endif // __PC__

#include <ethernetloopback.h>
#include <gpio.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "EthernetCtrl.h"

#ifndef __PC__
#include <serv.h>
#endif // __PC__
/*****************************************************************************
  DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  HALF_LEASE_TIME_TIMER,
  LEASE_TIME_ENDED_TIMER
};
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
EthernetCtrl* EthernetCtrl::mpInstance = 0;
/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 ****************************************************************************/
EthernetCtrl* EthernetCtrl::GetInstance()
{
  if (!mpInstance)
  {
    mpInstance = new EthernetCtrl();
  }
  return mpInstance;
}

/*****************************************************************************
 * Function -  RunSubTask
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::RunSubTask()
{
	if (mFirstTimeToRun == true)
	{
		mFirstTimeToRun = false;
		IPConfiguration::getInstance()->attach(this);
    	PasswordConfiguration::getInstance()->attach(this);
		IPConfigErrorHandler::getInstance()->attach(this);

		IPConfiguration::getInstance()->setIPAddress(IPAddress(mpIpAdd->GetValue()));
		IPConfiguration::getInstance()->setSubnetMask(IPAddress(mpSubnetMask->GetValue()));
		IPConfiguration::getInstance()->setDefaultGateway(IPAddress(mpDefaultGateWay->GetValue()));
		IPConfiguration::getInstance()->setDHCPEnabled(mpDHCPEnable->GetValue());
    	auto string hostname(mpHostName->GetValue());
		IPConfiguration::getInstance()->setHostname(hostname);
    	auto string password(mpPasswordName->GetValue());
    	MPCWebServer::getInstance()->setPassword(password);

		IPConfiguration::getInstance()->notify();
	}

  if (mIpIoErrorUpdate)
  {
		mIpIoErrorUpdate = false;

		int errorno = IPConfigErrorHandler::getInstance()->getErrorNo();

		switch (errorno)
		{
			case 0:
                mpMisuseFault->SetValue(false);
                mpNoDHCPAddressFault->SetValue(false);
				break;
			case IPCDHCPERROR :
                mpNoDHCPAddressFault->SetValue(true);
				break;
			case IPCNETWORKLOAD :
                mpMisuseFault->SetValue(true);
				break;
			default:
				// other IP configuration errors are possible!!
				break;
		}
  }

	if ( mIpIoUpdate == true)
	{
		mIpIoUpdate = false;
		auto unsigned int ip_to_save;
		auto bool bool_to_save;

		ip_to_save = IPConfiguration::getInstance()->getIPAddress().getIPAddressMpc();
		mpIpAdd->SetValue(ip_to_save);
		ip_to_save = IPConfiguration::getInstance()->getSubnetMask().getIPAddressMpc();
		mpSubnetMask->SetValue(ip_to_save);
		ip_to_save = IPConfiguration::getInstance()->getDefaultGateway().getIPAddressMpc();
		mpDefaultGateWay->SetValue(ip_to_save);

		ip_to_save = IPConfiguration::getInstance()->getActualIPAddress().getIPAddressMpc();
		mpActIpAdd->SetValue(ip_to_save);
		ip_to_save = IPConfiguration::getInstance()->getActualSubnetMask().getIPAddressMpc();
		mpActSubnetMask->SetValue(ip_to_save);
		ip_to_save = IPConfiguration::getInstance()->getActualDefaultGateway().getIPAddressMpc();
		mpActDefaultGateWay->SetValue(ip_to_save);

		bool_to_save = IPConfiguration::getInstance()->getDHCPisEnabled();
		mpDHCPEnable->SetValue(bool_to_save);

		auto string hostname;
		IPConfiguration::getInstance()->getHostname(hostname);
		mpHostName->SetValue(hostname.c_str());

		auto string password;
		PasswordConfiguration::getInstance()->getPassword(password);
		mpPasswordName->SetValue(password.c_str());
  }

  if ( mUpdate == true)
  {
    mUpdate = false;

    auto IPAddress my_ip_addr(mpIpAdd->GetValue());
    auto IPAddress my_subnet_mask(mpSubnetMask->GetValue());
    auto IPAddress my_default_gateway(mpDefaultGateWay->GetValue());
    auto string hostname(mpHostName->GetValue());

    IPConfiguration::getInstance()->setNewIPConfig(mpDHCPEnable->GetValue(),
                                                   my_ip_addr,
                                                   my_subnet_mask,
                                                   my_default_gateway,
                                                   hostname);

    auto string password(mpPasswordName->GetValue());
    MPCWebServer::getInstance()->setPassword(password);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 ****************************************************************************/

void EthernetCtrl::update(SubjectIO* pSubjectIo)
{
	if (pSubjectIo == IPConfigErrorHandler::getInstance())
	{
		 mIpIoErrorUpdate = true;
	}
	else
	{
		mIpIoUpdate = true;
	}

	ReqTaskTime();
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::SetSubjectPointer(int Id,Subject* pSubject)
{
  switch(Id)
  {
  case SP_ETH_CTRL_MAC_ADDRESS_HI:
    mpMacAddHi.Attach(pSubject);
    break;
  case SP_ETH_CTRL_MAC_ADDRESS_LO:
    mpMacAddLo.Attach(pSubject);
    break;
	case SP_ETH_CTRL_DHCP_ENABLE:
    mpDHCPEnable.Attach(pSubject);
    break;
  case SP_ETH_CTRL_WEB_SERVER_PASSWORD:
    mpPasswordName.Attach(pSubject);
    break;
	case SP_ETH_CTRL_HOSTNAME:
    mpHostName.Attach(pSubject);
    break;
	case SP_ETH_CTRL_IP_ADDRESS:
    mpIpAdd.Attach(pSubject);
    break;
  case SP_ETH_CTRL_IP_ADDRESS_ACTUAL:
    mpActIpAdd.Attach(pSubject);
    break;
	case SP_ETH_CTRL_DEFAULT_GATEWAY:
    mpDefaultGateWay.Attach(pSubject);
    break;
  case SP_ETH_CTRL_DEFAULT_GATEWAY_ACTUAL:
    mpActDefaultGateWay.Attach(pSubject);
    break;
  case SP_ETH_CTRL_SUBNET_MASK:
    mpSubnetMask.Attach(pSubject);
    break;
  case SP_ETH_CTRL_SUBNET_MASK_ACTUAL:
    mpActSubnetMask.Attach(pSubject);
    break;
  case SP_ETH_CTRL_PRIMARY_DNS:
    mpPrimaryDNS.Attach(pSubject);
    break;
  case SP_ETH_CTRL_PRIMARY_DNS_ACTUAL:
    mpActPrimaryDNS.Attach(pSubject);
    break;
  case SP_ETH_CTRL_SECONDARY_DNS:
    mpSecondaryDNS.Attach(pSubject);
    break;
  case SP_ETH_CTRL_SECONDARY_DNS_ACTUAL:
    mpActSecondaryDNS.Attach(pSubject);
    break;

  case SP_ETH_CTRL_MISUSE_FAULT:
    mpMisuseFault.Attach(pSubject);
    break;
  case SP_ETH_CTRL_NO_DHCP_ADDRESS_FAULT:
    mpNoDHCPAddressFault.Attach(pSubject);
    break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::ConnectToSubjects()
{
  mpHostName->Subscribe(this);
  mpPasswordName->Subscribe(this);
  mpDHCPEnable->Subscribe(this);
  mpIpAdd->Subscribe(this);
  mpSubnetMask->Subscribe(this);
  mpDefaultGateWay->Subscribe(this);
  mpPrimaryDNS->Subscribe(this);
  mpSecondaryDNS->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::Update(Subject* pSubject)
{
  mUpdate = true;

  if (mpDHCPEnable.Update(pSubject))
  {
    if (mpDHCPEnable->GetValue())
    {
      mpActIpAdd->SetValue(0);
      mpActSubnetMask->SetValue(0);
      mpActDefaultGateWay->SetValue(0);
    }
  }

  ReqTaskTime();
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::InitSubTask()
{
	mFirstTimeToRun = true;

  if (IsEthernetLoopBackTestEnabled() == true)
  {
    mEthernetLoopBackTestStatus = ETH_TESTING;
    InitLoopBackTest();
  }
  else
  {
    mEthernetLoopBackTestStatus = ETH_NOT_TO_BE_TESTED;
    InitNormal();
  }

  mpMisuseFault->SetValue(false);
  mpNoDHCPAddressFault->SetValue(false);

  ReqTaskTime();
}

/*****************************************************************************
 * Function - InitNormal
 * DESCRIPTION:
 *            test
 ****************************************************************************/
void EthernetCtrl::InitLoopBackTest()
{
#ifndef __PC__

    int result = 0; // RESULT 0 = FALSE, 1 = OK, 2 = NO_CONECTION
    int i = 0;
    int buf_fill;

    GPio::GetInstance()->SetAs((PORT_NO)37, OUTPUT, 0);
    //init_gpio(37, FALSE, ON);

    GPio::GetInstance()->Toggle((PORT_NO)37);
    //toggle(37);

    for (i=0; i<1000000;i++ ) /// DELAY
        {
        i++;
        i--;
        }

    GPio::GetInstance()->Toggle((PORT_NO)37);
    //toggle(37);

    for (i=0; i<1000000;i++ ) /// DELAY OPRINDELIGT
        {
        i++;
        i--;
        }
    for (i=0; i<1000000;i++ ) /// DELAY OPRINDELIGT
        {
        i++;
        i--;
        }
    MACInit();
    for (i=0; i<1000000;i++ ) /// DELAY
        {
        i++;
        i--;
        }
    PHYInit();
    for (i=0; i<1000000;i++ ) /// DELAY
        {
        i++;
        i--;
        }
    writepkt_FFFF();
    for (i=0; i<1000000;i++ ) /// DELAY
        {
        i++;
        i--;
        }
    readpkt_FFFF();
    for (i=0; i<1000000;i++ ) /// DELAY
        {
        i++;
        i--;
        }
    flush_TX_buf();
    for (i=0; i<1000000;i++ ) /// DELAY
        {
        i++;
        i--;
        }
    flush_RX_buf();
    // ****************************************************************************
    // *  START TEST
    // **************************************************************************** /
    for ( int j = 0; j<4; j++)
    {
        check_buf();
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            flush_TX_buf();                 // TEST
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            flush_RX_buf();                 // TEST
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            check_buf();
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            writepkt_FFFF();
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            check_buf();
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            flush_TX_buf();                 // TEST
        for (i=0; i<10000;i++ ) /// DELAY
            {
            i++;
            i--;
            }
            buf_fill = check_buf();
            if (buf_fill == 3)
            {
                if (readpkt_FFFF() == 0)
                {
                  result = 1;
                }
                else
                {
                  mEthernetLoopBackTestStatus = ETH_TEST_ERROR;
                }
            }
            else if ( buf_fill == 4)
            {
                  mEthernetLoopBackTestStatus = ETH_TEST_ERROR;
            }
            else
            {
                  mEthernetLoopBackTestStatus = ETH_TEST_ERROR;
            }
    }

    if (mEthernetLoopBackTestStatus != ETH_TESTING)
    {
      mEthernetLoopBackTestStatus = ETH_TEST_ERROR;
    }
    else
    {
      mEthernetLoopBackTestStatus = ETH_TEST_OK;
    }

    if (result)
    {
	result++;
    }


#endif // __PC__
}

/*****************************************************************************
 * Function - GetEthernetLoopBackTestStatus
 * DESCRIPTION:
 *
 ****************************************************************************/
ETHERNET_LOOP_BACK_TEST_TYPE EthernetCtrl::GetEthernetLoopBackTestStatus()
{
  return mEthernetLoopBackTestStatus;
}
/*****************************************************************************
 * Function - InitNormal
 * DESCRIPTION:
 *
 ****************************************************************************/
void EthernetCtrl::InitNormal()
{

}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
bool EthernetCtrl::IsEthernetLoopBackTestEnabled()
{
  int temp;
  temp = (*(unsigned char *)0xB1000000);
  if ( temp == 0)
  {
    return false;
  }
  else
  {
      return true;
  }
  //return true;
}

void EthernetCtrl::webifGetMacAddress(std::ostream& res)
{
  char szMACAddress[20];

  EthernetCtrlGetMacAddrString_C(szMACAddress);

  res << szMACAddress;
}

/*****************************************************************************
 * Function - Constructor
 *****************************************************************************/
EthernetCtrl::EthernetCtrl()
{
  mEthernetLoopBackTestStatus = ETH_NOT_TESTED;
  mUpdate = true;                       // force update
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
EthernetCtrl::~EthernetCtrl()
{
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

void EthernetCtrlGetMacAddr(unsigned short *mac0, unsigned short *mac1, unsigned short *mac2)
{
  U32 macHi, macLo;
  EthernetCtrl* pEthernetCtrl = EthernetCtrl::GetInstance();

  macHi = pEthernetCtrl->mpMacAddHi->GetValue();
  macLo = pEthernetCtrl->mpMacAddLo->GetValue();

  *mac0 = (macHi & 0xFF00) | ((macHi & 0xFF0000) >> 16);
  *mac1 = ((macLo & 0xFF0000) >> 8) | (macHi & 0xFF);
  *mac2 = (macLo & 0xFF) << 8 | ((macLo & 0xFF00) >> 8);
}


extern "C" void EthernetCtrlGetMacAddr_C(unsigned short *mac0, unsigned short *mac1, unsigned short *mac2)
{
	EthernetCtrlGetMacAddr(mac0, mac1, mac2);
}

extern "C" void EthernetCtrlGetMacAddrString_C(char* szMACAddress)
{
  unsigned short mac0, mac1, mac2;

  EthernetCtrlGetMacAddr(&mac0, &mac1, &mac2);

  sprintf(szMACAddress, "%04X%04X%04X", mac0, mac1, mac2);
}

