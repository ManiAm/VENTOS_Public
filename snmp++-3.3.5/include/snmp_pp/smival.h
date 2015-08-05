/*_############################################################################
  _## 
  _##  smival.h  
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


  SNMP++ S M I V A L . H

  SMIVALUE CLASS DEFINITION

  DESIGN + AUTHOR: Jeff Meyer

  DESCRIPTION:
  SMIValue class definition. Superclass for the various types
  of SNMP values (Address, Oid, Octet, etc.).  Provides
  only a few functions, most info is in subclass.

=====================================================================*/
// $Id: smival.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _SMIVALUE
#define _SMIVALUE

//----[ includes ]-----------------------------------------------------
#include "snmp_pp/smi.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif


//----[ macros ]-------------------------------------------------------
#if defined(USE_CPP_CASTS)
#define PP_CONST_CAST(___type, ___ptr)    const_cast< ___type >(___ptr)
#else
#define PP_CONST_CAST(___type, ___ptr)    ((___type)(___ptr))
#endif

//======================================================================
// SMI value structure conforming with SMI RFC
//
typedef struct SmiVALUE
{		/* smiVALUE portion of VarBind */
        SmiVALUE()
          : syntax(sNMP_SYNTAX_NULL)
        { memset( &value, 0, sizeof(value) ); }

	  SmiUINT32 syntax;	/* Insert SNMP_SYNTAX_<type> */
        union	{
          SmiINT    sNumber;    /* SNMP_SYNTAX_INT
                                   SNMP_SYNTAX_INT32 */
          SmiUINT32 uNumber;    /* SNMP_SYNTAX_UINT32
                                   SNMP_SYNTAX_CNTR32
                                   SNMP_SYNTAX_GAUGE32
                                   SNMP_SYNTAX_TIMETICKS */
          SmiCNTR64 hNumber;    /* SNMP_SYNTAX_CNTR64 */
          SmiOCTETS string;     /* SNMP_SYNTAX_OCTETS
                                   SNMP_SYNTAX_BITS
                                   SNMP_SYNTAX_OPAQUE
                                   SNMP_SYNTAX_IPADDR
                                   SNMP_SYNTAX_NSAPADDR */
          SmiOID    oid;        /* SNMP_SYNTAX_OID */
          SmiBYTE   empty;      /* SNMP_SYNTAX_NULL
                                   SNMP_SYNTAX_NOSUCHOBJECT
                                   SNMP_SYNTAX_NOSUCHINSTANCE
                                   SNMP_SYNTAX_ENDOFMIBVIEW */
		  }   value;
               } *SmiLPVALUE;
//=================================================================

//--------------------------------------------------------------------
//----[ SnmpSyntax class ]--------------------------------------------
//--------------------------------------------------------------------

/**
 * An "abstract" (pure virtual) class that serves as the base class
 * for all specific SNMP syntax types.
 */
class DLLOPT SnmpSyntax {

public:

  /**
   * Virtual function for getting a printable ASCII value for any SNMP
   * value. 
   *
   * @note The returned string is valid as long as the object is not
   *       modified.
   * @note This function is NOT thread safe.
   */
  virtual const char *get_printable() const = 0;

  /**
   * Return the current syntax.
   */
  virtual SmiUINT32 get_syntax() const = 0;

  /**
   * Virtual clone operation for creating a new Value from an existing
   * value. 
   * 
   * @note The caller MUST use the delete operation on the return
   *       value when done.
   */
  virtual  SnmpSyntax * clone() const = 0;

  /**
   * Virtual destructor to ensure deletion of derived classes...
   */
  virtual ~SnmpSyntax() {}

  /**
   * Overloaded assignment operator.
   *
   * @note This should be pure virtual, but buggy VC++ compiler
   *       complains about unresolved reference at link time.
   *       XXX probably happens because it's not implemented in
   *           all derived classes?
   */
  virtual SnmpSyntax& operator = (const SnmpSyntax &/*val*/) = 0;

  /**
   * Return validity of the object.
   */
  virtual bool valid() const = 0;

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const = 0;

  /**
   * Reset the object.
   */
  virtual void clear() = 0;

protected:
  SnmpSyntax()
    : smival()
  {}

  SmiVALUE smival;
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif  // _SMIVALUE
