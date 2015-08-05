/*_############################################################################
  _## 
  _##  config_snmp_pp.h.in  
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

#ifndef _CONFIG_SNMP_PP_H_
#define _CONFIG_SNMP_PP_H_

#define SNMP_PP_VERSION_STRING "3.3.5"
#define SNMP_PP_VERSION 3
#define SNMP_PP_RELEASE 3
#define SNMP_PP_PATCHLEVEL 5

//! The maximum size of a message that can be sent or received.
#define MAX_SNMP_PACKET 4096

#ifndef DLLOPT
#if defined (WIN32) && defined (SNMP_PP_DLL)
#ifdef SNMP_PP_EXPORTS
#define DLLOPT __declspec(dllexport)
#define DLLOPT_TEMPL
#else
#define DLLOPT __declspec(dllimport)	
#define DLLOPT_TEMPL extern
#endif
#else
#define DLLOPT
#define DLLOPT_TEMPL
#endif
#endif

#ifndef ON
#define ON 1
#endif

/*
 * some permanent parts from autoconf process
 */
#if 1
#define _SNMPv3 1
#else
#define _NO_SNMPv3 1
#endif
#if 1
#define SNMP_PP_IPv6 1
#endif
#if 1
#define ENABLE_THREADS 1
#else
#define _NO_THREADS
#endif
#if 1
#define HAVE_LIBSSL 1
#endif
#if 0
#define HAVE_LIBTOMCRYPT 1
#endif
#if 0
#define HAVE_LIBDES 1
#endif
#if 1
#define HAVE_PTHREAD 1
#endif

// define SNMP_PP_NAMESPACE to enclose all library names in Snmp_pp namespace
#if 1
#define SNMP_PP_NAMESPACE
#else
#undef SNMP_PP_NAMESPACE
#endif

// If you do not use SNMP++ for commercial purposes or if you
// have licensed IDEA (read README.v3) you may define the following
// to enable IDEA support. (note this is not defined by a rfc)
// #define _USE_IDEA

#if defined(_SNMPv3) || !defined(_NO_SNMPv3)
#  if defined(HAVE_LIBSSL)
#    define _USE_OPENSSL
#  elif defined(HAVE_LIBTOMCRYPT)
#    define _USE_LIBTOMCRYPT
#  elif HAVE_LIBDES
#    define _USE_3DES_EDE
#  else
#    warn No crypto library found - disable SNMPv3
#    undef _SNMPv3
#    define _NO_SNMPv3
#  endif
#endif

// define _NO_LOGGING if you do not want any logging output 
// (increases performance drastically and minimizes memory consumption)
#if 1
#undef _NO_LOGGING
#else
#define _NO_LOGGING
#endif

#if 1
#ifndef WITHOUT_LOG_PROFILES
#define WITH_LOG_PROFILES 1
#endif
#else
#undef WITH_LOG_PROFILES
#endif

// define _IPX_ADDRESS and/or _MAC_ADDRESS if you want to use the
// classess IpxAddress/IpxSockAddress and/or MacAddress
#if 0
#define _MAC_ADDRESS
#else
#undef _MAC_ADDRESS
#endif
#if 0
#define _IPX_ADDRESS
#else
#undef _IPX_ADDRESS
#endif

// define this if you want to send out broadcasts
#define SNMP_BROADCAST

// Some socket types
#if !(defined (CPU) && CPU == PPC603) && (defined __GNUC__ || defined __FreeBSD__ || defined _AIX) && ! defined __MINGW32__
  typedef socklen_t SocketLengthType;
#else
  typedef int SocketLengthType;
#endif

#ifdef SNMP_PP_IPv6
  typedef struct sockaddr_storage SocketAddrType;
#else
  typedef struct sockaddr_in SocketAddrType;
#endif

// Not fully tested!
//#define HAVE_POLL_SYSCALL

// Some older(?) compilers need a special declaration of
// template classes
// #define _OLD_TEMPLATE_COLLECTION

// can we use the reentrant version of these functions or
// are the standard functions thread safe
#ifdef __CYGWIN32__
#define HAVE_REENTRANT_LOCALTIME
#define HAVE_REENTRANT_GETHOSTBYADDR
#define HAVE_REENTRANT_GETHOSTBYNAME
#elif __MINGW32__
//FIXME: snmp++/src/address.cpp:865: error: `inet_ntop' was not declared in this scope
//FIXME: snmp++/src/address.cpp:988: error: `inet_pton' was not declared in this scope
//FIXME: snmp++/src/notifyqueue.cpp:538: error: `inet_pton' was not declared in this scope
#define HAVE_REENTRANT_GETHOSTBYNAME
#define HAVE_REENTRANT_LOCALTIME
#define HAVE_REENTRANT_GETHOSTBYADDR
#elif __DECCXX
#define HAVE_REENTRANT_GETHOSTBYNAME
#define HAVE_REENTRANT_GETHOSTBYADDR
#elif __HP_aCC
#define HAVE_REENTRANT_GETHOSTBYNAME
#define HAVE_REENTRANT_GETHOSTBYADDR
#elif _MSC_VER
#define HAVE_REENTRANT_GETHOSTBYNAME
#define HAVE_REENTRANT_LOCALTIME
#define HAVE_REENTRANT_GETHOSTBYADDR
#elif _AIX
#define HAVE_REENTRANT_GETHOSTBYNAME
#define HAVE_REENTRANT_GETHOSTBYADDR
#endif

// Define a unsigned 64 bit integer:
#ifdef WIN32
#include <windows.h>
#if defined(HAVE_WINSOCK2_H)
#include <winsock2.h>
#elif defined(HAVE_WINSOCK_H)
#include <winsock.h>
#endif
#ifdef HAVE_WSTCPIP_H
#include <ws2tcpip.h>
#endif
#ifdef HAVE_WSPIAPI_H
#include <wspiapi.h>
#endif
#ifdef __BCPLUSPLUS__
typedef unsigned __int64 pp_uint64;
#else
typedef ULONGLONG pp_uint64;
#endif
#else // not WIN32
typedef unsigned long long pp_uint64;
#endif

// Define a type used for sockets
#ifdef _MSC_VER
    typedef SOCKET SnmpSocket;
#else
    typedef int SnmpSocket;
#endif

#ifdef HAVE_POLL_SYSCALL
#include <poll.h>
#endif

#define SNMP_PP_DEFAULT_SNMP_PORT      161 // standard port # for SNMP
#define SNMP_PP_DEFAULT_SNMP_TRAP_PORT 162 // standard port # for SNMP traps

///////////////////////////////////////////////////////////////////////
// Changes below this line should not be necessary
///////////////////////////////////////////////////////////////////////


// Make use of mutable keyword
//#define SNMP_PP_MUTABLE mutable
#define SNMP_PP_MUTABLE

#define SAFE_INT_CAST(expr)  ((int)(expr))
#define SAFE_UINT_CAST(expr) ((unsigned int)(expr))

// Safe until 32 bit second counter wraps to zero (time functions)
#define SAFE_LONG_CAST(expr)  ((long)(expr))
#define SAFE_ULONG_CAST(expr) ((unsigned long)(expr))

#ifdef ENABLE_THREADS
#ifdef WIN32

#ifndef _THREADS
#define _WIN32THREADS
#define VC_EXTRALEAN
#define _THREADS
#endif

#else  // !WIN32

#ifndef _THREADS
#define _THREADS
#endif

#ifdef __APPLE__
#ifndef __unix
#define __unix
#endif
#endif

#ifndef POSIX_THREADS
#ifdef HAVE_PTHREAD
#define POSIX_THREADS
#endif
#endif

#endif // WIN32
#endif // ENABLE_THREADS

#endif // _CONFIG_SNMP_PP_H_
