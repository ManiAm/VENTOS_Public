/*_############################################################################
  _## 
  _##  vb.h  
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


  SNMP++ V B . H

  VARIABLE BINDING CLASS DEFINITION

  DESCRIPTION:
  This module contains the class definition for the variable binding
  class. The VB class is an encapsulation of a SNMP VB. A VB object is
  composed of an SNMP++ Oid and an SMI value. The Vb class utilizes Oid
  objects and thus requires the Oid class. The Vb class may be used
  stand alone and does not require use of any other snmp library.

  DESIGN + AUTHOR:  Peter E. Mellquist

=====================================================================*/
// $Id: vb.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _VB_CLS
#define _VB_CLS

#include "snmp_pp/oid.h"                 // oid class def
#include "snmp_pp/timetick.h"            // time ticks
#include "snmp_pp/counter.h"             // counter
#include "snmp_pp/gauge.h"               // gauge class
#include "snmp_pp/ctr64.h"               // 64 bit counters
#include "snmp_pp/octet.h"               // octet class
#include "snmp_pp/address.h"             // address class def
#include "snmp_pp/integer.h"             // integer class
#include "snmp_pp/snmperrs.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif


//------------[ VB Class Def ]-------------------------------------
/**
 * The Vb class is the encapsulation of the SNMP variable binding.
 *
 * Variable binding lists in SNMP++ are represented as arrays of Vb
 * objects. Vb objects are passed to and from SNMP objects to provide
 * getting or setting MIB values.  The vb class keeps its own memory
 * for objects and does not utilize pointers to external data
 * structures.
 */
class DLLOPT Vb
{
 //-----[ public members ]
 public:

  //-----[ constructors / destructors ]-------------------------------

  /**
   * Constructor with no arguments.
   *
   * This constructor creates an unitialized vb.
   */
  Vb() : iv_vb_value(0), exception_status(SNMP_CLASS_SUCCESS) {};

  /**
   * Constructor to initialize the oid.
   *
   * This constructor creates a vb with oid portion initialized.
   */
  Vb(const Oid &oid)
    : iv_vb_oid(oid), iv_vb_value(0), exception_status(SNMP_CLASS_SUCCESS) {};

  /**
   * Copy constructor.
   */
  Vb(const Vb &vb) : iv_vb_value(0) { *this = vb; };

  /**
   * Destructor that frees all allocated memory.
   */
  ~Vb() { free_vb(); };

  /**
   * Overloaded assignment operator.
   */
  Vb& operator=(const Vb &vb);

  /**
   * Clone operator.
   */
  Vb *clone( ) const { return new Vb(*this); };

  //-----[ set oid / get oid ]------------------------------------------

  /**
   * Set the oid from another oid.
   */
  void set_oid(const Oid &oid) { iv_vb_oid = oid; };

  /**
   * Get the oid portion.
   *
   * @note Check the validity of the object through Vb::valid() before
   *       calling this method.
   */
  void get_oid(Oid &oid) const { oid = iv_vb_oid; };

  /**
   * Get the oid portion as a const.
   *
   * @note Check the validity of the object through Vb::valid() before
   *       calling this method.
   */
  const Oid &get_oid() const { return iv_vb_oid; };

  //-----[ set value ]--------------------------------------------------

  /**
   * Set the value using any SnmpSyntax object.
   */
  void set_value(const SnmpSyntax &val)
    { free_vb(); iv_vb_value = val.clone(); };

  /**
   * Set the value with an int.
   *
   * The syntax of the Vb will be set to SMI INT32.
   */
  void set_value(const int i) { free_vb(); iv_vb_value = new SnmpInt32(i); };

  /**
   * Set the value with an unsigned int.
   *
   * The syntax of the Vb will be set to SMI UINT32.
   */
  void set_value(const unsigned int i)
    { free_vb(); iv_vb_value = new SnmpUInt32(i); };

  /**
   * Set the value with a long int.
   *
   * @note Even on 64 bit platforms, only 32 bits are used
   *
   * The syntax of the Vb will be set to SMI INT32.
   */
  void set_value(const long i)
    { free_vb(); iv_vb_value = new SnmpInt32(i); };

  /**
   * Set the value with an unsigned long int.
   *
   * @note Even on 64 bit platforms, only 32 bits are used
   *
   * The syntax of the Vb will be set to SMI UINT32.
   */
  void set_value(const unsigned long i)
    { free_vb(); iv_vb_value = new SnmpUInt32(i); };

  /**
   * Set value using a null terminated string.
   *
   * The syntax of the Vb will be set to SMI octet.
   */
  void set_value(const char *ptr)
    { free_vb(); iv_vb_value = new OctetStr(ptr); };

  /**
   * Set value using a string and length.
   *
   * The syntax of the Vb will be set to SMI octet.
   */
  void set_value(const unsigned char *ptr, const unsigned int len)
    { free_vb(); iv_vb_value = new OctetStr(ptr, len); };

