/*_############################################################################
  _## 
  _##  target.h  
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
  purpose. It is provided "AS-IS" without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.


  SNMP++  T A R G E T . H

  TARGET CLASS DEFINITION

  DESIGN + AUTHOR:  Peter E Mellquist

  DESCRIPTION:
  Target class defines target SNMP agents.

=====================================================================*/
// $Id: target.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _TARGET
#define _TARGET

//----[ includes ]-----------------------------------------------------

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/address.h"
#include "snmp_pp/octet.h"
#include "snmp_pp/collect.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif


//----[ enumerated types for SNMP versions ]---------------------------
/**
 * The SNMP version to use is passed with this enum.
 */
enum snmp_version
{
  version1,         ///< (0) SNMPv1 
  version2c         ///< (1) SNMPv2c
#ifdef _SNMPv3
  ,version2stern,   ///< (2) Dont use this!
  version3          ///< (3) SNMPv3
#endif
};

//----[ Target class ]-------------------------------------------------
/**
 * Abstract class used to provide a virtual interface into Targets.
 *
 * @note Although it is possible to create an object of this class,
 *       you won't be happy with that...
 */
class DLLOPT SnmpTarget
{
 public:

  /**
   * Enum to identify a target object through SnmpTarget::get_type() method.
   */
  enum target_type
  {
    type_base,    ///< It is a SnmpTarget object
    type_ctarget, ///< It is a CTarget object
    type_utarget  ///< It is a Utarget object
  };

  /**
   * Create a SnmpTarget object with default values.
   * The validity of the target will be false.
   */
  SnmpTarget()
    : validity(false), timeout(default_timeout), retries(default_retries),
      version(version1), ttype(type_base) {};

  /**
   * Create a SnmpTarget object with the given Address.
   */
  SnmpTarget(const Address &address)
    : validity(false), timeout(default_timeout), retries(default_retries),
      version(version1), ttype(type_base), my_address(address)
    { if (my_address.valid()) validity = true; };

  /**
   * Destructor that has nothing to do.
   */
  virtual ~SnmpTarget() {};

  /**
   * Return the type of the target object.
   *
   * If a SNMP message is received through a callback (that only
   * passes a SnmpTarget pointer to the callback function), this
   * method can be used to check the type of the object before doing a
   * cast to CTarget or UTarget.
   */
  target_type get_type() const { return ttype; };

  /**
   * Returns the validity of the target object.
   *
   * @return true, if the target is valid.
   */
  bool valid() const { return validity;};

  /**
   * Set the retry value.
   *
   * @param r - The number of retries if no response is received.
   */
  void set_retry(const int r) { retries = r; };

  /**
   * Get the retry value.
   *
   * @return The number of retries on timeout.
   */
  int get_retry() const { return retries; };


  /**
   * Set the timeout for requests.
   *
   * The default timeout for requests is 1 second (100).
   *
   * @param t - Timeout in 10ms, so 100 will set the timeout to 1 second.
   */
  void set_timeout(const unsigned long t) { timeout = t; };

  /**
   * Get the timeout.
   *
   * @return The timeout for requests sent using this target object.
   */
  unsigned long get_timeout() const { return timeout; };

  /**
   * Change the default timeout.
   *
   * Changing the default timeout value will only have an effect for
   * target objects that are created after setting this value.
   *
   * @param t - The new default timeout value
   */
  static void set_default_timeout(const unsigned long t)
    { default_timeout = t; };

  /**
   * Change the default retries vlaue.
   *
   * Changing the default retries value will only have an effect for
   * target objects that are created after setting this value.
   *
   * @param r - The new retries value
   */
  static void set_default_retries(const int r) { default_retries = r; };

  /**
   * Clone operator.
   *
   * Virtual clone operation for creating a new SnmpTarget from an existing
   * SnmpTarget.
   *
   * @note The caller MUST use the delete operation on the return
   *       value when done.
   *
   * @return A pointer to the new object on success, 0 on failure.
   */
  virtual SnmpTarget *clone() const;

  /**
   * Get the address object.
   *
   * @param address - GenAddress object to store the target address.
   * @return true on success.
   */
  bool get_address(GenAddress &address) const;

  /**
   * Get the address object.
   *
   * @return The target address.
   */
  const GenAddress &get_address() const { return my_address; };

  /**
   * Set the address object.
   *
   * @param address - The address that this target should use.
   * @return true on success.
   */
  virtual bool set_address(const Address &address);

  /**
   * Get the SNMP version for this target.
   *
   * @return The SNMP version of this target object.
   * @see enum snmp_version
   */
  snmp_version get_version() const { return version;};

  /**
   * Set the SNMP version of this target.
   *
   * @param v - The SNMP version that should be used for sending messages.
   */
  void set_version(const snmp_version v) { version = v; };

  /**
   * Overloeaded compare operator.
   *
   * Two SnmpTarget objects are considered equal, if all member
   * variables are equal.
   *
   * @return 1 if targets are equal, 0 if not.
   */
  int operator==(const SnmpTarget &rhs) const;

  /**
   * Reset the object.
   */
  virtual void clear();

 protected:
  bool validity;         ///< Validity of the object
  unsigned long timeout; ///< xmit timeout in 10 milli secs
  int retries;           ///< number of retries
  snmp_version version;  ///< SNMP version to use
  target_type ttype;     ///< Type of the target
  GenAddress my_address; ///< Address object

  static unsigned long default_timeout; ///< default timeout for new objects
  static int default_retries;           ///< default retries for new objects
};

//----[  CTarget class ]----------------------------------------------
/**
 * Community based target object.
 * This target can be used for SNMPv1 and SNMPv2c messages.
 */
class DLLOPT CTarget: public SnmpTarget
{
 public:
  /**
   * Constructor with no args.
   * The validity of the target will be false.
   */
  CTarget();

  /**
   * Constructor with all args.
   *
   * @param address - Address of the target host (cann be any address object)
   * @param read_community_name - Community for get requests
   * @param write_community_name - Community for set requests
   */
  CTarget(const Address &address,
	  const char *read_community_name,
	  const char *write_community_name);

  /**
   * Constructor with all args.
   *
   * @param address - Address of the target host (cann be any address object)
   * @param read_community_name - Community for get requests
   * @param write_community_name - Community for set requests
   */
  CTarget(const Address &address,
	  const OctetStr &read_community_name,
	  const OctetStr &write_community_name);

  /**
   * Constructor with only address.
   *
   * The read and write community names will be set to "public".
   *
   * @param address - Address of the target host (cann be any address object)
   */
  CTarget(const Address &address);

  /**
   * Constructor from existing CTarget.
   */
  CTarget(const CTarget &target);

  /**
   * Destructor, that has nothing to do.
   */
  ~CTarget() {};

  /**
   * Clone operator.
   *
   * Clone operation for creating a new CTarget from an existing
   * CTarget.
   *
   * @note The caller MUST use the delete operation on the return
   *       value when done.
   *
   * @return A pointer to the new object on success, 0 on failure.
   */
  SnmpTarget *clone() const { return (SnmpTarget *) new CTarget(*this); };

  /**
   * Get the read community name.
   *
   * @return C string of the read community.
   */
  const char * get_readcommunity() const
    { return (const char *) read_community.get_printable(); };

  /**
   * Get the read community name.
   *
   * @param oct - OctetStr that will be filled with the read community name.
   */
  void get_readcommunity(OctetStr& oct) const { oct = read_community; };

  /**
   * Set the read community name.
   *
   * @param str - The new read community name
   */
  void set_readcommunity(const char * str) { read_community = str; };

  /**
   * Set the read community name.
   *
   * @param oct - The new read community name
   */
  void set_readcommunity(const OctetStr& oct) { read_community = oct; };

  /**
   * Get the write community name.
   *
   * @return C string of the write community.
   */
  const char * get_writecommunity() const
    { return (const char *) write_community.get_printable(); };

  /**
   * Get the write community name.
   *
   * @param oct - OctetStr that will be filled with the write community name.
   */
  void get_writecommunity(OctetStr &oct) const { oct = write_community; };

  /**
   * Set the write community name.
   *
   * @param str - The new write community name
   */
  void set_writecommunity(const char *str) { write_community = str; };

  /**
   * Set the write community name.
   *
   * @param oct - The new write community name
   */
  void set_writecommunity(const OctetStr &oct) { write_community = oct; };

  /**
   * Overloaded assignment operator.
   */
  CTarget& operator=(const CTarget& target);

  /**
   * Overloeaded compare operator.
   *
   * Two CTarget objects are considered equal, if all member variables
   * and the base classes are equal.
   *
   * @return 1 if targets are equal, 0 if not.
   */
  int operator==(const CTarget &rhs) const;

  /**
   * Get all values of a CTarget object.
   *
   * @param read_comm  - Read community name
   * @param write_comm - Write community name
   * @param address    - Address of the target
   * @param t          - Timeout value
   * @param r          - Retries value
   * @param v          - The SNMP version of this target
   *
   * @return true on success and FALSE on failure.
   */
  bool resolve_to_C(OctetStr& read_comm, 
                    OctetStr& write_comm,
                    GenAddress &address, 
                    unsigned long &t,
                    int &r, 
                    unsigned char &v) const;

  /**
   * Reset the object.
   */
  void clear();

 protected:
  OctetStr read_community;        //  get community
  OctetStr write_community;       //  set community
};

// create OidCollection type
typedef SnmpCollection<SnmpTarget> TargetCollection;

#ifdef _SNMPv3
#define INITIAL_USER "initial"
#else
#define INITIAL_USER "public"
#endif

//----[  UTarget class ]----------------------------------------------
/**
 * User based Target.
 *
 * This class is used for SNMPv3 targets.
 */
