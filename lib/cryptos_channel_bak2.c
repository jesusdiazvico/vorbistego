/*                               -*- Mode: C -*- 
 * @file: cryptos_channel.c
 * @brief: 
 * @author: Jesus
 * Maintainer: 
 * @date: sáb jul 17 21:16:05 2010 (+0200)
 * @version: 
 * Last-Updated: jue ago 12 23:16:58 2010 (+0200)
 *           By: Jesus
 *     Update #: 382
 * URL: 
 */

#include <stdio.h>
#include <errno.h>

#include "cryptos_channel.h"
#include "miscellaneous.h"

// @todo Is desynchronization necessary?

/** 
 * @fn static int _is_supported_cipher(int algo, int *supported)
 * @brief Tests if the cipher algorithm <i>algo</i> is supported
 * 
 * @param algo The libgcrypt's algorithm ID.
 * @param supported Will store the result of the test.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
static int _is_supported_cipher(int algo, int *supported) {

  /* Input parameter control */
  if(!supported) {
    errno = EINVAL;
    message_log("_is_supported_cipher", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* For now, just ARCFOUR */
  if(algo == GCRY_CIPHER_ARCFOUR) {
    *supported = 1;
  } else {
    *supported = 0;
  }

  return I_CRYPTOS_OK;
  
}

/** 
 * @fn static int _is_supported_md(int algo, int hmac, int *supported)
 * @brief Tests if the digest algorithm <i>algo</i> is supported with
 *  hmac variant if hmac is set.
 * 
 * @param algo The libgcrypt's algorithm ID.
 * @param hmac Indicates algo with hmac variant.
 * @param supported Will store the result of the test.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
static int _is_supported_md(int algo, int hmac, int *supported) {

  /* Input parameters control */
  if(!supported) {
    errno = EINVAL;
    message_log("_is_supported_md", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Any message digest algorithm implemented in libgcrypt is supported */
  // @todo Qué pasa con MD_NONE y MD2 ? (no algo y reservado para MD2 pero
  // no implementado)
  if(!(gcry_md_test_algo(algo))) {
    *supported = 1;
  } else {
    *supported = 0;
  }

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _cipher(cryptos_config_t *cc, byte *data, int d_len,
 *		          byte *data_out, int do_len)
 * @brief Ciphers the block <i>data</i> using the key and algorithm spececified
 *  and stores the result in data_out.
 *
 * Ciphers the block <i>data</i> of len <i>d_len</i> bytes, using the context
 * defined by cryptos_config_t <i>cc</i>, and stores the result in 
 * <i>data_out</i>, which must have been allocated previously and has 
 * <i>do_len</i> bytes of length.
 *
 * @param cc The cryptographic configuration.
 * @param data The data to cipher.
 * @param d_len The length of <i>data</i> in bytes.
 * @param data_out The allocated data block that will store the result.
 * @param do_len The reserved size (in bytes) for <i>data_out</i>
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 */
static int _cipher(cryptos_config_t *cc, byte *data, int d_len, 
		   byte *data_out, int do_len) {

  gcry_error_t gce;
  byte *buffer;
  int buffer_len;

  /* Input parameters control */
  if(!cc || !data || d_len <= 0 || !data_out || do_len <= 0 ||
     do_len < d_len) {
    errno = EINVAL;
    message_log("_cipher", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Open the handler */
  gce = gcry_cipher_open(&cc->chd, cipher_algo, GCRY_CIPHER_MODE_STREAM,
			 GCRY_CIPHER_SECURE);
  if(gce != GPG_ERR_NO_ERROR) {
    message_log("cryptos_config_init", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  /* Prepare the keys and IVs using the packet, emission and IV fields from cc */
  buffer_len = sizeof(cc->emission)+sizeof(cc->packet);
  fprintf(stdout, "sizeof(cc->emission)+sizeof(cc->packet) = %d (must be at least 128 bits!)\n", 
	  buffer_len);
  if(!(buffer = (byte *) malloc(sizeof(byte)*buffer_len))) {
    free(buffer);
    message_log("_cipher", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Initialize the cipher algorithm to obtain the current packet key */
  gce = gcry_cipher_setkey(cc->chd, cc->key->key, cc->key->length);
  if(gce != GPG_ERR_NO_ERROR) {
    free(buffer);
    message_log("_cipher", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  gce = gcry_cipher_setiv(cc->chd, cc->iv->key, cc->iv->length);
  if(gce != GPG_ERR_NO_ERROR) {
    free(buffer);
    message_log("_cipher", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  /* Create a message composed by emission.packet (where . means concatenation)
     and cipher it. The result, of 128 bits, will be the current packet key. */
  memset(buffer, 0, buffer_len*sizeof(byte));
  memcpy(buffer, &cc->emission, sizeof(cc->emission));
  memcpy(&buffer[sizeof(cc->emission)], &cc->packet, sizeof(cc->packet));

  gce = gcry_cipher_encrypt(cc->chd, buffer, 
			    sizeof(cc->emission)+sizeof(cc->packet), NULL, 0);
  if(gce != GPG_ERR_NO_ERROR) {
    free(buffer);
    message_log("_cipher", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  /* The same key will be used to cipher and hmac (if specified) */
  fprintf(stdout, "Misma clave para cifrar y hmac... está bien?\n");
  gce = gcry_cipher_setkey(cc->chd, buffer, buffer_len);
  if(gce != GPG_ERR_NO_ERROR) {
    message_log("_cipher", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  /* Cipher the data and go */
  gce = gcry_cipher_encrypt(cc->chd, data_out, do_len, data, d_len);
  if(gce != GPG_ERR_NO_ERROR) {
    message_log("_cipher", gcry_strerror(gce));
    free(buffer);
    return I_CRYPTOS_ERR;
  }

  /* Close the handler */
  free(buffer);
  gcry_cipher_close(cc->chd); 

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _decipher(cryptos_config_t *cc, byte *data, int d_len,
 *		          byte *data_out, int do_len)
 * @brief Deciphers the block <i>data</i> using the key and algorithm spececified
 *  and stores the result in data_out.
 *
 * Deciphers the block <i>data</i> of len <i>d_len</i> bytes, using the context
 * defined by cryptos_config_t <i>cc</i>, and stores the result in 
 * <i>data_out</i>, which must have been allocated previously and has 
 * <i>do_len</i> bytes of length.
 *
 * @param cc The cryptographic configuration.
 * @param data The data to decipher.
 * @param d_len The length of <i>data</i> in bytes.
 * @param data_out The allocated data block that will store the result.
 * @param do_len The reserved size (in bytes) for <i>data_out</i>
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 *
 */
static int _decipher(cryptos_config_t *cc, byte *data, int d_len,
		     byte *data_out, int do_len) {

  gcry_error_t gce;

  /* Input parameters control */
  if(!cc || !data || d_len <= 0 || !data_out || do_len <= 0 ||
     do_len < d_len) {
    errno = EINVAL;
    message_log("_decipher", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Prepare the data to be ciphered and go */
  gce = gcry_cipher_decrypt(cc->chd, data_out, do_len, data, d_len);
  if(gce != GPG_ERR_NO_ERROR) {
    message_log("_decipher", gcry_strerror(gce));
    return I_CRYPTOS_ERR;
  }

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _digest(cryptos_config_t *cc, byte *data, int d_len, 
 *		   byte *data_out, int do_len)
 * @brief Calculates the digest of data and stores it in data_out
 *
 * @param cc Cryptos config. Contains the md handler
 * @param data Data to hash
 * @param d_len Length of <i>data</i> in bytes
 * @param data_out Array to store the hash in
 * @param do_len Allocated length for <i>data_out</i> in bytes
 *
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 * 
 */
static int _digest(cryptos_config_t *cc, byte *data, int d_len, 
		   byte *data_out, int do_len) {

  /* Input parameters control */
  if(!cc || !data || d_len <= 0 || !data_out || do_len <= 0 ||
     do_len < cc->md_len) {
    errno = EINVAL;
    message_log("_digest", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  gcry_md_write(cc->mdhd, data, d_len);
  memcpy(data_out, gcry_md_read(cc->mdhd, 0),cc->md_len);

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _check_integrity(cryptos_config_t *cc, byte *data, int d_len, 
 *		    	           byte *digest, int dig_len, int *equal)
 * @brief Tests if the digest of <i>data</i> equals <i>digest</i>
 *
 * Calculates the digest (using the algorithm specified in <i>cc</i>) of
 * <i>data</i>, of <i>d_len</i> allocated bytes, and compares it with 
 * <i>digest</i>, of <i>dig_len</i> allocated bytes. If they are equal, the
 * variable <i>equal</i> will be set to 1, if not, to 0.
 *
 * @param cc Cryptos config structure
 * @param data Data to hash and compare
 * @param d_len Allocated size, in bytes, of <i>data</i>
 * @param digest Digest to compare to
 * @param dig_len Allocated size, in bytes, of <i>digest</i>
 * @param equal Boolean. Will store the result of the comparision. Being
 *  1 if hash(data) = digest and 0 otherwise
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 * @retval I_CRYPTOS_ERR with errno != 0 && errno != EINVAL (see corresponding
 *         error code)
 */
static int _check_integrity(cryptos_config_t *cc, byte *data, int d_len, 
			    byte *digest, int dig_len, int *equal) {

  byte *data_dig;
  int rc;

  /* Input parameters control */
  if(!cc || !data || d_len <= 0 || !digest || dig_len <= 0 || !equal) {
    errno = EINVAL;
    message_log("_check_integrity", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  *equal = 0;
  
  if(!(data_dig = (byte *) malloc(sizeof(byte)*cc->md_len))) {
    message_log("_check_integrity", strerror(errno));
    return I_CRYPTOS_ERR;
  }
  memset(data_dig, 0, sizeof(byte)*cc->md_len);

  /* Calculate the digest of data */
  if(_digest(cc, data, d_len, data_dig, cc->md_len) == I_CRYPTOS_ERR) {
    free(data_dig);
    return I_CRYPTOS_ERR;
  }
  
  /* Compare */
  rc = memcmp(data, data_dig, cc->md_len);

  /* Update returning variables */
  if(rc) *equal = 1;
  else *equal = 0;
  
  free(data_dig);
  
  return I_CRYPTOS_OK;
}

/** 
 * @fn static int _write_packet_uint32_field(byte *packet, int packet_size, 
 *                           int offset, uint32_t *in)
 * @brief Writes <i>in<i> in the packet <i>packet</i> at the position given by 
 *        <i>offset</i>.
 *
 * Writes the unsigned int of 32 bits in <i>packet</i>, of <i>packet_size</i> 
 * bytes, starting at offset <i>offset</i> bytes. Uses big endian byte ordering,
 * i.e., the Most Significant Byte is written at a lowest memory position.
 * 
 * @param packet The packet to write in.
 * @param packet_size The allocated size for packet.
 * @param offset The offset to start writing at
 * @param in The variable to write.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
static int _write_packet_uint32_field(byte *packet, int packet_size, int offset, 
				      uint32_t in) {

  int i, field_bits, bit, byte_off, bit_off;

  /* Input parameters control */
  if(!packet || packet_size <= 0 || offset <= 0) {
    errno = EINVAL;
    message_log("_write_packet_field", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  field_bits = sizeof(uint32_t)*BITS_PER_BYTE;
  for(i=field_bits-1; i>=0; i--) {

    bit_off = i % BITS_PER_BYTE;
    byte_off = i / BITS_PER_BYTE;

    bit = in & (1 << i);
    if(bit) bit = 1;
    else bit = 0;
    
    packet[offset+(sizeof(uint32_t)-byte_off)-1] |= (bit << bit_off);

  }

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _write_packet_uint64_field(byte *packet, int packet_size, 
 *                           int offset, uint64_t *in)
 * @brief Writes <i>in<i> in the packet <i>packet</i> at the position given by 
 *        <i>offset</i>.
 *
 * Writes the unsigned int of 64 bits in <i>packet</i>, of <i>packet_size</i> 
 * bytes, starting at offset <i>offset</i> bytes. Uses big endian byte ordering,
 * i.e., the Most Significant Byte is written at a lowest memory position.
 * 
 * @param packet The packet to write in.
 * @param packet_size The allocated size for packet.
 * @param offset The offset to start writing at
 * @param in The variable to write.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
static int _write_packet_uint64_field(byte *packet, int packet_size, int offset, 
				      uint64_t in) {

  int i, field_bits, bit, byte_off, bit_off;

  /* Input parameters control */
  if(!packet || packet_size <= 0 || offset <= 0) {
    errno = EINVAL;
    message_log("_write_packet_field", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  field_bits = sizeof(uint64_t)*BITS_PER_BYTE;
  for(i=field_bits-1; i>=0; i--) {

    bit_off = i % BITS_PER_BYTE;
    byte_off = i / BITS_PER_BYTE;

    bit = in & (1 << i);
    if(bit) bit = 1;
    else bit = 0;
    
    packet[offset+(sizeof(uint64_t)-byte_off)-1] |= (bit << bit_off);

  }

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _read_packet_uint32_field(byte *packet, int packet_size, 
 *                                          int offset, uint32_t *field)
 * @brief Reads the field at offset <i>offset</i> bytes of <i>packet</i>, of size
 *  32 bits and stores it in <i>field</i>
 * 
 * @param packet The packet to read de field from
 * @param packet_size The size of <i>packet</i> in bytes
 * @param offset The offset at which the field starts, in bytes
 * @param field The variable to store the field in
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
static int _read_packet_uint32_field(byte *packet, int packet_size, int offset, 
				     uint32_t *field) {

  int i, field_bits, byte_off, bit_off, bit;
  uint32_t tmp;

  /* Input parameters control */
  if(!packet || packet_size <= 0 || offset <= 0 || !field) {
    errno = EINVAL;
    message_log("_read_packet_uint32_field", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  tmp = 0;
  field_bits = sizeof(uint32_t)*BITS_PER_BYTE;
  for(i=0; i<field_bits; i++) {
    
    /* Get the next bit and byte to read from packet */
    bit_off = i % BITS_PER_BYTE;
    byte_off = i / BITS_PER_BYTE;

    bit = packet[offset+byte_off] & (1 << bit_off);
    if(bit) bit = 1;
    else bit = 0;

    /* Update the given element */
    tmp |= (bit << (bit_off + byte_off*BITS_PER_BYTE));

  }

  *field = tmp;

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _read_packet_uint64_field(byte *packet, int packet_size, 
 *                                          int offset, uint64_t *field)
 * @brief Reads the field at offset <i>offset</i> bytes of <i>packet</i>, of size
 *  64 bits and stores it in <i>field</i>
 * 
 * @param packet The packet to read de field from
 * @param packet_size The size of <i>packet</i> in bytes
 * @param offset The offset at which the field starts, in bytes
 * @param field The variable to store the field in
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
static int _read_packet_uint64_field(byte *packet, int packet_size, int offset, 
				     uint64_t *field) {

  int i, field_bits, byte_off, bit_off, bit;
  uint64_t tmp;

  /* Input parameters control */
  if(!packet || packet_size <= 0 || offset <= 0 || !field) {
    errno = EINVAL;
    message_log("_read_packet_uint64_field", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  tmp = 0;
  field_bits = sizeof(uint64_t)*BITS_PER_BYTE;
  for(i=0; i<field_bits; i++) {
    
    /* Get the next bit and byte to read from packet */
    bit_off = i % BITS_PER_BYTE;
    byte_off = i / BITS_PER_BYTE;

    bit = packet[offset+byte_off] & (1 << bit_off);
    if(bit) bit = 1;
    else bit = 0;

    /* Update the given element */
    tmp |= (bit << (bit_off + byte_off*BITS_PER_BYTE));

  }

  *field = tmp;

  return I_CRYPTOS_OK;

}

/** 
 * @fn static int _prepare_packet_keys(cryptos_config_t *cc)
 * @brief Manipulates the keys to be ready to be used in a new packet
 *
 * The key used to cipher/decipher/hash is obtained following this flow:
 *  1) Initialize the algorithm with the IV in <i>cc->iv</i> and the key
 *     in <i>cc->key</i>.
 *  2) Cipher the bytestream composed by concatenating <i>cc->emission</i> and 
 *     <i>cc->packet</i>.
 *  3) Set the resulting ciphertext as the current packet key.
 * 
 * @param cc Cryptos config structure
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_CRYPTOS_OK if no error was present and I_CRYPTOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_CRYPTOS_OK with errno = 0 (No error).
 * @retval I_CRYPTOS_ERR with errno = EINVAL (invalid argument).
 */
/* static int _prepare_packet_keys(cryptos_config_t *cc) { */

/*   gcry_error_t gce; */
/*   byte *buffer; */
/*   int buffer_len; */

/*   /\* Input parameters control *\/ */
/*   if(!cc) { */
/*     errno = EINVAL; */
/*     message_log("_prepare_packet_key", strerror(errno)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

/*   buffer_len = sizeof(cc->emission)+sizeof(cc->packet); */
/*   fprintf(stdout, "sizeof(cc->emission)+sizeof(cc->packet) = %d (must be at least 128 bits!)\n",  */
/* 	  buffer_len); */
/*   if(!(buffer = (byte *) malloc(sizeof(byte)*buffer_len))) { */
/*     free(buffer); */
/*     message_log("_prepare_packet_key", strerror(errno)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

/*   /\* Initialize the cipher algorithm to obtain the current packet key *\/ */
/*   gce = gcry_cipher_setkey(cc->chd, cc->key->key, cc->key->length); */
/*   if(gce != GPG_ERR_NO_ERROR) { */
/*     free(buffer); */
/*     message_log("_prepare_packet_key", gcry_strerror(gce)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

/*   gce = gcry_cipher_setiv(cc->chd, cc->iv->key, cc->iv->length); */
/*   if(gce != GPG_ERR_NO_ERROR) { */
/*     free(buffer); */
/*     message_log("_cipher", gcry_strerror(gce)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

/*   /\* Create a message composed by emission.packet (where . means concatenation) */
/*      and cipher it. The result, of 128 bits, will be the current packet key. *\/ */
/*   memset(buffer, 0, buffer_len*sizeof(byte)); */
/*   memcpy(buffer, &cc->emission, sizeof(cc->emission)); */
/*   memcpy(&buffer[sizeof(cc->emission)], &cc->packet, sizeof(cc->packet)); */

/*   gce = gcry_cipher_encrypt(cc->chd, buffer,  */
/* 			    sizeof(cc->emission)+sizeof(cc->packet), NULL, 0); */
/*   if(gce != GPG_ERR_NO_ERROR) { */
/*     free(buffer); */
/*     message_log("_prepare_packet_key", gcry_strerror(gce)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

/*   /\* The same key will be used to cipher and hmac (if specified) *\/ */
/*   fprintf(stdout, "Misma clave para cifrar y hmac... está bien?\n"); */
/*   gce = gcry_cipher_setkey(cc->chd, buffer, buffer_len); */
/*   if(gce != GPG_ERR_NO_ERROR) { */
/*     message_log("_prepare_packet_key", gcry_strerror(gce)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

/*   if(cc->hmac) { */
/*     gce = gcry_md_setkey(cc->mdhd, buffer, buffer_len); */
/*     if(gce != GPG_ERR_NO_ERROR) { */
/*       message_log("_prepare_packet_key", gcry_strerror(gce)); */
/*       return I_CRYPTOS_ERR; */
/*     }       */
/*   } */

/*   return I_CRYPTOS_OK; */

/* } */

int cryptos_buffer_init(cryptos_protocol_buffer_t *cb, int fd, size_t size) {

  /* Input parameter control */
  if( !cb || fd < 0 || size <= 0) {
    errno = EINVAL;
    message_log("cryptos_init_buffer", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Allocate the buffer */
  if(!(cb->buffer = (byte *) malloc(sizeof(byte)*size))) {
    message_log("cryptos_init_buffer", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  memset(cb->buffer, 0, sizeof(byte)*size);
  cb->buffer_size = size;  
  cb->buffer_used = 0;
  cb->offset = 0;
  cb->fd = fd;
  return I_CRYPTOS_OK;

}


int cryptos_buffer_free(cryptos_protocol_buffer_t *clb) {

  if(!clb) {
    return I_CRYPTOS_OK;
  }

  if(clb->buffer) {
    free(clb->buffer);
  }

  return I_CRYPTOS_OK;

}

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
 * @param cc The cryptos_config_t structure to initialize
 * @param cipher_algo The cipher algorithm to use
 * @param key The key to use, must have at least 16 bytes
 * @param keylen Length of key, in bytes
 * @param md_algo The message digest algorithm
 * @param hmac Boolean. When set, indicates the hmac variant of the md_algo
 *        wants to be used (if possible)
 * @param iv Initialization vector. It is not a key, but uses the same
 *        structure. Must have a length of 16 bytes (128 bits). Will be NULL
 *        at the receiver's side.
 * @param ivlen Length of iv, in bytes. Will be 0 at the receiver's side.
 * @param emission Emission id
 * @param packet Packet id initial value. Will be 1 at the receiver's side.
 * @param default_data_size Default data size, in bytes, to send in each packet.
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
			uint64_t packet, uint64_t default_data_size) {

  gcry_error_t gce;
  int supported, i;
  unsigned int md_len, max_data, md_flags;
  byte def_iv[CRYPTOS_IV_HEADER_LEN];

  /* Input parameter control */
  if(!cc || !key || keylen < 16 || !cipher_algo || !md_algo) {
    errno = EINVAL;
    message_log("cryptos_config_init", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Test the cipher algorithm */
  if(_is_supported_cipher(cipher_algo, &supported) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  if(!supported) {
    errno = EINVAL;
    message_log("cryptos_config_init", 
		"The given cipher algorithm is not supported");
    return I_CRYPTOS_ERR;
  }

  /* Test the digest algorithm */
  if(_is_supported_md(md_algo, hmac, &supported) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  if(!supported) {
    errno = EINVAL;
    message_log("cryptos_config_init", 
		"The given message digest algorithm is not supported");
    return I_CRYPTOS_ERR;
  }

/*   /\* Obtains the libgcrypt's cipher handler *\/ */
/*   /\* For now, just stream mode with secure environment *\/ */
/*   gce = gcry_cipher_open(&cc->chd, cipher_algo, GCRY_CIPHER_MODE_STREAM,  */
/* 			 GCRY_CIPHER_SECURE); */
/*   if(gce != GPG_ERR_NO_ERROR) { */
/*     message_log("cryptos_config_init", gcry_strerror(gce)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

  /* Obtains the libgcrypt's digest handler */
  md_flags = 0;
  if(hmac) {
    md_flags = GCRY_MD_FLAG_HMAC;
  }

/*   gce = gcry_md_open(&cc->mdhd, md_algo, GCRY_MD_FLAG_SECURE | md_flags); */
/*   if(gce != GPG_ERR_NO_ERROR) { */
/*     message_log("cryptos_config_init", gcry_strerror(gce)); */
/*     return I_CRYPTOS_ERR; */
/*   } */

  /* Depending on the digest algorithm, the packets will have one size or another
     As we permit several digest algorithms, each one producing digests of 
     different sizes, the idea is to get always the same proportion of bytes of 
     data per byte of digest. */
  md_len = gcry_md_get_algo_dlen(md_algo);
  max_data = md_len*RATIO_DD;

  /* The maximum data to insert in a given packet will be, then, the minimum of
     max_data and the 2^number_of_bits_in_length_field-1 */
  
  cc->max_data = max_data;
  cc->cipher_algo = cipher_algo;
  cc->md_algo = md_algo;
  cc->md_len = md_len;
  cc->hmac = hmac;

  if(!(cc->key = (cryptos_key_t *) malloc(sizeof(cryptos_key_t)))) {
    message_log("cryptos_config_init", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  if(cryptos_key_init(key, keylen, cc->key) == I_CRYPTOS_ERR) {
    free(cc->key);
    return I_CRYPTOS_ERR;
  }

  if(!(cc->iv = (cryptos_key_t *) malloc(sizeof(cryptos_key_t)))) {
    message_log("cryptos_config_init", strerror(errno));
    free(cc->key);
    return I_CRYPTOS_ERR;
  }

  if(!iv) {
    for(i=0; i<CRYPTOS_IV_HEADER_LEN; i++) {
      def_iv[i] = CRYPTOS_DEFAULT_IV[i];
    }
    if(cryptos_key_init(def_iv, CRYPTOS_IV_HEADER_LEN, cc->iv) 
       == I_CRYPTOS_ERR) {
      free(cc->key);
      free(cc->iv);
      return I_CRYPTOS_ERR;
    }
  } else {
    if(cryptos_key_init(iv, ivlen, cc->iv) == I_CRYPTOS_ERR) {
      free(cc->key);
      free(cc->iv);
      return I_CRYPTOS_ERR;
    }
  }

  cc->emission = emission;
  cc->packet = packet;
  if(!default_data_size) {
    cc->default_data_size = CRYPTOS_DEFAULT_DATA_SIZE;
  } else {
    cc->default_data_size = default_data_size;
  }

  if(cc->default_data_size > max_data) {
    cc->default_data_size = max_data;
  }
  
  return I_CRYPTOS_OK;

}

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
int cryptos_config_free(cryptos_config_t *cc) {

  if(!cc) {
    return I_CRYPTOS_OK;
  }

/*   /\* Close the libgcrypt's cipher handler *\/ */
/*   gcry_cipher_close(cc->chd); */

/*   /\* Close the libgcrypt's digest handler *\/ */
/*   gcry_md_close(cc->mdhd); */

  cryptos_key_free(cc->key);
  cryptos_key_free(cc->iv);

  return I_CRYPTOS_OK;

}

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
int cryptos_config_set_key(cryptos_config_t *cc, cryptos_key_t *key) {

  /* Input parameters control */
  if(!cc || !key) {
    errno = EINVAL;
    message_log("cryptos_config_set_key", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  if(cc->key) {
    cryptos_key_free(cc->key);
  }

  cc->key = key;

  return I_CRYPTOS_OK;

}

/** 
 * @fn int cryptos_config_set_iv(cryptos_config_t *cc, cryptos_key_t *iv)
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
int cryptos_config_set_iv(cryptos_config_t *cc, cryptos_key_t *iv) {

  /* Input parameters control */
  if(!cc || !iv) {
    errno = EINVAL;
    message_log("cryptos_config_set_iv", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  if(cc->iv) {
    cryptos_key_free(cc->iv);
  }

  cc->iv = iv;

  return I_CRYPTOS_OK;

}

/** 
 * @fn int produce_packet(cryptos_config_t *cc, byte *data, uint64_t d_len,
 * 		          byte *packet, uint64_t *p_len, uint64_t *w_data)
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
 * - The field DATA will have the size marked in the DATA_LENGTH field
 * - The DIGEST field will have a variable size, depending on the digest 
 *   algorithm in use, and will vary from 24 bits (CRC24) to 64 bytes (SHA512)
 *   although high sizes are not likely to happen.
 *
 * @param cc Cryptos config structure
 * @param data The (plain) data to write
 * @param d_len The length of <i>data</i>, in bytes
 * @param packet The byte array in which store the result
 * @param p_len The memory allocated for <i>packet</i>, in bytes. In case it
 *        is not enough, the needed memory will be allocated and the number of
 *        resulting bytes returned in <i>p_len</i>.
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
		   byte *packet, uint64_t *p_len, uint64_t *w_data) {

  byte *buffer, *packet_out;
  uint64_t packet_len, i, index;
  uint32_t write;

  /* Input parameter control */
  if(!cc || !data || d_len <= 0 || !packet || !p_len || *p_len <= 0 || !w_data) {
    errno = EINVAL;
    message_log("produce_packet", strerror(errno));
    return I_CRYPTOS_ERR;
  } 

  /* Calculate the total packet length, in bytes */

  /* Get the data we'll write */
  write = d_len;
  if((unsigned int) d_len > CRYPTOS_MAX_DATA) {
    write = CRYPTOS_MAX_DATA;
  }

  if(write > (unsigned int) cc->max_data) {
    write = cc->max_data;
  }

  packet_len = CRYPTOS_SYNC_HEADER_LEN + CRYPTOS_LENGTH_HEADER_LEN + cc->iv->length +
    sizeof(cc->emission) + sizeof(cc->packet) + write + cc->md_len;

  if(!(packet_out = (byte *) malloc(sizeof(byte)*packet_len))) {
    message_log("produce_packet", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  memset(packet_out, 0, packet_len); 

  index = 0;

  /* Insert the synchro header */
  for(i=0; i<CRYPTOS_SYNC_HEADER_LEN; i++) {
    packet_out[i] = CRYPTOS_SYNC_HEADER[i];
    index++;
  }

  /* Insert length header, of CRYPTOS_LENGTH_HEADER_LEN bytes */
  if(_write_packet_uint32_field(packet_out, packet_len, index, 
				write) == I_CRYPTOS_ERR) {
    free(packet_out);
    return I_CRYPTOS_ERR;
  }
  index += CRYPTOS_LENGTH_HEADER_LEN;

  /* Insert IV */
  memcpy(&packet_out[index], cc->iv->key, cc->iv->length);
  index += cc->iv->length;

  /* Insert the emission id */
  if(_write_packet_uint64_field(packet_out, packet_len, index, 
				cc->emission) == I_CRYPTOS_ERR) {
    free(packet_out);
    return I_CRYPTOS_ERR;
  }
  index += CRYPTOS_EMISSION_HEADER_LEN;

  /* Insert the packet id */
  if(_write_packet_uint64_field(packet_out, packet_len, index, 
				cc->packet) == I_CRYPTOS_ERR) {
    free(packet_out);
    return I_CRYPTOS_ERR;
  }
  index += CRYPTOS_PACKET_HEADER_LEN;

/*   /\* Before ciphering or hashing, prepare the keys *\/ */
/*   if(_prepare_packet_keys(cc) == I_CRYPTOS_ERR) { */
/*     free(packet_out); */
/*     return I_CRYPTOS_ERR; */
/*   } */

  if(!(buffer = (byte *) malloc(sizeof(byte)*write))) {
    message_log("produce_packet", strerror(errno));
    free(packet_out);
    return I_CRYPTOS_ERR;
  }

  memset(buffer, 0, write*sizeof(byte));

  /* Cipher and insert the data */  
  if(_cipher(cc, data, write, buffer, write) == I_CRYPTOS_ERR) {
    free(packet_out);
    return I_CRYPTOS_ERR;
  }

  memcpy(&packet_out[index], buffer, write);
  index += write;
  free(buffer);

  /* Calculate and insert the integrity check */
  if(!(buffer = (byte *) malloc(sizeof(byte)*cc->md_len))) {
    message_log("produce_packet", strerror(errno));
    free(packet_out);
    return I_CRYPTOS_ERR;
  }
  memset(buffer, 0, sizeof(byte)*cc->md_len);

  /* Exclude the synchro header. After all, if the synchro header is changed,
     this packet won't be read */
  if(_digest(cc, &data[CRYPTOS_SYNC_HEADER_LEN], write-CRYPTOS_SYNC_HEADER_LEN,
	     buffer, cc->md_len) == I_CRYPTOS_ERR) {
    free(buffer);
    free(packet_out);
    return I_CRYPTOS_ERR;
  }

  memcpy(&packet_out[index], buffer, cc->md_len);
  free(buffer);
  index += cc->md_len;

  if((unsigned int) *p_len < packet_len) {
    if(!(packet = (byte *) realloc(packet, sizeof(byte)*packet_len))) {
      message_log("produce_packet", strerror(errno));
      return I_CRYPTOS_ERR;
    }
    *p_len = packet_len;
  }
  
  memcpy(packet, packet_out, packet_len*sizeof(byte));
  free(packet_out);

  /* Increase the packet id for the next packet */
  cc->packet++;
  
  /* Set the written data */
  *w_data = write;

  /* DEBUG! */
  print_bytestream(stdout, packet, packet_len);
  
  return I_CRYPTOS_OK;
  
}


/** 
 * @fn int parse_packet(cryptos_config_t *cc, byte *packet, int p_len,
 *		        byte *data, int *d_len, int *r_data)
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
 * @param r_data Finally successful recovered amount of data, in bytes.
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
		 byte *data, uint64_t *d_len, uint64_t *r_data) {

  byte buffer[CRYPTOS_MAX_DATA], iv[CRYPTOS_IV_HEADER_LEN];
  byte data_read[CRYPTOS_MAX_DATA];
  cryptos_key_t *iv_key;
  uint32_t data_length;
  uint64_t emission_id, packet_id;
  size_t to_hash_size;
  int i, offset, equal, header_len;

  /* Input parameters control */
  if(!cc || !packet || p_len <= 0 || !data || !d_len || *d_len <= 0 || !r_data) {
    errno = EINVAL;
    message_log("parse_packet", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* If this function is called with p_len <= CRYPTOS_HEADER_LEN+
     CRYPTOS_MIN_DIGEST_LEN, then there is no data to retrieve, as that is the
     minimum size a packet with data can have. This is not an error, though. */
  if(p_len <= CRYPTOS_HEADER_LEN+CRYPTOS_MIN_DIGEST_LEN) {
    *r_data = 0;
    return I_CRYPTOS_OK;
  }

  /* Get the SYNC field, if it does not match with CRYPTOS_SYNC_HEADER, this is
     not a real packet or something is missing. Either way, we'll have to discard
     this bytes. */
  memset(buffer, 0, CRYPTOS_MAX_DATA);
  memcpy(buffer, packet, CRYPTOS_SYNC_HEADER_LEN);

  for(i=0; i<CRYPTOS_SYNC_HEADER_LEN; i++) {
    if(buffer[i] != CRYPTOS_SYNC_HEADER[i]) {
      errno = EBADMSG;
      message_log("parse_packet", "Wrong SYNC field");
      *r_data = ++i;
      return I_CRYPTOS_ERR;
    }      
  }
  
  offset = CRYPTOS_SYNC_HEADER_LEN;
  memset(buffer, 0, CRYPTOS_SYNC_HEADER_LEN);

  /* Get the length of the data */
  if(_read_packet_uint32_field(packet, p_len, offset, 
			       &data_length) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  offset += CRYPTOS_LENGTH_HEADER_LEN;

  /* Knowing the amount of data to expect, we can test now if the buffer received
     has size enough to shelter a packet of that size (we could receive 
     incomplete packets here, thinking they were complete). */
  if(p_len < CRYPTOS_LENGTH_HEADER_LEN + data_length + cc->md_len) {
    *r_data = 0;
    return I_CRYPTOS_OK;
  }
  
  /* Get the IV */
  memcpy(iv, &packet[offset], CRYPTOS_IV_HEADER_LEN); 
 
  if(!(iv_key = (cryptos_key_t *) malloc(sizeof(cryptos_key_t)))) {
    message_log("parse_packet", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Create the key structure */
  if(cryptos_key_init(iv, CRYPTOS_IV_HEADER_LEN, iv_key) == I_CRYPTOS_ERR) {
    message_log("parse_packet", strerror(errno));
    return I_CRYPTOS_ERR;    
  }

  if(cryptos_config_set_iv(cc, iv_key) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  offset += CRYPTOS_IV_HEADER_LEN;

  /* Read the emission ID */
  if(_read_packet_uint64_field(packet, p_len, offset, 
			       &emission_id) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  offset += CRYPTOS_EMISSION_HEADER_LEN;

  /* An expected emission_id of 0 means every emission_id is valid,
     otherwise, we must receive the given emission id */
  if(emission_id != cc->emission && cc->emission) {
    errno = EBADMSG;
    message_log("parse_packet", "Wrong emission ID field");
    return I_CRYPTOS_ERR;
  }

  offset += CRYPTOS_EMISSION_HEADER_LEN;

  /* Read the packet ID */
  if(_read_packet_uint64_field(packet, p_len, offset, 
			       &packet_id) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  offset += CRYPTOS_PACKET_HEADER_LEN;
  
  if((packet_id != cc->packet) && packet_id != 0) {
    errno = EBADMSG;
    message_log("parse_packet", "Wrong packet ID field");
    return I_CRYPTOS_ERR;
  }

  /* If we receive a packet id of 0, that means we've reached the end of the
     communication */
  if(packet_id == 0) {
    cc->packet = 0;
  }
  
  offset += CRYPTOS_PACKET_HEADER_LEN;
  
  /* Read the data */
  memcpy(data_read, &packet[offset], data_length);
  offset += data_length;
  memset(buffer, 0, data_length);

/*   /\* Prepare the keys *\/ */
/*   if(_prepare_packet_keys(cc) == I_CRYPTOS_ERR) { */
/*     return I_CRYPTOS_ERR; */
/*   } */

  /* Run the integrity check, comparing with the received digest.
     Remember that the digest covers the whole packet except the SYNC header */
  header_len = CRYPTOS_LENGTH_HEADER_LEN + CRYPTOS_IV_HEADER_LEN + 
    CRYPTOS_EMISSION_HEADER_LEN + CRYPTOS_PACKET_HEADER_LEN;
  to_hash_size = header_len + data_length;
  if(_check_integrity(cc, &packet[CRYPTOS_SYNC_HEADER_LEN], to_hash_size, 
		      &packet[header_len+data_length+CRYPTOS_SYNC_HEADER_LEN], 
		      cc->md_len, &equal) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }
  
  /* Different digests */
  if(!equal) {
    return I_CRYPTOS_ERR;
  }

  /* If everything went OK, decipher the data and return */
  if((unsigned int) *d_len < data_length) {
    if(!(data = (byte *) realloc(data, sizeof(byte)*data_length))) {
      message_log("parse_packet", strerror(errno));
      return I_CRYPTOS_ERR;
    }
    *d_len = data_length;
  }

  if(_decipher(cc, data_read, data_length, data, *d_len) == I_CRYPTOS_ERR) {
    return I_CRYPTOS_ERR;
  }

  *r_data = data_length;  

  return I_CRYPTOS_OK;

}

/** 
 * @fn int cryptos_key_init(byte *byte_key, const int size, cryptos_key_t *key)
 * @brief Allocates memory for the key fields and sets them to the received 
 *  values.
 * 
 * Allocates memory for the key fields and sets them to the received values.
 *
 * @param[in] byte_key The key value.
 * @param[in] key_len The length of the <i>byte_key</i> argument, in bytes.
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
int cryptos_key_init(byte *byte_key, const int size, cryptos_key_t *key) {

  /* Input parameters control */
  if(!byte_key || size <= 0 || !key) {
    errno = EINVAL;
    message_log("cryptos_key_init", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  if(!(key->key = (byte *) malloc(sizeof(byte)*size))) {
    message_log("cryptos_key_init", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  memcpy(key->key, byte_key, size);
  key->length = size;

  return I_CRYPTOS_OK; 
  
}

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
int cryptos_key_free(cryptos_key_t *key) {

  /* Input parameters control */
  if(!key) {
    return I_CRYPTOS_OK;
  }

  free(key->key);

  return I_CRYPTOS_OK; 
  
}

int cryptos_cipher_algo_code(const char *cipher_algo_name, int *code) {

  int rc;

  if(!code) {
    errno = EINVAL;
    message_log("cryptos_cipher_algo_code", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Defaults to GCRY_CIPHER_ARCFOUR */
  if(!cipher_algo_name) {
    rc = gcry_cipher_map_name("ARCFOUR");
  } else {
    rc = gcry_cipher_map_name(cipher_algo_name);
  }

  *code = rc;
  
  if(!rc) {
    errno = EINVAL;
    message_log("cryptos_cipher_algo_code", "Unknown cipher algorithm");
    return I_CRYPTOS_ERR;
  }
  return I_CRYPTOS_OK;

}

int cryptos_md_algo_code(const char *md_algo_name, int *code) {

  int rc;

  if(!code) {
    errno = EINVAL;
    message_log("cryptos_md_algo_code", strerror(errno));
    return I_CRYPTOS_ERR;
  }

  /* Defaults to SHA1 */
  if(!md_algo_name) {
    rc = gcry_md_map_name("SHA1");
  } else {
    rc = gcry_md_map_name(md_algo_name);
  }
  
  *code = rc;
  if(!rc) {
    errno = EINVAL;
    message_log("cryptos_md_algo_code", "Unknown message digest algorithm");
    return I_CRYPTOS_ERR;
  }
  return I_CRYPTOS_OK;

}

/* cryptos_channel.c ends here */
