/** \file IPConfigErrorHandler.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: IPConfigErrorHandler.cpp
 *
 * $Log: IPConfigErrorHandler.cpp,v $
 * Revision 1.1.2.3  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.2  2005/05/20 14:29:14  jla
 * Commited RTIP and Runtime errors
 *
 * Revision 1.1.2.1  2005/05/12 14:03:15  jla
 * Added class for handling errors in IP configuration software
 *
 */

#include "IPConfigErrorHandler.h"
#include "IPConfiguration.h"

IPConfigErrorHandler* IPConfigErrorHandler::mpinstance = 0;

IPConfigErrorHandler::IPConfigErrorHandler()
 : merrorno(0)
{
}

IPConfigErrorHandler::~IPConfigErrorHandler()
{
}

IPConfigErrorHandler* IPConfigErrorHandler::getInstance()
{
	if (IPConfigErrorHandler::mpinstance == 0)
		IPConfigErrorHandler::mpinstance = new IPConfigErrorHandler;

	return IPConfigErrorHandler::mpinstance;
}

/**This method will also delete the detailed error text set with
 * setDetailedErrorText.
 *
 * \param error The error number.
 */
void IPConfigErrorHandler::setErrorNo(int error)
{
	merrorno = error;
	mdetailederrortext = "";

	notify();

}

/**
 *
 * \return The error number.
 */
int IPConfigErrorHandler::getErrorNo()
{
	int retval = merrorno;
	//merrorno = 0;
	return retval;
}

/**This method should be called before calling getErrorNo, as getErrorNo
 * resets the error number to 0.
 *
 * \return Reference to string containing the error text.
 */
string& IPConfigErrorHandler::getErrorText()
{
	switch (merrorno)
	{
		case 0:
		merrortext = IPCNOERROR_TEXT;
		break;

		case IPCIPADDRESSINUSE:
		merrortext = IPCIPADDRESSINUSE_TEXT;
		break;

		case IPCDHCPERROR:
		merrortext = IPCDHCPERROR_TEXT;
		break;

		case IPCTIMEOUT:
		merrortext = IPCTIMEOUT_TEXT;
		break;

		case IPCRUNTIMEERROR:
		merrortext = IPCRUNTIMEERROR_TEXT;
		break;

		case IPCRTIPERROR:
		merrortext = IPCRTIPERROR_TEXT;
		break;

		default:
		merrortext = IPCUNKNOWNERROR_TEXT;
		break;
	}

	return merrortext;
}

/**This method could be called to set a text that describes the
 * error in detail.
 *
 * \param s Reference to string containing error description.
 */
void IPConfigErrorHandler::setDetailedErrorText(const string& s)
{
	mdetailederrortext = s;
}

/**
 * \return Reference to string conaining detailed error description.
 */
string& IPConfigErrorHandler::getDetailedErrorText()
{
	return mdetailederrortext;
}

