/*                               -*- Mode: C -*- 
 * @file: cryptos_channel.h
 * @brief: 
 * @author: Jesus
 * Maintainer: 
 * @date: sáb jul 17 21:26:09 2010 (+0200)
 * @version: 
 * Last-Updated: lun ago 16 01:11:48 2010 (+0200)
 *           By: Jesus
 *     Update #: 84
 * URL: 
 */
#ifndef CRYPTOS_CHANNEL_H
#define CRYPTOS_CHANNEL_H

#include "cryptos_types.h"

/**
 * @def CRYPTOS_SYNC_HEADER_LEN
 * @brief Defines the number of bytes of the SYNC header field
 */
#define CRYPTOS_SYNC_HEADER_LEN 3

/**
 * @def CRYPTOS_LENGTH_HEADER_LEN
 * @brief Defines the number of bytes of the "length" header field
 */
#define CRYPTOS_LENGTH_HEADER_LEN 4

/**
 * @def CRYPTOS_IV_HEADER_LEN
 * @brief Defines the number of bytes of the IV header field
 */
#define CRYPTOS_IV_HEADER_LEN 16

/**
 * @def CRYPTOS_EMISSION_HEADER_LEN
 * @brief Defines the number of bytes of the EMISSION header field
 */
#define CRYPTOS_EMISSION_HEADER_LEN sizeof(uint64_t) /* 8 */

/**
 * @def CRYPTOS_PACKET_HEADER_LEN
 * @brief Defines the number of bytes of the PACKET header field
 */
#define CRYPTOS_PACKET_HEADER_LEN sizeof(uint64_t)/* 8 */

/**
 * @def CRYPTOS_MAX_DATA
 * @brief Defines the maximum number of bytes of the data field, which
 *  will depend on the CRYPTOS_LENGTH_HEADER_LEN bytes
 */
#define CRYPTOS_MAX_DATA 0xFFFFFFFF//(1 << (CRYPTOS_LENGTH_HEADER_LEN*BITS_PER_BYTE)) - 1

/**
 * @def CRYPTOS_DEFAULT_DATA_SIZE
 * @brief Defines the default size of the data field in crypto packets
 */
#define CRYPTOS_DEFAULT_DATA_SIZE 512

/**
 * @def CRYPTOS_MIN_DIGEST_LEN
 * @brief Defines the size of the message digest algorithm which produces the 
 *  minimum length digest, in bytes. Currently, CRC24.
 */
#define CRYPTOS_MIN_DIGEST_LEN 3

/**
 * @def CRYPTOS_MAX_DIGEST_LEN
 * @brief Defines the size of the message digest algorithm which produces the 
 *  maximum length digest, in bytes. Currently, SHA512.
 */
#define CRYPTOS_MAX_DIGEST_LEN 64

/**
 * @def RATIO_DD
 * @brief Defines the number of bytes of data per byte of message digest
 */
#define RATIO_DD 16

/**
 * @def CRYPTOS_BUFFER_MAX_SIZE
 * @brief The max size, in bytes, of the cryptos buffer. It is set to twice the
 *  size of a cryptos packet because it'll never store two whole packets.
 */
#define CRYPTOS_BUFFER_MAX_SIZE CRYPTOS_PACKET_MAX_LEN*2

/**
 * @var CRYPTOS_DEFAULT_IV
 * @brief Default IV to use when no IV is specified at the sender's side
 */
 // @todo Search better default IV values
static const byte CRYPTOS_DEFAULT_IV[CRYPTOS_IV_HEADER_LEN]={
  0x0A, 0x1B, 0x2C, 0x3D,
  0x4E, 0x5F, 0x6A, 0x7B,
  0x0A, 0x1B, 0x2C, 0x3D,
  0x4E, 0x5F, 0x6A, 0x7B};

/**
 * @var CRYPTOS_SYNC_HEADER
 * @brief Defines the synchro header field
 * @see CRYPTOS_SYNC_HEADER_LEN
 */
static const byte CRYPTOS_SYNC_HEADER[CRYPTOS_SYNC_HEADER_LEN]={
  0xFF,
  0xFF,
  0xFF
};

/**
 * @var CRYPTOS_HEADER_LEN
 * @brief Length of a cryptos packet header
 */
static const uint64_t CRYPTOS_HEADER_LEN = CRYPTOS_SYNC_HEADER_LEN+
  CRYPTOS_LENGTH_HEADER_LEN+CRYPTOS_IV_HEADER_LEN+CRYPTOS_EMISSION_HEADER_LEN+
  CRYPTOS_PACKET_HEADER_LEN;

/* Functions */

/** 
 * @fn int cryptos_buffer_init(cryptos_protocol_buffer_t *cb, int fd, size_t size)
 * @brief Initializes cryptographic layer buffer
 * 
 * Initializes the internal variables of the cryptos_config_t structure 
 * needed during the protocol.
 * 
 * @param[in] cb The cryptographic layer buffer structure to initialize
 * @param[in] fd The file descriptor from which the cryptographic layer will
 *  retrieve the data to produce crypto packets
 * @param[in] size The desired size of the buffer, in bytes.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 */
int cryptos_buffer_init(cryptos_protocol_buffer_t *cb, int fd, size_t size);

/** 
 * @fn int cryptos_buffer_free(cryptos_protocol_buffer_t *cb)
 * @brief Frees the resources allocated for the cryptographic layer buffer
 * 
 * @param[in] cb The cryptographic layer buffer structure to free
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 */
int cryptos_buffer_free(cryptos_protocol_buffer_t *cb);

/** 
 * @fn int cryptos_config_init(cryptos_config_t *cc, int cipher_algo, 
 *			byte *key, int keylen, int md_algo, int hmac, 
 *                      byte *iv, int ivlen, uint64_t emission, 
 *                      uint64_t packet, uint64_t default_data_size)
 * @brief Initializes the internal variables of the cryptos_config_t structure 
 *  needed during the protocol.
 * 
 * Initializes the internal variables of the cryptos_config_t structure 
 * needed during the protocol.
 * 
 * @param[in] cc The cryptos_config_t structure to initialize
 * @param[in] cipher_algo The cipher algorithm to use
 * @param[in] key The key to use, must have at least 16 bytes
 * @param[in] keylen Length of key, in bytes
 * @param[in] md_algo The message digest algorithm
 * @param[in] hmac Boolean. When set, indicates the hmac variant of the md_algo
 *        wants to be used (if possible)
 * @param[in] iv Initialization vector. It is not a key, but uses the same
 *        structure. Must have a length of 16 bytes (128 bits). Will be NULL
 *        at the receiver's side.
 * @param[in] ivlen Length of iv, in bytes. Will be 0 at the receiver's side.
 * @param[in] emission Emission id
 * @param[in] packet Packet id initial value. Will be 1 at the receiver's side.
 * @param[in] default_data_size Default data size, in bytes, to send in each packet.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 *
 * @see cryptos_config_t
 */
int cryptos_config_init(cryptos_config_t *cc, int cipher_algo, 
			byte *key, int keylen, int md_algo, int hmac, 
			byte *iv, int ivlen, uint64_t emission, 
			uint64_t packet, uint64_t default_data_size);

/** 
 * @fn int cryptos_config_free(cryptos_config_t *cc)
 * @brief Frees any internal variable of the structure.
 * 
 * @param cc The crytpos config structure to free.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
int cryptos_config_free(cryptos_config_t *cc);
/** 
 * @fn int cryptos_config_set_key(cryptos_config_t *cc, cryptos_key_t *key)
 * @brief Sets the key attribute of cryptos_config_t to the key specified,
 *  freeing the previous iv when necessary
 * 
 * @param cc The crytpos config structure
 * @param key The key
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
int cryptos_config_set_key(cryptos_config_t *cc, cryptos_key_t *key);

/** 
 * @fn int cryptos_config_set_iv(cryptos_config_t *cc, cryptos_key_t *iv)
 * @brief Sets the key attribute of cryptos_config_t to the key specified,
 *  freeing the previous iv when necessary
 * 
 * @param cc The crytpos config structure
 * @param iv The IV
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
int cryptos_config_set_iv(cryptos_config_t *cc, cryptos_key_t *iv);

/** 
 * @fn int produce_packet(cryptos_config_t *cc, byte *data, uint64_t d_len,
 * 		          byte *packet, uint64_t p_len, uint64_t *w_data)
 * @brief Produces a new packet ready to be sent
 * 
 * Prepares a new packet ready to be sent. The data field will be ciphered
 * and inserted in the packet. The header fields and integrity checks will
 * be generated accordingly to the cryptos configuration received, and, upon
 * successful execution, the packet id will be increased for the next packet.
 *
 * A packet will have the structure depicted below:
 *
 *  ------------------------------------------------------------------------
 * | SYNC | DATA_LENGTH | IV | EMISSION_ID | PACKET_ID |.. DATA .. | DIGEST |
 *  ------------------------------------------------------------------------
 *
 * - The field SYNC will be equal to CRYPTOS_SYNC_HEADER and will be of
 *   CRYPTOS_SYNC_HEADER_LEN bytes long.
 * - The field DATA_LENGTH will mark the length of the field DATA and will be of
 *   CRYPTOS_LENGTH_HEADER_LEN bytes long.
 * - The field IV will be of CRYPTOS_IV_LEN bytes
 * - The field EMISSION_ID will be CRYPTOS_EMISSION_LEN bytes long
 * - The field PACKET_ID will be CRYPTOS_PACKET_LEN bytes long
 * - The field DATA will have the size marked in the DATA_LENGTH field and will
 *   be the only field ciphered.
 * - The DIGEST field will have a variable size, depending on the digest 
 *   algorithm in use, and will vary from 24 bits (CRC24) to 64 bytes (SHA512)
 *   although high sizes are not likely to happen. The digest includes the 
 *   whole packet except the SYNC field.
 *
 * @param cc Cryptos config structure
 * @param data The (plain) data to write
 * @param d_len The length of <i>data</i>, in bytes
 * @param packet The byte array in which store the result
 * @param p_len The memory allocated for <i>packet</i>, in bytes.
 * @param w_data The number of data bytes successfuly included in the current
 *        packet. Note that the amount of received plain data can exceed the
 *        amount of data that can be included in the current packet.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 */
int produce_packet(cryptos_config_t *cc, byte *data, uint64_t d_len,
		   byte *packet, uint64_t p_len, uint64_t *w_data);

/** 
 * @fn int parse_packet(cryptos_config_t *cc, byte *packet, uint64_t p_len,
 *		        byte *data, uint64_t *d_len, uint64_t *r_data)
 * @brief Parses the received data, returning the data in it.
 * 
 * Parses the packet <i>packet</i>, of <i>p_len</i> bytes, using the cryptos
 * context defined by <i>cc</i>, verifying it's integrity and that matches with
 * the expected emission and packet IDs. If everything goes OK, the recoverd
 * data will be stored in <i>data</i>, of <i>d_len</i> allocated bytes, which
 * will be reallocated (and <i>d_len</i> updated consequently) if more space is
 * needed. The total amount of pure data recoverd, in bytes, will be stored in
 * <i>r_data</i>.
 * 
 * A packet will have the structure depicted below:
 *
 *  ------------------------------------------------------------------------
 * | SYNC | DATA_LENGTH | IV | EMISSION_ID | PACKET_ID |.. DATA .. | DIGEST |
 *  ------------------------------------------------------------------------
 *
 * - The field SYNC will be equal to CRYPTOS_SYNC_HEADER and will be of
 *   CRYPTOS_SYNC_HEADER_LEN bytes long.
 * - The field DATA_LENGTH will mark the length of the field DATA and will be of
 *   CRYPTOS_LENGTH_HEADER_LEN bytes long.
 * - The field IV will be of CRYPTOS_IV_LEN bytes
 * - The field EMISSION_ID will be CRYPTOS_EMISSION_LEN bytes long
 * - The field PACKET_ID will be CRYPTOS_PACKET_LEN bytes long
 * - The field DATA will have the size marked in the DATA_LENGTH field
 * - The DIGEST field will have a variable size, depending on the digest 
 *   algorithm in use, and will vary from 24 bits (CRC24) to 64 bytes (SHA512)
 *   although high sizes are not likely to happen.
 *
 * @param cc Cryptos context.
 * @param packet Packet to parse
 * @param p_len Allocated size, in bytes, for <i>packet</i>
 * @param data Array to store the recovered data in
 * @param d_len Allocated size, in bytes, for <i>data</i>
 * @param r_data Final successfully recovered amount of data, in bytes.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 */
int parse_packet(cryptos_config_t *cc, byte *packet, uint64_t p_len,
		 byte *data, uint64_t *d_len, uint64_t *r_data);

/** 
 * @fn int cryptos_key_init(byte *byte_key, const int size, cryptos_key_t *key)
 * @brief Allocates memory for the key fields and sets them to the received 
 *  values.
 * 
 * Allocates memory for the key fields and sets them to the received values.
 *
 * @param[in] byte_key The key value.
 * @param[in] size The size of the <i>byte_key</i> argument, in bytes.
 * @param[in, out] key A pointer to a previously allocated cryptos_key_t variable in
 *  which we'll store the key's members.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument). 
 * @retval I_CRYPTOS_ERR with errno = ERANGE (invalid key size).
 */
int cryptos_key_init(byte *byte_key, const int size, cryptos_key_t *key);

/** 
 * @fn int cryptos_key_free(cryptos_key_t *key)
 * @brief Frees the memory allocated for the internal variables of the 
 *  given cryptos_key.
 *
 * @param[in] key A pointer to the key to free.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 */
int cryptos_key_free(cryptos_key_t *key);

/**
 * @fn int cryptos_cipher_algo_code(const char *cipher_algo_name, int *code)
 * @brief Returns the GCRY code for the algorithm with <i>cipher_algo_name</i>
 *  name. If the specified name is NULL, returns the code for ARCFOUR, which
 *  is the default cipher algorithm in this system.

 * @param[in] cipher_algo_name The name of the ciphering algorithm, in char*
 *  representation.
 * @param[out] code Variable to store the corresponding code.
 *
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 */
int cryptos_cipher_algo_code(const char *cipher_algo_name, int *code);

/**
 * @fn int cryptos_md_algo_code(const char *md_algo_name, int *code)
 * @brief Returns the GCRY code for the algorithm with <i>md_algo_name</i>
 *  name. If the specified name is NULL, returns the code for SHA1, which
 *  is the default message digest algorithm in this system.

 * @param[in] md_algo_name The name of the digest algorithm, in char*
 *  representation.
 * @param[out] code Variable to store the corresponding code.
 *
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 */
int cryptos_md_algo_code(const char *md_algo_name, int *code);

#endif

/* cryptos_channel.h ends here */

