/*_############################################################################
  _## 
  _##  auth_priv.h  
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
// $Id: auth_priv.h 2833 2015-03-15 10:33:08Z katz $

#ifndef _AUTH_PRIV_
#define _AUTH_PRIV_

#include "snmp_pp/config_snmp_pp.h"

#ifdef _SNMPv3

#include "snmp_pp/usm_v3.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#define SNMPv3_USM_MAX_KEY_LEN        32

/* Accept Messages with auth/priv param fields up to this length */
#define SNMPv3_AP_MAXLENGTH_AUTHPARAM      128
#define SNMPv3_AP_MAXLENGTH_PRIVPARAM      128


#define SNMPv3_AP_OUTPUT_LENGTH_MD5   16
#define SNMPv3_AP_OUTPUT_LENGTH_SHA   20

class OctetStr;

/**
 * Abstract class for auth modules.
 *
 * This class has to be subclassed to add new authentication
 * protocols.
 *
 */
class DLLOPT Auth
{
public:

  virtual ~Auth() {}

  /**
   * Generate the localized key for the given password and engine id.
   *
   * @param password      - the password
   * @param password_len  - the length of the password
   * @param engine_id     - pointer to snmpEngineID
   * @param engine_id_len - length of snmpEngineID
   * @param key           - pointer to an empty buffer that will be filled
   *                        with generated key
   * @param key_len       - IN: length of the buffer
   *                        OUT: length of the key
   *
   * @return SNMPv3_USM_OK on success
   */
  virtual int password_to_key(const unsigned char *password,
                              const unsigned int   password_len,
                              const unsigned char *engine_id,
                              const unsigned int   engine_id_len,
                              unsigned char       *key,
                              unsigned int        *key_len) = 0;

  /**
   * Generate a hash value for the given data.
   *
   * @param data      - the data
   * @param data_len  - the length of the data
   * @param digest    - pointer to the generated digest
   *
   * @return SNMPv3_USM_OK on success
   */
  virtual int hash(const unsigned char *data,
                   const unsigned int   data_len,
                   unsigned char       *digest) const = 0;

  /**
   * Authenticate an outgoing message.
   *
   * This method fills the authentication parameters field of the
   * given message. The param auth_par_ptr is pointing inside the
   * message buffer and must be zeroed before the authentication value
   * is computed.
   *
   * @param key          - pointer to the (fixed length) key
   * @param msg          - pointer to the whole message
   * @param msg_len      - the length of the message
   * @param auth_par_ptr - pointer to the auth field inside the msg buffer
   *
   * @return SNMPv3_USM_OK on success and
   *         SNMPv3_USM_ERROR for unexpected errors.
   */
  virtual int auth_out_msg(const unsigned char *key,
                           unsigned char       *msg,
                           const int            msg_len,
                           unsigned char       *auth_par_ptr) = 0;


  /**
   * Authenticate an incoming message.
   *
   * This method checks if the value in the authentication parameters
   * field of the message is valid.
   *
   * The following procedure is used to verify the authenitcation value
   * - copy the authentication value to a temp buffer
   * - zero the auth field
   * - recalculate the authenthication value
   * - compare the two authentcation values
   * - write back the received authentication value if values differ
   *
   * @param key          - pointer to the (fixed length) key
   * @param msg          - pointer to the whole message
   * @param msg_len      - the length of the message
   * @param auth_par_ptr - pointer to the auth field inside the msg buffer
   * @param auth_par_len - Length of the received auth field
   *
   * @return SNMPv3_USM_OK if the msg is valid,
   *         SNMPv3_USM_AUTHENTICATION_FAILURE if not and
   *         SNMPv3_USM_ERROR for unexpected errors.
   */
  virtual int auth_inc_msg(const unsigned char *key,
                           unsigned char       *msg,
                           const int            msg_len,
                           unsigned char       *auth_par_ptr,
                           const int            auth_par_len) = 0;

  /**
   * Get the unique id of the authentication protocol.
   */
  virtual int get_id() const = 0;


  /**
   * Get the unique identifier string of the authentication protocol.
   */
  virtual const char *get_id_string() const = 0;

  /**
   * Set the pointer to the salt that should be used.
   */
  virtual void set_salt(pp_uint64 *new_salt) { salt = new_salt; };

  /**
   * Get the maximum length that is needed for the
   * msgAuthenticationParameters field.
   */
  virtual int get_auth_params_len() const = 0;

  /**
   * Get length of a hash output.
   */
  virtual int get_hash_len() const = 0;

 protected:
  pp_uint64 *salt;
};


/**
 * Abstract class for priv modules
 *
 * This class has to be subclassed to add new privacy
 * protocols.
 *
 */
class DLLOPT Priv
{
public:
  virtual ~Priv() {}

  /**
   * Encrypt the buffer with the given key.
   *
   * This method fills the privacy parameters field of the given
   * message.
   *
   * @param key            - pointer to the encryption key
   * @param key_len        - length of encryption key
   * @param buffer         - pointer to the unencrypted buffer
   * @param buffer_len     - length of the buffer
   * @param out_buffer     - pointer to the buffer for the encryptet data
   * @param out_buffer_len - Input:  Length of the output buffer.
   *                         Output: Bytes written
   * @param privacy_params - Buffer, where the privacy parameters
   *                         are written to.
   * @param privacy_params_len - Length of the privacy parameters buffer
   * @param engine_boots   - The engine boots value for the message
   * @param engine_time    - The engine time value for the message
   *
   * @return SNMPv3_USM_OK on success
   */
  virtual int encrypt(const unsigned char *key,
                      const unsigned int   key_len,
                      const unsigned char *buffer,
                      const unsigned int   buffer_len,
                      unsigned char       *out_buffer,
                      unsigned int        *out_buffer_len,
                      unsigned char       *privacy_params,
                      unsigned int        *privacy_params_len,
                      const unsigned long  engine_boots,
                      const unsigned long  engine_time) = 0;


  /**
   * Decrypt the buffer with the given key.
   *
   * This method needs the privacy parameters field for the given
   * message.
   *
   * @param key            - pointer to the (fixed length) dencryption key
   * @param key_len        - length of encryption key
   * @param buffer         - pointer to the encrypted buffer
   * @param buffer_len     - length of the buffer
   * @param out_buffer     - pointer to the buffer for the decryptet data
   * @param out_buffer_len - Input:  Length of the output buffer.
   *                         Output: Bytes written
   * @param privacy_params - Buffer, where the privacy parameters
   *                         are read from.
   * @param privacy_params_len - Length of the privacy parameters buffer
   * @param engine_boots   - The engine boots value for the message
   * @param engine_time    - The engine time value for the message
   *
   * @return SNMPv3_USM_OK on success
   */
  virtual int decrypt(const unsigned char *key,
                      const unsigned int   key_len,
                      const unsigned char *buffer,
                      const unsigned int   buffer_len,
                      unsigned char       *out_buffer,
                      unsigned int        *out_buffer_len,
                      const unsigned char *privacy_params,
                      const unsigned int   privacy_params_len,
		      const unsigned long  engine_boots,
		      const unsigned long  engine_time) = 0;

  /**
   * Extend a localized key that is too short.
   *
   * Some privacy protocols require a key that is longer than the key
   * generated by the pasword to key algorithm of the authentication
   * protocol. This function extends a short key to the required length.
   *
   * @param password      - the password
   * @param password_len  - the length of the password
   * @param engine_id     - pointer to snmpEngineID
   * @param engine_id_len - length of snmpEngineID
   * @param key           - pointer to the short key that was generated
   *                        using Auth::password_to_key() function
   * @param key_len       - IN: length of the short key
   *                        OUT: length of the extended key
   * @param max_key_len   - Length of the key buffer
   * @param auth          - Pointer of the authentication protocol that
   *                        should be used
   *
   * @return SNMPv3_USM_OK on success
   */

  virtual int extend_short_key(const unsigned char *password,
                               const unsigned int   password_len,
                               const unsigned char *engine_id,
                               const unsigned int   engine_id_len,
                               unsigned char       *key,
                               unsigned int        *key_len,
                               const unsigned int   max_key_len,
                               Auth                *auth) = 0;

  /**
   * Get the uniqhe id of the privacy protocol.
   */
  virtual int get_id() const = 0;

  /**
   * Get the unique identifier string of the privacy protocol.
   */
  virtual const char *get_id_string() const = 0;

  /**
   * Set the pointer to the salt that should be used.
   */
  virtual void set_salt(pp_uint64 *new_salt) { salt = new_salt; }

  /**
   * Get the maximum length that is needed for the
   * msgPrivacyParameters field.
   */
  virtual int get_priv_params_len() const = 0;

  /**
   * Get the minimum key length needed for encryption and decryption.
   */
  virtual int get_min_key_len() const = 0;

  /**
   * Decrease a too long length to the right value.
   */
  virtual void fix_key_len(unsigned int &key_len) const = 0;

 protected:
  pp_uint64 *salt;

};

typedef Auth* AuthPtr;
typedef Priv* PrivPtr;


/**
 * Class that holds all authentication and privacy protocols
 * for a snmp entity.
 */
class DLLOPT AuthPriv
{
public:

  /**
   * Default constructor, initializes random values
   */
  AuthPriv(int &construct_state);

  /**
   * Destructor, deletes all auth and priv protocol objets.
   */
  ~AuthPriv();

  /**
   * Add the default authentication protocols.
   *
   * The following authentication protocols are added:
   * - MD5
   * - SHA
   *
   * The following privacy protocols are added:
   * - DES
   * - AES128, AES192 and AES256 if libtomcrypt or OpenSSL is enabled
   * - IDEA if enabled
   *
   * @return SNMP_CLASS_SUCCESS or SNMP_CLASS_ERROR.
   */
  int add_default_modules();

  /**
   * Add a new authentication protocol.
   *
   * All added objects will be deleted in the destructor
   *
   * @param auth - Pointer to a new auth protocol object
   *
   * @return SNMP_CLASS_SUCCESS or SNMP_CLASS_ERROR
   */
  int add_auth(Auth *auth);

  /**
   * Delete a authentication protocol.
   *
   * @param auth_id - The id of the authentication protocol to remove
   *
   * @return SNMP_CLASS_SUCCESS or SNMP_CLASS_ERROR
   */
  int del_auth(const int auth_id);

  /**
   * Add a new privacy protocol.
   *
   * All added objects will be deleted in the destructor
   *
   * @param priv - Pointer to a new privacy protocol object
   *
   * @return SNMP_CLASS_SUCCESS or SNMP_CLASS_ERROR
   */
  int add_priv(Priv *priv);

  /**
   * Delete a privacy protocol.
   *
   * @param priv_id - The id of the privacy protocol to remove
   *
   * @return SNMP_CLASS_SUCCESS or SNMP_CLASS_ERROR
   */
  int del_priv(const int priv_id);

  /**
   * Call the password-to-key method of the specified authentication
   * protocol.
   */
  int password_to_key_auth(const int      auth_prot,
                           const unsigned char *password,
                           const unsigned int   password_len,
                           const unsigned char *engine_id,
                           const unsigned int   engine_id_len,
                           unsigned char *key,
                           unsigned int  *key_len);

  /**
   * Call the password-to-key method of the specified privacy
   * protocol.
   */
  int password_to_key_priv(const int      auth_prot,
                           const int      priv_prot,
                           const unsigned char *password,
                           const unsigned int   password_len,
                           const unsigned char *engine_id,
                           const unsigned int   engine_id_len,
                           unsigned char *key,
                           unsigned int  *key_len);

  /**
   * Get the keyChange value for the specified keys using the given
   * authentication protocol.
   */
  int get_keychange_value(const int       auth_prot,
                          const OctetStr& old_key,
                          const OctetStr& new_key,
                          OctetStr&       keychange_value);

  /**
   * Get a pointer to a privacy protocol object.
   */
  Priv *get_priv(const int priv_prot);

  /**
   * Get a pointer to a authentication protocol object.
   */
  Auth *get_auth(const int auth_prot);

  /**
   * Get the unique id for the given auth protocol.
   *
   * @param string_id - The string returned by Auth::get_id_string()
   *
   * @return The id or -1
   */
  int get_auth_id(const char *string_id) const;

  /**
   * Get the unique id for the given priv protocol.
   *
   * @param string_id - The string returned by Priv::get_id_string()
   *
   * @return The id or -1
   */
  int get_priv_id(const char *string_id) const;

  /**
   * Encrypt a message.
   */
  int encrypt_msg(const int            priv_prot,
                  const unsigned char *key,
                  const unsigned int   key_len,
                  const unsigned char *buffer,
                  const unsigned int   buffer_len,
                  unsigned char       *out_buffer,
                  unsigned int        *out_buffer_len,
                  unsigned char       *privacy_params,
                  unsigned int        *privacy_params_len,
                  const unsigned long  engine_boots,
                  const unsigned long  engine_time);

  /**
   * Decrypt a message.
   */
  int decrypt_msg(const int            priv_prot,
                  const unsigned char *key,
                  const unsigned int   key_len,
                  const unsigned char *buffer,
                  const unsigned int   buffer_len,
                  unsigned char       *out_buffer,
                  unsigned int        *out_buffer_len,
                  const unsigned char *privacy_params,
                  const unsigned int   privacy_params_len,
		  const unsigned long  engine_boots,
		  const unsigned long  engine_time);

  /**
   * Get the length of the authentication parameters field of the given
   * authentication protocol.
   */
  int get_auth_params_len(const int auth_prot);

  /**
   * Get the length of the privacy parameters field of the given
   * privacy protocol.
   */
  int get_priv_params_len(const int priv_prot);

  /**
   * Fill in the authentication field of an outgoing message
   */
  int auth_out_msg(const int            auth_prot,
                   const unsigned char *key,
                   unsigned char       *msg,
                   const int            msg_len,
                   unsigned char       *auth_par_ptr);

  /**
   * Check the authentication field of an incoming message
   */
  int auth_inc_msg(const int            auth_prot,
                   const unsigned char *key,
                   unsigned char       *msg,
                   const int            msg_len,
                   unsigned char       *auth_par_ptr,
                   const int            auth_par_len);

private:

  AuthPtr *auth;   ///< Array of pointers to Auth-objects
  PrivPtr *priv;   ///< Array of pointers to Priv-objects
  int   auth_size; ///< current size of the auth array
  int   priv_size; ///< current size of the priv array
  pp_uint64 salt;  ///< current salt value (64 bits)
};


/**
 * Authentication module using SHA.
 *
 * @see Auth
 */
class DLLOPT AuthSHA: public Auth
{
public:
  int password_to_key(const unsigned char *password,
		      const unsigned int   password_len,
		      const unsigned char *engine_id,
		      const unsigned int   engine_id_len,
		      unsigned char       *key,
		      unsigned int        *key_len);

  int hash(const unsigned char *data,
	   const unsigned int   data_len,
	   unsigned char       *digest) const;

  int auth_out_msg(const unsigned char *key,
		   unsigned char       *msg,
		   const int            msg_len,
		   unsigned char       *auth_par_ptr);

  int auth_inc_msg(const unsigned char *key,
		   unsigned char       *msg,
		   const int            msg_len,
		   unsigned char       *auth_par_ptr,
                   const int            auth_par_len);

  int get_id() const { return SNMP_AUTHPROTOCOL_HMACSHA; };

  const char *get_id_string() const { return "HMAC-SHA"; };

  int get_auth_params_len() const { return 12; };

  int get_hash_len() const { return SNMPv3_AP_OUTPUT_LENGTH_SHA;};
};

/**
 * Authentication module using MD5.
 *
 * @see Auth
 */
class DLLOPT AuthMD5: public Auth
{
public:
  int password_to_key(const unsigned char *password,
		      const unsigned int   password_len,
		      const unsigned char *engine_id,
		      const unsigned int   engine_id_len,
		      unsigned char       *key,
		      unsigned int        *key_len);

  int hash(const unsigned char *data,
	   const unsigned int   data_len,
	   unsigned char       *digest) const;

  int auth_out_msg(const unsigned char *key,
		   unsigned char       *msg,
		   const int            msg_len,
		   unsigned char       *auth_par_ptr);


  int auth_inc_msg(const unsigned char *key,
		   unsigned char       *msg,
		   const int            msg_len,
		   unsigned char       *auth_par_ptr,
                   const int            auth_par_len);

  int get_id() const { return SNMP_AUTHPROTOCOL_HMACMD5; };

  const char *get_id_string() const { return "HMAC-MD5"; };

  int get_auth_params_len() const { return 12; };

  int get_hash_len() const { return SNMPv3_AP_OUTPUT_LENGTH_MD5;};
};

/**
 * Encryption module using DES.
 *
 * @see Priv
 */
class DLLOPT PrivDES: public Priv
{
 public:
#if defined(_USE_LIBTOMCRYPT) && !defined(_USE_OPENSSL)
  PrivDES();
 private:
  int cipher;
 public:
#endif
  int encrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              unsigned char       *privacy_params,
              unsigned int        *privacy_params_len,
              const unsigned long  engine_boots,
              const unsigned long  engine_time);

  int decrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              const unsigned char *privacy_params,
              const unsigned int   privacy_params_len,
	      const unsigned long  engine_boots,
	      const unsigned long  engine_time);

  int extend_short_key(const unsigned char *password,
                       const unsigned int   password_len,
                       const unsigned char *engine_id,
                       const unsigned int   engine_id_len,
                       unsigned char       *key,
                       unsigned int        *key_len,
                       const unsigned int   max_key_len,
                       Auth                *auth)
  {
    (void)password;
    (void)password_len;
    (void)engine_id;
    (void)engine_id_len;
    (void)key;
    (void)key_len;
    (void)max_key_len;
    (void)auth;
    return SNMPv3_USM_ERROR; /* not needed for DES! */
  }

  int get_id() const { return SNMP_PRIVPROTOCOL_DES; };
  const char *get_id_string() const { return "DES"; };
  int get_priv_params_len() const { return 8; };
  int get_min_key_len() const { return 16; };
  void fix_key_len(unsigned int &key_len) const
    { key_len = (key_len >= 16 ? 16 : 0); };
};

#ifdef _USE_IDEA
/**
 * Encryption module using IDEA.
 *
 * @see Priv
 */
class DLLOPT PrivIDEA: public Priv
{
public:

  int encrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              unsigned char       *privacy_params,
              unsigned int        *privacy_params_len,
              const unsigned long  engine_boots,
              const unsigned long  engine_time);

  int decrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              const unsigned char *privacy_params,
              const unsigned int   privacy_params_len,
	      const unsigned long  engine_boots,
	      const unsigned long  engine_time);

  int extend_short_key(const unsigned char *password,
                       const unsigned int   password_len,
                       const unsigned char *engine_id,
                       const unsigned int   engine_id_len,
                       unsigned char       *key,
                       unsigned int        *key_len,
                       const unsigned int   max_key_len,
                       Auth                *auth)
    { return SNMPv3_USM_ERROR; /* not needed for IDEA! */ };

  int get_id() const { return SNMP_PRIVPROTOCOL_IDEA; };
  const char *get_id_string() const { return "IDEA"; };
  int get_priv_params_len() const { return 8; };
  int get_min_key_len() const { return 16; };
  void fix_key_len(unsigned int &key_len) const
    { key_len = (key_len >= 16 ? 16 : 0); };
};

#endif


#if defined(_USE_LIBTOMCRYPT) || defined(_USE_OPENSSL)

/**
 * Encryption module using AES.
 *
 * @see Priv
 */
class DLLOPT PrivAES: public Priv
{
public:

