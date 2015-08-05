/*_############################################################################
  _## 
  _##  snmpmsg.h  
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


  SNMP++ S N M P M E S S A G E . H

  SNMPMESSAGE CLASS DEFINITION

  DESIGN + AUTHOR:  Peter E Mellquist

  DESCRIPTION:
  ASN.1	encoding / decoding class
      		
=====================================================================*/
// $Id: snmpmsg.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _SNMPMSG
#define _SNMPMSG

#include "snmp_pp/config_snmp_pp.h"

#include "snmp_pp/smival.h"
#include "snmp_pp/pdu.h"
#include "snmp_pp/target.h"
#include "snmp_pp/asn1.h"
#include "snmp_pp/mp_v3.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

DLLOPT void freeSmivalDescriptor( SmiVALUE* );
DLLOPT int convertVbToSmival( const Vb&, SmiVALUE* );

#define SNMP_MSG_OID_SYSUPTIME "1.3.6.1.2.1.1.3.0"
#define SNMP_MSG_OID_TRAPID    "1.3.6.1.6.3.1.1.4.1.0"

class Snmp;

// SnmpMessage Class
class DLLOPT SnmpMessage
{
 public:

  // construct a SnmpMessage object
  SnmpMessage() : bufflen(MAX_SNMP_PACKET), valid_flag(false) {};
	// load up using a Pdu, community and SNMP version
	// performs ASN.1 serialization
	// result status returned
 private:
	int load( const Pdu &pdu,                // Pdu to serialize
                  const OctetStr &community,     // community name to use
                  const snmp_version version,    // SNMP version, v1 or v2
                  const OctetStr *engine_id,     // optional v3
                  const OctetStr *security_name, // optional v3
                  const int security_model);     // optional v3
 public:
	int load( const Pdu &pdu,              // Pdu to serialize
                  const OctetStr &community,   // community name to use
                  const snmp_version version)  // SNMP version, v1 or v2
	  { return load(pdu, community, version, 0, 0, 0); };

	// load up message using ASN.1 data stream
	// status is returned
	int load( unsigned char *data,         // data to be loaded
		  unsigned long len);	       // len of data to be loaded

	// unload ( unserialize ) into SNMP++ Pdu, community and version
	// status is returned
 private:
	int unload( Pdu &pdu,                    // Pdu returned
                    OctetStr &community,         // community name
                    snmp_version &version,       // version
                    OctetStr *engine_id,         // optional v3
                    OctetStr *security_name, // optional v3
                    long int *security_model,
		    UdpAddress *from_addr,
                    Snmp *snmp_session);
 public:
	int unload( Pdu &pdu,                    // Pdu returned
                    OctetStr &community,         // community name
                    snmp_version &version)       // version
	  { return unload(pdu, community, version, 0, 0, 0, 0, 0); };


#ifdef _SNMPv3
	int loadv3( const Pdu &pdu,               // Pdu to serialize
                    const OctetStr &engine_id,    // engine_id to use
                    const OctetStr &sec_name,     // securit_name to use
                    const int sec_model,          // security_model to use
                    const snmp_version version)   // SNMP version, v3
	{ return load(pdu, "", version, &engine_id, &sec_name, sec_model); }

	int unloadv3( Pdu &pdu,                  // Pdu returned
                      snmp_version &version,     // version
                      OctetStr &engine_id,       // optional v3
                      OctetStr &security_name,   // optional v3
                      long int &security_model,
		      UdpAddress &from_addr,
                      Snmp &snmp_session);

        // returns TRUE if the message in the buffer is a v3 message
        bool is_v3_message() {return v3MP::is_v3_msg(databuff, (int)bufflen);};

#endif

	// return the validity of the message
	bool valid() const         { return valid_flag;};

	// return raw data
	// check validity
	unsigned char *data()     { return databuff; };

	// returns len
	// check validity
	unsigned long len() const { return bufflen; };

protected:

	unsigned char databuff[MAX_SNMP_PACKET];
	unsigned int bufflen;
	bool valid_flag;
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif  // _SNMPMSG