  /**
   * Set the value portion of the vb to null, if its not already.
   */
  void set_null() { free_vb(); };

  //----[ get value ]------------------------------------------------

  /**
   * Get the value using a SnmpSyntax object.
   *
   * @param val - An object of a subclass of SnmpSyntax that will be
   *              assigned the value of the vb.
   *
   * @return SNMP_CLASS_SUCCESS if the vb value could be assigned to
   *         the passed SnmpSyntax object, else SNMP_CLASS_INVALID.
   */
  int get_value(SnmpSyntax &val) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb is SMI INT32.
   *
   * @param i - returned value
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(int &i) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb can
   * be mapped to an unsigned long (SMI types uint32, counter32, gauge
   * and timeticks).
   *
   * @param i - returned value
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(unsigned int &i) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb is SMI INT32.
   *
   * @param i - returned value
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(long &i) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb can
   * be mapped to an unsigned long (SMI types uint32, counter32, gauge
   * and timeticks).
   *
   * @param i - returned value
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(unsigned long &i) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb can
   * be mapped to an unsigned 64bit value (SMI type counter64).
   *
   * @param i - returned value
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(pp_uint64 &i) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb is SMI OCTET.
   *
   * @note The caller must provide a target string big enough to
   *       handle the vb string. No length checks are done within
   *       this method. The returned string will be null terminated.
   *
   * @param ptr - Pointer to already allocated space to hold the vb
   *              value. The first char will be set to zero on failure.
   * @param len - Returned length of the string. Will be set to 0 on failure.
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(unsigned char *ptr, unsigned long &len) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb is SMI OCTET.
   *
   * @note If the target space is not big enough to hold the complete
   *       string only part of the string is copied.
   *
   * @param ptr    - Pointer to already allocated space to hold the vb
   *                 value. The first char will be set to zero on failure.
   * @param len    - Returned length of the string. Will be set to 0
   *                 on failure.
   * @param maxlen - Maximum length of the space that ptr points to.
   * @param add_null_byte - Add a null byte at end of output string.
   *
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(unsigned char *ptr,
		unsigned long &len,
		const unsigned long maxlen,
		const bool add_null_byte = false) const;

  /**
   * Get the value.
   *
   * This method will only return success if the value of the vb is SMI OCTET.
   *
   * @note The caller must provide a target string big enough to
   *       handle the vb string. No length checks are done within
   *       this method. The returned string will be null terminated.
   *
   * @param ptr - Pointer to already allocated space to hold the vb
   *              value. The first char will be set to zero on failure.
   *
   * @return SNMP_CLASS_SUCCESS on success, else SNMP_CLASS_INVALID.
   */
  int get_value(char *ptr) const;

  /**
   * Clone the value portion of the variable binding.
   *
   * The returned pointer must be deleted by the caller.
   *
   * @return
   *    a pointer to a clone of the value of the receiver.
   */
  SnmpSyntax* clone_value() const
      { return ((iv_vb_value) ? iv_vb_value->clone() : 0); };


  //-----[ misc]--------------------------------------------------------

  /**
   * Return the syntax or the exception status.
   *
   * @return If the SNMPv2 exception status is set, it is returned.
   *         otherwise the syntax of the value object is returned.
   */
  SmiUINT32 get_syntax() const;

  /**
   * Set the syntax.
   *
   * The Value portion of the Vb will be deleted and a new value portion
   * is allocated with it's default value (zero).
   *
   * @param syntax - The new syntax.
   */
  void set_syntax(const SmiUINT32 syntax);

  /**
   * Set the exception status.
   *
   * @param status - the new SNMPv2 exception status.
   */
  void set_exception_status(const SmiUINT32 status)
    { free_vb(); exception_status = status; };

  /**
   * Get the exception status.
   */
  SmiUINT32 get_exception_status() const { return exception_status; };

  /**
   * Return a formatted version of the value.
   *
   * @return A null terminated string (empty if no value).
   */ 
  const char *get_printable_value() const;

  /**
   * Return a formatted version of the Oid.
   *
   * @return A null terminated string (may be empty if no Oid has been set).
   */
  const char *get_printable_oid() const
    { return iv_vb_oid.get_printable(); };

  /**
   * Return the validity of a Vb object.
   *
   * @return TRUE if oid and value have been set.
   */
  bool valid() const;

  /**
   * Return the space needed for serialization.
   *
   * @return the length of the BER encoding of this Vb.
   */
  int get_asn1_length() const;

  /**
   * Reset the object.
   */
  void clear() { free_vb(); iv_vb_oid.clear(); };

 //-----[ protected members ]
 protected:
  Oid iv_vb_oid;               // a vb is made up of a oid
  SnmpSyntax *iv_vb_value;     // and a value...
  SmiUINT32 exception_status;  // are there any vb exceptions??

  /**
   * Free the value portion.
   */
  void free_vb();
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif
