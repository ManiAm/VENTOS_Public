/*_############################################################################
  _## 
  _##  smi.h  
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


  SNMP++ S M I . H

  SMI DEFINITIONS

  AUTHOR:           Peter E Mellquist
=====================================================================*/
// $Id: smi.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _SMIDEF
#define _SMIDEF

// make sure configuration is included first
#include "snmp_pp/config_snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#define WINFAR
#define STRCAT strcat
#define STRLEN strlen
#define MEMCPY memcpy
#define STRCPY strcpy
#define MEMCMP memcmp
#define XPORT

//----------[ ASN/BER Base Types ]-----------------------------------------
/** @name ASN/BER Base Types
 *
 * Basic Encoding Rules (BER) (used in forming SYNTAXes and certain
 * SNMP types/values).
 */
//@{
#define aSN_UNIVERSAL    (0x00)
#define aSN_APPLICATION  (0x40)
#define aSN_CONTEXT      (0x80)
#define aSN_PRIVATE      (0xC0)
#define aSN_PRIMITIVE    (0x00)
#define aSN_CONSTRUCTOR  (0x20)
//@}

//------[ SNMP ObjectSyntax Values ]---------------------------------------
#define sNMP_SYNTAX_SEQUENCE  (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x10)

/** @name Syntax Types
 *
 * These values are used in the "syntax" member of the smiVALUE
 * structure which follows.
 *
 * The get_syntax() method of any class derived from SnmpSyntax returns
 * one of these values.
 *
 * @note UInt32 is indistinguishable from Gauge32 per SNMPv2 Draft Standard
 * @note NsapAddr is obsoleted as unique SMI type per SNMPv2 Draft Standard
 */
//@{
#define sNMP_SYNTAX_INT		(aSN_UNIVERSAL | aSN_PRIMITIVE | 0x02)
#define sNMP_SYNTAX_BITS	(aSN_UNIVERSAL | aSN_PRIMITIVE | 0x03)
#define sNMP_SYNTAX_OCTETS	(aSN_UNIVERSAL | aSN_PRIMITIVE | 0x04)
#define sNMP_SYNTAX_NULL	(aSN_UNIVERSAL | aSN_PRIMITIVE | 0x05)
#define sNMP_SYNTAX_OID		(aSN_UNIVERSAL | aSN_PRIMITIVE | 0x06)
#define sNMP_SYNTAX_INT32	sNMP_SYNTAX_INT
#define sNMP_SYNTAX_IPADDR	(aSN_APPLICATION | aSN_PRIMITIVE | 0x00)
#define sNMP_SYNTAX_CNTR32	(aSN_APPLICATION | aSN_PRIMITIVE | 0x01)
#define sNMP_SYNTAX_GAUGE32	(aSN_APPLICATION | aSN_PRIMITIVE | 0x02)
#define sNMP_SYNTAX_TIMETICKS	(aSN_APPLICATION | aSN_PRIMITIVE | 0x03)
#define sNMP_SYNTAX_OPAQUE	(aSN_APPLICATION | aSN_PRIMITIVE | 0x04)
#define sNMP_SYNTAX_CNTR64	(aSN_APPLICATION | aSN_PRIMITIVE | 0x06)
#define sNMP_SYNTAX_UINT32	sNMP_SYNTAX_GAUGE32
//@}

//-------------------------------------------------------------------------

//---------------[ Exception conditions for SNMPv2 ]-----------------------
/** @name Exception conditions for SNMPv2 */
//@{
#define sNMP_SYNTAX_NOSUCHOBJECT    (aSN_CONTEXT | aSN_PRIMITIVE | 0x00)
#define sNMP_SYNTAX_NOSUCHINSTANCE  (aSN_CONTEXT | aSN_PRIMITIVE | 0x01)
#define sNMP_SYNTAX_ENDOFMIBVIEW    (aSN_CONTEXT | aSN_PRIMITIVE | 0x02)
//@}

//--------------[ different types of PDU's ]-------------------------------
/** @name Pdu types */
//@{
#define sNMP_PDU_GET	    (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x0)
#define sNMP_PDU_GETNEXT    (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x1)
#define sNMP_PDU_RESPONSE   (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x2)
#define sNMP_PDU_SET	    (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x3)
#define sNMP_PDU_V1TRAP     (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x4)
#define sNMP_PDU_GETBULK    (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x5)
#define sNMP_PDU_INFORM     (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x6)
#define sNMP_PDU_TRAP       (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x7)
#define sNMP_PDU_REPORT     (aSN_CONTEXT | aSN_CONSTRUCTOR | 0x8)
//@}


//------[ smi typedefs ]---------------------------------------------------
/** @name SMI typedefs
 *
 * SNMP-related types from RFC1442 (SMI).
 */
//@{

// byte
typedef unsigned char    SmiBYTE,       WINFAR *SmiLPBYTE;

// int
typedef long             SmiINT,        WINFAR *SmiLPINT;

// int 32
typedef SmiINT           SmiINT32,      WINFAR *SmiLPINT32;

// unit32
typedef unsigned long    SmiUINT32,     WINFAR *SmiLPUINT32;

// octet struct
typedef struct {
     SmiUINT32 len;
     SmiLPBYTE ptr;}     SmiOCTETS,     WINFAR *SmiLPOCTETS;

// bits
typedef SmiOCTETS        SmiBITS,       WINFAR *SmiLPBITS;

// SMI oid struct
typedef struct {
     SmiUINT32   len;
     SmiLPUINT32 ptr;}   SmiOID,        WINFAR *SmiLPOID;

// ipaddr
typedef SmiOCTETS        SmiIPADDR,     WINFAR *SmiLPIPADDR;

// 32bit counter
typedef SmiUINT32        SmiCNTR32,     WINFAR *SmiLPCNTR32;

// gauge
typedef SmiUINT32        SmiGAUGE32,    WINFAR *SmiLPGAUGE32;

// timeticks
typedef SmiUINT32        SmiTIMETICKS,  WINFAR *SmiLPTIMETICKS;

// opaque
typedef SmiOCTETS        SmiOPAQUE,     WINFAR *SmiLPOPAQUE;

// nsapaddr
typedef SmiOCTETS        SmiNSAPADDR,   WINFAR *SmiLPNSAPADDR;

// 64 bit counter
typedef struct {
        SmiUINT32 hipart;
        SmiUINT32 lopart;} SmiCNTR64,   WINFAR *SmiLPCNTR64;
//@}

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif


