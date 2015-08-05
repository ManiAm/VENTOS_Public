/*_############################################################################
  _## 
  _##  timetick.h  
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


  SNMP++ T I M E T I C K. H

  TIMETICK CLASS DEFINITION

  DESIGN + AUTHOR:  Peter E Mellquist

  DESCRIPTION:
  Class definition for SMI Timeticks class.

=====================================================================*/
// $Id: timetick.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _TIMETICKS
#define _TIMETICKS

#include "snmp_pp/integer.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#define TICKOUTBUF 30 // max formatted time string

//------------[ TimeTicks Class ]-----------------------------------
/**
 * The timeticks class allows all the functionality of unsigned
 * integers but is recognized as a distinct SMI type. TimeTicks
 * objects may be get or set into Vb objects.
 */
class DLLOPT TimeTicks : public SnmpUInt32
{
 public:
#if 0
  /**
   * Constructs a zero TimeTicks object.
   */
  TimeTicks() : SnmpUInt32()
    { smival.syntax = sNMP_SYNTAX_TIMETICKS; };
#endif

  /**
   * Constructs a TimeTicks object with the given value.
   *
   * @param val - time in hundredths of seconds.
   */
  TimeTicks(const unsigned long val = 0)
    : SnmpUInt32(val)
  {
    smival.syntax = sNMP_SYNTAX_TIMETICKS;
  }

  /**
   * Copy constructor.
   *
   * @param t - Time for the new object.
   */
  TimeTicks(const TimeTicks &t)
    : SnmpUInt32(t)
  {
    smival.syntax = sNMP_SYNTAX_TIMETICKS;
  }

  /**
   * Destructor.
   */
  ~TimeTicks() {}

  /**
   * Return the syntax.
   *
   * @return Always returns sNMP_SYNTAX_TIMETICKS.
   */
  SmiUINT32 get_syntax() const { return sNMP_SYNTAX_TIMETICKS; }

  /**
   * Get a printable ASCII value.
   */
  const char *get_printable() const;

  /**
   * Clone operator.
   *
   * @return Pointer to a newly created copy of the object.
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *) new TimeTicks(*this); }

  /**
   * Map other SnmpSyntax objects to TimeTicks.
   */
  using SnmpUInt32::operator = ;

  /**
   * Overloaded assignment for TimeTicks.
   *
   * @param uli - new value
   * @return self reference
   */
  TimeTicks& operator=(const TimeTicks &uli)
  {
    smival.value.uNumber = uli.smival.value.uNumber;
    valid_flag = uli.valid_flag;
    m_changed = true;
    return *this;
  }

  /**
   * Overloaded assignment for unsigned longs.
   *
   * @param ul - new value in hundrets of seconds
   * @return self reference
   */
  TimeTicks& operator=(const unsigned long ul)
  {
    SnmpUInt32::operator = (ul);
    return *this;
  }

#if 0
  // XXX this operator is already provided by SnmpUInt32
  /**
   * Casting to unsigned long.
   *
   * @return Current value as hundrets of seconds
   */
  operator unsigned long() { return smival.value.uNumber; };

  /**
   * Reset the object.
   */
  void clear()
    { smival.value.uNumber = 0; m_changed = true; };
#endif
 protected:
  SNMP_PP_MUTABLE char output_buffer[TICKOUTBUF];  // for storing printed form
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif
