/*_############################################################################
  _## 
  _##  address.h  
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


  SNMP++ A D D R E S S . H

  ADDRESS CLASS DEFINITION

  DESIGN + AUTHOR:   Peter E Mellquist

  DESCRIPTION:
  Address class definition. Encapsulates various network
  addresses into easy to use, safe and portable classes.

=====================================================================*/
// $Id: address.h 2789 2014-11-28 05:53:35Z fock $

#ifndef _ADDRESS
#define _ADDRESS


//----[ includes ]-----------------------------------------------------
#include <string.h>

#include "snmp_pp/config_snmp_pp.h" // for _IPX_ADDRESS and _MAC_ADDRESS
#include "snmp_pp/smival.h"
#include "snmp_pp/collect.h"
#include "snmp_pp/reentrant.h"
#include "snmp_pp/octet.h"  // for OctetStr

// include sockets header files
// for Windows16 and Windows32 include Winsock
// otherwise assume UNIX
#if defined (CPU) && CPU == PPC603
#include <inetLib.h>
#include <hostLib.h>
#endif

#ifdef __unix
#if !defined(_AIX)
#include <unistd.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#if defined _AIX
#include <strings.h> // This is needed for FD_SET, bzero
#endif

#if !defined __CYGWIN32__ && !defined __hpux && !defined linux && !defined _AIX
extern int h_errno;  // defined in WinSock header, but not for UX?!
#endif
#endif // __unix

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

//----[ macros ]-------------------------------------------------------
#define ADDRBUF 50     // worst case of address lens
#define OUTBUFF 80     // worst case of output lens

#define IPLEN      4
#define UDPIPLEN   6
#define IP6LEN_NO_SCOPE   16
#define IP6LEN_WITH_SCOPE 20
#define UDPIP6LEN_NO_SCOPE   18
#define UDPIP6LEN_WITH_SCOPE 22
#define IS_IP6LEN(n) ((n==16) || (n==20))
#define IS_UDPIP6LEN(n) ((n==18) || (n==22))
#define IPXLEN     10
#define IPXSOCKLEN 12
#define MACLEN     6
#define MAX_FRIENDLY_NAME 80
#define PP_MAC_HASH0 19
#define PP_MAC_HASH1 13
#define PP_MAC_HASH2 7

//---[ forward declarations ]-----------------------------------------
class GenAddress;

//----[ Address class ]-----------------------------------------------

/**
 * Base class of all Address classes.
 */
class DLLOPT Address : public SnmpSyntax
{
  friend class GenAddress;

 public:
  //----[ enumerated types for address types ]---------------------------
  /**
   * Type returned by Address::get_type().
   */
  enum addr_type
  {
    type_ip,      ///< IpAddress (IPv4 or IPv6)
    type_ipx,     ///< IpxAddress
    type_udp,     ///< UdpAddress (IPv4 or IPv6)
    type_ipxsock, ///< IpxSockAddress
    type_mac,     ///< MacAddress
    type_invalid  ///< Used by GenAddress::get_type() if address is not valid
  };

  /**
   * Type returned by IpAddress::get_ip_version() and
   * UdpAddress::get_ip_version().
   */
  enum version_type
  {
    version_ipv4, ///< IPv4
    version_ipv6  ///< IPv6
  };

  /**
   * Type returned by Address::get_inet_address_type()
   */
  enum InetAddressType
  {
    e_unknown = 0,
    e_ipv4 = 1,
    e_ipv6 = 2,
    e_ipv4z = 3,
    e_ipv6z = 4,
    e_dns = 16
  };

  /**
   * Default constructor, clears the buffer and sets valid flag to false.
   */
  Address();

  /**
   * Allow destruction of derived classes.
   */
  virtual ~Address() {}

  /// overloaded equivlence operator, are two addresses equal?
  DLLOPT friend int operator==(const Address &lhs,const Address &rhs);

  /// overloaded not equivlence operator, are two addresses not equal?
  DLLOPT friend int operator!=(const Address &lhs, const Address &rhs)
    { return !(lhs == rhs); }

  /// overloaded > operator, is a1 > a2
  DLLOPT friend int operator>(const Address &lhs,const Address &rhs);

  /// overloaded >= operator, is a1 >= a2
  DLLOPT friend int operator>=(const Address &lhs,const Address &rhs)
    { if ((lhs > rhs) || (lhs == rhs)) return true;  return false; }

  /// overloaded < operator, is a1 < a2
  DLLOPT friend int operator<(const Address &lhs,const Address &rhs);

  /// overloaded <= operator, is a1 <= a2
  DLLOPT friend int operator<=(const Address &lhs, const Address &rhs)
    { if ((lhs < rhs) || (lhs == rhs)) return true; return false; }

  /// equivlence operator overloaded, are an address and a string equal?
  DLLOPT friend int operator==(const Address &lhs,const char *rhs);

  /// overloaded not equivlence operator, are an address and string not equal?
  DLLOPT friend int operator!=(const Address &lhs,const char *rhs)
    { return !(lhs == rhs); }

  /// overloaded < , is an address greater than a string?
  DLLOPT friend int operator>(const Address &lhs,const char *rhs);

  /// overloaded >=, is an address greater than or equal to a string?
  DLLOPT friend int operator>=(const Address &lhs,const char *rhs);

  /// overloaded < , is an address less than a string?
  DLLOPT friend int operator<(const Address &lhs,const char *rhs);

  /// overloaded <=, is an address less than or equal to a string?
  DLLOPT friend int operator<=(const Address &lhs,const char *rhs);

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const = 0;

  /**
   * Return if the object contains a valid address.
   *
   * @return true if the object is valid
   */
  virtual bool valid() const { return valid_flag; }

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const = 0;

  /**
   * Access as an array (read and write).
   * @note Only pass in values between 0 and get_length().
   *
   * @param position - pos to return
   * @return reference to the byte at the given position
   */
  unsigned char& operator[](const int position)
    { addr_changed = true; valid_flag = true;
      return (position < ADDRBUF) ? address_buffer[position]
                                  : address_buffer[0]; }

  /**
   * Access as an array (read only).
   * @note Only pass in values between 0 and get_length().
   *
   * @param position - pos to return
   * @return the byte at the given position
   */
  unsigned char operator[](const int position) const
    { return (unsigned char)((position < ADDRBUF) ? address_buffer[ position] : 0); }


  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const = 0;

  /**
   * Get the type of the address.
   * @see Address::addr_type
   */
  virtual addr_type get_type() const = 0;

  using SnmpSyntax::operator =;
  /**
   * Overloaded assignment operator.
   */
  virtual Address & operator = (const Address &val) = 0;
  virtual Address & operator = (const char *str) { valid_flag = parse_address(str); addr_changed = true; return *this; }

  // return a hash key
  virtual unsigned int hashFunction() const { return 0; }

 protected:
  SNMP_PP_MUTABLE bool addr_changed;
  bool valid_flag;
  unsigned char address_buffer[ADDRBUF]; // internal representation

  // parse the address string
  // redefined for each specific address subclass
  virtual bool parse_address(const char * inaddr) = 0;

  // format the output
  // redefined for each specific address subclass
  virtual void format_output() const = 0;

  /**
   * Trim of whitespaces at the start and the end of the string.
   *
   * @param ptr - string to trim
   */
  void trim_white_space(char * ptr);

  /**
   * Is this a GenAddress object.
   */
  virtual bool is_gen_address() const { return false; }

  /**
   * Reset the object.
   */
  void clear();
};


//-----------------------------------------------------------------------
//---------[ IP Address Class ]------------------------------------------
//-----------------------------------------------------------------------
class DLLOPT IpAddress : public Address
{
 public:
  /**
   * Construct an empty invalid IP address.
   */
  IpAddress();

