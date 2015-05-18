/** \file SubjectIO.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: SubjectIO.h
 *
 * $Log: SubjectIO.h,v $
 * Revision 1.1.2.2  2005/05/27 13:23:52  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/03 15:56:42  jla
 * *** empty log message ***
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#ifndef _SUBJECTIO_H_
#define _SUBJECTIO_H_

#include <list>
#include <algorithm>
#include "ObserverIO.h"

using namespace std;

/**\brief Abstract baseclass for subject in observer pattern.
 * 
 * An STL list is used for keeping track of observers.
 *
 * \note To integrate with the rest of the MPC software, you could make this a subclass of Subject
 */
class SubjectIO
{
public:
    void attach(ObserverIO *observer);          //!<Attach an observer to notify list
    void detach(ObserverIO *observer);          //!<Detach an observer from notify list
	void notify();								//!<Notifies all observers after state change

protected:
    SubjectIO();                                 //!<Constructor is protected to make class abstract
    virtual ~SubjectIO();

private:
    list<ObserverIO*> mobserverlist;             //!<List containing all observers to notify of updates
};

#endif //_SUBJECT_H_
