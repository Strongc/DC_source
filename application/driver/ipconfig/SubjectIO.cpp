/** \file SubjectIO.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: SubjectIO.cpp
 *
 * $Log: SubjectIO.cpp,v $
 * Revision 1.1.2.2  2005/05/09 13:18:11  jla
 * Removed TRACE
 *
 * Revision 1.1.2.1  2005/05/03 15:56:42  jla
 * *** empty log message ***
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#define TRACE(x)

#include <iostream>

#include "SubjectIO.h"

SubjectIO::SubjectIO()
{
	TRACE(cout << "Subject Constructor called" << endl;)
}

SubjectIO::~SubjectIO()
{
	TRACE(cout << "Subject Destructor called" << endl;)
}

void SubjectIO::attach(ObserverIO *observer)
{
	if (observer)
	{
		TRACE(cout << "Observer attached." << endl;)
		mobserverlist.push_back(observer);
	}
}

void SubjectIO::detach(ObserverIO *observer)
{
	TRACE(cout << "Observer removed." << endl;)
	remove(mobserverlist.begin(), mobserverlist.end(), observer);
}

void SubjectIO::notify()
{
	TRACE(cout << "Subject notify method." << endl;)

    list<ObserverIO*>::iterator i;

	for (i=mobserverlist.begin() ; i!=mobserverlist.end(); i++)
	{
		(*i)->update(this);
	}
}



