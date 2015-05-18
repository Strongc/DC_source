/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : EventList                                             */
/*                                                                          */
/* FILE NAME        : EventList.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 08-07-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <EventList.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              Class EventList functions
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION: Clear alle events in list
 *****************************************************************************/
EventList::EventList()
{
  int i;

  mNoOfEvents = 0;
  for(i=0; i<MAX_ELEMENTS; i++)
  {
    mEvents[i] = NULL;              // assign the NULL pointer to all elements, this is done to ensure that the
  }                                 // destructer not tries to delete an object that does not exist
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: Clear eventlist and free memory
 ****************************************************************************/
EventList::~EventList()
{
  int i;

  for(i=0; i<mNoOfEvents; i++)
  {
    if(mEvents[i]!=NULL)              // Dont delete if the pointer not point at an object
    {
      delete mEvents[i];
      mEvents[i] = NULL;
    }
  }
}

/*****************************************************************************
 * Function - SortEvents
 * DESCRIPTION: Sort all events according to time
 *****************************************************************************/
void EventList::SortEvents(void)
{
  int i,j;
  EventItem *temp_event;

  for(j=0; j<mNoOfEvents-1; j++)
  {
    for(i=j+1; i<mNoOfEvents; i++)
    {
      if(*mEvents[j]>*mEvents[i])
      {
        temp_event = mEvents[j];
        mEvents[j] = mEvents[i];
        mEvents[i] = temp_event;
      }
    }
  }
}


/*****************************************************************************
 * Function - AddEvent
 * DESCRIPTION: Add new event to the end of the list
 *****************************************************************************/
void EventList::AddEvent(int time, int value)
{
  if( mNoOfEvents>=MAX_ELEMENTS )
  {
    FatalErrorOccured("EventList, too many events");
  }
  else
  {
    mEvents[mNoOfEvents++] = new EventItem( time, value);
  }
}

/*****************************************************************************
 * Function - GetEvent
 * DESCRIPTION: Returns the event at the specified position
 *****************************************************************************/
int EventList::GetEvent(int index, int *time, int *value)
{
  int status = 0;

  if( (index>=0) && (index<mNoOfEvents) )
  {
    if( mEvents[index]!=NULL )
    {
      *time = mEvents[index]->time;
      *value = mEvents[index]->value;
    }
    else
    {
      status = 1;
    }
  }
  else
  {
    status = 2;
  }
  return (status);
}

/*****************************************************************************
 * Function - QueryEvents
 * DESCRIPTION: Returns the events in the time span given
 *****************************************************************************/
int EventList::QueryEvents(int startTime, int stopTime, int * start, int * count)
{
  int i;
  int stop;
  int stop1;
  int status = 0;

  i = 0;
  stop = false;
  stop1 = false;
  if((startTime>stopTime)&&(startTime>0))
  {
    status=1;
  }
  if( (mNoOfEvents>0) && (status==0) )
  {
    while( (mEvents[i]->time<startTime) && (stop==false) )
    {
      if( i>(mNoOfEvents-2) )           // there is no items in the interval
      {                                 // the start time is greater than the highest value in the interval
        stop = true;
      }
      else
      {
        i++;
      }
    }
    *start = i;                             // write the index of the first item in the interval
    while( (mEvents[i]->time<=stopTime) && (stop==false) && (stop1==false) )
    {
      if( i > (mNoOfEvents-2) )                // there is no items higher than the stop time in the interval
      {
        stop1 = true;
      }
      else
      {
        i++;
      }
    }
    if(stop1==true)
    {
      i++;
    }
    if(stop==true)
    {
      *count=0;                           // there is no items in the interval
    }
    else
    {
      *count=i-*start;
    }
  }
  else
  {
    *count=0;
  }
  return(status);
}

/*****************************************************************************
 * Function - GetTimeForNextEvent
 * DESCRIPTION: Returns the time for the next event
 *****************************************************************************/
int EventList::GetTimeForNextEvent(int time)
{
  int i;
  int time_next_event;

  time_next_event = -1;
  i = 0;
  while( i<mNoOfEvents )
  {
    if(mEvents[i]->time > time)
    {
      time_next_event = mEvents[i]->time;
      break;
    }
    i++;
  }
  return time_next_event;
}

/*****************************************************************************
 * Function - GetValue
 * DESCRIPTION: Returns the value for the given time
 *****************************************************************************/
int EventList::GetValue(int time)
{
  int my_value;
  int i;
  int my_time;
  bool run;

  i = 0;
  my_value = -1;
  if(!mNoOfEvents)
  {
    run=false;
  }
  else
  {
    run=true;
  }
  while(run)
  {
    my_time = mEvents[i]->time;
    if( my_time<=time )
    {
      my_value = mEvents[i]->value;
    }
    else
    {
      run=false;
    }
    i++;
    if( i>=mNoOfEvents )
    {
      run = false;
    }
  }
  return my_value;
}


/*****************************************************************************
 *
 *
 *              Class EventItem functions
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
EventItem::EventItem(int aTime, int aValue)
{
  time = aTime;
  value = aValue;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
EventItem::~EventItem()
{
}

/*****************************************************************************
 * Function - Operator: ">"
 * DESCRIPTION:
 *****************************************************************************/
bool EventItem::operator > (EventItem& event)
{
  if(time>event.time)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 * Function - Operator: "<"
 * DESCRIPTION:
 *****************************************************************************/
bool EventItem::operator < (EventItem& event)
{
  if(time<event.time)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 * Function - Operator: "<="
 * DESCRIPTION:
 *****************************************************************************/
bool EventItem::operator <= (EventItem& event)
{
  if(time<=event.time)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 * Function - Operator: ">="
 * DESCRIPTION:
 *****************************************************************************/
bool EventItem::operator >= (EventItem& event)
{
  if(time>=event.time)
  {
    return true;
  }
  else
  {
    return false;
  }
}
