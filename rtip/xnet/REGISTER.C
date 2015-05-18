#include "sock.h"

#if (!BUILD_NEW_BINARY)
#include "rtipapi.h"

/* ********************************************************************   */
/* INITIALIZE ADD-ONS                                                     */
/* ********************************************************************   */
/* register_add_ons() - register initialization routines                  */
/*                                                                        */
/*   Registers initialization routines so the various add-ons             */
/*   will be initialized by xn_rtip_init() and xn_rtip_reset();           */
/*   this needs to be called before xn_rtip_init()                        */
/*                                                                        */
void register_add_ons(void)
{
#if (INCLUDE_SNMP)
    XN_REGISTER_SNMP()
#endif      /* end of if SNMP */

#if (INCLUDE_DHCP_CLI)
    XN_REGISTER_DHCP_CLI()
#endif

#if (INCLUDE_FTP_SRV)
    XN_REGISTER_FTP_SRV()
#endif

#if (INCLUDE_WEB)
    XN_REGISTER_WEB_SRV()
#endif

#if (INCLUDE_TELNET_SRV)
    XN_REGISTER_TELNET_SRV()
#endif

    /*  **********************************************************    */
    /* DEVICE DRIVE                                                   */

#if (INCLUDE_PKT)
    XN_REGISTER_PKT()
#endif

#if (INCLUDE_LOOP)
    XN_REGISTER_LOOPBACK()
#endif

}

#endif /* !BUILD_NEW_BINARY */


