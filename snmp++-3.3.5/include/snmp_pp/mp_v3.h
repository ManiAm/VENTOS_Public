/*_############################################################################
  _## 
  _##  mp_v3.h  
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

// $Id: mp_v3.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _MP_V3
#define _MP_V3

#include "snmp_pp/config_snmp_pp.h"

#ifdef _SNMPv3

#include "snmp_pp/reentrant.h"
#include "snmp_pp/target.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

class Pdu;
class OctetStr;

#define MAX_HOST_NAME_LENGTH     128

#define oidMPDGroup                  "1.3.6.1.6.3.11.2.1"
#define oidSnmpUnknownSecurityModels "1.3.6.1.6.3.11.2.1.1.0"
#define oidSnmpInvalidMsgs           "1.3.6.1.6.3.11.2.1.2.0"
#define oidSnmpUnknownPDUHandlers    "1.3.6.1.6.3.11.2.1.3.0"

/** @name Error codes of the v3MP */
//@{
#define SNMPv3_MP_ERROR                       -1400
#define SNMPv3_MP_OK                          -1401
#define SNMPv3_MP_UNSUPPORTED_SECURITY_MODEL  -1402
#define SNMPv3_MP_NOT_IN_TIME_WINDOW          -1403
#define SNMPv3_MP_DOUBLED_MESSAGE             -1404
#define SNMPv3_MP_INVALID_MESSAGE             -1405
#define SNMPv3_MP_INVALID_ENGINEID            -1406
#define SNMPv3_MP_NOT_INITIALIZED             -1407
#define SNMPv3_MP_PARSE_ERROR                 -1408
#define SNMPv3_MP_UNKNOWN_MSGID               -1409
#define SNMPv3_MP_MATCH_ERROR                 -1410
#define SNMPv3_MP_COMMUNITY_ERROR             -1411
#define SNMPv3_MP_WRONG_USER_NAME             -1412
#define SNMPv3_MP_BUILD_ERROR                 -1413
#define SNMPv3_MP_USM_ERROR                   -1414
#define SNMPv3_MP_UNKNOWN_PDU_HANDLERS        -1415
#define SNMPv3_MP_UNAVAILABLE_CONTEXT         -1416
#define SNMPv3_MP_UNKNOWN_CONTEXT             -1417
#define SNMPv3_MP_REPORT_SENT                 -1418
//@}

/** @name Statistics on error codes. */
//@{
#define SNMPv3_MP_MAX_ERROR           SNMPv3_MP_ERROR
#define SNMPv3_MP_MIN_ERROR           SNMPv3_MP_REPORT_SENT
#define SNMPv3_MP_ERRORCOUNT          SNMPv3_MP_MAX_ERROR - SNMPv3_MP_MIN_ERROR
//@}

class Snmp;
class USM;

/**
 * The SNMPv3 Message Processing Model (v3MP).
 *
 * If SNMPv3 is used, the application needs to create _one_ object of
 * this class. This object will automatically create an object of the
 * USM class. A pointer to this object is returned from the get_usm()
 * method. See the USM documentation for a description on how to create
 * and delete users.
 *
 * The only thing that may be configured after creation of the v3MP is
 * the engine id table of the v3MP. Entries for other SNMP entities
 * can be added through add_to_engine_id_table(). It is only required
 * to add entries to this table if you want to disable engine id
 * discovery and/or you don't want the delay caused by the automatic
 * engine id discovery of SNMPv3.
 */
class DLLOPT v3MP
{
  friend class SnmpMessage;
  friend class CSNMPMessageQueue;
 public:
  /**
   * Initialize the v3MP.
   *
   * Set the engineID of this SNMP entity and the Snmp object used to
   * send reports. This function creates a new USM object that can
   * be configured after getting a pointer to it through get_usm().
   *
   * The user is responsible to save and restore and increment the
   * snmpEngineBoots counter (The functions getBootCounter() and
   * saveBootCounter() can be used to do this.).
   *
   * @param engine_id    - The locale engine id
   * @param engine_boots - The new value for the snmpEngineBoots counter
   * @param construct_status - OUT: SNMPv3_MP_OK or SNMPv3_MP_ERROR
   *
   */
  v3MP(const OctetStr& engine_id,
       unsigned int engine_boots, int &construct_status);

