/*_############################################################################
  _## 
  _##  octet.h  
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

		
  SNMP++ O C T E T . H

  OCTETSTR CLASS DEFINITION

  DESIGN + AUTHOR:  Peter E Mellquist

  DESCRIPTION:
  This class is fully contained and does not rely on or any other
  SNMP libraries. This class is portable across any platform
  which supports C++.
=====================================================================*/
// $Id: octet.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _OCTET_CLS
#define _OCTET_CLS

#include "snmp_pp/smival.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

//------------[ SNMP++ OCTETSTR CLASS DEF  ]-----------------------------
class DLLOPT OctetStr: public  SnmpSyntax
{
 public:

  /**
   * Enum for setting the hex output format.
   */
  enum OutputType
  {
    OutputHexAndClear,
    OutputHex,
    OutputClear
  };

  //-----------[ Constructors and Destrucotr ]----------------------

  /**
   * Constructs a valid OctetStr with zero length.
   */
  OctetStr();

  /**
   * Constructs a OctetStr with the given value.
   * The OctetStr will be valid unless a call to new fails.
   *
   * @param str - Null terminated string
   */
  OctetStr(const char *str);

  /**
   * Constructs a OctetStr with the given value.
   * The OctetStr will be valid unless a call to new fails.
   *
   * @param str - string that may contain null bytes
   * @param len - length of the string
   */
  OctetStr(const unsigned char *str, unsigned long len);

  /**
   * Construct a OctetStr from another OctetStr.
   * The OctetStr will be valid unless a call to new fails.
   *
   * @param octet - Value for the new object
   */
  OctetStr(const OctetStr &octet);

  /**
   * Destructor, frees allocated space.
   */
  ~OctetStr();

  //-----------[ Overloaded operators ]----------------------

  /**
   * Assign a char string to a OctetStr.
   */
  OctetStr& operator=(const char *str);

  /**
   * Assign a OctetStr to a OctetStr.
   */
  OctetStr& operator=(const OctetStr &octet);

  /**
   * Equal operator for two OctetStr.
   */
  DLLOPT friend int operator==(const OctetStr &lhs, const OctetStr &rhs);

  /**
   * Not equal operator for two OctetStr.
   */
  DLLOPT friend int operator!=(const OctetStr &lhs, const OctetStr &rhs);

  /**
   * Not equal operator for two OctetStr.
   */
  DLLOPT friend int operator<(const OctetStr &lhs, const OctetStr &rhs);

  /**
   * Less than operator for two OctetStr.
   */
  DLLOPT friend int operator<=(const OctetStr &lhs,const OctetStr &rhs);

  /**
   * Greater than operator for two OctetStr.
   */
  DLLOPT friend int operator>(const OctetStr &lhs, const OctetStr &rhs);

  /**
   * Greater than or equal operator for two OctetStr.
   */
  DLLOPT friend int operator>=(const OctetStr &lhs, const OctetStr &rhs);

  /**
   * Equal operator for OctetStr and char string.
   */
  DLLOPT friend int operator==(const OctetStr &lhs, const char *rhs);

  /**
   * Not equal operator for OctetStr and char string.
   */
  DLLOPT friend int operator!=(const OctetStr &lhs, const char *rhs);

  /**
   * Less than operator for OctetStr and char string.
   */
  DLLOPT friend int operator<(const OctetStr &lhs, const char *rhs);

  /**
   * Less than or equal operator for OctetStr and char string.
   */
  DLLOPT friend int operator<=(const OctetStr &lhs, const char *rhs);

  /**
   * Greater than operator for OctetStr and char string.
   */
  DLLOPT friend int operator>(const OctetStr &lhs, const char *rhs);

  /**
   * Greater than or equal operator for OctetStr and char string.
   */
  DLLOPT friend int operator>=(const OctetStr &lhs, const char *rhs);

  /**
   * Append a char string to this OctetStr.
   */
  OctetStr& operator+=(const char *a);

  /**
   * Append a single char to this OctetStr.
   */
  OctetStr& operator+=(const unsigned char c);

  /**
   * Append another OctetStr to this OctetStr.
   */
  OctetStr& operator+=(const OctetStr& octet);

  /**
   * Allow access as if it was an array.
   *
   * @note The given param is not checked for validity.
   */
  unsigned char &operator[](int i)
    { m_changed = true; validity = true; return smival.value.string.ptr[i]; };

  /**
   * Allow access as if it was an array for const OctetStr objects.
   *
   * @note The given param is not checked for validity.
   */
  unsigned char operator[](int i) const { return smival.value.string.ptr[i]; };

  /**
   * Return the syntax.
   *
   * @return This method always returns sNMP_SYNTAX_OCTETS.
   */
  SmiUINT32 get_syntax() const { return sNMP_SYNTAX_OCTETS; };

  /**
   * Return the space needed for serialization.
   */
  int get_asn1_length() const;