  /**
   * Construct an IP address from a string.
   *
   * The following formats can be used:
   * - hostname with or without domain ("www.agentpp.com", "printsrv")
   * - Numerical IPv4 address ("192.168.17.1")
   * - Numerical IPv6 address ("abcd:1234::a:b:1", "::abcd:1")
   * - Numerical IPv6 address with scope ("abcd:1234::a:b:1%3", "::abcd:1%1")
   *
   * @param inaddr - Hostname or IP address
   */
  IpAddress(const char *inaddr);

  /**
   * Construct an IP address from another IP address.
   *
   * @param ipaddr - address to copy
   */
  IpAddress(const IpAddress &ipaddr);

  /**
   * Construct an IP address from a GenAddress.
   *
   * @param genaddr - address to copy
   */
  IpAddress(const GenAddress &genaddr);

  /**
   * Destructor (ensure that SnmpSyntax::~SnmpSyntax() is overridden).
   */
  ~IpAddress() {}

  using Address::operator =;

  /**
   * Map other SnmpSyntax objects to IpAddress.
   */
  virtual SnmpSyntax& operator=(const SnmpSyntax &val);

  /**
   * Map other Address objects to IpAddress.
   */
  virtual Address& operator = (const Address &val);

  /**
   * Overloaded assignment operator for other IP addresses.
   */
  virtual IpAddress& operator=(const IpAddress &ipaddress);

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *) new IpAddress(*this); }

  /**
   * Return the friendly name. Does a reverse DNS lookup for the IP address.
   *
   * @param status - The errno value for the lookup
   *
   * @return the friendly name or a zero length string (no null pointer)
   */
  char *friendly_name(int &status);

  /**
   * Get a printable ASCII value of the address.
   *
   * @return String containing the numerical address
   */
  virtual const char *get_printable() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Logically AND the address with the param.
   *
   * @param ipaddr - address to use as mask
   */
  void mask(const IpAddress &ipaddr);


  /**
   * Get the count of matching bits from the left.
   *
   * @param match_ip - address to match with
   */
  int get_match_bits(const IpAddress match_ip) const;

  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const
    { return (ip_version == version_ipv4) ? IPLEN : 
	     (have_ipv6_scope ? IP6LEN_WITH_SCOPE : IP6LEN_NO_SCOPE); }

  /**
   * Get the InetAddressType value for this address
   */
  virtual int get_inet_address_type() const
    { return (ip_version == version_ipv4) ? e_ipv4 :
         (have_ipv6_scope ? e_ipv6z : e_ipv6); }

  /**
   * Return the type of the address.
   * @see Address::addr_type
   * @return Always Address:type_ip
   */
  virtual addr_type get_type() const { return type_ip; }

  /**
   * Return the syntax.
   *
   * @return This method always returns sNMP_SYNTAX_IPADDR.
   */
  virtual SmiUINT32 get_syntax() const { return sNMP_SYNTAX_IPADDR; }

  /**
   * Return clone as binary string
   *
   * @return Pointer to the newly created OctetStr (allocated through new).
   */
  virtual OctetStr *clone_as_hex() const;

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const
    { return get_length() + 2; }

  /**
   * Return the IP version of the address.
   *
   * @return one of Address::version_type
   */
  virtual version_type get_ip_version() const { return ip_version; }

  /**
   * Map a IPv4 address to a IPv6 address.
   *
   * @return - TRUE if no error occured.
   */
  virtual bool map_to_ipv6();

  /**
   * Get the IPv6 scope
   */
  virtual unsigned int get_scope() const;

  /**
   * Set the IPv6 scope
   */
  virtual bool set_scope(const unsigned int scope);

  /**
   * Reset the object.
   */
  void clear();

  bool has_ipv6_scope() const
      { return (ip_version == version_ipv6) && have_ipv6_scope; }

 protected:
  SNMP_PP_MUTABLE char output_buffer[OUTBUFF];           // output buffer

  // friendly name storage
  char iv_friendly_name[MAX_FRIENDLY_NAME];
  int  iv_friendly_name_status;

  // redefined parse address
  // specific to IP addresses
  virtual bool parse_address(const char *inaddr);

  // redefined format output
  // specific to IP addresses
  virtual void format_output() const;

  // parse a dotted string
  int parse_dotted_ipstring(const char *inaddr);

  // parse a coloned string
  int parse_coloned_ipstring(const char *inaddr);

  // using the currently defined address, do a DNS
  // and try to fill up the name
  int addr_to_friendly();

  // support both ipv4 and ipv6 addresses
  version_type ip_version;

  bool have_ipv6_scope;
};