  /**
   * Get the engine id of this SNMP entity.
   *
   * @param id - OUT: The engineID of this SNMP entity
   *
   * @return - SNMPv3_MP_OK or SNMPv3_MP_ERROR
   */
  void get_local_engine_id(OctetStr &id) { id = own_engine_id_oct; };

  /**
   * Get the engine id of this SNMP entity as a OctetStr reference.
   *
   * @return Local engine id.
   */
  const OctetStr& get_local_engine_id() const
    { return own_engine_id_oct; };

  /**
   * Get a pointer to the USM object that is used by the v3MP.
   */
  USM *get_usm() { return usm; };

  /**
   * Free all allocated ressources of the v3MP and leave it in an
   * uninitialized state. After a call to this function, you can use
   * mpInit() to reinitialize the v3MP.
   *
   */
  ~v3MP();

  /**
   * Add an entry to the engine id table.
   *
   * In this table all known engine ids are stored. If the discovery
   * mode of the USM is enabled, snmp++ will add entries to this table
   * whenever a new engine id is dicovered.
   *
   * @param engine_id - The engine id
   * @param host      - The numerical IP address
   * @param port      - The port
   *
   * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
   */
  int add_to_engine_id_table(const OctetStr &engine_id,
			     const OctetStr &host, int port)
    { return engine_id_table.add_entry(engine_id, host, port); };

  /**
   * Remove an entry from the engine id table.
   *
   * @param host      - The numerical IP address
   * @param port      - The port
   *
   * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
   */
  int remove_from_engine_id_table(const OctetStr &host, int port)
    { return engine_id_table.delete_entry(host, port); };

  /**
   * Remove an entry from the engine id table.
   *
   * @param engine_id - The engine id
   *
   * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
   */
  int remove_from_engine_id_table(const OctetStr &engine_id)
    { return engine_id_table.delete_entry(engine_id); };

  /**
   * Get the engine id of the SNMP entity at the given host/port.
   *
   * @param engine_id - OUT: The engine id
   * @param hostport  - The numerical IP address and port
   *                    (syntax: a.b.c.d/port)
   *
   * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
   *           SNMPv3_MP_OK
   */
  int get_from_engine_id_table(OctetStr &engine_id,
			       const OctetStr &hostport) const
    {  return engine_id_table.get_entry(engine_id, hostport); };

  /**
   * Get the engineID of the SNMP entity at the given host/port.
   *
   * @param engineID - OUT: The engineID
   * @param host     - The numerical IP address
   * @param port     - The port
   *
   * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
   *           SNMPv3_MP_OK
   */
  int get_from_engine_id_table(OctetStr &engineID,
			       const OctetStr &host, int port) const
    {  return engine_id_table.get_entry(engineID, host, port); };

  /**
   * Remove all entries from the engine id table.
   *
   * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
   *           SNMPv3_MP_OK
   */
  int reset_engine_id_table()
    {  return engine_id_table.reset(); };

  /**
   * Remove all occurences of this engine id from v3MP and USM.
   *
   * @param engine_id - The engine id to remove
   *
   * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
   *           SNMPv3_MP_OK
   */
  int remove_engine_id(const OctetStr &engine_id);

  // ----------[ Access to status counters for agent++ ]--------------

  /**
   * Get the value of the status counter snmpUnknownSecurityModels.
   *
   * @return - The status counter
   */
  unsigned long get_stats_unknown_security_models() const
    { return snmpUnknownSecurityModels; };

