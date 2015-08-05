/*_############################################################################
  _## 
  _##  pdu.h  
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

		
  SNMP++ P D U . H

  PDU CLASS DEFINITION

  DESIGN + AUTHOR:  Peter E Mellquist

  DESCRIPTION:
  Pdu class definition. Encapsulation of an SMI Protocol
  Data Unit (PDU) in C++.

=====================================================================*/
// $Id: pdu.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _PDU_CLS
#define _PDU_CLS

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/address.h"
#include "snmp_pp/timetick.h"
#include "snmp_pp/octet.h"
#include "snmp_pp/oid.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

class Vb;

#define PDU_MAX_RID 32767         ///< max request id to use
#define PDU_MIN_RID 1000          ///< min request id to use

//=======================================================================
//		     Pdu Class
//=======================================================================
/**
 * Pdu class...
 */
class DLLOPT Pdu
{
 public:

  /**
   * Constructor no args.
   *
   * This constructor creates a valid empty Pdu object.
   */
  Pdu();

  /**
   * Constructor with vbs.
   *
   * The Pdu class does not take ownership of the array and the Vb
   * objects, so if these were allocated with new, they must be freed
   * by te user with delete.
   *
   * @param pvbs      - Array of pointers to Vb objects
   * @param pvb_count - Length of the array
   */
  Pdu(Vb* pvbs, const int pvb_count);

  /**
   * Constructor with another Pdu instance.
   *
   * @param pdu - source pdu object
   */
  Pdu(const Pdu &pdu) : vbs(0), vbs_size(0), vb_count(0) { *this = pdu; };

  /**
   * Destructor
   */
  virtual ~Pdu();

  /**
   * Overloaded assignment operator.
   *
   * @param pdu - Pdu that should be assigned to this object
   */
  Pdu& operator=(const Pdu &pdu);

  /**
   * Append a vb to the pdu.
   *
   * @param vb - The Vb that should be added (as last vb) to the pdu
   */
  Pdu& operator+=(const Vb &vb);

  /**
   * Clone a Pdu object.
   *
   * @return Pointer to a newly created Pdu object, that is identical to this
   */
  Pdu *clone() const { return new Pdu(*this); };

  /**
   * Get Pointers to all Vbs from Pdu.
   *
   * The caller has to allocate the array. The returned pointers point
   * to the Vb objects that are internally used by the pdu. So any
   * changes to the returned Vb objects will change the pdu. If the
   * pdu is modified (e.g. through Pdu::trim()) afterwards, the
   * returned array will contain invalid pointers.
   *
   * @param pvbs - Array of empty pointers of size pvb_count
   * @param pvb_count - Amount of Vb pointers to get
   *
   * @return TRUE on success
   */
  int get_vblist(Vb* pvbs, const int pvb_count) const;

  /**
   * Deposit all Vbs to Pdu.
   *
   * The vb objects of the pdu will be freed and the objects from the
   * array will be cloned and added to the pdu. If this method returns
   * FALSE, the pdu will not conatin any Vb objects.
   *
   * @param pvbs - Array of valid pointers of size pvb_count
   * @param pvb_count - Amount of Vb pointers i the array
   *
   * @return TRUE on success
   */
  int set_vblist(Vb const * pvbs, const int pvb_count);

  /**
   * Get a particular Vb.
   *
   * @param vb - Object to store the vb
   * @param index - The vb to get (zero is the first vb)
   *
   * @return TRUE on success
   */
  int get_vb(Vb &vb, const int index) const;

  /**
   * Return a reference to a particular Vb.
   *
   * @note Before calling this method, make sure that there
   *       is a Vb using get_vb_count().
   *
   * @param index - The Vb to return starting with 0.
   * @return A const reference to the Vb
   */
  const Vb &get_vb(const int index) const { return *vbs[index]; };

  /**
   * Set a particular vb.
   *
   * If this method returns FALSE, the pdu has not been modified.
   *
   * @param vb - Source vb
   * @param index - The vb to set (zero is the first vb)
   *
   * @return TRUE on success
   */
  int set_vb(Vb const &vb, const int index);

