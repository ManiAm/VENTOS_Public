/*_############################################################################
  _## 
  _##  v3.h  
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
// $Id: v3.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _V3_H
#define _V3_H

#include <stdio.h>
#include <stdarg.h>

#include "snmp_pp/config_snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

class OctetStr;

/** @name SNMPv3 Security Model values
 */
//@{
#define SNMP_SECURITY_MODEL_ANY  0 //!< Not used in SNMP++.
#define SNMP_SECURITY_MODEL_V1   1 //!< Can be used for SNMPv1 only.
#define SNMP_SECURITY_MODEL_V2   2 //!< Can be used for SNMPv2 only.
#define SNMP_SECURITY_MODEL_USM  3 //!< Can be used for SNMPv3 only.
//@}

/**
 * Set the amount of log messages you want to get. To disable all
 * messages, set the level to -1
 *
 * @param db_level - New level
 */
DLLOPT void debug_set_level(const int db_level);

#ifdef _DEBUG

/**
 * SNMP++ logging function.
 *
 * The default is to log all messages with a level < 19. To change
 * that either edit v3.cpp or use a "extern unsigned int debug_level"
 * to change the level.
 *
 * @param db_level - Priority of the message (0 = high)
 * @param format   - Just like printf
 */
DLLOPT void debugprintf(int db_level, const char *format, ...);

/**
 * SNMP++ logging function for hex strings.
 *
 * @param db_level - Priority of the message (0 = high)
 * @param comment  - Comment printed before the hex dump (may be 0)
 * @param data     - pointer to the hex data
 * @param len      - length of the hex data
 */
DLLOPT void debughexcprintf(int db_level, const char* comment,
                            const unsigned char *data, const unsigned int len);

//! Wrapper for debughexcprintf() without comment.
#define debughexprintf(db_level, data, len) \
                      debughexcprintf(db_level, NULL, data, len);

#else

#ifndef _MSC_VER
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define debugprintf(db_level,format...)
#else
void debugprintf(int db_level, const char *format, ...);
#endif
#else
// disable any warning for wrong number of arguments in macro
#pragma warning(disable:4002)
#define debugprintf(db_level,format)
#endif //_MSC_VER

#define debughexprintf( db_level,          data, len)
#define debughexcprintf(db_level, comment, data, len)

#endif

#ifdef _SNMPv3

#define MAXLENGTH_ENGINEID       32
#define MAXLENGTH_CONTEXT_NAME   32
#define MAXLENGTH_FILENAME       255
#define MAXLENGTH_GLOBALDATA     42 // (2 + 1) + 7 + 7 + 3 + 7 + security

#define oidV3SnmpEngine                 "1.3.6.1.6.3.10.2.1"
#define oidV3SnmpEngineID               "1.3.6.1.6.3.10.2.1.1.0"
#define oidV3SnmpEngineBoots            "1.3.6.1.6.3.10.2.1.2.0"
#define oidV3SnmpEngineTime             "1.3.6.1.6.3.10.2.1.3.0"
#define oidV3SnmpEngineMaxMessageSize   "1.3.6.1.6.3.10.2.1.4.0"

// also defined in agent++/include/vacm.h
#ifndef oidSnmpUnavailableContexts
#define oidSnmpUnavailableContexts        "1.3.6.1.6.3.12.1.4.0"
#define oidSnmpUnknownContexts            "1.3.6.1.6.3.12.1.5.0"
#endif

/** @name Error codes (storing engineBoots)
 *
 * These values are returned by getBootCounter() and saveBootCounter().
 */
//@{
#define SNMPv3_OK                 0 //!< No error
#define SNMPv3_NO_ENTRY_ERROR    -1 //!< No line for the engine id found
#define SNMPv3_FILEOPEN_ERROR    -2 //!< Unable to open file
#define SNMPv3_TOO_LONG_ERROR    -3 //!< The given engineID is too long
#define SNMPv3_FILE_ERROR        -4 //!< The given file contains a wrong line
#define SNMPv3_FILECREATE_ERROR  -5 //!< The File could not be created
#define SNMPv3_FILERENAME_ERROR  -6 //!< Error renaming the temporary file 
//@}

/**
 * Compare two strings.
 *
 * @param str1    - The first byte array
 * @param ptr1len - Length of first array
 * @param str2    - The second byte array
 * @param ptr2len - Length of second array
 *
 * @return 1 if the strings are identical, 0 if not.
 */
DLLOPT int unsignedCharCompare(const unsigned char *str1,
                               const long int ptr1len,
                               const unsigned char *str2,
                               const long int ptr2len);

/**
 * String copy function.
 *
 * @note The returned string has to be deleted with "delete []".
 *
 * @param src    - Source string
 * @param srclen - Length of source string
 *
 * @return Pointer to a null terminated copy of src (or 0 on error).
 */
DLLOPT unsigned char *v3strcpy(const unsigned char *src, const int srclen);

/**
 * Encode the given string into the output buffer. For each byte
 * of the string two bytes in the output buffer are used. The
 * output buffer will contain chars from 0x64 to 0x79.
 *
 * @param in        - The string (for example engine id) to encode
 * @param in_length - The length of the engineID
 * @param out       - The output buffer for the encoded string, must have
 *                    lenth 2 * in_length
 */
DLLOPT void encodeString(const unsigned char* in, const int in_length,
			 char* out);

/**
 * Decode the given encoded string into the output buffer.
 *
 * @param in        - The encoded string
 * @param in_length - The length of the encoded string
 * @param out       - Buffer for the decoded string (half size of input
 *                    string). The String will be null terminated.
 */
DLLOPT void decodeString(const unsigned char* in, const int in_length,
			 char* out);

/**
 * Read the bootCounter of the given engineID stored in the given file.
 *
 * @param fileName - The name of the file
 * @param engineId - Read the bootCounter for this enigneID
 * @param boot     - OUT: the bootCounter that was read
 *
 * @return One of SNMPv3_OK, SNMPv3_TOO_LONG_ERROR, SNMPv3_FILE_ERROR,
 *         SNMPv3_NO_ENTRY_ERROR, SNMPv3_FILEOPEN_ERROR
 *
 */
DLLOPT int getBootCounter(const char *fileName,
                          const OctetStr &engineId, unsigned int &boot);

/**
 * Store the bootCounter of the given engineID in the given file.
 *
 * @param fileName - The name of the file
 * @param engineId - Store the bootCounter for this enigneID
 * @param boot     - The bootCounter
 *
 * @return One of SNMPv3_OK, SNMPv3_FILEOPEN_ERROR, SNMPv3_FILECREATE_ERROR,
 *         SNMPv3_FILERENAME_ERROR.
 *
 */
DLLOPT int saveBootCounter(const char *fileName,
                           const OctetStr &engineId, const unsigned int boot);


#endif // _SNMPv3

/**
 * Tool class for easy allocation of buffer space.
 */
template <class T> class Buffer
{
 public:
    /// Constructor: Allocate a buffer for size objects.
    Buffer(const unsigned int size)
    {
	ptr = new T[size];
	if (ptr)
	  len = size;
	else
	  len = 0;
    }

    /// Destructor: Free allocated buffer
    ~Buffer()
     {
	 if (ptr) delete [] ptr;
     }

    /// Get the buffer pointer
    T *get_ptr()
    {
	return ptr;
    }

    /// Overwrite the buffer space with zero.
    void clear()
    {
	if (ptr)
	    memset(ptr, 0, len * sizeof(T));
    }

 private:
    T *ptr;
    unsigned int len;
};

// only for compatibility do not use these values:
#define SecurityModel_any SNMP_SECURITY_MODEL_ANY
#define SecurityModel_v1  SNMP_SECURITY_MODEL_V1
#define SecurityModel_v2  SNMP_SECURITY_MODEL_V2
#define SecurityModel_USM SNMP_SECURITY_MODEL_USM

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _V3_H