  /**
   * Return validity of the object.
   */
  bool valid() const { return validity; };

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *) new OctetStr(*this); };

  /**
   * Map other SnmpSyntax objects to OctetStr.
   */
  SnmpSyntax& operator=(const SnmpSyntax &val);

  /**
   * Get a printable ASCII value of the string.
   *
   * @note Depending on the selected output format, this method will
   *       return get_printable_hex() or get_printable_clear() if the
   *       string contains not printable characters.
   *
   * @return Printable, null terminated string
   */
  const char *get_printable() const;

  /**
   * Get an ASCII formatted hex dump of the contents.
   * If the output format was set to OctetStr::OutputHexAndClear,
   * the produced string of this method will look like this:
   *                                                                     <pre>
   * 09 4F 63 74 65 74 53 74 72 3A 3A 67 65 74 5F 70    .OctetStr::get_p
   * 72 69 6E 74 61 62 6C 65 5F 68 65 78 28 29          rintable_hex()
   *                                                                     </pre>
   * If the output format was set to OctetStr::OutputHex the
   * produced string will look like this:
   *                                                                     <pre>
   * 09 4F 63 74 65 74 53 74 72 3A 3A 67 65 74 5F 70
   * 72 69 6E 74 61 62 6C 65 5F 68 65 78 28 29
   *                                                                     </pre>
   * @return Printable, null terminated string.
   */
  const char *get_printable_hex() const;

  /**
   * Get the contents with all non printable characters replaced.
   *
   * @return Printable, null terminated string.
   */
  const char *get_printable_clear() const;

  /**
   * Set the output format for get_pritable_hex().
   */
  static void set_hex_output_type(const enum OutputType ot)
    { hex_output_type = ot; };

  /**
   * Set the char get_printable_hex() and get_printable_clear()
   * will use for non printable characters.
   */
  static void set_np_char(const char np) { nonprintable_char = np; };

  /**
   * Set the data on an already constructed OctetStr.
   * The given string is copied to an internal member var, so the
   * params can be destroyed afterwards.
   *
   * @param str - The new string value
   * @param len - Length of the given string
   */
  void set_data(const unsigned char *str, unsigned long len);

  /**
   * Get the length of the string.
   */
  unsigned long len() const { return smival.value.string.len; };

  /**
   * Get a pointer to internal data.
   */
  unsigned char *data() const { return smival.value.string.ptr; };

  // compare n elements of an octet
  int nCompare(const unsigned long n, const OctetStr &o) const;

  /**
   * Build an OctetStr from a hex string.
   * Called with "5465  737469 6e672074686973206D657468 6f 64 21"
   * the returned value will be "Testing this method!"
   *
   * @param hex_string - The hex string (may contain spaces)
   * @return created string
   */
  static OctetStr from_hex_string(const OctetStr &hex_string);

  /**
   * Set the character for linefeeds in get_printable() functions.
   *
   * The default linefeeds are \n for Unix and \r\n on other systems.
   *
   * @param lf_chars - string less than 3 bytes
   * @return true on success
   */
  static bool set_linefeed_chars(const char* lf_chars);

  /**
   * Null out the contents of the string. The string will be empty
   * after calling this method
   */
  void clear();

  /**
   * Append or shorten the internal data buffer.
   *
   * The buffer will either be shortened or extended. In the second case
   * zeroes are added to the end of the string.
   *
   * @param new_len - The new length for the string
   * @return true on success
   */
  bool set_len(const unsigned long new_len);

 protected:

  enum OutputFunction
  {
      OutputFunctionDefault,
      OutputFunctionHex,
      OutputFunctionClear
  };

  SNMP_PP_MUTABLE char *output_buffer;	 // formatted Octet value
  SNMP_PP_MUTABLE unsigned int output_buffer_len; // allocated space for string
  SNMP_PP_MUTABLE bool m_changed;
  SNMP_PP_MUTABLE enum OutputType output_last_type;
  SNMP_PP_MUTABLE char output_last_np_char;
  SNMP_PP_MUTABLE enum OutputFunction output_last_function;


  bool validity;		         // validity boolean

  static enum OutputType hex_output_type;
  static char nonprintable_char;
  static char linefeed_chars[3];
};

//-----------[ End OctetStr Class ]-------------------------------------

/**
 * The OpaqueStr class represents the Opaque SNMP type. It is derived from
 * the SNMP++ class OctetStr and has the same interfaces and behavior,
 * except that its syntax is sNMP_SYNTAX_OPAQUE.
 */
class OpaqueStr: public OctetStr
{
 public:
  /**
   * Constructor creating a valid zero length OpaqueStr.
   */
  OpaqueStr(): OctetStr()
    { smival.syntax = sNMP_SYNTAX_OPAQUE; };

  /**
   * Constructs a OpaqueStr with the given value.
   * The OpaqueStr will be valid unless a call to new fails.
   *
   * @param str - Null terminated string
   */
  OpaqueStr(const char *str) : OctetStr(str)
    { smival.syntax = sNMP_SYNTAX_OPAQUE; };

  /**
   * Constructs a OpaqueStr with the given value.
   * The OpaqueStr will be valid unless a call to new fails.
   *
   * @param str - string that may contain null bytes
   * @param len - length of the string
   */
  OpaqueStr(const unsigned char *str, unsigned long length)
    : OctetStr(str, length) { smival.syntax = sNMP_SYNTAX_OPAQUE; };

  /**
   * Construct a OpaqueStr from an OctetStr.
   * The OpaqueStr will be valid unless a call to new fails.
   *
   * @param octet - Value for the new object
   */
  OpaqueStr(const OctetStr &octet) : OctetStr(octet)
    { smival.syntax = sNMP_SYNTAX_OPAQUE; };

  /**
   * Construct a OpaqueStr from another OpaqueStr.
   * The OpaqueStr will be valid unless a call to new fails.
   *
   * @param opaque - Value for the new object
   */
  OpaqueStr(const OpaqueStr& opaque) : OctetStr(opaque)
    { smival.syntax = sNMP_SYNTAX_OPAQUE; };

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  virtual SnmpSyntax *clone() const { return new OpaqueStr(*this); }

  /**
   * Return the syntax.
   *
   * @return This method always returns sNMP_SYNTAX_OPAQUE.
   */
  virtual SmiUINT32 get_syntax() const { return sNMP_SYNTAX_OPAQUE; };
  
  /**
   * Map other SnmpSyntax objects to OpaqueStr.
   */
  SnmpSyntax& operator=(const SnmpSyntax &val) 
   { return OctetStr::operator=(val); }

};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _OCTET_CLS
