/*_############################################################################
  _## 
  _##  msec.h  
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
/*
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
*/
// $Id: msec.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _MSEC_H_
#define _MSEC_H_

//----[ includes ]-----------------------------------------------------
#include <sys/types.h> /* NOTE: due to 10.10 bug, order is important
			* in that all routines must include types.h
			* and time.h in same order otherwise you will
			* get conflicting definitions of "fd_set"
			* resulting in link time errors.
			*/
#ifdef WIN32
#elif defined (CPU) && CPU == PPC603
#include <sys/times.h>
#else
#include <sys/time.h>
#include <sys/param.h>
#endif

#include <time.h>

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/smi.h"
#include "snmp_pp/reentrant.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

//----[ defines ]------------------------------------------------------
#define MSECOUTBUF 20

//----[ msec class ]---------------------------------------------------
/**
 * Time handling...
 */
class DLLOPT msec
{
 public:
  /**
   * Constructor, sets the time to the current system time.
   */
  msec() { refresh(); };

  /**
   * Constructor using another msec object
   *
   * @param in_msec - Time for this object
   */
  msec(const msec &in_msec) : m_time(in_msec.m_time), m_changed(true) {};

  /**
   * Constructor using seconds and milli sconds.
   *
   * @param sec    - Seconds
   * @param milsec - Milli seconds
   */
  msec(const int sec, const int milsec) : m_changed(true)
    { m_time.tv_sec  = sec; m_time.tv_usec = milsec; };

  DLLOPT friend int operator==(const msec &t1, const msec &t2);
  DLLOPT friend int operator!=(const msec &t1, const msec &t2);
  DLLOPT friend int operator<(const msec &t1, const msec &t2);
  DLLOPT friend int operator>(const msec &t1, const msec &t2);
  DLLOPT friend int operator<=(const msec &t1, const msec &t2)
    { return((t1 < t2) || (t1 == t2)); };
  DLLOPT friend int operator>=(const msec &t1, const msec &t2)
    { return((t1 > t2) || (t1 == t2)); };

  msec &operator-=(const long millisec);
  msec &operator-=(const timeval &t1);
  msec &operator+=(const long millisec);
  msec &operator+=(const timeval &t1);
  msec &operator=(const msec &t)
    { m_time = t.m_time; m_changed = true; return *this; };
  msec &operator=(const timeval &t1);

  /**
   * Use as an unsigned long.
   *
   * @return Time in milli seconds
   */
  operator unsigned long() const
    { return ((m_time.tv_sec * 1000) + m_time.tv_usec); };

  /**
   * Set the time to the current system time.
   */
  void refresh();

  /**
   * Set the object out into the future as far as possible.
   */
  void SetInfinite()
    { m_time.tv_sec = (time_t) -1; m_time.tv_usec = 0; m_changed = true; };

  /**
   * Check if the time is infinite.
   *
   * @return True, if the time is infinite.
   */
  int IsInfinite() const
    { return ((m_time.tv_sec == (time_t) -1) && (m_time.tv_usec == 0)); };

  /**
   * Get the difference between this and the given time.
   * If future is before this objects time, "timeout" will be set to zero.
   *
   * @param future  - Time to compare to
   * @param timeout - Will be filled with the difference
   */
  void GetDelta(const msec &future, timeval &timeout) const;

  /**
   * Get the difference between this object and the current system time.
   * If the system time is before this objects time,
   * "timeout" will be set to zero.
   *
   * @param timeout - Will be filled with the difference
   */
  void GetDeltaFromNow(timeval &timeout) const
    { msec now; now.GetDelta(*this, timeout); };

  /**
   * Return the time as printable string.
   */
  const char *get_printable() const;

private:
  timeval m_time;
  SNMP_PP_MUTABLE char m_output_buffer[MSECOUTBUF];
  SNMP_PP_MUTABLE bool m_changed;

#if !defined HAVE_LOCALTIME_R && !defined HAVE_REENTRANT_LOCALTIME
#ifdef _THREADS
  static SnmpSynchronized m_localtime_mutex;
#endif
#endif
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _MSEC_H_