  /**
   * Get the value of the status counter snmpInvalidMsgs.
   *
   * @return - The status counter
   */
  unsigned long get_stats_invalid_msgs() const
    { return snmpInvalidMsgs; };

  /**
   * Get the value of the status counter snmpUnknownPDUHandlers.
   *
   * @return - The status counter
   */
  unsigned long get_stats_unknown_pdu_handlers() const
    { return snmpUnknownPDUHandlers; };

  /**
   * Increment the value of the status counter snmpUnknownSecurityModels.
   */
  void inc_stats_unknown_security_models()
    { snmpUnknownSecurityModels++; };

  /**
   * Increment the value of the status counter snmpInvalidMsgs.
   */
  void inc_stats_invalid_msgs() { snmpInvalidMsgs++; };

  /**
   * Increment the value of the status counter snmpUnknownPDUHandlers.
   */
  void inc_stats_unknown_pdu_handlers() { snmpUnknownPDUHandlers++; };

  // temporary pointer will be removed...
  static v3MP *I;

 protected:

  /**
   * Parse the given buffer as a SNMPv3-Message.
   *
   * @param snmp_session     - IN: The session used to receive the msg
   * @param pdu              - OUT: Parsed values are put into this struct
   * @param inBuf            - The buffer to parse
   * @param inBufLength      - The length of the buffer
   * @param securityEngineID - OUT: The parsed securityEngineID
   * @param securityName     - OUT: The parsed securityName
   * @param contextEngineID  - OUT: The parsed contextEngineID
   * @param contextName      - OUT: The parsed contextName
   * @param securityLevel    - OUT: The parsed security level
   * @param msgSecurityModel - OUT: The security model used
   * @param spp_version      - OUT: SNMP version (SNMPv3)
   * @param from_address     - Where the message came from (used to send
   *                           a report if neccessary)
   *
   * @return - SNMPv3_MP_OK or any error listed in snmperr.h
   */
  int snmp_parse(Snmp *snmp_session,
                 struct snmp_pdu *pdu,
		 unsigned char *inBuf,
		 int inBufLength,
		 OctetStr &securityEngineID,
		 OctetStr &securityName,
		 OctetStr &contextEngineID,
		 OctetStr &contextName,
		 long     &securityLevel,
		 long     &msgSecurityModel,
		 snmp_version &spp_version,
		 UdpAddress from_address);

  /**
   * Tests if the given buffer contains a SNMPv3-Message. The buffer is
   * only parsed to extract the version of the message, no other checks
   * are made.
   *
   * @param buffer - The message
   * @param length - The length of the message
   *
   * @return - TRUE if the version could be extracted and it
   *           is a SNMPv3 message. On any error: FALSE.
   *
   */
  static bool is_v3_msg( unsigned char *buffer, int length);

  /**
   * Do the complete process of encoding the given values into the buffer
   * ready to send to the target.
   *
   * @param pdu              - The pdu structure
   * @param packet           - The buffer to store the serialized message
   * @param out_length       - IN: Length of the buffer,
   *                           OUT: Length of the message
   * @param securityEngineID - The securityEngineID
   * @param securityNameIn   - The securityName
   * @param securityModel    - Use this security model
   * @param securityLevel    - Use this security level
   * @param contextEngineID  - The contextEngineID
   * @param contextName      - The contextName
   *
   * @return - SNMPv3_MP_OK or any error listed in snmperr.h
   */
  int snmp_build(struct snmp_pdu *pdu,
		 unsigned char *packet,
		 int *out_length,           // maximum Bytes in packet
		 const OctetStr &securityEngineID,
		 const OctetStr &securityNameIn,
		 int securityModel, int securityLevel,
		 const OctetStr &contextEngineID,
		 const OctetStr &contextName);

  /**
   * Delete the entry with the given request id from the cache.
   * This function is used in eventlist.cpp when a request
   * has timed out.
   *
   * @param requestID - The request id.
   * @param local_request - Does the request id belong to a local or to
   *                        a remote request?
   */
  void delete_from_cache(unsigned long requestID,
			 const bool local_request = true)
    { cache.delete_entry(requestID, local_request); };

