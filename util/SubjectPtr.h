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
/* CLASS NAME       : SubjectPtr                                            */
/*                                                                          */
/* FILE NAME        : SubjectPtr.h                                          */
/*                                                                          */
/* CREATED DATE     : 18-04-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Implementation of Observer pattern              */
/****************************************************************************/

/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __SUBJECT_PTR_H__
#define __SUBJECT_PTR_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <factory.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ISubject.h>
#include <Subject.h>
#include <Observer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: SubjectPtr
 * DESCRIPTION: Subject pointer helper
 *****************************************************************************/
template<typename SUBJECT_PTR_TYPE>
class SubjectPtr
{
public:

	/********************************************************************
	LIFECYCLE - Default constructor.
	********************************************************************/
	SubjectPtr()
	{
		m_pSubject = NULL;
		m_updated = false;
	}

	/********************************************************************
	LIFECYCLE - Destructor.
	********************************************************************/
	virtual ~SubjectPtr(void) { /* nop */ }

	/*****************************************************************************
	 * FUNCTION...: IsValid
	 * DESCRIPTION: Returns true if a subject pointer has been attached
	 *****************************************************************************/
  bool IsValid()
  {
    return m_pSubject ? true : false;
  }

	/*****************************************************************************
	 * FUNCTION...: Attach
	 * DESCRIPTION: Attaches the specified subject pointer with type verification.
	 *              Returns true on success.
	 *****************************************************************************/
  bool Attach(Subject* pSubject)
  {
   	bool rc = false;

		if (m_pSubject == NULL)
		{
			// cast dynamically to verify type
			m_pSubject = dynamic_cast<SUBJECT_PTR_TYPE>(pSubject);

      // success?
      if (m_pSubject)
      {
        // ok, increment reference count and set rc
        pSubject->IncRefCount();
        rc = true;
      }
		}
    else
    {
      FatalErrorOccured("Subject attached twice!");
    }

    return rc;
  }

	/*****************************************************************************
	 * FUNCTION...: Detach
	 * DESCRIPTION: Detaches the attached subject pointer
	 *						Returns true if a subject pointer was attached
	 *****************************************************************************/
  bool Detach(void)
  {
   	bool rc = false;

		if (m_pSubject)
		{
			m_pSubject = NULL;
			m_updated = false;
			rc = true;
		}

    return rc;
  }

	/*****************************************************************************
	 * FUNCTION...: Detach
	 * DESCRIPTION: Detaches the specified subject pointer if equal to the attached
	 *              subject pointer
	 *						Returns true if the specified subject pointer was equal to the
   *              attached subject pointer
   *****************************************************************************/
  bool Detach(ISubject* pSubject)
  {
   	bool rc = false;

		if (m_pSubject == pSubject)
		{
			m_pSubject = NULL;
			m_updated = false;
			rc = true;
		}

    return rc;
  }

	/*****************************************************************************
	 * FUNCTION...: Equals
	 * DESCRIPTION: Returns true if the attached subject pointer equals the
	 *              specified subject pointer
	 *****************************************************************************/
  bool Equals(ISubject* pSubject)
  {
    return m_pSubject == pSubject ? true : false;
  }

	/*****************************************************************************
	 * FUNCTION...: Subscribe
	 * DESCRIPTION: Calls Subscribe on the attached subject pointer.
	 *              Returns true on success, false if no subject pointer has been
	 *              attached
	 *****************************************************************************/
  bool Subscribe(Observer* pObserver)
  {
    if (m_pSubject)
    {
      m_pSubject->Subscribe(pObserver);
			return true;
    }
    else
    {
			return false;
    }
  }

	/*****************************************************************************
	 * FUNCTION...: Unsubscribe
	 * DESCRIPTION: Calls Unsubscribe on the attached subject pointer.
	 *              Returns true on success, false if no subject pointer has been
	 *              attached
	 *****************************************************************************/
  bool Unsubscribe(Observer* pObserver)
  {
    if (m_pSubject)
    {
      m_pSubject->Unsubscribe(pObserver);
			return true;
    }
    else
    {
			return false;
    }
  }

	/*****************************************************************************
	 * FUNCTION...: UnsubscribeAndDetach
	 * DESCRIPTION: Calls Unsubscribe on the attached subject pointer and detaches
   *              the subject pointer.
	 *              Returns true on success, false if no subject pointer has been
	 *              attached
	 *****************************************************************************/
  bool UnsubscribeAndDetach(Observer* pObserver)
  {
    if (m_pSubject)
    {
      m_pSubject->Unsubscribe(pObserver);
      m_pSubject = NULL;
      m_updated = false;
			return true;
    }
    else
    {
			return false;
    }
  }

	/*****************************************************************************
	 * FUNCTION...: GetSubject
	 * DESCRIPTION: Returns the attached subject pointer or NULL if no subject
	 *              pointer has been attached
	 *****************************************************************************/
	SUBJECT_PTR_TYPE GetSubject(void)
	{
    if (m_pSubject)
    {
      return m_pSubject;
    }
    else
    {
      return NULL;
    }
	}

	/*****************************************************************************
	 * OPERATOR...: ->
	 * DESCRIPTION: Access to the attached subject pointer
	 *****************************************************************************/
 	SUBJECT_PTR_TYPE operator -> ()
	{
    if (m_pSubject)
    {
      return m_pSubject;
    }
    else
    {
      FatalErrorOccured("Subject not attached!");
      return NULL;  // We don't have a subject pointer so all we can return is a NULL-pointer
    }
	}

	/*****************************************************************************
	 * FUNCTION...: Update
	 * DESCRIPTION: If the specified subject matches the attached subject
	 *              the internal updated flag is set to true, hence
	 *              IsUpdated will return true.
	 *              Returns true if the internal updated flag was set to true.
	 *****************************************************************************/
  bool Update(ISubject* pSubject)
  {
		if (Equals(pSubject))
		{
			m_updated = true;
			return true;
		}

    return false;
  }

	/*****************************************************************************
	 * FUNCTION...: IsUpdated
	 * DESCRIPTION: Returns true if the subject has been updated (Update
	                called).
	 *              By default the updated flag is reset unless the reset argument
	 *              is set to false.
	 *****************************************************************************/
	bool IsUpdated(bool reset = true)
	{
 		bool rc = false;

    if (m_pSubject)
    {
      rc = m_updated;

  		if (reset)
  		{
  			m_updated = false;
  		}
    }
    else
    {
      m_updated = false;
    }

 		return rc;
	}

	/*****************************************************************************
	 * FUNCTION...: ResetUpdated
	 * DESCRIPTION: Resets the internal updated flag
	 *****************************************************************************/
	void ResetUpdated(void)
  {
    m_updated = false;
  }

	/*****************************************************************************
	 * FUNCTION...: SetUpdated
	 * DESCRIPTION: Sets the internal updated flag (can be used to force update)
	 *****************************************************************************/
	void SetUpdated(void)
  {
    m_updated = true;
  }

  /********************************************************************
   ATTRIBUTES
	 ********************************************************************/
private:
	SUBJECT_PTR_TYPE m_pSubject;
	bool m_updated;
};

#endif