class DLLOPT UTarget: public SnmpTarget
{
 public:
  /**
   * Constructor with no args.
   * The validity of the target will be false.
   */
  UTarget();

  /**
   * Constructor with all args.
   *
   * @param address   - Address of the target host (cann be any address object)
   * @param sec_name   - The security name
   * @param sec_model - The security model to use
   */
  UTarget(const Address &address,
	  const char *sec_name,
	  const int sec_model);

  /**
   * Constructor with all args.
   *
   * @param address   - Address of the target host (cann be any address object)
   * @param sec_name  - The security name
   * @param sec_model - The security model to use
   */
  UTarget(const Address &address,
	  const OctetStr &sec_name,
	  const int sec_model);

  /**
   * Constructor with only address.
   *
   * Assumes the following defaults: security_name: initial, version: SNMPv3,
   * security_model: v3MP.
   *
   * @param address - Address of the target host (cann be any address object)
   */
  UTarget(const Address &address);

  /**
   * Constructor from existing UTarget.
   */
  UTarget(const UTarget &target);

  /**
   * Destructor, that has nothing to do.
   */
  ~UTarget() {};

  /**
   * Clone operator.
   *
   * Clone operation for creating a new UTarget from an existing
   * UTarget.
   *
   * @note The caller MUST use the delete operation on the return
   *       value when done.
   *
   * @return A pointer to the new object on success, 0 on failure.
   */
  SnmpTarget *clone() const { return (SnmpTarget *) new UTarget(*this); };

  /**
   * Set the address object.
   *
   * This method is the same as in SnmpTarget, but it deletes engine_id.
   *
   * @param address - The address that this target should use.
   * @return true on success.
   */
  bool set_address(const Address &address);

  /**
   * Get the security name.
   *
   * @return A const reference to the security name.
   */
  const OctetStr& get_security_name() const { return security_name;} ;

  /**
   * Get the security name.
   *
   * @param oct - OctetStr that will be filled with the security name.
   */
  void get_security_name(OctetStr& oct) const { oct = security_name; };

  /**
   * Set the security name.
   *
   * @param str - The new security name
   */
  void set_security_name(const char * str) { security_name = str; };

  /**
   * Set the security name.
   *
   * @param oct - The new security name
   */
  void set_security_name(const OctetStr& oct) { security_name = oct; };

#ifdef _SNMPv3
  /**
   * Set the engine id.
   *
   * In most cases it is not necessary for the user to set the engine
   * id as snmp++ performs engine id discovery. If the engine id is
   * set by the user, no engine_id discovery is made, even if the
   * engine id set by the user is wrong.
   *
   * @param str - The engine id to use
   */
  void set_engine_id(const char * str) { engine_id = str; };

  /**
   * Set the engine id.
   *
   * In most cases it is not necessary for the user to set the engine
   * id as snmp++ performs engine id discovery. If the engine id is
   * set by the user, no engine_id discovery is made, even if the
   * engine id set by the user is wrong.
   *
   * @param oct - The engine id to use
   */
  void set_engine_id(const OctetStr &oct) { engine_id = oct; };

  /**
   * Get the engine id.
   *
   * @return A const reference to the enigne id of this target.
   */
  const OctetStr& get_engine_id() const { return engine_id; };

  /**
   * Get the engine id.
   *
   * @param oct - OctetStr that will be filled with the engine id
   */
  void get_engine_id(OctetStr& oct) const { oct = engine_id; };
#endif

  /**
   * Get the security_model.
   *
   * @return An integer representing the security_model of this target.
   */
  int get_security_model() const { return security_model; };

  /**
   * Set the security_model.
   *
   * @param sec_model - The security model to use.
   */
  void set_security_model(int sec_model) { security_model = sec_model; };

  /**
   * Overloaded assignment operator.
   */
  UTarget& operator=(const UTarget& target);

  /**
   * Overloeaded compare operator.
   *
   * Two UTarget objects are considered equal, if all member variables
   * (beside the engine id) and the base classes are equal.
   *
   * @return 1 if targets are equal, 0 if not.
   */
  virtual int operator==(const UTarget &rhs) const;

  /**
   * Get all values of a UTarget object.
   *
   * @param sec_name   - security name
   * @param sec_model  - security model
   * @param address    - Address of the target
   * @param t          - Timeout value
   * @param r          - Retries value
   * @param v          - The SNMP version of this target
   *
   * @return TRUE on success and FALSE on failure.
   */
  bool resolve_to_U(OctetStr&  sec_name,
		   int &sec_model,
		   GenAddress &address,
		   unsigned long &t,
		   int &r,
		   unsigned char &v) const;

  /**
   * Reset the object.
   */
  void clear();

 protected:
  OctetStr security_name;
  int security_model;
#ifdef _SNMPv3
  OctetStr engine_id;
#endif
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif //_TARGET
