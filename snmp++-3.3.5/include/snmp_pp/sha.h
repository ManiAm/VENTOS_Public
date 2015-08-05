/*_############################################################################
  _## 
  _##  sha.h  
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

#include "snmp_pp/config_snmp_pp.h"

#if !defined(_USE_LIBTOMCRYPT) && !defined(_USE_OPENSSL)

// $Id: sha.h 2359 2013-05-09 20:07:01Z fock $
/****************************************************************
 * SHS.h  -  Secure Hash Standard (draft) FIPS 180-1            *
 *                                                              *
 * Copyright (C) 1994  Uri Blumenthal, uri@watson.ibm.com       *
 * Copyright (C) 1994  IBM T. J. Watson esearch Center          *
 *                                                              *
 * Feel free to use this code, as long as you acknowledge the   *
 * ownership by U. Blumenthal and IBM Corp. and agree to hold   *
 * both harmless in case of ANY problem you may have with this  *
 * code.                                                        *
 ****************************************************************/
#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

typedef struct {
  /* Message Digest words */
  unsigned long int h[5];
  /* Message length in bits */
  unsigned long int count[2];
  /* Current byte position in not-full-yet buf */
  int index;
  /* Buffer for the remainder of bytes mod 64 */
  unsigned char X[64];
} SHA_CTX;

DLLOPT void SHAInit(SHA_CTX *ctx);
DLLOPT void SHAUpdate(SHA_CTX *ctx, const unsigned char *buf, unsigned int lenBuf);
DLLOPT void SHAFinal(unsigned char *digest, SHA_CTX *ctx);

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // !defined(_USE_LIBTOMCRYPT) && !defined(_USE_OPENSSL)

