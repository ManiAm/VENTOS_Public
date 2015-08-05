/*_############################################################################
  _## 
  _##  asn1.h  
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
// $Id: asn1.h 2543 2014-01-24 13:17:46Z fock $

#ifndef _ASN1
#define _ASN1

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/target.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifndef EIGHTBIT_SUBIDS
typedef unsigned long oid;
#define MAX_SUBID   0xFFFFFFFF
#else
typedef unsigned char oid;
#define MAX_SUBID   0xFF
#endif

#define MAX_OID_LEN      128 /* max subid's in an oid */

// asn.1 values
#define ASN_BOOLEAN      (0x01)
#ifndef ASN_INTEGER
#define ASN_INTEGER      (0x02)
#endif
#define ASN_BIT_STR      (0x03)
#define ASN_OCTET_STR    (0x04)
#ifndef ASN_NULL
#define ASN_NULL         (0x05)
#endif
#define ASN_OBJECT_ID    (0x06)
#ifndef ASN_SEQUENCE
#define ASN_SEQUENCE     (0x10)
#endif
#define ASN_SET          (0x11)
#ifndef ASN_UNIVERSAL
#define ASN_UNIVERSAL    (0x00)
#endif
#ifndef ASN_APPLICATION
#define ASN_APPLICATION  (0x40)
#endif
#ifndef ASN_CONTEXT
#define ASN_CONTEXT      (0x80)
#endif
#ifndef ASN_PRIVATE
#define ASN_PRIVATE      (0xC0)
#endif
#ifndef ASN_PRIMITIVE
#define ASN_PRIMITIVE    (0x00)
#endif
#ifndef ASN_CONSTRUCTOR
#define ASN_CONSTRUCTOR  (0x20)
#endif
#define ASN_LONG_LEN     (0x80)
#define ASN_EXTENSION_ID (0x1F)
#define ASN_BIT8         (0x80)

#define IS_CONSTRUCTOR(byte)  ((byte) & ASN_CONSTRUCTOR)
#define IS_EXTENSION_ID(byte) (((byte) & ASN_EXTENSION_ID) == ASN_EXTENSION_ID)

#define ASN_UNI_PRIM (ASN_UNIVERSAL | ASN_PRIMITIVE)
#define ASN_SEQ_CON  (ASN_SEQUENCE | ASN_CONSTRUCTOR)

#define ASN_MAX_NAME_LEN   128
#define SNMP_VERSION_1      0
#define SNMP_VERSION_2C     1
#define SNMP_VERSION_2STERN 2
#define SNMP_VERSION_3      3

// defined types (from the SMI, RFC 1065)
#define SMI_IPADDRESS       (ASN_APPLICATION | 0)
#define SMI_COUNTER         (ASN_APPLICATION | 1)
#define SMI_GAUGE           (ASN_APPLICATION | 2)
#define SMI_TIMETICKS       (ASN_APPLICATION | 3)
#define SMI_OPAQUE          (ASN_APPLICATION | 4)
#define SMI_NSAP            (ASN_APPLICATION | 5)
#define SMI_COUNTER64       (ASN_APPLICATION | 6)
#define SMI_UINTEGER        (ASN_APPLICATION | 7)

#define GET_REQ_MSG         (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x0)
#define GETNEXT_REQ_MSG     (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x1)
#define GET_RSP_MSG         (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x2)
#define SET_REQ_MSG         (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x3)
#define TRP_REQ_MSG         (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4)

#define GETBULK_REQ_MSG     (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x5)
#define INFORM_REQ_MSG      (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x6)
#define TRP2_REQ_MSG        (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x7)
#define REPORT_MSG          (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x8)

#define SNMP_NOSUCHOBJECT   (ASN_CONTEXT | ASN_PRIMITIVE | 0x0)
#define SNMP_NOSUCHINSTANCE (ASN_CONTEXT | ASN_PRIMITIVE | 0x1)
#define SNMP_ENDOFMIBVIEW   (ASN_CONTEXT | ASN_PRIMITIVE | 0x2)


#ifdef _DEBUG
#define ASNERROR(string)    debugprintf(3, "ASN parse error (%s)\n", string )
#else
#define ASNERROR(string)
#endif


typedef struct sockaddr_in  ipaddr;

// pdu
struct snmp_pdu {
    int        command;      // pdu type
    unsigned long  reqid;    // Request id
#ifdef _SNMPv3
    unsigned long  msgid;
    unsigned long  maxsize_scopedpdu;
#endif
    unsigned long  errstat;  // Error status
    unsigned long  errindex; // Error index

    // Trap information
    oid        *enterprise;   // System OID
    int        enterprise_length;
    ipaddr  agent_addr;       // address of object generating trap
    int        trap_type;     // trap type
    int        specific_type; // specific type
    unsigned long  time;      // Uptime

    // vb list
    struct variable_list *variables;
};

// vb list
struct variable_list {
    struct variable_list *next_variable;    // NULL for last variable
    oid        *name;                       // Object identifier of variable
    int        name_length;                 // number of subid's in name
    unsigned char   type;                   // ASN type of variable
    union {                                 // value of variable
    long    *integer;
    unsigned char     *string;
    oid    *objid;
    unsigned char   *bitstring;
    struct counter64 *counter64;
    } val;
    int        val_len;
};

struct counter64 {
    unsigned long high;
    unsigned long low;
};


// prototypes for encoding routines
DLLOPT unsigned char *asn_parse_int(unsigned char *data, int *datalength,
                                    unsigned char *type,
                                    long *intp);


inline unsigned char *asn_parse_int(unsigned char *data, int *datalength,
                                    unsigned char *type,
                                    unsigned long *intp)
{ return asn_parse_int(data, datalength, type, (long*)intp); }


DLLOPT unsigned char *asn_parse_unsigned_int(unsigned char *data,        
                                             int *datalength,
                                             unsigned char *type,
                                             unsigned long *intp);
inline unsigned char *asn_parse_unsigned_int(unsigned char *data,        
                                             int *datalength,
                                             unsigned char *type,
                                             long *intp)
{ return asn_parse_unsigned_int(data, datalength, type, (unsigned long*)intp); }

DLLOPT unsigned char *asn_build_int(unsigned char *data, int *datalength,
                                    const unsigned char type,
                                    const long *intp);

inline unsigned char *asn_build_int(unsigned char *data, int *datalength,
                                    const unsigned char type,
                                    const unsigned long *intp)
{ return asn_build_int(data, datalength, type, (const long*)intp); }

DLLOPT unsigned char *asn_build_unsigned_int(unsigned char *data,
                                             int *datalength,
                                             unsigned char type,
                                             unsigned long *intp);

DLLOPT unsigned char *asn_parse_string(unsigned char *data, int *datalength,
                                       unsigned char *type,
                                       unsigned char *string,
                                       int *strlength);

DLLOPT unsigned char *asn_build_string(unsigned char *data, int *datalength,
                                       const unsigned char type,
                                       const unsigned char *string,
                                       const int strlength);

DLLOPT unsigned char *asn_parse_header(unsigned char *data, int *datalength,
                                       unsigned char *type);

DLLOPT unsigned char *asn_build_header(unsigned char *data, int *datalength,
                                       unsigned char type, int length);

DLLOPT unsigned char *asn_build_sequence(unsigned char *data,
                                         int *datalength,
                                         unsigned char type,
                                         int length);

DLLOPT unsigned char *asn_parse_length(unsigned char *data,
                                       unsigned long *length);

DLLOPT unsigned char *asn_build_length(unsigned char *data, int *datalength,
                                       int length);

DLLOPT unsigned char *asn_parse_objid(unsigned char *data, int *datalength,
                                      unsigned char *type,
                                      oid *objid, int *objidlength);

DLLOPT unsigned char *asn_build_objid(unsigned char *data, int *datalength,
                                      unsigned char type,
                                      oid *objid, int objidlength);

DLLOPT void asn_build_subid(unsigned long subid, unsigned char*& bp);

DLLOPT unsigned char *asn_parse_null(unsigned char *data, int *datalength,
                                     unsigned char *type);

DLLOPT unsigned char *asn_build_null(unsigned char *data,int *datalength,
                                     unsigned char type);

DLLOPT unsigned char *asn_parse_bitstring(unsigned char *data, int *datalength,
                                          unsigned char *type,
                                          unsigned char *string,
                                          int *strlength);

DLLOPT unsigned char *asn_build_bitstring(unsigned char *data, int *datalength,
                                          unsigned char type,
                                          unsigned char *string,
                                          int strlength);

DLLOPT unsigned char *asn_parse_unsigned_int64(unsigned char *data,
                                               int *datalength,
                                               unsigned char *type,
                                               struct counter64 *cp);

DLLOPT unsigned char *asn_build_unsigned_int64(unsigned char *data,
                                               int *datalength,
                                               unsigned char type,
                                               struct counter64 *cp);

DLLOPT struct snmp_pdu *snmp_pdu_create(int command);

DLLOPT void snmp_free_pdu(struct snmp_pdu *pdu);

DLLOPT int snmp_build(struct snmp_pdu *pdu,
                      unsigned char *packet,
                      int *out_length,
                      const long version,
                      const unsigned char* community, const int community_len);

DLLOPT void snmp_add_var(struct snmp_pdu *pdu,
                         oid *name, int name_length,
                         SmiVALUE *smival);

DLLOPT int snmp_parse(struct snmp_pdu *pdu,
                      unsigned char *data, int data_length,
                      unsigned char *community_name, int &community_len,
                      snmp_version &version);

DLLOPT unsigned char *build_vb(struct snmp_pdu *pdu,
			       unsigned char *buf, int *buf_len);

DLLOPT unsigned char *build_data_pdu(struct snmp_pdu *pdu,
				     unsigned char *buf, int *buf_len,
				     unsigned char *vb_buf, int vb_buf_len);

DLLOPT unsigned char *snmp_build_var_op(unsigned char *data,
                                        oid * var_name, int *var_name_len,
                                        unsigned char var_val_type,
                                        int var_val_len,
                                        unsigned char *var_val,
                                        int *listlength);

