/*                               -*- Mode: C -*- 
 * @file: cryptos_types.h
 * @brief: This file defines the common constants, macros and data structures
 *  for the cryptographic functions and interfaces of the software TODO!!.
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * @date: dom ene 10 22:43:31 2010 (+0100)
 * @version: 
 * Last-Updated: jue ago 19 18:25:12 2010 (+0200)
 *           By: Jesus
 *     Update #: 30
 * URL: 
 */

#ifndef CRYPTOS_TYPES_H
#define CRYPTOS_TYPES_H

#include <gcrypt.h>
#include "global_types.h"

/* Constants */

/**
 * @def I_CRYPTOS_OK 
 * @brief Error code for 'No error' to use in functions of integer return type.
 */
#define I_CRYPTOS_OK 0

/**
 * @def I_CRYPTOS_ERR
 * @brief Error code for 'Error occured' to use in functions of integer return type.
 */
#define I_CRYPTOS_ERR 1

/**
 * @def I_CRYPTOS_CHECK_FAIL
 * @brief Error code for 'Integrity check failed'.
 */
#define I_CRYPTOS_CHECK_FAIL 2

/**
 * @struct cryptos_key_t cryptos_types.h "include/cryptos_types.h"
 * @brief Defines the key structure to be used in the cryptographic layer.
 */
typedef struct /* _cryptos_key_t */ {
  byte *key; /**< Key value */
  int length; /**< Key length, in bytes. */
} cryptos_key_t;

/**
 * @struct cryptos_protocol_buffer
 * @brief Buffer used within the cryptos protocol and which also serves as
 *  interface with the steganos protocol.
 */
typedef struct {
  int fd; /**< The file from which we'll read the data to send at the emitter's
	       side, or the file in which we'll write the data at the receiver's
	       side. */
  size_t offset; /**< Offset inside fd, in bytes. */
  byte *buffer; /**< Buffer to store data not successfuly sent or received in
		     previous packets. */
  size_t buffer_size; /**< Allocated size for buffer. */
  size_t buffer_used; /**< Current amount of bytes in buffer. */
} cryptos_protocol_buffer_t;

/**
 * @struct cryptos_config_t cryptos_types.h "include/cryptos_types.h"
 * @brief Defines the global configuration of the cryptographic protocol.
 *
 * @see cryptos_key_t
 */
typedef struct /*_cryptos_config_t*/ {
  int cipher_algo; /**< Cipher algorithm to use */
  int md_algo; /**< Digest algorithm to use */
  int md_len;  /**< Digest length */
  int max_data; /**< Max data to hide. Will depend on the cipher 
		   and digest algorithms */
  int hmac; /**< When active, indicate hmac variant of digest algorithm */
  gcry_cipher_hd_t chd; /**< Handler for the ciphering algorithm */
  gcry_md_hd_t mdhd; /**< Handler for the message digest algorithm */
/*   cryptos_key_t *master_key; /\**< Master key to use. For deriving keys. *\/ */
  cryptos_key_t *key;
  cryptos_key_t *iv; /**< IV to use in the cipher algorithm */
/*   cryptos_key_t *frame_key; /\* Frame specific key *\/ */
  uint64_t emission; /**< Current emission ID  */
  uint64_t packet; /**< Current packet ID */
  uint64_t default_data_size; /**< Default size of data, in bytes, to send in
			         a given packet. */
} cryptos_config_t;

#endif /* CRYPTOS_TYPES_H */

/* cryptos_types.h ends here */
