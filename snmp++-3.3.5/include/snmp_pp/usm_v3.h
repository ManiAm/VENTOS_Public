/*_############################################################################
  _## 
  _##  usm_v3.h  
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
// $Id: usm_v3.h 2833 2015-03-15 10:33:08Z katz $

#ifndef _USM_V3
#define _USM_V3

#include "snmp_pp/config_snmp_pp.h"

#ifdef _SNMPv3

#include "snmp_pp/smi.h"
#include "snmp_pp/octet.h"
#include "snmp_pp/address.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#ifndef MAXUINT32
#define MAXUINT32 4294967295u
#endif

// the maximum allowed length of the username
#define MAXLEN_USMUSERNAME 32
#define MAXLEN_USMSECURITYNAME MAXLEN_USMUSERNAME

#define SNMPv3_AUTHFLAG       0x01
#define SNMPv3_PRIVFLAG       0x02
#define SNMPv3_REPORTABLEFLAG 0x04

#define NOKEY      0
#define AUTHKEY    1
#define PRIVKEY    2
#define OWNAUTHKEY 3
#define OWNPRIVKEY 4

/** @name SecurityLevels
 *
 * When sending a SNMPv3 message, one of these security levels can be
 * set on the Pdu object.
 */
//@{
#define SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV  1 ///< noAuthNoPriv
#define SNMP_SECURITY_LEVEL_AUTH_NOPRIV    2 ///< authNoPriv
#define SNMP_SECURITY_LEVEL_AUTH_PRIV      3 ///< authPriv
//@}

/** @name AuthProtocols
 *
 * Each user of the USM must use one authentication protocol (which
 * may be none.
 */
//@{
#define SNMP_AUTHPROTOCOL_NONE    1 ///< None
#define SNMP_AUTHPROTOCOL_HMACMD5 2 ///< HMAC-MD5
#define SNMP_AUTHPROTOCOL_HMACSHA 3 ///< HMAC-SHA
//@}

/** @name PrivProtocols
 *
 * Each user of the USM must use one privacy protocol (which may be
 * none.
 */
//@{
#define SNMP_PRIVPROTOCOL_NONE     1 ///< None
#define SNMP_PRIVPROTOCOL_DES      2 ///< DES
#define SNMP_PRIVPROTOCOL_AES128   4 ///< AES128 (RFC 3826)

#define SNMP_PRIVPROTOCOL_IDEA     9 ///< IDEA (non standard)
#define SNMP_PRIVPROTOCOL_AES192  20 ///< AES192 (non standard)
#define SNMP_PRIVPROTOCOL_AES256  21 ///< AES256 (non standard)
#define SNMP_PRIVPROTOCOL_3DESEDE  3 ///< 3DES (expired draft standard)
#define SNMP_PRIVPROTOCOL_AES128W3DESKEYEXT  22 ///< AES128 with Key extension algotrithm from 3DESEDE (non standard)
#define SNMP_PRIVPROTOCOL_AES192W3DESKEYEXT  23 ///< AES192 with Key extension algotrithm from 3DESEDE (non standard)
#define SNMP_PRIVPROTOCOL_AES256W3DESKEYEXT  24 ///< AES256 with Key extension algotrithm from 3DESEDE (non standard)
//@}

/** @name USM-ErrorCodes
 *
 * Each method of the class USM may return one of the following
 * error codes.
 */
//@{
#define SNMPv3_USM_OK                          1400
#define SNMPv3_USM_ERROR                       1401
#define SNMPv3_USM_ERROR_CONFIGFILE            1402
#define SNMPv3_USM_UNSUPPORTED_SECURITY_LEVEL  1403
#define SNMPv3_USM_UNKNOWN_SECURITY_NAME       1404
#define SNMPv3_USM_ENCRYPTION_ERROR            1405
#define SNMPv3_USM_DECRYPTION_ERROR            1406
#define SNMPv3_USM_AUTHENTICATION_ERROR        1407
#define SNMPv3_USM_AUTHENTICATION_FAILURE      1408
#define SNMPv3_USM_PARSE_ERROR                 1409
#define SNMPv3_USM_UNKNOWN_ENGINEID            1410
#define SNMPv3_USM_NOT_IN_TIME_WINDOW          1411
#define SNMPv3_USM_UNSUPPORTED_AUTHPROTOCOL    1412
#define SNMPv3_USM_UNSUPPORTED_PRIVPROTOCOL    1413
#define SNMPv3_USM_ADDRESS_ERROR               1414
#define SNMPv3_USM_FILECREATE_ERROR            1415
#define SNMPv3_USM_FILEOPEN_ERROR              1416
#define SNMPv3_USM_FILERENAME_ERROR            1417
#define SNMPv3_USM_FILEDELETE_ERROR            1418
#define SNMPv3_USM_FILEWRITE_ERROR             1419
#define SNMPv3_USM_FILEREAD_ERROR              1420
//@}

/** @name Statistics on error codes. */
//@{
#define SNMPv3_USM_MAX_ERROR                   SNMPv3_USM_FILEREAD_ERROR
#define SNMPv3_USM_MIN_ERROR                   SNMPv3_USM_OK
#define SNMPv3_USM_ERRORCOUNT                  SNMPv3_USM_MAX_ERROR - SNMPv3_USM_MIN_ERROR
//@}

#define oidUsmStats                        "1.3.6.1.6.3.15.1.1"
#define oidUsmStatsUnsupportedSecLevels    "1.3.6.1.6.3.15.1.1.1.0"
#define oidUsmStatsNotInTimeWindows        "1.3.6.1.6.3.15.1.1.2.0"
#define oidUsmStatsUnknownUserNames        "1.3.6.1.6.3.15.1.1.3.0"
#define oidUsmStatsUnknownEngineIDs        "1.3.6.1.6.3.15.1.1.4.0"
#define oidUsmStatsWrongDigests            "1.3.6.1.6.3.15.1.1.5.0"
#define oidUsmStatsDecryptionErrors        "1.3.6.1.6.3.15.1.1.6.0"

#define oidUsmUserTable                    "1.3.6.1.6.3.15.1.2.2"
#define oidUsmUserEntry                    "1.3.6.1.6.3.15.1.2.2.1"

#define oidUsmAuthProtocolBase             "1.3.6.1.6.3.10.1.1"
#define oidUsmNoAuthProtocol               "1.3.6.1.6.3.10.1.1.1"
#define oidUsmHMACMD5AuthProtocol          "1.3.6.1.6.3.10.1.1.2"
#define oidUsmHMACSHAAuthProtocol          "1.3.6.1.6.3.10.1.1.3"

#define oidUsmPrivProtocolBase             "1.3.6.1.6.3.10.1.2"
#define oidUsmNoPrivProtocol               "1.3.6.1.6.3.10.1.2.1"
#define oidUsmDESPrivProtocol              "1.3.6.1.6.3.10.1.2.2"
#define oidUsmIDEAPrivProtocol             "1.3.6.1.6.3.10.1.2.9"
#define oidUsmAES128PrivProtocol           "1.3.6.1.6.3.10.1.2.4"
#define oidUsmAES192PrivProtocol           "1.3.6.1.6.3.10.1.2.20"
#define oidUsmAES256PrivProtocol           "1.3.6.1.6.3.10.1.2.21"
#define oidUsm3DESEDEPrivProtocol          "1.3.6.1.6.3.10.1.2.3"


#define USM_KeyUpdate            1
#define USM_PasswordUpdate       2
#define USM_PasswordKeyUpdate    3
#define USM_PasswordAllKeyUpdate 4

class SnmpTarget;
class Pdu;

struct UsmKeyUpdate;

struct UsmUserTableEntry {
  unsigned char *usmUserEngineID;     long int usmUserEngineIDLength;
  unsigned char *usmUserName;         long int usmUserNameLength;
  unsigned char *usmUserSecurityName; long int usmUserSecurityNameLength;
  long int  usmUserAuthProtocol;
  unsigned char *usmUserAuthKey;      long int usmUserAuthKeyLength;
  long int  usmUserPrivProtocol;
  unsigned char *usmUserPrivKey;      long int usmUserPrivKeyLength;
};

struct UsmUser {
  unsigned char *engineID;     long int engineIDLength;
  unsigned char *usmUserName;  long int usmUserNameLength;
  unsigned char *securityName; long int securityNameLength;
  long int  authProtocol;
  unsigned char *authKey;      long int authKeyLength;
  long int  privProtocol;
  unsigned char *privKey;      long int privKeyLength;
};

struct UsmUserNameTableEntry {
  OctetStr usmUserName;
  OctetStr usmUserSecurityName;
  long int  usmUserAuthProtocol;
  long int  usmUserPrivProtocol;
  unsigned char *authPassword;        long int authPasswordLength;
  unsigned char *privPassword;        long int privPasswordLength;
};

//-----------[ async methods callback ]-----------------------------------
typedef void (*usm_add_user_callback)(const OctetStr &engine_id,
                                      const OctetStr &usm_user_name,
                                      const OctetStr &usm_user_security_name,
                                      const int auth_protocol,
                                      const OctetStr &auth_key,
                                      const int priv_protocol,
                                      const OctetStr &priv_key);

struct SecurityStateReference;

class AuthPriv;
class USMTimeTable;
class USMUserNameTable;
class USMUserTable;
class v3MP;

/**
 * This is the class for the User Based Security Model.
 *
 * To add or delete users, the methods add_usm_user() and delete_usm_user()
 * should be used.
 *
 * USM distinguishes between userName and securityName. The following is
 * from section 2.1 of RFC3414:
 *
 * "userName: A string representing the name of the user.
 *
 *  securityName: A human-readable string representing the user in a format
 *  that is Security Model independent. There is a one-to-one relationship *
 *  between userName and securityName."
 */
class DLLOPT USM
{
  friend class v3MP;

public:

  /**
   * Create an instance of the USM.
   *
   * @param engine_boots - The new value for the snmpEngineBoots counter
   * @param engine_id    - The local snmp engine id
   * @param v3_mp        - Pointer to the parent v3MP object.
   * @param msg_id       - OUT: The initial value for the msgID
   * @param result       - OUT: construct status, should be SNMPv3_USM_OK
   */
  USM(unsigned int engine_boots, const OctetStr &engine_id, const v3MP *v3_mp,
      unsigned int *msg_id, int &result);

  /**
   * Destructor.
   */
  ~USM();

  /**
   * Enables the discovery mode of the USM, i.e. the USM accepts all messages
   * with unknown engine ids and adds these engine ids to its tables.
   */
  void set_discovery_mode() { discovery_mode = true; };

  /**
   * Disables the discovery mode of the USM, i.e. the USM will not accept any
   * message with an unknown engine id.
   */
  void unset_discovery_mode() { discovery_mode = false; };

  /**
   * Return TRUE if the USM discovery mode is enabled, FALSE else.
   */
  bool is_discovery_enabled() const { return discovery_mode; };

  /**
   * Add a new user to the usmUserNameTable. If the User is already known
   * to the USM, the old entry is replaced.
   * The USM will compute a userName for the given securityName, which
   * will be the same as securityName (recommended).
   *
   * If discovery mode is enabled, localized user entries are
   * automatically created for new engine ids.
   *
   * @param security_name - Unique securityName
   * @param auth_protocol - Possible values are:
   *                              SNMP_AUTHPROTOCOL_NONE,
   *                              SNMP_AUTHPROTOCOL_HMACMD5,
   *                              SNMP_AUTHPROTOCOL_HMACSHA
   * @param priv_protocol - Possible values are:
   *                              SNMP_PRIVPROTOCOL_NONE,
   *                              SNMP_PRIVPROTOCOL_DES,
   *                              SNMP_PRIVPROTOCOL_IDEA
   * @param auth_password - Secret password for authentication
   * @param priv_password - Secret password for privacy
   *
   * @return - SNMPv3_USM_OK or
   *           SNMP_v3_USM_ERROR (memory error, not initialized)
   */
  int add_usm_user(const OctetStr& security_name,
		   const long int  auth_protocol,
		   const long int  priv_protocol,
		   const OctetStr& auth_password,
		   const OctetStr& priv_password);

  /**
   * Add a new user to the usmUserNameTable. If the userName is already known
   * to the USM, the old entry is replaced.
   *
   * It is not recommended to add users with userName != securityName.
   *
   * @param  user_name     - Unique userName
   * @param  security_name - Unique securityName
   * @param  auth_protocol - Possible values are:
   *                              SNMP_AUTHPROTOCOL_NONE,
   *                              SNMP_AUTHPROTOCOL_HMACMD5,
   *                              SNMP_AUTHPROTOCOL_HMACSHA
   * @param  priv_protocol - Possible values are:
   *                              SNMP_PRIVPROTOCOL_NONE,
   *                              SNMP_PRIVPROTOCOL_DES,
   *                              SNMP_PRIVPROTOCOL_IDEA
   * @param  auth_password - Secret password for authentication
   * @param  priv_password - Secret password for privacy
   *
   * @return - SNMPv3_USM_OK or
   *           SNMP_v3_USM_ERROR (memory error, not initialized)
   */
  int add_usm_user(const OctetStr& user_name,
		   const OctetStr& security_name,
		   const long int  auth_protocol,
		   const long int  priv_protocol,
		   const OctetStr& auth_password,
		   const OctetStr& priv_password);


  /**
   * Add or replace a localized user in the USM table.
   *
   * This function uses build_localized_keys() to generate localized
   * keys for the given passwords. Then it calls add_localized_user()
   * to add/replace the localized entry for the user.
   *
   * The passwords are not stored, so no additonal engine id discovery
   * is possible.
   *
   * @param user_name         - The name of the user (in the USM)
   * @param security_name     - The securityName of the user, this name
   *                                is the same for all securityModels
   * @param auth_protocol     - Possible values are:
   *                                SNMP_AUTHPROTOCOL_NONE,
   *                                SNMP_AUTHPROTOCOL_HMACMD5,
   *                                SNMP_AUTHPROTOCOL_HMACSHA,...
   * @param priv_protocol     - Possible values are:
   *                                SNMP_PRIVPROTOCOL_NONE,
   *                                SNMP_PRIVPROTOCOL_DES,
   *                                SNMP_PRIVPROTOCOL_IDEA,...
   * @param  auth_password - Secret password for authentication
   * @param  priv_password - Secret password for privacy
   * @param engine_id         - The engineID, the key was localized with
   *
   * @return - SNMPv3_USM_OK
   *           SNMP_v3_USM_ERROR (not initialized, no memory)
   */
  int add_usm_user(const OctetStr& user_name,
		   const OctetStr& security_name,
		   const long int  auth_protocol,
		   const long int  priv_protocol,
		   const OctetStr& auth_password,
		   const OctetStr& priv_password,
		   const OctetStr& engine_id);

  int add_usm_user(const OctetStr& security_name,
		   const long int  auth_protocol,
		   const long int  priv_protocol,
		   const OctetStr& auth_password,
		   const OctetStr& priv_password,
		   const OctetStr& engine_id)
    { return add_usm_user(security_name, security_name, auth_protocol,
			  priv_protocol, auth_password, priv_password,
			  engine_id); };


  /**
   * Delete all occurences of the user with the given security name
   * from the USM.
   *
   * @param security_name - the securityName of the user
   */
  void delete_usm_user(const OctetStr& security_name);


  /**
   * Save all localized users into a file.
   *
   * @param file - filename including path
   *
   * @return SNMPv3_USM_ERROR, SNMPv3_USM_FILECREATE_ERROR,
   *         SNMPv3_USM_FILERENAME_ERROR or SNMPv3_USM_OK
   */
  int save_localized_users(const char *file);

  /**
   * Load localized users from a file.
   *
   * @param file - filename including path
   *
   * @return SNMPv3_USM_ERROR, SNMPv3_USM_FILEOPEN_ERROR,
   *         SNMPv3_USM_FILEREAD_ERROR or SNMPv3_USM_OK
   */
  int load_localized_users(const char *file);

  /**
   * Save all users with their passwords into a file.
   *
   * @param file - filename including path
   *
   * @return SNMPv3_USM_ERROR, SNMPv3_USM_FILECREATE_ERROR,
   *         SNMPv3_USM_FILERENAME_ERROR or SNMPv3_USM_OK
   */
  int save_users(const char *file);

  /**
   * Load users with their passwords from a file.
   *
   * @param file - filename including path
   *
   * @return SNMPv3_USM_ERROR, SNMPv3_USM_FILEOPEN_ERROR,
   *         SNMPv3_USM_FILEREAD_ERROR or SNMPv3_USM_OK
   */
  int load_users(const char *file);

  /**
   * Add or replace a localized user in the USM table. Use this method
   * only, if you know what you are doing.
   *
   * @param engine_id         - The engineID, the key was localized with
   * @param user_name         - The name of the user (in the USM)
   * @param security_name     - The securityName of the user, this name
   *                                is the same for all securityModels
   * @param auth_protocol     - Possible values are:
   *                                SNMP_AUTHPROTOCOL_NONE,
   *                                SNMP_AUTHPROTOCOL_HMACMD5,
   *                                SNMP_AUTHPROTOCOL_HMACSHA,...
   * @param auth_key          - The key used for authentications
   * @param priv_protocol     - Possible values are:
   *                                SNMP_PRIVPROTOCOL_NONE,
   *                                SNMP_PRIVPROTOCOL_DES,
   *                                SNMP_PRIVPROTOCOL_IDEA,...
   * @param priv_key          - The key used for privacy
   *
   * @return - SNMPv3_USM_OK
   *           SNMP_v3_USM_ERROR (not initialized, no memory)
   */
  int add_localized_user(const OctetStr &engine_id,
			 const OctetStr &user_name,
			 const OctetStr &security_name,
			 const long auth_protocol,
			 const OctetStr &auth_key,
			 const long priv_protocol,
			 const OctetStr &priv_key);

  /**
   * Generate localized keys for the given params.
   *
   * The buffers for the keys should be of size SNMPv3_USM_MAX_KEY_LEN.
   *
   * @param engine_id - 
   * @param auth_prot -
   * @param priv_prot -
   * @param auth_password     -
   * @param auth_password_len -
   * @param priv_password     -
   * @param priv_password_len -
   * @param auth_key     - allocated space for the authentication key
   * @param auth_key_len - IN: length of the buffer, OUT: key length
   * @param priv_key     - allocated space for the privacy key
   * @param priv_key_len - IN: length of the buffer, OUT: key length
   * @return SNMPv3_USM_OK, or USM error codes
   */
  int build_localized_keys(const OctetStr      &engine_id,
			   const int            auth_prot,
			   const int            priv_prot,
			   const unsigned char *auth_password,
			   const unsigned int   auth_password_len,
			   const unsigned char *priv_password,
			   const unsigned int   priv_password_len,
			   unsigned char *auth_key,
			   unsigned int  *auth_key_len,
			   unsigned char *priv_key,
			   unsigned int  *priv_key_len);

  /**
   * Delete all localized entries of this user from the usmUserTable.
   *
   * @param user_name - The userName that should be deleted
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (user deleted or not in table)
   */
  int delete_localized_user(const OctetStr& user_name);


  /**
   * Delete the entry with the given userName and engineID
   * from the usmUserTable
   *
   * @param engine_id - The engineID
   * @param user_name - The userName that should be deleted
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (user deleted or not in table)
   */
  int delete_localized_user(const OctetStr& engine_id,
			    const OctetStr& user_name);


  /**
   * Delete this engine id form all USM tables (users and engine time).
   *
   * @param engine_id - the engine id
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (entries deleted or not in table)
   */
  int remove_engine_id(const OctetStr &engine_id);

  /**
   * Delete the time information for the given engine id.
   *
   * @param engine_id - the engine id
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (entry deleted or not in table)
   */
  int remove_time_information(const OctetStr &engine_id);

  /**
   * Replace a localized key of the user and engineID in the
   * usmUserTable.
   *
   * @param user_name     - The name of the user in the USM
   * @param user_name_len - The length of the user name
   * @param engine_id     - Change the localized key for the SNMP
   *                        entity with this engine id
   * @param engine_id_len - The length of the engine id
   * @param new_key       - The new key
   * @param new_key_len   - The length of the new key
   * @param type_of_key   - AUTHKEY, OWNAUTHKEY, PRIVKEY or OWNPRIVKEY
   *
   * @return - SNMPv3_USM_ERROR (no such entry or not initialized),
   *           SNMPv3_USM_OK
   */
   int update_key(const unsigned char* user_name, const long user_name_len,
	          const unsigned char* engine_id, const long engine_id_len,
	          const unsigned char* new_key,   const long new_key_len,
	          const int type_of_key);

  /**
   * Search for a user with the given securityName and engineID
   * in the usmUserTable and return the entry. If no entry
   * could be found, the usmUserNameTable is searched for the given
   * securityName. If this table has an entry of this user, a
   * localized entry is generated, added to the usmUserTable and
   * returned to the caller.
   *
   * The caller has to call free_user() with the returned struct.
   *
   * @param engine_id         -
   * @param security_name     -
   *
   * @return - a pointer to the structure if an entry could be found
   *           or was generated, NULL for all errors
   */
  struct UsmUser *get_user(const OctetStr &engine_id,
			   const OctetStr &security_name);

  /**
   * Free the structure returned from get_user(OctetStr,OctetStr).
   */
  void free_user(struct UsmUser *&user);

  /**
   * Get the security name from a user name.
   *
   * @param user_name         -
   * @param user_name_len     -
   * @param security_name     - Buffer for the securityName
   *
   * @return - SNMPv3_USM_ERROR (not initialized, not found, buffer too small),
   *           SNMPv3_USM_OK
   */
  int get_security_name(const unsigned char *user_name,
			const long int user_name_len,
			OctetStr &security_name);

  /**
   * Get the user name from a security name.
   *
   * @param user_name         - Buffer for the userName
   * @param user_name_len     - Has to be set to the max length of the
   *                            buffer. Is set to the length of the found
   *                            securityName or to 0 if not found.
   * @param security_name     -
   * @param security_name_len -
   *
   * @return - SNMPv3_USM_ERROR (not initialized, not found, buffer too small),
   *           SNMPv3_USM_OK
   */
  int get_user_name(unsigned char *user_name,
		    long int *user_name_len,
		    const unsigned char *security_name,
		    const long int security_name_len);


  /**
   * Prepare a key update in the USM. The following procedure is used: To
   * prepare the key update, this function adds the neccessary variable
   * bindings to the Pdu to do the key update on the target SNMP entity.
   * The Pdu has to be sent to the target. If the key update on the target
   * is successful, usmCommitKeyUpdate() has to be called to do the local key
   * update. On failure usmAbortKeyUpdate() has to be called to free
   * temporary ressources.
   *
   * @param securityName - The name of the user
   * @param target       - A target to identify the SNMP entity on which the
   *                       key will be updated
   * @param newPassword  - The new password for the user
   * @param pdu          - A PDU into which this funktion adds the VBs needed
   *                       to change the keys on the target
   * @param type         - Indicates how and which key should be chaned:
   *                       possilbe values are: AUTHKEY, PRIVKEY and
   *                       OWNAUTHKEY, OWNPRIVKEY.
   * @param status       - The return status: SNMPv3_USM_OK or one of the
   *                       error codes
   *
   * @return - A structure, that is needed to commit/abort the key update.
   *           If an error occurs, the return value is NULL
   */
  struct UsmKeyUpdate* key_update_prepare(const OctetStr& securityName,
					  SnmpTarget& target,
					  const OctetStr& newPassword,
					  Pdu& pdu, int type,
					  int &status,
					  const OctetStr& oldpass = "",
					  const OctetStr& oldengid= "",
					  const OctetStr& newengid= "");

  /**
   * Abort the local key update.
   *
   * @param uku - The pointer returned by usmPrepareKeyUpdate()
   */
  void key_update_abort(struct UsmKeyUpdate *uku);


  /**
   * Commit the local key update.
   *
   * @param uku - The pointer returned by usmPrepareKeyUpdate()
   * @param update_type - One of USM_KeyUpdate, USM_PasswordKeyUpdate,
   *                      USM_PasswordAllKeyUpdate
   *
   * @return - SNMPv3_USM_ERROR, SNMPv3_USM_OK
   */
  int key_update_commit(struct UsmKeyUpdate *uku, int update_type);


  /**
   * Get a pointer to the AuthPriv object used by the USM.
   *
   */
  AuthPriv *get_auth_priv();


  /**
   * Return engineBoots and engineTime for a given engineID
   *
   * @param  engine_id    - The engineID of the SNMP entity
   * @param  engine_boots - OUT: boot counter (0 if not found)
   * @param  engine_time  - OUT: engine time (0 if not found)
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (entry found, values are filled)
   *           SNMPv3_USM_UNKNOWN_ENGINEID ( not found)
   */
  int get_time(const OctetStr &engine_id,
	       long int *engine_boots, long int *engine_time);



  /**
   * Return engineBoots and engineTime of the local snmp entity
   *
   * @param engine_boots - OUT: boot counter (0 if not found)
   * @param engine_time  - OUT: engine time (0 if not found)
   *
   * @return - SNMPv3_USM_ERROR (not initialized),
   *           SNMPv3_USM_OK (entry found, values are filled)
   */
  int get_local_time(long int *engine_boots, long int *engine_time) const;


  /**
   * Return the local snmp engine id.
   */
  const OctetStr& get_local_engine_id() const { return local_snmp_engine_id; };

  /**
   * Get the number of received messages with an unsupported securityLevel
   *
   * @return - usmStatsUnsupportedSecLevels
   */
  unsigned long get_stats_unsupported_sec_levels() const
    { return usmStatsUnsupportedSecLevels; };

  /**
   * Get the number of received messages outside time window
   *
   * @return - usmStatsNotInTimeWindows
   */
  unsigned long get_stats_not_in_time_windows() const
    { return usmStatsNotInTimeWindows; };

  /**
   * Get the number of received messages with a unknown userName
   *
   * @return - usmStatsUnknownUserNames
   */
  unsigned long get_stats_unknown_user_names() const
    { return usmStatsUnknownUserNames; };

  /**
   * Get the number of received messages with a unknown engineID
   *
   * @return - usmStatsUnknownEngineIDs
   */
  unsigned long get_stats_unknown_engine_ids() const
    { return usmStatsUnknownEngineIDs; };

  /**
   * Get the number of received messages with a wrong digest
   *
   * @return - usmStatsWrongDigests
   */
  unsigned long get_stats_wrong_digests() const
    { return usmStatsWrongDigests; };

  /**
   * Get the number of received messages with decryption errors
   *
   * @return - usmStatsDecryptionErrors
   */
  unsigned long get_stats_decryption_errors() const
    { return usmStatsDecryptionErrors; };

  //@{
  /**
   * Increase the stats counter. Should only be used by agent++.
   */
  void inc_stats_unsupported_sec_levels();
  void inc_stats_not_in_time_windows();
  void inc_stats_unknown_user_names();
  void inc_stats_unknown_engine_ids();
  void inc_stats_wrong_digests();
  void inc_stats_decryption_errors();
  //@}

  /**
   * Lock the UsmUserNameTable for access through peek_first_user()
   * and peek_next_user().
   */
  void lock_user_name_table();

  /**
   * Get a const pointer to the first entry of the UsmUserNameTable.
   *
   * @note Use lock_user_name_table() and unlock_user_name_table()
   *       for thread safety.
   */
  const UsmUserNameTableEntry *peek_first_user();

  /**
   * Get a const pointer to the next entry of the UsmUserNameTable.
   *
   * @note Use lock_user_name_table() and unlock_user_name_table()
   *       for thread safety.
   */
  const UsmUserNameTableEntry *peek_next_user(const UsmUserNameTableEntry *e);

  /**
   * Unlock the UsmUserNameTable after access through peek_first_user()
   * and peek_next_user().
   */
  void unlock_user_name_table();

  /**
   * Lock the UsmUserTable for access through peek_first_luser()
   * and peek_next_luser().
   */
  void lock_user_table();

  /**
   * Get a const pointer to the first entry of the UsmUserTable.
   *
   * @note Use lock_user_table() and unlock_user_table()
   *       for thread safety.
   */
  const UsmUserTableEntry *peek_first_luser();

  /**
   * Get a const pointer to the next entry of the UsmUserTable.
   *
   * @note Use lock_user_table() and unlock_user_table()
   *       for thread safety.
   */
  const UsmUserTableEntry *peek_next_luser(const UsmUserTableEntry *e);

  /**
   * Unlock the UsmUserTable after access through peek_first_luser()
   * and peek_next_luser().
   */
  void unlock_user_table();

  /**
   * for v3MP:
   *
   * Delete the pointers within the structure and the structure
   * itself.
   *
   * @param ssr - The structure that should be deleted.
   */
  void delete_sec_state_reference(struct SecurityStateReference *ssr);

  /**
   * Protected (for agent++):
   *
   * Get the user at the specified position of the usmUserTable.
   *
   * The returned pointer must NOT be deleted!
   *
   * @note lock_user_table() and unlock_user_table() must be used
   *       for thread synchronization.
   *
   * @param number - get the entry at position number (1...)
   *
   * @return - a pointer to the structure or NULL if number is out
   *           of range
   */
  const struct UsmUserTableEntry *get_user(int number);

