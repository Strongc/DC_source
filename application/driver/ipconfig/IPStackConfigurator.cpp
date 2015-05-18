/** \file IPStackConfigurator.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPStackConfigurator.cpp
 *
 * $Log: IPStackConfigurator.cpp,v $
 * Revision 1.2.2.6  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.2.2.5  2005/05/20 14:31:22  jla
 * Now a singleton. Added refreshIPConfig method.
 *
 * Revision 1.2.2.4  2005/05/14 10:22:45  jla
 * Added dhcp renew when hostname is changed
 *
 * Revision 1.2.2.3  2005/05/12 14:06:17  jla
 * Removed checking whether DHCP is already enabled
 *
 * Revision 1.2.2.2  2005/05/09 13:18:11  jla
 * Removed TRACE
 *
 * Revision 1.2.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#include "IPStackConfigurator.h"

IPStackConfigurator* IPStackConfigurator::mpinstance = 0;

IPStackConfigurator::IPStackConfigurator()
 :  mupdating(false),
  mpipconfiguration(0),
  mpdhcpclient(0),
  mpipstackconfif(0)
{
}

IPStackConfigurator::~IPStackConfigurator()
{
  mpipconfiguration->detach(this);        //Detach observer from subject
}

IPStackConfigurator* IPStackConfigurator::getInstance()
{
  if (mpinstance == 0)
  {
    mpinstance = new IPStackConfigurator;
  }

  return mpinstance;
}

/**Called by IPConfiguration when IP configuration (IP address etc.) is changed.
 *
 * If the hostname is changed, and the DHCP client is already enabled, a renew
 * of the lease is requested. This will make the DHCP client send the new hostname
 * to the DHCP server.
 *
 * \param subject Pointer to subject object.
 */
void IPStackConfigurator::update(SubjectIO *subject)
{
  if (subject == mpipconfiguration && !mupdating) //Test mupdating to avoid infinite recursive call
  {
    mupdating = true;

    if (mpipconfiguration->getDHCPisEnabled())
    {
      string hostname;
      mpipconfiguration->getHostname(hostname);
      if (hostname != mpdhcpclient->getHostname())
      {
          mpdhcpclient->setHostname(hostname);     
          mpdhcpclient->renew();      //do renew to update hostname on dhcp server
      }

      if ( !mpdhcpclient->isEnabled() )
      { 
          mpdhcpclient->enable();
          mpdhcpclient->renew();      //do renew when dhcp server is switched on
      }

    }
    else    //DHCP disabled
    {
      if (mpdhcpclient->isEnabled())
      {
          mpdhcpclient->disable();
      }

      mpipstackconfif->setIPAddress(mpipconfiguration->getIPAddress());
      mpipstackconfif->setSubnetMask(mpipconfiguration->getSubnetMask());
      mpipstackconfif->setDefaultGateway(mpipconfiguration->getDefaultGateway());

      mpipstackconfif->setDNSServers(
        mpipconfiguration->getActualPrimaryDNS(),
        mpipconfiguration->getActualSecondaryDNS());

      mpipconfiguration->setActualIPAddress(mpipconfiguration->getIPAddress());
      mpipconfiguration->setActualSubnetMask(mpipconfiguration->getSubnetMask());
      mpipconfiguration->setActualDefaultGateway(mpipconfiguration->getDefaultGateway());
      mpipconfiguration->setActualPrimaryDNS(mpipconfiguration->getPrimaryDNS());
      mpipconfiguration->setActualSecondaryDNS(mpipconfiguration->getSecondaryDNS());

      mpipconfiguration->notify();
    }

    mupdating = false;
  }
}

/**This method must be called by the DHCP client, when it receives a new
 * configuration from the DHCP server.
 *
 * A new DHCP configuration is ignored, if IP configuration is being updated
 * from update method.
 *
 * \param ipaddress New IP address from DHCP server.
 * \param subnetmask New subnet mask from DHCP server.
 * \param defaultgateway New default gateway from DHCP server.
 * \param primarydns New primary DNS from DHCP server.
 * \param secondarydns New secondary DNS from DHCP server.
 */
void IPStackConfigurator::newDHCPConfiguration( IPAddress ipaddress,
                        IPAddress subnetmask,
                        IPAddress defaultgateway,
                        IPAddress primarydns,
                        IPAddress secondarydns) //!<Set new configuration from DHCP
{
  if (!mupdating)
  {
    mpipstackconfif->setIPAddress(ipaddress);
    mpipstackconfif->setSubnetMask(subnetmask);
    mpipstackconfif->setDefaultGateway(defaultgateway);
    mpipstackconfif->setDNSServers(primarydns,secondarydns);

    mpipconfiguration->setActualIPAddress(ipaddress);
    mpipconfiguration->setActualSubnetMask(subnetmask);
    mpipconfiguration->setActualDefaultGateway(defaultgateway);
    mpipconfiguration->setActualPrimaryDNS(primarydns);
    mpipconfiguration->setActualSecondaryDNS(secondarydns);
    
    mpipconfiguration->notify();          //Update other observers
  }
}

/**Makes IPStackConfigurationInterface update the IP address, subnet mask
 * and default gateway.
 */
void IPStackConfigurator::refreshIPConfig()
{
  mpipstackconfif->setIPAddress(mpipconfiguration->getActualIPAddress());
  mpipstackconfif->setSubnetMask(mpipconfiguration->getActualSubnetMask());
  mpipstackconfif->setDefaultGateway(mpipconfiguration->getActualDefaultGateway());
  mpipstackconfif->setDNSServers(
    mpipconfiguration->getActualPrimaryDNS(),
    mpipconfiguration->getActualSecondaryDNS());

}

/**Set pointers to IPConfiguration, DHCPClient and IPStackConfigurationInterface
 * objects. The IPStackConfigurator (observer class) is also attached to the
 * IPConfiguration (subject class).
 *
 * \param pipconfiguration Pointer to IPConfiguration object.
 * \param pdhcpclient Pointer to DHCPClient object.
 * \param pipstackconfif Pointer to IPStackConfigurationInterface object.
 */
void IPStackConfigurator::setObjectPtrs(
            IPConfiguration *pipconfiguration,
            DHCPClient *pdhcpclient,
            IPStackConfigurationInterface *pipstackconfif)
{
  if (mpipconfiguration)
  {
    mpipconfiguration->detach(this);      //Detach from old subject
  }
  mpipconfiguration = pipconfiguration;
  mpipconfiguration->attach(this);        //Attach observer to subject

  mpdhcpclient = pdhcpclient;
  mpipstackconfif = pipstackconfif;

}