  /**
   * Get the number of vbs.
   *
   * @return The number of Vb objects within the pdu.
   */
  int get_vb_count() const { return vb_count; };

  /**
   * Get a Vb.
   *
   * @note The index has to be checked by the caller.
   *
   * @param i zero based index
   */
  Vb& operator[](const int i) { return *vbs[i]; };

  /**
   * Get the error status.
   *
   * @return The SNMP error status
   */
  int get_error_status() const { return error_status; };

  /**
   * Set the error status.
   *
   * @param err - The new SNMP error status.
   */
  void set_error_status(const int err) {  error_status = err; };

  /**
   * Clear the error status.
   */
  void clear_error_status() { error_status = 0; };

  /**
   * Get the error index.
   *
   * @return The SNMP error index
   */
  int get_error_index() const { return error_index; };

  /**
   * Set the error index.
   *
   * @param err - The new SNMP error index.
   */
  void set_error_index(const int index) { error_index = index; };

  /**
   * Clear the error index.
   */
  void clear_error_index() { error_index = 0; };

  /**
   * Clear error status and error index.
   */
  void clear_error() { set_error_status(0); set_error_index(0); }

  /**
   * Get the request id.
   *
   * @return The SNMP request id
   */
  unsigned long get_request_id() const { return request_id; };

  /**
   * Set the request id.
   *
   * @param rid - The new SNMP request id
   */
  void set_request_id(const unsigned long rid) { request_id = rid; };

  /**
   * Get the pdu type.
   */
  unsigned short get_type() const { return pdu_type; };

  /**
   * Set the pdu type.
   */
  void set_type(unsigned short type) { pdu_type = type; };

  /**
   * Returns validity of Pdu instance.
   */
  bool valid() const { return validity; };

  /**
   * Trim off vbs.
   *
   * @param count - number of vbs to trim of, starting with the last
   * @return TRUE on success, FALSE if nothing was done
   */
  int trim(const int count=1);

  /**
   * Delete a Vb anywhere within the Pdu.
   *
   * @param position - Delete the Vb at this position (starting with 0)
   * @return TRUE on success
   */
  int delete_vb(const int position);

  /**
   * Set notify timestamp.
   */
  void set_notify_timestamp(const TimeTicks &ts) { notify_timestamp = ts; };

  /**
   * Get notify timestamp.
   */
  void get_notify_timestamp(TimeTicks &ts) const { ts = notify_timestamp; };

  /**
   * Set the notify id.
   *
   * @return true if the set succeeded.
   */
  bool set_notify_id(const Oid &id)
    { notify_id = id; return (notify_id.len() == id.len()); };

  /**
   * Get the notify id.
   *
   * @return true if the get succeeded.
   */
  bool get_notify_id(Oid &id) const
    { id = notify_id; return (notify_id.len() == id.len()); };

  /**
   * Set the notify enterprise.
   *
   * @return true if the set succeeded.
   */
  bool set_notify_enterprise(const Oid &e)
    { notify_enterprise = e; return (notify_enterprise.len() == e.len()); };

  /**
   * Get the notify enterprise.
   *
   * @return true if the get succeeded.
   */
  bool get_notify_enterprise(Oid & e) const
    { e = notify_enterprise;  return (notify_enterprise.len() == e.len()); };

#ifdef _SNMPv3
  /**
   * Set the security level that should be used when this Pdu is sent.
   * The default security level of a Pdu is SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV.
   *
   * @param level - One of SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV,
   *                SNMP_SECURITY_LEVEL_AUTH_NOPRIV,
   *                SNMP_SECURITY_LEVEL_AUTH_PRIV
   */
  void set_security_level(const int level) { security_level = level; };

  /**
   * Return the security level of the Pdu.
   *
   * @return - the security level
   */
  int get_security_level() const { return security_level; };

  /**
   * Set the context name of the Pdu.
   *
   * @param name - The context name
   */
  bool set_context_name(const OctetStr &name)
    { context_name = name; return (context_name.valid() && name.valid()); };