DLLOPT unsigned char *snmp_parse_var_op(unsigned char *data,
                                        oid *var_name, int *var_name_len,
                                        unsigned char  *var_val_type,
                                        int *var_val_len,
                                        unsigned char  **var_val,        
                                        int *listlength);

DLLOPT int snmp_parse_data_pdu(struct snmp_pdu *pdu,
                               unsigned char *&data, int &length);
                               
DLLOPT int snmp_parse_vb(struct snmp_pdu *pdu,
                         unsigned char *&data, int &data_len);

DLLOPT void clear_pdu(struct snmp_pdu *pdu, bool clear_all = false);

/**
 * Encode the given values for the HeaderData into the buffer.
 *                                                                  <pre>
 *  HeaderData ::= SEQUENCE {
 *    msgID      INTEGER (0..2147483647),
 *    msgMaxSize INTEGER (484..2147483647),
 *    msgFlags   OCTET STRING (SIZE(1)),
 *    msgSecurityModel INTEGER (0..2147483647)
 *  }
 *                                                                 </pre>
 * @param outBuf         - The buffer
 * @param maxLength      - IN: length of the buffer
 *                         OUT: free bytes left in the buffer
 * @param msgID          - The message ID
 * @param maxMessageSize - The maximum size of a SNMPv3 message
 * @param msgFlags       - The message Flags
 * @param securityModel  - The security model
 *
 * @return - Pointer to the first free byte in the buffer or
 *           NULL if an error occured
 */
DLLOPT unsigned char *asn1_build_header_data(unsigned char *outBuf,
					     int *maxLength,
					     long msgID,
					     long maxMessageSize,
					     unsigned char msgFlags,
					     long securityModel);

/**
 * Parse the filled HeaderData of a SNMPv3 message and return
 * the encoded values.
 *                                                                  <pre>
 *      HeaderData ::= SEQUENCE {
 *          msgID      INTEGER (0..2147483647),
 *          msgMaxSize INTEGER (484..2147483647),
 *          msgFlags   OCTET STRING (SIZE(1)),
 *          msgSecurityModel INTEGER (0..2147483647)
 *      }
 *                                                                 </pre>
 *
 * @param buf                - The buffer to parse
 * @param buf_len            - IN: The length of the buffer
 *                             OUT: The number of bytes after this object
 *                                  int the buffer
 * @param msg_id             - OUT: The message id
 * @param msg_max_size       - OUT: THe maximum message size of the sender
 * @param msg_flags          - OUT: The message flags
 * @param msg_security_model - OUT: The security model used to build this
 *                                message
 *
 * @return -  Returns a pointer to the first byte past the end of
 *            the object HeaderData (i.e. the start of the next object).
 *            Returns NULL on any error.
 */
DLLOPT unsigned char *asn1_parse_header_data(unsigned char *buf, int *buf_len,
					     long *msg_id, long *msg_max_size,
					     unsigned char *msg_flags,
					     long *msg_security_model);

/**
 * Parse the ScopedPDU and return the encoded values.
 *                                                                  <pre>
 *      ScopedPDU ::= SEQUENCE {
 *          contextEngineID  OCTET STRING,
 *          contextName      OCTET STRING,
 *          data             ANY -- e.g., PDUs as defined in RFC 1905
 *      }
 *                                                                 </pre>
 *
 * @param scoped_pdu            - The buffer to parse
 * @param scoped_pdu_len        - IN: The length of the buffer
 *                                OUT: The number of bytes after this object
 *                                     int the buffer
 * @param context_engine_id     - OUT: The parsed contextEngineID
 * @param context_engine_id_len - OUT: The length of the contextEngineID
 * @param context_name          - OUT: The parsed contextName
 * @param context_name_len      - OUT: The length of the contextName
 *
 * @return - Pointer to the data object of the scopedPDU or
 *           NULL on any error.
 */
DLLOPT unsigned char *asn1_parse_scoped_pdu(
         unsigned char *scoped_pdu, int *scoped_pdu_len,
         unsigned char *context_engine_id, int *context_engine_id_len,
         unsigned char *context_name, int *context_name_len );

/**
 * Encode the given values for the scopedPDU into the buffer.
 *                                                                  <pre>
 *    ScopedPDU ::= SEQUENCE {
 *           contextEngineID OCTET STRING
 *           contextName     OCTET STRING
 *           data            ANY  -- PDU
 *       }
 *                                                                 </pre>
 * param outBuf            - The buffer
 * param max_len           - IN: length of the buffer
 *                           OUT: free bytes left in the buffer
 * param contextEngineID   - The contextEngineID
 * param contextEngineIDLength - The length of the contextEngineID
 * param contextName       - The contextName
 * param contextNameLength - The length of the contextName
 * param data              - The already encoded data
 * param dataLength        - The length of the data
 *
 * @return - Pointer to the first free byte in the buffer or
 *           NULL if an error occured
 */
DLLOPT unsigned char *asn1_build_scoped_pdu(
               unsigned char *outBuf, int *max_len,
               unsigned char *contextEngineID, long contextEngineIDLength,
               unsigned char *contextName, long contextNameLength,
               unsigned char *data, long dataLength);


#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif  // _ASN1