//------------------------------------------------------------------------
//---------[ UDP Address Class ]------------------------------------------
//------------------------------------------------------------------------
class DLLOPT UdpAddress : public IpAddress
{
 public:
  /**
   * Construct an empty invalid UDP address.
   */
  UdpAddress();

  /**
   * Construct an UDP address from a string.
   *
   * The following formats can be used additional to those recognized by
   * IpAdress:
   * - Port added to IPv4 address with '/' or ':'
   *   ("192.168.17.1:161", "192.168.17.1/161", "printsrv/161")
   * - Port added to IPv6 address with '/' or using '[...]:'
   *   ("::1/162", "[::1]/162", "[::1]:162")
   *
   * @param inaddr - Hostname or IP address
   */
  UdpAddress(const char *inaddr);

  /**
   * Construct an UDP address from another UDP address.
   *
   * @param udpaddr - address to copy
   */
  UdpAddress(const UdpAddress &udpaddr);

  /**
   * Construct an UDP address from a GenAddress.
   *
   * @param genaddr - address to copy
   */
  UdpAddress(const GenAddress &genaddr);

  /**
   * Construct an UDP address from a IP address.
   * The port will be set to 0.
   *
   * @param ipaddr - address to copy
   */
  UdpAddress(const IpAddress &ipaddr);

  /**
   * Destructor (ensure that SnmpSyntax::~SnmpSyntax() is overridden).
   */
  ~UdpAddress() {}

  using IpAddress::operator =;

  /**
   * Map other SnmpSyntax objects to UdpAddress.
   */
  virtual SnmpSyntax& operator=(const SnmpSyntax &val);

  /**
   * Map other Address objects to UdpAddress.
   */
  virtual Address & operator = (const Address &val);

  /**
   * Overloaded assignment operator for UdpAddress.
   */
  virtual UdpAddress& operator=(const UdpAddress &udpaddr);

  /**
   * Overloaded assignment operator for IpAddress.
   */
  virtual UdpAddress& operator=(const IpAddress &ipaddr);

  /**
   * Return the syntax.
   *
   * @return This method always returns sNMP_SYNTAX_OCTETS.
   */
  SmiUINT32 get_syntax() const { return sNMP_SYNTAX_OCTETS; }

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const { return get_length() + 2; }

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *) new UdpAddress(*this); }

  /**
   * Get a printable ASCII value of the address.
   *
   * @return String containing the numerical address
   */
  virtual const char *get_printable() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Set the port number.
   *
   * @note If the object is not valid(), the port may not be set.
   */
  void set_port(const unsigned short p);

  /**
   * Get the port number.
   *
   * @return The port number, or 0 is the object is not valid.
   */
  unsigned short get_port() const;

  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const
    { return (ip_version == version_ipv4) ? UDPIPLEN : 
             (have_ipv6_scope ? UDPIP6LEN_WITH_SCOPE : UDPIP6LEN_NO_SCOPE); }

  /**
   * Return the type of the address.
   * @see Address::addr_type
   * @return Always Address:type_udp
   */
  virtual addr_type get_type() const { return type_udp; }

  /**
   * Map a IPv4 UDP address to a IPv6 UDP address.
   *
   * @return - TRUE if no error occured.
   */
  virtual bool map_to_ipv6();

  /**
   * Reset the object.
   */
  void clear()
    { Address::clear(); memset(output_buffer, 0, sizeof(output_buffer)); }

  /**
   * Set the IPv6 scope
   */
  virtual bool set_scope(const unsigned int scope);

 protected:
  SNMP_PP_MUTABLE char output_buffer[OUTBUFF];           // output buffer
  char sep;                              // separator

  // redefined parse address
  // specific to IP addresses
  virtual bool parse_address(const char *inaddr);

  // redefined format output
  // specific to IP addresses
  virtual void format_output() const;
};