  /**
   * Get the properties of the specified user.
   *
   * The returned pointer must NOT be deleted!
   *
   * @note lock_user_table() and unlock_user_table() must be used
   *       for thread synchronization.
   *
   * @param security_name - The security name of the user
   *
   * @return - a pointer to the structure or NULL if number is out
   *           of range
   */
  const struct UsmUserNameTableEntry *get_user(const OctetStr &security_name);

  /**
   * Protected (for agent++):
   *
   * Get the number of elements in the usmUserTable
   *
   * @note lock_user_table() and unlock_user_table() must be used
   *       for thread synchronization.
   *
   * @return - number of elements
   */
  int get_user_count() const;


  /**
   * Protected (for agent++)
   *
   * Register a callback function that is called if a new localized user
   * has been added to the usm user table
   */
  void add_user_added_callback(const usm_add_user_callback cb);

  /**
   * Clear all user configuration from this USM instance. This method is
   * not synchronized. Do not use it while the USM is being used by other 
   * threads.
   * @return
   *    SNMPv3_USM_OK on success.
   */
  int remove_all_users();

 protected:

  /**
   * Get a new security state reference (for v3MP).
   *
   * @return - A newly created security state reference.
   */
  struct SecurityStateReference *get_new_sec_state_reference();

  /**
   * Generate a complete message that is ready to send to the target.
   *
   * @param globalData       - Buffer containing the serialized globalData,
   *                           ready to be copied into the wholeMsg
   * @param globalDataLength - The length of this buffer
   * @param maxMessageSize   - The maximum message size
   * @param securityEngineID - The engineID of the authoritative SNMP entity
   * @param securityName     - The name of the user
   * @param securityLevel    - The security Level for this Message
   * @param scopedPDU        - Buffer containing the serialized scopedPDU,
   *                           ready to be copied into the wholeMsg
   * @param scopedPDULength  - The length of this Buffer
   * @param securityStateReference - The reference that was generated when
   *                                 the request was parsed. For request, this
   *                                 param has to be NULL. The reference
   *                                 is deleted by this function.
   * @param wholeMsg         - OUT: the buffer for the whole message
   * @param wholeMsgLength   - IN:  lenght of the buffer.
   *                           OUT: length of the generated message
   *
   * @return - SNMPv3_USM_OK on success. See snmperrs.h for the error codes
   *           of the USM.
   */
  int generate_msg(
             unsigned char *globalData,       // message header, admin data
             int globalDataLength,
             int maxMessageSize,              // of the sending SNMP entity
             const OctetStr &securityEngineID,// authoritative SNMP entity
             const OctetStr &securityName,    // on behalf of this principal
             int  securityLevel,              // Level of Security requested
             unsigned char  *scopedPDU,       // message (plaintext) payload
             int scopedPDULength,
             struct SecurityStateReference *securityStateReference,
             unsigned char *wholeMsg,         // OUT complete generated message
             int *wholeMsgLength);            // OUT length of generated message



  /**
   * Parse a received message.
   *
   * @param maxMessageSize         - The maximum message size of the snding
   *                                 SNMP entity.
   * @param securityParameters     - The security parameters as received
   * @param securityParametersLength - The length of the security parameters
   * @param securityParametersPosition - The position of the security
   *                                     parameters in the message
   * @param securityLevel          - The securityLevel of the message
   * @param wholeMsg               - The buffer with the whole message
   * @param wholeMsgLength         - The length of the whole message
   * @param msgData                - The buffer with the messageData
   * @param msgDataLength          - The length of the messageData buffer
   * @param security_engine_id     - OUT: the authoritative engineID
   * @param security_name          - OUT: the name of the user
   * @param scopedPDU              - OUT: buffer containing the scopedPDU
   * @param scopedPDULength        - IN: length of the buffer
   *                                 OUT: length of the scopedPDU
   * @param maxSizeResponseScopedPDU - OUT: maximum size for a scopedPDU in a
   *                                        response message
   * @param securityStateReference - OUT: the securityStateReference
   * @param fromAddress            - IN: Address of the sender
   *
   * @return - SNMPv3_USM_OK on success. See snmperrs.h for the error codes
   *           of the USM.
   */
  int process_msg(
           int maxMessageSize,                // of the sending SNMP entity
           unsigned char *securityParameters, // for the received message
           int securityParametersLength,
           int securityParametersPosition,
           long int securityLevel,            // Level of Security
           unsigned char *wholeMsg,           // as received on the wire
           int wholeMsgLength,                // length as received on the wire
           unsigned char *msgData,
           int msgDataLength,
	   OctetStr &security_engine_id,      // authoritative SNMP entity
	   OctetStr &security_name,           //identification of the principal
           unsigned char *scopedPDU,          // message (plaintext) payload
           int *scopedPDULength,
           long *maxSizeResponseScopedPDU,// maximum size of the Response PDU
           struct SecurityStateReference *securityStateReference,
                                            // reference to security state
                                            // information, needed for response
           const UdpAddress &fromAddress);  // Address of the sender

private:

  /**
   * Delete the pointers in the structure and set all values to 0/NULL.
   *
   * @param usp - The structure that should be deleted
   */
  void delete_sec_parameters( struct UsmSecurityParameters *usp);


  /**
   * Serialize the given values into the buffer according to the BER.
   *
   *  UsmSecurityParameters ::=
   *      SEQUENCE {
   *      -- global User-based security parameters
   *          msgAuthoritativeEngineID     OCTET STRING (5..32)
   *          msgAuthoritativeEngineBoots  INTEGER (0..2147483647),
   *          msgAuthoritativeEngineTime   INTEGER (0..2147483647),
   *          msgUserName                  OCTET STRING (SIZE(0..32)),
   *       -- authentication protocol specific parameters
   *          msgAuthenticationParameters  OCTET STRING,
   *       -- privacy protocol specific parameters
   *          msgPrivacyParameters         OCTET STRING
   *      }
   *
   * @param outBuf    - buffer for the serialized values
   * @param maxLength - before call: length of the buffer
   *                    after call: bytes left in the buffer
   * @param sp        - the values to serialize
   * @param position  - after call: points to the first byte of the
   *                    field for the authentication parameter
   *
   * @return - a pointer to the first free byte in the buffer,
   *           NULL on error
   */
  unsigned char *build_sec_params(unsigned char *outBuf, int *maxLength,
                                  struct UsmSecurityParameters sp,
                                  int *position);

  /**
   * Serialize the given values acording to the BER into the
   * buffer. On success, the buffer contains a valid SNMPv3 message.
   *
   * @param outBuf             - buffer for the serialized values
   * @param maxLength          - before call: length of the buffer
   *                             after call: bytes left in the buffer
   * @param globalData         - Buffer that contains the serialized globalData
   * @param globalDataLength   - The length of this buffer
   * @param positionAuthPar    - after call: points to the first byte of the
   *                             field for the authentication parameter
   * @param securityParameters - The security parameters
   * @param msgData            - Buffer that contains the serialized msgData
   * @param msgDataLength      - The length of this buffer
   *
   * @return - a pointer to the first free byte in the buffer,
   * NULL on error
   */
  unsigned char *build_whole_msg(
                      unsigned char *outBuf, int *maxLength,
                      unsigned char *globalData, long int globalDataLength,
                      int *positionAuthPar,
                      struct UsmSecurityParameters  securityParameters,
                      unsigned char *msgData, long int msgDataLength);


  /**
   * Delete the pointers in the structure
   *
   * @param user - The structure that should be deleted
   */
  inline void delete_user_ptr(struct UsmUser *user);


 private:

  OctetStr local_snmp_engine_id; ///< local snmp engine id
  const v3MP *v3mp;          ///< Pointer to the v3MP that created this object

  // 0: don't accept messages from hosts with a unknown engine id
  bool discovery_mode;

   // MIB Counters
   unsigned int usmStatsUnsupportedSecLevels;
   unsigned int usmStatsNotInTimeWindows;
   unsigned int usmStatsUnknownUserNames;
   unsigned int usmStatsUnknownEngineIDs;
   unsigned int usmStatsWrongDigests;
   unsigned int usmStatsDecryptionErrors;

   // the instance of AuthPriv
   AuthPriv *auth_priv;

   // this table contains time values of contacted snmp entities
   USMTimeTable *usm_time_table;

   // Users that are known but not localized to a engine ID
   USMUserNameTable *usm_user_name_table;

   // Table containing localized Users ready to use
   USMUserTable *usm_user_table;

   // Callback for agent++ to indicate new users in usm tables
   usm_add_user_callback usm_add_user_cb;

};


// only for compatibility do not use these values and functions:
// =============================================================

#define SecurityLevel_noAuthNoPriv    SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV
#define SecurityLevel_authNoPriv      SNMP_SECURITY_LEVEL_AUTH_NOPRIV
#define SecurityLevel_authPriv        SNMP_SECURITY_LEVEL_AUTH_PRIV

#define SNMPv3_usmNoAuthProtocol      SNMP_AUTHPROTOCOL_NONE
#define SNMPv3_usmHMACMD5AuthProtocol SNMP_AUTHPROTOCOL_HMACMD5
#define SNMPv3_usmHMACSHAAuthProtocol SNMP_AUTHPROTOCOL_HMACSHA

#define SNMPv3_usmNoPrivProtocol     SNMP_PRIVPROTOCOL_NONE
#define SNMPv3_usmDESPrivProtocol    SNMP_PRIVPROTOCOL_DES
#define SNMPv3_usmIDEAPrivProtocol   SNMP_PRIVPROTOCOL_IDEA
#define SNMPv3_usmAES128PrivProtocol SNMP_PRIVPROTOCOL_AES128
#define SNMPv3_usmAES192PrivProtocol SNMP_PRIVPROTOCOL_AES192
#define SNMPv3_usmAES256PrivProtocol SNMP_PRIVPROTOCOL_AES256

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _SNMPv3

#endif
