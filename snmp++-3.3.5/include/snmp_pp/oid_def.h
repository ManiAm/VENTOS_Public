/*_############################################################################
  _## 
  _##  oid_def.h  
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

      SNMP++ O I D _ D E F . H

      OID_DEF DEFINITIONS

      DESCRIPTION:
        Some common Oid definitions.

      DESIGN + AUTHOR:        Peter E Mellquist
=====================================================================*/
// $Id: oid_def.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _OID_DEF
#define _OID_DEF

#include "snmp_pp/oid.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif


/** SMI trap oid def */
class snmpTrapsOid: public Oid {
 public:
  DLLOPT snmpTrapsOid() : Oid("1.3.6.1.6.3.1.1.5") {};
};

/** SMI Enterprose Oid */
class snmpTrapEnterpriseOid: public Oid {
 public:
  DLLOPT snmpTrapEnterpriseOid() : Oid("1.3.6.1.6.3.1.1.4.3.0") {};
};

/** SMI Cold Start Oid */
class coldStartOid: public snmpTrapsOid {
   public:
   DLLOPT coldStartOid() { *this+=".1"; };
};

/** SMI WarmStart Oid */
class warmStartOid: public snmpTrapsOid {
   public:
   DLLOPT warmStartOid() { *this+=".2"; };
};

/** SMI LinkDown Oid */
class linkDownOid: public snmpTrapsOid {
   public:
   DLLOPT linkDownOid() { *this+=".3"; };
};

/** SMI LinkUp Oid */
class linkUpOid: public snmpTrapsOid {
   public:
   DLLOPT linkUpOid() { *this+=".4"; };
};

/** SMI Authentication Failure Oid */
class authenticationFailureOid: public snmpTrapsOid {
   public:
   DLLOPT authenticationFailureOid() { *this+=".5"; };
};

/** SMI egpneighborloss Oid */
class egpNeighborLossOid: public snmpTrapsOid {
 public:
  DLLOPT egpNeighborLossOid() { *this+=".6"; };
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _OID_DEF