#ifdef _MAC_ADDRESS
//-------------------------------------------------------------------------
//---------[ 802.3 MAC Address Class ]-------------------------------------
//-------------------------------------------------------------------------
class DLLOPT MacAddress : public Address {

public:
  // constructor, no arguments
  MacAddress();

  // constructor with a string argument
  MacAddress(const char  *inaddr);

  // constructor with another MAC object
  MacAddress(const MacAddress  &macaddr);

  // construct a MacAddress with a GenAddress
  MacAddress(const GenAddress &genaddr);

  // destructor
  ~MacAddress() {}

  /**
   * Return the syntax.
   *
   * @return This method always returns sNMP_SYNTAX_OCTETS.
   */
  SmiUINT32 get_syntax() const { return sNMP_SYNTAX_OCTETS; }

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const { return MACLEN + 2; }

  using Address::operator =;
  /**
   * Map other SnmpSyntax objects to MacAddress.
   */
  virtual SnmpSyntax& operator=(const SnmpSyntax &val);

  // assignment to another IpAddress object overloaded
  MacAddress& operator=(const MacAddress &macaddress);

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *) new MacAddress(*this); }

  /**
   * Get a printable ASCII value of the address.
   *
   * @return String containing the numerical address
   */
  virtual const char *get_printable() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const { return MACLEN; }

  /**
   * Return the type of the address.
   * @see Address::addr_type
   * @return Always Address:type_mac
   */
  virtual addr_type get_type() const { return type_mac; }

  // return a hash key
  unsigned int hashFunction() const;

  /**
   * Reset the object.
   */
  void clear()
    { Address::clear(); memset(output_buffer, 0, sizeof(output_buffer)); }

 protected:
  SNMP_PP_MUTABLE char output_buffer[OUTBUFF];           // output buffer

  // redefined parse address for macs
  virtual bool parse_address(const char *inaddr);

  // redefined format output for MACs
  virtual void format_output() const;
};
#endif // _MAC_ADDRESS

#ifdef _IPX_ADDRESS
//------------------------------------------------------------------------
//---------[ IPX Address Class ]------------------------------------------
//------------------------------------------------------------------------
class DLLOPT IpxAddress : public Address {

public:
  // constructor no args
  IpxAddress();

  // constructor with a string arg
  IpxAddress(const char  *inaddr);

  // constructor with another ipx object
  IpxAddress(const IpxAddress  &ipxaddr);

  // construct with a GenAddress
  IpxAddress(const GenAddress &genaddr);

  // destructor
  ~IpxAddress() {}

  /**
   * Return the syntax.
   *
   * @return This method always returns sNMP_SYNTAX_OCTETS.
   */
  virtual SmiUINT32 get_syntax() const { return sNMP_SYNTAX_OCTETS; }

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const  { return IPXLEN + 2; }

  using Address::operator =;
  /**
   * Map other SnmpSyntax objects to IpxAddress.
   */
  virtual SnmpSyntax& operator=(const SnmpSyntax &val);

