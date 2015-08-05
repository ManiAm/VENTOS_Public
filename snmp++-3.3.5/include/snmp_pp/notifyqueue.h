/*_############################################################################
  _## 
  _##  notifyqueue.h  
  _##
  _##  SNMP++ v3.3
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##########################################################################*/
/*===================================================================

  Copyright (c) 1999
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.

      N O T I F Y Q U E U E. H

      CNotifyEventQueue CLASS DEFINITION

      COPYRIGHT HEWLETT PACKARD COMPANY 1999

      INFORMATION NETWORKS DIVISION

      NETWORK MANAGEMENT SECTION

      DESIGN + AUTHOR:        Tom Murray

      DESCRIPTION:
        Queue for holding sessions waiting for notifiactions

=====================================================================*/
// $Id: notifyqueue.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _NOTIFYQUEUE
#define _NOTIFYQUEUE

//----[ includes ]-----------------------------------------------------
#include <sys/types.h>          // NOTE: due to 10.10 bug, order is important
                                //   in that all routines must include types.h
                                //   and time.h in same order otherwise you
                                //   will get conflicting definitions of
                                //   "fd_set" resulting in link time errors.
#ifndef WIN32
#if !(defined CPU && CPU == PPC603)
#include <sys/time.h>	// time stuff and fd_set
#endif
#endif

//----[ snmp++ includes ]----------------------------------------------

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/oid.h"
#include "snmp_pp/target.h"
#include "snmp_pp/eventlist.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

class Snmp; // instead of snmp_pp.h
class msec;
class EventListHolder;

//----[ defines ]------------------------------------------------------

//----[ CNotifyEvent class ]-------------------------------------------

/*----------------------------------------------------------------*/
/* CNotifyEvent                                                   */
/*   a description of a sessions waiting for async notifiactions. */
/*----------------------------------------------------------------*/
class DLLOPT CNotifyEvent
{
 public:

  CNotifyEvent(Snmp* snmp,
	       const OidCollection &trapids,
	       const TargetCollection &targets);
  ~CNotifyEvent();
  Snmp * GetId() { return m_snmp; };
  int notify_filter(const Oid &trapid, SnmpTarget &target) const;
  int Callback(SnmpTarget &target, Pdu &pdu, SnmpSocket fd, int status);
  void get_filter(OidCollection &o, TargetCollection &t)
    { o = *notify_ids; t = *notify_targets; };

 protected:
  Snmp              *m_snmp;
  TargetCollection  *notify_targets;
  OidCollection     *notify_ids;
};

  /*-----------------------------------------------------------*/
  /* CNotifyEventQueue                                         */
  /*   class describing a collection of outstanding SNMP msgs. */
  /*-----------------------------------------------------------*/
class DLLOPT CNotifyEventQueue: public CEvents
{
  public:
    CNotifyEventQueue(EventListHolder *holder, Snmp *session);
    ~CNotifyEventQueue();
    int AddEntry(Snmp * snmp,
		 const OidCollection &trapids,
		 const TargetCollection &targets);
    CNotifyEvent * GetEntry(Snmp * snmp);
    void DeleteEntry(Snmp * snmp);

    // find the next timeout
    int GetNextTimeout(msec &/*timeout*/) { return 1; }; // we have no timeouts
    // set up parameters for select
#ifdef HAVE_POLL_SYSCALL
    int GetFdCount();
    bool GetFdArray(struct pollfd *readfds, int &remaining);
    int HandleEvents(const struct pollfd *readfds, const int fds);
#else
    void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
		   fd_set &exceptfds);
    int HandleEvents(const int maxfds,
                     const fd_set &readfds,
                     const fd_set &writefds,
                     const fd_set &exceptfds);
#endif
    // return number of outstanding messages
    int GetCount() { return m_msgCount; };

    int DoRetries(const msec &/*sendtime*/) { return 0; }; // nothing to retry

    int Done() { return 0; }; // we are never done
    void set_listen_port(int port) { m_listen_port = port; };
    int get_listen_port() { return m_listen_port; };
    SnmpSocket get_notify_fd() const;

  protected:

    /*-----------------------------------------------------------*/
    /* CNotifyEventQueueElt                                      */
    /*   a container for a single item on a linked lists of      */
    /*  CNotifyEvents.                                           */
    /*-----------------------------------------------------------*/
    class DLLOPT CNotifyEventQueueElt
    {
     public:
      CNotifyEventQueueElt(CNotifyEvent *notifyevent,
			   CNotifyEventQueueElt *next,
			   CNotifyEventQueueElt *previous);

      ~CNotifyEventQueueElt();
      CNotifyEventQueueElt *GetNext() { return m_Next; };
      CNotifyEvent *GetNotifyEvent() { return m_notifyevent; };
      CNotifyEvent *TestId(Snmp *snmp);

    private:

      CNotifyEvent *m_notifyevent;
      class CNotifyEventQueueElt *m_Next;
      class CNotifyEventQueueElt *m_previous;
    };

    void cleanup();

    CNotifyEventQueueElt m_head;
    int                  m_msgCount;
    SnmpSocket           m_notify_fd;
    int                  m_listen_port;
    EventListHolder *my_holder;
    Snmp *m_snmpSession;
    UdpAddress m_notify_addr;
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // NOTIFYQUEUE
