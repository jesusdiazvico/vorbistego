/*                               -*- Mode: C -*- 
 * @file: protocols.h
 * @brief:  Headers for the file channel.c from the software TODO!! 
 * @author: Jesus Diaz Vico
 * Maintainer: 
 * @date: dom ene 10 22:33:14 2010 (+0100)
 * @version: 
 * Last-Updated: lun ago 30 11:47:44 2010 (+0200)
 *           By: Jesus
 *     Update #: 104
 * URL: 
 */

#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include "vorbis/codec.h"
//#include "codec_internal.h"
#include "steganos_types.h"
#include "cryptos_types.h"
#include "global_types.h"

/* Functions */

/** 
 * @fn int cryptos_forward(cryptos_config_t *cc, cryptos_protocol_buffer_t *cb, 
 *		    uint64_t data_size )
 * @brief Produces a new cryptographic packet when necessary.
 *
 * When this function is called, and the cryptographic layer buffer is at risk
 * of underflow (i.e., if it has less than MAX_SUBLIMINAL_SIZE bits stores),
 * produces a new crypto packet, storing it in the cryptographic layer buffer.
 * The data to include is read from the file descriptor stored in the 
 * cryptographic layer buffer.
 *
 * @param[in] cc Cryptographic layer configuration structure.
 * @param[in] cb Cryptographic layer buffer.
 * @param[in] data_size The number of bits of data to include in the current
 *  crypto-packet. If a value of 0 is passed, or it exceeds the maximum data
 *  size per packet specified in <i>cc</i>, the default data size will be used
 *  instead.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
int cryptos_forward(cryptos_config_t *cc, cryptos_protocol_buffer_t *cb, 
 		    uint64_t data_size );

/** 
 * @fn int cryptos_inverse(cryptos_config_t *cc, cryptos_protocol_buffer_t *cb)
 * @brief Tries to recover a new crypto packet from the cryptographic layer 
 *  buffer
 *
 * When called, this function explores the cryptographic layer buffer, using the
 * context defined by the cryptographic configuration structure <i>cc</i>, and
 * if a complete new packet is successfully parsed, writes the recovered data
 * in the file descriptor stored in the cryptographic layer buffer.
 *
 * @param[in] cc Cryptographic layer configuration structure.
 * @param[in] cb Cryptographic layer buffer.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
int cryptos_inverse(cryptos_config_t *cc, cryptos_protocol_buffer_t *clb);

/** 
 * @fn int steganos_vorbis_config_init(vorbis_config_t *vc, int rate, int pcmend,
 *                                     int mult, int *postlist, 
 *                                     int *forward_index, int posts)
 * @brief Initializes the structure containing basic vorbis codec info needed in
 *  the steganographic layer.
 *
 * Initializes the structure containing basic vorbis codec info needed in
 * the steganographic layer. It is very important to note that, unless the other
 * *_init functions, this one DOES NOT allocate memory for the internal arrays,
 * as they are just make point the Vorbis ones. Therefore, there exists no 
 * respective *_free function, as the inner variables are supposed to be freed
 * by Vorbis code.
 *
 * @param[in, out] vc Structure that whil group all the essential Vorbis 
 *  variables
 * @param[in] rate Sampling rate
 * @param[in] pcmend Length of the floor and residue vectors
 * @param[in] mult Multiplier used in the posts calculation
 * @param[in] postlist Array of posts
 * @param[in] forward_index Array of posts X axis ordering
 * @param[in] posts Number of elements in the postlist array
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
int steganos_vorbis_config_init(vorbis_config_t *vc, int rate, int pcmend, int mult, 
				int *postlist, int *forward_index, int posts);

/** 
 * @fn int steganos_vorbis_config_free(vorbis_config_t *vc)
 * @brief Frees the given vorbis_config_t structure
 *
 * @param[in] vc Structure to free
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 */
int steganos_vorbis_config_free(vorbis_config_t *vc);

/** 
 * @fn int steganos_forward(steganos_state_t *ss, vorbis_config_t *vc, int *floor, 
		     int *posts, float *residue, byte *buffer, size_t buffer_len,
		     int *hided)
 * @brief Interface to the sender's steganographic functionality. 
 *
 * Through this interface, a sender program will be able to hide in the floor
 * and residue received vectors, from a Vorbis audio stream, the bitstream
 * stored in the field indicated by the <i>input</i> field within the
 * 'steganos_config.xml' file. The steganographic layer will try to hide as 
 * much data as possible, and will update <i>sent</i> with the number 
 * of bits successfully hided.
 *
 * @param[in] ss The steganographic protocol internal structure
 * @param[in] vc Vorbis block and look info.
 * @param[in] floor The Vorbis audio floor vector.
 * @param[in, out] posts The Vorbis posts vector.
 * @param[in, out] residue The Vorbis audio residue vector.
 * @param[in] buffer Buffer containing the data to send.
 * @param[in] buffer_len Length of buffer, in bytes, containing initialized data.
 * @param[in, out] hided Pure data successfully hided, in bits.
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 */
int steganos_forward(steganos_state_t *ss, vorbis_config_t *vc, int *floor, 
		     int *posts, float *residue, byte *buffer, size_t buffer_len,
		     int *hided);

/** 
 * @fn int steganos_inverse(steganos_state_t *ss, vorbis_config_t *vc, 
 *                          int *posts, int *floor, float *residue,
 *                          float sigma, byte *buffer, int buffer_sz,
 *                          int *sd_read)
 * @brief Interface to the receiver's steganographic functionality. 
 *
 * Through this interface, a receiver program can unhide as much subliminal data
 * as may had been hidden in the Vorbis audio <i>floor</i> and/or <i>residue</i> 
 * vectors, if any. 
 * 
 * @param[in] ss The steganographic protocol internal structure
 * @param[in] vc Vorbis block and look info.
 * @param[in] posts The Vorbis audio floor posts vector.
 * @param[in] floor The Vorbis audio floor vector.
 * @param[in] residue The Vorbis audio residue vector.
 * @param[in] sigma Sigma parameter if ISS.
 * @param[in, out] buffer Buffer to store the subliminal data.
 * @param[in] buffer_sz Size of <i>buffer</i>, in bytes.
 * @param[out] sd_read Number of bytes recovered
 * 
 * @return The corresponding error code for integer returning functions, i.e., 
 *  I_STEGANOS_OK if no error was present and I_STEGANOS_ERR if an error occured 
 *  with errno updated.
 * @retval I_STEGANOS_OK with errno = 0 (No error).
 * @retval I_STEGANOS_ERR with errno = EINVAL (invalid argument).
 *
 * @see steganos_state_t
 */
int steganos_inverse(steganos_state_t *ss, vorbis_config_t *vc, int *posts, 
		     int *floor, float *residue, float sigma, byte *buffer,
		     int buffer_sz, int *sd_read);

#endif /* PROTOCOLS_H */

/* protocols.h ends here */