  // assignment to another IpAddress object overloaded
  virtual IpxAddress& operator=(const IpxAddress &ipxaddress);

#ifdef _MAC_ADDRESS
  // get the host id portion of an ipx address
  int get_hostid(MacAddress& mac) const;
#endif

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *) new IpxAddress(*this); }

  /**
   * Get a printable ASCII value of the address.
   *
   * @return String containing the numerical address
   */
  virtual const char *get_printable() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const { return IPXLEN; }

  /**
   * Return the type of the address.
   * @see Address::addr_type
   * @return Always Address:type_ipx
   */
  virtual addr_type get_type() const { return type_ipx; }

  /**
   * Reset the object.
   */
  void clear()
    { Address::clear(); memset(output_buffer, 0, sizeof(output_buffer)); }

 protected:
  // ipx format separator
  char separator;
  SNMP_PP_MUTABLE char output_buffer[OUTBUFF];           // output buffer

  // redefined parse address for ipx strings
  virtual bool parse_address(const char  *inaddr);

  // redefined format output for ipx strings
  // uses same separator as when constructed
  virtual void format_output() const;

};



//------------------------------------------------------------------------
//---------[ IpxSock Address Class ]--------------------------------------
//------------------------------------------------------------------------
class DLLOPT IpxSockAddress : public IpxAddress {

public:
  // constructor, no args
  IpxSockAddress();

  // constructor with a dotted string
  IpxSockAddress(const char *inaddr);

  // construct an Udp address with another Udp address
  IpxSockAddress(const IpxSockAddress &ipxaddr);

  //constructor with a GenAddress
  IpxSockAddress(const GenAddress &genaddr);

  //constructor with a IpxAddress
  // default socket # is 0
  IpxSockAddress(const IpxAddress &ipxaddr);

  // destructor
  ~IpxSockAddress() {}

  // syntax type
  //virtual SmiUINT32 get_syntax() const { return sNMP_SYNTAX_OCTETS; }

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const { return IPXSOCKLEN + 2; }

  using IpxAddress::operator =;
  /**
   * Map other SnmpSyntax objects to IpxSockAddress.
   */
  virtual SnmpSyntax& operator=(const SnmpSyntax &val);

  // assignment to another IpAddress object overloaded
  virtual IpxSockAddress& operator=(const IpxSockAddress &ipxaddr);

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *)new IpxSockAddress(*this); }

  // set the socket number
  void set_socket(const unsigned short s);

  // get the socket number
  unsigned short get_socket() const;

  /**
   * Get a printable ASCII value of the address.
   *
   * @return String containing the numerical address
   */
  virtual const char *get_printable() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const
    { if (addr_changed) format_output(); return output_buffer; }

  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const { return IPXSOCKLEN; }

  /**
   * Return the type of the address.
   * @see Address::addr_type
   * @return Always Address:type_ipxsock
   */
  virtual addr_type get_type() const { return type_ipxsock; }

  /**
   * Reset the object.
   */
  void clear()
    { Address::clear(); memset(output_buffer, 0, sizeof(output_buffer)); }

 protected:
  SNMP_PP_MUTABLE char output_buffer[OUTBUFF];           // output buffer

  // redefined parse address for ipx strings
  virtual bool parse_address(const char  *inaddr);

  // redefined format output
  // specific to IP addresses
  virtual void format_output() const;
};
#endif // _IPX_ADDRESS




//-------------------------------------------------------------------------
//--------[ Generic Address ]----------------------------------------------
//-------------------------------------------------------------------------
class DLLOPT GenAddress : public Address
{
 public:
  /**
   * Construct an empty invalid generic address object.
   */
  GenAddress();

  /**
   * Construct a generic address from a string.
   *
   * To optimize the speed of the parsing method, use_type can be used
   * to indicate that the address string is of the specified type.
   *
   * @param addr     - address string
   * @param use_type - if this value is set, the input string is only
   *                   parsed for the given type 
   */
  GenAddress(const char *addr,
	     const Address::addr_type use_type = Address::type_invalid);

  /**
   * Construct a generic address from an Address object.
   *
   * @param addr - Any address object
   */
  GenAddress(const Address &addr);

  /**
   * Construct a generic address from another generic address object.
   *
   * @param addr - Generic address object to copy
   */
  GenAddress(const GenAddress &addr);

  /**
   * Destructor, free memory.
   */
  ~GenAddress() { if (address) delete address; }