  /**
   * Set the context name of the Pdu.
   *
   * @param name - The context name
   */
  bool set_context_name(const char *name)
    { context_name = name; return context_name.valid(); };

  /**
   * Get the context name of the Pdu.
   *
   * @param name - Object fot the context name
   */
  bool get_context_name(OctetStr &name) const
    { name = context_name; return (context_name.valid() && name.valid()); };

  /**
   * Get the context name of the Pdu.
   *
   * @return - Return the context name as an OctetStr
   */
  const OctetStr& get_context_name() const { return context_name; };

  /**
   * Set the context engine id of the Pdu.
   *
   * @param id - The new context engine id
   */
  bool set_context_engine_id(const OctetStr &id) { context_engine_id = id;
    return (context_engine_id.valid() && id.valid()); };

  /**
   * Set the context engine id of the Pdu.
   *
   * @param id - The new context engine id
   */
  bool set_context_engine_id(const char *id)
    { context_engine_id = id; return context_engine_id.valid(); };

  /**
   * Get the context engine id of the Pdu.
   *
   * @param id - Object for the context engine
   */
  bool get_context_engine_id(OctetStr &id) const { id = context_engine_id;
    return (context_engine_id.valid() && id.valid()); };

  /**
   * Get the context engine id of the Pdu.
   *
   * @return - Return the context engine id as an OctetStr
   */
  const OctetStr& get_context_engine_id() const { return context_engine_id; };

  /**
   * Set the SNMPv3 message id (msgID)
   *
   * @param msg_id - the message id of the received message
   */
  void set_message_id(const unsigned long msg_id) { message_id = msg_id; }

  /**
   * Get the SNMPv3 message id (msgID)
   *
   * @return - the message id of the received message
   */
  unsigned long get_message_id() const { return message_id; }

  /**
   * Set the maximum size of the scoped pdu to be included in a
   * possible response message.
   *
   * @param l - the maximum size
   */
  void set_maxsize_scopedpdu(unsigned long l) { maxsize_scopedpdu = l; };

  /**
   * Get the maximum size of the scoped pdu to be included in a
   * possible response message.
   *
   * @return - the maximum size
   */
  unsigned long get_maxsize_scopedpdu() const { return maxsize_scopedpdu; };

#endif // _SNMPv3

  /**
   * Get the SNMPv1 trap address
   */
  int get_v1_trap_address(GenAddress &address) const;

  /**
   * Set the SNMPv1 trap address
   */
  int set_v1_trap_address(const Address &address);

  /**
   * Return the length of the encoded vbs with pdu header.
   *
   * @note this method wll not work for v1 traps.
   */
  int get_asn1_length() const;

  /**
   * Clear the Pdu contents (destruct and construct in one go)
   */
  void clear();

  /**
   * Does the type of response match the type of request.
   */
  static bool match_type(const int request, const int response);

  //-------------[ protected members ]--------------------------
 protected:

  /**
   * Extend the vbs array.
   *
   * @return true on success
   */
  bool extend_vbs();

  Vb **vbs;                    // pointer to array of Vbs
  int vbs_size;                // Size of array
  int vb_count;                // count of Vbs
  int error_status;            // SMI error status
  int error_index;             // SMI error index
  bool validity;               // valid boolean
  unsigned long request_id;	 // SMI request id
  unsigned short pdu_type;	 // derived at run time based on request type
  // for notify Pdu objects only
  // traps & notifies
  TimeTicks notify_timestamp;      // a timestamp associated with an infor
  Oid notify_id;                   // an id
  Oid notify_enterprise;
  GenAddress v1_trap_address;      // address object
  int        v1_trap_address_set;
#ifdef _SNMPv3
  // specific Objects for SNMPv3
  int security_level;            // the securityLevel with which this Pdu
                                 // should be sent or was received
  unsigned long message_id;
  unsigned long maxsize_scopedpdu;
  OctetStr context_name;
  OctetStr context_engine_id;
#endif // _SNMPv3
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif //_PDU_CLS
