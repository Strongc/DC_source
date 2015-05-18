/** \file IPConfigErrorHandler.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPConfigErrorHandler.h
 *
 * $Log: IPConfigErrorHandler.h,v $
 * Revision 1.1.2.4  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.3  2005/05/26 09:25:57  jla
 * Added Networkload error
 *
 * Revision 1.1.2.2  2005/05/20 14:29:14  jla
 * Commited RTIP and Runtime errors
 *
 * Revision 1.1.2.1  2005/05/12 14:03:15  jla
 * Added class for handling errors in IP configuration software
 *
 */

#ifndef _IPCONFIGERRORHANDLER_H_
#define _IPCONFIGERRORHANDLER_H_

#include <string>
#include "SubjectIO.h"

using namespace std;

#define	IPCERRORBASE		0				//!<Start of numbering for IP configuration errors

#define IPCIPADDRESSINUSE	IPCERRORBASE+1	//!<Errornumber for trying to use IP address already in use.
#define IPCDHCPERROR		IPCERRORBASE+2	//!<Errornumber for DHCP error
#define	IPCTIMEOUT			IPCERRORBASE+3	//!<Errornumber for timeout error
#define IPCRUNTIMEERROR		IPCERRORBASE+4	//!<Errornumber for runtimeerror
#define	IPCRTIPERROR		IPCERRORBASE+5	//!<Errornumber for RTIP error
#define	IPCNETWORKLOAD		IPCERRORBASE+6	//!<Errornumber for excessive network load

#define	IPCIPADDRESSINUSE_TEXT	"Cannot set an IP address that is already in use."
#define IPCDHCPERROR_TEXT		"An error occured during comm. with DHCP server."
#define	IPCTIMEOUT_TEXT			"A general timeout error occured."
#define	IPCRUNTIMEERROR_TEXT	"A general runtime error occured."
#define	IPCRTIPERROR_TEXT		"An error occured in RTIP software."
#define IPCNETWORKLOAD_TEXT		"The network interface has been disabled due to excessive network traffic."

#define IPCNOERROR_TEXT			"No error."
#define IPCUNKNOWNERROR_TEXT	"Unknown error."

/**\brief Error handling
 *
 * This class takes care of errorhandling for the IP configuration software
 *
 * \todo Integrate with MPC errorhandling
 */
class IPConfigErrorHandler : public SubjectIO
{
public:
	virtual ~IPConfigErrorHandler();

	static IPConfigErrorHandler* getInstance();		//!<Get pointer to singleton instance

	void setErrorNo(int);							//!<Set error number when error occurs
	int getErrorNo();								//!<Return last error number set

	string& getErrorText();							//!<Get error text corresponding to error number.

	void setDetailedErrorText(const string&);		//!<Set detailed error description
	string& getDetailedErrorText();					//!<Get detailed error description

protected:
	IPConfigErrorHandler();

private:
	int merrorno;
	string merrortext;
	string mdetailederrortext;

	static IPConfigErrorHandler* mpinstance;
};

#endif //_IPCONFIGERRORHANDLER_H_
