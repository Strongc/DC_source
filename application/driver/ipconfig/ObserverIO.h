/** \file ObserverIO.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: ObserverIO.h
 *
 * $Log: ObserverIO.h,v $
 * Revision 1.1.2.2  2005/05/27 13:23:51  jla
 * Finished documentation
 *
 * Revision 1.1.2.1  2005/05/03 15:56:42  jla
 * *** empty log message ***
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#ifndef _OBSERVERIO_H_
#define _OBSERVERIO_H_

class SubjectIO;

/**\brief Abstract base class for observer in observer pattern.
 *
 * Inherit from this class to implement an observer.
 * The subclass must implement the update method.
 * 
 * \note To integrate with the rest of the MPC software, you could make this a subclass of Observer
 */
class ObserverIO
{
public:
    ObserverIO();
    virtual ~ObserverIO();

    virtual void update(SubjectIO *subject)=0;      //!<Pure virtual update method to be implemented by concrete observers
};

#endif //_OBSERVER_H_