  PrivAES(const int aes_type_);

  int encrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              unsigned char       *privacy_params,
              unsigned int        *privacy_params_len,
              const unsigned long  engine_boots,
              const unsigned long  engine_time);

  int decrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              const unsigned char *privacy_params,
              const unsigned int   privacy_params_len,
	      const unsigned long  engine_boots,
	      const unsigned long  engine_time);

  int extend_short_key(const unsigned char *password,
                       const unsigned int   password_len,
                       const unsigned char *engine_id,
                       const unsigned int   engine_id_len,
                       unsigned char       *key,
                       unsigned int        *key_len,
                       const unsigned int   max_key_len,
                       Auth                *auth);

  int get_id() const { return aes_type; };
  const char *get_id_string() const;
  int get_priv_params_len() const { return 8; };
  int get_min_key_len() const { return key_bytes; };
  void fix_key_len(unsigned int &key_len) const
    { key_len = (key_len >= (unsigned)key_bytes ? key_bytes : 0); };

 private:
  int aes_type;
  int key_bytes;
  int rounds;
#if defined(_USE_LIBTOMCRYPT) && !defined(_USE_OPENSSL)
  int cipher;
#endif
  bool need_byteswap;
};

/**
 * Encryption module using AES but using non standard key extension.
 *
 * @note This class adds compatibility with some devices that
 *       illegally use the 3DES key extension algorithm with
 *       AES privacy.
 * @see PrivAES
 */
class DLLOPT PrivAESW3DESKeyExt: public PrivAES
{
public:

  PrivAESW3DESKeyExt(const int aes_type_);

  int extend_short_key(const unsigned char *password,
                       const unsigned int   password_len,
                       const unsigned char *engine_id,
                       const unsigned int   engine_id_len,
                       unsigned char       *key,
                       unsigned int        *key_len,
                       const unsigned int   max_key_len,
                       Auth                *auth);

  const char *get_id_string() const;

  static int map_aes_type(const int t);

private:
  int own_aes_type;
};

#endif // _USE_LIBTOMCRYPT or _USE_OPENSSL

#ifdef _USE_3DES_EDE
/**
 * Encryption module using TripleDES-EDE KEY
 * 
 *
 * @see Priv
 */
#define TRIPLEDES_EDE_KEY_LEN 32


class DLLOPT Priv3DES_EDE: public Priv
{
public:
#if defined(_USE_LIBTOMCRYPT) && !defined(_USE_OPENSSL)
  Priv3DES_EDE();
 private:
  int cipher;
 public:
#endif

  int encrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              unsigned char       *privacy_params,
              unsigned int        *privacy_params_len,
              const unsigned long  engine_boots,
              const unsigned long  engine_time);

  int decrypt(const unsigned char *key,
              const unsigned int   key_len,
              const unsigned char *buffer,
              const unsigned int   buffer_len,
              unsigned char       *out_buffer,
              unsigned int        *out_buffer_len,
              const unsigned char *privacy_params,
              const unsigned int   privacy_params_len,
	      const unsigned long  engine_boots,
	      const unsigned long  engine_time);

  int extend_short_key(const unsigned char *password,
                       const unsigned int   password_len,
                       const unsigned char *engine_id,
                       const unsigned int   engine_id_len,
                       unsigned char       *key,
                       unsigned int        *key_len,
                       const unsigned int   max_key_len,
                       Auth                *auth);

  int get_id() const { return SNMP_PRIVPROTOCOL_3DESEDE; };
  const char *get_id_string() const { return "3DESEDE"; };
  int get_priv_params_len() const { return 8; };
  int get_min_key_len() const { return TRIPLEDES_EDE_KEY_LEN; };
  void fix_key_len(unsigned int &key_len) const
    { key_len = (key_len >= TRIPLEDES_EDE_KEY_LEN
                                              ? TRIPLEDES_EDE_KEY_LEN : 0); };
#ifdef _TEST
  bool test();
#endif
};

#endif // _USE_3DES_EDE

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif // _SNMPv3

#endif
