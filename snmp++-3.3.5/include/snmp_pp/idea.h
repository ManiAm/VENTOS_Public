/*_############################################################################
  _## 
  _##  idea.h  
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
// $Id: idea.h 2359 2013-05-09 20:07:01Z fock $

/*

idea.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Jun 25 04:44:30 1995 ylo

The IDEA encryption algorithm.

*/

#ifndef IDEA_H
#define IDEA_H

#include "snmp_pp/config_snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifdef _USE_IDEA

typedef unsigned short word16;
typedef unsigned int word32;

typedef struct
{
  word16 key_schedule[52];
} IDEAContext;

/* Sets idea key for encryption. */
void idea_set_key(IDEAContext *c, const unsigned char key[16]);

/* Destroys any sensitive data in the context. */
void idea_destroy_context(IDEAContext *c);

/* Performs the IDEA cipher transform on a block of data. */
void idea_transform(IDEAContext *c, word32 l, word32 r, word32 *output);

/* Encrypts len bytes from src to dest in CFB mode.  Len need not be a multiple
   of 8; if it is not, iv at return will contain garbage.
   Otherwise, iv will be modified at end to a value suitable for continuing
   encryption. */
void idea_cfb_encrypt(IDEAContext *c, unsigned char *iv, unsigned char *dest,
		      const unsigned char *src, unsigned int len);


/* Decrypts len bytes from src to dest in CFB mode.  Len need not be a multiple
   of 8; if it is not, iv at return will contain garbage.
   Otherwise, iv will be modified at end to a value suitable for continuing
   decryption. */
void idea_cfb_decrypt(IDEAContext *c, unsigned char *iv, unsigned char *dest,
		      const unsigned char *src, unsigned int len);

#endif /* IDEA_H */
#endif /* _USE_IDEA */

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

/*

getput.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jun 28 22:36:30 1995 ylo

Macros for storing and retrieving data in msb first and lsb first order.

*/

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifndef GETPUT_H
#define GETPUT_H

#ifdef _USE_IDEA

/*------------ macros for storing/extracting msb first words -------------*/

#define GET_32BIT(cp) (((unsigned long)(unsigned char)(cp)[0] << 24) | \
  		       ((unsigned long)(unsigned char)(cp)[1] << 16) | \
		       ((unsigned long)(unsigned char)(cp)[2] << 8) | \
		       ((unsigned long)(unsigned char)(cp)[3]))

#define GET_16BIT(cp) (((unsigned long)(unsigned char)(cp)[0] << 8) | \
		       ((unsigned long)(unsigned char)(cp)[1]))

#define PUT_32BIT(cp, value) do { \
  (cp)[0] = (value) >> 24; \
  (cp)[1] = (value) >> 16; \
  (cp)[2] = (value) >> 8; \
  (cp)[3] = (value); } while (0)

#define PUT_16BIT(cp, value) do { \
  (cp)[0] = (value) >> 8; \
  (cp)[1] = (value); } while (0)

/*------------ macros for storing/extracting lsb first words -------------*/

#define GET_32BIT_LSB_FIRST(cp) \
  (((unsigned long)(unsigned char)(cp)[0]) | \
  ((unsigned long)(unsigned char)(cp)[1] << 8) | \
  ((unsigned long)(unsigned char)(cp)[2] << 16) | \
  ((unsigned long)(unsigned char)(cp)[3] << 24))

#define GET_16BIT_LSB_FIRST(cp) \
  (((unsigned long)(unsigned char)(cp)[0]) | \
  ((unsigned long)(unsigned char)(cp)[1] << 8))

#define PUT_32BIT_LSB_FIRST(cp, value) do { \
  (cp)[0] = (value); \
  (cp)[1] = (value) >> 8; \
  (cp)[2] = (value) >> 16; \
  (cp)[3] = (value) >> 24; } while (0)

#define PUT_16BIT_LSB_FIRST(cp, value) do { \
  (cp)[0] = (value); \
  (cp)[1] = (value) >> 8; } while (0)

#endif // _USE_IDEA

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif /* GETPUT_H */