 public:

  /**
   * Delete the entry with the given request id from the cache.
   * This function is used in agent++ RequestList.
   *
   * @param requestID - The request id.
   * @param messageID - The message id.
   * @param local_request - Does the request id belong to a local or to
   *                        a remote request?
   */
  void delete_from_cache(unsigned long requestID,
			 unsigned long messageID,
			 const bool local_request)
    { cache.delete_entry(requestID, messageID, local_request); };

 private:

  /**
   * Send a report message.
   *
   * @param scopedPDU   - The scopedPDU as received. If the pdu is not
   *                      encrypted, the request id is extracted
   * @param scopedPDULength - The lkength of the scopedPDU
   * @param pdu         - The pdu structure.
   * @param errorCode   - The code of the error that occured.
   * @param sLevel      - Send the report with this security level.
   * @param sModel      - Use this security model.
   * @param sName       - Use this security name
   * @param destination - Send the report to this address.
   * @param snmp_session - Snmp session to use for sending a report
   *
   * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
   */
  int send_report(unsigned char* scopedPDU, int scopedPDULength,
		  struct snmp_pdu *pdu, int errorCode, int sLevel,
		  int sModel, OctetStr &sName,
		  UdpAddress &destination, Snmp *snmp_session);



  // =====================[ member classes ]==============================

  /**
   * The engine id table is used to store known engine ids with
   * corresponding hostadress and port.
   */
  class DLLOPT EngineIdTable
  {
   public:

    EngineIdTable(int initial_size = 10);
    ~EngineIdTable();

    /**
     * Add an entry to the table.
     *
     * @param engine_id - The engineID
     * @param host      - The numerical IP address
     * @param port      - The port
     *
     * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
     */
    int add_entry(const OctetStr &engine_id,
		  const OctetStr &host, int port);

    /**
     * Get the engine_id of the SNMP entity at the given host/port.
     *
     * @param engine_id - OUT: The engineID
     * @param hostport  - The numerical IP address and port
     *                    (syntax: a.b.c.d/port)
     *
     * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
     *           SNMPv3_MP_OK
     */
    int get_entry(OctetStr &engine_id, const OctetStr &hostport) const;

    /**
     * Get the engineID of the SNMP entity at the given host/port.
     *
     * @param engine_id - OUT: The engineID
     * @param host      - The numerical IP address
     * @param port      - The port
     *
     * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
     *           SNMPv3_MP_OK
     */
    int get_entry(OctetStr &engine_id, const OctetStr &host, int port) const;

    /**
     * Remove all entries from the engine id table.
     *
     * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
     *           SNMPv3_MP_OK
     */
    int reset();

    /**
     * Remove the given engine id from the table.
     *
     * @param engine_id - The engine id to remove
     *
     * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
     *           SNMPv3_MP_OK
     */
    int delete_entry(const OctetStr &engine_id);

    /**
     * Remove the entry for the given address/port from the table.
     *
     * @param host - Numeric IP address
     * @param port - listen port of the snmp entity
     *
     * @return - SNMPv3_MP_NOT_INITIALIZED, SNMPv3_MP_ERROR,
     *           SNMPv3_MP_OK
     */
    int delete_entry(const OctetStr &host, int port);

  private:
    bool initialize_table(const int size);

    struct Entry_T
    {
      OctetStr engine_id;
      OctetStr host;
      int port;
    };

    struct Entry_T *table;
    int max_entries;      ///< the maximum number of entries
    int entries;          ///< the current amount of entries
    SNMP_PP_MUTABLE SnmpSynchronized lock;
  };


  /**
   * Holds cache entries for currently processed requests.
   */
  class DLLOPT Cache
  {
   public:
    Cache();
    ~Cache();