  /**
   * Return the syntax.
   *
   * @return This method returns sNMP_SYNTAX_IPADDR, sNMP_SYNTAX_OCTETS
   *         or sNMP_SYNTAX_NULL if the generic address does not have
   *         an address object.
   */
  SmiUINT32 get_syntax() const
    { return address ? address->get_syntax() : sNMP_SYNTAX_NULL; }

  /**
   * Return the space needed for serialization.
   */
  virtual int get_asn1_length() const
    { return address ? address->get_asn1_length() : 2; }

  /**
   * Clone this object.
   *
   * @return Pointer to the newly created object (allocated through new).
   */
  SnmpSyntax *clone() const { return (SnmpSyntax *)new GenAddress(*this); }

  using Address::operator =;
  /**
   * Overloaded assignment operator for a GenAddress.
   */
  virtual GenAddress& operator=(const GenAddress &addr);

  /**
   * Overloaded assignment operator for a Address.
   */
  virtual Address& operator=(const Address &addr);

  /**
   * Map other SnmpSyntax objects to GenAddress.
   */
  virtual SnmpSyntax& operator=(const SnmpSyntax &val);

  /**
   * Get a printable ASCII value of the address.
   *
   * @return String containing the numerical address
   */
  virtual const char *get_printable() const
    { return (address) ? address->get_printable() : output_buffer; }

  /**
   * Overloaded operator for streaming output.
   *
   * @return String containing the numerical address
   */
  virtual operator const char *() const
    { return address ? (const char *)*address : output_buffer; }

  /**
   * Get the length of the binary address (accessible through operator[]).
   */
  virtual int get_length() const
    { return (address) ? address->get_length() : 0; }

  /**
   * Reset the object.
   */
  void clear() { if (address) address->clear(); }

  /**
   * Return the type of the address.
   * @see Address::addr_type
   * @return Type of the contained address object or Address::type_invalid
   *         if it is not valid().
   */
  virtual addr_type get_type() const
    { return (valid()) ? address->get_type() : type_invalid; }

  /**
   * Access the protected address.
   * The caller must make sure that this GenAddress object ist valid()
   * and is of the right type (get_type()).
   */
  const IpAddress  &cast_ipaddress()  const { return (IpAddress& )*address; }

  /**
   * Access the protected address.
   * The caller must make sure that this GenAddress object ist valid()
   * and is of the right type (get_type()).
   */
  const UdpAddress &cast_udpaddress() const { return (UdpAddress&)*address; }

#ifdef _MAC_ADDRESS
  /**
   * Access the protected address.
   * The caller must make sure that this GenAddress object ist valid()
   * and is of the right type (get_type()).
   */
  const MacAddress &cast_macaddress() const { return (MacAddress&)*address; }
#endif

#ifdef _IPX_ADDRESS
  /**
   * Access the protected address.
   * The caller must make sure that this GenAddress object ist valid()
   * and is of the right type (get_type()).
   */
  const IpxAddress &cast_ipxaddress() const { return (IpxAddress&)*address; }

  /**
   * Access the protected address.
   * The caller must make sure that this GenAddress object ist valid()
   * and is of the right type (get_type()).
   */
  const IpxSockAddress &cast_ipxsockaddress() const
    { return (IpxSockAddress&)*address; }
#endif

protected:
  // pointer to a concrete address
  Address *address;
  char output_buffer[1];           // output buffer

  // redefined parse address for generic address
  virtual bool parse_address(const char *addr)
    { return parse_address(addr, Address::type_invalid); }

  virtual bool parse_address(const char *addr,
			     const Address::addr_type use_type);

  // format output for a generic address
  virtual void format_output() const {}

  /**
   * Is this a GenAddress object.
   */
  virtual bool is_gen_address() const { return true; }
};

// create AddressCollection type
typedef SnmpCollection <GenAddress> AddressCollection;
typedef SnmpCollection <UdpAddress> UdpAddressCollection;

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif  //_ADDRESS