    struct Entry_T
    {
      int msg_id;
      unsigned long req_id;
      OctetStr sec_engine_id;
      int sec_model;
      OctetStr sec_name;
      int sec_level;
      OctetStr context_engine_id;
      OctetStr context_name;
      struct SecurityStateReference *sec_state_ref;
      int error_code;
      bool local_request;
    };

    /**
     * Add an entry to the cache.
     *
     * @param msg_id            - The message id of the message
     * @param req_id        - The request id of the message
     * @param sec_engine_id - The authoritative engineID
     * @param sec_model    - The security model used for this message
     * @param sec_name     - The name of the user
     * @param sec_level    - The security level used for this message
     * @param context_engine_id  - The context_engine_id
     * @param context_name      - The context_name
     * @param sec_state_ref - The reference of the USM
     * @param error_code        - The code of the error that occured while
     *                           parsing the received message
     *
     * @return - SNMPv3_MP_OK, SNMPv3_MP_ERROR or SNMPv3_DOUBLED_MESSAGE
     *           (an entry with the given values is already in the cache)
     */
    int add_entry(int msg_id, unsigned long req_id,
		  const OctetStr &sec_engine_id,
		  int sec_model,
		  const OctetStr &sec_name,
		  int sec_level,
		  const OctetStr &context_engine_id,
		  const OctetStr &context_name,
		  struct SecurityStateReference *sec_state_ref,
		  int error_code, bool local_request);
    /**
     * Search the cache for a message id, return the error code and
     * the sec_state_ref and delete the entry from the cache.
     *
     * @param msg_id     - Search for this message id
     * @param error_code - OUT: The error code of the received message
     * @param sec_state_ref - IN:  Pointer to a pointer of the structure
     *                        OUT: The structure as received by the USM when
     *                             the message was parsed.
     *
     * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
     */
    int get_entry(int msg_id, bool local_request, int *error_code,
		  struct SecurityStateReference **sec_state_ref);

    /**
     * Delete the entry with the given request id from the cache.
     * This function is used in eventlist.cpp when a request
     * has timed out.
     *
     * @param req_id - The request id.
     */
    void delete_entry(unsigned long req_id, bool local_request);

    /**
     * Delete the entry with the given request and message id from the cache.
     *
     * @param req_id - The request id.
     * @param msg_id - The message id.
     */
    void delete_entry(unsigned long req_id, int msg_id,
		      bool local_request);

    /**
     * Search the cache for a message id, return the whole entry and
     * delete the entry from the cache.
     *
     * @param searchedID - Search for this message id
     * @param res        - IN:  Pointer to an empy structure
     *                     OUT: The filled structure
     *
     * @return - SNMPv3_MP_ERROR, SNMPv3_MP_OK
     */
    int get_entry(int searchedID, bool local_request,
                  struct Cache::Entry_T *res);

    void delete_content(struct Cache::Entry_T &ce);

    void set_usm(USM *usm_to_use) { usm = usm_to_use; };

   private:
#ifdef _THREADS
    SNMP_PP_MUTABLE SnmpSynchronized lock;
#endif
    struct Entry_T *table; ///< whole table
    int max_entries;       ///< the maximum number of entries
    int entries;           ///< the current amount of entries
    USM *usm;
  };


  // =====================[ member variables ]==============================
  EngineIdTable engine_id_table;
  Cache cache;

  // the engineID of this SNMP entity
  unsigned char *own_engine_id;
  int own_engine_id_len;
  OctetStr own_engine_id_oct;

  unsigned int cur_msg_id;   ///< msgID to use for next message
  SNMP_PP_MUTABLE SnmpSynchronized cur_msg_id_lock;

  USM *usm;  ///< the USM object used

  // MIB Counters
  unsigned int snmpUnknownSecurityModels;
  unsigned int snmpInvalidMsgs;
  unsigned int snmpUnknownPDUHandlers;
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _SNMPv3

#endif
